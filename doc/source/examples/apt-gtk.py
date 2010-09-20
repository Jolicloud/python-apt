#!/usr/bin/python
"""Example for gtk widgets"""
import pygtk
pygtk.require("2.0")
import gtk

import apt.progress.gtk2


def main():
    """Main function."""
    win = gtk.Window()
    win.connect("destroy", gtk.main_quit)
    progress = apt.progress.gtk2.GtkAptProgress()
    win.set_title("GtkAptProgress Demo")
    win.add(progress)
    progress.show()
    win.show()
    cache = apt.cache.Cache(progress.open)
    if cache["xterm"].is_installed:
        cache["xterm"].mark_delete()
    else:
        cache["xterm"].mark_install()
    progress.show_terminal(expanded=True)
    cache.commit(progress.fetch, progress.install)
    gtk.main()

if __name__ == "__main__":
    main()
