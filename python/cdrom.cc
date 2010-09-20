/* cdrom.cc - Wrapper for pkgCdrom.
 *
 * Copyright 2004-2009 Canonical Ltd.
 * Copyright 2009 Julian Andres Klode <jak@debian.org>
 *
 * Authors: Michael Vogt
 *          Julian Andres Klode
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
#include "progress.h"

#include <apt-pkg/cdrom.h>

static char *cdrom_add_doc =
    "add(progress: apt_pkg.CdromProgress) -> bool\n\n"
    "Add the given CD-ROM to the sources.list. Returns True on success, may\n"
    "raise an error on failure or return False.";
static PyObject *cdrom_add(PyObject *Self,PyObject *Args)
{
    pkgCdrom &Cdrom = GetCpp<pkgCdrom>(Self);

    PyObject *pyCdromProgressInst = 0;
    if (PyArg_ParseTuple(Args, "O", &pyCdromProgressInst) == 0) {
        return 0;
    }

    PyCdromProgress progress;
    progress.setCallbackInst(pyCdromProgressInst);

    bool res = Cdrom.Add(&progress);

    return HandleErrors(PyBool_FromLong(res));
}

static char *cdrom_ident_doc =
    "ident(progress: apt_pkg.CdromProgress) -> str\n\n"
    "Try to identify the CD-ROM and if successful return the identity as a\n"
    "string. Otherwise, return None or raise an error.";
static PyObject *cdrom_ident(PyObject *Self,PyObject *Args)
{
    pkgCdrom &Cdrom = GetCpp<pkgCdrom>(Self);
    PyObject *pyCdromProgressInst = 0;
    if (PyArg_ParseTuple(Args, "O", &pyCdromProgressInst) == 0) {
        return 0;
    }

    PyCdromProgress progress;
    progress.setCallbackInst(pyCdromProgressInst);

    string ident;
    bool res = Cdrom.Ident(ident, &progress);

    if (res)
        return CppPyString(ident);
    else {
        Py_INCREF(Py_None);
        return HandleErrors(Py_None);
    }
}

#ifdef COMPAT_0_7
static PyObject *cdrom_ident_old(PyObject *Self,PyObject *Args)
{
    if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
       PyErr_WarnEx(PyExc_DeprecationWarning, "Method 'Ident' of the "
                    "'apt_pkg.Cdrom' object is deprecated, use 'ident' instead.",
                    1);
    pkgCdrom &Cdrom = GetCpp<pkgCdrom>(Self);

    PyObject *pyCdromProgressInst = 0;
    if (PyArg_ParseTuple(Args, "O", &pyCdromProgressInst) == 0) {
        return 0;
    }

    PyCdromProgress progress;
    progress.setCallbackInst(pyCdromProgressInst);

    string ident;
    bool res = Cdrom.Ident(ident, &progress);

    PyObject *result = Py_BuildValue("(bs)", res, ident.c_str());

    return HandleErrors(result);
}
#endif

static PyMethodDef cdrom_methods[] = {
    {"add",cdrom_add,METH_VARARGS,cdrom_add_doc},
    {"ident",cdrom_ident,METH_VARARGS,cdrom_ident_doc},
#ifdef COMPAT_0_7
    {"Ident",cdrom_ident_old,METH_VARARGS,"DEPRECATED. DO NOT USE"},
#endif
    {}
};

static PyObject *cdrom_new(PyTypeObject *type,PyObject *Args,PyObject *kwds)
{
    return CppPyObject_NEW<pkgCdrom>(NULL, type);
}

static char *cdrom_doc =
    "Cdrom()\n\n"
    "Cdrom objects can be used to identify Debian installation media and to\n"
    "add them to /etc/apt/sources.list.";
PyTypeObject PyCdrom_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.Cdrom",                     // tp_name
    sizeof(CppPyObject<pkgCdrom>),       // tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    CppDealloc<pkgCdrom>,                // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    0,                                   // tp_repr
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                 // tp_as_mapping
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    _PyAptObject_getattro,               // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_DEFAULT |                 // tp_flags
    Py_TPFLAGS_BASETYPE,
    cdrom_doc,                           // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    cdrom_methods,                       // tp_methods
    0,                                   // tp_members
    0,                                   // tp_getset
    0,                                   // tp_base
    0,                                   // tp_dict
    0,                                   // tp_descr_get
    0,                                   // tp_descr_set
    0,                                   // tp_dictoffset
    0,                                   // tp_init
    0,                                   // tp_alloc
    cdrom_new,                           // tp_new
};

#ifdef COMPAT_0_7
PyObject *GetCdrom(PyObject *Self,PyObject *Args)
{
    if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
       PyErr_WarnEx(PyExc_DeprecationWarning, "apt_pkg.GetCdrom() is deprecated. "
                    "Please see apt_pkg.Cdrom() for the replacement.", 1);
    return cdrom_new(&PyCdrom_Type,Args,0);
}
#endif
