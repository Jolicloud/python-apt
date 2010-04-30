:mod:`apt.package` --- Classes for package handling
====================================================


.. automodule:: apt.package


The Package class
-----------------
.. autoclass:: Package
    :members:

    .. note::

        Several methods have been deprecated in version 0.7.9 of python-apt,
        please see the :class:`Version` class for the new alternatives.

The Version class
-----------------
.. autoclass:: Version
    :members:


Dependency Information
----------------------
.. class:: BaseDependency

    The :class:`BaseDependency` class defines various attributes for accessing
    the parts of a dependency. The attributes are as follows:

    .. attribute:: name

        The name of the dependency

    .. attribute:: relation

        The relation (>>,>=,==,<<,<=,)

    .. attribute:: version

        The version or None.

    .. attribute:: preDepend

        Boolean value whether this is a pre-dependency.

.. class:: Dependency

    The dependency class represents a Or-Group of dependencies. It provides
    an attribute to access the :class:`BaseDependency` object for the available
    choices.

    .. attribute:: or_dependencies

        A list of :class:`BaseDependency` objects which could satisfy the
        requirement of the Or-Group.


Origin Information
-------------------
.. class:: Origin

    The :class:`Origin` class provides access to the origin of the package.
    It allows you to check the component, archive, the hostname, and even if
    this package can be trusted.

    .. attribute:: archive

        The archive (eg. unstable)

    .. attribute:: component

        The component (eg. main)

    .. attribute:: label

        The Label, as set in the Release file

    .. attribute:: origin

        The Origin, as set in the Release file

    .. attribute:: site

        The hostname of the site.

    .. attribute:: trusted

       Boolean value whether this is trustworthy. An origin can be trusted, if
       it provides a GPG-signed Release file and the GPG-key used is in the
       keyring used by apt (see apt-key).

Examples
---------
.. code-block:: python

    import apt

    cache = apt.Cache()
    pkg = cache['python-apt'] # Access the Package object for python-apt
    print 'python-apt is trusted:', pkg.candidate.origins[0].trusted

    # Mark python-apt for install
    pkg.markInstall()

    print 'python-apt is marked for install:', pkg.markedInstall

    print 'python-apt is (summary):', pkg.candidate.summary

    # Now, really install it
    cache.commit()
