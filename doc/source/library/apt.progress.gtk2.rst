:mod:`apt.progress.gtk2` --- Progress reporting for GTK+ interfaces
===================================================================
.. module:: apt.progress.gtk2

The :mod:`apt.progress.gtk2` module provides classes with GObject signals and
a class with a GTK+ widget for progress handling.


GObject progress classes
-------------------------
.. class:: GInstallProgress

    An implementation of :class:`apt.progress.base.InstallProgress` supporting
    GObject signals. The class emits the following signals:

    .. describe:: status-changed(status: str, percent: int)

        Emitted when the status of an operation changed.

    .. describe:: status-started()

        Emitted when the installation started.

    .. describe::  status-finished()

        Emitted when the installation finished.

    .. describe:: status-timeout()

        Emitted when a timeout happens

    .. describe:: status-error()

        Emitted in case of an error.

    .. describe:: status-conffile()

        Emitted when a conffile update is happening.


.. class:: GAcquireProgress

    An implementation of :class:`apt.progress.base.AcquireProgress` supporting
    GObject signals. The class emits the following signals:

    .. describe:: status-changed(description: str, percent: int)

        Emitted when the status of the fetcher changed, e.g. when the
        percentage increased.
        
    .. describe:: status-started()

        Emitted when the fetcher starts to fetch.

    .. describe:: status-finished()

        Emitted when the fetcher finished.


.. class:: GDpkgInstallProgress

    An implementation of :class:`apt.progress.base.InstallProgress` supporting
    GObject signals. This is the same as :class:`GInstallProgress` and is thus
    completely deprecated.

.. class:: GOpProgress

    An implementation of :class:`apt.progress.old.FetchProgress` supporting
    GObject signals. The class emits the following signals:

    .. describe:: status-changed(operation: str, percent: int)

        Emitted when the status of an operation changed.

    .. describe:: status-started()

        Emitted when it starts - Not implemented yet.

    .. describe:: status-finished()

        Emitted when all operations have finished.

GTK+ Widget
-----------
.. class:: GtkAptProgress

    Graphical progress for installation/fetch/operations, providing
    a progress bar, a terminal and a status bar for showing the progress
    of package manipulation tasks.

    .. method:: cancel_download()

        Cancel a currently running download.
        
    .. method:: clear()
    
        Reset all status information.

    .. attribute:: dpkg_install

        Return the install progress handler for dpkg.

    .. attribute:: fetch
    
        Return the fetch progress handler.

    .. method:: hide_terminal()

        Hide the expander with the terminal widget.

    .. attribute:: install
    
        Return the install progress handler.

    .. attribute:: open

        Return the cache opening progress handler.
        
    .. method:: show()
    
        Show the Box

    .. method:: show_terminal(expanded=False)
    
        Show an expander with a terminal widget which provides a way to
        interact with dpkg.


Example
-------
.. literalinclude:: ../examples/apt-gtk.py
