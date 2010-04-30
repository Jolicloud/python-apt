:mod:`apt` --- Highlevel apt package
=====================================
The highlevel apt package provides a lot of functionality, all
with an easy-to-use interface.

.. warning::
    The API of this package is not considered stable. Evenmore, it is considered
    to change the naming conventions in future to lowercase_with_underscores.

    In case this happens, the API will still be kept compatible, with the old
    functions provided as deprecated ones.

.. automodule:: apt
    :members:




.. toctree::
   :maxdepth: 2
   :glob:

   *


Classes exported in apt
------------------------
These classes are defined in the submodules, but are also exported directly
in the package.

.. class:: Cache

    Please see :class:`apt.cache.Cache` for documentation.

.. class:: Cdrom

    Please see :class:`apt.cdrom.Cdrom` for documentation.

.. class:: CdromProgress

    Please see :class:`apt.progress.CdromProgress` for documentation.

.. class:: FetchProgress

    Please see :class:`apt.progress.FetchProgress` for documentation.

.. class:: InstallProgress

    Please see :class:`apt.progress.InstallProgress` for documentation.

.. class:: OpProgress

    Please see :class:`apt.progress.OpProgress` for documentation.

.. class:: Package

    Please see :class:`apt.package.Package` for documentation.

.. class:: ProblemResolver

    Please see :class:`apt.cache.ProblemResolver` for documentation.
