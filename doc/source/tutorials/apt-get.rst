Doing stuff :command:`apt-get` does
===================================
:Author: Julian Andres Klode <jak@debian.org>
:Release: |release|
:Date: |today|

The following article will show how you can use python-apt to do actions done
by the :command:`apt-get` command.


Printing the URIs of all index files
------------------------------------
We all now that we can print the URIs of all our index files by running a
simple ``apt-get -s --print-uris update``. We can do the same. Responsible for
the source entries is the class :class:`apt_pkg.SourceList`, which can be
combined with an :class:`apt_pkg.Acquire` object using :meth:`get_indexes`.

First of all, we have to create the objects::

    acquire = apt_pkg.Acquire()
    slist = apt_pkg.SourceList()

Now we have to parse /etc/apt/sources.list and its friends, by using
:meth:`apt_pkg.SourceList.read_main_list`::
    slist.read_main_list()

Now the **slist** object knows about the location of the indexes. We now have
to load those indexes into the *acquire* object by calling
:meth:`apt_pkg.SourceList.get_indexes`::

    slist.get_indexes(acquire, True)

The first argument is the acquire object into which we will load these indexes,
and the second argument means that we want to fetch all indexes. Now the only
thing left to do is iterating over the list of items and printing out their
URIs. Luckily, there is :attr:`apt_pkg.Acquire.items` which allows us to
iterate over the items::

    for item in acquire.items:
        print item.desc_uri

In the end a program could look like this:

.. literalinclude:: ../examples/update-print-uris.py

