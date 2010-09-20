/* hashstring.cc - Wrapper around HashString
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

static PyObject *hashstring_new(PyTypeObject *type,PyObject *Args,
                                PyObject *kwds)
{
    char *Type = NULL;
    char *Hash = NULL;
    char *kwlist[] = {"type", "hash",  NULL};
    if (PyArg_ParseTupleAndKeywords(Args, kwds, "s|s:__new__", kwlist, &Type,
                                    &Hash) == 0)
        return 0;
    CppPyObject<HashString*> *PyObj = CppPyObject_NEW<HashString*>(NULL, type);
    if (Hash)
        PyObj->Object = new HashString(Type,Hash);
    else // Type is the combined form now (i.e. type:hash)
        PyObj->Object = new HashString(Type);
    return PyObj;
}

static PyObject *hashstring_repr(PyObject *self)
{
    HashString *hash = GetCpp<HashString*>(self);
    return PyString_FromFormat("<%s object: \"%s\">", self->ob_type->tp_name,
                               hash->toStr().c_str());
}

static PyObject *hashstring_str(PyObject *self)
{
    HashString *hash = GetCpp<HashString*>(self);
    return CppPyString(hash->toStr());
}

static PyObject *hashstring_get_hashtype(PyObject *self)
{
    HashString *hash = GetCpp<HashString*>(self);
    return CppPyString(hash->HashType());
}

static char *hashstring_verify_file_doc =
    "verify_file(filename: str) -> bool\n\n"
    "Verify that the file indicated by filename matches the hash.";

static PyObject *hashstring_verify_file(PyObject *self,PyObject *args)
{
    HashString *hash = GetCpp<HashString*>(self);
    char *filename;
    if (PyArg_ParseTuple(args, "s:verify_file", &filename) == 0)
        return 0;
    return PyBool_FromLong(hash->VerifyFile(filename));
}

static PyMethodDef hashstring_methods[] = {
    {"verify_file",hashstring_verify_file,METH_VARARGS,
     hashstring_verify_file_doc},
    {NULL}
};

static PyGetSetDef hashstring_getset[] = {
    {"hashtype",(getter)hashstring_get_hashtype,0,
     "The type of the hash, as a string (possible: MD5Sum,SHA1,SHA256)."},
    {NULL}
};

static char *hashstring_doc =
    "HashString(type, hash) OR HashString('type:hash')\n\n"
    "Create a new HashString object. The first form allows you to specify\n"
    "a type and a hash, and the second form a single string where type and\n"
    "hash are separated by a colon, e.g.::\n\n"
    "   HashString('MD5Sum', '6cc1b6e6655e3555ac47e5b5fe26d04e')\n\n"
    "Valid options for 'type' are: MD5Sum, SHA1, SHA256.";
PyTypeObject PyHashString_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.HashString",              // tp_name
    sizeof(CppPyObject<HashString*>),  // tp_basicsize
    0,                                 // tp_itemsize
    // Methods
    CppDeallocPtr<HashString*>,        // tp_dealloc
    0,                                 // tp_print
    0,                                 // tp_getattr
    0,                                 // tp_setattr
    0,                                 // tp_compare
    hashstring_repr,                   // tp_repr
    0,                                 // tp_as_number
    0,                                 // tp_as_sequence
    0,                                 // tp_as_mapping
    0,                                 // tp_hash
    0,                                 // tp_call
    hashstring_str,                    // tp_str
    0,                                 // tp_getattro
    0,                                 // tp_setattro
    0,                                 // tp_as_buffer
    Py_TPFLAGS_DEFAULT |               // tp_flags
    Py_TPFLAGS_BASETYPE,
    hashstring_doc,                    // tp_doc
    0,                                 // tp_traverse
    0,                                 // tp_clear
    0,                                 // tp_richcompare
    0,                                 // tp_weaklistoffset
    0,                                 // tp_iter
    0,                                 // tp_iternext
    hashstring_methods,                // tp_methods
    0,                                 // tp_members
    hashstring_getset,                 // tp_getset
    0,                                 // tp_base
    0,                                 // tp_dict
    0,                                 // tp_descr_get
    0,                                 // tp_descr_set
    0,                                 // tp_dictoffset
    0,                                 // tp_init
    0,                                 // tp_alloc
    hashstring_new,                    // tp_new
};
