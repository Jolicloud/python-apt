:mod:`apt_pkg` --- The low-level bindings for apt-pkg
=====================================================
.. module:: apt_pkg

The apt_pkg extensions provides a more low-level way to work with apt. It can
do everything apt can, and is written in C++. It has been in python-apt since
the beginning.

Module Initialization
---------------------

Initialization is needed for most functions, but not for all of them. Some can
be called without having run init*(), but will not return the expected value.

.. function:: init_config

    Initialize the configuration of apt. This is needed for most operations.

.. function:: init_system

    Initialize the system.

.. function:: init

    A short cut to calling :func:`init_config` and :func:`init_system`. You
    can use this if you do not use the command line parsing facilities provided
    by :func:`parse_commandline`, otherwise call :func:`init_config`, parse
    the commandline afterwards and finally call :func:`init_system`.


Working with the cache
----------------------
.. class:: Cache([progress])

    Return a :class:`Cache()` object. The optional parameter *progress*
    specifies an instance of :class:`apt.progress.OpProgress()` which will
    display the open progress.

    .. describe:: cache[pkgname]

        Return the :class:`Package()` object for the package name given by
        *pkgname*.

    .. describe:: pkgname in cache

        Check whether a package with the name given by *pkgname* exists in
        the cache.

    .. method:: update(progress, list[, pulse_interval])

        Update the package cache.

        The parameter *progress* points to an :class:`apt.progress.FetchProgress()`
        object. The parameter *list* refers to a :class:`SourceList()` object.

        The optional parameter *pulse_interval* describes the interval between
        the calls to the :meth:`FetchProgress.pulse` method.

    .. attribute:: depends_count

        The total number of dependencies.

    .. attribute:: package_count

        The total number of packages available in the cache.

    .. attribute:: packages

        A sequence of :class:`Package` objects.

    .. attribute:: provides_count

        The number of provided packages.

    .. attribute:: ver_file_count

        .. todo:: Seems to be some mixture of versions and pkgFile.

    .. attribute:: version_count

        The total number of package versions available in the cache.

    .. attribute:: package_file_count

        The total number of Packages files available (the Packages files
        listing the packages). This is the same as the length of the list in
        the attribute :attr:`file_list`.

    .. attribute:: file_list

        A list of :class:`PackageFile` objects.

.. class:: DepCache(cache)

    Return a :class:`DepCache` object. The parameter *cache* specifies an
    instance of :class:`Cache`.

    The DepCache object contains various methods to manipulate the cache,
    to install packages, to remove them, and much more.

    .. method:: commit(fprogress, iprogress)

        Apply all the changes made.

        The parameter *fprogress* has to be set to an instance of
        apt.progress.FetchProgress or one of its subclasses.

        The parameter *iprogress* has to be set to an instance of
        apt.progress.InstallProgress or one of its subclasses.

    .. method:: fix_broken()

        Try to fix all broken packages in the cache.

    .. method:: get_candidate_ver(pkg)

        Return the candidate version of the package, ie. the version that
        would be installed normally.

        The parameter *pkg* refers to an :class:`Package` object,
        available using the :class:`pkgCache`.

        This method returns a :class:`Version` object.

    .. method:: set_candidate_ver(pkg, version)

        The opposite of :meth:`pkgDepCache.get_candidate_ver`. Set the candidate
        version of the :class:`Package` *pkg* to the :class:`Version`
        *version*.

    .. method:: upgrade([dist_upgrade=False])

        Perform an upgrade. More detailed, this marks all the upgradable
        packages for upgrade. You still need to call
        :meth:`pkgDepCache.commit` for the changes to apply.

        To perform a dist-upgrade, the optional parameter *dist_upgrade* has
        to be set to True.

    .. method:: fix_broken()

        Fix broken packages.

    .. method:: read_pinfile()

        Read the policy, eg. /etc/apt/preferences.

    .. method:: minimize_upgrade()

        Go over the entire set of packages and try to keep each package marked
        for upgrade. If a conflict is generated then the package is restored.

        .. todo::
            Explain better..

    .. method:: mark_auto(pkg)

        Mark the :class:`Package` *pkg* as automatically installed.

    .. method:: mark_keep(pkg)

        Mark the :class:`Package` *pkg* for keep.

    .. method:: mark_delete(pkg[, purge])

        Mark the :class:`Package` *pkg* for delete. If *purge* is True,
        the configuration files will be removed as well.

    .. method:: mark_install(pkg[, auto_inst=True[, from_user=True]])

        Mark the :class:`Package` *pkg* for install.

        If *auto_inst* is ``True``, the dependencies of the package will be
        installed as well. This is the default.

        If *from_user* is ``True``, the package will be marked as manually
        installed. This is the default.

    .. method:: set_reinstall(pkg)

        Set if the :class:`Package` *pkg* should be reinstalled.

    .. method:: is_upgradable(pkg)

        Return ``1`` if the package is upgradable.

        The package can be upgraded by calling :meth:`pkgDepCache.MarkInstall`.

    .. method:: is_now_broken(pkg)

        Return `1` if the package is broken now (including changes made, but
        not committed).

    .. method:: is_inst_broken(pkg)

        Return ``1`` if the package is broken on the current install. This
        takes changes which have not been committed not into effect.

    .. method:: is_garbage(pkg)

        Return ``1`` if the package is garbage, ie. if it is automatically
        installed and no longer referenced by other packages.

    .. method:: is_auto_installed(pkg)

        Return ``1``  if the package is automatically installed (eg. as the
        dependency of another package).

    .. method:: marked_install(pkg)

        Return ``1`` if the package is marked for install.

    .. method:: marked_upgrade(pkg)

        Return ``1`` if the package is marked for upgrade.

    .. method:: marked_delete(pkg)

        Return ``1`` if the package is marked for delete.

    .. method:: marked_keep(pkg)

        Return ``1`` if the package is marked for keep.

    .. method:: marked_reinstall(pkg)

        Return ``1`` if the package should be installed.

    .. method:: marked_downgrade(pkg)

        Return ``1`` if the package should be downgraded.

    .. attribute:: keep_count

        Integer, number of packages marked as keep

    .. attribute:: inst_count

        Integer, number of packages marked for installation.

    .. attribute:: del_count

        Number of packages which should be removed.

    .. attribute:: broken_count

        Number of packages which are broken.

    .. attribute:: usr_size

        The size required for the changes on the filesystem. If you install
        packages, this is positive, if you remove them its negative.

    .. attribute:: deb_size

        The size of the packages which are needed for the changes to be
        applied.

    .. attribute:: policy

        The underlying :class:`Policy` object used by the :class:`DepCache` to
        select candidate versions.


.. class:: PackageManager(depcache)

    Return a new :class:`PackageManager` object. The parameter *depcache*
    specifies a :class:`DepCache` object.

    :class:`PackageManager` objects provide several methods and attributes,
    which will be listed here:

    .. method:: get_archives(fetcher, list, records)

        Add all the selected packages to the :class:`Acquire()` object
        *fetcher*.

        The parameter *list* refers to a :class:`SourceList()` object.

        The parameter *records* refers to a :class:`PackageRecords()` object.

    .. method:: do_install()

        Install the packages.

    .. method:: fix_missing

        Fix the installation if a package could not be downloaded.

    .. attribute:: RESULT_COMPLETED

        A constant for checking whether the the result is 'completed'.

        Compare it against the return value of :meth:`PackageManager.get_archives`
        or :meth:`PackageManager.do_install`.

    .. attribute:: RESULT_FAILED

        A constant for checking whether the the result is 'failed'.

        Compare it against the return value of :meth:`PackageManager.get_archives`
        or :meth:`PackageManager.do_install`.

    .. attribute:: RESULT_INCOMPLETE

        A constant for checking whether the the result is 'incomplete'.

        Compare it against the return value of :meth:`PackageManager.get_archives`
        or :meth:`PackageManager.do_install`.

Improve performance with :class:`ActionGroup`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. class:: ActionGroup(depcache)

    Create a new :class:`ActionGroup()` object for the :class:`DepCache` object
    given by the parameter *depcache*.

    :class:`ActionGroup()` objects make operations on the cache faster by
    delaying certain cleanup operations until the action group is released.

    ActionGroup is also a context manager and therefore supports the
    :keyword:`with` statement. But because it becomes active as soon as it
    is created, you should not create an ActionGroup() object before entering
    the with statement.

    If you want to use ActionGroup as a with statement (which is recommended
    because it makes it easier to see when an actiongroup is active), always
    use the following form::

        with apt_pkg.ActionGroup(depcache):
            ...

    For code which has to run on Python versions prior to 2.5, you can also
    use the traditional way::

        actiongroup = apt_pkg.ActionGroup(depcache)
        ...
        actiongroup.release()

    :class:`ActionGroup` provides the following method:

    .. method:: release()

        Release the ActionGroup. This will reactive the collection of package
        garbage.

Resolving Dependencies
^^^^^^^^^^^^^^^^^^^^^^

.. class:: ProblemResolver(depcache)

    Return a new :class:`ProblemResolver` object. The parameter *depcache*
    specifies a :class:`pDepCache` object.

    The problem resolver helps when there are problems in the package
    selection. An example is a package which conflicts with another, already
    installed package.

    .. method:: protect(pkg)

        Protect the :class:`Package()` object given by the parameter *pkg*.

        .. todo::

            Really document it.

    .. method:: install_protect()

        Protect all installed packages from being removed.

    .. method:: remove(pkg)

        Remove the :class:`Package()` object given by the parameter *pkg*.

        .. todo::

            Really document it.

    .. method:: clear(pkg)

        Reset the :class:`Package()` *pkg* to the default state.

        .. todo::

            Really document it.

    .. method:: resolve()

        Try to resolve problems by installing and removing packages.

    .. method:: resolve_by_keep()

        Try to resolve problems only by using keep.


:class:`Package`
^^^^^^^^^^^^^^^^^
.. class:: Package

    The pkgCache::Package objects are an interface to package specific
    features.


    Attributes:

    .. attribute:: current_ver

        The version currently installed, or None. This returns a
        :class:`Version` object.

    .. attribute:: id

        The ID of the package. This can be used to store information about
        the package. The ID is an int value.

    .. attribute:: name

        This is the name of the package.

    .. attribute:: provides_list

        A list of packages providing this package. More detailed, this is a
        list of tuples (str:pkgname, ????, :class:`Version`).

        If you want to check for check for virtual packages, the expression
        ``pkg.provides_list and not pkg._version_list`` helps you. It detects if
        the package is provided by something else and is not available as a
        real package.

    .. attribute:: rev_depends_list

        An iterator of :class:`Dependency` objects for dependencies on this
        package.

    .. attribute:: section

        The section of the package, as specified in the record. The list of
        possible sections is defined in the Policy.

    .. attribute:: version_list

        A list of :class:`Version` objects for all versions available in the
        cache.

    **States**:

    .. attribute:: selected_state

        The state we want it to be, ie. if you mark a package for installation,
        this is :attr:`apt_pkg.SELSTATE_INSTALL`.

        See :ref:`SelStates` for a list of available states.

    .. attribute:: inst_state

        The state the currently installed version is in. This is normally
        :attr:`apt_pkg.INSTSTATE_OK`, unless the installation failed.

        See :ref:`InstStates` for a list of available states.

    .. attribute:: current_state

        The current state of the package (not installed, unpacked, installed,
        etc). See :ref:`CurStates` for a list of available states.

    **Flags**:

    .. attribute:: auto

        Whether the package was installed automatically as a dependency of
        another package. (or marked otherwise as automatically installed)

    .. attribute:: essential

        Whether the package is essential.

    .. attribute:: important

        Whether the package is important.

Example:
~~~~~~~~~
.. literalinclude:: ../examples/cache-packages.py



:class:`Version`
^^^^^^^^^^^^^^^^^
.. class:: Version

    The version object contains all information related to a specific package
    version.

    .. attribute:: ver_str

        The version, as a string.

    .. attribute:: section

        The usual sections (eg. admin, net, etc.). Prefixed with the component
        name for packages not in main (eg. non-free/admin).

    .. attribute:: arch

        The architecture of the package, eg. amd64 or all.

    .. attribute:: file_list

        A list of (:class:`PackageFile`, int: index) tuples for all Package
        files containing this version of the package.

    .. attribute:: depends_list_str

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

    .. attribute:: depends_list

        This is basically the same as :attr:`Version.DependsListStr`,
        but instead of the ('pkgname', 'version', 'relation') tuples,
        it returns :class:`Dependency` objects, which can assist you with
        useful functions.

    .. attribute:: parent_pkg

        The :class:`Package` object this version belongs to.

    .. attribute:: provides_list

        This returns a list of all packages provided by this version. Like
        :attr:`Package.provides_list`, it returns a list of tuples
        of the form ('virtualpkgname', ???, :class:`Version`), where as the
        last item is the same as the object itself.

    .. attribute:: size

        The size of the .deb file, in bytes.

    .. attribute:: installed_size

        The size of the package (in kilobytes), when unpacked on the disk.

    .. attribute:: hash

        An integer hash value.

    .. attribute:: id

        An integer id.

    .. attribute:: priority

        The integer representation of the priority. This can be used to speed
        up comparisons a lot, compared to :attr:`Version.priority_str`.

        The values are defined in the :mod:`apt_pkg` extension, see
        :ref:`Priorities` for more information.

    .. attribute:: priority_str

        Return the priority of the package version, as a string, eg.
        "optional".

    .. attribute:: downloadable

        Whether this package can be downloaded from a remote site.

    .. attribute:: translated_description

        Return a :class:`Description` object.


:class:`Dependency`
^^^^^^^^^^^^^^^^^^^^
.. class:: Dependency

    Represent a dependency from one package to another one.

    .. method:: all_targets

        A list of :class:`Version` objects which satisfy the dependency,
        and do not conflict with already installed ones.

        From my experience, if you use this method to select the target
        version, it is the best to select the last item unless any of the
        other candidates is already installed. This leads to results being
        very close to the normal package installation.

    .. method:: smart_target_pkg

        Return a :class:`Version` object of a package which satisfies the
        dependency and does not conflict with installed packages
        (the 'natural target').

    .. attribute:: target_ver

        The target version of the dependency, as string. Empty string if the
        dependency is not versioned.

    .. attribute:: target_pkg

        The :class:`Package` object of the target package.

    .. attribute:: parent_ver

        The :class:`Version` object of the parent version, ie. the package
        which declares the dependency.

    .. attribute:: parent_pkg

        The :class:`Package` object of the package which declares the
        dependency. This is the same as using ParentVer.ParentPkg.

    .. attribute:: comp_type

        The type of comparison (>=, ==, >>, <=), as string.

    .. attribute:: dep_type

        The type of the dependency, as string, eg. "Depends".

    .. attribute:: dep_type_enum

        The type of the dependency, as an integer which can be compared to
        one of the TYPE_* constants below.

    .. attribute:: dep_type_untranslated

        The type of the depndency, as an untranslated string.

    .. attribute:: id

        The ID of the package, as integer.

    .. attribute:: TYPE_CONFLICTS

        Constant for checking against dep_type_enum

    .. attribute:: TYPE_DEPENDS

        Constant for checking against dep_type_enum

    .. attribute:: TYPE_DPKG_BREAKS

        Constant for checking against dep_type_enum

    .. attribute:: TYPE_ENHANCES

        Constant for checking against dep_type_enum

    .. attribute:: TYPE_OBSOLETES

        Constant for checking against dep_type_enum

    .. attribute:: TYPE_PREDEPENDS

        Constant for checking against dep_type_enum
        
    .. attribute:: TYPE_RECOMMENDS

        Constant for checking against dep_type_enum
        
    .. attribute:: TYPE_REPLACES

        Constant for checking against dep_type_enum
        
    .. attribute:: TYPE_SUGGESTS

        Constant for checking against dep_type_enum

Example: Find all missing dependencies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
With the help of Dependency.AllTargets(), you can easily find all packages with
broken dependencies:

.. literalinclude:: ../examples/missing-deps.py


:class:`Description`
^^^^^^^^^^^^^^^^^^^^^
.. class:: Description

    Represent the description of the package.

    .. attribute:: language_code

        The language code of the description

    .. attribute:: md5

        The md5 hashsum of the description

    .. attribute:: file_list

        A list of tuples (:class:`PackageFile`, int: index).



Index Files
-------------


.. todo::

    Complete them

.. class:: MetaIndex

    .. attribute:: uri
    .. attribute:: dist
    .. attribute:: is_trusted
    .. attribute:: index_files


.. class:: IndexFile

    .. method:: archive_uri(path)

        Return the full url to path in the archive.

    .. attribute:: label

        Return the Label.

    .. attribute:: describe

        A description of the :class:`IndexFile`.

    .. attribute:: exists

        Return whether the file exists.

    .. attribute:: has_packages

        Return whether the file has packages.

    .. attribute:: size

        Size of the file

    .. attribute:: is_trusted

        Whether we can trust the file.


.. class:: PackageFile

    A :class:`PackageFile` represents a Packages file, eg.
    /var/lib/dpkg/status.

    .. attribute:: architecture

        The architecture of the package file.

    .. attribute:: archive

        The archive (eg. unstable)

    .. attribute:: component

        The component (eg. main)

    .. attribute:: filename

        The name of the file.

    .. attribute:: id

        The ID of the package. This is an integer which can be used to store
        further information about the file [eg. as dictionary key].

    .. attribute:: index_type

        The sort of the index file. In normal cases, this is
        'Debian Package Index'.

    .. attribute:: label

        The Label, as set in the Release file

    .. attribute:: not_automatic

        Whether packages from this list will be updated automatically. The
        default for eg. example is 0 (aka false).

    .. attribute:: not_source

        Whether the file has no source from which it can be updated. In such a
        case, the value is 1; else 0. /var/lib/dpkg/status is 0 for example.

        Example::

            for pkgfile in cache.file_list:
                if pkgfile.not_source:
                    print 'The file %s has no source.' % pkgfile.filename

    .. attribute:: origin

        The Origin, as set in the Release file

    .. attribute:: site

        The hostname of the site.

    .. attribute:: size

        The size of the file.

    .. attribute:: version

        The version, as set in the release file (eg. "4.0" for "Etch")



The following example shows how to use PackageFile:

.. literalinclude:: ../examples/cache-pkgfile.py


Records
--------

.. class:: PackageRecords(cache)

    Create a new :class:`PackageRecords` object, for the packages in the cache
    specified by the parameter *cache*.

    Provide access to the packages records. This provides very useful
    attributes for fast (convient) access to some fields of the record.

    .. method:: lookup(verfile_iter)

        Change the actual package to the package given by the verfile_iter.

        The parameter *verfile_iter* refers to a tuple consisting
        of (:class:`PackageFile()`, int: index), as returned by various
        attributes, including :attr:`Version.file_list`.

        Example (shortened)::

            cand = depcache.GetCandidateVer(cache['python-apt'])
            records.Lookup(cand.FileList[0])
            # Now you can access the record
            print records.SourcePkg # == python-apt

    .. attribute:: filename

        Return the field 'Filename' of the record. This is the path to the
        package, relative to the base path of the archive.

    .. attribute:: md5_hash

        Return the MD5 hashsum of the package This refers to the field
        'MD5Sum' in the raw record.

    .. attribute:: sha1_hash

        Return the SHA1 hashsum of the package. This refers to the field 'SHA1'
        in the raw record.

    .. attribute:: sha256_hash

        Return the SHA256 hashsum of the package. This refers to the field
        'SHA256' in the raw record.

        .. versionadded:: 0.7.9

    .. attribute:: source_pkg

        Return the source package.

    .. attribute:: source_ver

        Return the source version.

    .. attribute:: maintainer

        Return the maintainer of the package.

    .. attribute:: short_desc

        Return the short description. This is the summary on the first line of
        the 'Description' field.

    .. attribute:: long_desc

        Return the long description. These are lines 2-END from the
        'Description' field.

    .. attribute:: name

        Return the name of the package. This is the 'Package' field.

    .. attribute:: homepage

        Return the Homepage. This is the 'Homepage' field.

    .. attribute:: record

        Return the whole record as a string. If you want to access fields of
        the record not available as an attribute, you can use
        :class:`apt_pkg.TagSection` to parse the record and access the field
        name.

        Example::

            section = apt_pkg.TagSection(records.record)
            print section['SHA256'] # Use records.sha256_hash instead


.. class:: SourceRecords

    This represents the entries in the Sources files, ie. the dsc files of
    the source packages.

    .. note::

        If the Lookup failed, because no package could be found, no error is
        raised. Instead, the attributes listed below are simply not existing
        anymore (same applies when no Lookup has been made, or when it has
        been restarted).

    .. method:: lookup(pkgname)

        Lookup the record for the package named *pkgname*. To access all
        available records, you need to call it multiple times.

        Imagine a package P with two versions X, Y. The first ``lookup(P)``
        would set the record to version X and the second ``lookup(P)`` to
        version Y.

    .. method:: restart()

        Restart the lookup.

        Imagine a package P with two versions X, Y. The first ``Lookup(P)``
        would set the record to version X and the second ``Lookup(P)`` to
        version Y.

        If you now call ``restart()``, the internal position will be cleared.
        Now you can call ``lookup(P)`` again to move to X.

    .. attribute:: package

        The name of the source package.

    .. attribute:: version

        A string describing the version of the source package.

    .. attribute:: maintainer

        A string describing the name of the maintainer.

    .. attribute:: section

        A string describing the section.

    .. attribute:: record

        The whole record, as a string. You can use :func:`apt_pkg.ParseSection`
        if you need to parse it.

        You need to parse the record if you want to access fields not available
        via the attributes, eg. 'Standards-Version'

    .. attribute:: binaries

        Return a list of strings describing the package names of the binaries
        created by the source package. This matches the 'Binary' field in the
        raw record.

    .. attribute:: index

        The index in the Sources files.

    .. attribute:: files

        The list of files. This returns a list of tuples with the contents
        ``(str: md5, int: size, str: path, str:type)``.

    .. attribute:: build_depends

        Return a dictionary representing the build-time dependencies of the
        package. The format is the same as for :attr:`Version.depends_list_str`
        and possible keys being ``"Build-Depends"``, ``"Build-Depends-Indep"``,
        ``"Build-Conflicts"`` or ``"Build-Conflicts-Indep"``.


The Acquire interface
----------------------
The Acquire Interface is responsible for all sorts of downloading in apt. All
packages, index files, etc. downloading is done using the Acquire functionality.

The :mod:`apt_pkg` module provides a subset of this functionality which allows
you to implement file downloading in your applications. Together with the
:class:`PackageManager` class you can also fetch all the packages marked for
installation.


.. class:: Acquire([progress])

    Return an :class:`Acquire` object. The parameter *progress* refers to
    an :class:`apt.progress.FetchProgress()` object.

    Acquire objects maintaing a list of items which will be fetched or have
    been fetched already during the lifetime of this object. To add new items
    to this list, you can create new :class:`AcquireFile` objects which allow
    you to add single files.

    Acquire items have multiple methods and attributes:

    .. method:: run()

        Fetch all the items which have been added by :class:`AcquireFile`.

    .. method:: shutdown()

        Shut the fetcher down.

    .. attribute:: total_needed

        The total amount of bytes needed (including those of files which are
        already present)

    .. attribute:: fetch_needed

        The total amount of bytes which need to be fetched.

    .. attribute:: partial_present

        Whether some files have been acquired already. (???)

    .. attribute:: items

        A list of :class:`AcquireItem` objects which are attached to the
        queue of this object.

    .. attribute:: workers

        A list of :class:`AcquireWorker` objects which are currently active
        on this instance.

.. class:: AcquireItem

    The :class:`AcquireItem()` objects represent the items of a
    :class:`Acquire` object. :class:`AcquireItem()` objects can not be created
    by the user, they are solely available through the :attr:`Acquire.items`
    list of an :class:`Acquire` object.

    .. attribute:: id

        The ID of the item.

    .. attribute:: complete

        Is the item completely acquired?

    .. attribute:: local

        Is the item a local file?

    .. attribute:: mode

        A string indicating the current mode e.g. ``"Fetching"``.

    .. attribute:: is_trusted

        Can the file be trusted?

    .. attribute:: filesize

        The size of the file, in bytes.

    .. attribute:: error_text

        The error message. For example, when a file does not exist on a http
        server, this will contain a 404 error message.

    .. attribute:: destfile

        The location the file is saved as.

    .. attribute:: desc_uri

        The source location.

    **Status**:

    .. attribute:: status

        Integer, representing the status of the item.

    .. attribute:: STAT_IDLE

        Constant for comparing :attr:`AcquireItem.status`.

    .. attribute:: STAT_FETCHING

        Constant for comparing :attr:`AcquireItem.status`

    .. attribute:: STAT_DONE

        Constant for comparing :attr:`AcquireItem.status`

    .. attribute:: STAT_ERROR

        Constant for comparing :attr:`AcquireItem.status`

    .. attribute:: STAT_AUTH_ERROR

        Constant for comparing :attr:`AcquireItem.status`

.. class:: AcquireFile(owner, uri[, md5, size, descr, short_descr, destdir, destfile])

    Create a new :class:`AcquireFile()` object and register it with *acquire*,
    so it will be fetched. You must always keep around a reference to the
    object, otherwise it will be removed from the Acquire queue again.

    The parameter *owner* refers to an :class:`Acquire()` object as returned
    by :func:`GetAcquire`. The file will be added to the Acquire queue
    automatically.

    The parameter *uri* refers to the location of the file, any protocol
    of apt is supported.

    The parameter *md5* refers to the md5sum of the file. This can be used
    for checking the file.

    The parameter *size* can be used to specify the size of the package,
    which can then be used to calculate the progress and validate the download.

    The parameter *descr* is a descripition of the download. It may be
    used to describe the item in the progress class. *short_descr* is the
    short form of it.

    You can use *destdir* to manipulate the directory where the file will
    be saved in. Instead of *destdir*, you can also specify the full path to
    the file using the parameter *destfile*. You can not combine both.

    In terms of attributes, this class is a subclass of :class:`AcquireItem`
    and thus inherits all its attributes.

.. class:: AcquireWorker

    An :class:`AcquireWorker` object represents a subprocess responsible for
    fetching files from remote locations. This class is not instanciable from
    Python.

    .. attribute:: current_item

        The item which is currently being fetched. This returns an
        :class:`AcquireItemDesc` object.

    .. attribute:: current_size

        How many bytes of the file have been downloaded. Zero if the current
        progress of the file cannot be determined.

    .. attribute:: resumepoint

        The amount of the file that was already downloaded prior to starting
        this worker.

    .. attribute:: status

        The most recent status string received from the subprocess.

    .. attribute:: total_size

        The total number of bytes to be downloaded. Zero if the total size is
        unknown.

.. class:: AcquireItemDesc

    An :class:`AcquireItemDesc` object stores information about the item which
    can be used to describe the item.

    .. attribute:: description

        The long description given to the item.

    .. attribute:: owner

        The :class:`AcquireItem` object owning this object.

    .. attribute:: shortdesc

        A short description which has been given to this item.

    .. attribute:: uri

        The URI from which to download this item.

.. class:: AcquireProgress

    A monitor object for downloads controlled by the Acquire class. This is
    an mostly abstract class. You should subclass it and implement the
    methods to get something useful.

    Methods defined here:

    .. method:: done(item: AcquireItemDesc)

        Invoked when an item is successfully and completely fetched.

    .. method:: fail(item: AcquireItemDesc)

        Invoked when the process of fetching an item encounters a fatal error.

    .. method:: fetch(item: AcquireItemDesc)

        Invoked when some of an item's data is fetched.

    .. method:: ims_hit(item: AcquireItemDesc)

        Invoked when an item is confirmed to be up-to-date. For instance,
        when an HTTP download is informed that the file on the server was
        not modified.

    .. method:: media_change(media: str, drive: str) -> bool

        Invoked when the user should be prompted to change the inserted
        removable media.

        This method should not return until the user has confirmed to the user
        interface that the media change is complete.

        The parameter *media* is the name of the media type that should be
        changed, the parameter *drive* is the identifying name of the drive
        whose media should be changed.

        Return True if the user confirms the media change, False if it is
        cancelled.

    .. method:: pulse(owner: Acquire) -> bool

        Periodically invoked while the Acquire process is underway.

        Return False if the user asked to cancel the whole Acquire process.

    .. method:: start()

        Invoked when the Acquire process starts running.

    .. method:: stop()

        Invoked when the Acquire process stops running.

    There are also some data descriptors:

    .. attribute:: current_bytes

        The number of bytes fetched.

    .. attribute:: current_cps

        The current rate of download, in bytes per second.

    .. attribute:: current_items

        The number of items that have been successfully downloaded.

    .. attribute:: elapsed_time

        The amount of time that has elapsed since the download started.

    .. attribute:: fetched_bytes

        The total number of bytes accounted for by items that were
        successfully fetched.

    .. attribute:: last_bytes

        The number of bytes fetched as of the previous call to pulse(),
        including local items.

    .. attribute:: total_bytes

        The total number of bytes that need to be fetched. This member is
        inaccurate, as new items might be enqueued while the download is
        in progress!

    .. attribute:: total_items

        The total number of items that need to be fetched. This member is
        inaccurate, as new items might be enqueued while the download is
        in progress!


Hashes
------
The apt_pkg module also provides several hash functions. If you develop
applications with python-apt it is often easier to use these functions instead
of the ones provides in Python's :mod:`hashlib` module.

The module provides the two classes :class:`Hashes` and :class:`HashString` for
generic hash support:

.. class:: Hashes(object)

    Calculate all supported hashes of the object. *object* may either be a
    string, in which cases the hashes of the string are calculated, or a
    :class:`file()` object or file descriptor, in which case the hashes of
    its contents is calculated. The calculated hashes are then available via
    attributes:

    .. attribute:: md5

        The MD5 hash of the data, as string.

    .. attribute:: sha1

        The SHA1 hash of the data, as string.

    .. attribute:: sha256

        The SHA256 hash of the data, as string.

.. class:: HashString(type: str, hash: str)

    HashString objects store the type of a hash and the corresponding hash.
    They are used by e.g :meth:`IndexRecords.lookup`. The first parameter,
    *type* refers to one of MD5Sum, SHA1 and SHA256. The second parameter
    *hash* is the corresponding hash.

    .. describe:: str(hashstring)

        Convert the HashString to a string by joining the hash type and the
        hash using ':', e.g. ``"MD5Sum:d41d8cd98f00b204e9800998ecf8427e"``.

    .. attribute:: hashtype

        The type of the hash. This may be MD5Sum, SHA1 or SHA256.

    .. method:: verify_file(filename: str) -> bool

        Verify that the file given by the parameter *filename* matches the hash
        stored in this object.

The :mod:`apt_pkg` module also provides the functions :func:`md5sum`,
:func:`sha1sum` and :func:`sha256sum` for creating a single hash from a
:class:`bytes` or :class:`file` object:

.. function:: md5sum(object)

    Return the md5sum of the object. *object* may either be a string, in
    which case the md5sum of the string is returned, or a :class:`file()`
    object (or a file descriptor), in which case the md5sum of its contents is
    returned.

    .. versionchanged:: 0.8.0
        Added support for using file descriptors.

.. function:: sha1sum(object)

    Return the sha1sum of the object. *object* may either be a string, in
    which case the sha1sum of the string is returned, or a :class:`file()`
    object (or a file descriptor), in which case the sha1sum of its contents
    is returned.

    .. versionchanged:: 0.8.0
        Added support for using file descriptors.

.. function:: sha256sum(object)

    Return the sha256sum of the object. *object* may either be a string, in
    which case the sha256sum of the string is returned, or a :class:`file()`
    object  (or a file descriptor), in which case the sha256sum of its contents
    is returned.

    .. versionchanged:: 0.8.0
        Added support for using file descriptors.

Debian control files
--------------------
Debian control files are files containing multiple stanzas of :RFC:`822`-style
header sections. They are widely used in the Debian community, and can represent
many kinds of information. One example for such a file is the
:file:`/var/lib/dpkg/status` file which contains a list of the currently
installed packages.

The :mod:`apt_pkg` module provides two classes to read those files and parts
thereof and provides a function :func:`RewriteSection` which takes a
:class:`TagSection()` object and sorting information and outputs a sorted
section as a string.

.. class:: TagFile(file)

    An object which represents a typical debian control file. Can be used for
    Packages, Sources, control, Release, etc. Such an object provides two
    kinds of API which should not be used together:

    The first API implements the iterator protocol and should be used whenever
    possible because it has less side effects than the other one. It may be
    used e.g. with a for loop::

        tagf = apt_pkg.TagFile(open('/var/lib/dpkg/status'))
        for section in tagfile:
            print section['Package']
        
    .. method:: next()

        A TagFile is its own iterator. This method is part of the iterator
        protocol and returns a :class:`TagSection` object for the next
        section in the file. If there is no further section, this method
        raises the :exc:`StopIteration` exception.

        From Python 3 on, this method is not available anymore, and the
        global function ``next()`` replaces it.

    The second API uses a shared :class:`TagSection` object which is exposed
    through the :attr:`section` attribute. This object is modified by calls
    to :meth:`step` and :meth:`jump`. This API provides more control and may
    use less memory, but is not recommended because it works by modifying
    one object. It can be used like this::

        tagf = apt_pkg.TagFile(open('/var/lib/dpkg/status'))
        tagf.step()
        print tagf.section['Package']

    .. method:: step

        Step forward to the next section. This simply returns ``1`` if OK, and
        ``0`` if there is no section.

    .. method:: offset

        Return the current offset (in bytes) from the beginning of the file.

    .. method:: jump(offset)

        Jump back/forward to *offset*. Use ``jump(0)`` to jump to the
        beginning of the file again.

    .. attribute:: section

        This is the current :class:`TagSection()` instance.

.. class:: TagSection(text)

    Represent a single section of a debian control file.

    .. describe:: section[key]

        Return the value of the field at *key*. If *key* is not available,
        raise :exc:`KeyError`.

    .. describe:: key in section

        Return ``True`` if *section* has a key *key*, else ``False``.

      .. versionadded:: 0.8.0

    .. method:: bytes

        The number of bytes in the section.

    .. method:: find(key, default='')

        Return the value of the field at the key *key* if available,
        else return *default*.

    .. method:: find_flag(key)

        Find a yes/no value for the key *key*. An example for such a
        field is 'Essential'.

    .. method:: get(key, default='')

        Return the value of the field at the key *key* if available, else
        return *default*.

    .. method:: keys()

        Return a list of keys in the section.

.. function:: rewrite_section(section: TagSection, order: list, rewrite_list: list) -> str

    Rewrite the section given by *section* using *rewrite_list*, and order the
    fields according to *order*.

    The parameter *order* is a :class:`list` object containing the names of the
    fields in the order they should appear in the rewritten section.
    :data:`apt_pkg.REWRITE_PACKAGE_ORDER` and
    :data:`apt_pkg.REWRITE_SOURCE_ORDER` are two predefined lists for rewriting
    package and source sections, respectively.

    The parameter *rewrite_list* is a list of tuples of the form
    ``(tag, newvalue[, renamed_to])``, whereas *tag* describes the field which
    should be changed, *newvalue* the value which should be inserted or
    ``None`` to delete the field, and the optional *renamed_to* can be used
    to rename the field.

.. data:: REWRITE_PACKAGE_ORDER

    The order in which the information for binary packages should be rewritten,
    i.e. the order in which the fields should appear.

.. data:: REWRITE_SOURCE_ORDER

    The order in which the information for source packages should be rewritten,
    i.e. the order in which the fields should appear.

Dependencies
------------
.. function:: check_dep(pkgver, op, depver)

    Check that the dependency requirements consisting of op and depver can be
    satisfied by the version pkgver.

    Example::

        >>> bool(apt_pkg.check_dep("1.0", ">=", "1"))
        True

The following two functions provide the ability to parse dependencies. They
use the same format as :attr:`Version.depends_list_str`.

.. function:: parse_depends(depends)

    Parse the string *depends* which contains dependency information as
    specified in Debian Policy, Section 7.1.

    Returns a list. The members of this list are lists themselves and contain
    one or more tuples in the format ``(package,version,operation)`` for every
    'or'-option given, e.g.::

        >>> apt_pkg.parse_depends("PkgA (>= VerA) | PkgB (>= VerB)")
        [[('PkgA', 'VerA', '>='), ('PkgB', 'VerB', '>=')]]


    .. note::

        The behavior of this function is different than the behavior of the
        old function :func:`ParseDepends()`, because the third field
        ``operation`` uses `>` instead of `>>` and `<` instead of `<<` which
        is specified in control files.


.. function:: parse_src_depends(depends)

    Parse the string *depends* which contains dependency information as
    specified in Debian Policy, Section 7.1.

    Returns a list. The members of this list are lists themselves and contain
    one or more tuples in the format ``(package,version,operation)`` for every
    'or'-option given, e.g.::

        >>> apt_pkg.parse_depends("PkgA (>= VerA) | PkgB (>= VerB)")
        [[('PkgA', 'VerA', '>='), ('PkgB', 'VerB', '>=')]]


    Furthemore, this function also supports to limit the architectures, as
    used in e.g. Build-Depends::

        >>> apt_pkg.parse_src_depends("a (>= 01) [i386 amd64]")
        [[('a', '01', '>=')]]

    .. note::

        The behavior of this function is different than the behavior of the
        old function :func:`ParseDepends()`, because the third field
        ``operation`` uses `>` instead of `>>` and `<` instead of `<<` which
        is specified in control files.


Configuration
-------------

.. class:: Configuration()

    Configuration() objects store the configuration of apt, mostly created
    from the contents of :file:`/etc/apt.conf` and the files in
    :file:`/etc/apt.conf.d`.

    .. describe:: key in conf

      Return ``True`` if *conf* has a key *key*, else ``False``.

    .. describe:: conf[key]

        Return the value of the option given key *key*. If it does not
        exist, raise :exc:`KeyError`.

    .. describe:: conf[key] = value

        Set the option at *key* to *value*.

    .. method:: find(key[, default=''])

        Return the value for the given key *key*. This is the same as
        :meth:`Configuration.get`.

        If *key* does not exist, return *default*.

    .. method:: find_file(key[, default=''])

        Return the filename hold by the configuration at *key*. This formats the
        filename correctly and supports the Dir:: stuff in the configuration.

        If *key* does not exist, return *default*.

    .. method:: find_dir(key[, default='/'])

        Return the absolute path to the directory specified in *key*. A
        trailing slash is appended.

        If *key* does not exist, return *default*.

    .. method:: find_i(key[, default=0])

        Return the integer value stored at *key*.

        If *key* does not exist, return *default*.

    .. method:: find_b(key[, default=0])

        Return the boolean value stored at *key*. This returns an integer, but
        it should be treated like True/False.

        If *key* does not exist, return *default*.

    .. method:: set(key, value)

        Set the value of *key* to *value*.

    .. method:: exists(key)

        Check whether the key *key* exists in the configuration.

    .. method:: subtree(key)

        Return a sub tree starting at *key*. The resulting object can be used
        like this one.

    .. method:: list([key])

        List all items at *key*. Normally, return the keys at the top level,
        eg. APT, Dir, etc.

        Use *key* to specify a key of which the childs will be returned.

    .. method:: value_list([key])

        Same as :meth:`Configuration.list`, but this time for the values.

    .. method:: my_tag()

        Return the tag name of the current tree. Normally this is an empty
        string, but for subtrees it is the key of the subtree.

    .. method:: clear(key)

        Clear the configuration. Remove all values and keys at *key*.

    .. method:: keys([key])

        Return all the keys, recursive. If *key* is specified, ... (FIXME)

    .. method:: get(key[, default=''])

        This behaves just like :meth:`dict.get` and :meth:`Configuration.find`,
        it returns the value of key or if it does not exist, *default*.

.. data:: config

    A :class:`Configuration()` object with the default configuration. This
    object is initialized by calling :func:`init_config`.


.. function:: read_config_file(configuration, filename)

    Read the configuration file specified by the parameter *filename* and add
    the settings therein to the :class:`Configuration()` object specified by
    the parameter *configuration*

.. function:: read_config_dir(configuration, dirname)

    Read configuration files in the directory specified by the parameter
    *dirname* and add the settings therein to the :class:`Configuration()`
    object specified by the parameter *configuration*.

.. function:: read_config_file_isc(configuration, filename)

    Read the configuration file specified by the parameter *filename* and add
    the settings therein to the :class:`Configuration()` object specified by
    the parameter *configuration*

.. function:: parse_commandline(configuration, options, argv)

    This function is like getopt except it manipulates a configuration space.
    output is a list of non-option arguments (filenames, etc). *options* is a
    list of tuples of the form ``('c',"long-opt or None",
    "Configuration::Variable","optional type")``.

    Where ``type`` may be one of HasArg, IntLevel, Boolean, InvBoolean,
    ConfigFile, or ArbItem. The default is Boolean.

Locking
--------
When working on the global cache, it is important to lock the cache so other
programs do not modify it. This module provides two context managers for
locking the package system or file-based locking.

.. class:: SystemLock

    Context manager for locking the package system. The lock is established
    as soon as the method __enter__() is called. It is released when
    __exit__() is called. If the lock can not be acquired or can not be
    released an exception is raised.

    This should be used via the 'with' statement, e.g.::

        with apt_pkg.SystemLock():
            ... # Do your stuff here.
        ... # Now it's unlocked again

    Once the block is left, the lock is released automatically. The object
    can be used multiple times::

        lock = apt_pkg.SystemLock()
        with lock:
            ...
        with lock:
            ...

.. class:: FileLock(filename: str)

    Context manager for locking using a file. The lock is established
    as soon as the method __enter__() is called. It is released when
    __exit__() is called. If the lock can not be acquired or can not be
    released, an exception is raised.

    This should be used via the 'with' statement, e.g.::

        with apt_pkg.FileLock(filename):
            ...

    Once the block is left, the lock is released automatically. The object
    can be used multiple times::

        lock = apt_pkg.FileLock(filename)
        with lock:
            ...
        with lock:
            ...

For Python versions prior to 2.5, similar functionality is provided by the
following three functions:

.. function:: get_lock(filename, errors=False) -> int

    Create an empty file at the path specified by the parameter *filename* and
    lock it. If this fails and *errors* is **True**, the function raises an
    error. If *errors* is **False**, the function returns -1.

    The lock can be acquired multiple times within the same process, and can be
    released by calling :func:`os.close` on the return value which is the file
    descriptor of the created file.

.. function:: pkgsystem_lock()

    Lock the global pkgsystem. The lock should be released by calling
    :func:`pkgsystem_unlock` again. If this function is called n-times, the
    :func:`pkgsystem_unlock` function must be called n-times as well to release
    all acquired locks.

.. function:: pkgsystem_unlock()

    Unlock the global pkgsystem. This reverts the effect of
    :func:`pkgsystem_unlock`.


Other classes
--------------
.. class:: Cdrom()

    Return a Cdrom object with the following methods:

    .. method:: ident(progress)

        Identify the cdrom. The parameter *progress* refers to an
        :class:`apt.progress.CdromProgress()` object.

    .. method:: add(progress)

        Add the cdrom to the sources.list file. The parameter *progress*
        refers to an :class:`apt.progress.CdromProgress()` object.

.. class:: SourceList

    This is for :file:`/etc/apt/sources.list`.

    .. method:: find_index(pkgfile)

        Return a :class:`IndexFile` object for the :class:`PackageFile`
        *pkgfile*.

    .. method:: read_main_list

        Read the main list.

    .. method:: get_indexes(acq[, all])

        Add the index files to the :class:`Acquire()` object *acq*. If *all* is
        given and ``True``, all files are fetched.

    .. attribute:: list

        A list of :class:`MetaIndex` objects.

String functions
----------------
.. function:: base64_encode(string)

    Encode the given string using base64, e.g::

        >>> apt_pkg.base64_encode(u"A")
        'QQ=='

.. function:: check_domain_list(host, list)

    See if Host is in a ',' separated list, e.g.::

        apt_pkg.check_domain_list("alioth.debian.org","debian.net,debian.org")

.. function:: dequote_string(string)

    Dequote the string specified by the parameter *string*, e.g.::

        >>> apt_pkg.dequote_string("%61%70%74%20is%20cool")
        'apt is cool'

.. function:: quote_string(string, repl)

    For every character listed in the string *repl*, replace all occurences in
    the string *string* with the correct HTTP encoded value:

        >>> apt_pkg.quote_string("apt is cool","apt")
        '%61%70%74%20is%20cool'

.. function:: size_to_str(size)

    Return a string presenting the human-readable version of the integer
    *size*. When calculating the units (k,M,G,etc.) the size is divided by the
    factor 1000.

    Example::

        >>> apt_pkg.size_to_str(10000)
        '10.0k'

.. function:: string_to_bool(input)

    Parse the string *input* and return one of **-1**, **0**, **1**.

    .. table:: Return values

        ===== =============================================
        Value      Meaning
        ===== =============================================
         -1   The string *input* is not recognized.
          0   The string *input* evaluates to **False**.
         +1   The string *input* evaluates to **True**.
        ===== =============================================

    Example::

        >>> apt_pkg.string_to_bool("yes")
        1
        >>> apt_pkg.string_to_bool("no")
        0
        >>> apt_pkg.string_to_bool("not-recognized")
        -1

.. function:: str_to_time(rfc_time)

    Convert the :rfc:`1123` conforming string *rfc_time* to the unix time, and
    return the integer. This is the opposite of :func:`TimeRFC1123`.

    Example::

        >> apt_pkg.str_to_time('Thu, 01 Jan 1970 00:00:00 GMT')
        0

.. function:: time_rfc1123(seconds)

    Format the unix time specified by the integer *seconds*, according to the
    requirements of :rfc:`1123`.

    Example::

        >>> apt_pkg.time_rfc1123(0)
        'Thu, 01 Jan 1970 00:00:00 GMT'


.. function:: time_to_str(seconds)

    Format a given duration in a human-readable manner. The parameter *seconds*
    refers to a number of seconds, given as an integer. The return value is a
    string with a unit like 's' for seconds.

    Example::

        >>> apt_pkg.time_to_str(3601)
        '1h0min1s'

.. function:: upstream_version(version)

    Return the string *version*, eliminating everything following the last
    '-'. Thus, this should be equivalent to ``version.rsplit('-', 1)[0]``.

.. function:: uri_to_filename(uri)

    Take a string *uri* as parameter and return a filename which can be used to
    store the file, based on the URI.

    Example::

        >>> apt_pkg.uri_to_filename('http://debian.org/index.html')
        'debian.org_index.html'


.. function:: version_compare(a, b)

    Compare two versions, *a* and *b*, and return an integer value which has
    the same characteristic as the built-in :func:`cmp` function.

    .. table:: Return values

        ===== =============================================
        Value      Meaning
        ===== =============================================
        > 0   The version *a* is greater than version *b*.
        = 0   Both versions are equal.
        < 0   The version *a* is less than version *b*.
        ===== =============================================




Module Constants
----------------
.. _CurStates:

Package States
^^^^^^^^^^^^^^^
.. data:: CURSTATE_CONFIG_FILES
.. data:: CURSTATE_HALF_CONFIGURED
.. data:: CURSTATE_HALF_INSTALLED
.. data:: CURSTATE_INSTALLED
.. data:: CURSTATE_NOT_INSTALLED
.. data:: CURSTATE_UNPACKED

.. _InstStates:

Installed states
^^^^^^^^^^^^^^^^
.. data:: INSTSTATE_HOLD
.. data:: INSTSTATE_HOLD_REINSTREQ
.. data:: INSTSTATE_OK
.. data:: INSTSTATE_REINSTREQ

.. _Priorities:

Priorities
^^^^^^^^^^^
.. data:: PRI_EXTRA
.. data:: PRI_IMPORTANT
.. data:: PRI_OPTIONAL
.. data:: PRI_REQUIRED
.. data:: PRI_STANDARD


.. _SelStates:

Select states
^^^^^^^^^^^^^
.. data:: SELSTATE_DEINSTALL
.. data:: SELSTATE_HOLD
.. data:: SELSTATE_INSTALL
.. data:: SELSTATE_PURGE
.. data:: SELSTATE_UNKNOWN


Build information
^^^^^^^^^^^^^^^^^
.. data:: DATE

    The date on which this extension has been compiled.

.. data:: LIB_VERSION

    The version of the apt_pkg library. This is **not** the version of apt,
    nor the version of python-apt.

.. data:: TIME

    The time this extension has been built.

.. data:: VERSION

    The version of apt (not of python-apt).
