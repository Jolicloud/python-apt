// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: depcache.cc,v 1.5 2003/06/03 03:03:23 mdz Exp $
/* ######################################################################

   DepCache - Wrapper for the depcache related functions

   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include "generic.h"
#include "apt_pkgmodule.h"

#include <apt-pkg/pkgcache.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/policy.h>
#include <apt-pkg/sptr.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/error.h>
#include <apt-pkg/pkgsystem.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/acquire-item.h>
#include <Python.h>

#include <iostream>
#include "progress.h"

#ifndef _
#define _(x) (x)
#endif



// DepCache Class								/*{{{*/
// ---------------------------------------------------------------------



static PyObject *PkgDepCacheInit(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *pyCallbackInst = 0;
   if (PyArg_ParseTuple(Args, "|O", &pyCallbackInst) == 0)
      return 0;

   if(pyCallbackInst != 0) {
      PyOpProgress progress;
      progress.setCallbackInst(pyCallbackInst);
      depcache->Init(&progress);
   } else {
      depcache->Init(0);
   }

   pkgApplyStatus(*depcache);

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}

static PyObject *PkgDepCacheCommit(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *pyInstallProgressInst = 0;
   PyObject *pyFetchProgressInst = 0;
   if (PyArg_ParseTuple(Args, "OO",
			&pyFetchProgressInst, &pyInstallProgressInst) == 0) {
      return 0;
   }
   FileFd Lock;
   if (_config->FindB("Debug::NoLocking", false) == false) {
      Lock.Fd(GetLock(_config->FindDir("Dir::Cache::Archives") + "lock"));
      if (_error->PendingError() == true)
         return HandleErrors();
   }

   pkgRecords Recs(*depcache);
   if (_error->PendingError() == true)
      HandleErrors(Py_None);

   pkgSourceList List;
   if(!List.ReadMainList())
      return HandleErrors(Py_None);

   PyFetchProgress progress;
   progress.setCallbackInst(pyFetchProgressInst);

   pkgAcquire Fetcher(&progress);
   pkgPackageManager *PM;
   PM = _system->CreatePM(depcache);
   if(PM->GetArchives(&Fetcher, &List, &Recs) == false ||
      _error->PendingError() == true) {
      std::cerr << "Error in GetArchives" << std::endl;
      return HandleErrors();
   }

   //std::cout << "PM created" << std::endl;

   PyInstallProgress iprogress;
   iprogress.setCallbackInst(pyInstallProgressInst);

   // Run it
   while (1)
   {
      bool Transient = false;

      if (Fetcher.Run() == pkgAcquire::Failed)
	 return false;

      // Print out errors
      bool Failed = false;
      for (pkgAcquire::ItemIterator I = Fetcher.ItemsBegin(); I != Fetcher.ItemsEnd(); I++)
      {

	 //std::cout << "looking at: " << (*I)->DestFile
	 //	   << " status: " << (*I)->Status << std::endl;

	 if ((*I)->Status == pkgAcquire::Item::StatDone &&
	     (*I)->Complete == true)
	    continue;

	 if ((*I)->Status == pkgAcquire::Item::StatIdle)
	 {
	    //std::cout << "transient failure" << std::endl;

	    Transient = true;
	    //Failed = true;
	    continue;
	 }

	 //std::cout << "something is wrong!" << std::endl;

	 _error->Warning(_("Failed to fetch %s  %s\n"),(*I)->DescURI().c_str(),
			 (*I)->ErrorText.c_str());
	 Failed = true;
      }

      if (Transient == true && Failed == true)
      {
	 _error->Error(_("--fix-missing and media swapping is not currently supported"));
	 Py_INCREF(Py_None);
	 return HandleErrors(Py_None);
      }

      // Try to deal with missing package files
      if (Failed == true && PM->FixMissing() == false)
      {
	 //std::cerr << "Unable to correct missing packages." << std::endl;
	 _error->Error("Aborting install.");
	 Py_INCREF(Py_None);
	 return HandleErrors(Py_None);
      }

      // fail if something else went wrong
      //FIXME: make this more flexible, e.g. with a failedDl handler
      if(Failed)
	 return Py_BuildValue("b", false);
      _system->UnLock(true);

      pkgPackageManager::OrderResult Res = iprogress.Run(PM);
      //std::cout << "iprogress.Run() returned: " << (int)Res << std::endl;

      if (Res == pkgPackageManager::Failed || _error->PendingError() == true) {
	 return HandleErrors(Py_BuildValue("b", false));
      }
      if (Res == pkgPackageManager::Completed) {
	 //std::cout << "iprogress.Run() returned Completed " << std::endl;
	 return Py_BuildValue("b", true);
      }

      //std::cout << "looping again, install unfinished" << std::endl;

      // Reload the fetcher object and loop again for media swapping
      Fetcher.Shutdown();
      if (PM->GetArchives(&Fetcher,&List,&Recs) == false) {
	 return Py_BuildValue("b", false);
      }
      _system->Lock();
   }

   return HandleErrors(Py_None);
}

static PyObject *PkgDepCacheSetCandidateVer(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);
   PyObject *PackageObj;
   PyObject *VersionObj;
   if (PyArg_ParseTuple(Args,"O!O!",
			&PyPackage_Type, &PackageObj,
			&PyVersion_Type, &VersionObj) == 0)
      return 0;

   pkgCache::VerIterator &I = GetCpp<pkgCache::VerIterator>(VersionObj);
   if(I.end()) {
      return HandleErrors(Py_BuildValue("b",false));
   }
   depcache->SetCandidateVersion(I);

   return HandleErrors(Py_BuildValue("b",true));
}

static PyObject *PkgDepCacheGetCandidateVer(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);
   PyObject *PackageObj;
   PyObject *CandidateObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache & State = (*depcache)[Pkg];
   pkgCache::VerIterator I = State.CandidateVerIter(*depcache);

   if(I.end()) {
      Py_INCREF(Py_None);
      return Py_None;
   }
   CandidateObj = CppPyObject_NEW<pkgCache::VerIterator>(PackageObj,&PyVersion_Type,I);

   return CandidateObj;
}

static PyObject *PkgDepCacheUpgrade(PyObject *Self,PyObject *Args)
{
   bool res;
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   char distUpgrade=0;
   if (PyArg_ParseTuple(Args,"|b",&distUpgrade) == 0)
      return 0;

   Py_BEGIN_ALLOW_THREADS
   if(distUpgrade)
      res = pkgDistUpgrade(*depcache);
   else
      res = pkgAllUpgrade(*depcache);
   Py_END_ALLOW_THREADS

   Py_INCREF(Py_None);
   return HandleErrors(Py_BuildValue("b",res));
}

static PyObject *PkgDepCacheMinimizeUpgrade(PyObject *Self,PyObject *Args)
{
   bool res;
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   Py_BEGIN_ALLOW_THREADS
   res = pkgMinimizeUpgrade(*depcache);
   Py_END_ALLOW_THREADS

   Py_INCREF(Py_None);
   return HandleErrors(Py_BuildValue("b",res));
}


static PyObject *PkgDepCacheReadPinFile(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);


   char *file=NULL;
   if (PyArg_ParseTuple(Args,"|s",&file) == 0)
      return 0;

   if(file == NULL)
      ReadPinFile((pkgPolicy&)depcache->GetPolicy());
   else
      ReadPinFile((pkgPolicy&)depcache->GetPolicy(), file);

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}


static PyObject *PkgDepCacheFixBroken(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   bool res=true;
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   res &=pkgFixBroken(*depcache);
   res &=pkgMinimizeUpgrade(*depcache);

   return HandleErrors(Py_BuildValue("b",res));
}


static PyObject *PkgDepCacheMarkKeep(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache*>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   depcache->MarkKeep(Pkg);

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}

static PyObject *PkgDepCacheSetReInstall(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache*>(Self);

   PyObject *PackageObj;
   char value = 0;
   if (PyArg_ParseTuple(Args,"O!b",&PyPackage_Type,&PackageObj, &value) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   depcache->SetReInstall(Pkg,value);

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}


static PyObject *PkgDepCacheMarkDelete(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   char purge = 0;
   if (PyArg_ParseTuple(Args,"O!|b",&PyPackage_Type,&PackageObj, &purge) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   depcache->MarkDelete(Pkg,purge);

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}


static PyObject *PkgDepCacheMarkInstall(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   char autoInst=1;
   char fromUser=1;
   if (PyArg_ParseTuple(Args,"O!|bb",&PyPackage_Type,&PackageObj,
			&autoInst, &fromUser) == 0)
      return 0;

   Py_BEGIN_ALLOW_THREADS
   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   depcache->MarkInstall(Pkg, autoInst, 0, fromUser);
   Py_END_ALLOW_THREADS

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}

static PyObject *PkgDepCacheMarkAuto(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache*>(Self);

   PyObject *PackageObj;
   char value = 0;
   if (PyArg_ParseTuple(Args,"O!b",&PyPackage_Type,&PackageObj, &value) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   depcache->MarkAuto(Pkg,value);

   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}


static PyObject *PkgDepCacheIsUpgradable(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.Upgradable()));
}

static PyObject *PkgDepCacheIsGarbage(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.Garbage));
}

static PyObject *PkgDepCacheIsAutoInstalled(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.Flags & pkgCache::Flag::Auto));
}

static PyObject *PkgDepCacheIsNowBroken(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.NowBroken()));
}

static PyObject *PkgDepCacheIsInstBroken(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.InstBroken()));
}


static PyObject *PkgDepCacheMarkedInstall(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.NewInstall()));
}


static PyObject *PkgDepCacheMarkedUpgrade(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.Upgrade()));
}

static PyObject *PkgDepCacheMarkedDelete(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.Delete()));
}

static PyObject *PkgDepCacheMarkedKeep(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.Keep()));
}

static PyObject *PkgDepCacheMarkedDowngrade(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   return HandleErrors(Py_BuildValue("b",state.Downgrade()));
}

static PyObject *PkgDepCacheMarkedReinstall(PyObject *Self,PyObject *Args)
{
   pkgDepCache *depcache = GetCpp<pkgDepCache *>(Self);

   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;

   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   pkgDepCache::StateCache &state = (*depcache)[Pkg];

   bool res = state.Install() && (state.iFlags & pkgDepCache::ReInstall);

   return HandleErrors(Py_BuildValue("b",res));
}


static PyMethodDef PkgDepCacheMethods[] =
{
   {"init",PkgDepCacheInit,METH_VARARGS,"Init the depcache (done on construct automatically)"},
   {"get_candidate_ver",PkgDepCacheGetCandidateVer,METH_VARARGS,"Get candidate version"},
   {"set_candidate_ver",PkgDepCacheSetCandidateVer,METH_VARARGS,"Set candidate version"},

   // global cache operations
   {"upgrade",PkgDepCacheUpgrade,METH_VARARGS,"Perform Upgrade (optional boolean argument if dist-upgrade should be performed)"},
   {"fix_broken",PkgDepCacheFixBroken,METH_VARARGS,"Fix broken packages"},
   {"read_pinfile",PkgDepCacheReadPinFile,METH_VARARGS,"Read the pin policy"},
   {"minimize_upgrade",PkgDepCacheMinimizeUpgrade, METH_VARARGS,"Go over the entire set of packages and try to keep each package marked for upgrade. If a conflict is generated then the package is restored."},
   // Manipulators
   {"mark_keep",PkgDepCacheMarkKeep,METH_VARARGS,"Mark package for keep"},
   {"mark_delete",PkgDepCacheMarkDelete,METH_VARARGS,"Mark package for delete (optional boolean argument if it should be purged)"},
   {"mark_install",PkgDepCacheMarkInstall,METH_VARARGS,"Mark package for Install"},
   {"mark_auto",PkgDepCacheMarkAuto,METH_VARARGS,"mark_auto(pkg: apt_pkg.Package, auto: bool)\n\nMark package as automatically installed."},
   {"set_reinstall",PkgDepCacheSetReInstall,METH_VARARGS,"Set if the package should be reinstalled"},
   // state information
   {"is_upgradable",PkgDepCacheIsUpgradable,METH_VARARGS,"Is pkg upgradable"},
   {"is_now_broken",PkgDepCacheIsNowBroken,METH_VARARGS,"Is pkg is now broken"},
   {"is_inst_broken",PkgDepCacheIsInstBroken,METH_VARARGS,"Is pkg broken on the current install"},
   {"is_garbage",PkgDepCacheIsGarbage,METH_VARARGS,"Is pkg garbage (mark-n-sweep)"},
   {"is_auto_installed",PkgDepCacheIsAutoInstalled,METH_VARARGS,"Is pkg marked as auto installed"},
   {"marked_install",PkgDepCacheMarkedInstall,METH_VARARGS,"Is pkg marked for install"},
   {"marked_upgrade",PkgDepCacheMarkedUpgrade,METH_VARARGS,"Is pkg marked for upgrade"},
   {"marked_delete",PkgDepCacheMarkedDelete,METH_VARARGS,"Is pkg marked for delete"},
   {"marked_keep",PkgDepCacheMarkedKeep,METH_VARARGS,"Is pkg marked for keep"},
   {"marked_reinstall",PkgDepCacheMarkedReinstall,METH_VARARGS,"Is pkg marked for reinstall"},
   {"marked_downgrade",PkgDepCacheMarkedDowngrade,METH_VARARGS,"Is pkg marked for downgrade"},
   // Action
   {"commit", PkgDepCacheCommit, METH_VARARGS, "Commit pending changes"},
   {}
};

#define depcache (GetCpp<pkgDepCache *>(Self))
static PyObject *PkgDepCacheGetKeepCount(PyObject *Self,void*) {
   return Py_BuildValue("l", depcache->KeepCount());
}
static PyObject *PkgDepCacheGetInstCount(PyObject *Self,void*) {
   return Py_BuildValue("l", depcache->InstCount());
}
static PyObject *PkgDepCacheGetDelCount(PyObject *Self,void*) {
   return Py_BuildValue("l", depcache->DelCount());
}
static PyObject *PkgDepCacheGetBrokenCount(PyObject *Self,void*) {
   return Py_BuildValue("l", depcache->BrokenCount());
}
static PyObject *PkgDepCacheGetUsrSize(PyObject *Self,void*) {
   return Py_BuildValue("d", depcache->UsrSize());
}
static PyObject *PkgDepCacheGetDebSize(PyObject *Self,void*) {
   return Py_BuildValue("d", depcache->DebSize());
}
#undef depcache

static PyObject *PkgDepCacheGetPolicy(PyObject *Self,void*) {
   PyObject *Owner = GetOwner<pkgDepCache*>(Self);
   pkgDepCache *DepCache = GetCpp<pkgDepCache*>(Self);
   pkgPolicy *Policy = (pkgPolicy *)&DepCache->GetPolicy();
   CppPyObject<pkgPolicy*> *PyPolicy =
        CppPyObject_NEW<pkgPolicy*>(Owner,&PyPolicy_Type,Policy);
   // Policy should not be deleted, it is managed by CacheFile.
   PyPolicy->NoDelete = true;
   return PyPolicy;
}


static PyGetSetDef PkgDepCacheGetSet[] = {
    {"broken_count",PkgDepCacheGetBrokenCount},
    {"deb_size",PkgDepCacheGetDebSize},
    {"del_count",PkgDepCacheGetDelCount},
    {"inst_count",PkgDepCacheGetInstCount},
    {"keep_count",PkgDepCacheGetKeepCount},
    {"usr_size",PkgDepCacheGetUsrSize},
    {"policy",PkgDepCacheGetPolicy},
    {}
};

static PyObject *PkgDepCacheNew(PyTypeObject *type,PyObject *Args,PyObject *kwds)
{
   PyObject *Owner;
   char *kwlist[] = {"cache", 0};
   if (PyArg_ParseTupleAndKeywords(Args,kwds,"O!",kwlist,&PyCache_Type,
                                   &Owner) == 0)
      return 0;


   // the owner of the Python cache object is a cachefile object, get it
   PyObject *CacheFilePy = GetOwner<pkgCache*>(Owner);
   // get the pkgCacheFile from the cachefile
   pkgCacheFile *CacheF = GetCpp<pkgCacheFile*>(CacheFilePy);
   // and now the depcache
   pkgDepCache *depcache = (pkgDepCache *)(*CacheF);

   CppPyObject<pkgDepCache*> *DepCachePyObj;
   DepCachePyObj = CppPyObject_NEW<pkgDepCache*>(Owner,type,depcache);

   // Do not delete the underlying pointer, it is managed by the cachefile.
   DepCachePyObj->NoDelete = true;

   return HandleErrors(DepCachePyObj);
}

static char *doc_PkgDepCache = "DepCache(cache) -> DepCache() object\n\n"
    "A DepCache() holds extra information on the state of the packages.\n\n"
    "The parameter *cache* refers to an apt_pkg.Cache() object.";
PyTypeObject PyDepCache_Type =
{
   PyVarObject_HEAD_INIT(&PyType_Type, 0)
   "apt_pkg.DepCache",                  // tp_name
   sizeof(CppPyObject<pkgDepCache *>),   // tp_basicsize
   0,                                   // tp_itemsize
   // Methods
   CppDeallocPtr<pkgDepCache *>,   // tp_dealloc
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
   doc_PkgDepCache,                     // tp_doc
   CppTraverse<pkgDepCache *>,     // tp_traverse
   CppClear<pkgDepCache *>,        // tp_clear
   0,                                   // tp_richcompare
   0,                                   // tp_weaklistoffset
   0,                                   // tp_iter
   0,                                   // tp_iternext
   PkgDepCacheMethods,                  // tp_methods
   0,                                   // tp_members
   PkgDepCacheGetSet,                   // tp_getset
   0,                                   // tp_base
   0,                                   // tp_dict
   0,                                   // tp_descr_get
   0,                                   // tp_descr_set
   0,                                   // tp_dictoffset
   0,                                   // tp_init
   0,                                   // tp_alloc
   PkgDepCacheNew,                      // tp_new
};

#ifdef COMPAT_0_7
PyObject *GetDepCache(PyObject *Self,PyObject *Args)
{
    if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
       PyErr_WarnEx(PyExc_DeprecationWarning,"apt_pkg.GetDepCache() is deprecated"
                    ". Please see apt_pkg.DepCache() for the replacement.",1);
    return PkgDepCacheNew(&PyDepCache_Type,Args,0);
}
#endif



									/*}}}*/


// pkgProblemResolver Class						/*{{{*/
// ---------------------------------------------------------------------
static PyObject *PkgProblemResolverNew(PyTypeObject *type,PyObject *Args,PyObject *kwds)
{
   PyObject *Owner;
   char *kwlist[] = {"depcache",0};
   if (PyArg_ParseTupleAndKeywords(Args,kwds,"O!",kwlist,&PyDepCache_Type,
                                   &Owner) == 0)
      return 0;

   pkgDepCache *depcache = GetCpp<pkgDepCache*>(Owner);
   pkgProblemResolver *fixer = new pkgProblemResolver(depcache);
   CppPyObject<pkgProblemResolver*> *PkgProblemResolverPyObj;
   PkgProblemResolverPyObj = CppPyObject_NEW<pkgProblemResolver*>(Owner,
						      type,
						      fixer);
   HandleErrors(PkgProblemResolverPyObj);

   return PkgProblemResolverPyObj;
}

#ifdef COMPAT_0_7
PyObject *GetPkgProblemResolver(PyObject *Self,PyObject *Args) {
    if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
       PyErr_WarnEx(PyExc_DeprecationWarning, "apt_pkg.GetPkgProblemResolver() is"
                    " deprecated. Please see apt_pkg.ProblemResolver() for the "
                    "replacement.", 1);
    return PkgProblemResolverNew(&PyProblemResolver_Type,Args,0);
}
#endif

static PyObject *PkgProblemResolverResolve(PyObject *Self,PyObject *Args)
{
   bool res;
   pkgProblemResolver *fixer = GetCpp<pkgProblemResolver *>(Self);

   char brokenFix=1;
   if (PyArg_ParseTuple(Args,"|b",&brokenFix) == 0)
      return 0;

   Py_BEGIN_ALLOW_THREADS
   res = fixer->Resolve(brokenFix);
   Py_END_ALLOW_THREADS

   return HandleErrors(Py_BuildValue("b", res));
}

static PyObject *PkgProblemResolverResolveByKeep(PyObject *Self,PyObject *Args)
{
   bool res;
   pkgProblemResolver *fixer = GetCpp<pkgProblemResolver *>(Self);
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;

   Py_BEGIN_ALLOW_THREADS
   res = fixer->ResolveByKeep();
   Py_END_ALLOW_THREADS

   return HandleErrors(Py_BuildValue("b", res));
}

static PyObject *PkgProblemResolverProtect(PyObject *Self,PyObject *Args)
{
   pkgProblemResolver *fixer = GetCpp<pkgProblemResolver *>(Self);
   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;
   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   fixer->Protect(Pkg);
   Py_INCREF(Py_None);
   return HandleErrors(Py_None);

}
static PyObject *PkgProblemResolverRemove(PyObject *Self,PyObject *Args)
{
   pkgProblemResolver *fixer = GetCpp<pkgProblemResolver *>(Self);
   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;
   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   fixer->Remove(Pkg);
   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}

static PyObject *PkgProblemResolverClear(PyObject *Self,PyObject *Args)
{
   pkgProblemResolver *fixer = GetCpp<pkgProblemResolver *>(Self);
   PyObject *PackageObj;
   if (PyArg_ParseTuple(Args,"O!",&PyPackage_Type,&PackageObj) == 0)
      return 0;
   pkgCache::PkgIterator &Pkg = GetCpp<pkgCache::PkgIterator>(PackageObj);
   fixer->Clear(Pkg);
   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}

static PyObject *PkgProblemResolverInstallProtect(PyObject *Self,PyObject *Args)
{
   pkgProblemResolver *fixer = GetCpp<pkgProblemResolver *>(Self);
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;
   fixer->InstallProtect();
   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}

static PyMethodDef PkgProblemResolverMethods[] =
{
   // config
   {"protect", PkgProblemResolverProtect, METH_VARARGS, "protect(PkgIterator)"},
   {"remove", PkgProblemResolverRemove, METH_VARARGS, "remove(PkgIterator)"},
   {"clear", PkgProblemResolverClear, METH_VARARGS, "clear(PkgIterator)"},
   {"install_protect", PkgProblemResolverInstallProtect, METH_VARARGS, "install_protect()"},

   // Actions
   {"resolve", PkgProblemResolverResolve, METH_VARARGS, "Try to intelligently resolve problems by installing and removing packages"},
   {"resolve_by_keep", PkgProblemResolverResolveByKeep, METH_VARARGS, "Try to resolv problems only by using keep"},
   {}
};

PyTypeObject PyProblemResolver_Type =
{
   PyVarObject_HEAD_INIT(&PyType_Type, 0)
   "apt_pkg.ProblemResolver",                       // tp_name
   sizeof(CppPyObject<pkgProblemResolver *>),   // tp_basicsize
   0,                                   // tp_itemsize
   // Methods
   CppDeallocPtr<pkgProblemResolver *>,// tp_dealloc
   0,                                   // tp_print
   0,                                   // tp_getattr
   0,                                   // tp_setattr
   0,                                   // tp_compare
   0,                                   // tp_repr
   0,                                   // tp_as_number
   0,                                   // tp_as_sequence
   0,	                                // tp_as_mapping
   0,                                   // tp_hash
   0,                                   // tp_call
   0,                                   // tp_str
   _PyAptObject_getattro,               // tp_getattro
   0,                                   // tp_setattro
   0,                                   // tp_as_buffer
   (Py_TPFLAGS_DEFAULT |                // tp_flags
    Py_TPFLAGS_BASETYPE |
    Py_TPFLAGS_HAVE_GC),
   "ProblemResolver Object",            // tp_doc
   CppTraverse<pkgProblemResolver *>, // tp_traverse
   CppClear<pkgProblemResolver *>, // tp_clear
   0,                                   // tp_richcompare
   0,                                   // tp_weaklistoffset
   0,                                   // tp_iter
   0,                                   // tp_iternext
   PkgProblemResolverMethods,           // tp_methods
   0,                                   // tp_members
   0,                                   // tp_getset
   0,                                   // tp_base
   0,                                   // tp_dict
   0,                                   // tp_descr_get
   0,                                   // tp_descr_set
   0,                                   // tp_dictoffset
   0,                                   // tp_init
   0,                                   // tp_alloc
   PkgProblemResolverNew,               // tp_new
};

									/*}}}*/

// pkgActionGroup Class						        /*{{{*/
// ---------------------------------------------------------------------


static PyObject *PkgActionGroupRelease(PyObject *Self,PyObject *Args)
{
   pkgDepCache::ActionGroup *ag = GetCpp<pkgDepCache::ActionGroup*>(Self);
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;
   ag->release();
   Py_INCREF(Py_None);
   return HandleErrors(Py_None);
}

static PyObject *PkgActionGroupEnter(PyObject *Self,PyObject *Args) {
   if (PyArg_ParseTuple(Args,"") == 0)
      return 0;
    return Self;
}
static PyObject *PkgActionGroupExit(PyObject *Self,PyObject *Args) {
   pkgDepCache::ActionGroup *ag = GetCpp<pkgDepCache::ActionGroup*>(Self);
   ag->release();
   Py_RETURN_FALSE;
}

static PyMethodDef PkgActionGroupMethods[] =
{
   {"release", PkgActionGroupRelease, METH_VARARGS, "release()"},
   {"__exit__", PkgActionGroupExit, METH_VARARGS, "__exit__(...) -> "
               "Release the action group, for 'with' statement."},
   {"__enter__", PkgActionGroupEnter, METH_VARARGS, "__enter__() -> "
               "Enter, for the 'with' statement. Does nothing."},
   {}
};

static PyObject *PkgActionGroupNew(PyTypeObject *type,PyObject *Args,PyObject *kwds)
{
   PyObject *Owner;
   char *kwlist[] = {"depcache", 0};
   if (PyArg_ParseTupleAndKeywords(Args,kwds,"O!",kwlist,&PyDepCache_Type,
                                   &Owner) == 0)
      return 0;

   pkgDepCache *depcache = GetCpp<pkgDepCache*>(Owner);
   pkgDepCache::ActionGroup *group = new pkgDepCache::ActionGroup(*depcache);
   CppPyObject<pkgDepCache::ActionGroup*> *PkgActionGroupPyObj;
   PkgActionGroupPyObj = CppPyObject_NEW<pkgDepCache::ActionGroup*>(Owner,
						      type,
						      group);
   HandleErrors(PkgActionGroupPyObj);

   return PkgActionGroupPyObj;

}

static char *doc_PkgActionGroup = "ActionGroup(depcache)\n\n"
    "Create a new ActionGroup() object. The parameter *depcache* refers to an\n"
    "apt_pkg.DepCache() object.\n\n"
    "ActionGroups disable certain cleanup actions, so modifying many packages\n"
    "is much faster.\n\n"
    "ActionGroup() can also be used with the 'with' statement, but be aware\n"
    "that the ActionGroup() is active as soon as it is created, and not just\n"
    "when entering the context. This means you can write::\n\n"
    "    with apt_pkg.ActionGroup(depcache):\n"
    "        depcache.markInstall(pkg)\n\n"
    "Once the block of the with statement is left, the action group is \n"
    "automatically released from the cache.";


PyTypeObject PyActionGroup_Type =
{
   PyVarObject_HEAD_INIT(&PyType_Type, 0)
   "apt_pkg.ActionGroup",               // tp_name
   sizeof(CppPyObject<pkgDepCache::ActionGroup*>),   // tp_basicsize
   0,                                   // tp_itemsize
   // Methods
   CppDeallocPtr<pkgDepCache::ActionGroup*>,        // tp_dealloc
   0,                                   // tp_print
   0,                                   // tp_getattr
   0,                                   // tp_setattr
   0,                                   // tp_compare
   0,                                   // tp_repr
   0,                                   // tp_as_number
   0,                                   // tp_as_sequence
   0,	                                // tp_as_mapping
   0,                                   // tp_hash
   0,                                   // tp_call
   0,                                   // tp_str
   0,                                   // tp_getattro
   0,                                   // tp_setattro
   0,                                   // tp_as_buffer
   (Py_TPFLAGS_DEFAULT |                // tp_flags
    Py_TPFLAGS_BASETYPE |
    Py_TPFLAGS_HAVE_GC),
   doc_PkgActionGroup,                  // tp_doc
   CppTraverse<pkgDepCache::ActionGroup*>, // tp_traverse
   CppClear<pkgDepCache::ActionGroup*>, // tp_clear
   0,                                   // tp_richcompare
   0,                                   // tp_weaklistoffset
   0,                                   // tp_iter
   0,                                   // tp_iternext
   PkgActionGroupMethods,               // tp_methods
   0,                                   // tp_members
   0,                                   // tp_getset
   0,                                   // tp_base
   0,                                   // tp_dict
   0,                                   // tp_descr_get
   0,                                   // tp_descr_set
   0,                                   // tp_dictoffset
   0,                                   // tp_init
   0,                                   // tp_alloc
   PkgActionGroupNew,                   // tp_new
};

#ifdef COMPAT_0_7
PyObject *GetPkgActionGroup(PyObject *Self,PyObject *Args)
{
    if (getenv("PYTHON_APT_DEPRECATION_WARNINGS") != NULL)
       PyErr_WarnEx(PyExc_DeprecationWarning, "apt_pkg.GetPkgActionGroup() is "
                    "deprecated. Please see apt_pkg.ActionGroup() for the "
                    "replacement.", 1);
    return PkgActionGroupNew(&PyActionGroup_Type,Args,0);
}
#endif


									/*}}}*/
