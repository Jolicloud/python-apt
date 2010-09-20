#!/usr/bin/python
# example how to deal with the depcache

import apt_pkg
import sys

from progress import TextCdromProgress


# init
apt_pkg.init()

cdrom = apt_pkg.Cdrom()
print cdrom

progress = TextCdromProgress()

(res, ident) = cdrom.Ident(progress)
print "ident result is: %s (%s) " % (res, ident)

apt_pkg.Config.Set("APT::CDROM::Rename", "True")
cdrom.Add(progress)

print "Exiting"
sys.exit(0)
