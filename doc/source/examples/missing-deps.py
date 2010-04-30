#!/usr/bin/python
"""Check the archive for missing dependencies"""
import apt_pkg


def fmt_dep(dep):
    """Format a Dependency object [of apt_pkg] as a string."""
    ret = dep.TargetPkg.Name
    if dep.TargetVer:
        ret += " (%s %s)" % (dep.CompType, dep.TargetVer)
    return ret


def check_version(pkgver):
    """Check the version of the package"""
    missing = []

    for or_group in pkgver.DependsList.get("Pre-Depends", []) + \
                    pkgver.DependsList.get("Depends", []):
        if not any(dep.AllTargets() for dep in or_group):
            # If none of the or-choices can be satisfied, add it to missing
            missing.append(or_group)

    if missing:
        print "Package:", pkgver.ParentPkg.Name
        print "Version:", pkgver.VerStr
        print "Missing:",
        print ", ".join(" | ".join(fmt_dep(dep) for dep in or_group)
                        for or_group in missing)
        print


def main():
    """The main function."""
    apt_pkg.InitConfig()
    apt_pkg.InitSystem()

    cache = apt_pkg.GetCache()

    for pkg in sorted(cache.Packages, key=lambda pkg: pkg.Name):
        # pkg is from a list of packages, sorted by name.
        for version in pkg.VersionList:
            # Check every version
            for pfile, _ in version.FileList:
                if (pfile.Origin == "Debian" and pfile.Component == "main" and
                    pfile.Archive == "unstable"):
                    # We only want packages from Debian unstable main.
                    check_version(version)
                    break

if __name__ == "__main__":
    main()
