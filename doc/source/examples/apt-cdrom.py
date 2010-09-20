#!/usr/bin/python
import sys

import apt_pkg
import apt


def show_help():
    print ("apt %s compiled on %s %s" % (apt_pkg.VERSION,
                    apt_pkg.DATE, apt_pkg.TIME))
    if apt_pkg.config.find_b("version"):
        return 0

    # Copied from apt-cdrom
    print ("Usage: apt-cdrom [options] command\n"
           "\n"
           "apt-cdrom is a tool to add CDROM's to APT's source list. The\n"
           "CDROM mount point and device information is taken from apt.conf\n"
           "and /etc/fstab.\n"
           "\n"
           "Commands:\n"
           "   add - Add a CDROM\n"
           "   ident - Report the identity of a CDROM\n"
           "\n"
           "Options:\n"
           "  -h   This help text\n"
           "  -d   CD-ROM mount point\n"
           "  -r   Rename a recognized CD-ROM\n"
           "  -m   No mounting\n"
           "  -f   Fast mode, don't check package files\n"
           "  -a   Thorough scan mode\n"
           "  -c=? Read this configuration file\n"
           "  -o=? Set an arbitrary configuration option, eg -o "
           "dir::cache=/tmp\n"
           "See fstab(5)")
    return 0


def main(args):
    arguments = apt_pkg.parse_commandline(apt_pkg.config,
                    [('h', "help", "help"),
                     ('v', "version", "version"),
                     ('d', "cdrom", "Acquire::cdrom::mount", "HasArg"),
                     ('r', "rename", "APT::CDROM::Rename"),
                     ('m', "no-mount", "APT::CDROM::NoMount"),
                     ('f', "fast", "APT::CDROM::Fast"),
                     ('n', "just-print", "APT::CDROM::NoAct"),
                     ('n', "recon", "APT::CDROM::NoAct"),
                     ('n', "no-act", "APT::CDROM::NoAct"),
                     ('a', "thorough", "APT::CDROM::Thorough"),
                     ('c', "config-file", "", "ConfigFile"),
                     ('o', "option", "", "ArbItem")], args)

    if apt_pkg.config.find_b("help") or apt_pkg.config.find_b("version"):
        return show_help()

    progress = apt.progress.text.CdromProgress()
    cdrom = apt_pkg.Cdrom()

    if not arguments:
        return show_help()
    elif arguments[0] == 'add':
        cdrom.add(progress)
    elif arguments[0] == 'ident':
        cdrom.ident(progress)
    else:
        sys.stderr.write('E: Invalid operation %s\n' % arguments[0])
        return 1

if __name__ == '__main__':
    sys.exit(main(sys.argv))
