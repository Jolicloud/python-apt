// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: generic.h,v 1.4 2002/03/10 05:45:34 mdz Exp $
/* ######################################################################

   generic - Some handy functions to make integration a tad simpler

   Python needs this little _HEAD tacked onto the front of the object..
   This complicates the integration with C++. We use some templates to
   make that quite transparent to us. It would have been nice if Python
   internally used a page from the C++ ref counting book to hide its little
   header from the world, but it doesn't.

   The CppPyObject has the target object and the Python header, this is
   needed to ensure proper alignment.
   GetCpp returns the C++ object from a PyObject.
   CppPyObject_NEW creates the Python object and then uses placement new
     to init the C++ class.. This is good for simple situations and as an
     example on how to do it in other more specific cases.
   CppPyObject_Dealloc should be used in the Type as the destructor
     function.
   HandleErrors converts errors from the internal _error stack into Python
     exceptions and makes sure the _error stack is empty.

   ##################################################################### */
									/*}}}*/
#ifndef GENERIC_H
#define GENERIC_H

#include <Python.h>
#include <string>
#include <iostream>
#include <new>
#include <cstdlib>

#if PYTHON_API_VERSION < 1013
typedef int Py_ssize_t;
#endif

/* Define compatibility for Python 3.
 *
 * We will use the names PyString_* to refer to the default string type
 * of the current Python version (PyString on 2.X, PyUnicode on 3.X).
 *
 * When we really need unicode strings, we will use PyUnicode_* directly, as
 * long as it exists in Python 2 and Python 3.
 *
 * When we want bytes in Python 3, we use PyBytes*_ instead of PyString_* and
 * define aliases from PyBytes_* to PyString_* for Python 2.
 */

#if PY_MAJOR_VERSION >= 3
#define PyString_Check PyUnicode_Check
#define PyString_FromString PyUnicode_FromString
#define PyString_FromStringAndSize PyUnicode_FromStringAndSize
#define PyString_AsString PyUnicode_AsString
#define PyString_FromFormat PyUnicode_FromFormat
#define PyString_Type PyUnicode_Type
#define PyInt_Check PyLong_Check
#define PyInt_AsLong PyLong_AsLong
// Force 0.7 compatibility to be off in Python 3 builds
#undef COMPAT_0_7
#else
// Compatibility for Python 2.5 and previous.
#if (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION <= 5)
#define PyBytes_Check PyString_Check
#define PyBytes_AS_STRING PyString_AS_STRING
#define PyBytes_AsString PyString_AsString
#define PyBytes_AsStringAndSize PyString_AsStringAndSize
#define PyBytes_FromStringAndSize PyString_FromStringAndSize
#define PyVarObject_HEAD_INIT(type, size) PyObject_HEAD_INIT(type) size,
#endif
#endif

// Hacks to make Python 2.4 build.
#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION <= 4
#define PyErr_WarnEx(cat,msg,stacklevel) PyErr_Warn(cat,msg)
#endif


static inline const char *PyUnicode_AsString(PyObject *op) {
    // Convert to bytes object, using the default encoding.
    PyObject *bytes = PyUnicode_AsEncodedString(op,0,0);
    return bytes ? PyBytes_AS_STRING(bytes) : 0;
}

// Convert any type of string based object to a const char.
#if PY_MAJOR_VERSION < 3
static inline const char *PyObject_AsString(PyObject *object) {
    if (PyBytes_Check(object))
        return PyBytes_AsString(object);
    else if (PyUnicode_Check(object))
        return PyUnicode_AsString(object);
    else
        PyErr_SetString(PyExc_TypeError, "Argument must be str.");
    return 0;
}
#else
static inline const char *PyObject_AsString(PyObject *object) {
    if (PyUnicode_Check(object) == 0) {
        PyErr_SetString(PyExc_TypeError, "Argument must be str.");
        return 0;
    }
    return PyUnicode_AsString(object);
}
#endif

template <class T> struct CppPyObject : public PyObject
{
   // We are only using CppPyObject and friends as dumb structs only, ie the
   // c'tor is never called.
   // However if T doesn't have a default c'tor C++ doesn't generate one for
   // CppPyObject (since it can't know how it should initialize Object).
   //
   // This causes problems then in CppPyObject, for which C++ can't create
   // a c'tor that calls the base class c'tor (which causes a compilation
   // error).
   // So basically having the c'tor here removes the need for T to have a
   // default c'tor, which is not always desireable.
   CppPyObject() { };

   // The owner of the object. The object keeps a reference to it during its
   // lifetime.
   PyObject *Owner;

   // Flag which causes the underlying object to not be deleted.
   bool NoDelete;

   // The underlying C++ object.
   T Object;
};

template <class T>
inline T &GetCpp(PyObject *Obj)
{
   return ((CppPyObject<T> *)Obj)->Object;
}

template <class T>
inline PyObject *GetOwner(PyObject *Obj)
{
   return ((CppPyObject<T> *)Obj)->Owner;
}


template <class T>
inline CppPyObject<T> *CppPyObject_NEW(PyObject *Owner,PyTypeObject *Type)
{
   #ifdef ALLOC_DEBUG
   std::cerr << "=== ALLOCATING " << Type->tp_name << "+ ===\n";
   #endif
   CppPyObject<T> *New = (CppPyObject<T>*)Type->tp_alloc(Type, 0);
   new (&New->Object) T;
   New->Owner = Owner;
   Py_XINCREF(Owner);
   return New;
}

template <class T,class A>
inline CppPyObject<T> *CppPyObject_NEW(PyObject *Owner, PyTypeObject *Type,A const &Arg)
{
   #ifdef ALLOC_DEBUG
   std::cerr << "=== ALLOCATING " << Type->tp_name << "+ ===\n";
   #endif
   CppPyObject<T> *New = (CppPyObject<T>*)Type->tp_alloc(Type, 0);
   new (&New->Object) T(Arg);
   New->Owner = Owner;
   Py_XINCREF(Owner);
   return New;
}

// Traversal and Clean for  objects
template <class T>
int CppTraverse(PyObject *self, visitproc visit, void* arg) {
    Py_VISIT(((CppPyObject<T> *)self)->Owner);
    return 0;
}

template <class T>
int CppClear(PyObject *self) {
    Py_CLEAR(((CppPyObject<T> *)self)->Owner);
    return 0;
}

template <class T>
void CppDealloc(PyObject *iObj)
{
   #ifdef ALLOC_DEBUG
   std::cerr << "=== DEALLOCATING " << iObj->ob_type->tp_name << "+ ===\n";
   #endif
   CppPyObject<T> *Obj = (CppPyObject<T> *)iObj;
   if (!((CppPyObject<T>*)Obj)->NoDelete)
      Obj->Object.~T();
   CppClear<T>(iObj);
   iObj->ob_type->tp_free(iObj);
}


template <class T>
void CppDeallocPtr(PyObject *iObj)
{
   #ifdef ALLOC_DEBUG
   std::cerr << "=== DEALLOCATING " << iObj->ob_type->tp_name << "*+ ===\n";
   #endif
   CppPyObject<T> *Obj = (CppPyObject<T> *)iObj;
   if (!((CppPyObject<T>*)Obj)->NoDelete)
      delete Obj->Object;
   CppClear<T>(iObj);
   iObj->ob_type->tp_free(iObj);
}

inline PyObject *CppPyString(std::string Str)
{
   return PyString_FromStringAndSize(Str.c_str(),Str.length());
}

inline PyObject *Safe_FromString(const char *Str)
{
   if (Str == 0)
      return PyString_FromString("");
   return PyString_FromString(Str);
}

// Convert _error into Python exceptions
PyObject *HandleErrors(PyObject *Res = 0);

// Convert a list of strings to a char **
const char **ListToCharChar(PyObject *List,bool NullTerm = false);
PyObject *CharCharToList(const char **List,unsigned long Size = 0);

# ifdef COMPAT_0_7
PyObject *_PyAptObject_getattro(PyObject *self, PyObject *attr);
# else
#  define _PyAptObject_getattro 0
# endif

#endif
