# apt/progress/__init__.py - Initialization file for apt.progress.
#
# Copyright (c) 2009 Julian Andres Klode <jak@debian.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
"""Progress reporting.

This package provides progress reporting for the python-apt package. The module
'base' provides classes with no output, the module 'gtk2' provides classes for
GTK+ applications, and the module 'text' provides classes for terminals, etc.
"""
import apt_pkg


__all__ = []


if apt_pkg._COMPAT_0_7:
    from apt.progress.old import (CdromProgress, DpkgInstallProgress,
                                  DumbInstallProgress, FetchProgress,
                                  InstallProgress, OpProgress,
                                  OpTextProgress, TextFetchProgress)
