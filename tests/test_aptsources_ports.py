#!/usr/bin/env python
import os
import unittest


import apt_pkg
import aptsources.sourceslist
import aptsources.distro


class TestAptSourcesPorts(unittest.TestCase):
    """Test aptsources on ports.ubuntu.com."""

    def setUp(self):
        apt_pkg.init_config()
        apt_pkg.init_system()
        apt_pkg.config.set("APT::Architecture", "powerpc")
        apt_pkg.config.set("Dir::Etc",
                           os.path.abspath("data/aptsources_ports"))
        apt_pkg.config.set("Dir::Etc::sourceparts", "/xxx")
        if os.path.exists("../build/data/templates"):
            self.templates = os.path.abspath("../build/data/templates")
        else:
            self.templates = "/usr/share/python-apt/templates/"

    def testMatcher(self):
        """aptsources_ports: Test matcher."""
        apt_pkg.config.set("Dir::Etc::sourcelist", "sources.list")
        sources = aptsources.sourceslist.SourcesList(True, self.templates)
        distro = aptsources.distro.get_distro("Ubuntu", "hardy", "desc",
                                              "8.04")
        distro.get_sources(sources)
        # test if all suits of the current distro were detected correctly
        dist_templates = set()
        for s in sources:
            if not s.line.strip() or s.line.startswith("#"):
                continue
            if not s.template:
                self.fail("source entry '%s' has no matcher" % s)


if __name__ == "__main__":
    unittest.main()
