#!/usr/bin/python
import apt_pkg


def main():
    """Example for PackageFile()"""
    apt_pkg.init()
    cache = apt_pkg.GetCache()
    for pkgfile in cache.FileList:
        print 'Package-File:', pkgfile.FileName
        print 'Index-Type:', pkgfile.IndexType # 'Debian Package Index'
        if pkgfile.NotSource:
            print 'Source: None'
        else:
            if pkgfile.Site:
                # There is a source, and a site, print the site
                print 'Source:', pkgfile.Site
            else:
                # It seems to be a local repository
                print 'Source: Local package file'
        if pkgfile.NotAutomatic:
            # The system won't be updated automatically (eg. experimental)
            print 'Automatic: No'
        else:
            print 'Automatic: Yes'
        print

if __name__ == '__main__':
    main()
