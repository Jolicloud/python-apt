/* hashes.cc - Wrapper around apt-pkg's Hashes.
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
#include "generic.h"
#include "apt_pkgmodule.h"
#include <apt-pkg/hashes.h>

static PyObject *hashes_new(PyTypeObject *type,PyObject *args,
                            PyObject *kwds)
{
    return CppPyObject_NEW<Hashes>(NULL, type);
}

static int hashes_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *object = 0;
    int Fd;
    char *kwlist[] = {"object",  NULL};

    if (PyArg_ParseTupleAndKeywords(args, kwds, "|O:__init__", kwlist,
                                    &object) == 0)
        return -1;
    if (object == 0)
        return 0;
    Hashes &hashes = GetCpp<Hashes>(self);

    if (PyBytes_Check(object) != 0) {
        char *s;
        Py_ssize_t len;
        PyBytes_AsStringAndSize(object, &s, &len);
        hashes.Add((const unsigned char*)s, len);
    }
    else if ((Fd = PyObject_AsFileDescriptor(object)) != -1) {
        struct stat St;
        if (fstat(Fd, &St) != 0 || hashes.AddFD(Fd, St.st_size) == false) {
            PyErr_SetFromErrno(PyExc_SystemError);
            return -1;
        }
    }
    else {
        PyErr_SetString(PyExc_TypeError,
                        "__init__() only understand strings and files");
        return -1;
    }
    return 0;
}

static PyObject *hashes_get_md5(PyObject *self, void*)
{
    return CppPyString(GetCpp<Hashes>(self).MD5.Result().Value());
}

static PyObject *hashes_get_sha1(PyObject *self, void*)
{
    return CppPyString(GetCpp<Hashes>(self).SHA1.Result().Value());
}

static PyObject *hashes_get_sha256(PyObject *self, void*)
{
    return CppPyString(GetCpp<Hashes>(self).SHA256.Result().Value());
}

static PyGetSetDef hashes_getset[] = {
    {"md5",hashes_get_md5,0,"The MD5Sum of the file as a string."},
    {"sha1",hashes_get_sha1,0,"The SHA1Sum of the file as a string."},
    {"sha256",hashes_get_sha256,0,"The SHA256Sum of the file as a string."},
    {}
};

static char *hashes_doc =
    "Hashes([object: (bytes, file)])\n\n"
    "Calculate hashes for the given object. It can be used to create all\n"
    "supported hashes for a file.\n\n"
    "The parameter *object* can be a bytes (3.X) / str (2.X) object, or an\n"
    "object providing the fileno() method or an integer describing a file\n"
    "descriptor.";

PyTypeObject PyHashes_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.Hashes",                  // tp_name
    sizeof(CppPyObject<Hashes>),       // tp_basicsize
    0,                                 // tp_itemsize
    // Methods
    CppDealloc<Hashes>,                // tp_dealloc
    0,                                 // tp_print
    0,                                 // tp_getattr
    0,                                 // tp_setattr
    0,                                 // tp_compare
    0,                                 // tp_repr
    0,                                 // tp_as_number
    0,                                 // tp_as_sequence
    0,                                 // tp_as_mapping
    0,                                 // tp_hash
    0,                                 // tp_call
    0,                                 // tp_str
    0,                                 // tp_getattro
    0,                                 // tp_setattro
    0,                                 // tp_as_buffer
    Py_TPFLAGS_DEFAULT |               // tp_flags
    Py_TPFLAGS_BASETYPE,
    hashes_doc,                        // tp_doc
    0,                                 // tp_traverse
    0,                                 // tp_clear
    0,                                 // tp_richcompare
    0,                                 // tp_weaklistoffset
    0,                                 // tp_iter
    0,                                 // tp_iternext
    0,                                 // tp_methods
    0,                                 // tp_members
    hashes_getset,                     // tp_getset
    0,                                 // tp_base
    0,                                 // tp_dict
    0,                                 // tp_descr_get
    0,                                 // tp_descr_set
    0,                                 // tp_dictoffset
    hashes_init,                       // tp_init
    0,                                 // tp_alloc
    hashes_new,                        // tp_new
};
