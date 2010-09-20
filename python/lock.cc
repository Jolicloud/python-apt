/*
 * lock.cc - Context managers for implementing locking.
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
#include <Python.h>
#include <apt-pkg/init.h>
#include <apt-pkg/error.h>
#include "generic.h"

static PyObject *systemlock_exit(PyObject *self, PyObject *args)
{

    PyObject *exc_type = 0;
    PyObject *exc_value = 0;
    PyObject *traceback = 0;
    if (!PyArg_UnpackTuple(args, "__exit__", 3, 3, &exc_type, &exc_value,
                           &traceback)) {
        return 0;
    }

    if (_system->UnLock() == 0) {
        // The unlock failed. If no exception happened within the suite, we
        // will raise an error here. Otherwise, we just display the error, so
        // Python can handle the original exception instead.
        HandleErrors();
        if (exc_type == Py_None)
            return NULL;
        else
            PyErr_WriteUnraisable(self);
    }
    // Return False, as required by the context manager protocol.
    Py_RETURN_FALSE;
}

static PyObject *systemlock_enter(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    if (!_system->Lock())
        return HandleErrors();
    Py_INCREF(self);
    return self;
}

static PyObject *systemlock_new(PyTypeObject *type, PyObject *args,
                                PyObject *kwds)
{
    if (_system == 0) {
        PyErr_SetString(PyExc_ValueError,"_system not initialized");
        return 0;
    }
    return PyType_GenericNew(type,args,kwds);
}

static PyMethodDef systemlock_methods[] = {
    {"__enter__",systemlock_enter,METH_VARARGS,"Lock the system."},
    {"__exit__",systemlock_exit,METH_VARARGS,"Unlock the system."},
    {NULL}
};

static char *systemlock_doc = "SystemLock()\n\n"
    "Context manager for locking the package system. The lock is established\n"
    "as soon as the method __enter__() is called. It is released when\n"
    "__exit__() is called.\n\n"
    "This should be used via the 'with' statement, e.g.::\n\n"
    "   with apt_pkg.SystemLock():\n"
    "       ...\n\n"
    "Once the block is left, the lock is released automatically. The object\n"
    "can be used multiple times::\n\n"
    "   lock = apt_pkg.SystemLock()\n"
    "   with lock:\n"
    "       ...\n"
    "   with lock:\n"
    "       ...\n\n";

PyTypeObject PySystemLock_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.SystemLock",                // tp_name
    0,                                   // tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    0,                                   // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    0,                                   // tp_repr
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    (Py_TPFLAGS_DEFAULT |                // tp_flags
     Py_TPFLAGS_BASETYPE),
    systemlock_doc,                      // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    systemlock_methods,                  // tp_methods
    0,                                   // tp_members
    0,                                   // tp_getset
    0,                                   // tp_base
    0,                                   // tp_dict
    0,                                   // tp_descr_get
    0,                                   // tp_descr_set
    0,                                   // tp_dictoffset
    0,                                   // tp_init
    0,                                   // tp_alloc
    systemlock_new,                      // tp_new
};


/**
 * File Based locking.
 *
 * The counter is increased by every call to filelock_enter() and decreased by
 * every call to filelock_exit(). When the counter reaches 0, the underlying
 * file descriptor is closed.
 *
 * Members:
 * @member char* filename   The name of the file
 * @member int   lock_count How many times we have locked it.
 * @member int   fd         The filedescriptor returned by GetLock() or 0.
 */
struct filelock_object {
    PyObject_HEAD
    char *filename;
    int lock_count;
    int fd;
};

static PyObject *filelock_enter(filelock_object *self, PyObject *args)
{
    self->lock_count++;
    // If we have no lock yet, get a lock.
    if (self->lock_count == 1) {
        self->fd = GetLock(self->filename, true);
        if (self->fd == -1) {
            self->lock_count--;
            return HandleErrors();
        }
    }
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *filelock_exit(filelock_object *self, PyObject *args)
{
    // Count down the lock_count, if it is less than 0, reset it to 0.
    self->lock_count--;
    if (self->lock_count < 0)
        self->lock_count = 0;
    if (self->lock_count == 0 && self->fd != 0 && close(self->fd) == -1) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }
    Py_RETURN_FALSE;
}

static PyObject *filelock_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    char *filename = 0;
    char *kwlist[] = {"filename",  NULL};
    if (PyArg_ParseTupleAndKeywords(args, kwds, "s:__init__", kwlist,
                                    &filename) == 0) {
        return NULL;
    }
    filelock_object *self = (filelock_object *)type->tp_alloc(type, 0);
    // Copy the string into the object.
    self->filename = new char[strlen(filename) + 1];
    strcpy(self->filename, filename);
    return (PyObject *)self;
}

static void filelock_dealloc(filelock_object *self)
{
    delete[] self->filename;
    ((PyObject*)self)->ob_type->tp_free(self);
}

static PyMethodDef filelock_methods[] = {
    {"__enter__",(PyCFunction)filelock_enter,METH_VARARGS,"Lock the system."},
    {"__exit__",(PyCFunction)filelock_exit,METH_VARARGS,"Unlock the system."},
    {NULL}
};

static char *filelock_doc = "SystemLock(filename: str)\n\n"
    "Context manager for locking using a file. The lock is established\n"
    "as soon as the method __enter__() is called. It is released when\n"
    "__exit__() is called.\n\n"
    "This should be used via the 'with' statement, e.g.::\n\n"
    "   with apt_pkg.FileLock(filename):\n"
    "       ...\n\n"
    "Once the block is left, the lock is released automatically. The object\n"
    "can be used multiple times::\n\n"
    "   lock = apt_pkg.FileLock(filename)\n"
    "   with lock:\n"
    "       ...\n"
    "   with lock:\n"
    "       ...\n\n";

PyTypeObject PyFileLock_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.FileLock",                  // tp_name
    sizeof(filelock_object),            // tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    destructor(filelock_dealloc),        // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    0,                                   // tp_repr
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    (Py_TPFLAGS_DEFAULT |                // tp_flags
     Py_TPFLAGS_BASETYPE),
    filelock_doc,                        // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    filelock_methods,                    // tp_methods
    0,                                   // tp_members
    0,                                   // tp_getset
    0,                                   // tp_base
    0,                                   // tp_dict
    0,                                   // tp_descr_get
    0,                                   // tp_descr_set
    0,                                   // tp_dictoffset
    0,                                   // tp_init
    0,                                   // tp_alloc
    filelock_new,                        // tp_new
};
