// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: tag.cc,v 1.3 2002/02/26 01:36:15 mdz Exp $
/* ######################################################################

   Tag - Binding for the RFC 822 tag file parser

   Upon reflection I have make the TagSection wrapper look like a map..
   The other option was to use a sequence (which nicely matches the internal
   storage) but really makes no sense to a Python Programmer.. One
   specialized lookup is provided, the FindFlag lookup - as well as the
   usual set of duplicate things to match the C++ interface.

   The TagFile interface is also slightly different, it has a built in
   internal TagSection object that is used. Do not hold onto a reference
   to a TagSection and let TagFile go out of scope! The underlying storage
   for the section will go away and it will seg.

   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "generic.h"
#include "apt_pkgmodule.h"

#include <apt-pkg/tagfile.h>

#include <stdio.h>
#include <iostream>
#include <Python.h>

using namespace std;
									/*}}}*/
/* We need to keep a private copy of the data.. */
struct TagSecData : public CppPyObject<pkgTagSection>
{
   char *Data;
};

// The owner of the TagFile is a Python file object.
struct TagFileData : public CppPyObject<pkgTagFile>
{
   TagSecData *Section;
   FileFd Fd;
};

// Traversal and Clean for owned objects
int TagFileTraverse(PyObject *self, visitproc visit, void* arg) {
    Py_VISIT(((TagFileData *)self)->Section);
    Py_VISIT(((TagFileData *)self)->Owner);
    return 0;
}

int TagFileClear(PyObject *self) {
    Py_CLEAR(((TagFileData *)self)->Section);
    Py_CLEAR(((TagFileData *)self)->Owner);
    return 0;
}


									/*}}}*/
// TagSecFree - Free a Tag Section					/*{{{*/
// ---------------------------------------------------------------------
/* */
void TagSecFree(PyObject *Obj)
{
   TagSecData *Self = (TagSecData *)Obj;
   delete [] Self->Data;
   CppDealloc<pkgTagSection>(Obj);
}
									/*}}}*/
// TagFileFree - Free a Tag File					/*{{{*/
// ---------------------------------------------------------------------
/* */
void TagFileFree(PyObject *Obj)
{
   #ifdef ALLOC_DEBUG
   std::cerr << "=== DEALLOCATING " << Obj->ob_type->tp_name << "^ ===\n";
   #endif
   TagFileData *Self = (TagFileData *)Obj;
   Py_CLEAR(Self->Section);
   Self->Object.~pkgTagFile();
   Self->Fd.~FileFd();
   Py_CLEAR(Self->Owner);
   Obj->ob_type->tp_free(Obj);
}
									/*}}}*/

// Tag Section Wrappers							/*{{{*/
static char *doc_Find = "Find(Name) -> String/None";
static PyObject *TagSecFind(PyObject *Self,PyObject *Args)
{
   char *Name = 0;
   char *Default = 0;
   if (PyArg_ParseTuple(Args,"s|z",&Name,&Default) == 0)
      return 0;

   const char *Start;
   const char *Stop;
   if (GetCpp<pkgTagSection>(Self).Find(Name,Start,Stop) == false)
   {
      if (Default == 0)
	 Py_RETURN_NONE;
      return PyString_FromString(Default);
   }
   return PyString_FromStringAndSize(Start,Stop-Start);
}

static char *doc_FindRaw = "FindRaw(Name) -> String/None";
static PyObject *TagSecFindRaw(PyObject *Self,PyObject *Args)
{
   char *Name = 0;
   char *Default = 0;
   if (PyArg_ParseTuple(Args,"s|z",&Name,&Default) == 0)
      return 0;

   unsigned Pos;
   if (GetCpp<pkgTagSection>(Self).Find(Name,Pos) == false)
   {
      if (Default == 0)
	 Py_RETURN_NONE;
      return PyString_FromString(Default);
   }

   const char *Start;
   const char *Stop;
   GetCpp<pkgTagSection>(Self).Get(Start,Stop,Pos);

   return PyString_FromStringAndSize(Start,Stop-Start);
}

static char *doc_FindFlag = "FindFlag(Name) -> integer/none";
static PyObject *TagSecFindFlag(PyObject *Self,PyObject *Args)
{
   char *Name = 0;
   if (PyArg_ParseTuple(Args,"s",&Name) == 0)
      return 0;

   unsigned long Flag = 0;
   if (GetCpp<pkgTagSection>(Self).FindFlag(Name,Flag,1) == false)
   {
      Py_INCREF(Py_None);
      return Py_None;
   }
   return Py_BuildValue("i",Flag);
}

// Map access, operator []
static PyObject *TagSecMap(PyObject *Self,PyObject *Arg)
{
   if (PyString_Check(Arg) == 0)
   {
      PyErr_SetNone(PyExc_TypeError);
      return 0;
   }

   const char *Start;
   const char *Stop;
   if (GetCpp<pkgTagSection>(Self).Find(PyString_AsString(Arg),Start,Stop) == false)
   {
      PyErr_SetString(PyExc_KeyError,PyString_AsString(Arg));
      return 0;
   }

   return PyString_FromStringAndSize(Start,Stop-Start);
}

// len() operation
static Py_ssize_t TagSecLength(PyObject *Self)
{
   pkgTagSection &Sec = GetCpp<pkgTagSection>(Self);
   return Sec.Count();
}

// Look like a mapping
static char *doc_Keys = "keys() -> List";
static PyObject *TagSecKeys(PyObject *Self,PyObject *Args)
{
   pkgTagSection &Tags = GetCpp<pkgTagSection>(Self);
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   // Convert the whole configuration space into a list
   PyObject *List = PyList_New(0);
   for (unsigned int I = 0; I != Tags.Count(); I++)
   {
      const char *Start;
      const char *Stop;
      Tags.Get(Start,Stop,I);
      const char *End = Start;
      for (; End < Stop && *End != ':'; End++);

      PyObject *Obj;
      PyList_Append(List,Obj = PyString_FromStringAndSize(Start,End-Start));
      Py_DECREF(Obj);
   }
   return List;
}

#if PY_MAJOR_VERSION < 3
static char *doc_Exists = "Exists(Name) -> integer";
static PyObject *TagSecExists(PyObject *Self,PyObject *Args)
{
   char *Name = 0;
   if (PyArg_ParseTuple(Args,"s",&Name) == 0)
      return 0;

   const char *Start;
   const char *Stop;
   if (GetCpp<pkgTagSection>(Self).Find(Name,Start,Stop) == false)
      return Py_BuildValue("i",0);
   return Py_BuildValue("i",1);
}
#endif

static int TagSecContains(PyObject *Self,PyObject *Arg)
{
   if (PyString_Check(Arg) == 0)
       return 0;
   const char *Name = PyString_AsString(Arg);
   const char *Start;
   const char *Stop;
   if (GetCpp<pkgTagSection>(Self).Find(Name,Start,Stop) == false)
      return 0;
   return 1;
}

static char *doc_Bytes = "Bytes() -> integer";
static PyObject *TagSecBytes(PyObject *Self,PyObject *Args)
{
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   return Py_BuildValue("i",GetCpp<pkgTagSection>(Self).size());
}

static PyObject *TagSecStr(PyObject *Self)
{
   const char *Start;
   const char *Stop;
   GetCpp<pkgTagSection>(Self).GetSection(Start,Stop);
   return PyString_FromStringAndSize(Start,Stop-Start);
}
									/*}}}*/
// TagFile Wrappers							/*{{{*/
static char *doc_Step = "Step() -> Integer\n0 means EOF.";
static PyObject *TagFileStep(PyObject *Self,PyObject *Args)
{
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   TagFileData &Obj = *(TagFileData *)Self;
   if (Obj.Object.Step(Obj.Section->Object) == false)
      return HandleErrors(Py_BuildValue("i",0));

   return HandleErrors(Py_BuildValue("i",1));
}

// TagFile Wrappers							/*{{{*/
static PyObject *TagFileNext(PyObject *Self)
{
   TagFileData &Obj = *(TagFileData *)Self;
   // Replace the section.
   Py_CLEAR(Obj.Section);
   Obj.Section = (TagSecData*)(&PyTagSection_Type)->tp_alloc(&PyTagSection_Type, 0);
   new (&Obj.Section->Object) pkgTagSection();
   Obj.Section->Owner = Self;
   Py_INCREF(Obj.Section->Owner);
   Obj.Section->Data = 0;
   if (Obj.Object.Step(Obj.Section->Object) == false)
      return HandleErrors(NULL);

   // Bug-Debian: http://bugs.debian.org/572596
   // Duplicate the data here and scan the duplicated section data; in order
   // to not use any shared storage.
   // TODO: Provide an API in apt-pkg to do this; this is really ugly.

   // Fetch old section data
   const char *Start;
   const char *Stop;
   Obj.Section->Object.GetSection(Start,Stop);
   // Duplicate the data
   Obj.Section->Data = new char[Stop-Start];
   strncpy(Obj.Section->Data, Start, Stop-Start);
   // Rescan it
   Obj.Section->Object.Scan(Obj.Section->Data, Stop-Start);

   Py_INCREF(Obj.Section);
   return HandleErrors(Obj.Section);
}

static PyObject *TagFileIter(PyObject *Self) {
    Py_INCREF(Self);
    return Self;
}

static char *doc_Offset = "Offset() -> Integer";
static PyObject *TagFileOffset(PyObject *Self,PyObject *Args)
{
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;
   return Py_BuildValue("i",((TagFileData *)Self)->Object.Offset());
}

static char *doc_Jump = "Jump(Offset) -> Integer";
static PyObject *TagFileJump(PyObject *Self,PyObject *Args)
{
   int Offset;
   if (PyArg_ParseTuple(Args,"i",&Offset) == 0)
      return 0;

   TagFileData &Obj = *(TagFileData *)Self;
   if (Obj.Object.Jump(Obj.Section->Object,Offset) == false)
      return HandleErrors(Py_BuildValue("i",0));

   return HandleErrors(Py_BuildValue("i",1));
}
									/*}}}*/
// ParseSection - Parse a single section from a tag file		/*{{{*/
// ---------------------------------------------------------------------
static PyObject *TagSecNew(PyTypeObject *type,PyObject *Args,PyObject *kwds) {
   char *Data;
   char *kwlist[] = {"text", 0};
   if (PyArg_ParseTupleAndKeywords(Args,kwds,"s",kwlist,&Data) == 0)
      return 0;

   // Create the object..
   TagSecData *New = (TagSecData*)type->tp_alloc(type, 0);
   new (&New->Object) pkgTagSection();
   New->Data = new char[strlen(Data)+2];
   snprintf(New->Data,strlen(Data)+2,"%s\n",Data);

   if (New->Object.Scan(New->Data,strlen(New->Data)) == false)
   {
      cerr << New->Data << endl;
      Py_DECREF((PyObject *)New);
      PyErr_SetString(PyExc_ValueError,"Unable to parse section data");
      return 0;
   }

   New->Object.Trim();

   return New;
}

#ifdef COMPAT_0_7
char *doc_ParseSection ="ParseSection(Text) -> TagSection() object. Deprecated.";
PyObject *ParseSection(PyObject *self,PyObject *Args)
{
   if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
      PyErr_WarnEx(PyExc_DeprecationWarning, "apt_pkg.ParseSection() is "
                   "deprecated. Please see apt_pkg.TagSection() for the "
                   "replacement.", 1);
    return TagSecNew(&PyTagSection_Type,Args,0);
}
#endif
									/*}}}*/
// ParseTagFile - Parse a tagd file					/*{{{*/
// ---------------------------------------------------------------------
/* This constructs the parser state. */

static PyObject *TagFileNew(PyTypeObject *type,PyObject *Args,PyObject *kwds)
{
   PyObject *File;
   char *kwlist[] = {"file", 0};
   if (PyArg_ParseTupleAndKeywords(Args,kwds,"O",kwlist,&File) == 0)
      return 0;
   int fileno = PyObject_AsFileDescriptor(File);
   if (fileno == -1)
      return 0;

   TagFileData *New = (TagFileData*)type->tp_alloc(type, 0);
   new (&New->Fd) FileFd(fileno,false);
   New->Owner = File;
   Py_INCREF(New->Owner);
   new (&New->Object) pkgTagFile(&New->Fd);

   // Create the section
   New->Section = (TagSecData*)(&PyTagSection_Type)->tp_alloc(&PyTagSection_Type, 0);
   new (&New->Section->Object) pkgTagSection();
   New->Section->Owner = New;
   Py_INCREF(New->Section->Owner);
   New->Section->Data = 0;

   return HandleErrors(New);
}
#ifdef COMPAT_0_7
char *doc_ParseTagFile = "ParseTagFile(File) -> TagFile() object. Deprecated.";
PyObject *ParseTagFile(PyObject *self,PyObject *Args) {
   if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
      PyErr_WarnEx(PyExc_DeprecationWarning, "apt_pkg.ParseTagFile() is "
                   "deprecated. Please see apt_pkg.TagFile() for the "
                   "replacement.", 1);
    return TagFileNew(&PyTagFile_Type,Args,0);
}
#endif
									/*}}}*/
// RewriteSection - Rewrite a section..					/*{{{*/
// ---------------------------------------------------------------------
/* An interesting future extension would be to add a user settable
   order list */
char *doc_RewriteSection =
"RewriteSection(Section,Order,RewriteList) -> String\n"
"\n"
"The section rewriter allows a section to be taken in, have fields added,\n"
"removed or changed and then put back out. During this process the fields\n"
"within the section are sorted to corrispond to a proper order. Order is a\n"
"list of field names with their proper capitialization.\n"
"apt_pkg.RewritePackageOrder and apt_pkg.RewriteSourceOrder are two predefined\n"
"orders.\n"
"RewriteList is a list of tuples. Each tuple is of the form:\n"
"  (Tag,NewValue[,RenamedTo])\n"
"Tag specifies the tag in the source section. NewValue is the new value of\n"
"that tag and the optional RenamedTo field can cause the tag to be changed.\n"
"If NewValue is None then the tag is removed\n"
"Ex. ('Source','apt','Package') is used for .dsc files.";
PyObject *RewriteSection(PyObject *self,PyObject *Args)
{
   PyObject *Section;
   PyObject *Order;
   PyObject *Rewrite;
   if (PyArg_ParseTuple(Args,"O!O!O!",&PyTagSection_Type,&Section,
			&PyList_Type,&Order,&PyList_Type,&Rewrite) == 0)
      return 0;

   // Convert the order list
   const char **OrderList = ListToCharChar(Order,true);

   // Convert the Rewrite list.
   TFRewriteData *List = new TFRewriteData[PySequence_Length(Rewrite)+1];
   memset(List,0,sizeof(*List)*(PySequence_Length(Rewrite)+1));
   for (int I = 0; I != PySequence_Length(Rewrite); I++)
   {
      List[I].NewTag = 0;
      if (PyArg_ParseTuple(PySequence_GetItem(Rewrite,I),"sz|s",
			  &List[I].Tag,&List[I].Rewrite,&List[I].NewTag) == 0)
      {
	 delete [] OrderList;
	 delete [] List;
	 return 0;
      }
   }

   /* This is a glibc extension.. If not running on glibc I'd just take
      this whole function out, it is probably infrequently used */
   char *bp = 0;
   size_t size;
   FILE *F = open_memstream (&bp, &size);

   // Do the rewrite
   bool Res = TFRewrite(F,GetCpp<pkgTagSection>(Section),OrderList,List);
   delete [] OrderList;
   delete [] List;
   fclose(F);

   if (Res == false)
   {
      free(bp);
      return HandleErrors();
   }

   // Return the string
   PyObject *ResObj = PyString_FromStringAndSize(bp,size);
   free(bp);
   return HandleErrors(ResObj);
}
									/*}}}*/

// Method table for the Tag Section object
static PyMethodDef TagSecMethods[] =
{
   // Query
   {"find",TagSecFind,METH_VARARGS,doc_Find},
   {"find_raw",TagSecFindRaw,METH_VARARGS,doc_FindRaw},
   {"find_flag",TagSecFindFlag,METH_VARARGS,doc_FindFlag},
   {"bytes",TagSecBytes,METH_VARARGS,doc_Bytes},

   // Python Special
   {"keys",TagSecKeys,METH_VARARGS,doc_Keys},
#if PY_MAJOR_VERSION < 3
   {"has_key",TagSecExists,METH_VARARGS,doc_Exists},
#endif
   {"get",TagSecFind,METH_VARARGS,doc_Find},
   {}
};


PySequenceMethods TagSecSeqMeth = {0,0,0,0,0,0,0,TagSecContains,0,0};
PyMappingMethods TagSecMapMeth = {TagSecLength,TagSecMap,0};


static char *doc_TagSec = "TagSection(text) -> Create a new object.\n\n"
   "TagSection() objects provide methods to access rfc822-style formatted\n"
   "header sections, like those in debian/control or Packages files.\n\n"
   "TagSection() behave like read-only dictionaries and also provide access\n"
   "to the functions provided by the C++ class (e.g. Find)";
PyTypeObject PyTagSection_Type =
{
   PyVarObject_HEAD_INIT(&PyType_Type, 0)
   "apt_pkg.TagSection",                // tp_name
   sizeof(TagSecData),                  // tp_basicsize
   0,                                   // tp_itemsize
   // Methods
   TagSecFree,                          // tp_dealloc
   0,                                   // tp_print
   0,                                   // tp_getattr
   0,                                   // tp_setattr
   0,                                   // tp_compare
   0,                                   // tp_repr
   0,                                   // tp_as_number
   &TagSecSeqMeth,                      // tp_as_sequence
   &TagSecMapMeth,                      // tp_as_mapping
   0,                                   // tp_hash
   0,                                   // tp_call
   TagSecStr,                           // tp_str
   _PyAptObject_getattro,               // tp_getattro
   0,                                   // tp_setattro
   0,                                   // tp_as_buffer
   (Py_TPFLAGS_DEFAULT |                // tp_flags
    Py_TPFLAGS_BASETYPE |
    Py_TPFLAGS_HAVE_GC),
   doc_TagSec,                          // tp_doc
   CppTraverse<pkgTagSection>,     // tp_traverse
   CppClear<pkgTagSection>,         // tp_clear
   0,                                   // tp_richcompare
   0,                                   // tp_weaklistoffset
   0,                                   // tp_iter
   0,                                   // tp_iternext
   TagSecMethods,                       // tp_methods
   0,                                   // tp_members
   0,                                   // tp_getset
   0,                                   // tp_base
   0,                                   // tp_dict
   0,                                   // tp_descr_get
   0,                                   // tp_descr_set
   0,                                   // tp_dictoffset
   0,                                   // tp_init
   0,                                   // tp_alloc
   TagSecNew,                           // tp_new
};

// Method table for the Tag File object
static PyMethodDef TagFileMethods[] =
{
   // Query
   {"step",TagFileStep,METH_VARARGS,doc_Step},
   {"offset",TagFileOffset,METH_VARARGS,doc_Offset},
   {"jump",TagFileJump,METH_VARARGS,doc_Jump},

   {}
};

// Return the current section.
static PyObject *TagFileGetSection(PyObject *Self,void*) {
   PyObject *Obj = ((TagFileData *)Self)->Section;
   Py_INCREF(Obj);
   return Obj;
}

static PyGetSetDef TagFileGetSet[] = {
    {"section",TagFileGetSection,0,"Return a TagSection.",0},
    {}
};


static char *doc_TagFile = "TagFile(file) -> TagFile() object. \n\n"
   "TagFile() objects provide access to debian control files, which consists\n"
   "of multiple RFC822-like formatted sections.\n\n"
   "To provide access to those sections, TagFile objects provide an iterator\n"
   "which yields TagSection objects for each section.\n\n"
   "TagFile objects also provide another API which uses a shared TagSection\n"
   "object in the 'section' member. The functions step() and jump() can be\n"
   "used to navigate in the file; and offset() tells the current position.\n\n"
   "It is important to not mix the use of both APIs, because this can have\n"
   "unwanted effects.\n\n"
   "The parameter *file* refers to an object providing a fileno() method or\n"
   "a file descriptor (an integer)";

// Type for a Tag File
PyTypeObject PyTagFile_Type =
{
   PyVarObject_HEAD_INIT(&PyType_Type, 0)
   "apt_pkg.TagFile",                   // tp_name
   sizeof(TagFileData),                 // tp_basicsize
   0,                                   // tp_itemsize
   // Methods
   TagFileFree,                         // tp_dealloc
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
    Py_TPFLAGS_BASETYPE |
    Py_TPFLAGS_HAVE_GC),
   doc_TagFile,                         // tp_doc
   TagFileTraverse,                     // tp_traverse
   TagFileClear,                        // tp_clear
   0,                                   // tp_richcompare
   0,                                   // tp_weaklistoffset
   TagFileIter,                         // tp_iter
   TagFileNext,                         // tp_iternext
   TagFileMethods,                      // tp_methods
   0,                                   // tp_members
   TagFileGetSet,                       // tp_getset
   0,                                   // tp_base
   0,                                   // tp_dict
   0,                                   // tp_descr_get
   0,                                   // tp_descr_set
   0,                                   // tp_dictoffset
   0,                                   // tp_init
   0,                                   // tp_alloc
   TagFileNew,                          // tp_new

};
