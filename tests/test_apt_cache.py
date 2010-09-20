#!/usr/bin/python
#
# Copyright (C) 2010 Julian Andres Klode <jak@debian.org>
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.
"""Unit tests for verifying the correctness of check_dep, etc in apt_pkg."""
import unittest

import apt


class TestAptCache(unittest.TestCase):
    """ test the apt cache """

    def testAptCache(self):
        """cache: iterate all packages and all dependencies """
        cache = apt.Cache()
        # number is not meaningful and just need to be "big enough",
        # the important bit is the test against __len__
        self.assertTrue(len(cache) > 100)
        # go over the cache and all dependencies, just to see if
        # that is possible and does not crash
        for pkg in cache:
            if pkg.candidate:
                for or_dep in pkg.candidate.dependencies:
                    for dep in or_dep.or_dependencies:
                        self.assertTrue(dep.name)
                        self.assertTrue(isinstance(dep.relation, str))
                        self.assertTrue(dep.pre_depend in (True, False))

if __name__ == "__main__":
    unittest.main()
