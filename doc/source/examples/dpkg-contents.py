#!/usr/bin/python
"""Emulate dpkg --contents"""

import grp
import pwd
import stat
import sys
import time

import apt_inst


def format_mode(what, mode):
    """Return the symbolic mode"""
    s_mode = dict(DIR="d", HARDLINK="h", FILE="-").get(what)
    s_mode += ((mode & stat.S_IRUSR) and "r" or "-")
    s_mode += ((mode & stat.S_IWUSR) and "w" or "-")
    s_mode += ((mode & stat.S_IXUSR) and (mode & stat.S_ISUID and "s" or "x")
                                      or (mode & stat.S_ISUID and "S" or "-"))
    s_mode += ((mode & stat.S_IRGRP) and "r" or "-")
    s_mode += ((mode & stat.S_IWGRP) and "w" or "-")
    s_mode += ((mode & stat.S_IXGRP) and (mode & stat.S_ISGID and "s" or "x")
                                      or (mode & stat.S_ISGID and "S" or "-"))
    s_mode += ((mode & stat.S_IROTH) and "r" or "-")
    s_mode += ((mode & stat.S_IWOTH) and "w" or "-")
    s_mode += ((mode & stat.S_IXOTH) and "x" or "-")
    return s_mode


def callback(what, name, link, mode, uid, gid, size, mtime, major, minor):
    """callback for deb_extract"""
    s_mode = format_mode(what, mode)
    s_owner = "%s/%s" % (pwd.getpwuid(uid)[0], grp.getgrgid(gid)[0])
    s_size = "%9d" % size
    s_time = time.strftime("%Y-%m-%d %H:%M", time.localtime(mtime))
    s_name = name.startswith(".") and name or ("./" + name)
    if link:
        s_name += " link to %s" % link
    print s_mode, s_owner, s_size, s_time, s_name


def main():
    """Main function"""
    if len(sys.argv) < 2:
        print >> sys.stderr, "need filename argumnet"
        sys.exit(1)

    fobj = open(sys.argv[1])
    try:
        apt_inst.deb_extract(fobj, callback, "data.tar.gz")
    finally:
        fobj.close()

if __name__ == "__main__":
    main()
