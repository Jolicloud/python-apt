#!/usr/bin/python
# Builds on python2.X and python3
# $Id: setup.py,v 1.2 2002/01/08 07:13:21 jgg Exp $
import glob
import os
import sys

from distutils.core import setup, Extension
cmdclass = {}

try:
    from DistUtilsExtra.command import build_extra, build_i18n
    from DistUtilsExtra.auto import clean_build_tree
    cmdclass['build'] = build_extra.build_extra
    cmdclass['build_i18n'] = build_i18n.build_i18n
    cmdclass['clean'] = clean_build_tree
except ImportError:
    print('W: [python%s] DistUtilsExtra import error.' % sys.version[:3])

try:
    from sphinx.setup_command import BuildDoc
    cmdclass['build_sphinx'] = BuildDoc
except ImportError:
    print('W: [python%s] Sphinx import error.' % sys.version[:3])

if sys.version_info[0] == 3:
    from distutils.command.build_py import build_py_2to3
    cmdclass['build_py'] = build_py_2to3

# The apt_pkg module.
files = ['apt_pkgmodule.cc', 'acquire.cc', 'cache.cc', 'cdrom.cc',
         'configuration.cc', 'depcache.cc', 'generic.cc', 'hashes.cc',
         'hashstring.cc', 'indexfile.cc', 'indexrecords.cc', 'metaindex.cc',
         'pkgmanager.cc', 'pkgrecords.cc', 'pkgsrcrecords.cc', 'policy.cc',
         'progress.cc', 'sourcelist.cc', 'string.cc', 'tag.cc',
         'lock.cc', 'acquire-item.cc', 'python-apt-helpers.cc']
files = sorted(['python/' + fname for fname in files], key=lambda s: s[:-3])
apt_pkg = Extension("apt_pkg", files, libraries=["apt-pkg"])

# The apt_inst module
files = ["python/apt_instmodule.cc", "python/generic.cc", "python/tar.cc",
         "python/arfile.cc", "python/tarfile.cc"]
apt_inst = Extension("apt_inst", files, libraries=["apt-pkg", "apt-inst"])

# Replace the leading _ that is used in the templates for translation
if len(sys.argv) > 1 and sys.argv[1] == "build":
    if not os.path.exists("build/data/templates/"):
        os.makedirs("build/data/templates")
    for template in glob.glob('data/templates/*.info.in'):
        source = open(template, "r")
        build = open("build/" + template[:-3], "w")
        for line in source:
            build.write(line.lstrip("_"))
        source.close()
        build.close()
    for template in glob.glob('data/templates/*.mirrors'):
        import shutil
        shutil.copy(template, os.path.join("build", template))
        

setup(name="python-apt",
      description="Python bindings for APT",
      version=os.environ.get('DEBVER'),
      author="APT Development Team",
      author_email="deity@lists.debian.org",
      ext_modules=[apt_pkg, apt_inst],
      packages=['apt', 'apt.progress', 'aptsources'],
      data_files = [('share/python-apt/templates',
                    glob.glob('build/data/templates/*.info')),
                    ('share/python-apt/templates',
                    glob.glob('data/templates/*.mirrors'))],
      cmdclass = cmdclass,
      license = 'GNU GPL',
      platforms = 'posix')
