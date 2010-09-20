/*
 * arfile.cc - Wrapper around ARArchive and ARArchive::Member.
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
#include "apt_instmodule.h"
#include <apt-pkg/arfile.h>
#include <apt-pkg/error.h>
#include <apt-pkg/sptr.h>
#include <utime.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static PyObject *armember_get_name(PyObject *self, void *closure)
{
    return CppPyString(GetCpp<ARArchive::Member*>(self)->Name);
}

static PyObject *armember_get_mtime(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<ARArchive::Member*>(self)->MTime);
}

static PyObject *armember_get_uid(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<ARArchive::Member*>(self)->UID);
}

static PyObject *armember_get_gid(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<ARArchive::Member*>(self)->GID);
}

static PyObject *armember_get_mode(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<ARArchive::Member*>(self)->Mode);
}

static PyObject *armember_get_size(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<ARArchive::Member*>(self)->Size);
}

static PyObject *armember_get_start(PyObject *self, void *closure)
{
    return Py_BuildValue("k", GetCpp<ARArchive::Member*>(self)->Start);
}

static PyObject *armember_repr(PyObject *self)
{
    return PyString_FromFormat("<%s object: name:'%s'>",
                               self->ob_type->tp_name,
                               GetCpp<ARArchive::Member*>(self)->Name.c_str());
}

static PyGetSetDef armember_getset[] = {
    {"gid",armember_get_gid,0,"The group id of the owner."},
    {"mode",armember_get_mode,0,"The mode of the file."},
    {"mtime",armember_get_mtime,0,"Last time of modification."},
    {"name",armember_get_name,0,"The name of the file."},
    {"size",armember_get_size,0,"The size of the files."},
    {"start",armember_get_start,0,
     "The offset in the archive where the file starts."},
    {"uid",armember_get_uid,0,"The user id of the owner."},
    {NULL}
};

static const char *armember_doc =
    "An ArMember object represents a single file within an AR archive. For\n"
    "Debian packages this can be e.g. control.tar.gz. This class provides\n"
    "information about this file, such as the mode and size.";
PyTypeObject PyArMember_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_inst.ArMember",                 // tp_name
    sizeof(CppPyObject<ARArchive::Member*>),  // tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    CppDeallocPtr<ARArchive::Member*>, // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    armember_repr,                       // tp_repr
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
    armember_doc,                        // tp_doc
    CppTraverse<ARArchive::Member*>,// tp_traverse
    CppClear<ARArchive::Member*>,   // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    0,                                   // tp_iter
    0,                                   // tp_iternext
    0,                                   // tp_methods
    0,                                   // tp_members
    armember_getset,                     // tp_getset
};

// We just add an inline method and should thus be ABI compatible in a way that
// we can simply cast ARArchive instances to PyARArchiveHack.
class PyARArchiveHack : public ARArchive
{
public:
    inline Member *Members() {
        return List;
    }
};

struct PyArArchiveObject : public CppPyObject<PyARArchiveHack*> {
    FileFd Fd;
};

static const char *ararchive_getmember_doc =
    "getmember(name: str) -> ArMember\n\n"
    "Return a ArMember object for the member given by name. Raise\n"
    "LookupError if there is no ArMember with the given name.";
static PyObject *ararchive_getmember(PyArArchiveObject *self, PyObject *arg)
{
    const char *name;
    CppPyObject<ARArchive::Member*> *ret;
    if (! (name = PyObject_AsString(arg)))
        return 0;

    const ARArchive::Member *member = self->Object->FindMember(name);
    if (!member) {
        PyErr_Format(PyExc_LookupError,"No member named '%s'",name);
        return 0;
    }

    // Create our object.
    ret = CppPyObject_NEW<ARArchive::Member*>(self,&PyArMember_Type);
    ret->Object = const_cast<ARArchive::Member*>(member);
    ret->NoDelete = true;
    return ret;
}

static const char *ararchive_extractdata_doc =
    "extractdata(name: str) -> bytes\n\n"
    "Return the contents of the member, as a bytes object. Raise\n"
    "LookupError if there is no ArMember with the given name.";
static PyObject *ararchive_extractdata(PyArArchiveObject *self, PyObject *args)
{
    char *name = 0;
    if (PyArg_ParseTuple(args, "s:extractdata", &name) == 0)
        return 0;
    const ARArchive::Member *member = self->Object->FindMember(name);
    if (!member) {
        PyErr_Format(PyExc_LookupError,"No member named '%s'",name);
        return 0;
    }
    if (!self->Fd.Seek(member->Start))
        return HandleErrors();

    char* value = new char[member->Size];
    self->Fd.Read(value, member->Size, true);
    PyObject *result = PyBytes_FromStringAndSize(value, member->Size);
    delete[] value;
    return result;
}

// Helper class to close the FD automatically.
class IntFD {
    public:
    int fd;
    inline operator int() { return fd; };
    inline IntFD(int fd): fd(fd) { };
    inline ~IntFD() { close(fd); };
};

static PyObject *_extract(FileFd &Fd, const ARArchive::Member *member,
                          const char *dir)
{
    if (!Fd.Seek(member->Start))
        return HandleErrors();

    string outfile_str = flCombine(dir,member->Name);
    char *outfile = (char*)outfile_str.c_str();

    // We are not using FileFd here, because we want to raise OSErrror with
    // the correct errno and filename. IntFD's are closed automatically.
    IntFD outfd(open(outfile, O_NDELAY|O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,
		             member->Mode));
    if (outfd == -1)
        return PyErr_SetFromErrnoWithFilename(PyExc_OSError, outfile);
    if (fchmod(outfd, member->Mode) == -1)
        return PyErr_SetFromErrnoWithFilename(PyExc_OSError, outfile);
    if (fchown(outfd, member->UID, member->GID) != 0 && errno != EPERM)
        return PyErr_SetFromErrnoWithFilename(PyExc_OSError, outfile);

    // Read 4 KiB from the file, until all of the file is read. Deallocated
    // automatically when the function returns.
    SPtrArray<char> value = new char[4096];
    unsigned long size = member->Size;
    unsigned long read = 4096;
    while (size > 0) {
        if (size < read)
            read = size;
        if (!Fd.Read(value, read, true))
            return HandleErrors();
        if (write(outfd, value, read) != (signed)read)
            return PyErr_SetFromErrnoWithFilename(PyExc_OSError, outfile);
        size -= read;
    }
    utimbuf time = {member->MTime, member->MTime};
    if (utime(outfile,&time) == -1)
        return PyErr_SetFromErrnoWithFilename(PyExc_OSError, outfile);
    Py_RETURN_TRUE;
}

static const char *ararchive_extract_doc =
    "extract(name: str[, target: str]) -> bool\n\n"
    "Extract the member given by name into the directory given by target.\n"
    "If the extraction failed, an error is raised. Otherwise, the method\n"
    "returns True if the owner could be set or False if the owner could not\n"
    "be changed. It may also raise LookupError if there is member with\n"
    "the given name.";
static PyObject *ararchive_extract(PyArArchiveObject *self, PyObject *args)
{
    char *name = 0;
    char *target = "";
    if (PyArg_ParseTuple(args, "s|s:extract", &name, &target) == 0)
        return 0;

    const ARArchive::Member *member = self->Object->FindMember(name);

    if (!member) {
        PyErr_Format(PyExc_LookupError,"No member named '%s'",name);
        return 0;
    }
    return _extract(self->Fd, member, target);
}

static const char *ararchive_extractall_doc =
    "extract([target: str]) -> bool\n\n"
    "Extract all into the directory given by target.\n"
    "If the extraction failed, an error is raised. Otherwise, the method\n"
    "returns True if the owner could be set or False if the owner could not\n"
    "be changed.";

static PyObject *ararchive_extractall(PyArArchiveObject *self, PyObject *args)
{
    char *target = "";
    if (PyArg_ParseTuple(args, "|s:extractall", &target) == 0)
        return 0;

    const ARArchive::Member *member = self->Object->Members();

    do {
        if (_extract(self->Fd, member, target) == 0)
            return 0;
    } while ((member = member->Next));
    Py_RETURN_TRUE;
}

static const char *ararchive_gettar_doc =
    "gettar(name: str, comp: str) -> TarFile\n\n"
    "Return a TarFile object for the member given by 'name' which will be\n"
    "decompressed using the compression algorithm given by 'comp'.\n"
    "This is almost equal to:\n\n"
    "   member = arfile.getmember(name)\n"
    "   tarfile = TarFile(file, member.start, member.size, 'gzip')'\n\n"
    "It just opens a new TarFile on the given position in the stream.";
static PyObject *ararchive_gettar(PyArArchiveObject *self, PyObject *args)
{
    const char *name;
    const char *comp;
    if (PyArg_ParseTuple(args, "ss:gettar", &name, &comp) == 0)
        return 0;

    const ARArchive::Member *member = self->Object->FindMember(name);
    if (!member) {
        PyErr_Format(PyExc_LookupError,"No member named '%s'",name);
        return 0;
    }

    PyTarFileObject *tarfile = (PyTarFileObject*)CppPyObject_NEW<ExtractTar*>(self,&PyTarFile_Type);
    new (&tarfile->Fd) FileFd(self->Fd);
    tarfile->min = member->Start;
    tarfile->Object = new ExtractTar(self->Fd, member->Size, comp);
    return HandleErrors(tarfile);
}

static const char *ararchive_getmembers_doc =
    "getmembers() -> list\n\n"
    "Return a list of all members in the AR archive.";
static PyObject *ararchive_getmembers(PyArArchiveObject *self)
{
    PyObject *list = PyList_New(0);
    ARArchive::Member *member = self->Object->Members();
    do {
        CppPyObject<ARArchive::Member*> *ret;
        ret = CppPyObject_NEW<ARArchive::Member*>(self,&PyArMember_Type);
        ret->Object = member;
        ret->NoDelete = true;
        PyList_Append(list, ret);
        Py_DECREF(ret);
    } while ((member = member->Next));
    return list;
}

static const char *ararchive_getnames_doc =
    "getnames() -> list\n\n"
    "Return a list of the names of all members in the AR archive.";
static PyObject *ararchive_getnames(PyArArchiveObject *self)
{
    PyObject *list = PyList_New(0);
    ARArchive::Member *member = self->Object->Members();
    do {
        PyObject *item = CppPyString(member->Name);
        PyList_Append(list, item);
        Py_DECREF(item);
    } while ((member = member->Next));
    return list;
}

// Just run getmembers() and return an iterator over the list.
static PyObject *ararchive_iter(PyArArchiveObject *self) {
    PyObject *members = ararchive_getmembers(self);
    PyObject *iter = PyObject_GetIter(members);
    Py_DECREF(members);
    return iter;
}

static PyMethodDef ararchive_methods[] = {
    {"getmember",(PyCFunction)ararchive_getmember,METH_O,
     ararchive_getmember_doc},
    {"gettar",(PyCFunction)ararchive_gettar,METH_VARARGS,
     ararchive_gettar_doc},
    {"extractdata",(PyCFunction)ararchive_extractdata,METH_VARARGS,
     ararchive_extractdata_doc},
    {"extract",(PyCFunction)ararchive_extract,METH_VARARGS,
     ararchive_extract_doc},
    {"extractall",(PyCFunction)ararchive_extractall,METH_VARARGS,
     ararchive_extractall_doc},
    {"getmembers",(PyCFunction)ararchive_getmembers,METH_NOARGS,
     ararchive_getmembers_doc},
    {"getnames",(PyCFunction)ararchive_getnames,METH_NOARGS,
     ararchive_getnames_doc},
    {NULL}
};

static PyObject *ararchive_new(PyTypeObject *type, PyObject *args,
                               PyObject *kwds)
{
    PyObject *file;
    PyArArchiveObject *self;
    char *filename = 0;
    int fileno;
    if (PyArg_ParseTuple(args,"O:__new__",&file) == 0)
        return 0;

    // We receive a filename.
    if ((filename = (char*)PyObject_AsString(file))) {
        self = (PyArArchiveObject *)CppPyObject_NEW<ARArchive*>(0,type);
        new (&self->Fd) FileFd(filename,FileFd::ReadOnly);
    }
    // We receive a file object.
    else if ((fileno = PyObject_AsFileDescriptor(file)) != -1) {
        // Clear the error set by PyObject_AsString().
        PyErr_Clear();
        self = (PyArArchiveObject *)CppPyObject_NEW<ARArchive*>(file,type);
        new (&self->Fd) FileFd(fileno,false);
    }
    else {
        return 0;
    }
    self->Object = (PyARArchiveHack*)new ARArchive(self->Fd);
    if (_error->PendingError() == true)
        return HandleErrors();
    return self;
}

static void ararchive_dealloc(PyObject *self)
{
    ((PyArArchiveObject *)(self))->Fd.~FileFd();
    CppDeallocPtr<ARArchive*>(self);
}

// Return bool or -1 (exception).
static int ararchive_contains(PyObject *self, PyObject *arg)
{
    const char *name = PyObject_AsString(arg);
    if (!name)
        return -1;
    return (GetCpp<ARArchive*>(self)->FindMember(name) != 0);
}

static PySequenceMethods ararchive_as_sequence = {
    0,0,0,0,0,0,0,ararchive_contains,0,0
};

static PyMappingMethods ararchive_as_mapping = {
    0,(PyCFunction)ararchive_getmember,0
};

static const char *ararchive_doc =
    "ArArchive(file: str/int/file)\n\n"
    "An ArArchive object represents an archive in the 4.4 BSD AR format, \n"
    "which is used for e.g. deb packages.\n\n"
    "The parameter 'file' may be a string specifying the path of a file, or\n"
    "a file-like object providing the fileno() method. It may also be an int\n"
    "specifying a file descriptor (returned by e.g. os.open()).\n"
    "The recommended way is to pass in the path to the file.";

PyTypeObject PyArArchive_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_inst.ArArchive",                // tp_name
    sizeof(PyArArchiveObject),           // tp_basicsize
    0,                                   // tp_itemsize
    // Methods
    ararchive_dealloc,                   // tp_dealloc
    0,                                   // tp_print
    0,                                   // tp_getattr
    0,                                   // tp_setattr
    0,                                   // tp_compare
    0,                                   // tp_repr
    0,                                   // tp_as_number
    &ararchive_as_sequence,              // tp_as_sequence
    &ararchive_as_mapping,               // tp_as_mapping
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_DEFAULT |                 // tp_flags
    Py_TPFLAGS_HAVE_GC,
    ararchive_doc,                       // tp_doc
    CppTraverse<ARArchive*>,        // tp_traverse
    CppClear<ARArchive*>,           // tp_clear
    0,                                   // tp_richcompare
    0,                                   // tp_weaklistoffset
    (getiterfunc)ararchive_iter,         // tp_iter
    0,                                   // tp_iternext
    ararchive_methods,                   // tp_methods
    0,                                   // tp_members
    0,                                   // tp_getset
    0,                                   // tp_base
    0,                                   // tp_dict
    0,                                   // tp_descr_get
    0,                                   // tp_descr_set
    0,                                   // tp_dictoffset
    0,                                   // tp_init
    0,                                   // tp_alloc
    ararchive_new                        // tp_new
};

/**
 * Representation of a Debian package.
 *
 * This does not resemble debDebFile in apt-inst, but instead is a subclass
 * of ArFile which adds properties for the control.tar.{lzma,bz2,gz} and
 * data.tar.{lzma,bz2,gz} members which return TarFile objects. It also adds
 * a descriptor 'version' which returns the content of 'debian-binary'.
 *
 * We are using it this way as it seems more natural to represent this special
 * kind of AR archive as an AR archive with some extras.
 */
struct PyDebFileObject : PyArArchiveObject {
    PyObject *data;
    PyObject *control;
    PyObject *debian_binary;
};

static PyObject *debfile_get_data(PyDebFileObject *self)
{
    return Py_INCREF(self->data), self->data;
}

static PyObject *debfile_get_control(PyDebFileObject *self)
{
    return Py_INCREF(self->control), self->control;
}

static PyObject *debfile_get_debian_binary(PyDebFileObject *self)
{
    return Py_INCREF(self->debian_binary), self->debian_binary;
}

static PyObject *_gettar(PyDebFileObject *self, const ARArchive::Member *m,
                         const char *comp)
{
    if (!m)
        return 0;
    PyTarFileObject *tarfile = (PyTarFileObject*)CppPyObject_NEW<ExtractTar*>(self,&PyTarFile_Type);
    new (&tarfile->Fd) FileFd(self->Fd);
    tarfile->min = m->Start;
    tarfile->Object = new ExtractTar(self->Fd, m->Size, comp);
    return tarfile;
}



static PyObject *debfile_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyDebFileObject *self = (PyDebFileObject*)ararchive_new(type, args, kwds);
    if (self == NULL)
        return NULL;

    // DebFile
    self->control = _gettar(self, self->Object->FindMember("control.tar.gz"),
                            "gzip");
    if (!self->control)
        return PyErr_Format(PyExc_SystemError, "No debian archive, missing %s",
                            "control.tar.gz");

    self->data = _gettar(self, self->Object->FindMember("data.tar.gz"),
                         "gzip");
    if (!self->data)
        self->data = _gettar(self, self->Object->FindMember("data.tar.bz2"),
                             "bzip2");
    if (!self->data)
        self->data = _gettar(self, self->Object->FindMember("data.tar.lzma"),
                             "lzma");
    if (!self->data)
        return PyErr_Format(PyExc_SystemError, "No debian archive, missing %s",
                            "data.tar.gz or data.tar.bz2 or data.tar.lzma");


    const ARArchive::Member *member = self->Object->FindMember("debian-binary");
    if (!member)
        return PyErr_Format(PyExc_SystemError, "No debian archive, missing %s",
                            "debian-binary");

    if (!self->Fd.Seek(member->Start))
        return HandleErrors();

    char* value = new char[member->Size];
    self->Fd.Read(value, member->Size, true);
    self->debian_binary = PyBytes_FromStringAndSize(value, member->Size);
    delete[] value;
    return self;
}

static int debfile_traverse(PyObject *_self, visitproc visit, void* arg)
{
    PyDebFileObject *self = (PyDebFileObject*)_self;
    Py_VISIT(self->data);
    Py_VISIT(self->control);
    Py_VISIT(self->debian_binary);
    return PyArArchive_Type.tp_traverse(self, visit, arg);
}

static int debfile_clear(PyObject *_self) {
    PyDebFileObject *self = (PyDebFileObject*)_self;
    Py_CLEAR(self->data);
    Py_CLEAR(self->control);
    Py_CLEAR(self->debian_binary);
    return PyArArchive_Type.tp_clear(self);
}

static void debfile_dealloc(PyObject *self) {
    debfile_clear((PyDebFileObject *)self);
    PyArArchive_Type.tp_dealloc(self);
}

static PyGetSetDef debfile_getset[] = {
    {"control",(getter)debfile_get_control,0,
     "The TarFile object associated with the control.tar.gz member."},
    {"data",(getter)debfile_get_data,0,
     "The TarFile object associated with the data.tar.{gz,bz2,lzma} member."},
    {"debian_binary",(getter)debfile_get_debian_binary,0,
     "The package version, as contained in debian-binary."},
    {NULL}
};

static const char *debfile_doc =
    "DebFile(file: str/int/file)\n\n"
    "A DebFile object represents a file in the .deb package format.\n\n"
    "The parameter 'file' may be a string specifying the path of a file, or\n"
    "a file-like object providing the fileno() method. It may also be an int\n"
    "specifying a file descriptor (returned by e.g. os.open()).\n"
    "The recommended way is to pass in the path to the file.\n\n"
    "It differs from ArArchive by providing the members 'control', 'data'\n"
    "and 'version' for accessing the control.tar.gz,data.tar.{gz,bz2,lzma}\n"
    ",debian-binary members in the archive.";

PyTypeObject PyDebFile_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "apt_inst.DebFile",                // tp_name
    sizeof(PyDebFileObject),           // tp_basicsize
    0,                                 // tp_itemsize
    // Methods
    debfile_dealloc,                   // tp_dealloc
    0,                                 // tp_print
    0,                                 // tp_getattr
    0,                                 // tp_setattr
    0,                                 // tp_compare
    0,                                 // tp_repr
    0,                                 // tp_as_number
    0,                                 // tp_as_sequence
    0,                                 // tp_as_mapping
    0,                                 // tp_hash
    0,                                 // tp_call
    0,                                 // tp_str
    0,                                 // tp_getattro
    0,                                 // tp_setattro
    0,                                 // tp_as_buffer
    Py_TPFLAGS_DEFAULT |               // tp_flags
    Py_TPFLAGS_HAVE_GC,
    debfile_doc,                       // tp_doc
    debfile_traverse,                  // tp_traverse
    debfile_clear,                     // tp_clear
    0,                                 // tp_richcompare
    0,                                 // tp_weaklistoffset
    0,                                 // tp_iter
    0,                                 // tp_iternext
    0,                                 // tp_methods
    0,                                 // tp_members
    debfile_getset,                    // tp_getset
    &PyArArchive_Type,                 // tp_base
    0,                                 // tp_dict
    0,                                 // tp_descr_get
    0,                                 // tp_descr_set
    0,                                 // tp_dictoffset
    0,                                 // tp_init
    0,                                 // tp_alloc
    debfile_new                        // tp_new
};
