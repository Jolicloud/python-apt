#!/usr/bin/python
"""Emulate dpkg --extract package.deb outdir"""
import os
import sys

import apt_inst


def main():
    """Main function."""
    if len(sys.argv) < 3:
        print >> sys.stderr, "Usage:", __file__, "package.deb outdir"
        sys.exit(1)
    if not os.path.exists(sys.argv[2]):
        print >> sys.stderr, "The directory %s does not exist" % sys.argv[2]
        sys.exit(1)

    fobj = open(sys.argv[1])
    try:
        apt_inst.deb_extract_archive(fobj, sys.argv[2])
    finally:
        fobj.close()

if __name__ == "__main__":
    main()
