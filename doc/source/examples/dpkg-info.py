#!/usr/bin/python
"""Emulate dpkg --info package.deb control-file"""
import sys

from apt_inst import deb_extract_control


def main():
    """Main function."""
    if len(sys.argv) < 3:
        print >> sys.stderr, 'Usage: tool file.deb control-file'
        sys.exit(0)
    fobj = open(sys.argv[1])
    try:
        print deb_extract_control(fobj, sys.argv[2])
    finally:
        fobj.close()

if __name__ == '__main__':
    main()
