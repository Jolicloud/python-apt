:mod:`apt.cache` --- The Cache class
=====================================
.. automodule:: apt.cache

The Cache class
---------------

.. autoclass:: Cache
    :members:
    :undoc-members:

    .. describe:: cache[pkgname]

        Return a :class:`Package()` for the package with the name *pkgname*.

Example
^^^^^^^

The following example shows how to load the cache, update it, and upgrade
all the packages on the system::

    import apt
    import apt.progress

    # First of all, open the cache
    cache = apt.Cache()
    # Now, lets update the package list
    cache.update()
    # We need to re-open the cache because it needs to read the package list
    cache.open(None)
    # Now we can do the same as 'apt-get upgrade' does
    cache.upgrade()
    # or we can play 'apt-get dist-upgrade'
    cache.upgrade(True)
    # Q: Why does nothing happen?
    # A: You forgot to call commit()!
    cache.commit(apt.progress.TextFetchProgress(),
                 apt.progress.InstallProgress())



Working with Filters
--------------------
.. autoclass:: Filter
    :members:
    :inherited-members:
    :undoc-members:

.. autoclass:: MarkedChangesFilter
    :members:
    :inherited-members:
    :undoc-members:

.. autoclass:: FilteredCache
    :members:
    :inherited-members:
    :undoc-members:


Example
^^^^^^^

This is an example for a filtered cache, which only allows access to the
packages whose state has been changed, eg. packages marked for installation::

    >>> from apt.cache import FilteredCache, Cache, MarkedChangesFilter
    >>> cache = apt.Cache()
    >>> changed = apt.FilteredCache(cache)
    >>> changed.set_filter(MarkedChangesFilter())
    >>> print len(changed) == len(cache.get_changes()) # Both need to have same length
    True

The ProblemResolver class
--------------------------

.. autoclass:: ProblemResolver
    :members:

Exceptions
----------
.. autoexception:: FetchCancelledException
.. autoexception:: FetchFailedException
.. autoexception:: LockFailedException
