#!/usr/bin/python
import apt_pkg


def main():
    """Example for PackageFile()"""
    apt_pkg.init()
    cache = apt_pkg.Cache()
    for pkgfile in cache.file_list:
        print 'Package-File:', pkgfile.filename
        print 'Index-Type:', pkgfile.index_type # 'Debian Package Index'
        if pkgfile.not_source:
            print 'Source: None'
        else:
            if pkgfile.site:
                # There is a source, and a site, print the site
                print 'Source:', pkgfile.site
            else:
                # It seems to be a local repository
                print 'Source: Local package file'
        if pkgfile.not_automatic:
            # The system won't be updated automatically (eg. experimental)
            print 'Automatic: No'
        else:
            print 'Automatic: Yes'
        print

if __name__ == '__main__':
    main()
