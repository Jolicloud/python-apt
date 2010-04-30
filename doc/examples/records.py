#!/usr/bin/env python

import apt

cache = apt.Cache()

for pkg in cache:
    if not pkg.candidate.record:
        continue
    if "Task" in pkg.candidate.record:
        print "Pkg %s is part of '%s'" % (
            pkg.name, pkg.candidate.record["Task"].split())
        #print pkg.candidateRecord
