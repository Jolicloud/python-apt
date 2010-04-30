# progress.py - progress reporting classes
#
#  Copyright (c) 2005-2009 Canonical
#
#  Author: Michael Vogt <michael.vogt@ubuntu.com>
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
"""progress reporting classes.

This module provides classes for progress reporting. They can be used with
e.g., for reporting progress on the cache opening process, the cache update
progress, or the package install progress.
"""

import errno
import fcntl
import os
import re
import select
import sys

import apt_pkg


__all__ = ('CdromProgress', 'DpkgInstallProgress', 'DumbInstallProgress',
          'FetchProgress', 'InstallProgress', 'OpProgress', 'OpTextProgress',
          'TextFetchProgress')


class OpProgress(object):
    """Abstract class to implement reporting on cache opening.

    Subclass this class to implement simple Operation progress reporting.
    """

    def __init__(self):
        self.op = None
        self.subOp = None

    def update(self, percent):
        """Called periodically to update the user interface."""

    def done(self):
        """Called once an operation has been completed."""


class OpTextProgress(OpProgress):
    """A simple text based cache open reporting class."""

    def __init__(self):
        OpProgress.__init__(self)

    def update(self, percent):
        """Called periodically to update the user interface."""
        sys.stdout.write("\r%s: %.2i  " % (self.subOp, percent))
        sys.stdout.flush()

    def done(self):
        """Called once an operation has been completed."""
        sys.stdout.write("\r%s: Done\n" % self.op)


class FetchProgress(object):
    """Report the download/fetching progress.

    Subclass this class to implement fetch progress reporting
    """

    # download status constants
    dlDone = 0
    dlQueued = 1
    dlFailed = 2
    dlHit = 3
    dlIgnored = 4
    dlStatusStr = {dlDone: "Done",
                   dlQueued: "Queued",
                   dlFailed: "Failed",
                   dlHit: "Hit",
                   dlIgnored: "Ignored"}

    def __init__(self):
        self.eta = 0.0
        self.percent = 0.0
        # Make checking easier
        self.currentBytes = 0
        self.currentItems = 0
        self.totalBytes = 0
        self.totalItems = 0
        self.currentCPS = 0

    def start(self):
        """Called when the fetching starts."""

    def stop(self):
        """Called when all files have been fetched."""

    def updateStatus(self, uri, descr, shortDescr, status):
        """Called when the status of an item changes.

        This happens eg. when the downloads fails or is completed.
        """
    def update_status_full(self, uri, descr, short_descr, status, file_size,
                           partial_size):
        """Called when the status of an item changes.

        This happens eg. when the downloads fails or is completed. This
        version include information on current filesize and partial size
        """

    def pulse(self):
        """Called periodically to update the user interface.

        Return True to continue or False to cancel.
        """
        self.percent = (((self.currentBytes + self.currentItems) * 100.0) /
                        float(self.totalBytes + self.totalItems))
        if self.currentCPS > 0:
            self.eta = ((self.totalBytes - self.currentBytes) /
                        float(self.currentCPS))
        return True

    def pulse_items(self, items):
        """Called periodically to update the user interface.
        This function includes details about the items being fetched
        Return True to continue or False to cancel.

        """
        self.percent = (((self.currentBytes + self.currentItems) * 100.0) /
                        float(self.totalBytes + self.totalItems))
        if self.currentCPS > 0:
            self.eta = ((self.totalBytes - self.currentBytes) /
                        float(self.currentCPS))
        return True

    def mediaChange(self, medium, drive):
        """react to media change events."""


class TextFetchProgress(FetchProgress):
    """ Ready to use progress object for terminal windows """

    def __init__(self):
        FetchProgress.__init__(self)
        self.items = {}

    def updateStatus(self, uri, descr, shortDescr, status):
        """Called when the status of an item changes.

        This happens eg. when the downloads fails or is completed.
        """
        if status != self.dlQueued:
            print "\r%s %s" % (self.dlStatusStr[status], descr)
        self.items[uri] = status

    def pulse(self):
        """Called periodically to update the user interface.

        Return True to continue or False to cancel.
        """
        FetchProgress.pulse(self)
        if self.currentCPS > 0:
            s = "[%2.f%%] %sB/s %s" % (self.percent,
                                       apt_pkg.SizeToStr(int(self.currentCPS)),
                                       apt_pkg.TimeToStr(int(self.eta)))
        else:
            s = "%2.f%% [Working]" % (self.percent)
        print "\r%s" % (s),
        sys.stdout.flush()
        return True

    def stop(self):
        """Called when all files have been fetched."""
        print "\rDone downloading            "

    def mediaChange(self, medium, drive):
        """react to media change events."""
        print ("Media change: please insert the disc labeled "
               "'%s' in the drive '%s' and press enter") % (medium, drive)

        return raw_input() not in ('c', 'C')


class DumbInstallProgress(object):
    """Report the install progress.

    Subclass this class to implement install progress reporting.
    """

    def startUpdate(self):
        """Start update."""

    def run(self, pm):
        """Start installation."""
        return pm.DoInstall()

    def finishUpdate(self):
        """Called when update has finished."""

    def updateInterface(self):
        """Called periodically to update the user interface"""


class InstallProgress(DumbInstallProgress):
    """An InstallProgress that is pretty useful.

    It supports the attributes 'percent' 'status' and callbacks for the dpkg
    errors and conffiles and status changes.
    """

    def __init__(self):
        DumbInstallProgress.__init__(self)
        self.selectTimeout = 0.1
        (read, write) = os.pipe()
        self.writefd = write
        self.statusfd = os.fdopen(read, "r")
        fcntl.fcntl(self.statusfd.fileno(), fcntl.F_SETFL, os.O_NONBLOCK)
        self.read = ""
        self.percent = 0.0
        self.status = ""

    def error(self, pkg, errormsg):
        """Called when a error is detected during the install."""

    def conffile(self, current, new):
        """Called when a conffile question from dpkg is detected."""

    def statusChange(self, pkg, percent, status):
        """Called when the status changed."""

    def updateInterface(self):
        """Called periodically to update the interface."""
        if self.statusfd is None:
            return
        try:
            while not self.read.endswith("\n"):
                self.read += os.read(self.statusfd.fileno(), 1)
        except OSError, (errno_, errstr):
            # resource temporarly unavailable is ignored
            if errno_ != errno.EAGAIN and errno_ != errno.EWOULDBLOCK:
                print errstr
        if not self.read.endswith("\n"):
            return

        s = self.read
        #print s
        try:
            (status, pkg, percent, status_str) = s.split(":", 3)
        except ValueError:
            # silently ignore lines that can't be parsed
            self.read = ""
            return
        #print "percent: %s %s" % (pkg, float(percent)/100.0)
        if status == "pmerror":
            self.error(pkg, status_str)
        elif status == "pmconffile":
            # we get a string like this:
            # 'current-conffile' 'new-conffile' useredited distedited
            match = re.match("\s*\'(.*)\'\s*\'(.*)\'.*", status_str)
            if match:
                self.conffile(match.group(1), match.group(2))
        elif status == "pmstatus":
            if float(percent) != self.percent or status_str != self.status:
                self.statusChange(pkg, float(percent),
                                  status_str.strip())
                self.percent = float(percent)
                self.status = status_str.strip()
        self.read = ""

    def fork(self):
        """Fork."""
        return os.fork()

    def waitChild(self):
        """Wait for child progress to exit.

        The return values is the full status returned from os.waitpid()
        (not only the return code).
        """
        while True:
            try:
                select.select([self.statusfd], [], [], self.selectTimeout)
            except select.error, (errno_, errstr):
                if errno_ != errno.EINTR:
                    raise
            self.updateInterface()
            try:
                (pid, res) = os.waitpid(self.child_pid, os.WNOHANG)
                if pid == self.child_pid:
                    break
            except OSError, (errno_, errstr):
                if errno_ != errno.EINTR:
                    raise
                if errno_ == errno.ECHILD:
                    break
        return res

    def run(self, pm):
        """Start installing. 
        
        Returns the PackageManager status:
        (pm.ResultCompleted, pm.ResultFailed, pm.ResultIncomplete)
        """
        pid = self.fork()
        if pid == 0:
            # pm.DoInstall might raise a exception,
            # when this happens, we need to catch
            # it, otherwise os._exit() is not run
            # and the execution continues in the 
            # parent code leading to very confusing bugs
            try:
                res = pm.DoInstall(self.writefd)
            except Exception, e:
                os._exit(pm.ResultFailed)
            os._exit(res)
        self.child_pid = pid
        res = self.waitChild()
        return os.WEXITSTATUS(res)


class CdromProgress(object):
    """Report the cdrom add progress.

    Subclass this class to implement cdrom add progress reporting.
    """

    def __init__(self):
        pass

    def update(self, text, step):
        """Called periodically to update the user interface."""

    def askCdromName(self):
        """Called to ask for the name of the cdrom."""

    def changeCdrom(self):
        """Called to ask for the cdrom to be changed."""


class DpkgInstallProgress(InstallProgress):
    """Progress handler for a local Debian package installation."""

    def run(self, debfile):
        """Start installing the given Debian package."""
        self.debfile = debfile
        self.debname = os.path.basename(debfile).split("_")[0]
        pid = self.fork()
        if pid == 0:
            # child
            res = os.system("/usr/bin/dpkg --status-fd %s -i %s" % \
                            (self.writefd, debfile))
            os._exit(os.WEXITSTATUS(res))
        self.child_pid = pid
        res = self.waitChild()
        return res

    def updateInterface(self):
        """Process status messages from dpkg."""
        if self.statusfd is None:
            return
        while True:
            try:
                self.read += os.read(self.statusfd.fileno(), 1)
            except OSError, (errno_, errstr):
                # resource temporarly unavailable is ignored
                if errno_ != 11:
                    print errstr
                break
            if not self.read.endswith("\n"):
                continue

            statusl = self.read.split(":")
            if len(statusl) < 3:
                print "got garbage from dpkg: '%s'" % self.read
                self.read = ""
                break
            pkg_name = statusl[1].strip()
            status = statusl[2].strip()
            #print status
            if status == "error":
                self.error(pkg_name, status)
            elif status == "conffile-prompt":
                # we get a string like this:
                # 'current-conffile' 'new-conffile' useredited distedited
                match = re.match("\s*\'(.*)\'\s*\'(.*)\'.*", statusl[3])
                if match:
                    self.conffile(match.group(1), match.group(2))
            else:
                self.status = status
            self.read = ""
