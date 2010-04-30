#! /usr/bin/env python
# $Id: setup.py,v 1.2 2002/01/08 07:13:21 jgg Exp $
import glob
import os
import shutil
import sys

from distutils.core import setup, Extension
from distutils.sysconfig import parse_makefile
from DistUtilsExtra.command import build_extra, build_i18n


# The apt_pkg module
files = map(lambda source: "python/"+source,
            parse_makefile("python/makefile")["APT_PKG_SRC"].split())
apt_pkg = Extension("apt_pkg", files, libraries=["apt-pkg"])

# The apt_inst module
files = map(lambda source: "python/"+source,
            parse_makefile("python/makefile")["APT_INST_SRC"].split())
apt_inst = Extension("apt_inst", files, libraries=["apt-pkg", "apt-inst"])

# Replace the leading _ that is used in the templates for translation
templates = []

# build doc
if len(sys.argv) > 1 and sys.argv[1] == "build":
    if not os.path.exists("build/data/templates/"):
        os.makedirs("build/data/templates")
    for template in glob.glob('data/templates/*.info.in'):
        source = open(template, "r")
        build = open(os.path.join("build", template[:-3]), "w")
        lines = source.readlines()
        for line in lines:
            build.write(line.lstrip("_"))
        source.close()
        build.close()


if len(sys.argv) > 1 and sys.argv[1] == "clean" and '-a' in sys.argv:
    for dirname in "build/doc", "doc/build", "build/data", "build/mo":
        if os.path.exists(dirname):
            print "Removing", dirname
            shutil.rmtree(dirname)
        else:
            print "Not removing", dirname, "because it does not exist"

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
      cmdclass = {"build": build_extra.build_extra,
                  "build_i18n": build_i18n.build_i18n},
      license = 'GNU GPL',
      platforms = 'posix')

if (len(sys.argv) > 1 and sys.argv[1] == "build" and
    sys.version_info[0] >= 2 and sys.version_info[1] >= 5):
    import sphinx
    sphinx.main(["sphinx", "-b", "html", "-d", "build/doc/doctrees",
                os.path.abspath("doc/source"), "build/doc/html"])
    sphinx.main(["sphinx", "-b", "text", "-d", "build/doc/doctrees",
                os.path.abspath("doc/source"), "build/doc/text"])
