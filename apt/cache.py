# cache.py - apt cache abstraction
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

import os
import weakref

import apt_pkg
from apt import Package
import apt.progress


class FetchCancelledException(IOError):
    """Exception that is thrown when the user cancels a fetch operation."""


class FetchFailedException(IOError):
    """Exception that is thrown when fetching fails."""


class LockFailedException(IOError):
    """Exception that is thrown when locking fails."""


class Cache(object):
    """Dictionary-like package cache.

    This class has all the packages that are available in it's
    dictionary
    """

    def __init__(self, progress=None, rootdir=None, memonly=False):
        self._callbacks = {}
        self._weakref = weakref.WeakValueDictionary()
        self._set = set()
        if memonly:
            # force apt to build its caches in memory
            apt_pkg.Config.Set("Dir::Cache::pkgcache", "")
        if rootdir:
            if os.path.exists(rootdir+"/etc/apt/apt.conf"):
                apt_pkg.ReadConfigFile(apt_pkg.Config,
                                       rootdir + "/etc/apt/apt.conf")
            if os.path.isdir(rootdir+"/etc/apt/apt.conf.d"):
                apt_pkg.ReadConfigDir(apt_pkg.Config,
                                      rootdir + "/etc/apt/apt.conf.d")
            apt_pkg.Config.Set("Dir", rootdir)
            apt_pkg.Config.Set("Dir::State::status",
                               rootdir + "/var/lib/dpkg/status")
            # create required dirs/files when run with special rootdir
            # automatically
            self._check_and_create_required_dirs(rootdir)
            # Call InitSystem so the change to Dir::State::Status is actually
            # recognized (LP: #320665)
            apt_pkg.InitSystem()
        self.open(progress)

    def _check_and_create_required_dirs(self, rootdir):
        """
        check if the required apt directories/files are there and if
        not create them
        """
        files = ["/var/lib/dpkg/status",
                 "/etc/apt/sources.list",
                ]
        dirs = ["/var/lib/dpkg",
                "/etc/apt/",
                "/var/cache/apt/archives/partial",
                "/var/lib/apt/lists/partial",
               ]
        for d in dirs:
            if not os.path.exists(rootdir+d):
                print "creating: ",rootdir+d
                os.makedirs(rootdir+d)
        for f in files:
            if not os.path.exists(rootdir+f):
                open(rootdir+f,"w")

    def _runCallbacks(self, name):
        """ internal helper to run a callback """
        if name in self._callbacks:
            for callback in self._callbacks[name]:
                callback()

    def open(self, progress=None):
        """ Open the package cache, after that it can be used like
            a dictionary
        """
        if progress is None:
            progress = apt.progress.OpProgress()
        self._runCallbacks("cache_pre_open")
        self._cache = apt_pkg.GetCache(progress)
        self._depcache = apt_pkg.GetDepCache(self._cache)
        self._records = apt_pkg.GetPkgRecords(self._cache)
        self._list = apt_pkg.GetPkgSourceList()
        self._list.ReadMainList()
        self._set.clear()
        self._weakref.clear()

        progress.Op = "Building data structures"
        i=last=0
        size=len(self._cache.Packages)
        for pkg in self._cache.Packages:
            if progress is not None and last+100 < i:
                progress.update(i/float(size)*100)
                last=i
            # drop stuff with no versions (cruft)
            if len(pkg.VersionList) > 0:
                self._set.add(pkg.Name)

            i += 1

        progress.done()
        self._runCallbacks("cache_post_open")

    def __getitem__(self, key):
        """ look like a dictionary (get key) """
        try:
            return self._weakref[key]
        except KeyError:
            if key in self._set:
                key = str(key)
                pkg = self._weakref[key] = Package(self, self._cache[key])
                return pkg
            else:
                raise KeyError('The cache has no package named %r' % key)

    def __iter__(self):
        for pkgname in self._set:
            yield self[pkgname]
        raise StopIteration

    def has_key(self, key):
        return (key in self._set)

    def __contains__(self, key):
        return (key in self._set)

    def __len__(self):
        return len(self._set)

    def keys(self):
        return list(self._set)

    def getChanges(self):
        """ Get the marked changes """
        changes = []
        for p in self:
            if p.markedUpgrade or p.markedInstall or p.markedDelete or \
               p.markedDowngrade or p.markedReinstall:
                changes.append(p)
        return changes

    def upgrade(self, distUpgrade=False):
        """ Upgrade the all package, DistUpgrade will also install
            new dependencies
        """
        self.cachePreChange()
        self._depcache.Upgrade(distUpgrade)
        self.cachePostChange()

    @property
    def requiredDownload(self):
        """Get the size of the packages that are required to download."""
        pm = apt_pkg.GetPackageManager(self._depcache)
        fetcher = apt_pkg.GetAcquire()
        pm.GetArchives(fetcher, self._list, self._records)
        return fetcher.FetchNeeded

    @property
    def additionalRequiredSpace(self):
        """Get the size of the additional required space on the fs."""
        return self._depcache.UsrSize

    @property
    def reqReinstallPkgs(self):
        """Return the packages not downloadable packages in reqreinst state."""
        reqreinst = set()
        for pkg in self:
            if (not pkg.candidate.downloadable and
                (pkg._pkg.InstState == apt_pkg.InstStateReInstReq or
                 pkg._pkg.InstState == apt_pkg.InstStateHoldReInstReq)):
                reqreinst.add(pkg.name)
        return reqreinst

    def _runFetcher(self, fetcher):
        # do the actual fetching
        res = fetcher.Run()

        # now check the result (this is the code from apt-get.cc)
        failed = False
        transient = False
        errMsg = ""
        for item in fetcher.Items:
            if item.Status == item.StatDone:
                continue
            if item.StatIdle:
                transient = True
                continue
            errMsg += "Failed to fetch %s %s\n" % (item.DescURI,
                                                   item.ErrorText)
            failed = True

        # we raise a exception if the download failed or it was cancelt
        if res == fetcher.ResultCancelled:
            raise FetchCancelledException(errMsg)
        elif failed:
            raise FetchFailedException(errMsg)
        return res

    def _fetchArchives(self, fetcher, pm):
        """ fetch the needed archives """

        # get lock
        lockfile = apt_pkg.Config.FindDir("Dir::Cache::Archives") + "lock"
        lock = apt_pkg.GetLock(lockfile)
        if lock < 0:
            raise LockFailedException("Failed to lock %s" % lockfile)

        try:
            # this may as well throw a SystemError exception
            if not pm.GetArchives(fetcher, self._list, self._records):
                return False
            # now run the fetcher, throw exception if something fails to be
            # fetched
            return self._runFetcher(fetcher)
        finally:
            os.close(lock)

    def isVirtualPackage(self, pkgname):
        """Return whether the package is a virtual package."""
        pkg = self._cache[pkgname]
        return bool(pkg.ProvidesList and not pkg.VersionList)

    def getProvidingPackages(self, virtual):
        """
        Return a list of packages which provide the virtual package of the
        specified name
        """
        providers = []
        try:
            vp = self._cache[virtual]
            if len(vp.VersionList) != 0:
                return providers
        except KeyError:
            return providers
        for pkg in self:
            v = self._depcache.GetCandidateVer(pkg._pkg)
            if v is None:
                continue
            for p in v.ProvidesList:
                if virtual == p[0]:
                    # we found a pkg that provides this virtual pkg
                    providers.append(pkg)
        return providers

    def update(self, fetchProgress=None, pulseInterval=0):
        " run the equivalent of apt-get update "
        lockfile = apt_pkg.Config.FindDir("Dir::State::Lists") + "lock"
        lock = apt_pkg.GetLock(lockfile)
        if lock < 0:
            raise LockFailedException("Failed to lock %s" % lockfile)

        try:
            if fetchProgress is None:
                fetchProgress = apt.progress.FetchProgress()
            return self._cache.Update(fetchProgress, self._list, pulseInterval)
        finally:
            os.close(lock)

    def installArchives(self, pm, installProgress):
        installProgress.startUpdate()
        res = installProgress.run(pm)
        installProgress.finishUpdate()
        return res

    def commit(self, fetchProgress=None, installProgress=None):
        """ Apply the marked changes to the cache """
        # FIXME:
        # use the new acquire/pkgmanager interface here,
        # raise exceptions when a download or install fails
        # and send proper error strings to the application.
        # Current a failed download will just display "error"
        # which is less than optimal!

        if fetchProgress is None:
            fetchProgress = apt.progress.FetchProgress()
        if installProgress is None:
            installProgress = apt.progress.InstallProgress()

        pm = apt_pkg.GetPackageManager(self._depcache)
        fetcher = apt_pkg.GetAcquire(fetchProgress)
        while True:
            # fetch archives first
            res = self._fetchArchives(fetcher, pm)

            # then install
            res = self.installArchives(pm, installProgress)
            if res == pm.ResultCompleted:
                break
            elif res == pm.ResultFailed:
                raise SystemError("installArchives() failed")
            elif res == pm.ResultIncomplete:
                 pass
            else:
                 raise SystemError("internal-error: unknown result code from InstallArchives: %s" % res)
            # reload the fetcher for media swaping
            fetcher.Shutdown()
        return (res == pm.ResultCompleted)

    def clear(self):
        """ Unmark all changes """
        self._depcache.Init()

    # cache changes

    def cachePostChange(self):
        " called internally if the cache has changed, emit a signal then "
        self._runCallbacks("cache_post_change")

    def cachePreChange(self):
        """ called internally if the cache is about to change, emit
            a signal then """
        self._runCallbacks("cache_pre_change")

    def actiongroup(self):
        """Return an ActionGroup() object for the current cache.

        Action groups can be used to speedup actions. The action group is
        active as soon as it is created, and disabled when the object is
        deleted or when release() is called.
        """
        return apt_pkg.GetPkgActionGroup(self._depcache)

    def connect(self, name, callback):
        """ connect to a signal, currently only used for
            cache_{post,pre}_{changed,open} """
        if not name in self._callbacks:
            self._callbacks[name] = []
        self._callbacks[name].append(callback)

    @property
    def broken_count(self):
        """Return the number of packages with broken dependencies."""
        return self._depcache.BrokenCount

    @property
    def delete_count(self):
        """Return the number of packages marked for deletion."""
        return self._depcache.DelCount

    @property
    def install_count(self):
        """Return the number of packages marked for installation."""
        return self._depcache.InstCount

    @property
    def keep_count(self):
        """Return the number of packages marked as keep."""
        return self._depcache.KeepCount


class ProblemResolver(object):
    """Resolve problems due to dependencies and conflicts.

    The first argument 'cache' is an instance of apt.Cache.
    """

    def __init__(self, cache):
        self._resolver = apt_pkg.GetPkgProblemResolver(cache._depcache)

    def clear(self, package):
        """Reset the package to the default state."""
        self._resolver.Clear(package._pkg)

    def install_protect(self):
        """mark protected packages for install or removal."""
        self._resolver.InstallProtect()

    def protect(self, package):
        """Protect a package so it won't be removed."""
        self._resolver.Protect(package._pkg)

    def remove(self, package):
        """Mark a package for removal."""
        self._resolver.Remove(package._pkg)

    def resolve(self):
        """Resolve dependencies, try to remove packages where needed."""
        self._resolver.Resolve()

    def resolve_by_keep(self):
        """Resolve dependencies, do not try to remove packages."""
        self._resolver.ResolveByKeep()


# ----------------------------- experimental interface


class Filter(object):
    """ Filter base class """

    def apply(self, pkg):
        """ Filter function, return True if the package matchs a
            filter criteria and False otherwise
        """
        return True


class MarkedChangesFilter(Filter):
    """ Filter that returns all marked changes """

    def apply(self, pkg):
        if pkg.markedInstall or pkg.markedDelete or pkg.markedUpgrade:
            return True
        else:
            return False


class FilteredCache(object):
    """ A package cache that is filtered.

        Can work on a existing cache or create a new one
    """

    def __init__(self, cache=None, progress=None):
        if cache is None:
            self.cache = Cache(progress)
        else:
            self.cache = cache
        self.cache.connect("cache_post_change", self.filterCachePostChange)
        self.cache.connect("cache_post_open", self.filterCachePostChange)
        self._filtered = {}
        self._filters = []

    def __len__(self):
        return len(self._filtered)

    def __getitem__(self, key):
        return self.cache[key]

    def __iter__(self):
        for pkgname in self._filtered:
            yield self.cache[pkgname]

    def keys(self):
        return self._filtered.keys()

    def has_key(self, key):
        return (key in self._filtered)

    def __contains__(self, key):
        return (key in self._filtered)

    def _reapplyFilter(self):
        " internal helper to refilter "
        self._filtered = {}
        for pkg in self.cache:
            for f in self._filters:
                if f.apply(pkg):
                    self._filtered[pkg.name] = 1
                    break

    def setFilter(self, filter):
        """Set the current active filter."""
        self._filters = []
        self._filters.append(filter)
        #self._reapplyFilter()
        # force a cache-change event that will result in a refiltering
        self.cache.cachePostChange()

    def filterCachePostChange(self):
        """Called internally if the cache changes, emit a signal then."""
        #print "filterCachePostChange()"
        self._reapplyFilter()

#    def connect(self, name, callback):
#        self.cache.connect(name, callback)

    def __getattr__(self, key):
        """we try to look exactly like a real cache."""
        #print "getattr: %s " % key
        return getattr(self.cache, key)


def cache_pre_changed():
    print "cache pre changed"


def cache_post_changed():
    print "cache post changed"


# internal test code
if __name__ == "__main__":
    print "Cache self test"
    apt_pkg.init()
    c = Cache(apt.progress.OpTextProgress())
    c.connect("cache_pre_change", cache_pre_changed)
    c.connect("cache_post_change", cache_post_changed)
    print ("aptitude" in c)
    p = c["aptitude"]
    print p.name
    print len(c)

    for pkg in c.keys():
        x= c[pkg].name

    c.upgrade()
    changes = c.getChanges()
    print len(changes)
    for p in changes:
        #print p.name
        x = p.name


    # see if fetching works
    for d in ["/tmp/pytest", "/tmp/pytest/partial"]:
        if not os.path.exists(d):
            os.mkdir(d)
    apt_pkg.Config.Set("Dir::Cache::Archives", "/tmp/pytest")
    pm = apt_pkg.GetPackageManager(c._depcache)
    fetcher = apt_pkg.GetAcquire(apt.progress.TextFetchProgress())
    c._fetchArchives(fetcher, pm)
    #sys.exit(1)

    print "Testing filtered cache (argument is old cache)"
    f = FilteredCache(c)
    f.cache.connect("cache_pre_change", cache_pre_changed)
    f.cache.connect("cache_post_change", cache_post_changed)
    f.cache.upgrade()
    f.setFilter(MarkedChangesFilter())
    print len(f)
    for pkg in f.keys():
        #print c[pkg].name
        x = f[pkg].name

    print len(f)

    print "Testing filtered cache (no argument)"
    f = FilteredCache(progress=apt.progress.OpTextProgress())
    f.cache.connect("cache_pre_change", cache_pre_changed)
    f.cache.connect("cache_post_change", cache_post_changed)
    f.cache.upgrade()
    f.setFilter(MarkedChangesFilter())
    print len(f)
    for pkg in f.keys():
        #print c[pkg].name
        x = f[pkg].name

    print len(f)
