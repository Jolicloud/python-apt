Contributing to python-apt
==========================
:Author: Julian Andres Klode <jak@debian.org>
:Release: |release|
:Date: |today|

Let's say you need a new feature, you can develop it, and you want to get it
included in python-apt. Then be sure to follow the following guidelines.

Available branches
-------------------
First of all, let's talk a bit about the bzr branches of python-apt. In the
following parts, we will assume that you use bzr to create your changes and
submit them.

**mvo:** http://people.ubuntu.com/~mvo/bzr/python-apt/mvo
    This is Michael Vogt's branch. Most of the development of apt happens here,
    as he is the lead maintainer of python-apt.

    This branch is also available from Launchpads super mirror, via
    ``lp:python-apt``. Checkouts from Launchpad are generally faster and can
    use the bzr protocoll.

    VCS-Browser: https://code.launchpad.net/~mvo/python-apt/python-apt--mvo

**debian-sid:** http://bzr.debian.org/apt/python-apt/debian-sid
    This is the official Debian branch of python-apt. All code which will be
    uploaded to Debian is here. It is not as up-to-date as the mvo branch,
    because this branch often gets updated just right before the release
    happens.

    VCS-Browser: http://bzr.debian.org/loggerhead/apt/python-apt/debian-sid/changes

**debian-experimental:** http://bzr.debian.org/apt/python-apt/debian-experimental

    This is another official Debian branch of python-apt, for releases
    targetted at Debian experimental. This branch may contain unstable code
    and may thus not work correctly.

    VCS-Browser: http://bzr.debian.org/loggerhead/apt/python-apt/debian-experimental/changes

**jak:** http://bzr.debian.org/users/jak/python-apt/jak
    This is Julian Andres Klode's (the documentation author's) branch. This
    is the place where cleanup and documentation updates happen. It is based
    off debian-sid or mvo.

    VCS-Browser: http://bzr.debian.org/loggerhead/users/jak/python-apt/jak/changes

**ubuntu:** ``lp:~ubuntu-core-dev/python-apt/ubuntu``
    This is the official Ubuntu development branch. The same notes apply as
    for the debian-sid branch above.

    VCS-Browser: https://code.launchpad.net/~ubuntu-core-dev/python-apt/ubuntu


.. highlightlang:: c

C++ Coding style
----------------
This document gives coding conventions for the C++ code comprising
the C++ extensions of Python APT.  Please see the companion
informational PEP describing style guidelines for Python code (:PEP:`8`).

Note, rules are there to be broken.  Two good reasons to break a
particular rule:

    (1) When applying the rule would make the code less readable, even
        for someone who is used to reading code that follows the rules.

    (2) To be consistent with surrounding code that also breaks it
        (maybe for historic reasons) -- although this is also an
        opportunity to clean up someone else's mess (in true XP style).

This part of the document is derived from :PEP:`7` which was written by
Guido van Rossum.


C++ dialect
^^^^^^^^^^^

- Use ISO standard C++ (the 1998 version of the standard).

- All function declarations and definitions must use full
  prototypes (i.e. specify the types of all arguments).

- Use C++ style // one-line comments where useful.

- No compiler warnings with ``gcc -std=c++98 -Wall -Wno-write-strings``. There
  should also be no errors with ``-pedantic`` added.


Code lay-out
^^^^^^^^^^^^

- Use 3-space indents, in files that already use them. In new source files,
  that were created after this rule was introduced, use 4-space indents.

  At some point, the whole codebase may be converted to use only
  4-space indents.

- No line should be longer than 79 characters.  If this and the
  previous rule together don't give you enough room to code, your
  code is too complicated -- consider using subroutines.

- No line should end in whitespace.  If you think you need
  significant trailing whitespace, think again -- somebody's
  editor might delete it as a matter of routine.

- Function definition style: function name in column 2, outermost
  curly braces in column 1, blank line after local variable
  declarations::

    static int extra_ivars(PyTypeObject *type, PyTypeObject *base)
    {
        int t_size = PyType_BASICSIZE(type);
        int b_size = PyType_BASICSIZE(base);

        assert(t_size >= b_size); /* type smaller than base! */
        ...
        return 1;
    }

- Code structure: one space between keywords like 'if', 'for' and
  the following left paren; no spaces inside the paren; braces as
  shown::

    if (mro != NULL) {
        ...
    }
    else {
        ...
    }

- The return statement should *not* get redundant parentheses::

    return Py_None; /* correct */
    return(Py_None); /* incorrect */

- Function and macro call style: ``foo(a, b, c)`` -- no space before
  the open paren, no spaces inside the parens, no spaces before
  commas, one space after each comma.

- Always put spaces around assignment, Boolean and comparison
  operators.  In expressions using a lot of operators, add spaces
  around the outermost (lowest-priority) operators.

- Breaking long lines: if you can, break after commas in the
  outermost argument list.  Always indent continuation lines
  appropriately, e.g.::

    PyErr_Format(PyExc_TypeError,
            "cannot create '%.100s' instances",
            type->tp_name);

- When you break a long expression at a binary operator, the
  operator goes at the end of the previous line, e.g.::

    if (type->tp_dictoffset != 0 && base->tp_dictoffset == 0 &&
        type->tp_dictoffset == b_size &&
        (size_t)t_size == b_size + sizeof(PyObject *))
        return 0; /* "Forgive" adding a __dict__ only */

- Put blank lines around functions, structure definitions, and
  major sections inside functions.

- Comments go before the code they describe.

- All functions and global variables should be declared static
  unless they are to be part of a published interface


Naming conventions
^^^^^^^^^^^^^^^^^^

- Use a ``Py`` prefix for public functions; never for static
  functions.  The ``Py_`` prefix is reserved for global service
  routines like ``Py_FatalError``; specific groups of routines
  (e.g. specific object type APIs) use a longer prefix,
  e.g. ``PyString_`` for string functions.

- Public functions and variables use MixedCase with underscores,
  like this: ``PyObject_GetAttr``, ``Py_BuildValue``, ``PyExc_TypeError``.

- Internal functions and variables use lowercase with underscores, like
  this: ``hashes_get_sha1.``

- Occasionally an "internal" function has to be visible to the
  loader; we use the _Py prefix for this, e.g.: ``_PyObject_Dump``.

- Macros should have a MixedCase prefix and then use upper case,
  for example: ``PyString_AS_STRING``, ``Py_PRINT_RAW``.


Documentation Strings
^^^^^^^^^^^^^^^^^^^^^
- The first line of each function docstring should be a "signature
  line" that gives a brief synopsis of the arguments and return
  value.  For example::

    PyDoc_STRVAR(myfunction__doc__,
    "myfunction(name: str, value) -> bool\n\n"
    "Determine whether name and value make a valid pair.");

  The signature line should be formatted using the format for function
  annotations described in :PEP:`3107`, whereas the annotations shall reflect
  the name of the type (e.g. ``str``). The leading ``def`` and the trailing
  ``:`` as used for function definitions must not be included.

  Always include a blank line between the signature line and the
  text of the description.

  If the return value for the function is always ``None`` (because
  there is no meaningful return value), do not include the
  indication of the return type.

- When writing multi-line docstrings, be sure to always use
  string literal concatenation::

    PyDoc_STRVAR(myfunction__doc__,
    "myfunction(name, value) -> bool\n\n"
    "Determine whether name and value make a valid pair.");


Python Coding Style
-------------------
The coding style for all code written in python is :PEP:`8`. Exceptions from
this rule are the documentation, where code is sometimes formatted differently
to explain aspects, and functions provided for 0.7 compatibility purposes.

When writing code, use tools like pylint, pyflakes, pychecker and pep8.py from
http://svn.browsershots.org/trunk/devtools/pep8/ to verify that your code is
OK. Fix all the problems which seem reasonable, and mention the unfixed issues
when asking for merge.

In order to make the automatic generation of Python 3 code using 2to possible,
code written in Python may not utilize any functionality unsupported by 2to3 or
deprecated as of Python 2.6.

Submitting your patch
---------------------
First of all, the patch you create should be based against the most current
branch of python-apt (debian-sid or debian-experimental). If it is a bugfix,
you should probably use debian-sid. If you choose the wrong branch, we will
ask you to rebase your patches against the correct one.

Once you have made your change, check that it:

    * conforms to :PEP:`8` (checked with pep8.py). It should, at least not
      introduce new errors. (and never have whitespace at end of line)
    * produces no new errors in pychecker, pyflakes and pylint (unless you
      can't fix them, but please tell so when requesting the merge, so it can
      be fixed before hitting one of the main branches).
    * does not change the behaviour of existing code in a non-compatible way.

If your change follows all points of the checklist, you can commit it to your
repository. (You could commit it first, and check later, and then commit the
fixes, but commits should be logical and it makes no sense to have to commits
for one logical unit).

Once you have made all your changes,  you can run ``bzr send -o patch-name``
to create a so called *merge-directive*, which contains your changes and
allows us to preserve the history of your changes. (But please replace patch-name
with something useful).

Now report a bug against the python-apt package, attach the merge directive
you created in the previous step, and tag it with 'patch'. It might also be
a good idea to prefix the bug report with '[PATCH]'.

If your patch introduces new functions, parameters, etc. , but does not update
the content of this documentation, please CC. jak@debian.org, and add a short
notice to the bug report. Also see `Documentation updates`

Once your patch got merged, you can *pull* the branch into which it has been
merged into your local one. If you have made changes since you submitted your
patch, you may need to *merge* the branch instead.

.. note::

    If you plan to work on python-apt for a longer time, it may be a good
    idea to publish your branch somewhere. Alioth (http://alioth.debian.org)
    and Launchpad (https://launchpad.net) provide bzr hosting. You can also
    use any webspace with ftp or sftp connection (for the upload). Then you do
    not need to send *merge directives*, but you can point to your branch
    instead.


Documentation updates
---------------------
If you want to update the documentation, please follow the procedure as written
above. You can send your content in plain text, but reStructuredText is the
preferred format. I (Julian Andres Klode) will review your patch and include
it.

.. highlightlang:: sh

Example patch session
----------------------
In the following example, we edit a file, create a merge directive (an enhanced
patch), and report a wishlist bug with this patch against the python-apt
package::

    user@pc:~$ bzr clone http://bzr.debian.org/apt/python-apt/debian-sid/
    user@pc:~$ cd debian-sid
    user@pc:~/debian-sid$ editor FILES
    user@pc:~/debian-sid$ pep8.py FILES # PEP 8 check, see above.
    user@pc:~/debian-sid$ pylint -e FILES # Check with pylint
    user@pc:~/debian-sid$ pyflakes FILES  # Check with pyflakes
    user@pc:~/debian-sid$ pychecker FILES # Check with pychecker
    user@pc:~/debian-sid$ bzr commit
    user@pc:~/debian-sid$ bzr send -o my-patch
    user@pc:~/debian-sid$ reportbug --severity=wishlist --tag=patch --attach=my-patch python-apt
    user@pc:~/debian-sid$ # Add --list-cc=jak@debian.org if you change docs.
