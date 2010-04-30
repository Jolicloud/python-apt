# package.py - apt package abstraction
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
"""Functionality related to packages."""
import gettext
import httplib
import os
import sys
import re
import socket
import subprocess
import urllib2
import warnings

try:
    from collections import Sequence
except ImportError:
    Sequence = object

import apt_pkg
import apt.progress

__all__ = ('BaseDependency', 'Dependency', 'Origin', 'Package', 'Record',
           'Version', 'VersionList')


def _(string):
    """Return the translation of the string."""
    return gettext.dgettext("python-apt", string)


def _file_is_same(path, size, md5):
    """Return True if the file is the same."""
    if (os.path.exists(path) and os.path.getsize(path) == size and
        apt_pkg.md5sum(open(path)) == md5):
        return True


class FetchError(Exception):
    """Raised when a file could not be fetched."""


class BaseDependency(object):
    """A single dependency.

    Attributes defined here:
        name      - The name of the dependency
        relation  - The relation (>>,>=,==,<<,<=,)
        version   - The version depended on
        rawtype   - The type of the dependendy (e.g. 'Recommends')
        preDepend - Boolean value whether this is a pre-dependency.
    """

    def __init__(self, name, rel, ver, pre, rawtype=None):
        self.name = name
        self.relation = rel
        self.version = ver
        self.preDepend = pre
        self.rawtype = rawtype

    def __repr__(self):
        return ('<BaseDependency: name:%r relation:%r version:%r preDepend:%r>'
                % (self.name, self.relation, self.version, self.preDepend))


class Dependency(object):
    """Represent an Or-group of dependencies.

    Attributes defined here:
        or_dependencies - The possible choices
    """

    def __init__(self, alternatives):
        self.or_dependencies = alternatives

    def __repr__(self):
        return repr(self.or_dependencies)


class DeprecatedProperty(property):
    """A property which gives DeprecationWarning on access.

    This is only used for providing the properties in Package, which have been
    replaced by the ones in Version.
    """

    def __init__(self, fget=None, fset=None, fdel=None, doc=None):
        property.__init__(self, fget, fset, fdel, doc)
        self.__doc__ = ':Deprecated: ' + (doc or fget.__doc__ or '')

    def __get__(self, obj, type=None):
        if obj is not None:
            warnings.warn("Accessed deprecated property %s.%s, please see the "
                          "Version class for alternatives." %
                           ((obj.__class__.__name__ or type.__name__),
                           self.fget.func_name), DeprecationWarning, 2)
        return property.__get__(self, obj, type)


class Origin(object):
    """The origin of a version.

    Attributes defined here:
        archive   - The archive (eg. unstable)
        component - The component (eg. main)
        label     - The Label, as set in the Release file
        origin    - The Origin, as set in the Release file
        site      - The hostname of the site.
        trusted   - Boolean value whether this is trustworthy.
    """

    def __init__(self, pkg, VerFileIter):
        self.archive = VerFileIter.Archive
        self.component = VerFileIter.Component
        self.label = VerFileIter.Label
        self.origin = VerFileIter.Origin
        self.site = VerFileIter.Site
        self.not_automatic = VerFileIter.NotAutomatic
        # check the trust
        indexfile = pkg._pcache._list.FindIndex(VerFileIter)
        if indexfile and indexfile.IsTrusted:
            self.trusted = True
        else:
            self.trusted = False

    def __repr__(self):
        return ("<Origin component:%r archive:%r origin:%r label:%r "
                "site:%r isTrusted:%r>") % (self.component, self.archive,
                                            self.origin, self.label,
                                            self.site, self.trusted)


class Record(object):
    """Represent a pkgRecord.

    It can be accessed like a dictionary and can also give the original package
    record if accessed as a string.
    """

    def __init__(self, record_str):
        self._rec = apt_pkg.ParseSection(record_str)

    def __str__(self):
        return str(self._rec)

    def __getitem__(self, key):
        return self._rec[key]

    def __contains__(self, key):
        return self._rec.has_key(key)

    def __iter__(self):
        return iter(self._rec.keys())

    def iteritems(self):
        """An iterator over the (key, value) items of the record."""
        for key in self._rec.keys():
            yield key, self._rec[key]

    def get(self, key, default=None):
        """Return record[key] if key in record, else `default`.

        The parameter `default` must be either a string or None.
        """
        return self._rec.get(key, default)

    def has_key(self, key):
        """deprecated form of 'key in x'."""
        return self._rec.has_key(key)


class Version(object):
    """Representation of a package version.

    :since: 0.7.9
    """

    def __init__(self, package, cand):
        self.package = package
        self._cand = cand

    def _cmp(self, other):
        try:
            return apt_pkg.VersionCompare(self._cand.VerStr, other.version)
        except AttributeError:
            return apt_pkg.VersionCompare(self._cand.VerStr, other)

    def __eq__(self, other):
        try:
            return self._cmp(other) == 0
        except TypeError:
            return NotImplemented

    def __ge__(self, other):
        try:
            return self._cmp(other) >= 0
        except TypeError:
            return NotImplemented

    def __gt__(self, other):
        try:
            return self._cmp(other) > 0
        except TypeError:
            return NotImplemented

    def __le__(self, other):
        try:
            return self._cmp(other) <= 0
        except TypeError:
            return NotImplemented

    def __lt__(self, other):
        try:
            return self._cmp(other) < 0
        except TypeError:
            return NotImplemented

    def __ne__(self, other):
        try:
            return self._cmp(other) != 0
        except TypeError:
            return NotImplemented

    def __hash__(self):
        return self._cand.Hash

    def __repr__(self):
        return '<Version: package:%r version:%r>' % (self.package.name,
                                                     self.version)

    @property
    def _records(self):
        """Internal helper that moves the Records to the right position."""
        if self.package._pcache._records.Lookup(self._cand.FileList[0]):
            return self.package._pcache._records

    @property
    def installed_size(self):
        """Return the size of the package when installed."""
        return self._cand.InstalledSize

    @property
    def homepage(self):
        """Return the homepage for the package."""
        return self._records.Homepage

    @property
    def size(self):
        """Return the size of the package."""
        return self._cand.Size

    @property
    def architecture(self):
        """Return the architecture of the package version."""
        return self._cand.Arch

    @property
    def downloadable(self):
        """Return whether the version of the package is downloadable."""
        return bool(self._cand.Downloadable)

    @property
    def version(self):
        """Return the version as a string."""
        return self._cand.VerStr

    @property
    def summary(self):
        """Return the short description (one line summary)."""
        desc_iter = self._cand.TranslatedDescription
        self.package._pcache._records.Lookup(desc_iter.FileList.pop(0))
        return self.package._pcache._records.ShortDesc

    @property
    def raw_description(self):
        """return the long description (raw)."""
        return self._records.LongDesc

    @property
    def section(self):
        """Return the section of the package."""
        return self._cand.Section

    @property
    def description(self, format=True, useDots=False):
        """Return the formatted long description.

        Return the formated long description according to the Debian policy
        (Chapter 5.6.13).
        See http://www.debian.org/doc/debian-policy/ch-controlfields.html
        for more information.
        """
        self.summary # This does the lookup for us.
        desc = ''
        try:
            dsc = unicode(self.package._pcache._records.LongDesc, "utf-8")
        except UnicodeDecodeError, err:
            return _("Invalid unicode in description for '%s' (%s). "
                  "Please report.") % (self.package.name, err)

        lines = iter(dsc.split("\n"))
        # Skip the first line, since its a duplication of the summary
        lines.next()
        for raw_line in lines:
            if raw_line.strip() == ".":
                # The line is just line break
                if not desc.endswith("\n"):
                    desc += "\n\n"
                continue
            if raw_line.startswith("  "):
                # The line should be displayed verbatim without word wrapping
                if not desc.endswith("\n"):
                    line = "\n%s\n" % raw_line[2:]
                else:
                    line = "%s\n" % raw_line[2:]
            elif raw_line.startswith(" "):
                # The line is part of a paragraph.
                if desc.endswith("\n") or desc == "":
                    # Skip the leading white space
                    line = raw_line[1:]
                else:
                    line = raw_line
            else:
                line = raw_line
            # Add current line to the description
            desc += line
        return desc

    @property
    def source_name(self):
        """Return the name of the source package."""
        try:
            return self._records.SourcePkg or self.package.name
        except IndexError:
            return self.package.name

    @property
    def priority(self):
        """Return the priority of the package, as string."""
        return self._cand.PriorityStr

    @property
    def record(self):
        """Return a Record() object for this version."""
        return Record(self._records.Record)

    def get_dependencies(self, *types):
        """Return a list of Dependency objects for the given types."""
        depends_list = []
        depends = self._cand.DependsList
        for t in types:
            try:
                for depVerList in depends[t]:
                    base_deps = []
                    for depOr in depVerList:
                        base_deps.append(BaseDependency(depOr.TargetPkg.Name,
                                        depOr.CompType, depOr.TargetVer,
                                        (t == "PreDepends"), rawtype=t))
                    depends_list.append(Dependency(base_deps))
            except KeyError:
                pass
        return depends_list

    @property
    def dependencies(self):
        """Return the dependencies of the package version."""
        return self.get_dependencies("PreDepends", "Depends")

    @property
    def recommends(self):
        """Return the recommends of the package version."""
        return self.get_dependencies("Recommends")

    @property
    def origins(self):
        """Return a list of origins for the package version."""
        origins = []
        for (verFileIter, index) in self._cand.FileList:
            origins.append(Origin(self.package, verFileIter))
        return origins

    @property
    def filename(self):
        """Return the path to the file inside the archive."""
        return self._records.FileName

    @property
    def md5(self):
        """Return the md5sum of the binary."""
        return self._records.MD5Hash

    @property
    def sha1(self):
        """Return the sha1sum of the binary."""
        return self._records.SHA1Hash

    @property
    def sha256(self):
        """Return the sha1sum of the binary."""
        return self._records.SHA256Hash

    def _uris(self):
        """Return an iterator over all available urls."""
        for (packagefile, index) in self._cand.FileList:
            indexfile = self.package._pcache._list.FindIndex(packagefile)
            if indexfile:
                yield indexfile.ArchiveURI(self._records.FileName)

    @property
    def uris(self):
        """Return a list of all available uris for the binary."""
        return list(self._uris())

    @property
    def uri(self):
        """Return a single URI for the binary."""
        return self._uris().next()

    def fetch_binary(self, destdir='', progress=None):
        """Fetch the binary version of the package.

        The parameter 'destdir' specifies the directory where the package will
        be fetched to.

        The parameter 'progress' may refer to an apt.progress.FetchProgress()
        object. If not specified or None, apt.progress.TextFetchProgress() is
        used.
        """
        base = os.path.basename(self._records.FileName)
        destfile = os.path.join(destdir, base)
        if _file_is_same(destfile, self.size, self._records.MD5Hash):
            print 'Ignoring already existing file:', destfile
            return
        acq = apt_pkg.GetAcquire(progress or apt.progress.TextFetchProgress())
        apt_pkg.GetPkgAcqFile(acq, self.uri, self._records.MD5Hash, self.size,
                              base, destFile=destfile)
        acq.Run()
        for item in acq.Items:
            if item.Status != item.StatDone:
                raise FetchError("The item %r could not be fetched: %s" %
                                    (item.DestFile, item.ErrorText))
        return os.path.abspath(destfile)

    def fetch_source(self, destdir="", progress=None, unpack=True):
        """Get the source code of a package.

        The parameter 'destdir' specifies the directory where the source will
        be fetched to.

        The parameter 'progress' may refer to an apt.progress.FetchProgress()
        object. If not specified or None, apt.progress.TextFetchProgress() is
        used.

        The parameter 'unpack' describes whether the source should be unpacked
        (True) or not (False). By default, it is unpacked.

        If 'unpack' is True, the path to the extracted directory is returned.
        Otherwise, the path to the .dsc file is returned.
        """
        src = apt_pkg.GetPkgSrcRecords()
        acq = apt_pkg.GetAcquire(progress or apt.progress.TextFetchProgress())

        dsc = None
        src.Lookup(self.package.name)
        try:
            while self.version != src.Version:
                src.Lookup(self.package.name)
        except AttributeError:
            raise ValueError("No source for %r" % self)
        for md5, size, path, type in src.Files:
            base = os.path.basename(path)
            destfile = os.path.join(destdir, base)
            if type == 'dsc':
                dsc = destfile
            if os.path.exists(base) and os.path.getsize(base) == size:
                fobj = open(base)
                try:
                    if apt_pkg.md5sum(fobj) == md5:
                        print 'Ignoring already existing file:', destfile
                        continue
                finally:
                    fobj.close()
            apt_pkg.GetPkgAcqFile(acq, src.Index.ArchiveURI(path), md5, size,
                                  base, destFile=destfile)
        acq.Run()

        for item in acq.Items:
            if item.Status != item.StatDone:
                raise FetchError("The item %r could not be fetched: %s" %
                                    (item.DestFile, item.ErrorText))

        if unpack:
            outdir = src.Package + '-' + apt_pkg.UpstreamVersion(src.Version)
            outdir = os.path.join(destdir, outdir)
            subprocess.check_call(["dpkg-source", "-x", dsc, outdir])
            return os.path.abspath(outdir)
        else:
            return os.path.abspath(dsc)


class VersionList(Sequence):
    """Provide a mapping & sequence interface to all versions of a package.

    This class can be used like a dictionary, where version strings are the
    keys. It can also be used as a sequence, where integers are the keys.

    You can also convert this to a dictionary or a list, using the usual way
    of dict(version_list) or list(version_list). This is useful if you need
    to access the version objects multiple times, because they do not have to
    be recreated this way.

    Examples ('package.versions' being a version list):
        '0.7.92' in package.versions # Check whether 0.7.92 is a valid version.
        package.versions[0] # Return first version or raise IndexError
        package.versions[0:2] # Return a new VersionList for objects 0-2
        package.versions['0.7.92'] # Return version 0.7.92 or raise KeyError
        package.versions.keys() # All keys, as strings.
        max(package.versions)
    """

    def __init__(self, package, slice=None):
        self._package = package # apt.package.Package()
        self._versions = package._pkg.VersionList # [apt_pkg.Version(), ...]
        if slice:
            self._versions = self._versions[slice]

    def __getitem__(self, item):
        if isinstance(item, slice):
            return self.__class__(self._package, item)
        try:
            # Sequence interface, item is an integer
            return Version(self._package, self._versions[item])
        except TypeError:
            # Dictionary interface item is a string.
            for ver in self._versions:
                if ver.ver_str == item:
                    return Version(self._package, ver)
        raise KeyError("Version: %r not found." % (item))

    def __repr__(self):
        return '<VersionList: %r>' % self.keys()

    def __iter__(self):
        """Return an iterator over all value objects."""
        return (Version(self._package, ver) for ver in self._versions)

    def __contains__(self, item):
        if isinstance(item, Version): # Sequence interface
            item = item.version
        # Dictionary interface.
        for ver in self._versions:
            if ver.ver_str == item:
                return True
        return False

    def __eq__(self, other):
        return list(self) == list(other)

    def __len__(self):
        return len(self._versions)

    # Mapping interface

    def keys(self):
        """Return a list of all versions, as strings."""
        return [ver.VerStr for ver in self._versions]

    def get(self, key, default=None):
        """Return the key or the default."""
        try:
            return self[key]
        except LookupError:
            return default


class Package(object):
    """Representation of a package in a cache.

    This class provides methods and properties for working with a package. It
    lets you mark the package for installation, check if it is installed, and
    much more.
    """

    def __init__(self, pcache, pkgiter):
        """ Init the Package object """
        self._pkg = pkgiter
        self._pcache = pcache           # python cache in cache.py
        self._changelog = ""            # Cached changelog

    def __repr__(self):
        return '<Package: name:%r id:%r>' % (self._pkg.Name, self._pkg.ID)

    def candidate(self):
        """Return the candidate version of the package.

        This property is writeable to allow you to set the candidate version
        of the package. Just assign a Version() object, and it will be set as
        the candidate version.
        """
        cand = self._pcache._depcache.GetCandidateVer(self._pkg)
        if cand is not None:
            return Version(self, cand)

    def __set_candidate(self, version):
        """Set the candidate version of the package."""
        self._pcache.cachePreChange()
        self._pcache._depcache.SetCandidateVer(self._pkg, version._cand)
        self._pcache.cachePostChange()

    candidate = property(candidate, __set_candidate)

    @property
    def installed(self):
        """Return the currently installed version of the package.

        :since: 0.7.9"""
        if self._pkg.CurrentVer is not None:
            return Version(self, self._pkg.CurrentVer)

    @property
    def name(self):
        """Return the name of the package."""
        return self._pkg.Name

    @property
    def id(self):
        """Return a uniq ID for the package.

        This can be used eg. to store additional information about the pkg."""
        return self._pkg.ID

    def __hash__(self):
        """Return the hash of the object.

        This returns the same value as ID, which is unique."""
        return self._pkg.ID

    @property
    def essential(self):
        """Return True if the package is an essential part of the system."""
        return self._pkg.Essential

    @DeprecatedProperty
    def installedVersion(self):
        """Return the installed version as string.

        Deprecated, please use installed.version instead."""
        return getattr(self.installed, 'version', None)

    @DeprecatedProperty
    def candidateVersion(self):
        """Return the candidate version as string."""
        return getattr(self.candidate, "version", None)

    @DeprecatedProperty
    def candidateDependencies(self):
        """Return a list of candidate dependencies."""
        return getattr(self.candidate, "dependencies", None)

    @DeprecatedProperty
    def installedDependencies(self):
        """Return a list of installed dependencies."""
        return getattr(self.installed, 'dependencies', [])

    @DeprecatedProperty
    def architecture(self):
        """Return the Architecture of the package"""
        return getattr(self.candidate, "architecture", None)

    @DeprecatedProperty
    def candidateDownloadable(self):
        """Return True if the candidate is downloadable."""
        return getattr(self.candidate, "downloadable", None)

    @DeprecatedProperty
    def installedDownloadable(self):
        """Return True if the installed version is downloadable."""
        return getattr(self.installed, 'downloadable', False)

    @DeprecatedProperty
    def sourcePackageName(self):
        """Return the source package name as string."""
        try:
            return self.candidate._records.SourcePkg or self._pkg.Name
        except AttributeError:
            try:
                return self.installed._records.SourcePkg or self._pkg.Name
            except AttributeError:
                return self._pkg.Name

    @DeprecatedProperty
    def homepage(self):
        """Return the homepage field as string."""
        return getattr(self.candidate, "homepage", None)

    @property
    def section(self):
        """Return the section of the package."""
        return self._pkg.Section

    @DeprecatedProperty
    def priority(self):
        """Return the priority (of the candidate version)."""
        return getattr(self.candidate, "priority", None)

    @DeprecatedProperty
    def installedPriority(self):
        """Return the priority (of the installed version)."""
        return getattr(self.installed, 'priority', None)

    @DeprecatedProperty
    def summary(self):
        """Return the short description (one line summary)."""
        return getattr(self.candidate, "summary", None)

    @DeprecatedProperty
    def description(self):
        """Return the formatted long description.

        Return the formated long description according to the Debian policy
        (Chapter 5.6.13).
        See http://www.debian.org/doc/debian-policy/ch-controlfields.html
        for more information.
        """
        return getattr(self.candidate, "description", None)

    @DeprecatedProperty
    def rawDescription(self):
        """return the long description (raw)."""
        return getattr(self.candidate, "raw_description", None)

    @DeprecatedProperty
    def candidateRecord(self):
        """Return the Record of the candidate version of the package."""
        return getattr(self.candidate, "record", None)

    @DeprecatedProperty
    def installedRecord(self):
        """Return the Record of the candidate version of the package."""
        return getattr(self.installed, 'record', '')

    # depcache states

    @property
    def markedInstall(self):
        """Return True if the package is marked for install."""
        return self._pcache._depcache.MarkedInstall(self._pkg)

    @property
    def markedUpgrade(self):
        """Return True if the package is marked for upgrade."""
        return self._pcache._depcache.MarkedUpgrade(self._pkg)

    @property
    def markedDelete(self):
        """Return True if the package is marked for delete."""
        return self._pcache._depcache.MarkedDelete(self._pkg)

    @property
    def markedKeep(self):
        """Return True if the package is marked for keep."""
        return self._pcache._depcache.MarkedKeep(self._pkg)

    @property
    def markedDowngrade(self):
        """ Package is marked for downgrade """
        return self._pcache._depcache.MarkedDowngrade(self._pkg)

    @property
    def markedReinstall(self):
        """Return True if the package is marked for reinstall."""
        return self._pcache._depcache.MarkedReinstall(self._pkg)

    @property
    def isInstalled(self):
        """Return True if the package is installed."""
        return (self._pkg.CurrentVer is not None)

    @property
    def isUpgradable(self):
        """Return True if the package is upgradable."""
        return (self.isInstalled and
                self._pcache._depcache.IsUpgradable(self._pkg))

    @property
    def isAutoRemovable(self):
        """Return True if the package is no longer required.

        If the package has been installed automatically as a dependency of
        another package, and if no packages depend on it anymore, the package
        is no longer required.
        """
        return self.isInstalled and self._pcache._depcache.IsGarbage(self._pkg)

    # sizes

    @DeprecatedProperty
    def packageSize(self):
        """Return the size of the candidate deb package."""
        return getattr(self.candidate, "size", None)

    @DeprecatedProperty
    def installedPackageSize(self):
        """Return the size of the installed deb package."""
        return getattr(self.installed, 'size', 0)

    @DeprecatedProperty
    def candidateInstalledSize(self):
        """Return the size of the candidate installed package."""
        return getattr(self.candidate, "installed_size", None)

    @DeprecatedProperty
    def installedSize(self):
        """Return the size of the currently installed package."""
        return getattr(self.installed, 'installed_size', 0)

    @property
    def installedFiles(self):
        """Return a list of files installed by the package.

        Return a list of unicode names of the files which have
        been installed by this package
        """
        path = "/var/lib/dpkg/info/%s.list" % self.name
        try:
            file_list = open(path)
            try:
                return file_list.read().decode().split("\n")
            finally:
                file_list.close()
        except EnvironmentError:
            return []

    def getChangelog(self, uri=None, cancel_lock=None):
        """
        Download the changelog of the package and return it as unicode
        string.

        The parameter `uri` refers to the uri of the changelog file. It may
        contain multiple named variables which will be substitued. These
        variables are (src_section, prefix, src_pkg, src_ver). An example is
        the Ubuntu changelog:
            "http://changelogs.ubuntu.com/changelogs/pool" \\
                "/%(src_section)s/%(prefix)s/%(src_pkg)s" \\
                "/%(src_pkg)s_%(src_ver)s/changelog"

        The parameter `cancel_lock` refers to an instance of threading.Lock,
        which if set, prevents the download.
        """
        # Return a cached changelog if available
        if self._changelog != "":
            return self._changelog

        if uri is None:
            if not self.candidate:
                pass
            if self.candidate.origins[0].origin == "Debian":
                uri = "http://packages.debian.org/changelogs/pool" \
                      "/%(src_section)s/%(prefix)s/%(src_pkg)s" \
                      "/%(src_pkg)s_%(src_ver)s/changelog"
            elif self.candidate.origins[0].origin == "Ubuntu":
                uri = "http://changelogs.ubuntu.com/changelogs/pool" \
                      "/%(src_section)s/%(prefix)s/%(src_pkg)s" \
                      "/%(src_pkg)s_%(src_ver)s/changelog"
            else:
                return _("The list of changes is not available")

        # get the src package name
        src_pkg = self.candidate.source_name

        # assume "main" section
        src_section = "main"
        # use the section of the candidate as a starting point
        section = self.candidate.section

        # get the source version, start with the binaries version
        bin_ver = self.candidate.version
        src_ver = self.candidate.version
        #print "bin: %s" % binver
        try:
            # FIXME: This try-statement is too long ...
            # try to get the source version of the pkg, this differs
            # for some (e.g. libnspr4 on ubuntu)
            # this feature only works if the correct deb-src are in the
            # sources.list
            # otherwise we fall back to the binary version number
            src_records = apt_pkg.GetPkgSrcRecords()
            src_rec = src_records.Lookup(src_pkg)
            if src_rec:
                src_ver = src_records.Version
                #if apt_pkg.VersionCompare(binver, srcver) > 0:
                #    srcver = binver
                if not src_ver:
                    src_ver = bin_ver
                #print "srcver: %s" % src_ver
                section = src_records.Section
                #print "srcsect: %s" % section
            else:
                # fail into the error handler
                raise SystemError
        except SystemError:
            src_ver = bin_ver

        l = section.split("/")
        if len(l) > 1:
            src_section = l[0]

        # lib is handled special
        prefix = src_pkg[0]
        if src_pkg.startswith("lib"):
            prefix = "lib" + src_pkg[3]

        # stip epoch
        l = src_ver.split(":")
        if len(l) > 1:
            src_ver = "".join(l[1:])

        uri = uri % {"src_section": src_section,
                     "prefix": prefix,
                     "src_pkg": src_pkg,
                     "src_ver": src_ver}

        timeout = socket.getdefaulttimeout()

        # FIXME: when python2.4 vanishes from the archive,
        #        merge this into a single try..finally block (pep 341)
        try:
            try:
                # Set a timeout for the changelog download
                socket.setdefaulttimeout(2)

                # Check if the download was canceled
                if cancel_lock and cancel_lock.isSet():
                    return ""
                changelog_file = urllib2.urlopen(uri)
                # do only get the lines that are new
                changelog = ""
                regexp = "^%s \((.*)\)(.*)$" % (re.escape(src_pkg))
                while True:
                    # Check if the download was canceled
                    if cancel_lock and cancel_lock.isSet():
                        return ""
                    # Read changelog line by line
                    line_raw = changelog_file.readline()
                    if line_raw == "":
                        break
                    # The changelog is encoded in utf-8, but since there isn't
                    # any http header, urllib2 seems to treat it as ascii
                    line = line_raw.decode("utf-8")

                    #print line.encode('utf-8')
                    match = re.match(regexp, line)
                    if match:
                        # strip epoch from installed version
                        # and from changelog too
                        installed = self.installedVersion
                        if installed and ":" in installed:
                            installed = installed.split(":", 1)[1]
                        changelog_ver = match.group(1)
                        if changelog_ver and ":" in changelog_ver:
                            changelog_ver = changelog_ver.split(":", 1)[1]
                        if (installed and apt_pkg.VersionCompare(changelog_ver,
                                                              installed) <= 0):
                            break
                    # EOF (shouldn't really happen)
                    changelog += line

                # Print an error if we failed to extract a changelog
                if len(changelog) == 0:
                    changelog = _("The list of changes is not available")
                self._changelog = changelog

            except urllib2.HTTPError:
                return _("The list of changes is not available yet.\n\n"
                         "Please use http://launchpad.net/ubuntu/+source/%s/"
                         "%s/+changelog\n"
                         "until the changes become available or try again "
                         "later.") % (src_pkg, src_ver)
            except (IOError, httplib.BadStatusLine):
                return _("Failed to download the list of changes. \nPlease "
                         "check your Internet connection.")
        finally:
            socket.setdefaulttimeout(timeout)
        return self._changelog

    @DeprecatedProperty
    def candidateOrigin(self):
        """Return a list of Origin() objects for the candidate version."""
        return getattr(self.candidate, "origins", None)

    @property
    def versions(self):
        """Return a VersionList() object for all available versions.

        :since: 0.7.9
        """
        return VersionList(self)

    @property
    def is_inst_broken(self):
        """Return True if the to-be-installed package is broken."""
        return self._pcache._depcache.IsInstBroken(self._pkg)

    @property
    def is_now_broken(self):
        """Return True if the installed package is broken."""
        return self._pcache._depcache.IsNowBroken(self._pkg)

    # depcache actions

    def markKeep(self):
        """Mark a package for keep."""
        self._pcache.cachePreChange()
        self._pcache._depcache.MarkKeep(self._pkg)
        self._pcache.cachePostChange()

    def markDelete(self, autoFix=True, purge=False):
        """Mark a package for install.

        If autoFix is True, the resolver will be run, trying to fix broken
        packages. This is the default.

        If purge is True, remove the configuration files of the package as
        well. The default is to keep the configuration.
        """
        self._pcache.cachePreChange()
        self._pcache._depcache.MarkDelete(self._pkg, purge)
        # try to fix broken stuffsta
        if autoFix and self._pcache._depcache.BrokenCount > 0:
            Fix = apt_pkg.GetPkgProblemResolver(self._pcache._depcache)
            Fix.Clear(self._pkg)
            Fix.Protect(self._pkg)
            Fix.Remove(self._pkg)
            Fix.InstallProtect()
            Fix.Resolve()
        self._pcache.cachePostChange()

    def markInstall(self, autoFix=True, autoInst=True, fromUser=True):
        """Mark a package for install.

        If autoFix is True, the resolver will be run, trying to fix broken
        packages. This is the default.

        If autoInst is True, the dependencies of the packages will be installed
        automatically. This is the default.

        If fromUser is True, this package will not be marked as automatically
        installed. This is the default. Set it to False if you want to be able
        to remove the package at a later stage if no other package depends on
        it.
        """
        self._pcache.cachePreChange()
        self._pcache._depcache.MarkInstall(self._pkg, autoInst, fromUser)
        # try to fix broken stuff
        if autoFix and self._pcache._depcache.BrokenCount > 0:
            fixer = apt_pkg.GetPkgProblemResolver(self._pcache._depcache)
            fixer.Clear(self._pkg)
            fixer.Protect(self._pkg)
            fixer.Resolve(True)
        self._pcache.cachePostChange()

    def markUpgrade(self):
        """Mark a package for upgrade."""
        if self.isUpgradable:
            fromUser = not self._pcache._depcache.IsAutoInstalled(self._pkg)
            self.markInstall(fromUser=fromUser)
        else:
            # FIXME: we may want to throw a exception here
            sys.stderr.write(("MarkUpgrade() called on a non-upgrable pkg: "
                              "'%s'\n") % self._pkg.Name)

    def commit(self, fprogress, iprogress):
        """Commit the changes.

        The parameter `fprogress` refers to a FetchProgress() object, as
        found in apt.progress.

        The parameter `iprogress` refers to an InstallProgress() object, as
        found in apt.progress.
        """
        self._pcache._depcache.Commit(fprogress, iprogress)


def _test():
    """Self-test."""
    print "Self-test for the Package modul"
    import random
    import apt
    apt_pkg.init()
    progress = apt.progress.OpTextProgress()
    cache = apt.Cache(progress)
    pkg = cache["apt-utils"]
    print "Name: %s " % pkg.name
    print "ID: %s " % pkg.id
    print "Priority (Candidate): %s " % pkg.candidate.priority
    print "Priority (Installed): %s " % pkg.installed.priority
    print "Installed: %s " % pkg.installed.version
    print "Candidate: %s " % pkg.candidate.version
    print "CandidateDownloadable: %s" % pkg.candidate.downloadable
    print "CandidateOrigins: %s" % pkg.candidate.origins
    print "SourcePkg: %s " % pkg.candidate.source_name
    print "Section: %s " % pkg.section
    print "Summary: %s" % pkg.candidate.summary
    print "Description (formated) :\n%s" % pkg.candidate.description
    print "Description (unformated):\n%s" % pkg.candidate.raw_description
    print "InstalledSize: %s " % pkg.candidate.installed_size
    print "PackageSize: %s " % pkg.candidate.size
    print "Dependencies: %s" % pkg.installed.dependencies
    print "Recommends: %s" % pkg.installed.recommends
    for dep in pkg.candidate.dependencies:
        print ",".join("%s (%s) (%s) (%s)" % (o.name, o.version, o.relation,
                        o.preDepend) for o in dep.or_dependencies)
    print "arch: %s" % pkg.candidate.architecture
    print "homepage: %s" % pkg.candidate.homepage
    print "rec: ", pkg.candidate.record


    print cache["2vcard"].getChangelog()
    for i in True, False:
        print "Running install on random upgradable pkgs with AutoFix: %s " % i
        for pkg in cache:
            if pkg.isUpgradable:
                if random.randint(0, 1) == 1:
                    pkg.markInstall(i)
        print "Broken: %s " % cache._depcache.BrokenCount
        print "InstCount: %s " % cache._depcache.InstCount

    print
    # get a new cache
    for i in True, False:
        print "Randomly remove some packages with AutoFix: %s" % i
        cache = apt.Cache(progress)
        for name in cache.keys():
            if random.randint(0, 1) == 1:
                try:
                    cache[name].markDelete(i)
                except SystemError:
                    print "Error trying to remove: %s " % name
        print "Broken: %s " % cache._depcache.BrokenCount
        print "DelCount: %s " % cache._depcache.DelCount

# self-test
if __name__ == "__main__":
    _test()
