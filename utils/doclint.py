#!/usr/bin/python
# Documentation lint.
# Copyright (C) 2009 Julian Andres Klode <jak@debian.org>
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.
#
# This comes without any warranty.
"""Read the pickle file created by sphinx and check it."""

from __future__ import with_statement
import cPickle
import os
import sys


def handle(filename):
    with open(filename) as fobj:
        index = cPickle.load(fobj)

        objects = index['descrefs']
        modules = index['modules']
        types = index['desctypes']

    for modname in modules:
        module = __import__(modname, fromlist=["*"])

        for modmember in objects[modname]:
            if not modmember in module.__dict__:
                print 'W: Unknown', modname + '.' + modmember
            elif types[objects[modname][modmember][1]] == u'class':
                if modname + '.' + modmember not in objects:
                    print 'I: No members', modname + '.' + modmember
                    continue
                for member in objects.get(modname + '.' + modmember):
                    if not member in dir(module.__dict__[modmember]):
                        print 'W: Unknown', modname + '.' + modmember + '.' + member

                    assert(types[objects[modname+"."+modmember][member][1]] in ('method', 'attribute'))

        all = getattr(module, '__all__', [])
        for modmember in dir(module):
            if getattr(module.__dict__[modmember], "__module__", modname) != modname:
                continue
            if isinstance(module.__dict__[modmember], type(module)):
                continue
            if modmember.startswith("_"):
                continue
            if not modmember in objects[modname] and (not all or modmember in all):
                print 'E: Missing', modname + '.' + modmember
            elif not modmember in objects[modname]:
                print 'W: Missing', modname + '.' + modmember
            elif types[objects[modname][modmember][1]] == u'class':
                for member in dir(module.__dict__[modmember]):
                    if member.startswith("_"):
                        continue
                    try:
                        contin = False
                        for base in module.__dict__[modmember].__bases__:
                            if member in dir(base):
                                contin = True
                        if contin:
                            continue
                    except:
                        pass
                    if not member in objects.get(modname + '.' + modmember, ""):
                        print 'E: Missing', modname + '.' + modmember + '.' + member


if __name__ == '__main__':
    scriptdir = os.path.dirname(__file__)
    parentdir = os.path.join(scriptdir, "..")
    directory = os.path.join(parentdir,  "doc", "build", "pickle")
    directory = os.path.normpath(directory)
    sys.path.insert(0, os.path.abspath(parentdir))
    handle(os.path.join(directory, "searchindex.pickle"))
