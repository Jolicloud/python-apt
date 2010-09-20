#!/usr/bin/python
#
# Copyright (C) 2009 Julian Andres Klode <jak@debian.org>
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.
"""Unit tests for verifying the correctness of hashsums in apt_pkg.

Unit tests to verify the correctness of Hashes, HashString and the various
functions like md5sum."""
import unittest
import hashlib
import sys

import apt_pkg


class TestHashes(unittest.TestCase):
    """Test apt_pkg.Hashes() and the various apt_pkg.*sum() functions."""

    def setUp(self):
        """Prepare the tests, create reference values..."""
        self.file = open(apt_pkg.__file__, "rb")
        self.value = self.file.read()
        self.hashes = apt_pkg.Hashes(self.value)
        self.file.seek(0)
        self.fhashes = apt_pkg.Hashes(self.file)
        # Reference values.
        self.md5 = hashlib.md5(self.value).hexdigest()
        self.sha1 = hashlib.sha1(self.value).hexdigest()
        self.sha256 = hashlib.sha256(self.value).hexdigest()
        self.file.seek(0)

    def tearDown(self):
        """Cleanup, Close the file object used for the tests."""
        self.file.close()

    def test_md5sum(self):
        """hashes: Test apt_pkg.md5sum()"""
        self.assertEqual(apt_pkg.md5sum(self.value), self.md5)
        self.assertEqual(apt_pkg.md5sum(self.file), self.md5)

    def test_sha1sum(self):
        """hashes: Test apt_pkg.sha1sum()"""
        self.assertEqual(apt_pkg.sha1sum(self.value), self.sha1)
        self.assertEqual(apt_pkg.sha1sum(self.file), self.sha1)

    def test_sha256sum(self):
        """hashes: Test apt_pkg.sha256sum()"""
        self.assertEqual(apt_pkg.sha256sum(self.value), self.sha256)
        self.assertEqual(apt_pkg.sha256sum(self.file), self.sha256)

    def test_bytes(self):
        """hashes: Test apt_pkg.Hashes(bytes)"""
        self.assertEqual(self.hashes.md5, self.md5)
        self.assertEqual(self.hashes.sha1, self.sha1)
        self.assertEqual(self.hashes.sha256, self.sha256)

    def test_file(self):
        """hashes: Test apt_pkg.Hashes(file)."""
        self.assertEqual(self.hashes.md5, self.fhashes.md5)
        self.assertEqual(self.hashes.sha1, self.fhashes.sha1)
        self.assertEqual(self.hashes.sha256, self.fhashes.sha256)

    def test_unicode(self):
        """hashes: Test apt_pkg.Hashes(unicode)."""
        if sys.version_info[0] == 3:
            self.assertRaises(TypeError, apt_pkg.Hashes, "D")
            self.assertRaises(TypeError, apt_pkg.md5sum, "D")
            self.assertRaises(TypeError, apt_pkg.sha1sum, "D")
            self.assertRaises(TypeError, apt_pkg.sha256sum, "D")
        else:
            self.assertRaises(TypeError, apt_pkg.Hashes, unicode())
            self.assertRaises(TypeError, apt_pkg.md5sum, unicode())
            self.assertRaises(TypeError, apt_pkg.sha1sum, unicode())
            self.assertRaises(TypeError, apt_pkg.sha256sum, unicode())


class TestHashString(unittest.TestCase):
    """Test apt_pkg.HashString()."""

    def setUp(self):
        """Prepare the test by reading the file."""
        self.hashes = apt_pkg.Hashes(open(apt_pkg.__file__))
        self.md5 = apt_pkg.HashString("MD5Sum", self.hashes.md5)
        self.sha1 = apt_pkg.HashString("SHA1", self.hashes.sha1)
        self.sha256 = apt_pkg.HashString("SHA256", self.hashes.sha256)

    def test_md5(self):
        """hashes: Test apt_pkg.HashString().md5"""
        self.assertEqual("MD5Sum:%s" % self.hashes.md5, str(self.md5))
        self.assertTrue(self.md5.verify_file(apt_pkg.__file__))

    def test_sha1(self):
        """hashes: Test apt_pkg.HashString().sha1"""
        self.assertEqual("SHA1:%s" % self.hashes.sha1, str(self.sha1))
        self.assertTrue(self.sha1.verify_file(apt_pkg.__file__))

    def test_sha256(self):
        """hashes: Test apt_pkg.HashString().sha256"""
        self.assertEqual("SHA256:%s" % self.hashes.sha256, str(self.sha256))
        self.assertTrue(self.sha256.verify_file(apt_pkg.__file__))

    def test_wrong(self):
        """hashes: Test apt_pkg.HashString(wrong_type)."""
        self.assertRaises(TypeError, apt_pkg.HashString, 0)
        if sys.version_info[0] == 3:
            self.assertRaises(TypeError, apt_pkg.HashString, bytes())


if __name__ == "__main__":
    unittest.main()
