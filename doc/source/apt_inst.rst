:mod:`apt_inst` - Working with local Debian packages
====================================================
.. module:: apt_inst

The :mod:`apt_inst` extension provides access to functions for working with
locally available Debian packages (.deb files) and tar files.


Checking packages
------------------
.. function:: arCheckMember(file, membername)

    Check if the member specified by the parameter *membername* exists in
    the AR file referenced by the :class:`file` object *file*.


Listing contents
-----------------
.. function:: debExtract(file, func, chunk)

    Call the function referenced by *func* for each member of the tar file
    *chunk* which is contained in the AR file referenced by the file object
    *file*.

    An example would be::

        debExtract(open("package.deb"), my_callback, "data.tar.gz")

    See :ref:`emulating-dpkg-contents` for a more detailed example.

.. function:: tarExtract(file,func,comp)

    Call the function *func* for each member of the tar file *file*.

    *Comp* is a string determining the compressor used. Possible options are
    "lzma", "bzip2" and "gzip".


Callback
^^^^^^^^^
Both of these functions expect a callback with the signature
``(what, name, link, mode, uid, gid, size, mtime, major, minor)``.

The parameter *what* describes the type of the member. It can be 'FILE',
'DIR', or 'HARDLINK'.

The parameter *name* refers to the name of the member. In case of links,
*link* refers to the target of the link.


Extracting contents
-------------------

.. function:: debExtractArchive(file, rootdir)

    Extract the archive referenced by the :class:`file` object *file*
    into the directory specified by *rootdir*.

    See :ref:`emulating-dpkg-extract` for an example.

    .. warning::

        If the directory given by *rootdir* does not exist, the package is
        extracted into the current directory.

.. function:: debExtractControl(file[, member='control'])

    Return the indicated file from the control tar. The default is 'control'.

    If you want to print the control file of a given package, you could do
    something like::

        print debExtractControl(open("package.deb"))

    :return: The contents of the file, as :class:`str`.


.. _emulating-dpkg-extract:

Example: Emulating :program:`dpkg` :option:`--extract`
-------------------------------------------------------
Here is a code snippet which emulates dpkg -x. It can be run as
:program:`tool` :option:`pkg.deb` :option:`outdir`.

.. literalinclude:: examples/dpkg-extract.py


.. _emulating-dpkg-contents:

Example: Emulating :program:`dpkg` :option:`--contents`
-------------------------------------------------------
.. literalinclude:: examples/dpkg-contents.py

Example: Emulating :program:`dpkg` :option:`--info`
----------------------------------------------------
.. literalinclude:: examples/dpkg-info.py
