.. highlightlang:: c++

Embedding Python APT
====================
This is a very basic tutorial for working with the C++ bindings.

Basics
-------
To use the python-apt C++ bindings, first include the
``python-apt/python-apt.h`` header::

    #include <python-apt/python-apt.h>

Now, the module needs to be initialized. This is done by calling the function
:cfunc:`import_apt_pkg`. This function returns 0 on success and a negative
value in case of failure::

    if (import_apt_pkg() < 0)
        return;

Longer example
--------------
The following code will create a standalone application which provides a
module ``client`` with the attribute ``hash`` which stores an object of the
type :class:`apt_pkg.HashString`:

.. literalinclude:: ../../client-example.cc


.. highlightlang:: sh

If this file were called client-example.cc, you could compile it using::

    g++ -lapt-pkg -lpython2.5 -I/usr/include/python2.5 -o client client-example.cc
