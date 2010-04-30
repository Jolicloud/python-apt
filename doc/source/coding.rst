Coding for python-apt
======================
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

**jak:** http://bzr.debian.org/users/jak/python-apt/jak
    This is Julian Andres Klode's (the documentation author's) branch. This
    is the place where cleanup and documentation updates happen. It is based
    off debian-sid or mvo.

    VCS-Browser: http://bzr.debian.org/loggerhead/users/jak/python-apt/jak/changes

**ubuntu:** ``lp:~ubuntu-core-dev/python-apt/ubuntu``
    This is the official Ubuntu development branch. The same notes apply as
    for the debian-sid branch above.

    VCS-Browser: https://code.launchpad.net/~ubuntu-core-dev/python-apt/ubuntu


C++ Coding style
----------------
When you work on the C++ code in the python/ directory, you should follow some
basic rules.

The indentation of the code is a bit non-standard. We currently use 3 spaces
indentation for the C++ code.

When you create new functions, you should follow some naming conventions. All
C++ functions are named according to the ``CamelCase`` convention.

The resulting Python functions should be ``CamelCase`` as well in apt_pkg, or
``mixedCase`` in apt_inst. The same applies for variables, parameters,
attributes, etc.

.. note::

    This coding style guidelines are incomplete. If you have any questions
    send an email to deity@lists.debian.org.

.. note::

    The coding style may be changed completely during the port to Python 3.0.
    But this will not happen very soon.


Python Coding Style
-------------------
The coding style for all code written in python is :PEP:`8`. For modules added
from version 0.7.9 on, there are no exceptions.

Modules introduced prior to 0.7.9 use mixedCase names for methods, functions
and variables. These names will be replaced by names conforming to :PEP:`8`
in a future release of python-apt.

Therefore, try to reduce the introduction of the mixedName code to the absolute
minimum (sometimes you can also use shorter names).

To prepare the port to Python 3.0, code should not use any functionality which
is deprecated as of Python 2.6.

The has_key() functionality may be used only on TagSection objects; as they
provide no other way to do this. If someone is willing to adapt TagSection to
support ``key in mapping`` and ``iter(mapping)``, this would be great.

.. note::

    You can use the tool pep8.py from http://svn.browsershots.org/trunk/devtools/pep8/
    to validate your code. Please also run pylint, pychecker, and pyflakes and
    fix all new **errors** they report (undefined names, etc.).

Submitting your patch
---------------------
First of all, the patch you create should be based against the debian-sid
branch of python-apt.

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
above. But please CC: jak@debian.org in the bug report.

You can send your content in plain text, but reStructuredText is the preferred
format. I (Julian Andres Klode) will review your patch and will forward them to
Michael Vogt, for inclusion in his branch. On release, this will be merged into
the debian-sid branch.


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
