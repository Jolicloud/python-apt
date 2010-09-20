/*
 * acquire-item.cc - Wrapper around pkgAcquire::Item and pkgAcqFile.
 *
 * Copyright 2004-2009 Canonical Ltd.
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

#include "generic.h"
#include "apt_pkgmodule.h"

#include <apt-pkg/acquire-item.h>
#include <map>

using namespace std;

inline pkgAcquire::Item *acquireitem_tocpp(PyObject *self)
{
    pkgAcquire::Item *itm = GetCpp<pkgAcquire::Item*>(self);
    if (itm == 0)
        PyErr_SetString(PyExc_ValueError, "Acquire() has been shut down or "
                        "the AcquireFile() object has been deallocated.");
    return itm;
}

static PyObject *acquireitem_get_complete(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? PyBool_FromLong(item->Complete) : 0;
}

static PyObject *acquireitem_get_desc_uri(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? CppPyString(item->DescURI()) : 0;
}

static PyObject *acquireitem_get_destfile(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? CppPyString(item->DestFile) : 0;
}


static PyObject *acquireitem_get_error_text(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? CppPyString(item->ErrorText) : 0;
}

static PyObject *acquireitem_get_filesize(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? Py_BuildValue("i", item->FileSize) : 0;
}

static PyObject *acquireitem_get_id(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? Py_BuildValue("k", item->ID) : 0;
}

static PyObject *acquireitem_get_mode(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? Py_BuildValue("s", item->Mode) : 0;
}

static PyObject *acquireitem_get_is_trusted(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? PyBool_FromLong(item->IsTrusted()) : 0;
}

static PyObject *acquireitem_get_local(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? PyBool_FromLong(item->Local) : 0;
}

static PyObject *acquireitem_get_partialsize(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? Py_BuildValue("i", item->PartialSize) : 0;
}

static PyObject *acquireitem_get_status(PyObject *self, void *closure)
{
    pkgAcquire::Item *item = acquireitem_tocpp(self);
    return item ? Py_BuildValue("i", item->Status) : 0;
}

static int acquireitem_set_id(PyObject *self, PyObject *value, void *closure)
{
    pkgAcquire::Item *Itm = acquireitem_tocpp(self);
    if (Itm == 0)
        return -1;
    if (PyLong_Check(value)) {
        Itm->ID = PyLong_AsLong(value);
    }
    else if (PyInt_Check(value)) {
        Itm->ID = PyInt_AsLong(value);
    }
    else {
        PyErr_SetString(PyExc_TypeError, "value must be integer.");
        return -1;
    }
    return 0;
}


static PyGetSetDef acquireitem_getset[] = {
    {"complete",acquireitem_get_complete},
    {"desc_uri",acquireitem_get_desc_uri},
    {"destfile",acquireitem_get_destfile},
    {"error_text",acquireitem_get_error_text},
    {"filesize",acquireitem_get_filesize},
    {"id",acquireitem_get_id,acquireitem_set_id},
    {"mode",acquireitem_get_mode},
    {"is_trusted",acquireitem_get_is_trusted},
    {"local",acquireitem_get_local},
    {"partialsize",acquireitem_get_partialsize},
    {"status",acquireitem_get_status},
    {}
};

static PyObject *acquireitem_repr(PyObject *Self)
{
    pkgAcquire::Item *Itm = acquireitem_tocpp(Self);
    if (Itm == 0)
        return 0;
    return PyString_FromFormat("<%s object: "
                               "Status: %i Complete: %i Local: %i IsTrusted: %i "
                               "FileSize: %lu DestFile:'%s' "
                               "DescURI: '%s' ID:%lu ErrorText: '%s'>",
                               Self->ob_type->tp_name,
                               Itm->Status, Itm->Complete, Itm->Local, Itm->IsTrusted(),
                               Itm->FileSize, Itm->DestFile.c_str(),  Itm->DescURI().c_str(),
                               Itm->ID,Itm->ErrorText.c_str());
}

static void acquireitem_dealloc(PyObject *self)
{
    CppDeallocPtr<pkgAcquire::Item*>(self);
}

PyTypeObject PyAcquireItem_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.AcquireItem",         // tp_name
    sizeof(CppPyObject<pkgAcquire::Item*>),   // tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    acquireitem_dealloc,                  // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    acquireitem_repr,                     // tp_repr
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    _PyAptObject_getattro,               // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_HAVE_GC,                  // tp_flags
    "AcquireItem Object",                // tp_doc
    CppTraverse<pkgAcquire::Item*>, // tp_traverse
    CppClear<pkgAcquire::Item*>,    // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    0,                                   // tp_methods
    0,                                   // tp_members
    acquireitem_getset,                   // tp_getset
};

static PyObject *acquirefile_new(PyTypeObject *type, PyObject *Args, PyObject * kwds)
{
    PyObject *pyfetcher;
    char *uri, *md5, *descr, *shortDescr, *destDir, *destFile;
    int size = 0;
    uri = md5 = descr = shortDescr = destDir = destFile = "";

    char *kwlist[] = {"owner","uri", "md5", "size", "descr", "short_descr",
                      "destdir", "destfile", NULL
                     };

    if (PyArg_ParseTupleAndKeywords(Args, kwds, "O!s|sissss", kwlist,
                                    &PyAcquire_Type, &pyfetcher, &uri, &md5,
                                    &size, &descr, &shortDescr, &destDir, &destFile) == 0)
        return 0;

    pkgAcquire *fetcher = GetCpp<pkgAcquire*>(pyfetcher);
    pkgAcqFile *af = new pkgAcqFile(fetcher,  // owner
                                    uri, // uri
                                    md5,  // md5
                                    size,   // size
                                    descr, // descr
                                    shortDescr,
                                    destDir,
                                    destFile); // short-desc
    CppPyObject<pkgAcqFile*> *AcqFileObj = CppPyObject_NEW<pkgAcqFile*>(pyfetcher, type);
    AcqFileObj->Object = af;
    return AcqFileObj;
}


static char *acquirefile_doc =
    "AcquireFile(owner, uri[, md5, size, descr, short_descr, destdir,"
    "destfile]) -> New AcquireFile() object\n\n"
    "The parameter *owner* refers to an apt_pkg.Acquire() object. You can use\n"
    "*destdir* OR *destfile* to specify the destination directory/file.";

PyTypeObject PyAcquireFile_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.AcquireFile",                // tp_name
    sizeof(CppPyObject<pkgAcqFile*>),// tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    acquireitem_dealloc,                 // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    0,                                   // tp_repr
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,	                                 // tp_as_mapping
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_DEFAULT |                 // tp_flags
    Py_TPFLAGS_BASETYPE |
    Py_TPFLAGS_HAVE_GC,
    acquirefile_doc,                     // tp_doc
    CppTraverse<pkgAcqFile*>,       // tp_traverse
    CppClear<pkgAcqFile*>,          // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    0,                                   // tp_methods
    0,                                   // tp_members
    0,                                   // tp_getset
    &PyAcquireItem_Type,                 // tp_base
    0,                                   // tp_dict
    0,                                   // tp_descr_get
    0,                                   // tp_descr_set
    0,                                   // tp_dictoffset
    0,                                   // tp_init
    0,                                   // tp_alloc
    acquirefile_new,                     // tp_new
};

#ifdef COMPAT_0_7
char *doc_GetPkgAcqFile =
    "GetPkgAcqFile(pkgAquire, uri[, md5, size, descr, shortDescr, destDir, destFile]) -> PkgAcqFile\n";
PyObject *GetPkgAcqFile(PyObject *Self, PyObject *Args, PyObject * kwds)
{
    if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
       PyErr_WarnEx(PyExc_DeprecationWarning, "apt_pkg.GetPkgAcqFile() is "
                    "deprecated. Please see apt_pkg.AcquireFile() for the "
                    "replacement", 1);
    PyObject *pyfetcher;
    char *uri, *md5, *descr, *shortDescr, *destDir, *destFile;
    int size = 0;
    uri = md5 = descr = shortDescr = destDir = destFile = "";

    char * kwlist[] = {"owner","uri", "md5", "size", "descr", "shortDescr",
                       "destDir", "destFile", NULL
                      };

    if (PyArg_ParseTupleAndKeywords(Args, kwds, "O!s|sissss", kwlist,
                                    &PyAcquire_Type, &pyfetcher, &uri, &md5,
                                    &size, &descr, &shortDescr, &destDir, &destFile) == 0)
        return 0;

    pkgAcquire *fetcher = GetCpp<pkgAcquire*>(pyfetcher);
    pkgAcqFile *af = new pkgAcqFile(fetcher,  // owner
                                    uri, // uri
                                    md5,  // md5
                                    size,   // size
                                    descr, // descr
                                    shortDescr,
                                    destDir,
                                    destFile); // short-desc
    CppPyObject<pkgAcqFile*> *AcqFileObj = CppPyObject_NEW<pkgAcqFile*>(NULL, &PyAcquireFile_Type);
    AcqFileObj->Object = af;
    AcqFileObj->NoDelete = true;

    return AcqFileObj;
}
#endif
