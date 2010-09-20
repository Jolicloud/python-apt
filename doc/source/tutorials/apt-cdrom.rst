Writing your own apt-cdrom
==========================
:Author: Julian Andres Klode <jak@debian.org>
:Release: |release|
:Date: |today|

This article explains how to utilise python-apt to build your own clone of the
:command:`apt-cdrom` command. To do this, we will take a look at the
:mod:`apt.cdrom` and :mod:`apt.progress.text` modules, and we will learn how
to use apt_pkg.parse_commandline to parse commandline arguments. The code shown
here works on Python 2 and Python 3.

Basics
------
The first step in building your own :command:`apt-cdrom` clone is to import the
:mod:`apt` package, which will import :mod:`apt.cdrom` and
:mod:`apt.progress.text`::

    import apt

Now we have to create a new :class:`apt.cdrom.Cdrom` object and pass to it an
:class:`apt.progress.text.CdromProgress` object, which is responsible for
displaying the progress and asking questions::

    cdrom = apt.Cdrom(apt.progress.text.CdromProgress())

Now we have to choose the action, depending on the given options on the
command line. For now, we simply use the value of ``sys.argv[1]``::

    import sys
    if sys.argv[1] == 'add':
        cdrom.add()
    elif sys.argv[1] == 'ident':
        cdrom.ident()

Now we have a basic :command:`apt-cdrom` clone which can add and identify
CD-ROMs::

    import sys

    import apt

    cdrom = apt.Cdrom(apt.progress.text.CdromProgress())
    if sys.argv[1] == 'add':
        cdrom.add()
    elif sys.argv[1] == 'ident':
        cdrom.ident()

Advanced example with command-line parsing
-------------------------------------------
Our example clearly misses a way to parse the commandline in a correct
manner. Luckily, :mod:`apt_pkg` provides us with a function to do this:
:func:`apt_pkg.parse_commandline`. To use it, we add ``import apt_pkg`` right
after import apt::

    import sys

    import apt_pkg
    import apt


:func:`apt_pkg.parse_commandline` is similar to :mod:`getopt` functions, it
takes a list of recognized options and the arguments and returns all unknown
arguments. If it encounters an unknown argument which starts with a leading
'-', the function raises an error indicating that the option is unknown. The
major difference is that this function manipulates the apt configuration space.

The function takes 3 arguments. The first argument is an
:class:`apt_pkg.Configuration` object. The second argument is a list of tuples
of the form ``(shortopt, longopt, config, type)``, whereas *shortopt* is a
character indicating the short option name, *longopt* a string indicating the
corresponding long option (e.g. ``"--help"``), *config* the name of the
configuration item which should be set and *type* the type of the argument.

For apt-cdrom, we can use the following statement::

    arguments = apt_pkg.parse_commandline(apt_pkg.config,
                    [('h', "help", "help"),
                     ('v', "version", "version"),
                     ('d', "cdrom", "Acquire::cdrom::mount", "HasArg"),
                     ('r', "rename", "APT::CDROM::Rename"),
                     ('m', "no-mount", "APT::CDROM::NoMount"),
                     ('f', "fast", "APT::CDROM::Fast"),
                     ('n', "just-print", "APT::CDROM::NoAct"),
                     ('n', "recon", "APT::CDROM::NoAct"),
                     ('n', "no-act", "APT::CDROM::NoAct"),
                     ('a', "thorough", "APT::CDROM::Thorough"),
                     ('c', "config-file", "", "ConfigFile"),
                     ('o', "option", "", "ArbItem")], args)


This allows us to support all options supported by apt-cdrom. The first option
is --help. As you can see, it omits the fourth field of the tuple; which means
it is a boolean argument. Afterwards you could use
``apt_pkg.config.find_b("help")`` to see whether ``--help`` was specified. In
``('d',"cdrom","Acquire::cdrom::mount","HasArg")`` the fourth field is
``"HasArg"``. This means that the option has an argument, in this case the
location of the mount pint. ``('c',"config-file","","ConfigFile")`` shows how
to include configuration files. This option takes a parameter which points to
a configuration file which will be added to the configuration space.
('o',"option","","ArbItem") is yet another type of option, which allows users
to set configuration options on the commandline.

Now we have to check whether help or version is specified, and print a message
and exit afterwards. To do this, we use :meth:`apt_pkg.Configuration.find_b`
which returns ``True`` if the configuration option exists and evaluates to
``True``::

    if apt_pkg.config.find_b("help"):
        print("This should be a help message")
        sys.exit(0)
    elif apt_pkg.config.find_b("version"):
        print("Version blah.")
        sys.exit(0)


Now we are ready to create our progress object and our cdrom object. Instead
of using :class:`apt.Cdrom` like in the first example, we will use
:class:`apt_pkg.Cdrom` which provides a very similar interface. We could also
use :class:`apt.Cdrom`, but `apt.Cdrom` provides options like *nomount* which
conflict with our commandline parsing::

    progress = apt.progress.text.CdromProgress()
    cdrom = apt_pkg.Cdrom()


Now we have to do the action requested by the user on the commandline. To see
which option was requested, we check the list ``arguments`` which was returned
by ``apt_pkg.parse_commandline`` above, and afterwards call ``cdrom.add`` or
``cdrom.ident``::

    if apt_pkg.config.find_b("help"):
        print("This should be a help message")
        sys.exit(0)
    elif apt_pkg.config.find_b("version"):
        print("Version blah.")
        sys.exit(0)

    if not arguments:
        sys.stderr.write('E: No operation specified\n')
        sys.exit(1)
    elif arguments[0] == 'add':
        cdrom.add(progress)
    elif arguments[0] == 'ident':
        cdrom.ident(progress)
    else:
        sys.stderr.write('E: Invalid operation %s\n' % arguments[0])
        sys.exit(1)


After putting all our actions into a main() function, we get a completely
working apt-cdrom clone, which just misses useful ``--help`` and ``--version``
options. If we add a function show_help(), we get an even more complete
apt-cdrom clone:

.. literalinclude:: ../examples/apt-cdrom.py
