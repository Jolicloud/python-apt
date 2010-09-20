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
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
#  USA
"""GObject-powered progress classes and a GTK+ status widget."""

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

import apt_pkg
from apt_pkg import gettext as _
from apt.deprecation import function_deprecated_by, AttributeDeprecatedBy
from apt.progress import base

if apt_pkg._COMPAT_0_7:
    from apt.progress import old


__all__ = ['GAcquireProgress', 'GInstallProgress', 'GOpProgress',
           'GtkAptProgress']


def mksig(params=(), run=gobject.SIGNAL_RUN_FIRST, rettype=gobject.TYPE_NONE):
    """Simplified Create a gobject signal.

    This allows us to write signals easier, because we just need to define the
    type of the parameters (in most cases).

    ``params`` is a tuple which defines the types of the arguments.
    """
    return (run, rettype, params)


class GOpProgress(gobject.GObject, base.OpProgress):
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
        base.OpProgress.__init__(self)
        gobject.GObject.__init__(self)
        self._context = glib.main_context_default()

    def update(self, percent=None):
        """Called to update the percentage done"""
        base.OpProgress.update(self, percent)
        self.emit("status-changed", self.op, self.percent)
        while self._context.pending():
            self._context.iteration()

    def done(self):
        """Called when all operation have finished."""
        base.OpProgress.done(self)
        self.emit("status-finished")

    if apt_pkg._COMPAT_0_7:
        subOp = AttributeDeprecatedBy('subop')
        Op = AttributeDeprecatedBy('op')


class GInstallProgress(gobject.GObject, base.InstallProgress):
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
        base.InstallProgress.__init__(self)
        gobject.GObject.__init__(self)
        self.finished = False
        self.apt_status = -1
        self.time_last_update = time.time()
        self.term = term
        reaper = vte.reaper_get()
        reaper.connect("child-exited", self.child_exited)
        self.env = ["VTE_PTY_KEEP_FD=%s" % self.writefd,
                    "DEBIAN_FRONTEND=gnome",
                    "APT_LISTCHANGES_FRONTEND=gtk"]
        self._context = glib.main_context_default()

    def child_exited(self, term, pid, status):
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

    def start_update(self):
        """Called when the update starts.

        Emits: status-started()
        """
        self.emit("status-started")

    def run(self, obj):
        """Run."""
        self.finished = False
        return base.InstallProgress.run(self, obj)

    def finish_update(self):
        """Called when the update finished.

        Emits: status-finished()
        """
        self.emit("status-finished")

    def processing(self, pkg, stage):
        """Called when entering a new stage in dpkg."""
        # We have no percentage or alike, send -1 to let the bar pulse.
        self.emit("status-changed", ("Installing %s...") % pkg, -1)

    def status_change(self, pkg, percent, status):
        """Called when the status changed.

        Emits: status-changed(status, percent)
        """
        self.time_last_update = time.time()
        self.emit("status-changed", status, percent)

    def update_interface(self):
        """Called periodically to update the interface.

        Emits: status-timeout() [When a timeout happens]
        """
        base.InstallProgress.update_interface(self)
        while self._context.pending():
            self._context.iteration()
        if self.time_last_update + self.INSTALL_TIMEOUT < time.time():
            self.emit("status-timeout")

    def fork(self):
        """Fork the process."""
        return self.term.forkpty(envv=self.env)

    def wait_child(self):
        """Wait for the child process to exit."""
        while not self.finished:
            self.update_interface()
        return self.apt_status

    if apt_pkg._COMPAT_0_7:
        updateInterface = function_deprecated_by(update_interface)
        startUpdate = function_deprecated_by(start_update)
        finishUpdate = function_deprecated_by(finish_update)
        statusChange = function_deprecated_by(status_change)
        waitChild = function_deprecated_by(wait_child)
        childExited = function_deprecated_by(child_exited)


GDpkgInstallProgress = GInstallProgress


class GAcquireProgress(gobject.GObject, base.AcquireProgress):
    """A Fetch Progress with GObject signals.

    Signals:

        * status-changed(str: description, int: percent)
        * status-started()
        * status-finished()

    DEPRECATED.
    """

    __gsignals__ = {"status-changed": mksig((str, int)),
                    "status-started": mksig(),
                    "status-finished": mksig()}

    def __init__(self):
        base.AcquireProgress.__init__(self)
        gobject.GObject.__init__(self)
        self._continue = True
        self._context = glib.main_context_default()

    def start(self):
        base.AcquireProgress.start(self)
        self.emit("status-started")

    def stop(self):
        base.AcquireProgress.stop(self)
        self.emit("status-finished")

    def cancel(self):
        self._continue = False

    def pulse(self, owner):
        base.AcquireProgress.pulse(self, owner)
        current_item = self.current_items + 1
        if current_item > self.total_items:
            current_item = self.total_items
        if self.current_cps > 0:
            text = (_("Downloading file %(current)li of %(total)li with "
                      "%(speed)s/s") % \
                      {"current": current_item,
                       "total": self.total_items,
                       "speed": apt_pkg.size_to_str(self.current_cps)})
        else:
            text = (_("Downloading file %(current)li of %(total)li") % \
                      {"current": current_item,
                       "total": self.total_items})

        percent = (((self.current_bytes + self.current_items) * 100.0) /
                        float(self.total_bytes + self.total_items))
        self.emit("status-changed", text, percent)
        while self._context.pending():
            self._context.iteration()
        return self._continue

if apt_pkg._COMPAT_0_7:

    class GFetchProgress(gobject.GObject, old.FetchProgress):
        """A Fetch Progress with GObject signals.

        Signals:

            * status-changed(str: description, int: percent)
            * status-started()
            * status-finished()

        DEPRECATED.
        """

        __gsignals__ = {"status-changed": mksig((str, int)),
                        "status-started": mksig(),
                        "status-finished": mksig()}

        def __init__(self):
            old.FetchProgress.__init__(self)
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
            old.FetchProgress.pulse(self)
            current_item = self.currentItems + 1
            if current_item > self.totalItems:
                current_item = self.totalItems
            if self.current_cps > 0:
                text = (_("Downloading file %(current)li of %(total)li with "
                          "%(speed)s/s") % \
                          {"current": current_item,
                           "total": self.totalItems,
                           "speed": apt_pkg.size_to_str(self.currentCPS)})
            else:
                text = (_("Downloading file %(current)li of %(total)li") % \
                          {"current": current_item,
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
        self._progress_acquire = GAcquireProgress()
        self._progress_acquire.connect("status-changed",
                                       self._on_status_changed)
        self._progress_acquire.connect("status-started",
                                       self._on_status_started)
        self._progress_acquire.connect("status-finished",
                                     self._on_status_finished)

        self._progress_fetch = None
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
        return self._progress_install

    if apt_pkg._COMPAT_0_7:

        @property
        def fetch(self):
            """Return the fetch progress handler."""
            if self._progress_fetch is None:
                self._progress_fetch = GFetchProgress()
                self._progress_fetch.connect("status-changed",
                                            self._on_status_changed)
                self._progress_fetch.connect("status-started",
                                            self._on_status_started)
                self._progress_fetch.connect("status-finished",
                                            self._on_status_finished)
            return self._progress_fetch

    @property
    def acquire(self):
        """Return the acquire progress handler."""
        return self._progress_acquire

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
        if percent is None or percent == -1:
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

    import apt
    from apt.debfile import DebPackage

    win = gtk.Window()
    apt_progress = GtkAptProgress()
    win.set_title("GtkAptProgress Demo")
    win.add(apt_progress)
    apt_progress.show()
    win.show()
    cache = apt.cache.Cache(apt_progress.open)
    pkg = cache["xterm"]
    if pkg.is_installed:
        pkg.mark_delete()
    else:
        pkg.mark_install()
    apt_progress.show_terminal(True)
    try:
        cache.commit(apt_progress.acquire, apt_progress.install)
    except Exception, exc:
        print >> sys.stderr, "Exception happened:", exc
    if len(sys.argv) > 1:
        deb = DebPackage(sys.argv[1], cache)
        deb.install(apt_progress.dpkg_install)
    win.connect("destroy", gtk.main_quit)
    gtk.main()


if __name__ == "__main__":
    _test()

# vim: ts=4 et sts=4
