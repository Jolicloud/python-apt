// -*- mode: cpp; mode: fold -*-
// Description                                /*{{{*/
// $Id: metaindex.cc,v 1.2 2003/12/26 17:04:22 mdz Exp $
/* ######################################################################

   metaindex - Wrapper for the metaIndex functions

   ##################################################################### */
                                    /*}}}*/
// Include Files                            /*{{{*/
#include "generic.h"
#include "apt_pkgmodule.h"

#include <apt-pkg/metaindex.h>

#include <Python.h>

static PyObject *MetaIndexGetURI(PyObject *Self,void*) {
    metaIndex *meta = GetCpp<metaIndex*>(Self);
    return Safe_FromString(meta->GetURI().c_str());
}

static PyObject *MetaIndexGetDist(PyObject *Self,void*) {
    metaIndex *meta = GetCpp<metaIndex*>(Self);
    return Safe_FromString(meta->GetDist().c_str());
}

static PyObject *MetaIndexGetIsTrusted(PyObject *Self,void*) {
    metaIndex *meta = GetCpp<metaIndex*>(Self);
    return Py_BuildValue("i",(meta->IsTrusted()));
}

static PyObject *MetaIndexGetIndexFiles(PyObject *Self,void*) {
    metaIndex *meta = GetCpp<metaIndex*>(Self);
    PyObject *List = PyList_New(0);
    vector<pkgIndexFile *> *indexFiles = meta->GetIndexFiles();
    for (vector<pkgIndexFile *>::const_iterator I = indexFiles->begin();
        I != indexFiles->end(); I++)
    {
        CppPyObject<pkgIndexFile*> *Obj;
        Obj = CppPyObject_NEW<pkgIndexFile*>(Self, &PyIndexFile_Type,*I);
        // Do not delete pkgIndexFile*, they are managed by metaIndex.
        Obj->NoDelete = true;
        PyList_Append(List,Obj);
        Py_DECREF(Obj);
    }
    return List;
}

static PyGetSetDef MetaIndexGetSet[] = {
   {"dist",MetaIndexGetDist},
   {"index_files",MetaIndexGetIndexFiles},
   {"is_trusted",MetaIndexGetIsTrusted},
   {"uri",MetaIndexGetURI},
   {}
};

#define S(x) (x ? x : "")
static PyObject *MetaIndexRepr(PyObject *Self)
{
    metaIndex *meta = GetCpp<metaIndex*>(Self);
    return PyString_FromFormat("<%s object: type='%s', uri:'%s' dist='%s' "
                               "is_trusted='%i'>", Self->ob_type->tp_name,
                               S(meta->GetType()),  meta->GetURI().c_str(),
                               meta->GetDist().c_str(), meta->IsTrusted());
}
#undef S

PyTypeObject PyMetaIndex_Type =
{
   PyVarObject_HEAD_INIT(&PyType_Type, 0)
   "apt_pkg.MetaIndex",                    // tp_name
   sizeof(CppPyObject<metaIndex*>),   // tp_basicsize
   0,                                      // tp_itemsize
   // Methods
   CppDeallocPtr<metaIndex*>,         // tp_dealloc
   0,                                      // tp_print
   0,                                      // tp_getattr
   0,                                      // tp_setattr
   0,                                      // tp_compare
   MetaIndexRepr,                          // tp_repr
   0,                                      // tp_as_number
   0,                                      // tp_as_sequence
   0,                                      // tp_as_mapping
   0,                                      // tp_hash
   0,                                      // tp_call
   0,                                      // tp_str
   _PyAptObject_getattro,                  // tp_getattro
   0,                                      // tp_setattro
   0,                                      // tp_as_buffer
   Py_TPFLAGS_DEFAULT,                     // tp_flags
   "apt_pkg.MetaIndex Object",             // tp_doc
   0,                                      // tp_traverse
   0,                                      // tp_clear
   0,                                      // tp_richcompare
   0,                                      // tp_weaklistoffset
   0,                                      // tp_iter
   0,                                      // tp_iternext
   0,                                      // tp_methods
   0,                                      // tp_members
   MetaIndexGetSet,                        // tp_getset
};
