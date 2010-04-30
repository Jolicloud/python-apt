#!/usr/bin/python
"""Example for packages. Print all essential and important packages"""

import apt_pkg


def main():
    """Main."""
    apt_pkg.InitConfig()
    apt_pkg.InitSystem()
    cache = apt_pkg.GetCache()
    print "Essential packages:"
    for pkg in cache.Packages:
        if pkg.Essential:
            print " ", pkg.Name
    print "Important packages:"
    for pkg in cache.Packages:
        if pkg.Important:
            print " ", pkg.Name

if __name__ == "__main__":
    main()
