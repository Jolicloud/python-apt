// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: generic.cc,v 1.1.1.1 2001/02/20 06:32:01 jgg Exp $
/* ######################################################################

   generic - Some handy functions to make integration a tad simpler

   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include "generic.h"
using namespace std;


#include <apt-pkg/error.h>
									/*}}}*/

// HandleErrors - This moves errors from _error to Python Exceptions	/*{{{*/
// ---------------------------------------------------------------------
/* We throw away all warnings and only propogate the first error. */
PyObject *HandleErrors(PyObject *Res)
{
   if (_error->PendingError() == false)
   {
      // Throw away warnings
      _error->Discard();
      return Res;
   }

   if (Res != 0) {
      Py_DECREF(Res);
   }

   string Err;
   int errcnt = 0;
   while (_error->empty() == false)
   {
      string Msg;
      bool Type = _error->PopMessage(Msg);
      if (errcnt > 0)
         Err.append(", ");
      Err.append((Type == true ? "E:" : "W:"));
      Err.append(Msg);
      ++errcnt;
   }
   if (errcnt == 0)
      Err = "Internal Error";
   PyErr_SetString(PyExc_SystemError,Err.c_str());
   return 0;
}

# ifdef COMPAT_0_7
// Helpers for deprecation.

// Given the name of the old attribute, return the name of the new attribute
// in a PyObject.
static PyObject *_PyApt_NewNameForAttribute(const char *attr) {
    // Some exceptions from the standard algorithm.
    if (strcasecmp(attr, "FileName") == 0) return PyString_FromString("filename");
    if (strcasecmp(attr, "DestFile") == 0) return PyString_FromString("destfile");
    if (strcasecmp(attr, "FileSize") == 0) return PyString_FromString("filesize");
    if (strcasecmp(attr, "SubTree") == 0) return PyString_FromString("subtree");
    if (strcasecmp(attr, "ReadPinFile") == 0) return PyString_FromString("read_pinfile");
    if (strcasecmp(attr, "SetReInstall") == 0) return PyString_FromString("set_reinstall");
    if (strcasecmp(attr, "URI") == 0) return PyString_FromString("uri");
    if (strcasecmp(attr, "ArchiveURI") == 0) return PyString_FromString("archive_uri");
    if (strcasecmp(attr, "MD5Hash") == 0) return PyString_FromString("md5_hash");
    if (strcasecmp(attr, "SHA1Hash") == 0) return PyString_FromString("sha1_hash");
    if (strcasecmp(attr, "SHA256Hash") == 0) return PyString_FromString("sha256_hash");
    if (strcasecmp(attr, "UntranslatedDepType") == 0) return PyString_FromString("dep_type_untranslated");
    size_t attrlen = strlen(attr);
    // Reserve the old name + 5, this should reduce resize to a minimum.
    string new_name;
    new_name.reserve(attrlen + 5);
    for(unsigned int i=0; i < attrlen; i++) {
        // Replace all uppercase ASCII characters with their lower-case ones.
        if (attr[i] > 64 && attr[i] < 91) {
            if (i > 0)
                new_name += "_";
            new_name += attr[i] + 32;
        } else {
            new_name += attr[i];
        }
    }
    return CppPyString(new_name);
}

// Handle deprecated attributes by setting a warning and returning the new
// attribute.
PyObject *_PyAptObject_getattro(PyObject *self, PyObject *attr) {
    PyObject *value = PyObject_GenericGetAttr(self, attr);
    if (value == NULL) {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        const char *attrname = PyObject_AsString(attr);
        PyObject *newattr = _PyApt_NewNameForAttribute(attrname);
        value = PyObject_GenericGetAttr(self, newattr);
        if (value != NULL) {
            const char *newattrname = PyString_AsString(newattr);
            const char *cls = self->ob_type->tp_name;
            char *warning_string = new char[strlen(newattrname) + strlen(cls) +
                                            strlen(attrname) + 66];
            sprintf(warning_string, "Attribute '%s' of the '%s' object is "
                    "deprecated, use '%s' instead.", attrname, cls, newattrname);
            if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
                PyErr_WarnEx(PyExc_DeprecationWarning, warning_string, 1);
            delete[] warning_string;
        } else {
            Py_XINCREF(ptype);
            Py_XINCREF(pvalue);
            Py_XINCREF(ptraceback);
            PyErr_Restore(ptype, pvalue, ptraceback);
        }
        Py_DECREF(newattr);
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
    }
    return value;
}
# endif //COMPAT_0_7
									/*}}}*/
// ListToCharChar - Convert a list to an array of char char		/*{{{*/
// ---------------------------------------------------------------------
/* Caller must free the result. 0 on error. */
const char **ListToCharChar(PyObject *List,bool NullTerm)
{
   // Convert the argument list into a char **
   int Length = PySequence_Length(List);
   const char **Res = new const char *[Length + (NullTerm == true?1:0)];
   for (int I = 0; I != Length; I++)
   {
      PyObject *Itm = PySequence_GetItem(List,I);
      if (PyString_Check(Itm) == 0)
      {
	 PyErr_SetNone(PyExc_TypeError);
	 delete [] Res;
	 return 0;
      }
      Res[I] = PyString_AsString(Itm);
   }
   if (NullTerm == true)
      Res[Length] = 0;
   return Res;
}
									/*}}}*/
// CharCharToList - Inverse of the above				/*{{{*/
// ---------------------------------------------------------------------
/* Zero size indicates the list is Null terminated. */
PyObject *CharCharToList(const char **List,unsigned long Size)
{
   if (Size == 0)
   {
      for (const char **I = List; *I != 0; I++)
	 Size++;
   }

   // Convert the whole configuration space into a list
   PyObject *PList = PyList_New(Size);
   for (unsigned long I = 0; I != Size; I++, List++)
      PyList_SetItem(PList,I,PyString_FromString(*List));

   return PList;
}
									/*}}}*/
