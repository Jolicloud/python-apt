/*
 * arfile.cc - Wrapper around ExtractTar which behaves like Python's tarfile.
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

#include "generic.h"
#include "apt_instmodule.h"
#include <apt-pkg/extracttar.h>
#include <apt-pkg/error.h>
#include <apt-pkg/dirstream.h>

/**
 * A subclass of pkgDirStream which calls a Python callback.
 *
 * This calls a Python callback in FinishedFile() with the Item as the first
 * argument and the data as the second argument.
 *
 * It can also work without a callback, in which case it just sets the
 * 'py_member' and 'py_data' members. This can be combined with setting
 * 'member' to extract a single member into the memory.
 */
class PyDirStream : public pkgDirStream
{

public:
    PyObject *callback;
    PyObject *py_data;
    // The requested member or NULL.
    const char *member;
    // Set to true if an error occured in the Python callback.
    bool error;
    // Place where the copy of the data is stored.
    char *copy;
    // The size of the copy
    size_t copy_size;

    virtual bool DoItem(Item &Itm,int &Fd);
    virtual bool FinishedFile(Item &Itm,int Fd);
    virtual bool Process(Item &Itm,const unsigned char *Data,
                         unsigned long Size,unsigned long Pos);

    PyDirStream(PyObject *callback, const char *member=0) : callback(callback),
        py_data(0), member(member), error(false), copy(0)
    {
        Py_XINCREF(callback);
    }

    virtual ~PyDirStream() {
        Py_XDECREF(callback);
        Py_XDECREF(py_data);
        delete[] copy;
    }
};

bool PyDirStream::DoItem(Item &Itm, int &Fd)
{
    if (!member || strcmp(Itm.Name, member) == 0) {
        // Allocate a new buffer if the old one is too small.
        if (copy == NULL || copy_size < Itm.Size) {
            delete[] copy;
            copy = new char[Itm.Size];
            copy_size = Itm.Size;
        }
        Fd = -2;
    } else {
        Fd = -1;
    }
    return true;
}

bool PyDirStream::Process(Item &Itm,const unsigned char *Data,
                          unsigned long Size,unsigned long Pos)
{
    memcpy(copy + Pos, Data,Size);
    return true;
}

bool PyDirStream::FinishedFile(Item &Itm,int Fd)
{
    if (member && strcmp(Itm.Name, member) != 0)
        // Skip non-matching Items, if a specific one is requested.
        return true;

    Py_XDECREF(py_data);
    py_data = PyBytes_FromStringAndSize(copy, Itm.Size);

    if (!callback)
        return true;

    // The current member and data.
    CppPyObject<Item> *py_member;
    py_member = CppPyObject_NEW<Item>(0, &PyTarMember_Type);
    // Clone our object, including the strings in it.
    py_member->Object = Itm;
    py_member->Object.Name = new char[strlen(Itm.Name)+1];
    py_member->Object.LinkTarget = new char[strlen(Itm.LinkTarget)+1];
    strcpy(py_member->Object.Name, Itm.Name);
    strcpy(py_member->Object.LinkTarget,Itm.LinkTarget);
    py_member->NoDelete = true;
    error = PyObject_CallFunctionObjArgs(callback, py_member, py_data, 0) == 0;
    // Clear the old objects and create new ones.
    Py_XDECREF(py_member);
    return (!error);
}

void tarmember_dealloc(PyObject *self) {
    // We cloned those strings, delete them again.
    delete[] GetCpp<pkgDirStream::Item>(self).Name;
    delete[] GetCpp<pkgDirStream::Item>(self).LinkTarget;
    CppDealloc<pkgDirStream::Item>(self);
}

// The tarfile.TarInfo interface for our TarMember class.
static PyObject *tarmember_isblk(PyObject *self, PyObject *args)
{
    return PyBool_FromLong(GetCpp<pkgDirStream::Item>(self).Type ==
                           pkgDirStream::Item::BlockDevice);
}
static PyObject *tarmember_ischr(PyObject *self, PyObject *args)
{
    return PyBool_FromLong(GetCpp<pkgDirStream::Item>(self).Type ==
                           pkgDirStream::Item::CharDevice);
}
static PyObject *tarmember_isdev(PyObject *self, PyObject *args)
{
    pkgDirStream::Item::Type_t type = GetCpp<pkgDirStream::Item>(self).Type;
    return PyBool_FromLong(type == pkgDirStream::Item::CharDevice ||
                           type == pkgDirStream::Item::BlockDevice ||
                           type == pkgDirStream::Item::FIFO);
}

static PyObject *tarmember_isdir(PyObject *self, PyObject *args)
{
    return PyBool_FromLong(GetCpp<pkgDirStream::Item>(self).Type ==
                           pkgDirStream::Item::Directory);
}

static PyObject *tarmember_isfifo(PyObject *self, PyObject *args)
{
    return PyBool_FromLong(GetCpp<pkgDirStream::Item>(self).Type ==
                           pkgDirStream::Item::FIFO);
}

static PyObject *tarmember_isfile(PyObject *self, PyObject *args)
{
    return PyBool_FromLong(GetCpp<pkgDirStream::Item>(self).Type ==
                           pkgDirStream::Item::File);
}
static PyObject *tarmember_islnk(PyObject *self, PyObject *args)
{
    return PyBool_FromLong(GetCpp<pkgDirStream::Item>(self).Type ==
                           pkgDirStream::Item::HardLink);
}
static PyObject *tarmember_isreg(PyObject *self, PyObject *args)
{
    return tarmember_isfile(self, NULL);
}
static PyObject *tarmember_issym(PyObject *self, PyObject *args)
{
    return PyBool_FromLong(GetCpp<pkgDirStream::Item>(self).Type ==
                           pkgDirStream::Item::SymbolicLink);
}

static PyObject *tarmember_get_name(PyObject *self, void *closure)
{
    return PyString_FromString(GetCpp<pkgDirStream::Item>(self).Name);
}

static PyObject *tarmember_get_linkname(PyObject *self, void *closure)
{
    return Safe_FromString(GetCpp<pkgDirStream::Item>(self).LinkTarget);
}

static PyObject *tarmember_get_mode(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<pkgDirStream::Item>(self).Mode);
}

static PyObject *tarmember_get_uid(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<pkgDirStream::Item>(self).UID);
}
static PyObject *tarmember_get_gid(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<pkgDirStream::Item>(self).GID);
}
static PyObject *tarmember_get_size(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<pkgDirStream::Item>(self).Size);
}

static PyObject *tarmember_get_mtime(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<pkgDirStream::Item>(self).MTime);
}

static PyObject *tarmember_get_major(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<pkgDirStream::Item>(self).Major);
}

static PyObject *tarmember_get_minor(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<pkgDirStream::Item>(self).Minor);
}

static PyObject *tarmember_repr(PyObject *self)
{
    return PyString_FromFormat("<%s object: name:'%s'>",
                               self->ob_type->tp_name,
                               GetCpp<pkgDirStream::Item>(self).Name);

}


static PyMethodDef tarmember_methods[] = {
    {"isblk",tarmember_isblk,METH_NOARGS,
        "Determine whether the member is a block device."},
    {"ischr",tarmember_ischr,METH_NOARGS,
     "Determine whether the member is a character device."},
    {"isdev",tarmember_isdev,METH_NOARGS,
     "Determine whether the member is a device (block,character or FIFO)."},
    {"isdir",tarmember_isdir,METH_NOARGS,
     "Determine whether the member is a directory."},
    {"isfifo",tarmember_isfifo,METH_NOARGS,
     "Determine whether the member is a FIFO."},
    {"isfile",tarmember_isfile,METH_NOARGS,
     "Determine whether the member is a regular file."},
    {"islnk",tarmember_islnk,METH_NOARGS,
     "Determine whether the member is a hardlink."},
    {"isreg",tarmember_isreg,METH_NOARGS,
     "Determine whether the member is a regular file, same as isfile()."},
    {"issym",tarmember_issym,METH_NOARGS,
     "Determine whether the member is a symbolic link."},
    {NULL}
};

static PyGetSetDef tarmember_getset[] = {
    {"gid",tarmember_get_gid,0,"The owner's group id"},
    {"linkname",tarmember_get_linkname,0,"The target of the link."},
    {"major",tarmember_get_major,0,"The major ID of the device."},
    {"minor",tarmember_get_minor,0,"The minor ID of the device."},
    {"mode",tarmember_get_mode,0,"The mode (permissions)."},
    {"mtime",tarmember_get_mtime,0,"Last time of modification."},
    {"name",tarmember_get_name,0,"The name of the file."},
    {"size",tarmember_get_size,0,"The size of the file."},
    {"uid",tarmember_get_uid,0,"The owner's user id."},
    {NULL}
};

static const char *tarmember_doc =
    "Represent a single member of a 'tar' archive.\n\n"
    "This class, which has been modelled after 'tarfile.TarInfo', represents\n"
    "information about a given member in an archive.";
PyTypeObject PyTarMember_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_inst.TarMember",                // tp_name
    sizeof(CppPyObject<pkgDirStream::Item>),           // tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    tarmember_dealloc,                   // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    tarmember_repr,                      // tp_repr
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_DEFAULT |                 // tp_flags
    Py_TPFLAGS_HAVE_GC,
    tarmember_doc,                       // tp_doc
    CppTraverse<pkgDirStream::Item>,       // tp_traverse
    CppClear<pkgDirStream::Item>,          // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    tarmember_methods,                   // tp_methods
    0,                                   // tp_members
    tarmember_getset                     // tp_getset
};



static PyObject *tarfile_new(PyTypeObject *type,PyObject *args,PyObject *kwds)
{
    PyObject *file;
    PyTarFileObject *self;
    char *filename;
    int fileno;
    int min = 0;
    int max = 0xFFFFFFFF;
    char *comp = "gzip";

    static char *kwlist[] = {"file","min","max","comp",NULL};
    if (PyArg_ParseTupleAndKeywords(args, kwds, "O|iis", kwlist, &file, &min,
                                    &max,&comp) == 0)
        return 0;

    self = (PyTarFileObject*)CppPyObject_NEW<ExtractTar*>(file,type);

    // We receive a filename.
    if ((filename = (char*)PyObject_AsString(file)))
        new (&self->Fd) FileFd(filename,FileFd::ReadOnly);
    else if ((fileno = PyObject_AsFileDescriptor(file)) != -1) {
        // clear the error set by PyObject_AsString().
        PyErr_Clear();
        new (&self->Fd) FileFd(fileno,false);
    }
    else {
        Py_DECREF(self);
        return 0;
    }

    self->min = min;
    self->Object = new ExtractTar(self->Fd,max,comp);
    if (_error->PendingError() == true)
        return HandleErrors(self);
    return self;
}

static const char *tarfile_extractall_doc =
    "extractall([rootdir: str]) -> True\n\n"
    "Extract the archive in the current directory. The argument 'rootdir'\n"
    "can be used to change the target directory.";
static PyObject *tarfile_extractall(PyObject *self, PyObject *args)
{
    string cwd = SafeGetCWD();
    char *rootdir = 0;
    if (PyArg_ParseTuple(args,"|s:extractall",&rootdir) == 0)
        return 0;

    if (rootdir) {
        if (chdir(rootdir) == -1)
            return PyErr_SetFromErrnoWithFilename(PyExc_OSError, rootdir);
    }

    pkgDirStream Extract;

    ((PyTarFileObject*)self)->Fd.Seek(((PyTarFileObject*)self)->min);
    bool res = GetCpp<ExtractTar*>(self)->Go(Extract);



    if (rootdir) {
        if (chdir(cwd.c_str()) == -1)
            return PyErr_SetFromErrnoWithFilename(PyExc_OSError,
                                                  (char*)cwd.c_str());
    }
    return HandleErrors(PyBool_FromLong(res));
}

static const char *tarfile_go_doc =
    "go(callback: callable[, member: str]) -> True\n\n"
    "Go through the archive and call the callable callback for each\n"
    "member with 2 arguments. The first argument is the TarMember and\n"
    "the second one is the data, as bytes.\n\n"
    "The optional parameter 'member' can be used to specify the member for\n"
    "which call the callback. If not specified, it will be called for all\n"
    "members. If specified and not found, LookupError will be raised.";
static PyObject *tarfile_go(PyObject *self, PyObject *args)
{
    PyObject *callback;
    char *member = 0;
    if (PyArg_ParseTuple(args,"O|s",&callback,&member) == 0)
        return 0;
    if (member && strcmp(member, "") == 0)
        member = 0;
    pkgDirStream Extract;
    PyDirStream stream(callback, member);
    ((PyTarFileObject*)self)->Fd.Seek(((PyTarFileObject*)self)->min);
    bool res = GetCpp<ExtractTar*>(self)->Go(stream);
    if (stream.error)
        return 0;
    if (member && !stream.py_data)
        return PyErr_Format(PyExc_LookupError, "There is no member named '%s'",
                            member);
    return HandleErrors(PyBool_FromLong(res));
}

static const char *tarfile_extractdata_doc =
    "extractdata(member: str) -> bytes\n\n"
    "Return the contents of the member, as a bytes object. Raise\n"
    "LookupError if there is no member with the given name.";
static PyObject *tarfile_extractdata(PyObject *self, PyObject *args)
{
    const char *member;
    if (PyArg_ParseTuple(args,"s",&member) == 0)
        return 0;
    PyDirStream stream(NULL, member);
    ((PyTarFileObject*)self)->Fd.Seek(((PyTarFileObject*)self)->min);
    // Go through the stream.
    GetCpp<ExtractTar*>(self)->Go(stream);

    if (!stream.py_data)
        return PyErr_Format(PyExc_LookupError, "There is no member named '%s'",
                            member);
    if (stream.error) {
        return 0;
    }
    return Py_INCREF(stream.py_data), stream.py_data;
}

static PyMethodDef tarfile_methods[] = {
    {"extractdata",tarfile_extractdata,METH_VARARGS,tarfile_extractdata_doc},
    {"extractall",tarfile_extractall,METH_VARARGS,tarfile_extractall_doc},
    {"go",tarfile_go,METH_VARARGS,tarfile_go_doc},
    {NULL}
};

static PyObject *tarfile_repr(PyObject *self)
{
    return PyString_FromFormat("<%s object: %s>", self->ob_type->tp_name,
                               PyString_AsString(PyObject_Repr(GetOwner<ExtractTar*>(self))));
}

static const char *tarfile_doc =
    "TarFile(file: str/int/file[, min: int, max: int, comp: str])\n\n"
    "The parameter 'file' may be a string specifying the path of a file, or\n"
    "a file-like object providing the fileno() method. It may also be an int\n"
    "specifying a file descriptor (returned by e.g. os.open()).\n\n"
    "The parameter 'min' describes the offset in the file where the archive\n"
    "begins and the parameter 'max' is the size of the archive.\n\n"
    "The compression of the archive is set by the parameter 'comp'. It can\n"
    "be set to any program supporting the -d switch, the default being gzip.";
PyTypeObject PyTarFile_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_inst.TarFile",                  // tp_name
    sizeof(PyTarFileObject),             // tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    CppDealloc<ExtractTar*>,        // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    tarfile_repr,                        // tp_repr
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_DEFAULT |                 // tp_flags
    Py_TPFLAGS_HAVE_GC,
    tarfile_doc,                         // tp_doc
    CppTraverse<ExtractTar*>,       // tp_traverse
    CppClear<ExtractTar*>,          // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    tarfile_methods,                     // tp_methods
    0,                                   // tp_members
    0,                                   // tp_getset
    0,                                   // tp_base
    0,                                   // tp_dict
    0,                                   // tp_descr_get
    0,                                   // tp_descr_set
    0,                                   // tp_dictoffset
    0,                                   // tp_init
    0,                                   // tp_alloc
    tarfile_new                          // tp_new
};
