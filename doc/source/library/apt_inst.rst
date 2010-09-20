:mod:`apt_inst` - Working with local Debian packages
====================================================
.. module:: apt_inst

This module provides useful classes and functions to work with archives,
modelled after the :class:`tarfile.TarFile` class. For working with Debian
packages, the :class:`DebFile` class should be used as it provides easy access
to the control.tar.* and data.tar.* members.

The classes are mostly modeled after the :class:`tarfile.TarFile` class and
enhanced with APT-specific methods. Because APT only provides a stream based
view on a tar archive, this module's :class:`TarFile` class only provides a
very small subset of those functions.

AR Archives
-----------
.. class:: ArArchive(file)

    An ArArchive object represents an archive in the 4.4 BSD AR format,
    which is used for e.g. deb packages.

    The parameter *file* may be a string specifying the path of a file, or
    a :class:`file`-like object providing the :meth:`fileno` method. It may
    also be an int specifying a file descriptor (returned by e.g.
    :func:`os.open`). The recommended way is to pass in the path to the file.

    ArArchive (and its subclasses) support the iterator protocol, meaning that
    an :class:`ArArchive` object can be iterated over yielding the members in
    the archive (same as :meth:`getmembers`).

    .. describe:: archive[key]

        Return a ArMember object for the member given by *key*. Raise
        LookupError if there is no ArMember with the given name.

    .. describe:: key in archive

        Return True if a member with the name *key* is found in the archive, it
        is the same function as :meth:`getmember`.

    .. method:: extract(name[, target: str]) -> bool

        Extract the member given by *name* into the directory given by
        *target*. If the extraction failed, an error is raised. Otherwise,
        the method returns True if the owner could be set or False if the
        owner could not be changed. It may also raise LookupError if there
        is no member with the given name.

        The parameter *target* is completely optional. If it is not given, the
        function extracts into the current directory.

    .. method:: extractall([target: str]) -> bool

         Extract all into the directory given by target or the current
         directory if target is not given. If the extraction failed, an error
         is raised. Otherwise, the method returns True if the owner could be
         set or False if the owner could not be changed.

    .. method:: extractdata(name: str) -> bytes

        Return the contents of the member given by *name*, as a bytes object.
        Raise LookupError if there is no ArMember with the given name.

    .. method:: getmember(name: str) -> ArMember

        Return a ArMember object for the member given by *name*. Raise
        LookupError if there is no ArMember with the given name.

    .. method:: getmembers() -> list

        Return a list of all members in the AR archive.

    .. method:: getnames() -> list

        Return a list of the names of all members in the AR archive.

    .. method:: gettar(name: str, comp: str) -> TarFile

        Return a TarFile object for the member given by *name* which will be
        decompressed using the compression algorithm given by *comp*.
        This is almost equal to::

           member = arfile.getmember(name)
           tarfile = TarFile(file, member.start, member.size, 'gzip')'

        It just opens a new TarFile on the given position in the stream.

.. class:: ArMember

    An ArMember object represents a single file within an AR archive. For
    Debian packages this can be e.g. control.tar.gz. This class provides
    information about this file, such as the mode and size. It has no
    constructor.

    .. attribute:: gid

        The group id of the owner.

    .. attribute:: mode

        The mode of the file.

    .. attribute:: mtime

        Last time of modification.

    .. attribute:: name

        The name of the file.

    .. attribute:: size

        The size of the files.

    .. attribute:: start

        The offset in the archive where the file starts.

    .. attribute:: uid

        The user id of the owner.

Debian Packages
---------------
.. class:: DebFile(file)

    A DebFile object represents a file in the .deb package format. It inherits
    :class:`ArArchive`. In addition to the attributes and methods from
    :class:`ArArchive`, DebFile provides the following methods:

    .. attribute:: control

        The :class:`TarFile` object associated with the control.tar.gz member.

    .. attribute:: data

        The :class:`TarFile` object associated with the data.tar.{gz,bz2,lzma}
        member.

    .. attribute:: debian_binary

        The package version, as contained in debian-binary.

Tar Archives
-------------
.. class:: TarFile(file[, min: int, max: int, comp: str])

    A TarFile object represents a single .tar file stream.

    The parameter *file* may be a string specifying the path of a file, or
    a :class:`file`-like object providing the :meth:`fileno` method. It may
    also be an int specifying a file descriptor (returned by e.g.
    :func:`os.open`).

    The parameter *min* describes the offset in the file where the archive
    begins and the parameter *max* is the size of the archive.

    The compression of the archive is set by the parameter *comp*. It can
    be set to any program supporting the -d switch, the default being gzip.

    .. method:: extractall([rootdir: str]) -> True

        Extract the archive in the current directory. The argument *rootdir*
        can be used to change the target directory.

    .. method:: extractdata(member: str) -> bytes

        Return the contents of the member, as a bytes object. Raise
        LookupError if there is no member with the given name.

    .. method:: go(callback: callable[, member: str]) -> True

        Go through the archive and call the callable *callback* for each
        member with 2 arguments. The first argument is the :class:`TarMember`
        and the second one is the data, as bytes.

        The optional parameter *member* can be used to specify the member for
        which call the callback. If not specified, it will be called for all
        members. If specified and not found, LookupError will be raised.

.. class:: TarMember

    Represent a single member of a 'tar' archive.

    This class which has been modelled after :class:`tarfile.TarInfo`
    represents information about a given member in an archive.

    .. method:: isblk()

        Determine whether the member is a block device.

    .. method:: ischr()

        Determine whether the member is a character device.

    .. method:: isdev()

        Determine whether the member is a device (block,character or FIFO).

    .. method:: isdir()

        Determine whether the member is a directory.

    .. method:: isfifo()

        Determine whether the member is a FIFO.

    .. method:: isfile()

        Determine whether the member is a regular file.
        
    .. method:: islnk()

        Determine whether the member is a hardlink.

    .. method:: isreg()

        Determine whether the member is a regular file, same as isfile().

    .. method:: issym()

        Determine whether the member is a symbolic link.
        
    .. attribute:: gid

        The owner's group id

    .. attribute:: linkname

        The target of the link.

    .. attribute:: major

        The major ID of the device.

    .. attribute:: minor

        The minor ID of the device.

    .. attribute:: mode

        The mode (permissions).

    .. attribute:: mtime

        Last time of modification.

    .. attribute:: name

        The name of the file.

    .. attribute:: size

        The size of the file.

    .. attribute:: uid

        The owner's user id.



Deprecated functions
---------------------
The following functions have been shipped in python-apt for a longer time and
are deprecated as of release 0.7.92. They are listed here to help developers
to port their applications to the new API which is completely different. For
this purpose each function documentation includes an example showing how the
function can be replaced.

.. function:: arCheckMember(file, membername)

    Check if the member specified by the parameter *membername* exists in
    the AR file referenced by the parameter *file*, which may be a
    :class:`file()` object, a file descriptor, or anything implementing a
    :meth:`fileno` method.

    This function has been replaced by using the :keyword:`in` check on an
    :class:`ArArchive` object::

        member in ArArchive(file)    

.. function:: debExtract(file, func, chunk)

    Call the function referenced by *func* for each member of the tar file
    *chunk* which is contained in the AR file referenced by the parameter
    *file*, which may be a :class:`file()` object, a file descriptor, or
    anything implementing a :meth:`fileno` method.

    The function *func* is a callback with the signature
    ``(what, name, link, mode, uid, gid, size, mtime, major, minor)``. The
    parameter *what* describes the type of the member. It can be 'FILE',
    'DIR', or 'HARDLINK'. The parameter *name* refers to the name of the
    member. In case of links, *link* refers to the target of the link.

    This function is deprecated and has been replaced by the :meth:`TarFile.go`
    method. The following example shows the old code and the new code::

        debExtract(open("package.deb"), my_callback, "data.tar.gz") #old

        DebFile("package.deb").data.go(my_callback)

    Please note that the signature of the callback is different in
    :meth:`TarFile.go`.

.. function:: tarExtract(file,func,comp)

    Call the function *func* for each member of the tar file *file*.

    The parameter *comp* is a string determining the compressor used. Possible
    options are "lzma", "bzip2" and "gzip". The parameter *file* may be
    a :class:`file()` object, a file descriptor, or anything implementing
    a :meth:`fileno` method.

    The function *func* is a callback with the signature
    ``(what, name, link, mode, uid, gid, size, mtime, major, minor)``. The
    parameter *what* describes the type of the member. It can be 'FILE',
    'DIR', or 'HARDLINK'. The parameter *name* refers to the name of the
    member. In case of links, *link* refers to the target of the link.

    This function is deprecated and has been replaced by the :meth:`TarFile.go`
    method. The following example shows the old code and the new code::

        tarExtract(open("archive.tar.gz"), my_callback, "gzip") #old
        TarFile("archive.tar.gz", 0, 0, "gzip").go(my_callback)

    Please note that the signature of the callback is different in
    :meth:`TarFile.go`.

.. function:: debExtractArchive(file, rootdir)

    Extract the archive referenced by the :class:`file` object *file*
    into the directory specified by *rootdir*.

    The parameter *file* may be a :class:`file()` object, a file descriptor, or
    anything implementing a :meth:`fileno` method.

    This function has been replaced by :meth:`TarFile.extractall` and
    :attr:`DebFile.data`::

        debExtractArchive(open("package.deb"), rootdir) # old
        DebFile("package.deb").data.extractall(rootdir) # new

.. function:: debExtractControl(file[, member='control'])

    Return the indicated file as a string from the control tar. The default
    is 'control'. The parameter *file* may be a :class:`file()` object, a
    file descriptor, or anything implementing a :meth:`fileno` method.

    This function has been replaced by :attr:`DebFile.control` and
    :meth:`TarFile.extractdata`. In the following example, both commands
    return the contents of the control file::

        debExtractControl(open("package.deb"))
        DebFile("package.deb").control.extractdata("control")
