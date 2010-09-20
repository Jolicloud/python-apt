#!/usr/bin/python

import apt_pkg

apt_pkg.init()

apt_pkg.Config.Set("APT::Acquire::Translation", "de")

cache = apt_pkg.Cache()
depcache = apt_pkg.DepCache(cache)

pkg = cache["gcc"]
cand = depcache.GetCandidateVer(pkg)
print cand

desc = cand.TranslatedDescription
print desc
print desc.FileList
(f, index) = desc.FileList.pop(0)

records = apt_pkg.PackageRecords(cache)
records.Lookup((f, index))
desc = records.LongDesc
print len(desc)
print desc
