#!/usr/bin/python2.6
#
# migrate-0.8.py - Find use of deprecated methods/attributes in the code.
#
# Copyright (C) 2009 Julian Andres Klode <jak@debian.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
"""migrate-0.8.py - Find all occurences of funcs./attrs. deprecated in 0.8.

Usage: python2.6 migrate-0.8.py [options] <file/directory>...

This reads the list of all functions and attributes available only in
COMPAT_0_7 builds and checks for occurences in the given Python modules. Has
to be run from the python-apt source code directory.

Requires python2.6 to be installed.

Parameters:
    -h  Display this help
    -c  Colorize the matching parts in the output.
"""
import _ast
import ast
import glob
import linecache
import os
import re
import sys
import types
from collections import defaultdict
from textwrap import fill

color=False
if len(sys.argv) > 1 and sys.argv[1] in ('-c', '--color', '--colour'):
    color=True
    del sys.argv[1]

if '-h' in sys.argv or '--help' in sys.argv or not sys.argv[1:]:
    print __doc__.strip()
    sys.exit(0)

if os.path.dirname(__file__).endswith('utils'):
    sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))

deprecated_cpp_stuff = set([
    '.Add', '.AllTargets', '.Arch', '.Architecture', '.Archive', 
    '.ArchiveURI', '.Auto', '.Base64Encode', '.Binaries', '.BrokenCount', 
    '.BuildDepends', '.Bytes', '.CheckDep', '.CheckDomainList', '.Clear', 
    '.Close', '.Commit', '.CompType', '.Complete', '.Component', '.Config', 
    '.CurStateConfigFiles', '.CurStateHalfConfigured', 
    '.CurStateHalfInstalled', '.CurStateInstalled', '.CurStateNotInstalled', 
    '.CurStateUnPacked', '.CurrentState', '.CurrentVer', '.Date', 
    '.DeQuoteString', '.DebSize', '.DelCount', '.DepType', '.DependsCount', 
    '.DependsList', '.DependsListStr', '.DescURI', '.Describe', '.DestFile', 
    '.Dist', '.DoInstall', '.Downloadable', '.ErrorText', '.Essential', 
    '.Exists', '.FetchNeeded', '.FileList', '.FileName', '.FileSize', 
    '.Files', '.Find', '.FindB', '.FindDir', '.FindFile', '.FindFlag', 
    '.FindI', '.FindIndex', '.FindRaw', '.FixBroken', '.FixMissing', 
    '.GetAcquire', '.GetArchives', '.GetCache', '.GetCandidateVer', 
    '.GetCdrom', '.GetDepCache', '.GetIndexes', '.GetLock', 
    '.GetPackageManager', '.GetPkgAcqFile', '.GetPkgActionGroup', 
    '.GetPkgProblemResolver', '.GetPkgRecords', '.GetPkgSourceList', 
    '.GetPkgSrcRecords', '.HasPackages', '.Hash', '.Homepage', '.ID', 
    '.Ident', '.Important', '.Index', '.IndexFiles', '.IndexType', '.Init', 
    '.InitConfig', '.InitSystem', '.InstCount', '.InstState', 
    '.InstStateHold', '.InstStateHoldReInstReq', '.InstStateOk', 
    '.InstStateReInstReq', '.InstallProtect', '.InstalledSize', 
    '.IsAutoInstalled', '.IsGarbage', '.IsInstBroken', '.IsNowBroken', 
    '.IsTrusted', '.IsUpgradable', '.Items', '.Jump', '.KeepCount', 
    '.Label', '.LanguageCode', '.LibVersion', '.List', '.Local', 
    '.LongDesc', '.Lookup', '.MD5Hash', '.Maintainer', '.MarkDelete', 
    '.MarkInstall', '.MarkKeep', '.MarkedDelete', '.MarkedDowngrade', 
    '.MarkedInstall', '.MarkedKeep', '.MarkedReinstall', '.MarkedUpgrade', 
    '.MinimizeUpgrade', '.MyTag', '.Name', '.NotAutomatic', '.NotSource', 
    '.Offset', '.Open', '.Origin', '.Package', '.PackageCount', 
    '.PackageFileCount', '.Packages', '.ParentPkg', '.ParentVer', 
    '.ParseCommandLine', '.ParseDepends', '.ParseSection', 
    '.ParseSrcDepends', '.ParseTagFile', '.PartialPresent', 
    '.PkgSystemLock', '.PkgSystemUnLock', '.PriExtra', '.PriImportant', 
    '.PriOptional', '.PriRequired', '.PriStandard', '.Priority', 
    '.PriorityStr', '.Protect', '.ProvidesCount', '.ProvidesList', 
    '.QuoteString', '.ReadConfigDir', '.ReadConfigFile', 
    '.ReadConfigFileISC', '.ReadMainList', '.ReadPinFile', '.Record', 
    '.Remove', '.Resolve', '.ResolveByKeep', '.Restart', '.RevDependsList', 
    '.RewriteSection', '.RewriteSourceOrder', '.Run', '.SHA1Hash', 
    '.SHA256Hash', '.Section', '.SelStateDeInstall', '.SelStateHold', 
    '.SelStateInstall', '.SelStatePurge', '.SelStateUnknown', 
    '.SelectedState', '.Set', '.SetCandidateVer', '.SetReInstall', 
    '.ShortDesc', '.Shutdown', '.Site', '.Size', '.SizeToStr', 
    '.SmartTargetPkg', '.SourcePkg', '.SourceVer', '.Status', '.Step', 
    '.StrToTime', '.StringToBool', '.SubTree', '.TargetPkg', '.TargetVer', 
    '.Time', '.TimeRFC1123', '.TimeToStr', '.TotalNeeded', 
    '.TranslationDescription', '.URI', '.URItoFileName', '.Update', 
    '.Upgrade', '.UpstreamVersion', '.UsrSize', '.ValueList', 
    '.VerFileCount', '.VerStr', '.Version', '.VersionCompare', 
    '.VersionCount', '.VersionList', '.newConfiguration', 'Base64Encode', 
    'CheckDep', 'CheckDomainList', 'Config', 'CurStateConfigFiles', 
    'CurStateHalfConfigured', 'CurStateHalfInstalled', 'CurStateInstalled', 
    'CurStateNotInstalled', 'CurStateUnPacked', 'Date', 'DeQuoteString', 
    'GetAcquire', 'GetCache', 'GetCdrom', 'GetDepCache', 'GetLock', 
    'GetPackageManager', 'GetPkgAcqFile', 'GetPkgActionGroup', 
    'GetPkgProblemResolver', 'GetPkgRecords', 'GetPkgSourceList', 
    'GetPkgSrcRecords', 'InitConfig', 'InitSystem', 'InstStateHold', 
    'InstStateHoldReInstReq', 'InstStateOk', 'InstStateReInstReq', 
    'LibVersion', 'ParseCommandLine', 'ParseDepends', 'ParseSection', 
    'ParseSrcDepends', 'ParseTagFile', 'PkgSystemLock', 'PkgSystemUnLock', 
    'PriExtra', 'PriImportant', 'PriOptional', 'PriRequired', 'PriStandard', 
    'QuoteString', 'ReadConfigDir', 'ReadConfigFile', 'ReadConfigFileISC', 
    'RewriteSection', 'RewriteSourceOrder', 'SelStateDeInstall', 
    'SelStateHold', 'SelStateInstall', 'SelStatePurge', 'SelStateUnknown', 
    'SizeToStr', 'StrToTime', 'StringToBool', 'Time', 'TimeRFC1123', 
    'TimeToStr', 'URItoFileName', 'UpstreamVersion', 'VersionCompare', 
    'newConfiguration', '.has_key'])

def do_color(string, words):
    """Colorize (red) the given words in the given string."""
    if not color:
        return string
    for word in words:
        word = re.escape(word)
        string = re.sub('([^_]*)(%s)([^_]*)' % word, "\\1\033[31m\033[1m" +
                        r"\2" + "\033[0m\\3", string)
    return string

def find_deprecated_py():
    """Find all the deprecated functions and attributes.

    Import apt_pkg, set _COMPAT_0_7 to 0, import apt and aptsources and
    create a list of all attributes. Then remove the imported modules,
    reimport them (with _COMPAT_0_7=1), and see which functions do not
    exist anymore.
    """

    modules = ('apt', 'apt.package', 'apt.cdrom', 'apt.cache', 'apt.debfile',
               'apt.progress', 'apt.progress.old', 'aptsources.distinfo',
               'aptsources.distro', 'aptsources.sourceslist')

    import apt_pkg
    apt_pkg._COMPAT_0_7 = 0

    empty = set(sys.modules)
    new, deprecated = set(), set()

    for mname in sorted(modules):
        module = __import__(mname, fromlist=['*'])

        for clsname in dir(module):
            cls = getattr(module, clsname)
            if not isinstance(cls, types.TypeType):
                new.add(clsname)
                continue
            # Attributes/Methods
            new.update(clsname + '.' + name for name in dir(cls))

    for mname in sys.modules.keys():
        if not mname in empty:
            del sys.modules[mname]

    apt_pkg._COMPAT_0_7 = 1

    for mname in sorted(modules):
        module = __import__(mname, fromlist=['*'])
        for clsname in dir(module):
            cls = getattr(module, clsname)
            if not isinstance(cls, types.TypeType):
                deprecated.add(clsname)
                continue
            for name in dir(cls):
                if not clsname + '.' + name in new:
                    # Attributes/Methods, which are deprecated (not in new).
                    deprecated.add('.' + name)

    for mname in sys.modules.keys():
        if not mname in empty:
            del sys.modules[mname]

    return deprecated.difference(new)


def find_occurences(all_old, files):
    """Find all ocurrences in the given Python files."""
    for fname in files:
        if not os.path.exists(fname):
            continue
        if not (fname in sys.argv or fname.endswith('.py') or
                re.match('^#.*python.*', open(fname).readline())):
            continue

        words = defaultdict(lambda: set())
        try:
            node = ast.parse(open(fname, "rU").read(), fname)
        except Exception, e:
            print >> sys.stderr, "Ignoring %s: %s" % (fname, e)
            continue
        for i in ast.walk(node):
            if isinstance(i, _ast.ImportFrom):
                for alias in i.names:
                    if alias.name in all_old:
                        words[i.lineno].add(alias.name)
            if isinstance(i, _ast.Name) and i.id in all_old:
                words[i.lineno].add(i.id)

            if isinstance(i, _ast.Attribute) and ('.' + i.attr in all_old):
                words[i.lineno].add(i.attr)

        for lineno in sorted(words):
            line = do_color(linecache.getline(fname, lineno).rstrip('\n'),
                            words[lineno])
            print '%s:%s:%s' % (fname, lineno, line)

# Now, let's find them in the code.

print __doc__.split("\n")[0]
print
print fill('Information: Please verify that the results are correct before '
           'you modify any code, because there may be false positives.', 79)
print
if color:
    print fill('Information: The color is not always correct, because we '
               'simply highlight the matched words (like grep).', 79)
    print

all_old = deprecated_cpp_stuff

if not '-P' in sys.argv:
    all_old |= find_deprecated_py()
else:
    sys.argv.remove('-P')

files = set()
for path in sys.argv[1:]:
    if not os.path.exists(path):
        raise ValueError('Path does not exist: %s' % path)
    if os.path.isfile(path):
        files.add(path)
    else:
        for root, dirs, files_ in os.walk(path):
            for fname in files_:
                files.add(os.path.normpath(os.path.join(root, fname)))

find_occurences(all_old, sorted(files))
