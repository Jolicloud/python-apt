#!/usr/bin/python
#  get_debian_mirrors.py - Parse Mirrors.masterlist and create a mirror list.
#
#  Copyright (c) 2010 Julian Andres Klode <jak@debian.org>
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
import collections
import urllib2
from debian_bundle import deb822

mirrors = collections.defaultdict(set)
masterlist = urllib2.urlopen("http://cvs.debian.org/webwml/webwml/english/"
                             "mirror/Mirrors.masterlist?revision=HEAD")

for mirror in deb822.Deb822.iter_paragraphs(masterlist):
    country = mirror["Country"].split(None, 1)[0]
    site = mirror["Site"]
    for proto in 'http', 'ftp':
        if "Archive-%s" % proto in mirror:
            mirrors[country].add("%s://%s%s" % (proto, site,
                                                mirror["Archive-%s" % proto]))

for country in sorted(mirrors):
    print "#LOC:%s" % country
    print "\n".join(sorted(mirrors[country]))
