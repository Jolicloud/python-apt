:mod:`apt.progress.base` --- Abstract classes for progress reporting
====================================================================
.. module:: apt.progress.base

This module provides base classes for progress handlers from which all
progress classes should inherit from. Progress reporting classes not
inheriting from those classes may not work and are not supported.

When creating a subclass of one of those classes, all overridden methods should
call the parent's method first before doing anything else, because the parent
method may have to set some attributes. Subclasses not doing so may not work
correctly or may not work at all and are completely unsupported.

AcquireProgress
---------------
.. class:: AcquireProgress

    A monitor object for downloads controlled by the Acquire class. This base
    class does nothing and should only be used as a base class to inherit
    from. Instances of this class can be passed to the constructor of
    :class:`apt_pkg.Acquire` and the Acquire object then uses it to report
    its progress.

    This class provides several methods which may be overridden by subclasses
    to implement progress reporting:

    .. method:: done(item: apt_pkg.AcquireItemDesc)

        Invoked when an item is successfully and completely fetched.

    .. method:: fail(item: apt_pkg.AcquireItemDesc)

        Invoked when the process of fetching an item encounters a fatal error
        like a non existing file or no connection to the server.

    .. method:: fetch(item: apt_pkg.AcquireItemDesc)

        Invoked when some of the item's data is fetched. This normally means
        that the file is being fetched now and e.g. the headers have been
        retrieved already.

    .. method:: ims_hit(item: apt_pkg.AcquireItemDesc)

        Invoked when an item is confirmed to be up-to-date. For instance,
        when an HTTP download is informed that the file on the server was
        not modified.

    .. method:: media_change(media: str, drive: str) -> bool

        Prompt the user to change the inserted removable media. This function
        is called whenever a media change is needed to ask the user to insert
        the needed media.
 
        The parameter *media* decribes the name of the the media type that
        should be changed, whereas the parameter *drive* should be the
        identifying name of the drive whose media should be changed.

        This method should not return until the user has confirmed to the user
        interface that the media change is complete. It must return True if
        the user confirms the media change, or False to cancel it.

    .. method:: pulse(owner: apt_pkg.Acquire) -> bool

        This method gets invoked while the Acquire progress given by the
        parameter *owner* is underway. It should display information about
        the current state. It must return ``True`` to continue the acquistion
        or ``False`` to cancel it. This base implementation always returns
        ``True``.

    .. method:: start()

        Invoked when the Acquire process starts running.

    .. method:: stop()

        Invoked when the Acquire process stops running.

    In addition to those methods, this class provides several attributes which
    are set automatically and represent the fetch progress:

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


CdromProgress
-------------
.. class:: CdromProgress

    Base class for reporting the progress of adding a cdrom which could be
    used with apt_pkg.Cdrom to produce an utility like apt-cdrom.
    
    Methods defined here:
    
    .. method:: ask_cdrom_name() -> str
    
        Ask for the name of the cdrom. This method is called when a CD-ROM
        is added (e.g. via :meth:`apt_pkg.Cdrom.add`) and no label for the
        CD-ROM can be found.

        Implementations should request a label from the user (e.g. via
        :func:`raw_input`) and return this label from the function. The
        operation can be cancelled if the function returns ``None`` instead
        of a string.
    
    .. method:: change_cdrom() -> bool
    
        Ask for the CD-ROM to be changed. This method should return ``True``
        if the CD-ROM has been changed or ``False`` if the CD-ROM has not been
        changed and the operation should be cancelled. This base implementation
        returns ``False`` and thus cancels the operation.
    
    .. method:: update(text: str, current: int)
    
        Periodically invoked in order to update the interface and give
        information about the progress of the operation.

        This method has two parameters. The first parameter *text* defines
        the text which should be displayed to the user as the progress
        message. The second parameter *current* is an integer describing how
        many steps have been completed already.

    .. attribute:: total_steps

        The number of total steps, set automatically by python-apt. It may be
        used in conjunction with the parameter *current* of the :meth:`update`
        method to show how far the operation progressed.


OpProgress
----------
.. class:: OpProgress

    OpProgress classes are used for reporting the progress of operations
    such as opening the cache. It is based on the concept of one operation
    consisting of a series of sub operations.
    
    Methods defined here:
    
    .. method:: done()
    
        Called once an operation has been completed.
    
    .. method:: update([percent=None])
    
        Called periodically to update the user interface. This function should
        use the attributes defined below to display the progress information.

        The optional parameter *percent* is included for compatibility
        reasons and may be removed at a later time.
    
    The following attributes are available and are changed by the classes
    wanting to emit progress:
    
    .. attribute:: major_change
    
        An automatically set boolean value describing whether the current call
        to update is caused by a major change. In this case, the last operation
        has finished.
    
    .. attribute:: op

        An automatically set string which describes the current operation in
        an human-readable way.
    
    .. attribute:: percent
    
        An automatically set float value describing how much of the operation
        has been completed, in percent.
    
    .. attribute:: subop

        An automatically set string which describes the current sub-operation
        in an human-readable way.


InstallProgress
---------------
.. class:: InstallProgress

    InstallProgress classes make it possible to monitor the progress of dpkg
    and APT and emit information at certain stages. It uses file descriptors
    to read the status lines from APT/dpkg and parses them and afterwards calls
    the callback methods.

    Subclasses should override the following methods in order to implement
    progress reporting:

    .. method:: conffile(current, new)
    
        Called when a conffile question from dpkg is detected.

        .. note::

            This part of the API is semi-stable and may be extended with 2 more
            parameters before the release of 0.7.100.

    .. method:: error(pkg, errormsg)
    
        (Abstract) Called when a error is detected during the install.

    The following method should be overriden to implement progress reporting
    for dpkg-based runs i.e. calls to :meth:`run` with a filename:

    .. method:: processing(pkg, stage)

        This method is called just before a processing stage starts. The
        parameter *pkg* is the name of the package and the parameter *stage*
        is one of the stages listed in the dpkg manual under the status-fd
        option, i.e. "upgrade", "install" (both sent before unpacking),
        "configure", "trigproc", "remove", "purge".

    .. method:: dpkg_status_change(pkg: str, status: str)

        This method is called whenever the dpkg status of the package
        changes. The parameter *pkg* is the name of the package and the
        parameter *status* is one of the status strings used in the status
        file (:file:`/var/lib/dpkg/status`) and documented
        in :manpage:`dpkg(1)`.

    The following methods should be overriden to implement progress reporting
    for  :meth:`run` calls with an :class:`apt_pkg.PackageManager` object as
    their parameter:

    .. method:: status_change(pkg, percent, status)

        This method implements progress reporting for package installation by
        APT and may be extended to dpkg at a later time.

        This method takes two parameters: The parameter *percent* is a float
        value describing the overall progress and the parameter *status* is a
        string describing the current status in an human-readable manner.

    .. method:: start_update()
    
        This method is called before the installation of any package starts.

    .. method:: finish_update()
    
        This method is called when all changes have been applied.

    There are also several methods which are fully implemented and should not
    be overriden by subclasses unless the subclass has very special needs:

    .. method:: fork() -> int
    
        Fork a child process and return 0 to the child process and the PID of
        the child to the parent process. This implementation just calls
        :func:`os.fork` and returns its value.

    .. method:: run(obj)
        
        This method runs install actions. The parameter *obj* may either
        be a PackageManager object in which case its **do_install()** method is
        called or the path to a deb file.
    
        If the object is a :class:`apt_pkg.PackageManager`, the functions
        returns the result of calling its ``do_install()`` method. Otherwise,
        the function returns the exit status of dpkg. In both cases, ``0``
        means that there were no problems and ``!= 0`` means that there were
        issues.

    .. method:: update_interface()
    
        This method is responsible for reading the status from dpkg/APT and
        calling the correct callback methods. Subclasses should not override
        this method.

    .. method:: wait_child()
    
        This method is responsible for calling :meth:`update_interface` from
        time to time. It exits once the child has exited. The return value
        is the full status returned by :func:`os.waitpid` (not only the
        return code). Subclasses should not override this method.

    The class also provides several attributes which may be useful:

    .. attribute:: percent

        The percentage of completion as it was in the last call to
        :meth:`status_change`.

    .. attribute:: status

        The status string passed in the last call to :meth:`status_change`.

    .. attribute:: select_timeout

        Used in :meth:`wait_child` to when calling :func:`select.select`
        on dpkg's/APT's status descriptor. Subclasses may set their own value
        if needed.

    .. attribute:: statusfd

        A readable :class:`file` object from which the status information from
        APT or dpkg is read.

    .. attribute:: writefd

        A writable :class:`file` object to which dpkg or APT write their status
        information.
