#!/usr/bin/python
#
# Copyright (C) 2010 Julian Andres Klode <jak@debian.org>
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.
"""Unit tests for verifying the correctness of check_dep, etc in apt_pkg."""
import unittest

import apt_pkg


class TestDependencies(unittest.TestCase):

    def setUp(self):
        apt_pkg.init()

    def testCheckDep(self):
        """dependencies: Test apt_pkg.CheckDep() for '<' and '>'

        The CheckDep function should treat '<' as '<=' and '>' as '>=', for
        compatibility reasons."""
        if not hasattr(apt_pkg, 'CheckDep'):
            return
        self.assertFalse(apt_pkg.CheckDep("1", "<", "0"))
        self.assertTrue(apt_pkg.CheckDep("1", "<", "1"))
        self.assertTrue(apt_pkg.CheckDep("1", "<", "2"))

        self.assertFalse(apt_pkg.CheckDep("0", ">", "1"))
        self.assertTrue(apt_pkg.CheckDep("1", ">", "1"))
        self.assertTrue(apt_pkg.CheckDep("2", ">", "1"))

    def test_check_dep(self):
        "dependencies: Test apt_pkg.check_dep()"
        self.assertFalse(apt_pkg.check_dep("1", "<<", "0"))
        self.assertFalse(apt_pkg.check_dep("1", "<<", "1"))
        self.assertTrue(apt_pkg.check_dep("1", "<<", "2"))

        self.assertFalse(apt_pkg.check_dep("1", "<", "0"))
        self.assertFalse(apt_pkg.check_dep("1", "<", "1"))
        self.assertTrue(apt_pkg.check_dep("1", "<", "2"))

        self.assertFalse(apt_pkg.check_dep("1", "<=", "0"))
        self.assertTrue(apt_pkg.check_dep("1", "<=", "1"))
        self.assertTrue(apt_pkg.check_dep("1", "<=", "2"))

        self.assertFalse(apt_pkg.check_dep("0", "=", "1"))
        self.assertTrue(apt_pkg.check_dep("1", "=", "1"))
        self.assertFalse(apt_pkg.check_dep("2", "=", "1"))

        self.assertFalse(apt_pkg.check_dep("0", ">=", "1"))
        self.assertTrue(apt_pkg.check_dep("1", ">=", "1"))
        self.assertTrue(apt_pkg.check_dep("2", ">=", "1"))

        self.assertFalse(apt_pkg.check_dep("0", ">", "1"))
        self.assertFalse(apt_pkg.check_dep("1", ">", "1"))
        self.assertTrue(apt_pkg.check_dep("2", ">", "1"))

        self.assertFalse(apt_pkg.check_dep("0", ">>", "1"))
        self.assertFalse(apt_pkg.check_dep("1", ">>", "1"))
        self.assertTrue(apt_pkg.check_dep("2", ">>", "1"))

    def test_parse_depends(self):
        """dependencies: Test apt_pkg.parse_depends()"""
        deps = apt_pkg.parse_depends("p1a (<< 1a) | p1b (>> 1b)")
        self.assertTrue(isinstance(deps, list))
        self.assertEqual(len(deps), 1)
        self.assertTrue(isinstance(deps[0], list))
        self.assertEqual(len(deps[0]), 2)
        self.assertEqual(len(deps[0][0]), 3)
        self.assertEqual(len(deps[0][1]), 3)
        self.assertEqual(deps[0][0][0], "p1a")
        self.assertEqual(deps[0][0][1], "1a")
        self.assertEqual(deps[0][0][2], "<")
        self.assertEqual(deps[0][1][0], "p1b")
        self.assertEqual(deps[0][1][1], "1b")
        self.assertEqual(deps[0][1][2], ">")

        # Check that the type of comparison is parsed correctly.
        self.assertEqual("<", apt_pkg.parse_depends("p1 (<< 1)")[0][0][2])
        self.assertEqual("<=", apt_pkg.parse_depends("p1 (< 1)")[0][0][2])
        self.assertEqual("<=", apt_pkg.parse_depends("p1 (<= 1)")[0][0][2])
        self.assertEqual("=", apt_pkg.parse_depends("p1 (= 1)")[0][0][2])
        self.assertEqual(">=", apt_pkg.parse_depends("p1 (>= 1)")[0][0][2])
        self.assertEqual(">=", apt_pkg.parse_depends("p1 (> 1)")[0][0][2])
        self.assertEqual(">", apt_pkg.parse_depends("p1 (>> 1)")[0][0][2])

    def test_parse_src_depends(self):
        """dependencies: Test apt_pkg.parse_src_depends()."""
        # Check that architecture exclusion works
        # depends_this: Current architecture is included
        # depends_this_too: Another architecture is excluded
        # depends_other: The current architecture is excluded
        # depends_other: Another architecture is requested.
        architecture = apt_pkg.config["APT::Architecture"]
        depends_this = apt_pkg.parse_src_depends("p [%s]" % architecture)
        depends_this_too = apt_pkg.parse_src_depends("p [!not-existing-arch]")
        depends_other = apt_pkg.parse_src_depends("p [!%s]" % architecture)
        depends_other_too = apt_pkg.parse_src_depends("p [not-existing-arch]")

        self.assertEqual(len(depends_this), len(depends_this_too), 1)
        self.assertEqual(len(depends_other), len(depends_other_too), 0)

    def testParseDepends(self):
        """dependencies: Test apt_pkg.ParseDepends()."""
        if not hasattr(apt_pkg, 'ParseDepends'):
            return
        # Check that the type of comparison is parsed correctly.
        self.assertEqual("<<", apt_pkg.ParseDepends("p1 (<< 1)")[0][0][2])
        self.assertEqual("<=", apt_pkg.ParseDepends("p1 (< 1)")[0][0][2])
        self.assertEqual("<=", apt_pkg.ParseDepends("p1 (<= 1)")[0][0][2])
        self.assertEqual("=", apt_pkg.ParseDepends("p1 (= 1)")[0][0][2])
        self.assertEqual(">=", apt_pkg.ParseDepends("p1 (>= 1)")[0][0][2])
        self.assertEqual(">=", apt_pkg.ParseDepends("p1 (> 1)")[0][0][2])
        self.assertEqual(">>", apt_pkg.ParseDepends("p1 (>> 1)")[0][0][2])

if __name__ == "__main__":
    unittest.main()
