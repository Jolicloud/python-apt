/* acquire.cc - Wrapper for pkgAcquire.
 *
 * Copyright 2004-2009 Canonical Ltd
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

#include <apt-pkg/acquire-item.h>
#include <apt-pkg/acquire-worker.h>


static PyObject *acquireworker_get_current_item(PyObject *self, void *closure)
{
    pkgAcquire::Worker *worker = GetCpp<pkgAcquire::Worker*>(self);
    pkgAcquire::ItemDesc *desc = worker->CurrentItem;
    if (desc == NULL) {
        Py_RETURN_NONE;
    }
    PyObject *PyAcq  = GetOwner<pkgAcquire::Worker*>(self);
    PyObject *PyItem = PyAcquireItem_FromCpp(desc->Owner, false, PyAcq);
    PyObject *PyDesc = PyAcquireItemDesc_FromCpp(desc, false, PyItem);
    Py_XDECREF(PyItem);
    return PyDesc;
}

static PyObject *acquireworker_get_status(PyObject *self, void *closure)
{
    return CppPyString(GetCpp<pkgAcquire::Worker*>(self)->Status);
}

static PyObject *acquireworker_get_current_size(PyObject *self, void *closure)
{
    return Py_BuildValue("k",GetCpp<pkgAcquire::Worker*>(self)->CurrentSize);
}

static PyObject *acquireworker_get_total_size(PyObject *self, void *closure)
{
    return Py_BuildValue("k",GetCpp<pkgAcquire::Worker*>(self)->TotalSize);
}

static PyObject *acquireworker_get_resumepoint(PyObject *self, void *closure)
{
    return Py_BuildValue("k",GetCpp<pkgAcquire::Worker*>(self)->ResumePoint);
}

static PyGetSetDef acquireworker_getset[] = {
    {"current_item",acquireworker_get_current_item},
    {"status",acquireworker_get_status},
    {"current_size",acquireworker_get_current_size},
    {"total_size",acquireworker_get_total_size},
    {"resumepoint",acquireworker_get_resumepoint},
    {NULL}
};


PyTypeObject PyAcquireWorker_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.AcquireWorker",                // tp_name
    sizeof(CppPyObject<pkgAcquire::Worker*>),// tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    CppDealloc<pkgAcquire::Worker*>, // tp_dealloc
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
    Py_TPFLAGS_DEFAULT|                  // tp_flags
    Py_TPFLAGS_HAVE_GC,
    0,                 // tp_doc
    CppTraverse<pkgAcquire::Worker*>, // tp_traverse
    CppClear<pkgAcquire::Worker*>,   // tp_clear
    0,                                    // tp_richcompare
    0,                                    // tp_weaklistoffset
    0,                                    // tp_iter
    0,                                    // tp_iternext
    0,                                    // tp_methods
    0,                                    // tp_members
    acquireworker_getset,                 // tp_getset
};


static pkgAcquire::ItemDesc* acquireitemdesc_tocpp(PyObject *self) {
    pkgAcquire::ItemDesc *item = GetCpp<pkgAcquire::ItemDesc*>(self);
    if (item == NULL)
        PyErr_SetString(PyExc_ValueError, "Acquire has been shutdown");
    return item;
}

static PyObject *acquireitemdesc_get_uri(PyObject *self, void *closure)
{
    pkgAcquire::ItemDesc *item = acquireitemdesc_tocpp(self);
    return item ? CppPyString(item->URI) : NULL;
}
static PyObject *acquireitemdesc_get_description(PyObject *self, void *closure)
{
    pkgAcquire::ItemDesc *item = acquireitemdesc_tocpp(self);
    return item ? CppPyString(item->Description) : NULL;
}
static PyObject *acquireitemdesc_get_shortdesc(PyObject *self, void *closure)
{
    pkgAcquire::ItemDesc *item = acquireitemdesc_tocpp(self);
    return item ? CppPyString(item->ShortDesc) : NULL;
}
static PyObject *acquireitemdesc_get_owner(CppPyObject<pkgAcquire::ItemDesc*> *self, void *closure)
{
    if (self->Owner != NULL) {
        Py_INCREF(self->Owner);
        return self->Owner;
    }
    else if (self->Object) {
        self->Owner = PyAcquireItem_FromCpp(self->Object->Owner, false, NULL);
        Py_INCREF(self->Owner);
        return self->Owner;
    }
    Py_RETURN_NONE;
}

static PyGetSetDef acquireitemdesc_getset[] = {
    {"uri",acquireitemdesc_get_uri,0,"The URI from which to download this item."},
    {"description",acquireitemdesc_get_description},
    {"shortdesc",acquireitemdesc_get_shortdesc},
    {"owner",(getter)acquireitemdesc_get_owner},
    {NULL}
};

static char *acquireitemdesc_doc =
    "Represent an AcquireItemDesc";

PyTypeObject PyAcquireItemDesc_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.AcquireItemDesc",                // tp_name
    sizeof(CppPyObject<pkgAcquire::ItemDesc*>),// tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    CppDealloc<pkgAcquire::ItemDesc*>, // tp_dealloc
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
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    (Py_TPFLAGS_DEFAULT |                // tp_flags
    Py_TPFLAGS_HAVE_GC),
    acquireitemdesc_doc,                 // tp_doc
    CppTraverse<pkgAcquire::ItemDesc*>,// tp_traverse
    CppClear<pkgAcquire::ItemDesc*>, // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    0,                                   // tp_methods
    0,                                   // tp_members
    acquireitemdesc_getset,              // tp_getset
    0,                                   // tp_base
    0,                                   // tp_dict
    0,                                   // tp_descr_get
    0,                                   // tp_descr_set
    0,                                   // tp_dictoffset
    0,                                   // tp_init
    0,                                   // tp_alloc
    0,                                   // tp_new
};

static PyObject *PkgAcquireRun(PyObject *Self,PyObject *Args)
{
    pkgAcquire *fetcher = GetCpp<pkgAcquire*>(Self);

    int pulseInterval = 500000;
    if (PyArg_ParseTuple(Args, "|i", &pulseInterval) == 0)
        return 0;

    pkgAcquire::RunResult run = fetcher->Run(pulseInterval);

    return HandleErrors(Py_BuildValue("i",run));
}


static PyObject *PkgAcquireShutdown(PyObject *Self,PyObject *Args)
{
    pkgAcquire *fetcher = GetCpp<pkgAcquire*>(Self);
    if (PyArg_ParseTuple(Args, "") == 0)
        return 0;
    fetcher->Shutdown();
    Py_INCREF(Py_None);
    return HandleErrors(Py_None);
}



static PyMethodDef PkgAcquireMethods[] = {
    {"run",PkgAcquireRun,METH_VARARGS,"Run the fetcher"},
    {"shutdown",PkgAcquireShutdown, METH_VARARGS,"Shutdown the fetcher"},
    {}
};

#define fetcher (GetCpp<pkgAcquire*>(Self))
static PyObject *PkgAcquireGetTotalNeeded(PyObject *Self,void*)
{
    return Py_BuildValue("d", fetcher->TotalNeeded());
}
static PyObject *PkgAcquireGetFetchNeeded(PyObject *Self,void*)
{
    return Py_BuildValue("d", fetcher->FetchNeeded());
}
static PyObject *PkgAcquireGetPartialPresent(PyObject *Self,void*)
{
    return Py_BuildValue("d", fetcher->PartialPresent());
}
#undef fetcher

static PyObject *PkgAcquireGetWorkers(PyObject *self, void *closure)
{
    PyObject *List = PyList_New(0);
    pkgAcquire *Owner = GetCpp<pkgAcquire*>(self);
    PyObject *PyWorker = NULL;
    for (pkgAcquire::Worker *Worker = Owner->WorkersBegin();
            Worker != 0; Worker = Owner->WorkerStep(Worker)) {
        PyWorker = PyAcquireWorker_FromCpp(Worker, false, self);
        PyList_Append(List, PyWorker);
        Py_DECREF(PyWorker);
    }
    return List;
}
static PyObject *PkgAcquireGetItems(PyObject *Self,void*)
{
    pkgAcquire *fetcher = GetCpp<pkgAcquire*>(Self);
    PyObject *List = PyList_New(0);
    PyObject *Obj;
    for (pkgAcquire::ItemIterator I = fetcher->ItemsBegin();
            I != fetcher->ItemsEnd(); I++) {
        Obj = PyAcquireItem_FromCpp(*I, false, Self);
        PyList_Append(List,Obj);
        Py_DECREF(Obj);
    }
    return List;
}

static PyGetSetDef PkgAcquireGetSet[] = {
    {"fetch_needed",PkgAcquireGetFetchNeeded},
    {"items",PkgAcquireGetItems},
    {"workers",PkgAcquireGetWorkers},
    {"partial_present",PkgAcquireGetPartialPresent},
    {"total_needed",PkgAcquireGetTotalNeeded},
    {}
};

static PyObject *PkgAcquireNew(PyTypeObject *type,PyObject *Args,PyObject *kwds)
{
    pkgAcquire *fetcher;

    PyObject *pyFetchProgressInst = NULL;
    char *kwlist[] = {"progress", 0};
    if (PyArg_ParseTupleAndKeywords(Args,kwds,"|O",kwlist,&pyFetchProgressInst) == 0)
        return 0;

    PyFetchProgress *progress = 0;
    if (pyFetchProgressInst != NULL) {
        // FIXME: memleak?
        progress = new PyFetchProgress();
        progress->setCallbackInst(pyFetchProgressInst);
        fetcher = new pkgAcquire(progress);
    }
    else {
        fetcher = new pkgAcquire();
    }

    PyObject *FetcherObj = CppPyObject_NEW<pkgAcquire*>(NULL, type, fetcher);

    if (progress != 0)
        progress->setPyAcquire(FetcherObj);
    // prepare our map of items.
    return FetcherObj;
}

/**
 * Create a new apt_pkg.Acquire Python object from the pkgAcquire object.
 */
PyObject *PyAcquire_FromCpp(pkgAcquire *fetcher, bool Delete, PyObject *owner) {
    CppPyObject<pkgAcquire*> *obj = CppPyObject_NEW<pkgAcquire*>(owner, &PyAcquire_Type, fetcher);
    obj->NoDelete = (!Delete);
    return obj;
}

static char *doc_PkgAcquire =
    "Acquire(progress: apt_pkg.AcquireProgress) -> Acquire() object.\n\n"
    "Create a new acquire object. The parameter *progress* can be used to\n"
    "specify an apt_pkg.AcquireProgress() object, which will display the\n"
    "progress of the fetching.";

PyTypeObject PyAcquire_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_pkg.Acquire",                   // tp_name
    sizeof(CppPyObject<pkgAcquire*>),    // tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    CppDeallocPtr<pkgAcquire*>,          // tp_dealloc
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
    _PyAptObject_getattro,               // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    (Py_TPFLAGS_DEFAULT |                // tp_flags
    Py_TPFLAGS_BASETYPE),
    doc_PkgAcquire,                      // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    PkgAcquireMethods,                   // tp_methods
    0,                                   // tp_members
    PkgAcquireGetSet,                    // tp_getset
    0,                                   // tp_base
    0,                                   // tp_dict
    0,                                   // tp_descr_get
    0,                                   // tp_descr_set
    0,                                   // tp_dictoffset
    0,                                   // tp_init
    0,                                   // tp_alloc
    PkgAcquireNew,                       // tp_new
};

#ifdef COMPAT_0_7
PyObject *GetAcquire(PyObject *Self,PyObject *Args)
{
    if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
       PyErr_WarnEx(PyExc_DeprecationWarning,"apt_pkg.GetAcquire() is deprecated."
                    " Please see apt_pkg.Acquire() for the replacement.", 1);
    return PkgAcquireNew(&PyAcquire_Type,Args,0);
}
#endif

