#!/usr/bin/python

import apt_pkg

apt_pkg.init()

sources = apt_pkg.SourceList()
sources.ReadMainList()

cache = apt_pkg.Cache()
depcache = apt_pkg.DepCache(cache)
pkg = cache["libimlib2"]
cand = depcache.GetCandidateVer(pkg)
for (f, i) in cand.FileList:
    index = sources.FindIndex(f)
    print index
    if index:
        print index.Size
        print index.IsTrusted
        print index.Exists
        print index.HasPackages
        print index.ArchiveURI("some/path")
