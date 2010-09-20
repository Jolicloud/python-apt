// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: sourcelist.cc,v 1.2 2003/12/26 17:04:22 mdz Exp $
/* ######################################################################

   SourcesList - Wrapper for the SourcesList functions

   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include "generic.h"
#include "apt_pkgmodule.h"

#include <apt-pkg/sourcelist.h>

#include <Python.h>
									/*}}}*/



// PkgsourceList Class							/*{{{*/
// ---------------------------------------------------------------------

static char *doc_PkgSourceListFindIndex = "xxx";
static PyObject *PkgSourceListFindIndex(PyObject *Self,PyObject *Args)
{
   pkgSourceList *list = GetCpp<pkgSourceList*>(Self);
   PyObject *pyPkgFileIter;
   CppPyObject<pkgIndexFile*> *pyPkgIndexFile;

   if (PyArg_ParseTuple(Args, "O!", &PyPackageFile_Type,&pyPkgFileIter) == 0)
      return 0;

   pkgCache::PkgFileIterator &i = GetCpp<pkgCache::PkgFileIterator>(pyPkgFileIter);
   pkgIndexFile *index;
   if(list->FindIndex(i, index))
   {
      pyPkgIndexFile = CppPyObject_NEW<pkgIndexFile*>(pyPkgFileIter,&PyIndexFile_Type,index);
      // Do not delete the pkgIndexFile*, it is managed by pkgSourceList.
      pyPkgIndexFile->NoDelete = true;
      return pyPkgIndexFile;
   }

   //&PyIndexFile_Type,&pyPkgIndexFile)

   Py_INCREF(Py_None);
   return Py_None;
}

static char *doc_PkgSourceListReadMainList = "xxx";
static PyObject *PkgSourceListReadMainList(PyObject *Self,PyObject *Args)
{
   pkgSourceList *list = GetCpp<pkgSourceList*>(Self);
   bool res = list->ReadMainList();

   return HandleErrors(Py_BuildValue("b",res));
}

static char *doc_PkgSourceListGetIndexes = "Load the indexes into the fetcher";
static PyObject *PkgSourceListGetIndexes(PyObject *Self,PyObject *Args)
{
   pkgSourceList *list = GetCpp<pkgSourceList*>(Self);

   PyObject *pyFetcher;
   char all = 0;
   if (PyArg_ParseTuple(Args, "O!|b",&PyAcquire_Type,&pyFetcher, &all) == 0)
      return 0;

   pkgAcquire *fetcher = GetCpp<pkgAcquire*>(pyFetcher);
   bool res = list->GetIndexes(fetcher, all);

   return HandleErrors(Py_BuildValue("b",res));
}

static PyMethodDef PkgSourceListMethods[] =
{
   {"find_index",PkgSourceListFindIndex,METH_VARARGS,doc_PkgSourceListFindIndex},
   {"read_main_list",PkgSourceListReadMainList,METH_VARARGS,doc_PkgSourceListReadMainList},
   {"get_indexes",PkgSourceListGetIndexes,METH_VARARGS,doc_PkgSourceListGetIndexes},
   {}
};

static PyObject *PkgSourceListGetList(PyObject *Self,void*)
{
   pkgSourceList *list = GetCpp<pkgSourceList*>(Self);
   PyObject *List = PyList_New(0);
   for (vector<metaIndex *>::const_iterator I = list->begin();
        I != list->end(); I++)
   {
      CppPyObject<metaIndex*> *Obj;
      Obj = CppPyObject_NEW<metaIndex*>(Self, &PyMetaIndex_Type,*I);
      // Never delete metaIndex*, they are managed by the pkgSourceList.
      Obj->NoDelete = true;
      PyList_Append(List,Obj);
      Py_DECREF(Obj);
   }
   return List;
}

static PyGetSetDef PkgSourceListGetSet[] = {
    {"list",PkgSourceListGetList,0,"A list of MetaIndex() objects.",0},
    {}
};

static PyObject *PkgSourceListNew(PyTypeObject *type,PyObject *args,PyObject *kwds)
{
   char *kwlist[] = {0};
   if (PyArg_ParseTupleAndKeywords(args,kwds,"",kwlist) == 0)
      return 0;
   return CppPyObject_NEW<pkgSourceList*>(NULL, type,new pkgSourceList());
}

PyTypeObject PySourceList_Type =
{
   PyVarObject_HEAD_INIT(&PyType_Type, 0)
   "apt_pkg.SourceList",                          // tp_name
   sizeof(CppPyObject<pkgSourceList*>),   // tp_basicsize
   0,                                   // tp_itemsize
   // Methods
   CppDeallocPtr<pkgSourceList*>,       // tp_dealloc
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
   "pkgSourceList Object",              // tp_doc
   0,                                   // tp_traverse
   0,                                   // tp_clear
   0,                                   // tp_richcompare
   0,                                   // tp_weaklistoffset
   0,                                   // tp_iter
   0,                                   // tp_iternext
   PkgSourceListMethods,                // tp_methods
   0,                                   // tp_members
   PkgSourceListGetSet,                 // tp_getset
   0,                                   // tp_base
   0,                                   // tp_dict
   0,                                   // tp_descr_get
   0,                                   // tp_descr_set
   0,                                   // tp_dictoffset
   0,                                   // tp_init
   0,                                   // tp_alloc
   PkgSourceListNew,                    // tp_new
};

#ifdef COMPAT_0_7
PyObject *GetPkgSourceList(PyObject *Self,PyObject *Args)
{
   if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
      PyErr_WarnEx(PyExc_DeprecationWarning, "apt_pkg.GetPkgSourceList() is "
                   "deprecated. Please see apt_pkg.SourceList() for the "
                   "replacement.", 1);
   return PkgSourceListNew(&PySourceList_Type,Args,0);
}
#endif
