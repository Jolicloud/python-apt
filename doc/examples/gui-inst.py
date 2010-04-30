#!/usr/bin/python
# example how to install in a custom terminal widget
# see also gnome bug: #169201
import pygtk
pygtk.require('2.0')
import gtk

import apt.progress.gtk2


if __name__ == "__main__":

    win = gtk.Window()
    progress = apt.progress.gtk2.GtkAptProgress()
    win.set_title("GtkAptProgress Demo")
    win.add(progress)
    progress.show()
    win.show()

    cache = apt.cache.Cache(progress.open)
    if cache["2vcard"].isInstalled:
        cache["2vcard"].markDelete()
    else:
        cache["2vcard"].markInstall()
    progress.show_terminal(expanded=True)
    cache.commit(progress.fetch, progress.install)
    gtk.main()
