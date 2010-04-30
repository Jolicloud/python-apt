Classes in apt_pkg
==================

.. todo::

    This should be split and cleaned up a bit.

:class:`Acquire`
----------------
.. class:: Acquire

    .. method:: Run()

        Fetch all the items which have been added by
        :func:`apt_pkg.GetPkgAcqFile`.

    .. method:: Shutdown

        Shut the fetcher down.

:class:`PkgAcqFile`
-------------------
.. class:: PkgAcqFile

    This class provides no methods or attributes

:class:`AcquireItem`
---------------------

.. class:: AcquireItem

    .. attribute:: ID

        The ID of the item.

    .. attribute:: Complete

        Is the item completely acquired?

    .. attribute:: Local

        Is the item a local file?

    .. attribute:: IsTrusted

        Can the file be trusted?

    .. attribute:: FileSize

        The size of the file, in bytes.

    .. attribute:: ErrorText

        The error message. For example, when a file does not exist on a http
        server, this will contain a 404 error message.

    .. attribute:: DestFile

        The location the file is saved as.

    .. attribute:: DescURI

        The source location.

    **Status**:

    .. attribute:: Status

        Integer, representing the status of the item.

    .. attribute:: StatIdle

        Constant for comparing :attr:`AcquireItem.Status`.

    .. attribute:: StatFetching

        Constant for comparing :attr:`AcquireItem.Status`

    .. attribute:: StatDone

        Constant for comparing :attr:`AcquireItem.Status`

    .. attribute:: StatError

        Constant for comparing :attr:`AcquireItem.Status`

    .. attribute:: StatAuthError

        Constant for comparing :attr:`AcquireItem.Status`

:class:`ActionGroup`
--------------------

.. class:: ActionGroup

    Normally, apt checkes the package cache after every modification for
    packages which are automatically installed but on which no package depends
    anymore (it collects the package garbage).

    Using ActionGroups you can turn this off and therefore make your code
    much faster.

    Initialize it using :func:`apt_pkg.GetPkgActionGroup`, eg::

        apt_pkg.GetPkgActionGroup(depcache)

    .. method:: release

        Release the ActionGroup. This will reactive the collection of package
        garbage.


:class:`Configuration`
----------------------

.. class:: Configuration

    The Configuration objects store the configuration of apt.

    .. describe:: conf[key]

        Return the value of the option given key *key*. If it does not
        exist, raise :exc:`KeyError`.

    .. describe:: conf[key] = value

        Set the option at *key* to *value*.

    .. method:: Find(key[, default=''])

        Return the value for the given key *key*. This is the same as
        :meth:`Configuration.get`.

        If *key* does not exist, return *default*.

    .. method:: FindFile(key[, default=''])

        Return the filename hold by the configuration at *key*. This formats the
        filename correctly and supports the Dir:: stuff in the configuration.

        If *key* does not exist, return *default*.

    .. method:: FindDir(key[, default='/'])

        Return the absolute path to the directory specified in *key*. A
        trailing slash is appended.

        If *key* does not exist, return *default*.

    .. method:: FindI(key[, default=0])

        Return the integer value stored at *key*.

        If *key* does not exist, return *default*.

    .. method:: FindB(key[, default=0])

        Return the boolean value stored at *key*. This returns an integer, but
        it should be treated like True/False.

        If *key* does not exist, return *default*.

    .. method:: Set(key, value)

        Set the value of *key* to *value*.

    .. method:: Exists(key)

        Check whether the key *key* exists in the configuration.

    .. method:: SubTree(key)

        Return a sub tree starting at *key*. The resulting object can be used
        like this one.

    .. method:: List([key])

        List all items at *key*. Normally, return the keys at the top level,
        eg. APT, Dir, etc.

        Use *key* to specify a key of which the childs will be returned.

    .. method:: ValueList([key])

        Same as :meth:`Configuration.List`, but this time for the values.

    .. method:: MyTag()

        Return the tag name of the current tree. Normally this is an empty
        string, but for subtrees it is the key of the subtree.

    .. method:: Clear(key)

        Clear the configuration. Remove all values and keys at *key*.

    .. method:: keys([key])

        Return all the keys, recursive. If *key* is specified, ... (FIXME)

    .. method:: has_key(key)

        Return whether the configuration contains the key *key*.

    .. method:: get(key[, default=''])

        This behaves just like :meth:`dict.get` and :meth:`Configuration.Find`,
        it returns the value of key or if it does not exist, *default*.


:class:`pkgCache`
-----------------
.. class:: pkgCache

    The :class:`pkgCache` class prov

    .. describe:: cache[pkgname]

        Return the :class:`Package()` object for the package name given by
        *pkgname*.

    .. method:: Close()

        Close the package cache.

    .. method:: Open([progress])

        Open the package cache again. The parameter *progress* may be set to
        an :class:`apt.progress.OpProgress()` object or `None`.

    .. method:: Update(progress, list[, pulseInterval])

        Update the package cache.

        The parameter *progress* points to an :class:`apt.progress.FetchProgress()`
        object.

        The parameter *list* refers to an object as returned by
        :func:`apt_pkg.GetPkgSourceList`.

        The optional parameter *pulseInterval* describes the interval between
        the calls to the :meth:`FetchProgress.pulse` method.

    .. attribute:: DependsCount

        The total number of dependencies.

    .. attribute:: PackageCount

        The total number of packages available in the cache.

    .. attribute:: ProvidesCount

        The number of provided packages.

    .. attribute:: VerFileCount

        .. todo:: Seems to be some mixture of versions and pkgFile.

    .. attribute:: VersionCount

        The total number of package versions available in the cache.

    .. attribute:: PackageFileCount

        The total number of Packages files available (the Packages files
        listing the packages). This is the same as the length of the list in
        the attribute :attr:`FileList`.

    .. attribute:: FileList

        A list of :class:`PackageFile` objects.


:class:`PackageFile`
--------------------
.. class:: PackageFile

    A :class:`PackageFile` represents a Packages file, eg.
    /var/lib/dpkg/status.

    .. attribute:: Architecture

        The architecture of the package file.

    .. attribute:: Archive

        The archive (eg. unstable)

    .. attribute:: Component

        The component (eg. main)

    .. attribute:: FileName

        The name of the file.

    .. attribute:: ID

        The ID of the package. This is an integer which can be used to store
        further information about the file [eg. as dictionary key].

    .. attribute:: IndexType

        The sort of the index file. In normal cases, this is
        'Debian Package Index'.

    .. attribute:: Label

        The Label, as set in the Release file

    .. attribute:: NotAutomatic

        Whether packages from this list will be updated automatically. The
        default for eg. example is 0 (aka false).

    .. attribute:: NotSource

        Whether the file has no source from which it can be updated. In such a
        case, the value is 1; else 0. /var/lib/dpkg/status is 0 for example.

        Example::

            for pkgfile in cache.FileList:
                if pkgfile.NotSource:
                    print 'The file %s has no source.' % pkgfile.FileName

    .. attribute:: Origin

        The Origin, as set in the Release file

    .. attribute:: Site

        The hostname of the site.

    .. attribute:: Size

        The size of the file.

    .. attribute:: Version

        The version, as set in the release file (eg. "4.0" for "Etch")


Example
^^^^^^^
.. literalinclude:: ../examples/cache-pkgfile.py


:class:`Package`
----------------

.. class:: Package

    The pkgCache::Package objects are an interface to package specific
    features.


    Attributes:

    .. attribute:: CurrentVer

        The version currently installed, or None. This returns a
        :class:`Version` object.

    .. attribute:: ID

        The ID of the package. This can be used to store information about
        the package. The ID is an int value.

    .. attribute:: Name

        This is the name of the package.

    .. attribute:: ProvidesList

        A list of packages providing this package. More detailed, this is a
        list of tuples (str:pkgname, ????, :class:`Version`).

        If you want to check for check for virtual packages, the expression
        ``pkg.ProvidesList and not pkg.VersionList`` helps you. It detects if
        the package is provided by something else and is not available as a
        real package.

    .. attribute:: RevDependsList

        An iterator of :class:`Dependency` objects for dependencies on this
        package.

    .. attribute:: Section

        The section of the package, as specified in the record. The list of
        possible sections is defined in the Policy.

    .. attribute:: VersionList

        A list of :class:`Version` objects for all versions available in the
        cache.

    **States**:

    .. attribute:: SelectedState

        The state we want it to be, ie. if you mark a package for installation,
        this is :attr:`apt_pkg.SelStateInstall`.

        See :ref:`SelStates` for a list of available states.

    .. attribute:: InstState

        The state the currently installed version is in. This is normally
        :attr:`apt_pkg.InstStateOK`, unless the installation failed.

        See :ref:`InstStates` for a list of available states.

    .. attribute:: CurState

        The current state of the package (not installed, unpacked, installed,
        etc). See :ref:`CurStates` for a list of available states.

    **Flags**:

    .. attribute:: Auto

        Whether the package was installed automatically as a dependency of
        another package. (or marked otherwise as automatically installed)

    .. attribute:: Essential

        Whether the package is essential.

    .. attribute:: Important

        Whether the package is important.

Example:
^^^^^^^^^
.. literalinclude:: ../examples/cache-packages.py



:class:`Version`
----------------
.. class:: Version

    The version object contains all information related to a specific package
    version.

    .. attribute:: VerStr

        The version, as a string.

    .. attribute:: Section

        The usual sections (eg. admin, net, etc.). Prefixed with the component
        name for packages not in main (eg. non-free/admin).

    .. attribute:: Arch

        The architecture of the package, eg. amd64 or all.

    .. attribute:: FileList

        A list of (:class:`PackageFile`, int: index) tuples for all Package
        files containing this version of the package.

    .. attribute:: DependsListStr

        A dictionary of dependencies. The key specifies the type of the
        dependency ('Depends', 'Recommends', etc.).


        The value is a list, containing items which refer to the or-groups of
        dependencies. Each of these or-groups is itself a list, containing
        tuples like ('pkgname', 'version', 'relation') for each or-choice.

        An example return value for a package with a 'Depends: python (>= 2.4)'
        would be::

            {'Depends': [
                            [
                             ('python', '2.4', '>=')
                            ]
                        ]
            }

        The same for a dependency on A (>= 1) | B (>= 2)::

            {'Depends': [
                            [
                                ('A', '1', '>='),
                                ('B', '2', '>='),
                            ]
                        ]
            }

    .. attribute:: DependsList

        This is basically the same as :attr:`Version.DependsListStr`,
        but instead of the ('pkgname', 'version', 'relation') tuples,
        it returns :class:`Dependency` objects, which can assist you with
        useful functions.

    .. attribute:: ParentPkg

        The :class:`Package` object this version belongs to.

    .. attribute:: ProvidesList

        This returns a list of all packages provided by this version. Like
        :attr:`Package.ProvidesList`, it returns a list of tuples
        of the form ('virtualpkgname', ???, :class:`Version`), where as the
        last item is the same as the object itself.

    .. attribute:: Size

        The size of the .deb file, in bytes.

    .. attribute:: InstalledSize

        The size of the package (in kilobytes), when unpacked on the disk.

    .. attribute:: Hash

        An integer hash value.

    .. attribute:: ID

        An integer id.

    .. attribute:: Priority

        The integer representation of the priority. This can be used to speed
        up comparisons a lot, compared to :attr:`Version.PriorityStr`.

        The values are defined in the :mod:`apt_pkg` extension, see
        :ref:`Priorities` for more information.

    .. attribute:: PriorityStr

        Return the priority of the package version, as a string, eg.
        "optional".

    .. attribute:: Downloadable

        Whether this package can be downloaded from a remote site.

    .. attribute:: TranslatedDescription

        Return a :class:`Description` object.


:class:`Dependency`
-------------------
.. class:: Dependency

    Represent a dependency from one package to another one.

    .. method:: AllTargets

        A list of :class:`Version` objects which satisfy the dependency,
        and do not conflict with already installed ones.

        From my experience, if you use this method to select the target
        version, it is the best to select the last item unless any of the
        other candidates is already installed. This leads to results being
        very close to the normal package installation.

    .. method:: SmartTargetPkg

        Return a :class:`Version` object of a package which satisfies the
        dependency and does not conflict with installed packages
        (the 'natural target').

    .. attribute:: TargetVer

        The target version of the dependency, as string. Empty string if the
        dependency is not versioned.

    .. attribute:: TargetPkg

        The :class:`Package` object of the target package.

    .. attribute:: ParentVer

        The :class:`Version` object of the parent version, ie. the package
        which declares the dependency.

    .. attribute:: ParentPkg

        The :class:`Package` object of the package which declares the
        dependency. This is the same as using ParentVer.ParentPkg.

    .. attribute:: CompType

        The type of comparison (>=, ==, >>, <=), as string.

    .. attribute:: DepType

        The type of the dependency, as string, eg. "Depends".

    .. attribute:: ID

        The ID of the package, as integer.

Example: Find all missing dependencies
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
With the help of Dependency.AllTargets(), you can easily find all packages with
broken dependencies:

.. literalinclude:: ../examples/missing-deps.py


:class:`Description`
--------------------
.. class:: Description

    Represent the description of the package.

    .. attribute:: LanguageCode

        The language code of the description

    .. attribute:: md5

        The md5 hashsum of the description

    .. attribute:: FileList

        A list of tuples (:class:`PackageFile`, int: index).



:class:`pkgDepCache`
--------------------
.. class:: pkgDepCache

    The pkgDepCache object contains various methods to manipulate the cache,
    to install packages, to remove them, and much more.

    .. method:: Commit(fprogress, iprogress)

        Apply all the changes made.

        The parameter *fprogress* has to be set to an instance of
        apt.progress.FetchProgress or one of its subclasses.

        The parameter *iprogress* has to be set to an instance of
        apt.progress.InstallProgress or one of its subclasses.

    .. method:: FixBroken()

        Try to fix all broken packages in the cache.

    .. method:: GetCandidateVer(pkg)

        Return the candidate version of the package, ie. the version that
        would be installed normally.

        The parameter *pkg* refers to an :class:`Package` object,
        available using the :class:`pkgCache`.

        This method returns a :class:`Version` object.

    .. method:: SetCandidateVer(pkg, version)

        The opposite of :meth:`pkgDepCache.GetCandidateVer`. Set the candidate
        version of the :class:`Package` *pkg* to the :class:`Version`
        *version*.


    .. method:: Upgrade([distUpgrade=False])

        Perform an upgrade. More detailed, this marks all the upgradable
        packages for upgrade. You still need to call
        :meth:`pkgDepCache.Commit` for the changes to apply.

        To perform a dist-upgrade, the optional parameter *distUpgrade* has
        to be set to True.

    .. method:: FixBroken()

        Fix broken packages.

    .. method:: ReadPinFile()

        Read the policy, eg. /etc/apt/preferences.

    .. method:: MinimizeUpgrade()

        Go over the entire set of packages and try to keep each package marked
        for upgrade. If a conflict is generated then the package is restored.

        .. todo::
            Explain better..

    .. method:: MarkKeep(pkg)

        Mark the :class:`Package` *pkg* for keep.

    .. method:: MarkDelete(pkg[, purge])

        Mark the :class:`Package` *pkg* for delete. If *purge* is True,
        the configuration files will be removed as well.

    .. method:: MarkInstall(pkg[, autoInst=True[, fromUser=True]])

        Mark the :class:`Package` *pkg* for install.

        If *autoInst* is ``True``, the dependencies of the package will be
        installed as well. This is the default.

        If *fromUser* is ``True``, the package will be marked as manually
        installed. This is the default.

    .. method:: SetReinstall(pkg)

        Set if the :class:`Package` *pkg* should be reinstalled.

    .. method:: IsUpgradable(pkg)

        Return ``1`` if the package is upgradable.

        The package can be upgraded by calling :meth:`pkgDepCache.MarkInstall`.

    .. method:: IsNowBroken(pkg)

        Return `1` if the package is broken now (including changes made, but
        not committed).

    .. method:: IsInstBroken(pkg)

        Return ``1`` if the package is broken on the current install. This
        takes changes which have not been committed not into effect.

    .. method:: IsGarbage(pkg)

        Return ``1`` if the package is garbage, ie. if it is automatically
        installed and no longer referenced by other packages.

    .. method:: IsAutoInstalled(pkg)

        Return ``1``  if the package is automatically installed (eg. as the
        dependency of another package).

    .. method:: MarkedInstall(pkg)

        Return ``1`` if the package is marked for install.

    .. method:: MarkedUpgrade(pkg)

        Return ``1`` if the package is marked for upgrade.

    .. method:: MarkedDelete(pkg)

        Return ``1`` if the package is marked for delete.

    .. method:: MarkedKeep(pkg)

        Return ``1`` if the package is marked for keep.

    .. method:: MarkedReinstall(pkg)

        Return ``1`` if the package should be installed.

    .. method:: MarkedDowngrade(pkg)

        Return ``1`` if the package should be downgraded.

    .. attribute:: KeepCount

        Integer, number of packages marked as keep

    .. attribute:: InstCount

        Integer, number of packages marked for installation.

    .. attribute:: DelCount

        Number of packages which should be removed.

    .. attribute:: BrokenCount

        Number of packages which are broken.

    .. attribute:: UsrSize

        The size required for the changes on the filesystem. If you install
        packages, this is positive, if you remove them its negative.

    .. attribute:: DebSize

        The size of the packages which are needed for the changes to be
        applied.


:class:`MetaIndex`
------------------

.. todo::

    Complete them

.. class:: MetaIndex

    .. attribute:: URI
    .. attribute:: Dist
    .. attribute:: IsTrusted
    .. attribute:: IndexFiles


:class:`PackageIndexFile`
-------------------------

.. class:: PackageIndexFile

    .. method:: ArchiveURI(path)

        Return the full url to path in the archive.

    .. attribute:: Label

        Return the Label.

    .. attribute:: Exists

        Return whether the file exists.

    .. attribute:: HasPackages

        Return whether the file has packages.

    .. attribute:: Size

        Size of the file

    .. attribute:: IsTrusted

        Whether we can trust the file.


:class:`PkgManager`
-------------------

.. class:: PkgManager

    Class, as returned by :func:`apt_pkg.GetPackageManager`.

    .. method:: GetArchives(fetcher, list, records)

        Add all the selected packages to the :class:`Acquire()` object
        *fetcher*.

        The parameter *list* refers to a :class:`PkgSourceList()` object, as
        returned by :func:`apt_pkg.GetPkgSourceList`.

        The parameter *records* refers to a :class:`pkgRecords()` object, as
        returned by :func:`apt_pkg.GetPkgRecords`.

    .. method:: DoInstall()

        Install the packages.

    .. method:: FixMissing

        Fix the installation if a package could not be downloaded.

    .. attribute:: ResultCompleted

        A constant for checking whether the the result is 'completed'.

        Compare it against the return value of :meth:`PkgManager.GetArchives`
        or :meth:`PkgManager.DoInstall`.

    .. attribute:: ResultFailed

        A constant for checking whether the the result is 'failed'.

        Compare it against the return value of :meth:`PkgManager.GetArchives`
        or :meth:`PkgManager.DoInstall`.

    .. attribute:: ResultIncomplete

        A constant for checking whether the the result is 'incomplete'.

        Compare it against the return value of :meth:`PkgManager.GetArchives`
        or :meth:`PkgManager.DoInstall`.

:class:`pkgRecords`
--------------------

.. class:: PkgRecords

    Provide access to the packages records. This provides very useful
    attributes for fast (convient) access to some fields of the record.

    See :func:`apt_pkg.GetPkgRecords` for initialization.


    .. method:: Lookup(verfile_iter)

        Change the actual package to the package given by the verfile_iter.

        The parameter *verfile_iter* refers to a tuple consisting
        of (:class:`PackageFile()`, int: index), as returned by various
        attributes, including :attr:`Version.FileList`.

        Example (shortened)::

            cand = depcache.GetCandidateVer(cache['python-apt'])
            records.Lookup(cand.FileList[0])
            # Now you can access the record
            print records.SourcePkg # == python-apt

    .. attribute:: FileName

        Return the field 'Filename' of the record. This is the path to the
        package, relative to the base path of the archive.

    .. attribute:: MD5Hash

        Return the MD5 hashsum of the package This refers to the field
        'MD5Sum' in the raw record.

    .. attribute:: SHA1Hash

        Return the SHA1 hashsum of the package. This refers to the field 'SHA1'
        in the raw record.

    .. attribute:: SHA256Hash

        Return the SHA256 hashsum of the package. This refers to the field
        'SHA256' in the raw record.

        .. versionadded:: 0.7.9

    .. attribute:: SourcePkg

        Return the source package.

    .. attribute:: SourceVer

        Return the source version.

    .. attribute:: Maintainer

        Return the maintainer of the package.

    .. attribute:: ShortDesc

        Return the short description. This is the summary on the first line of
        the 'Description' field.

    .. attribute:: LongDesc

        Return the long description. These are lines 2-END from the
        'Description' field.

    .. attribute:: Name

        Return the name of the package. This is the 'Package' field.

    .. attribute:: Homepage

        Return the Homepage. This is the 'Homepage' field.

    .. attribute:: Record

        Return the whole record as a string. If you want to access fields of
        the record not available as an attribute, you can use
        :func:`apt_pkg.ParseSection` to parse the record and access the field
        name.

        Example::

            section = apt_pkg.ParseSection(records.Record)
            print section['SHA256']

:class:`PkgSrcRecords`
----------------------

.. class:: PkgSrcRecords

    This represents the entries in the Sources files, ie. the dsc files of
    the source packages.

    .. note::

        If the Lookup failed, because no package could be found, no error is
        raised. Instead, the attributes listed below are simply not existing
        anymore (same applies when no Lookup has been made, or when it has
        been restarted).

    .. method:: Lookup(pkgname)

        Lookup the record for the package named *pkgname*. To access all
        available records, you need to call it multiple times.

        Imagine a package P with two versions X, Y. The first ``Lookup(P)``
        would set the record to version X and the second ``Lookup(P)`` to
        version Y.

    .. method:: Restart()

        Restart the lookup.

        Imagine a package P with two versions X, Y. The first ``Lookup(P)``
        would set the record to version X and the second ``Lookup(P)`` to
        version Y.

        If you now call ``Restart()``, the internal position will be cleared.
        Now you can call ``Lookup(P)`` again to move to X.

    .. attribute:: Package

        The name of the source package.

    .. attribute:: Version

        A string describing the version of the source package.

    .. attribute:: Maintainer

        A string describing the name of the maintainer.

    .. attribute:: Section

        A string describing the section.

    .. attribute:: Record

        The whole record, as a string. You can use :func:`apt_pkg.ParseSection`
        if you need to parse it.

        You need to parse the record if you want to access fields not available
        via the attributes, eg. 'Standards-Version'

    .. attribute:: Binaries

        Return a list of strings describing the package names of the binaries
        created by the source package. This matches the 'Binary' field in the
        raw record.

    .. attribute:: Index

        The index in the Sources files.

    .. attribute:: Files

        The list of files. This returns a list of tuples with the contents
        ``(str: md5, int: size, str: path, str:type)``.

    .. attribute:: BuildDepends

        Return the list of Build dependencies, as
        ``(str: package, str: version, int: op, int: type)``.

        .. table:: Values of *op*

            ===== =============================================
            Value      Meaning
            ===== =============================================
            0x0   No Operation (no versioned build dependency)
            0x10  | (or) - this will be added to the other values
            0x1   <= (less than or equal)
            0x2   >= (greater than or equal)
            0x3   << (less than)
            0x4   >> (greater than)
            0x5   == (equal)
            0x6   != (not equal)
            ===== =============================================

        .. table:: Values of *type*

            ===== ===================
            Value Meaning
            ===== ===================
            0     Build-Depends
            1     Build-Depends-Indep
            2     Build-Conflicts
            3     Build-Conflicts-Indep
            ===== ===================

        **Example**: In the following content, we will imagine a
        build-dependency::

            Build-Depends: A (>= 1) | B (>= 1), C

        This results in::

            [('A', '1', 18, 0), # 18 = 16 + 2 = 0x10 + 0x2
             ('B', '1', 2, 0),
             ('C', '', 0, 0)]

        This is **not** the same as returned by
        :func:`apt_pkg.ParseSrcDepends`.


:class:`PkgSourceList`
-----------------------

.. class:: PkgSourceList

    This is for :file:`/etc/apt/sources.list`.

    .. method:: FindIndex(pkgfile)

        Return a :class:`PackageIndexFile` object for the :class:`PackageFile`
        *pkgfile*.

    .. method:: ReadMainList

        Read the main list.

    .. method:: GetIndexes(acq[, all])

        Add the index files to the :class:`Acquire()` object *acq*. If *all* is
        given and ``True``, all files are fetched.


:class:`ProblemResolver`
------------------------
.. class:: ProblemResolver

    The problem resolver helps when there are problems in the package
    selection. An example is a package which conflicts with another, already
    installed package.

    .. method:: Protect(pkg)

        Protect the :class:`Package()` object given by the parameter *pkg*.

        .. todo::

            Really document it.

    .. method:: InstallProtect()

        Protect all installed packages from being removed.

    .. method:: Remove(pkg)

        Remove the :class:`Package()` object given by the parameter *pkg*.

        .. todo::

            Really document it.

    .. method:: Clear(pkg)

        Reset the :class:`Package()` *pkg* to the default state.

        .. todo::

            Really document it.

    .. method:: Resolve()

        Try to resolve problems by installing and removing packages.

    .. method:: ResolveByKeep()

        Try to resolve problems only by using keep.


:class:`TagFile`
----------------

.. class:: TagFile

    An object which represents a typical debian control file. Can be used for
    Packages, Sources, control, Release, etc.

    Use :func:`apt_pkg.ParseTagFile` to parse a file.

    .. method:: Step

        Step forward to the next section. This simply returns ``1`` if OK, and
        ``0`` if there is no section

    .. method:: Offset

        Return the current offset (in bytes) from the beginning of the file.

    .. method:: Jump(offset)

        Jump back/forward to *offset*. Use ``Jump(0)`` to jump to the
        beginning of the file again.

    .. attribute:: Section

        This is the current :class:`TagSection()` instance.

:class:`TagSection`
-------------------

.. class:: TagSection

    Represent a single section of a debian control file.

    .. describe:: section[key]

        Return the value of the field at *key*. If *key* is not available,
        raise :exc:`KeyError`.

    .. method:: Bytes

        The number of bytes in the section.

    .. method:: Find(key, default='')

        Return the value of the field at the key *key* if available,
        else return *default*.

    .. method:: FindFlag(key)

        Find a yes/no value for the key *key*. An example for such a
        field is 'Essential'.

    .. method:: get(key, default='')

        Return the value of the field at the key *key* if available, else
        return *default*.

    .. method:: has_key(key)

        Check whether the field with named by *key* exists.

    .. method:: keys()

        Return a list of keys in the section.
