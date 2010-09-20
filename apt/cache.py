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
from apt_pkg import gettext as _
from apt.deprecation import (AttributeDeprecatedBy, function_deprecated_by,
                             deprecated_args)
import apt.progress.text


class FetchCancelledException(IOError):
    """Exception that is thrown when the user cancels a fetch operation."""


class FetchFailedException(IOError):
    """Exception that is thrown when fetching fails."""


class LockFailedException(IOError):
    """Exception that is thrown when locking fails."""


class Cache(object):
    """Dictionary-like package cache.

    This class has all the packages that are available in it's
    dictionary.

    Keyword arguments:
    progress -- a OpProgress object
    rootdir -- a alternative root directory. if that is given
               the system sources.list and system lists/ files are
               not read, only files relative to the given rootdir
    memonly -- build the cache in memory only
    """

    def __init__(self, progress=None, rootdir=None, memonly=False):
        self._cache = None
        self._depcache = None
        self._records = None
        self._list = None
        self._callbacks = {}
        self._weakref = weakref.WeakValueDictionary()
        self._set = set()
        if memonly:
            # force apt to build its caches in memory
            apt_pkg.config.set("Dir::Cache::pkgcache", "")
        if rootdir:
            if os.path.exists(rootdir+"/etc/apt/apt.conf"):
                apt_pkg.read_config_file(apt_pkg.config,
                                       rootdir + "/etc/apt/apt.conf")
            if os.path.isdir(rootdir+"/etc/apt/apt.conf.d"):
                apt_pkg.read_config_dir(apt_pkg.config,
                                      rootdir + "/etc/apt/apt.conf.d")
            apt_pkg.config.set("Dir", rootdir)
            apt_pkg.config.set("Dir::State::status",
                               rootdir + "/var/lib/dpkg/status")
            # create required dirs/files when run with special rootdir
            # automatically
            self._check_and_create_required_dirs(rootdir)
            # Call InitSystem so the change to Dir::State::Status is actually
            # recognized (LP: #320665)
            apt_pkg.init_system()
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
            if not os.path.exists(rootdir + d):
                print "creating: ", rootdir + d
                os.makedirs(rootdir + d)
        for f in files:
            if not os.path.exists(rootdir + f):
                open(rootdir + f, "w")

    def _run_callbacks(self, name):
        """ internal helper to run a callback """
        if name in self._callbacks:
            for callback in self._callbacks[name]:
                callback()

    def open(self, progress=None):
        """ Open the package cache, after that it can be used like
            a dictionary
        """
        if progress is None:
            progress = apt.progress.base.OpProgress()
        self._run_callbacks("cache_pre_open")

        self._cache = apt_pkg.Cache(progress)
        self._depcache = apt_pkg.DepCache(self._cache)
        self._records = apt_pkg.PackageRecords(self._cache)
        self._list = apt_pkg.SourceList()
        self._list.read_main_list()
        self._set.clear()
        self._weakref.clear()

        progress.op = _("Building data structures")
        i = last = 0
        size = len(self._cache.packages)
        for pkg in self._cache.packages:
            if progress is not None and last+100 < i:
                progress.update(i/float(size)*100)
                last = i
            # drop stuff with no versions (cruft)
            if len(pkg.version_list) > 0:
                self._set.add(pkg.name)

            i += 1

        progress.done()
        self._run_callbacks("cache_post_open")

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

    def get_changes(self):
        """ Get the marked changes """
        changes = []
        for pkg in self:
            if (pkg.marked_upgrade or pkg.marked_install or pkg.marked_delete
                or pkg.marked_downgrade or pkg.marked_reinstall):
                changes.append(pkg)
        return changes

    @deprecated_args
    def upgrade(self, dist_upgrade=False):
        """Upgrade all packages.

        If the parameter *dist_upgrade* is True, new dependencies will be
        installed as well (and conflicting packages may be removed). The
        default value is False.
        """
        self.cache_pre_change()
        self._depcache.upgrade(dist_upgrade)
        self.cache_post_change()

    @property
    def required_download(self):
        """Get the size of the packages that are required to download."""
        pm = apt_pkg.PackageManager(self._depcache)
        fetcher = apt_pkg.Acquire()
        pm.get_archives(fetcher, self._list, self._records)
        return fetcher.fetch_needed

    @property
    def required_space(self):
        """Get the size of the additional required space on the fs."""
        return self._depcache.usr_size

    @property
    def req_reinstall_pkgs(self):
        """Return the packages not downloadable packages in reqreinst state."""
        reqreinst = set()
        for pkg in self:
            if (not pkg.candidate.downloadable and
                (pkg._pkg.inst_state == apt_pkg.INSTSTATE_REINSTREQ or
                 pkg._pkg.inst_state == apt_pkg.INSTSTATE_HOLD_REINSTREQ)):
                reqreinst.add(pkg.name)
        return reqreinst

    def _run_fetcher(self, fetcher):
        # do the actual fetching
        res = fetcher.run()

        # now check the result (this is the code from apt-get.cc)
        failed = False
        transient = False
        err_msg = ""
        for item in fetcher.items:
            if item.status == item.STAT_DONE:
                continue
            if item.STAT_IDLE:
                transient = True
                continue
            err_msg += "Failed to fetch %s %s\n" % (item.desc_uri,
                                                   item.error_text)
            failed = True

        # we raise a exception if the download failed or it was cancelt
        if res == fetcher.RESULT_CANCELLED:
            raise FetchCancelledException(err_msg)
        elif failed:
            raise FetchFailedException(err_msg)
        return res

    def _fetch_archives(self, fetcher, pm):
        """ fetch the needed archives """

        # get lock
        lockfile = apt_pkg.config.find_dir("Dir::Cache::Archives") + "lock"
        lock = apt_pkg.get_lock(lockfile)
        if lock < 0:
            raise LockFailedException("Failed to lock %s" % lockfile)

        try:
            # this may as well throw a SystemError exception
            if not pm.get_archives(fetcher, self._list, self._records):
                return False
            # now run the fetcher, throw exception if something fails to be
            # fetched
            return self._run_fetcher(fetcher)
        finally:
            os.close(lock)

    def is_virtual_package(self, pkgname):
        """Return whether the package is a virtual package."""
        try:
            pkg = self._cache[pkgname]
        except KeyError:
            return False
        else:
            return bool(pkg.provides_list and not pkg.version_list)

    def get_providing_packages(self, virtual):
        """
        Return a list of packages which provide the virtual package of the
        specified name
        """
        providers = []
        try:
            vp = self._cache[virtual]
            if len(vp.version_list) != 0:
                return providers
        except KeyError:
            return providers
        for pkg in self:
            v = self._depcache.get_candidate_ver(pkg._pkg)
            if v is None:
                continue
            for p in v.provides_list:
                if virtual == p[0]:
                    # we found a pkg that provides this virtual pkg
                    providers.append(pkg)
        return providers

    @deprecated_args
    def update(self, fetch_progress=None, pulse_interval=0,
               raise_on_error=True):
        """Run the equivalent of apt-get update.

        The first parameter *fetch_progress* may be set to an instance of
        apt.progress.FetchProgress, the default is apt.progress.FetchProgress()
        .
        """
        lockfile = apt_pkg.config.find_dir("Dir::State::Lists") + "lock"
        lock = apt_pkg.get_lock(lockfile)

        if lock < 0:
            raise LockFailedException("Failed to lock %s" % lockfile)

        try:
            if fetch_progress is None:
                fetch_progress = apt.progress.base.AcquireProgress()
            try:
                res = self._cache.update(fetch_progress, self._list,
                                         pulse_interval)
            except SystemError, e:
                raise FetchFailedException(e)
            if not res and raise_on_error:
                raise FetchFailedException()
            else:
                return res
        finally:
            os.close(lock)

    @deprecated_args
    def install_archives(self, pm, install_progress):
        """
        The first parameter *pm* refers to an object returned by
        apt_pkg.PackageManager().

        The second parameter *install_progress* refers to an InstallProgress()
        object of the module apt.progress.
        """
        # compat with older API
        try:
            install_progress.startUpdate()
        except AttributeError:
            install_progress.start_update()
        res = install_progress.run(pm)
        try:
            install_progress.finishUpdate()
        except AttributeError:
            install_progress.finish_update()
        return res

    @deprecated_args
    def commit(self, fetch_progress=None, install_progress=None):
        """Apply the marked changes to the cache.

        The first parameter, *fetch_progress*, refers to a FetchProgress()
        object as found in apt.progress, the default being
        apt.progress.FetchProgress().

        The second parameter, *install_progress*, is a
        apt.progress.InstallProgress() object.
        """
        # FIXME:
        # use the new acquire/pkgmanager interface here,
        # raise exceptions when a download or install fails
        # and send proper error strings to the application.
        # Current a failed download will just display "error"
        # which is less than optimal!

        if fetch_progress is None:
            fetch_progress = apt.progress.base.AcquireProgress()
        if install_progress is None:
            install_progress = apt.progress.base.InstallProgress()

        pm = apt_pkg.PackageManager(self._depcache)
        fetcher = apt_pkg.Acquire(fetch_progress)
        while True:
            # fetch archives first
            res = self._fetch_archives(fetcher, pm)

            # then install
            res = self.install_archives(pm, install_progress)
            if res == pm.RESULT_COMPLETED:
                break
            elif res == pm.RESULT_FAILED:
                raise SystemError("installArchives() failed")
            elif res == pm.RESULT_INCOMPLETE:
                pass
            else:
                raise SystemError("internal-error: unknown result code "
                                  "from InstallArchives: %s" % res)
            # reload the fetcher for media swaping
            fetcher.shutdown()
        return (res == pm.RESULT_COMPLETED)

    def clear(self):
        """ Unmark all changes """
        self._depcache.init()

    # cache changes

    def cache_post_change(self):
        " called internally if the cache has changed, emit a signal then "
        self._run_callbacks("cache_post_change")

    def cache_pre_change(self):
        """ called internally if the cache is about to change, emit
            a signal then """
        self._run_callbacks("cache_pre_change")

    def connect(self, name, callback):
        """ connect to a signal, currently only used for
            cache_{post,pre}_{changed,open} """
        if not name in self._callbacks:
            self._callbacks[name] = []
        self._callbacks[name].append(callback)

    def actiongroup(self):
        """Return an ActionGroup() object for the current cache.

        Action groups can be used to speedup actions. The action group is
        active as soon as it is created, and disabled when the object is
        deleted or when release() is called.

        You can use the action group as a context manager, this is the
        recommended way::

            with cache.actiongroup():
                for package in my_selected_packages:
                    package.mark_install()

        This way, the ActionGroup is automatically released as soon as the
        with statement block is left. It also has the benefit of making it
        clear which parts of the code run with a action group and which
        don't.
        """
        return apt_pkg.ActionGroup(self._depcache)

    @property
    def broken_count(self):
        """Return the number of packages with broken dependencies."""
        return self._depcache.broken_count

    @property
    def delete_count(self):
        """Return the number of packages marked for deletion."""
        return self._depcache.del_count

    @property
    def install_count(self):
        """Return the number of packages marked for installation."""
        return self._depcache.inst_count

    @property
    def keep_count(self):
        """Return the number of packages marked as keep."""
        return self._depcache.keep_count

    if apt_pkg._COMPAT_0_7:
        _runCallbacks = function_deprecated_by(_run_callbacks)
        getChanges = function_deprecated_by(get_changes)
        requiredDownload = AttributeDeprecatedBy('required_download')
        additionalRequiredSpace = AttributeDeprecatedBy('required_space')
        reqReinstallPkgs = AttributeDeprecatedBy('req_reinstall_pkgs')
        _runFetcher = function_deprecated_by(_run_fetcher)
        _fetchArchives = function_deprecated_by(_fetch_archives)
        isVirtualPackage = function_deprecated_by(is_virtual_package)
        getProvidingPackages = function_deprecated_by(get_providing_packages)
        installArchives = function_deprecated_by(install_archives)
        cachePostChange = function_deprecated_by(cache_post_change)
        cachePreChange = function_deprecated_by(cache_pre_change)


class ProblemResolver(object):
    """Resolve problems due to dependencies and conflicts.

    The first argument 'cache' is an instance of apt.Cache.
    """

    def __init__(self, cache):
        self._resolver = apt_pkg.ProblemResolver(cache._depcache)

    def clear(self, package):
        """Reset the package to the default state."""
        self._resolver.clear(package._pkg)

    def install_protect(self):
        """mark protected packages for install or removal."""
        self._resolver.install_protect()

    def protect(self, package):
        """Protect a package so it won't be removed."""
        self._resolver.protect(package._pkg)

    def remove(self, package):
        """Mark a package for removal."""
        self._resolver.remove(package._pkg)

    def resolve(self):
        """Resolve dependencies, try to remove packages where needed."""
        self._resolver.resolve()

    def resolve_by_keep(self):
        """Resolve dependencies, do not try to remove packages."""
        self._resolver.resolve_by_keep()


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
        if pkg.marked_install or pkg.marked_delete or pkg.marked_upgrade:
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
        self.cache.connect("cache_post_change", self.filter_cache_post_change)
        self.cache.connect("cache_post_open", self.filter_cache_post_change)
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

    def _reapply_filter(self):
        " internal helper to refilter "
        self._filtered = {}
        for pkg in self.cache:
            for f in self._filters:
                if f.apply(pkg):
                    self._filtered[pkg.name] = 1
                    break

    def set_filter(self, filter):
        """Set the current active filter."""
        self._filters = []
        self._filters.append(filter)
        #self._reapplyFilter()
        # force a cache-change event that will result in a refiltering
        self.cache.cache_post_change()

    def filter_cache_post_change(self):
        """Called internally if the cache changes, emit a signal then."""
        #print "filterCachePostChange()"
        self._reapply_filter()


#    def connect(self, name, callback):
#        self.cache.connect(name, callback)

    def __getattr__(self, key):
        """we try to look exactly like a real cache."""
        #print "getattr: %s " % key
        return getattr(self.cache, key)

    if apt_pkg._COMPAT_0_7:
        _reapplyFilter = function_deprecated_by(_reapply_filter)
        setFilter = function_deprecated_by(set_filter)
        filterCachePostChange = function_deprecated_by(\
                                                    filter_cache_post_change)


def cache_pre_changed():
    print "cache pre changed"


def cache_post_changed():
    print "cache post changed"


def _test():
    """Internal test code."""
    print "Cache self test"
    apt_pkg.init()
    cache = Cache(apt.progress.text.OpProgress())
    cache.connect("cache_pre_change", cache_pre_changed)
    cache.connect("cache_post_change", cache_post_changed)
    print ("aptitude" in cache)
    pkg = cache["aptitude"]
    print pkg.name
    print len(cache)

    for pkgname in cache.keys():
        assert cache[pkgname].name == pkgname

    cache.upgrade()
    changes = cache.get_changes()
    print len(changes)
    for pkg in changes:
        assert pkg.name



    # see if fetching works
    for dirname in ["/tmp/pytest", "/tmp/pytest/partial"]:
        if not os.path.exists(dirname):
            os.mkdir(dirname)
    apt_pkg.config.set("Dir::Cache::Archives", "/tmp/pytest")
    pm = apt_pkg.PackageManager(cache._depcache)
    fetcher = apt_pkg.Acquire(apt.progress.text.AcquireProgress())
    cache._fetch_archives(fetcher, pm)
    #sys.exit(1)

    print "Testing filtered cache (argument is old cache)"
    filtered = FilteredCache(cache)
    filtered.cache.connect("cache_pre_change", cache_pre_changed)
    filtered.cache.connect("cache_post_change", cache_post_changed)
    filtered.cache.upgrade()
    filtered.set_filter(MarkedChangesFilter())
    print len(filtered)
    for pkgname in filtered.keys():
        assert pkgname == filtered[pkg].name

    print len(filtered)

    print "Testing filtered cache (no argument)"
    filtered = FilteredCache(progress=apt.progress.base.OpProgress())
    filtered.cache.connect("cache_pre_change", cache_pre_changed)
    filtered.cache.connect("cache_post_change", cache_post_changed)
    filtered.cache.upgrade()
    filtered.set_filter(MarkedChangesFilter())
    print len(filtered)
    for pkgname in filtered.keys():
        assert pkgname == filtered[pkgname].name

    print len(filtered)
if __name__ == '__main__':
    _test()
