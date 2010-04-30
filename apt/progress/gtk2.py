#!/usr/bin/env python
#
# Copyright (c) 2004-2009 Canonical
#
# Authors: Michael Vogt <michael.vogt@ubuntu.com>
#          Sebastian Heinlein <glatzor@ubuntu.com>
#          Julian Andres Klode <jak@debian.org>
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of the
#  License, or (at your option) any later version.
#
#  his program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
#  USA
"""GObject-powered progress classes and a GTK+ status widget."""

from gettext import gettext as _
import os
import time

import pygtk
pygtk.require('2.0')
import gtk
try:
    import glib
except ImportError:
    import gobject as glib
import gobject
import pango
import vte

import apt
import apt_pkg


def mksig(params=(), run=gobject.SIGNAL_RUN_FIRST, rettype=gobject.TYPE_NONE):
    """Simplified Create a gobject signal.

    This allows us to write signals easier, because we just need to define the
    type of the parameters (in most cases).

    ``params`` is a tuple which defines the types of the arguments.
    """
    return (run, rettype, params)


class GOpProgress(gobject.GObject, apt.progress.OpProgress):
    """Operation progress with GObject signals.

    Signals:

        * status-changed(str: operation, int: percent)
        * status-started()  - Not Implemented yet
        * status-finished()

    """

    __gsignals__ = {"status-changed": mksig((str, int)),
                    "status-started": mksig(),
                    "status-finished": mksig()}

    def __init__(self):
        apt.progress.OpProgress.__init__(self)
        gobject.GObject.__init__(self)
        self._context = glib.main_context_default()

    def update(self, percent):
        """Called to update the percentage done"""
        self.emit("status-changed", self.op, percent)
        while self._context.pending():
            self._context.iteration()

    def done(self):
        """Called when all operation have finished."""
        self.emit("status-finished")


class GInstallProgress(gobject.GObject, apt.progress.InstallProgress):
    """Installation progress with GObject signals.

    Signals:

        * status-changed(str: status, int: percent)
        * status-started()
        * status-finished()
        * status-timeout()
        * status-error()
        * status-conffile()

    """
    # Seconds until a maintainer script will be regarded as hanging
    INSTALL_TIMEOUT = 5 * 60

    __gsignals__ = {"status-changed": mksig((str, int)),
                    "status-started": mksig(),
                    "status-timeout": mksig(),
                    "status-error": mksig(),
                    "status-conffile": mksig(),
                    "status-finished": mksig()}

    def __init__(self, term):
        apt.progress.InstallProgress.__init__(self)
        gobject.GObject.__init__(self)
        self.finished = False
        self.time_last_update = time.time()
        self.term = term
        reaper = vte.reaper_get()
        reaper.connect("child-exited", self.childExited)
        self.env = ["VTE_PTY_KEEP_FD=%s" % self.writefd,
                    "DEBIAN_FRONTEND=gnome",
                    "APT_LISTCHANGES_FRONTEND=gtk"]
        self._context = glib.main_context_default()

    def childExited(self, term, pid, status):
        """Called when a child process exits"""
        self.apt_status = os.WEXITSTATUS(status)
        self.finished = True

    def error(self, pkg, errormsg):
        """Called when an error happens.

        Emits: status-error()
        """
        self.emit("status-error")

    def conffile(self, current, new):
        """Called during conffile.

        Emits: status-conffile()
        """
        self.emit("status-conffile")

    def startUpdate(self):
        """Called when the update starts.

        Emits: status-started()
        """
        self.emit("status-started")

    def finishUpdate(self):
        """Called when the update finished.

        Emits: status-finished()
        """
        self.emit("status-finished")

    def statusChange(self, pkg, percent, status):
        """Called when the status changed.

        Emits: status-changed(status, percent)
        """
        self.time_last_update = time.time()
        self.emit("status-changed", status, percent)

    def updateInterface(self):
        """Called periodically to update the interface.

        Emits: status-timeout() [When a timeout happens]
        """
        apt.progress.InstallProgress.updateInterface(self)
        while self._context.pending():
            self._context.iteration()
        if self.time_last_update + self.INSTALL_TIMEOUT < time.time():
            self.emit("status-timeout")

    def fork(self):
        """Fork the process."""
        return self.term.forkpty(envv=self.env)

    def waitChild(self):
        """Wait for the child process to exit."""
        while not self.finished:
            self.updateInterface()
        return self.apt_status


class GDpkgInstallProgress(apt.progress.DpkgInstallProgress, GInstallProgress):
    """An InstallProgress for local installations.

    Signals:

        * status-changed(str: status, int: percent)
        * status-started()  - Not Implemented yet
        * status-finished()
        * status-timeout() - When the maintainer script hangs
        * status-error() - When an error happens
        * status-conffile() - On Conffile
    """

    def run(self, debfile):
        """Install the given package."""
        apt.progress.DpkgInstallProgress.run(self, debfile)

    def updateInterface(self):
        """Called periodically to update the interface.

        Emits: status-timeout() [When a timeout happens]"""
        apt.progress.DpkgInstallProgress.updateInterface(self)
        if self.time_last_update + self.INSTALL_TIMEOUT < time.time():
            self.emit("status-timeout")


class GFetchProgress(gobject.GObject, apt.progress.FetchProgress):
    """A Fetch Progress with GObject signals.

    Signals:

        * status-changed(str: description, int: percent)
        * status-started()
        * status-finished()
    """

    __gsignals__ = {"status-changed": mksig((str, int)),
                    "status-started": mksig(),
                    "status-finished": mksig()}

    def __init__(self):
        apt.progress.FetchProgress.__init__(self)
        gobject.GObject.__init__(self)
        self._continue = True
        self._context = glib.main_context_default()

    def start(self):
        self.emit("status-started")

    def stop(self):
        self.emit("status-finished")

    def cancel(self):
        self._continue = False

    def pulse(self):
        apt.progress.FetchProgress.pulse(self)
        currentItem = self.currentItems + 1
        if currentItem > self.totalItems:
            currentItem = self.totalItems
        if self.currentCPS > 0:
            text = (_("Downloading file %(current)li of %(total)li with "
                      "%(speed)s/s") % \
                      {"current": currentItem,
                       "total": self.totalItems,
                       "speed": apt_pkg.SizeToStr(self.currentCPS)})
        else:
            text = (_("Downloading file %(current)li of %(total)li") % \
                      {"current": currentItem,
                       "total": self.totalItems})
        self.emit("status-changed", text, self.percent)
        while self._context.pending():
            self._context.iteration()
        return self._continue


class GtkAptProgress(gtk.VBox):
    """Graphical progress for installation/fetch/operations.

    This widget provides a progress bar, a terminal and a status bar for
    showing the progress of package manipulation tasks.
    """

    def __init__(self):
        gtk.VBox.__init__(self)
        self.set_spacing(6)
        # Setup some child widgets
        self._expander = gtk.Expander(_("Details"))
        self._terminal = vte.Terminal()
        #self._terminal.set_font_from_string("monospace 10")
        self._expander.add(self._terminal)
        self._progressbar = gtk.ProgressBar()
        # Setup the always italic status label
        self._label = gtk.Label()
        attr_list = pango.AttrList()
        attr_list.insert(pango.AttrStyle(pango.STYLE_ITALIC, 0, -1))
        self._label.set_attributes(attr_list)
        self._label.set_ellipsize(pango.ELLIPSIZE_END)
        self._label.set_alignment(0, 0)
        # add child widgets
        self.pack_start(self._progressbar, False)
        self.pack_start(self._label, False)
        self.pack_start(self._expander, False)
        # Setup the internal progress handlers
        self._progress_open = GOpProgress()
        self._progress_open.connect("status-changed", self._on_status_changed)
        self._progress_open.connect("status-started", self._on_status_started)
        self._progress_open.connect("status-finished",
                                    self._on_status_finished)
        self._progress_fetch = GFetchProgress()
        self._progress_fetch.connect("status-changed", self._on_status_changed)
        self._progress_fetch.connect("status-started", self._on_status_started)
        self._progress_fetch.connect("status-finished",
                                     self._on_status_finished)
        self._progress_install = GInstallProgress(self._terminal)
        self._progress_install.connect("status-changed",
                                       self._on_status_changed)
        self._progress_install.connect("status-started",
                                       self._on_status_started)
        self._progress_install.connect("status-finished",
                                     self._on_status_finished)
        self._progress_install.connect("status-timeout",
                                     self._on_status_timeout)
        self._progress_install.connect("status-error",
                                     self._on_status_timeout)
        self._progress_install.connect("status-conffile",
                                     self._on_status_timeout)
        self._progress_dpkg_install = GDpkgInstallProgress(self._terminal)
        self._progress_dpkg_install.connect("status-changed",
                                       self._on_status_changed)
        self._progress_dpkg_install.connect("status-started",
                                       self._on_status_started)
        self._progress_dpkg_install.connect("status-finished",
                                     self._on_status_finished)
        self._progress_dpkg_install.connect("status-timeout",
                                     self._on_status_timeout)
        self._progress_dpkg_install.connect("status-error",
                                     self._on_status_timeout)
        self._progress_dpkg_install.connect("status-conffile",
                                     self._on_status_timeout)

    def clear(self):
        """Reset all status information."""
        self._label.set_label("")
        self._progressbar.set_fraction(0)
        self._expander.set_expanded(False)

    @property
    def open(self):
        """Return the cache opening progress handler."""
        return self._progress_open

    @property
    def install(self):
        """Return the install progress handler."""
        return self._progress_install

    @property
    def dpkg_install(self):
        """Return the install progress handler for dpkg."""
        return self._dpkg_progress_install

    @property
    def fetch(self):
        """Return the fetch progress handler."""
        return self._progress_fetch

    def _on_status_started(self, progress):
        """Called when something starts."""
        self._on_status_changed(progress, _("Starting..."), 0)
        while gtk.events_pending():
            gtk.main_iteration()

    def _on_status_finished(self, progress):
        """Called when something finished."""
        self._on_status_changed(progress, _("Complete"), 100)
        while gtk.events_pending():
            gtk.main_iteration()

    def _on_status_changed(self, progress, status, percent):
        """Called when the status changed."""
        self._label.set_text(status)
        if percent is None:
            self._progressbar.pulse()
        else:
            self._progressbar.set_fraction(percent/100.0)
        while gtk.events_pending():
            gtk.main_iteration()

    def _on_status_timeout(self, progress):
        """Called when timeout happens."""
        self._expander.set_expanded(True)
        while gtk.events_pending():
            gtk.main_iteration()

    def cancel_download(self):
        """Cancel a currently running download."""
        self._progress_fetch.cancel()

    def show_terminal(self, expanded=False):
        """Show the expander for the terminal.

        Show an expander with a terminal widget which provides a way
        to interact with dpkg
        """
        self._expander.show()
        self._terminal.show()
        self._expander.set_expanded(expanded)
        while gtk.events_pending():
            gtk.main_iteration()

    def hide_terminal(self):
        """Hide the expander with the terminal widget."""
        self._expander.hide()
        while gtk.events_pending():
            gtk.main_iteration()

    def show(self):
        """Show the Box"""
        gtk.HBox.show(self)
        self._label.show()
        self._progressbar.show()
        while gtk.events_pending():
            gtk.main_iteration()


def _test():
    """Test function"""
    import sys

    from apt.debfile import DebPackage

    win = gtk.Window()
    apt_progress = GtkAptProgress()
    win.set_title("GtkAptProgress Demo")
    win.add(apt_progress)
    apt_progress.show()
    win.show()
    cache = apt.cache.Cache(apt_progress.open)
    pkg = cache["xterm"]
    if pkg.isInstalled:
        pkg.markDelete()
    else:
        pkg.markInstall()
    apt_progress.show_terminal(True)
    try:
        cache.commit(apt_progress.fetch, apt_progress.install)
    except Exception, exc:
        print >> sys.stderr, "Exception happened:", exc
    if len(sys.argv) > 1:
        deb = DebPackage(sys.argv[1], cache)
        deb.install(apt_progress.dpkg_install)
    gtk.main()


if __name__ == "__main__":
    _test()

# vim: ts=4 et sts=4
