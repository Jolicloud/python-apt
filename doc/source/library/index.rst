Python APT Library
==================
Python APT's library provides access to almost every functionality supported
by the underlying apt-pkg and apt-inst libraries. This means that it is
possible to rewrite frontend programs like apt-cdrom in Python, and this is
relatively easy, as can be seen in e.g. :doc:`../tutorials/apt-cdrom`.

When going through the library, the first two modules are :mod:`apt_pkg` and
:mod:`apt_inst`. These modules are more or less straight bindings to the
apt-pkg and apt-inst libraries and the base for the rest of python-apt.

Going forward, the :mod:`apt` package appears. This package is using
:mod:`apt_pkg` and :mod`apt_inst` to provide easy to use ways to manipulate
the cache, fetch packages, or install new packages. It also provides useful
progress classes, for text and GTK+ interfaces. The last package is
:mod:`aptsources`. The aptsources package provides classes and functions to
read files like :file:`/etc/apt/sources.list` and to modify them.

.. toctree::
    :maxdepth: 1

    apt_pkg
    apt_inst

    apt.cache
    apt.cdrom
    apt.debfile
    apt.package
    apt.progress.base
    apt.progress.text
    apt.progress.gtk2
    apt.progress.qt4

    aptsources.distinfo
    aptsources.distro
    aptsources.sourceslist

