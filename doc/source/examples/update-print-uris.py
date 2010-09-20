#!/usr/bin/python
"""Print out the URIs of all indexes files.

This behaves somewhat like apt-get --print-uris update."""
import apt_pkg


def main():
    apt_pkg.init_config()
    apt_pkg.init_system()
    acquire = apt_pkg.Acquire()
    slist = apt_pkg.SourceList()
    # Read the list
    slist.read_main_list()
    # Add all indexes to the fetcher.
    slist.get_indexes(acquire, True)

    # Now print the URI of every item.
    for item in acquire.items:
        print item.desc_uri

if __name__ == '__main__':
    main()
