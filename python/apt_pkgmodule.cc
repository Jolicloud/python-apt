// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: apt_pkgmodule.cc,v 1.5 2003/07/23 02:20:24 mdz Exp $
/* ######################################################################

   apt_pkgmodule - Top level for the python module. Create the internal
                   structures for the module in the interpriter.

   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include "apt_pkgmodule.h"
#include "generic.h"

#include <apt-pkg/configuration.h>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/packagemanager.h>
#include <apt-pkg/version.h>
#include <apt-pkg/deblistparser.h>
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/md5.h>
#include <apt-pkg/sha1.h>
#include <apt-pkg/sha256.h>
#include <apt-pkg/init.h>
#include <apt-pkg/pkgsystem.h>

#include <sys/stat.h>
#include <libintl.h>
#include <unistd.h>
#include <Python.h>
									/*}}}*/

/**
 * A Python->C->Python gettext() function.
 *
 * Python's gettext() ignores setlocale() which causes a strange behavior
 * because the values received from apt-pkg respect setlocale(). We circumvent
 * this problem by calling the C version of gettext(). This is also much
 * faster.
 */
static PyObject *py_gettext(PyObject *self, PyObject *Args) {
    const char *msg;
    char *domain = "python-apt";
    if (PyArg_ParseTuple(Args,"s|s:gettext",&msg, &domain) == 0)
        return 0;

    return PyString_FromString(dgettext(domain, msg));
}

// newConfiguration - Build a new configuration class			/*{{{*/
// ---------------------------------------------------------------------
#ifdef COMPAT_0_7
static char *doc_newConfiguration = "Construct a configuration instance";
static PyObject *newConfiguration(PyObject *self,PyObject *args)
{
   if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
      PyErr_WarnEx(PyExc_DeprecationWarning, "apt_pkg.newConfiguration() is "
                  "deprecated. Use apt_pkg.Configuration() instead.", 1);
   return CppPyObject_NEW<Configuration*>(NULL, &PyConfiguration_Type, new Configuration());
}
#endif
									/*}}}*/

// Version Wrappers							/*{{{*/
// These are kind of legacy..
static char *doc_VersionCompare = "VersionCompare(a,b) -> int";
static PyObject *VersionCompare(PyObject *Self,PyObject *Args)
{
   char *A;
   char *B;
   int LenA;
   int LenB;

   if (PyArg_ParseTuple(Args,"s#s#",&A,&LenA,&B,&LenB) == 0)
      return 0;

   if (_system == 0)
   {
      PyErr_SetString(PyExc_ValueError,"_system not initialized");
      return 0;
   }

   return Py_BuildValue("i",_system->VS->DoCmpVersion(A,A+LenA,B,B+LenB));
}

static char *doc_CheckDep =
    "check_dep(pkg_ver: str, dep_op: str, dep_ver: str) -> bool\n\n"
    "Check that the given requirement is fulfilled; i.e. that the version\n"
    "string given by 'pkg_ver' matches the version string 'dep_ver' under\n"
    "the condition specified by the operator 'dep_op' (<,<=,=,>=,>).\n\n"
    "This functions returns True if 'pkg_ver' matches 'dep_ver' under the\n"
    "condition 'dep_op'; e.g. this returns True:\n\n"
    "    apt_pkg.check_dep('1', '<=', '2')";
static PyObject *CheckDep(PyObject *Self,PyObject *Args)
{
   char *A;
   char *B;
   char *OpStr;
   unsigned int Op = 0;

   if (PyArg_ParseTuple(Args,"sss",&A,&OpStr,&B) == 0)
      return 0;

   if (strcmp(OpStr, ">") == 0) OpStr = ">>";
   if (strcmp(OpStr, "<") == 0) OpStr = "<<";
   if (*debListParser::ConvertRelation(OpStr,Op) != 0)
   {
      PyErr_SetString(PyExc_ValueError,"Bad comparision operation");
      return 0;
   }

   if (_system == 0)
   {
      PyErr_SetString(PyExc_ValueError,"_system not initialized");
      return 0;
   }

   return PyBool_FromLong(_system->VS->CheckDep(A,Op,B));
}

#ifdef COMPAT_0_7
static char *doc_CheckDepOld = "CheckDep(PkgVer,DepOp,DepVer) -> bool";
static PyObject *CheckDepOld(PyObject *Self,PyObject *Args)
{
   char *A;
   char *B;
   char *OpStr;
   unsigned int Op = 0;

   if (PyArg_ParseTuple(Args,"sss",&A,&OpStr,&B) == 0)
      return 0;
   if (*debListParser::ConvertRelation(OpStr,Op) != 0)
   {
      PyErr_SetString(PyExc_ValueError,"Bad comparision operation");
      return 0;
   }

   if (_system == 0)
   {
      PyErr_SetString(PyExc_ValueError,"_system not initialized");
      return 0;
   }

   return PyBool_FromLong(_system->VS->CheckDep(A,Op,B));
}
#endif

static char *doc_UpstreamVersion = "UpstreamVersion(a) -> string";
static PyObject *UpstreamVersion(PyObject *Self,PyObject *Args)
{
   char *Ver;
   if (PyArg_ParseTuple(Args,"s",&Ver) == 0)
      return 0;
   return CppPyString(_system->VS->UpstreamVersion(Ver));
}

static char *doc_ParseDepends =
"ParseDepends(s) -> list of tuples\n"
"\n"
"The resulting tuples are (Pkg,Ver,Operation). Each anded dependency is a\n"
"list of or'd dependencies\n"
"Source depends are evaluated against the curernt arch and only those that\n"
"Match are returned.\n\n"
"apt_pkg.Parse{,Src}Depends() are old forms which return >>,<< instead of >,<";
static PyObject *RealParseDepends(PyObject *Self,PyObject *Args,
                                  bool ParseArchFlags, string name,
                                  bool debStyle=false)
{
   string Package;
   string Version;
   unsigned int Op;

   const char *Start;
   const char *Stop;
   int Len;

   if (PyArg_ParseTuple(Args,(char *)("s#:" + name).c_str(),&Start,&Len) == 0)
      return 0;
   Stop = Start + Len;
   PyObject *List = PyList_New(0);
   PyObject *LastRow = 0;
   while (1)
   {
      if (Start == Stop)
	 break;

      Start = debListParser::ParseDepends(Start,Stop,Package,Version,Op,
					  ParseArchFlags);
      if (Start == 0)
      {
	 PyErr_SetString(PyExc_ValueError,"Problem Parsing Dependency");
	 Py_DECREF(List);
	 return 0;
      }

      if (LastRow == 0)
	 LastRow = PyList_New(0);

      if (Package.empty() == false)
      {
	 PyObject *Obj;
	 PyList_Append(LastRow,Obj = Py_BuildValue("sss",Package.c_str(),
						   Version.c_str(),
						debStyle ? pkgCache::CompTypeDeb(Op) : pkgCache::CompType(Op)));
	 Py_DECREF(Obj);
      }

      // Group ORd deps into a single row..
      if ((Op & pkgCache::Dep::Or) != pkgCache::Dep::Or)
      {
	 if (PyList_Size(LastRow) != 0)
	    PyList_Append(List,LastRow);
	 Py_DECREF(LastRow);
	 LastRow = 0;
      }
   }
   return List;
}
static PyObject *ParseDepends(PyObject *Self,PyObject *Args)
{
   return RealParseDepends(Self, Args, false, "parse_depends");
}
static PyObject *ParseSrcDepends(PyObject *Self,PyObject *Args)
{
   return RealParseDepends(Self, Args, true, "parse_src_depends");
}
#ifdef COMPAT_0_7
static PyObject *ParseDepends_old(PyObject *Self,PyObject *Args)
{
   return RealParseDepends(Self, Args, false, "ParseDepends", true);
}
static PyObject *ParseSrcDepends_old(PyObject *Self,PyObject *Args)
{
   return RealParseDepends(Self, Args, true, "ParseSrcDepends", true);
}
#endif
									/*}}}*/
// md5sum - Compute the md5sum of a file or string			/*{{{*/
// ---------------------------------------------------------------------
static char *doc_md5sum = "md5sum(String) -> String or md5sum(File) -> String";
static PyObject *md5sum(PyObject *Self,PyObject *Args)
{
   PyObject *Obj;
   if (PyArg_ParseTuple(Args,"O",&Obj) == 0)
      return 0;

   // Digest of a string.
   if (PyBytes_Check(Obj) != 0)
   {
      char *s;
      Py_ssize_t len;
      MD5Summation Sum;
      PyBytes_AsStringAndSize(Obj, &s, &len);
      Sum.Add((const unsigned char*)s, len);
      return CppPyString(Sum.Result().Value());
   }

   // Digest of a file
   int Fd = PyObject_AsFileDescriptor(Obj);
   if (Fd != -1)
   {
      MD5Summation Sum;
      struct stat St;
      if (fstat(Fd,&St) != 0 ||
	  Sum.AddFD(Fd,St.st_size) == false)
      {
	 PyErr_SetFromErrno(PyExc_SystemError);
	 return 0;
      }

      return CppPyString(Sum.Result().Value());
   }

   PyErr_SetString(PyExc_TypeError,"Only understand strings and files");
   return 0;
}
									/*}}}*/
// sha1sum - Compute the sha1sum of a file or string			/*{{{*/
// ---------------------------------------------------------------------
static char *doc_sha1sum = "sha1sum(String) -> String or sha1sum(File) -> String";
static PyObject *sha1sum(PyObject *Self,PyObject *Args)
{
   PyObject *Obj;
   if (PyArg_ParseTuple(Args,"O",&Obj) == 0)
      return 0;

   // Digest of a string.
   if (PyBytes_Check(Obj) != 0)
   {
      char *s;
      Py_ssize_t len;
      SHA1Summation Sum;
      PyBytes_AsStringAndSize(Obj, &s, &len);
      Sum.Add((const unsigned char*)s, len);
      return CppPyString(Sum.Result().Value());
   }

   // Digest of a file
   int Fd = PyObject_AsFileDescriptor(Obj);
   if (Fd != -1)
   {
      SHA1Summation Sum;
      struct stat St;
      if (fstat(Fd,&St) != 0 ||
	  Sum.AddFD(Fd,St.st_size) == false)
      {
	 PyErr_SetFromErrno(PyExc_SystemError);
	 return 0;
      }

      return CppPyString(Sum.Result().Value());
   }

   PyErr_SetString(PyExc_TypeError,"Only understand strings and files");
   return 0;
}
									/*}}}*/
// sha256sum - Compute the sha1sum of a file or string			/*{{{*/
// ---------------------------------------------------------------------
static char *doc_sha256sum = "sha256sum(String) -> String or sha256sum(File) -> String";
static PyObject *sha256sum(PyObject *Self,PyObject *Args)
{
   PyObject *Obj;
   if (PyArg_ParseTuple(Args,"O",&Obj) == 0)
      return 0;

   // Digest of a string.
   if (PyBytes_Check(Obj) != 0)
   {
      char *s;
      Py_ssize_t len;
      SHA256Summation Sum;
      PyBytes_AsStringAndSize(Obj, &s, &len);
      Sum.Add((const unsigned char*)s, len);
      return CppPyString(Sum.Result().Value());
   }

   // Digest of a file
   int Fd = PyObject_AsFileDescriptor(Obj);
   if (Fd != -1)
   {
      SHA256Summation Sum;
      struct stat St;
      if (fstat(Fd,&St) != 0 ||
	  Sum.AddFD(Fd,St.st_size) == false)
      {
	 PyErr_SetFromErrno(PyExc_SystemError);
	 return 0;
      }

      return CppPyString(Sum.Result().Value());
   }

   PyErr_SetString(PyExc_TypeError,"Only understand strings and files");
   return 0;
}
									/*}}}*/
// init - 3 init functions						/*{{{*/
// ---------------------------------------------------------------------
static char *doc_Init =
"init() -> None\n"
"Legacy. Do InitConfig then parse the command line then do InitSystem\n";
static PyObject *Init(PyObject *Self,PyObject *Args)
{
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   pkgInitConfig(*_config);
   pkgInitSystem(*_config,_system);

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}

static char *doc_InitConfig =
"initconfig() -> None\n"
"Load the default configuration and the config file\n";
static PyObject *InitConfig(PyObject *Self,PyObject *Args)
{
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   pkgInitConfig(*_config);

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}

static char *doc_InitSystem =
"initsystem() -> None\n"
"Construct the underlying system\n";
static PyObject *InitSystem(PyObject *Self,PyObject *Args)
{
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   pkgInitSystem(*_config,_system);

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}
									/*}}}*/

// fileutils.cc: GetLock						/*{{{*/
// ---------------------------------------------------------------------
static char *doc_GetLock =
"GetLock(string) -> int\n"
"This will create an empty file of the given name and lock it. Once this"
" is done all other calls to GetLock in any other process will fail with"
" -1. The return result is the fd of the file, the call should call"
" close at some time\n";
static PyObject *GetLock(PyObject *Self,PyObject *Args)
{
   const char *file;
   char errors = false;
   if (PyArg_ParseTuple(Args,"s|b",&file,&errors) == 0)
      return 0;

   int fd = GetLock(file, errors);

   return  HandleErrors(Py_BuildValue("i", fd));
}

static char *doc_PkgSystemLock =
"PkgSystemLock() -> boolean\n"
"Get the global pkgsystem lock\n";
static PyObject *PkgSystemLock(PyObject *Self,PyObject *Args)
{
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   bool res = _system->Lock();

   Py_INCREF(Py_None);
   return HandleErrors(Py_BuildValue("b", res));
}

static char *doc_PkgSystemUnLock =
"PkgSystemUnLock() -> boolean\n"
"Unset the global pkgsystem lock\n";
static PyObject *PkgSystemUnLock(PyObject *Self,PyObject *Args)
{
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   bool res = _system->UnLock();

   Py_INCREF(Py_None);
   return HandleErrors(Py_BuildValue("b", res));
}

									/*}}}*/

// initapt_pkg - Core Module Initialization				/*{{{*/
// ---------------------------------------------------------------------
/* */
static PyMethodDef methods[] =
{
   // Constructors
   {"init",Init,METH_VARARGS,doc_Init},
   {"init_config",InitConfig,METH_VARARGS,doc_InitConfig},
   {"init_system",InitSystem,METH_VARARGS,doc_InitSystem},

   // Internationalization.
   {"gettext",py_gettext,METH_VARARGS,
    "gettext(msg: str[, domain: str = 'python-apt']) -> str\n\n"
    "Translate the given string. Much Faster than Python's version and only\n"
    "does translations after setlocale() has been called."},

   // Tag File
   {"rewrite_section",RewriteSection,METH_VARARGS,doc_RewriteSection},

   // Locking
   {"get_lock",GetLock,METH_VARARGS,doc_GetLock},
   {"pkgsystem_lock",PkgSystemLock,METH_VARARGS,doc_PkgSystemLock},
   {"pkgsystem_unlock",PkgSystemUnLock,METH_VARARGS,doc_PkgSystemUnLock},

   // Command line
   {"read_config_file",LoadConfig,METH_VARARGS,doc_LoadConfig},
   {"read_config_dir",LoadConfigDir,METH_VARARGS,doc_LoadConfigDir},
   {"read_config_file_isc",LoadConfigISC,METH_VARARGS,doc_LoadConfig},
   {"parse_commandline",ParseCommandLine,METH_VARARGS,doc_ParseCommandLine},

   // Versioning
   {"version_compare",VersionCompare,METH_VARARGS,doc_VersionCompare},
   {"check_dep",CheckDep,METH_VARARGS,doc_CheckDep},
   {"upstream_version",UpstreamVersion,METH_VARARGS,doc_UpstreamVersion},

   // Depends
   {"parse_depends",ParseDepends,METH_VARARGS,doc_ParseDepends},
   {"parse_src_depends",ParseSrcDepends,METH_VARARGS,doc_ParseDepends},

   // Stuff
   {"md5sum",md5sum,METH_VARARGS,doc_md5sum},
   {"sha1sum",sha1sum,METH_VARARGS,doc_sha1sum},
   {"sha256sum",sha256sum,METH_VARARGS,doc_sha256sum},

   // Strings
   {"check_domain_list",StrCheckDomainList,METH_VARARGS,"CheckDomainList(String,String) -> Bool"},
   {"quote_string",StrQuoteString,METH_VARARGS,"QuoteString(String,String) -> String"},
   {"dequote_string",StrDeQuote,METH_VARARGS,"DeQuoteString(String) -> String"},
   {"size_to_str",StrSizeToStr,METH_VARARGS,"SizeToStr(int) -> String"},
   {"time_to_str",StrTimeToStr,METH_VARARGS,"TimeToStr(int) -> String"},
   {"uri_to_filename",StrURItoFileName,METH_VARARGS,"URItoFileName(String) -> String"},
   {"base64_encode",StrBase64Encode,METH_VARARGS,"Base64Encode(String) -> String"},
   {"string_to_bool",StrStringToBool,METH_VARARGS,"StringToBool(String) -> int"},
   {"time_rfc1123",StrTimeRFC1123,METH_VARARGS,"TimeRFC1123(int) -> String"},
   {"str_to_time",StrStrToTime,METH_VARARGS,"StrToTime(String) -> Int"},

   // DEPRECATED
   #ifdef COMPAT_0_7
   {"CheckDep",CheckDepOld,METH_VARARGS,doc_CheckDepOld},
   {"newConfiguration",newConfiguration,METH_VARARGS,doc_newConfiguration},
   {"InitConfig",InitConfig,METH_VARARGS,doc_InitConfig},
   {"InitSystem",InitSystem,METH_VARARGS,doc_InitSystem},

   {"ParseSection",ParseSection,METH_VARARGS,doc_ParseSection},
   {"ParseTagFile",ParseTagFile,METH_VARARGS,doc_ParseTagFile},
   {"RewriteSection",RewriteSection,METH_VARARGS,doc_RewriteSection},

   {"GetLock",GetLock,METH_VARARGS,doc_GetLock},
   {"PkgSystemLock",PkgSystemLock,METH_VARARGS,doc_PkgSystemLock},
   {"PkgSystemUnLock",PkgSystemUnLock,METH_VARARGS,doc_PkgSystemUnLock},

   {"ReadConfigFile",LoadConfig,METH_VARARGS,doc_LoadConfig},
   {"ReadConfigDir",LoadConfigDir,METH_VARARGS,doc_LoadConfigDir},
   {"ReadConfigFileISC",LoadConfigISC,METH_VARARGS,doc_LoadConfig},
   {"ParseCommandLine",ParseCommandLine,METH_VARARGS,doc_ParseCommandLine},

   {"VersionCompare",VersionCompare,METH_VARARGS,doc_VersionCompare},
   {"UpstreamVersion",UpstreamVersion,METH_VARARGS,doc_UpstreamVersion},

   {"ParseDepends",ParseDepends_old,METH_VARARGS,doc_ParseDepends},
   {"ParseSrcDepends",ParseSrcDepends_old,METH_VARARGS,doc_ParseDepends},

   {"CheckDomainList",StrCheckDomainList,METH_VARARGS,"CheckDomainList(String,String) -> Bool"},
   {"QuoteString",StrQuoteString,METH_VARARGS,"QuoteString(String,String) -> String"},
   {"DeQuoteString",StrDeQuote,METH_VARARGS,"DeQuoteString(String) -> String"},
   {"SizeToStr",StrSizeToStr,METH_VARARGS,"SizeToStr(int) -> String"},
   {"TimeToStr",StrTimeToStr,METH_VARARGS,"TimeToStr(int) -> String"},
   {"URItoFileName",StrURItoFileName,METH_VARARGS,"URItoFileName(String) -> String"},
   {"Base64Encode",StrBase64Encode,METH_VARARGS,"Base64Encode(String) -> String"},
   {"StringToBool",StrStringToBool,METH_VARARGS,"StringToBool(String) -> int"},
   {"TimeRFC1123",StrTimeRFC1123,METH_VARARGS,"TimeRFC1123(int) -> String"},
   {"StrToTime",StrStrToTime,METH_VARARGS,"StrToTime(String) -> Int"},

   {"GetCache",TmpGetCache,METH_VARARGS,"GetCache() -> PkgCache"},
   {"GetDepCache",GetDepCache,METH_VARARGS,"GetDepCache(Cache) -> DepCache"},
   {"GetPkgRecords",GetPkgRecords,METH_VARARGS,"GetPkgRecords(Cache) -> PkgRecords"},
   {"GetPkgSrcRecords",GetPkgSrcRecords,METH_VARARGS,"GetPkgSrcRecords() -> PkgSrcRecords"},
   {"GetPkgSourceList",GetPkgSourceList,METH_VARARGS,"GetPkgSourceList() -> PkgSourceList"},
   {"GetPkgProblemResolver",GetPkgProblemResolver,METH_VARARGS,"GetDepProblemResolver(DepCache) -> PkgProblemResolver"},
   {"GetPkgActionGroup",GetPkgActionGroup,METH_VARARGS,"GetPkgActionGroup(DepCache) -> PkgActionGroup"},
   {"GetCdrom",GetCdrom,METH_VARARGS,"GetCdrom() -> Cdrom"},
   {"GetAcquire",GetAcquire,METH_VARARGS,"GetAcquire() -> Acquire"},
   {"GetPkgAcqFile",(PyCFunction)GetPkgAcqFile,METH_KEYWORDS|METH_VARARGS, doc_GetPkgAcqFile},
   {"GetPackageManager",GetPkgManager,METH_VARARGS,"GetPackageManager(DepCache) -> PackageManager"},
   #endif

   {}
};

static struct _PyAptPkgAPIStruct API = {
   &PyAcquire_Type,           // acquire_type
   &PyAcquire_FromCpp,        // acquire_fromcpp
   &PyAcquire_ToCpp,          // acquire_tocpp
   &PyAcquireFile_Type,       // acquirefile_type
   &PyAcquireFile_FromCpp,    // acquirefile_fromcpp
   &PyAcquireFile_ToCpp,      // acquirefile_tocpp
   &PyAcquireItem_Type,       // acquireitem_type
   &PyAcquireItem_FromCpp,    // acquireitem_fromcpp
   &PyAcquireItem_ToCpp,      // acquireitem_type
   &PyAcquireItemDesc_Type,   // acquireitemdesc_type
   &PyAcquireItemDesc_FromCpp,// acquireitemdesc_fromcpp
   &PyAcquireItemDesc_ToCpp,  // acquireitemdesc_tocpp
   &PyAcquireWorker_Type,     // acquireworker_type
   &PyAcquireWorker_FromCpp,  // acquireworker_fromcpp
   &PyAcquireWorker_ToCpp,    // acquireworker_tocpp
   &PyActionGroup_Type,       // actiongroup_type
   &PyActionGroup_FromCpp,    // actiongroup_fromcpp
   &PyActionGroup_ToCpp,      // actiongroup_tocpp
   &PyCache_Type,             // cache_type
   &PyCache_FromCpp,          // cache_fromcpp
   &PyCache_ToCpp,            // cache_tocpp
   &PyCacheFile_Type,         // cachefile_type
   &PyCacheFile_FromCpp,      // cachefile_fromcpp
   &PyCacheFile_ToCpp,        // cachefile_tocpp
   &PyCdrom_Type,             // cdrom_type
   &PyCdrom_FromCpp,          // cdrom_fromcpp
   &PyCdrom_ToCpp,            // cdrom_tocpp
   &PyConfiguration_Type,     // configuration_type
   &PyConfiguration_FromCpp,  // configuration_fromcpp
   &PyConfiguration_ToCpp,    // configuration_tocpp
   &PyDepCache_Type,          // depcache_type
   &PyDepCache_FromCpp,       // depcache_fromcpp
   &PyDepCache_ToCpp,         // depcache_tocpp
   &PyDependency_Type,        // dependency_type
   &PyDependency_FromCpp,     // dependency_fromcpp
   &PyDependency_ToCpp,       // dependency_tocpp
   &PyDependencyList_Type,    // dependencylist_type
   0,                         // FIXME: dependencylist_fromcpp
   0,                         // FIXME: dependencylist_tocpp
   &PyDescription_Type,       // description_type
   &PyDescription_FromCpp,    // description_fromcpp
   &PyDescription_ToCpp,      // description_tocpp
   &PyHashes_Type,            // hashes_type
   &PyHashes_FromCpp,         // hashes_fromcpp
   &PyHashes_ToCpp,           // hashes_tocpp
   &PyHashString_Type,        // hashstring_type
   &PyHashString_FromCpp,     // hashstring_fromcpp
   &PyHashString_ToCpp,       // hashstring_tocpp
   &PyIndexRecords_Type,      // indexrecords_type
   &PyIndexRecords_FromCpp,     // indexrecords_tocpp
   &PyIndexRecords_ToCpp,     // indexrecords_tocpp
   &PyMetaIndex_Type,         // metaindex_type
   &PyMetaIndex_FromCpp,        // metaindex_tocpp
   &PyMetaIndex_ToCpp,        // metaindex_tocpp
   &PyPackage_Type,           // package_type
   &PyPackage_FromCpp,          // package_tocpp
   &PyPackage_ToCpp,          // package_tocpp
   &PyPackageFile_Type,       // packagefile_type
   &PyPackageFile_FromCpp,      // packagefile_tocpp
   &PyPackageFile_ToCpp,      // packagefile_tocpp
   &PyIndexFile_Type,         // packageindexfile_type
   &PyIndexFile_FromCpp,        // packageindexfile_tocpp
   &PyIndexFile_ToCpp,        // packageindexfile_tocpp
   &PyPackageList_Type,       // packagelist_type
   0,                         // FIXME: packagelist_fromcpp
   0,                         // FIXME: packagelist_tocpp
   &PyPackageManager_Type,    // packagemanager_type
   &PyPackageManager_FromCpp,   // packagemanager_type
   &PyPackageManager_ToCpp,   // packagemanager_type
   &PyPackageRecords_Type,    // packagerecords_type
   0,                         // FIXME: packagerecords_fromcpp
   0,                         // FIXME: packagerecords_tocpp
   &PyPolicy_Type,            // policy_type
   &PyPolicy_FromCpp,           // policy_tocpp
   &PyPolicy_ToCpp,           // policy_tocpp
   &PyProblemResolver_Type,   // problemresolver_type
   &PyProblemResolver_FromCpp,  // problemresolver_tocpp
   &PyProblemResolver_ToCpp,  // problemresolver_tocpp
   &PySourceList_Type,        // sourcelist_type
   &PySourceList_FromCpp,       // sourcelist_tocpp
   &PySourceList_ToCpp,       // sourcelist_tocpp
   &PySourceRecords_Type,     // sourcerecords_type
   0,                         // FIXME: sourcerecords_fromcpp
   0,                         // FIXME: sourcerecords_tocpp
   &PyTagFile_Type,           // tagfile_type
   &PyTagFile_FromCpp,          // tagfile_tocpp
   &PyTagFile_ToCpp,          // tagfile_tocpp
   &PyTagSection_Type,        // tagsection_type
   &PyTagSection_FromCpp,       // tagsection_tocpp
   &PyTagSection_ToCpp,       // tagsection_tocpp
   &PyVersion_Type,           // version_type
   &PyVersion_FromCpp,          // version_tocpp
   &PyVersion_ToCpp,          // version_tocpp
};


#define ADDTYPE(mod,name,type) { \
    if (PyType_Ready(type) == -1) INIT_ERROR; \
    Py_INCREF(type); \
    PyModule_AddObject(mod,name,(PyObject *)type); }


#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "apt_pkg",
        "Classes and functions wrapping the apt-pkg library.\n\n"
        "The apt_pkg module provides...",
        -1,
        methods,
        0,
        0,
        0,
        0,
};

#define INIT_ERROR return 0
extern "C" PyObject * PyInit_apt_pkg()
#else
#define INIT_ERROR return
extern "C" void initapt_pkg()
#endif
{
   // Finalize our types to add slots, etc.
   if (PyType_Ready(&PyConfiguration_Type) == -1) INIT_ERROR;
   if (PyType_Ready(&PyCacheFile_Type) == -1) INIT_ERROR;

   // Initialize the module
   #if PY_MAJOR_VERSION >= 3
   PyObject *Module = PyModule_Create(&moduledef);
   #else
   PyObject *Module = Py_InitModule("apt_pkg",methods);
   #endif

   // Global variable linked to the global configuration class
   CppPyObject<Configuration*> *Config = CppPyObject_NEW<Configuration*>(NULL, &PyConfiguration_Type);
   Config->Object = _config;
   // Global configuration, should never be deleted.
   Config->NoDelete = true;
   PyModule_AddObject(Module,"config",Config);
   #ifdef COMPAT_0_7
   Py_INCREF(Config);
   PyModule_AddObject(Module,"Config",Config);
   #endif




   // Add our classes.
   /* ============================ tag.cc ============================ */
   ADDTYPE(Module,"TagSection",&PyTagSection_Type);
   ADDTYPE(Module,"TagFile",&PyTagFile_Type);
   /* ============================ acquire.cc ============================ */
   ADDTYPE(Module,"Acquire",&PyAcquire_Type);
   ADDTYPE(Module,"AcquireFile",&PyAcquireFile_Type);
   ADDTYPE(Module,"AcquireItem",&PyAcquireItem_Type); // NO __new__()
   ADDTYPE(Module,"AcquireWorker",&PyAcquireWorker_Type); // NO __new__()
   /* ============================ cache.cc ============================ */
   ADDTYPE(Module,"Cache",&PyCache_Type);
   ADDTYPE(Module,"Dependency",&PyDependency_Type); // NO __new__()
   ADDTYPE(Module,"Description",&PyDescription_Type); // NO __new__()
   ADDTYPE(Module,"PackageFile",&PyPackageFile_Type); // NO __new__()
   ADDTYPE(Module,"PackageList",&PyPackageList_Type);  // NO __new__(), internal
   ADDTYPE(Module,"DependencyList",&PyDependencyList_Type); // NO __new__(), internal
   ADDTYPE(Module,"Package",&PyPackage_Type); // NO __new__()
   ADDTYPE(Module,"Version",&PyVersion_Type); // NO __new__()
   /* ============================ cdrom.cc ============================ */
   ADDTYPE(Module,"Cdrom",&PyCdrom_Type);
   /* ========================= configuration.cc ========================= */
   ADDTYPE(Module,"Configuration",&PyConfiguration_Type);
   /* ========================= depcache.cc ========================= */
   ADDTYPE(Module,"ActionGroup",&PyActionGroup_Type);
   ADDTYPE(Module,"DepCache",&PyDepCache_Type);
   ADDTYPE(Module,"ProblemResolver",&PyProblemResolver_Type);
   /* ========================= indexfile.cc ========================= */
   ADDTYPE(Module,"IndexFile",&PyIndexFile_Type); // NO __new__()
   /* ========================= metaindex.cc ========================= */
   ADDTYPE(Module,"MetaIndex",&PyMetaIndex_Type); // NO __new__()
   /* ========================= pkgmanager.cc ========================= */
   ADDTYPE(Module,"PackageManager",&PyPackageManager_Type);
   /* ========================= pkgrecords.cc ========================= */
   ADDTYPE(Module,"PackageRecords",&PyPackageRecords_Type);
   /* ========================= pkgsrcrecords.cc ========================= */
   ADDTYPE(Module,"SourceRecords",&PySourceRecords_Type);
   /* ========================= sourcelist.cc ========================= */
   ADDTYPE(Module,"SourceList",&PySourceList_Type);
   ADDTYPE(Module,"IndexRecords",&PyIndexRecords_Type);
   ADDTYPE(Module,"HashString",&PyHashString_Type);
   ADDTYPE(Module,"Policy",&PyPolicy_Type);
   ADDTYPE(Module,"Hashes",&PyHashes_Type);
   ADDTYPE(Module,"AcquireItemDesc",&PyAcquireItemDesc_Type);
   ADDTYPE(Module,"SystemLock",&PySystemLock_Type);
   ADDTYPE(Module,"FileLock",&PyFileLock_Type);
   // Tag file constants
   PyModule_AddObject(Module,"REWRITE_PACKAGE_ORDER",
                      CharCharToList(TFRewritePackageOrder));

   PyModule_AddObject(Module,"REWRITE_SOURCE_ORDER",
                      CharCharToList(TFRewriteSourceOrder));


   // Acquire constants.
   // some constants
   PyDict_SetItemString(PyAcquire_Type.tp_dict, "RESULT_CANCELLED",
                        Py_BuildValue("i", pkgAcquire::Cancelled));
   PyDict_SetItemString(PyAcquire_Type.tp_dict, "RESULT_CONTINUE",
                        Py_BuildValue("i", pkgAcquire::Continue));
   PyDict_SetItemString(PyAcquire_Type.tp_dict, "RESULT_FAILED",
                        Py_BuildValue("i", pkgAcquire::Failed));
#ifdef COMPAT_0_7
   PyDict_SetItemString(PyAcquire_Type.tp_dict, "ResultCancelled",
                        Py_BuildValue("i", pkgAcquire::Cancelled));
   PyDict_SetItemString(PyAcquire_Type.tp_dict, "ResultContinue",
                        Py_BuildValue("i", pkgAcquire::Continue));
   PyDict_SetItemString(PyAcquire_Type.tp_dict, "ResultFailed",
                        Py_BuildValue("i", pkgAcquire::Failed));
#endif
    // Dependency constants
   PyDict_SetItemString(PyDependency_Type.tp_dict, "TYPE_DEPENDS",
                        Py_BuildValue("i", pkgCache::Dep::Depends));
   PyDict_SetItemString(PyDependency_Type.tp_dict, "TYPE_PREDEPENDS",
                        Py_BuildValue("i", pkgCache::Dep::PreDepends));
   PyDict_SetItemString(PyDependency_Type.tp_dict, "TYPE_SUGGESTS",
                        Py_BuildValue("i", pkgCache::Dep::Suggests));
   PyDict_SetItemString(PyDependency_Type.tp_dict, "TYPE_RECOMMENDS",
                        Py_BuildValue("i", pkgCache::Dep::Suggests));
   PyDict_SetItemString(PyDependency_Type.tp_dict, "TYPE_CONFLICTS",
                        Py_BuildValue("i", pkgCache::Dep::Conflicts));
   PyDict_SetItemString(PyDependency_Type.tp_dict, "TYPE_REPLACES",
                        Py_BuildValue("i", pkgCache::Dep::Replaces));
   PyDict_SetItemString(PyDependency_Type.tp_dict, "TYPE_OBSOLETES",
                        Py_BuildValue("i", pkgCache::Dep::Obsoletes));
   PyDict_SetItemString(PyDependency_Type.tp_dict, "TYPE_DPKG_BREAKS",
                        Py_BuildValue("i", pkgCache::Dep::DpkgBreaks));
   PyDict_SetItemString(PyDependency_Type.tp_dict, "TYPE_ENHANCES",
                        Py_BuildValue("i", pkgCache::Dep::Enhances));


   // PackageManager constants
   PyDict_SetItemString(PyPackageManager_Type.tp_dict, "RESULT_COMPLETED",
                        Py_BuildValue("i", pkgPackageManager::Completed));
   PyDict_SetItemString(PyPackageManager_Type.tp_dict, "RESULT_FAILED",
                        Py_BuildValue("i", pkgPackageManager::Failed));
   PyDict_SetItemString(PyPackageManager_Type.tp_dict, "RESULT_INCOMPLETE",
                        Py_BuildValue("i", pkgPackageManager::Incomplete));

#ifdef COMPAT_0_7
   PyDict_SetItemString(PyPackageManager_Type.tp_dict, "ResultCompleted",
                        Py_BuildValue("i", pkgPackageManager::Completed));
   PyDict_SetItemString(PyPackageManager_Type.tp_dict, "ResultFailed",
                        Py_BuildValue("i", pkgPackageManager::Failed));
   PyDict_SetItemString(PyPackageManager_Type.tp_dict, "ResultIncomplete",
                        Py_BuildValue("i", pkgPackageManager::Incomplete));
#endif

   // AcquireItem Constants.
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "STAT_IDLE",
                        Py_BuildValue("i", pkgAcquire::Item::StatIdle));
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "STAT_FETCHING",
                        Py_BuildValue("i", pkgAcquire::Item::StatFetching));
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "STAT_DONE",
                        Py_BuildValue("i", pkgAcquire::Item::StatDone));
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "STAT_ERROR",
                        Py_BuildValue("i", pkgAcquire::Item::StatError));
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "STAT_AUTH_ERROR",
                        Py_BuildValue("i", pkgAcquire::Item::StatAuthError));

#ifdef COMPAT_0_7
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "StatIdle",
                        Py_BuildValue("i", pkgAcquire::Item::StatIdle));
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "StatFetching",
                        Py_BuildValue("i", pkgAcquire::Item::StatFetching));
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "StatDone",
                        Py_BuildValue("i", pkgAcquire::Item::StatDone));
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "StatError",
                        Py_BuildValue("i", pkgAcquire::Item::StatError));
   PyDict_SetItemString(PyAcquireItem_Type.tp_dict, "StatAuthError",
                        Py_BuildValue("i", pkgAcquire::Item::StatAuthError));
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 1
   PyObject *PyCapsule = PyCapsule_New(&API, "apt_pkg._C_API", NULL);
#else
   PyObject *PyCapsule = PyCObject_FromVoidPtr(&API, NULL);
#endif
   PyModule_AddObject(Module, "_C_API", PyCapsule);
   // Version..
   PyModule_AddStringConstant(Module,"VERSION",(char *)pkgVersion);
   PyModule_AddStringConstant(Module,"LIB_VERSION",(char *)pkgLibVersion);
   PyModule_AddStringConstant(Module,"DATE",__DATE__);
   PyModule_AddStringConstant(Module,"TIME",__TIME__);

   // My constants
   PyModule_AddIntConstant(Module,"PRI_IMPORTANT",pkgCache::State::Important);
   PyModule_AddIntConstant(Module,"PRI_REQUIRED",pkgCache::State::Required);
   PyModule_AddIntConstant(Module,"PRI_STANDARD",pkgCache::State::Standard);
   PyModule_AddIntConstant(Module,"PRI_OPTIONAL",pkgCache::State::Optional);
   PyModule_AddIntConstant(Module,"PRI_EXTRA",pkgCache::State::Extra);
   // CurState
   PyModule_AddIntConstant(Module,"CURSTATE_NOT_INSTALLED",pkgCache::State::NotInstalled);
   PyModule_AddIntConstant(Module,"CURSTATE_UNPACKED",pkgCache::State::UnPacked);
   PyModule_AddIntConstant(Module,"CURSTATE_HALF_CONFIGURED",pkgCache::State::HalfConfigured);
   PyModule_AddIntConstant(Module,"CURSTATE_HALF_INSTALLED",pkgCache::State::HalfInstalled);
   PyModule_AddIntConstant(Module,"CURSTATE_CONFIG_FILES",pkgCache::State::ConfigFiles);
   PyModule_AddIntConstant(Module,"CURSTATE_INSTALLED",pkgCache::State::Installed);
   // SelState
   PyModule_AddIntConstant(Module,"SELSTATE_UNKNOWN",pkgCache::State::Unknown);
   PyModule_AddIntConstant(Module,"SELSTATE_INSTALL",pkgCache::State::Install);
   PyModule_AddIntConstant(Module,"SELSTATE_HOLD",pkgCache::State::Hold);
   PyModule_AddIntConstant(Module,"SELSTATE_DEINSTALL",pkgCache::State::DeInstall);
   PyModule_AddIntConstant(Module,"SELSTATE_PURGE",pkgCache::State::Purge);
   // InstState
   PyModule_AddIntConstant(Module,"INSTSTATE_OK",pkgCache::State::Ok);
   PyModule_AddIntConstant(Module,"INSTSTATE_REINSTREQ",pkgCache::State::ReInstReq);
   PyModule_AddIntConstant(Module,"INSTSTATE_HOLD",pkgCache::State::Hold);
   PyModule_AddIntConstant(Module,"INSTSTATE_HOLD_REINSTREQ",pkgCache::State::HoldReInstReq);

   // DEPRECATED API
   #ifdef COMPAT_0_7
   PyModule_AddObject(Module,"RewritePackageOrder",
                      CharCharToList(TFRewritePackageOrder));
   PyModule_AddObject(Module,"RewriteSourceOrder",
                      CharCharToList(TFRewriteSourceOrder));
   PyModule_AddStringConstant(Module,"LibVersion",(char *)pkgLibVersion);
   PyModule_AddStringConstant(Module,"Date",__DATE__);
   PyModule_AddStringConstant(Module,"Time",__TIME__);
   PyModule_AddIntConstant(Module,"PriImportant",pkgCache::State::Important);
   PyModule_AddIntConstant(Module,"PriRequired",pkgCache::State::Required);
   PyModule_AddIntConstant(Module,"PriStandard",pkgCache::State::Standard);
   PyModule_AddIntConstant(Module,"PriOptional",pkgCache::State::Optional);
   PyModule_AddIntConstant(Module,"PriExtra",pkgCache::State::Extra);
   PyModule_AddIntConstant(Module,"CurStateNotInstalled",pkgCache::State::NotInstalled);
   PyModule_AddIntConstant(Module,"CurStateUnPacked",pkgCache::State::UnPacked);
   PyModule_AddIntConstant(Module,"CurStateHalfConfigured",pkgCache::State::HalfConfigured);
   PyModule_AddIntConstant(Module,"CurStateHalfInstalled",pkgCache::State::HalfInstalled);
   PyModule_AddIntConstant(Module,"CurStateConfigFiles",pkgCache::State::ConfigFiles);
   PyModule_AddIntConstant(Module,"CurStateInstalled",pkgCache::State::Installed);
   PyModule_AddIntConstant(Module,"SelStateUnknown",pkgCache::State::Unknown);
   PyModule_AddIntConstant(Module,"SelStateInstall",pkgCache::State::Install);
   PyModule_AddIntConstant(Module,"SelStateHold",pkgCache::State::Hold);
   PyModule_AddIntConstant(Module,"SelStateDeInstall",pkgCache::State::DeInstall);
   PyModule_AddIntConstant(Module,"SelStatePurge",pkgCache::State::Purge);
   PyModule_AddIntConstant(Module,"InstStateOk",pkgCache::State::Ok);
   PyModule_AddIntConstant(Module,"InstStateReInstReq",pkgCache::State::ReInstReq);
   PyModule_AddIntConstant(Module,"InstStateHold",pkgCache::State::Hold);
   PyModule_AddIntConstant(Module,"InstStateHoldReInstReq",pkgCache::State::HoldReInstReq);
   PyModule_AddIntConstant(Module,"_COMPAT_0_7",1);
   #else
   PyModule_AddIntConstant(Module,"_COMPAT_0_7",0);
   #endif
   #if PY_MAJOR_VERSION >= 3
   return Module;
   #endif
}
									/*}}}*/

