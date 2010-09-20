#!/usr/bin/python
import unittest

import apt_pkg
import apt.progress.base


class TestCache(unittest.TestCase):
    """Test invocation of apt_pkg.Cache()"""

    def setUp(self):
        apt_pkg.init_config()
        apt_pkg.init_system()

    def test_wrong_invocation(self):
        """cache_invocation: Test wrong invocation."""
        apt_cache = apt_pkg.Cache(apt.progress.base.OpProgress())

        self.assertRaises(ValueError, apt_pkg.Cache, apt_cache)
        self.assertRaises(ValueError, apt_pkg.Cache,
                          apt.progress.base.AcquireProgress())
        self.assertRaises(ValueError, apt_pkg.Cache, 0)

    def test_proper_invocation(self):
        """cache_invocation: Test correct invocation."""
        apt_cache = apt_pkg.Cache(apt.progress.base.OpProgress())
        apt_depcache = apt_pkg.DepCache(apt_cache)

if __name__ == "__main__":
    unittest.main()
