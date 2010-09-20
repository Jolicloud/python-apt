#!/usr/bin/python
"""Example for packages. Print all essential and important packages"""

import apt_pkg


def main():
    """Main."""
    apt_pkg.init_config()
    apt_pkg.init_system()
    cache = apt_pkg.Cache()
    print "Essential packages:"
    for pkg in cache.packages:
        if pkg.essential:
            print " ", pkg.name
    print "Important packages:"
    for pkg in cache.packages:
        if pkg.important:
            print " ", pkg.name

if __name__ == "__main__":
    main()
