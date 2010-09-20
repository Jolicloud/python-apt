/*
 * client-example.cc - A simple example for using the python-apt C++ API.
 *
 * Copyright 2009 Julian Andres Klode <jak@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <python-apt/python-apt.h>
#include <apt-pkg/hashes.h>

// The module initialization.
extern "C" void initclient() {
    if (import_apt_pkg() < 0)
        return;
    // Initialize a module.
    PyObject *Module = Py_InitModule("client", NULL);

    // Create a HashString, which will be added to the module.
    HashString *hash = new HashString("0966a120bb936bdc6fdeac445707aa6b");
    // Create a Python object for the hashstring and add it to the module
    PyModule_AddObject(Module, "hash", PyHashString_FromCpp(hash, false, NULL));

    // Another example: Add the HashString type to the module.
    Py_INCREF(&PyHashString_Type);
    PyModule_AddObject(Module, "HashString", (PyObject*)(&PyHashString_Type));
}

int main(int argc, char *argv[]) {
    // Return value.
    int ret = 0;
    // Initialize python 
    Py_Initialize();
    // Make the client module importable
    PyImport_AppendInittab("client", &initclient);
    // Set the commandline arguments.
    PySys_SetArgv(argc, argv);

    // Import the module, so the user does not have to import it.
    if (PyRun_SimpleString("import client\n") < 0) {
        // Failure (should never be reached)
        ret = 1;
        goto end;
    }

    // Run IPython if available, otherwise a normal interpreter.
    if (PyRun_SimpleString("from IPython.Shell import start\n") == 0)
        PyRun_SimpleString("start(user_ns=dict(client=client)).mainloop()\n");
    else
        Py_Main(argc, argv);

end:
    Py_Finalize();
    return ret;
}
