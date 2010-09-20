// Description								/*{{{*/
// $Id: progress.cc,v 1.5 2003/06/03 03:03:23 mvo Exp $
/* ######################################################################

   Progress - Wrapper for the progress related functions

   ##################################################################### */
#include <Python.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <utility>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/acquire-worker.h>
#include "progress.h"
#include "generic.h"
#include "apt_pkgmodule.h"

/**
 * Set an attribute on an object, after creating the value with
 * Py_BuildValue(fmt, arg). Afterwards, decrease its refcount and return
 * whether setting the attribute was successful.
 */
template<class T>
inline bool setattr(PyObject *object, const char *attr, const char *fmt, T arg)
{
    if (!object)
        return false;
    PyObject *value = Py_BuildValue(fmt, arg);

    int result = PyObject_SetAttrString(object, attr, value);
    Py_DECREF(value);
    return result != -1;
}

inline PyObject *TUPLEIZE(PyObject *op) {
    PyObject *ret = Py_BuildValue("(O)", op);
    Py_DECREF(op);
    return ret;
}

// generic
bool PyCallbackObj::RunSimpleCallback(const char* method_name,
				      PyObject *arglist,
				      PyObject **res)
{
   if(callbackInst == 0) {
      Py_XDECREF(arglist);
      return false;
   }

   PyObject *method = PyObject_GetAttrString(callbackInst,(char*) method_name);
   if(method == NULL) {
      //std::cerr << "Can't find '" << method_name << "' method" << std::endl;
      Py_XDECREF(arglist);
      if (res) {
	 Py_INCREF(Py_None);
	 *res = Py_None;
      }
      return false;
   }

   PyObject *result = PyObject_CallObject(method, arglist);
   Py_XDECREF(arglist);

   if(result == NULL) {
      // exception happend
      std::cerr << "Error in function " << method_name << std::endl;
      PyErr_Print();
      PyErr_Clear();

      return false;
   }
   if(res != NULL)
      *res = result;
   else
      Py_XDECREF(result);
   Py_XDECREF(method);

   return true;
}


// OpProgress interface
void PyOpProgress::Update()
{
   // Build up the argument list...
   if(!CheckChange(0.7))
      return;

   setattr(callbackInst, "op", "s", Op.c_str());
   setattr(callbackInst, "subop", "s", SubOp.c_str());
   setattr(callbackInst, "major_change", "b", MajorChange);
   setattr(callbackInst, "percent", "f", Percent);
#ifdef COMPAT_0_7
   setattr(callbackInst, "Op", "s", Op.c_str());
   setattr(callbackInst, "subOp", "s", SubOp.c_str());
   setattr(callbackInst, "majorChange", "b", MajorChange);
   PyObject *arglist = Py_BuildValue("(f)", Percent);
   RunSimpleCallback("update", arglist);
#else
   RunSimpleCallback("update");
#endif
}

void PyOpProgress::Done()
{
   RunSimpleCallback("done");
}



// fetcher interface



// apt interface

PyObject *PyFetchProgress::GetDesc(pkgAcquire::ItemDesc *item) {
    if (!pyAcquire && item->Owner && item->Owner->GetOwner()) {
        pyAcquire = PyAcquire_FromCpp(item->Owner->GetOwner(), false, NULL);
    }
    PyObject *pyItem = PyAcquireItem_FromCpp(item->Owner, false, pyAcquire);
    PyObject *pyDesc = PyAcquireItemDesc_FromCpp(item, false, pyItem);
    Py_DECREF(pyItem);
    return pyDesc;
}

bool PyFetchProgress::MediaChange(string Media, string Drive)
{
   PyCbObj_END_ALLOW_THREADS
   //std::cout << "MediaChange" << std::endl;
   PyObject *arglist = Py_BuildValue("(ss)", Media.c_str(), Drive.c_str());
   PyObject *result;

   if(PyObject_HasAttrString(callbackInst, "mediaChange"))
      RunSimpleCallback("mediaChange", arglist, &result);
   else
      RunSimpleCallback("media_change", arglist, &result);

   bool res = true;
   if(!PyArg_Parse(result, "b", &res)) {
      // no return value or None, assume false
      PyCbObj_BEGIN_ALLOW_THREADS
      return false;
   }

   PyCbObj_BEGIN_ALLOW_THREADS
   return res;
}

void PyFetchProgress::UpdateStatus(pkgAcquire::ItemDesc &Itm, int status)
{
   //std::cout << "UpdateStatus: " << Itm.URI << " " << status << std::endl;
   // Added object file size and object partial size to
   // parameters that are passed to updateStatus.
   // -- Stephan
   PyObject *arglist = Py_BuildValue("(sssikk)", Itm.URI.c_str(),
				     Itm.Description.c_str(),
				     Itm.ShortDesc.c_str(),
				     status,
				     Itm.Owner->FileSize,
				     Itm.Owner->PartialSize);

   RunSimpleCallback("update_status_full", arglist);

   // legacy version of the interface

   arglist = Py_BuildValue("(sssi)", Itm.URI.c_str(), Itm.Description.c_str(),
                           Itm.ShortDesc.c_str(), status);

   if(PyObject_HasAttrString(callbackInst, "updateStatus"))
      RunSimpleCallback("updateStatus", arglist);
   else
      RunSimpleCallback("update_status", arglist);
}

void PyFetchProgress::IMSHit(pkgAcquire::ItemDesc &Itm)
{
   PyCbObj_END_ALLOW_THREADS
   if (PyObject_HasAttrString(callbackInst, "ims_hit"))
       RunSimpleCallback("ims_hit", TUPLEIZE(GetDesc(&Itm)));
   else
       UpdateStatus(Itm, DLHit);
   PyCbObj_BEGIN_ALLOW_THREADS
}

void PyFetchProgress::Fetch(pkgAcquire::ItemDesc &Itm)
{
   PyCbObj_END_ALLOW_THREADS
   if (PyObject_HasAttrString(callbackInst, "fetch"))
       RunSimpleCallback("fetch", TUPLEIZE(GetDesc(&Itm)));
   else
       UpdateStatus(Itm, DLQueued);
   PyCbObj_BEGIN_ALLOW_THREADS
}

void PyFetchProgress::Done(pkgAcquire::ItemDesc &Itm)
{
   PyCbObj_END_ALLOW_THREADS
   if (PyObject_HasAttrString(callbackInst, "done"))
       RunSimpleCallback("done", TUPLEIZE(GetDesc(&Itm)));
   else
       UpdateStatus(Itm, DLDone);
   PyCbObj_BEGIN_ALLOW_THREADS
}

void PyFetchProgress::Fail(pkgAcquire::ItemDesc &Itm)
{
   PyCbObj_END_ALLOW_THREADS
   if (PyObject_HasAttrString(callbackInst, "fail")) {
       RunSimpleCallback("fail", TUPLEIZE(GetDesc(&Itm)));
       PyCbObj_BEGIN_ALLOW_THREADS
       return;
   }

   // Ignore certain kinds of transient failures (bad code)
   if (Itm.Owner->Status == pkgAcquire::Item::StatIdle) {
      PyCbObj_BEGIN_ALLOW_THREADS
      return;
   }

   if (Itm.Owner->Status == pkgAcquire::Item::StatDone)
   {
      UpdateStatus(Itm, DLIgnored);
   }


   if (PyObject_HasAttrString(callbackInst, "fail"))
       RunSimpleCallback("fail", TUPLEIZE(GetDesc(&Itm)));
   else
       UpdateStatus(Itm, DLFailed);
   PyCbObj_BEGIN_ALLOW_THREADS
}

void PyFetchProgress::Start()
{
   //std::cout << "Start" << std::endl;
   pkgAcquireStatus::Start();

#ifdef COMPAT_0_7
   setattr(callbackInst, "currentCPS", "d", 0);
   setattr(callbackInst, "currentBytes", "d", 0);
   setattr(callbackInst, "currentItems", "k", 0);
   setattr(callbackInst, "totalItems", "k", 0);
   setattr(callbackInst, "totalBytes", "d", 0);
#endif

   RunSimpleCallback("start");
   /* After calling the start method we can safely allow
    * other Python threads to do their work for now.
    */
   PyCbObj_BEGIN_ALLOW_THREADS
}


void PyFetchProgress::Stop()
{
   /* After the stop operation occured no other threads
    * are allowed. This is done so we have a matching
    * PyCbObj_END_ALLOW_THREADS to our previous
    * PyCbObj_BEGIN_ALLOW_THREADS (Python requires this!).
    */

   PyCbObj_END_ALLOW_THREADS
   //std::cout << "Stop" << std::endl;
   pkgAcquireStatus::Stop();
   RunSimpleCallback("stop");
}

bool PyFetchProgress::Pulse(pkgAcquire * Owner)
{
   PyCbObj_END_ALLOW_THREADS
   pkgAcquireStatus::Pulse(Owner);

   //std::cout << "Pulse" << std::endl;
   if(callbackInst == 0) {
      PyCbObj_BEGIN_ALLOW_THREADS
      return false;
   }

   setattr(callbackInst, "last_bytes", "d", LastBytes);
   setattr(callbackInst, "current_cps", "d", CurrentCPS);
   setattr(callbackInst, "current_bytes", "d", CurrentBytes);
   setattr(callbackInst, "total_bytes", "d", TotalBytes);
   setattr(callbackInst, "fetched_bytes", "d", FetchedBytes);
   setattr(callbackInst, "elapsed_time", "k", ElapsedTime);
   setattr(callbackInst, "current_items", "k", CurrentItems);
   setattr(callbackInst, "total_items", "k", TotalItems);

   // New style
   if (!PyObject_HasAttrString(callbackInst, "updateStatus")) {
      PyObject *result1;
      bool res1 = true;

      if (pyAcquire == NULL) {
         pyAcquire = PyAcquire_FromCpp(Owner, false, NULL);
      }
      Py_INCREF(pyAcquire);

      if (RunSimpleCallback("pulse", TUPLEIZE(pyAcquire) , &result1)) {
      if (result1 != NULL && PyArg_Parse(result1, "b", &res1) && res1 == false) {
         // the user returned a explicit false here, stop
         PyCbObj_BEGIN_ALLOW_THREADS
         return false;
      }
     }
     PyCbObj_BEGIN_ALLOW_THREADS
     return true;
   }
#ifdef COMPAT_0_7
   setattr(callbackInst, "currentCPS", "d", CurrentCPS);
   setattr(callbackInst, "currentBytes", "d", CurrentBytes);
   setattr(callbackInst, "totalBytes", "d", TotalBytes);
   setattr(callbackInst, "fetchedBytes", "d", FetchedBytes);
   setattr(callbackInst, "currentItems", "k", CurrentItems);
   setattr(callbackInst, "totalItems", "k", TotalItems);
   // Go through the list of items and add active items to the
   // activeItems vector.
   map<pkgAcquire::Worker *, pkgAcquire::ItemDesc *> activeItemMap;

   for(pkgAcquire::Worker *Worker = Owner->WorkersBegin();
       Worker != 0; Worker = Owner->WorkerStep(Worker)) {

     if (Worker->CurrentItem == 0) {
       // Ignore workers with no item running
       continue;
     }
     activeItemMap.insert(std::make_pair(Worker, Worker->CurrentItem));
   }

   // Create the tuple that is passed as argument to pulse().
   // This tuple contains activeItemMap.size() item tuples.
   PyObject *arglist;

   if (((int)activeItemMap.size()) > 0) {
     PyObject *itemsTuple = PyTuple_New((Py_ssize_t) activeItemMap.size());

     // Go through activeItems, create an item tuple in the form
     // (URI, Description, ShortDesc, FileSize, PartialSize) and
     // add that tuple to itemsTuple.
     map<pkgAcquire::Worker *, pkgAcquire::ItemDesc *>::iterator iter;
     int tuplePos;

     for(tuplePos = 0, iter = activeItemMap.begin();
         iter != activeItemMap.end(); ++iter, tuplePos++) {
       pkgAcquire::Worker *worker = iter->first;
       pkgAcquire::ItemDesc *itm = iter->second;

       PyObject *itmTuple = Py_BuildValue("(ssskk)", itm->URI.c_str(),
					  itm->Description.c_str(),
					  itm->ShortDesc.c_str(),
					  worker->TotalSize,
					  worker->CurrentSize);
       PyTuple_SetItem(itemsTuple, tuplePos, itmTuple);
     }

     // Now our itemsTuple is ready for being passed to pulse().
     // pulse() is going to receive a single argument, being the
     // tuple of items, which again contains one tuple with item
     // information per item.
     //
     // Python Example:
     //
     // class MyFetchProgress(FetchProgress):
     //   def pulse(self, items):
     //     for itm in items:
     //       uri, desc, shortdesc, filesize, partialsize = itm
     //
     arglist = PyTuple_Pack(1, itemsTuple);
   }
   else {
     arglist = Py_BuildValue("(())");
   }

   PyObject *result;
   bool res = true;

   if (RunSimpleCallback("pulse_items", arglist, &result)) {
      if (result != NULL && PyArg_Parse(result, "b", &res) && res == false) {
         // the user returned a explicit false here, stop
         PyCbObj_BEGIN_ALLOW_THREADS
         return false;
      }
   }


   arglist = Py_BuildValue("()");
   if (!RunSimpleCallback("pulse", arglist, &result)) {
     PyCbObj_BEGIN_ALLOW_THREADS
     return true;
   }

   if((result == NULL) || (!PyArg_Parse(result, "b", &res)))
   {
      // most of the time the user who subclasses the pulse()
      // method forgot to add a return {True,False} so we just
      // assume he wants a True
      PyCbObj_BEGIN_ALLOW_THREADS
      return true;
   }

   PyCbObj_BEGIN_ALLOW_THREADS
   // fetching can be canceld by returning false
   return res;
#else
   return false;
#endif
}



// install progress

void PyInstallProgress::StartUpdate()
{
   if (!RunSimpleCallback("startUpdate"))
       RunSimpleCallback("start_update");
   PyCbObj_BEGIN_ALLOW_THREADS
}

void PyInstallProgress::UpdateInterface()
{
   PyCbObj_END_ALLOW_THREADS
   if (!RunSimpleCallback("updateInterface"))
       RunSimpleCallback("update_interface");
   PyCbObj_BEGIN_ALLOW_THREADS
}

void PyInstallProgress::FinishUpdate()
{
   PyCbObj_END_ALLOW_THREADS
   if (!RunSimpleCallback("finishUpdate"))
       RunSimpleCallback("finish_update");
}

pkgPackageManager::OrderResult PyInstallProgress::Run(pkgPackageManager *pm)
{
   pkgPackageManager::OrderResult res;
   int ret;
   pid_t child_id;

#if 0 // FIXME: this needs to be merged into apt to support medium swaping
   res = pm->DoInstallPreFork();
   if (res == pkgPackageManager::Failed)
       return res;
#endif

   // support custom fork methods
   if(PyObject_HasAttrString(callbackInst, "fork")) {
      PyObject *method = PyObject_GetAttrString(callbackInst, "fork");
      std::cerr << "custom fork found" << std::endl;
      PyObject *arglist = Py_BuildValue("()");
      PyObject *result = PyObject_CallObject(method, arglist);
      Py_DECREF(arglist);
      if (result == NULL) {
	 std::cerr << "fork method invalid" << std::endl;
	 PyErr_Print();
	 return pkgPackageManager::Failed;
      }
      if(!PyArg_Parse(result, "i", &child_id) ) {
	 std::cerr << "custom fork() result could not be parsed?"<< std::endl;
	 return pkgPackageManager::Failed;
      }
      std::cerr << "got pid: " << child_id << std::endl;
   } else {
      //std::cerr << "using build-in fork()" << std::endl;
      child_id = fork();
   }


#if 0 // FIXME: this needs to be merged into apt to support medium swaping
   if (child_id == 0) {
      res = pm->DoInstallPostFork();
      _exit(res);
   }
#endif
   if (child_id == 0) {
      PyObject *v = PyObject_GetAttrString(callbackInst, "writefd");
      if(v) {
	 int fd = PyObject_AsFileDescriptor(v);
	 cout << "got fd: " << fd << endl;
	 res = pm->DoInstall(fd);
      } else {
	 res = pm->DoInstall();
      }
      //std::cout << "res: " << res << std::endl;
      _exit(res);
   }

   StartUpdate();


   PyCbObj_END_ALLOW_THREADS
   if(PyObject_HasAttrString(callbackInst, "waitChild") ||
      PyObject_HasAttrString(callbackInst, "wait_child")) {
      PyObject *method;
      if (PyObject_HasAttrString(callbackInst, "waitChild"))
          method = PyObject_GetAttrString(callbackInst, "waitChild");
      else
          method = PyObject_GetAttrString(callbackInst, "wait_child");
      //std::cerr << "custom waitChild found" << std::endl;
      PyObject *arglist = Py_BuildValue("(i)",child_id);
      PyObject *result = PyObject_CallObject(method, arglist);
      Py_DECREF(arglist);
      if (result == NULL) {
	 std::cerr << "waitChild method invalid" << std::endl;
	 PyErr_Print();
	 PyCbObj_BEGIN_ALLOW_THREADS
	 return pkgPackageManager::Failed;
      }
      if(!PyArg_Parse(result, "i", &res) ) {
	 std::cerr << "custom waitChild() result could not be parsed?"<< std::endl;
	 PyCbObj_BEGIN_ALLOW_THREADS
	 return pkgPackageManager::Failed;
      }
      PyCbObj_BEGIN_ALLOW_THREADS
      //std::cerr << "got child_res: " << res << std::endl;
   } else {
      //std::cerr << "using build-in waitpid()" << std::endl;
      PyCbObj_BEGIN_ALLOW_THREADS
      while (waitpid(child_id, &ret, WNOHANG) == 0) {
	 PyCbObj_END_ALLOW_THREADS
	 UpdateInterface();
         PyCbObj_BEGIN_ALLOW_THREADS
      }

      res = (pkgPackageManager::OrderResult) WEXITSTATUS(ret);
      //std::cerr << "build-in waitpid() got: " << res << std::endl;
   }

   FinishUpdate();

   return res;
}


//-----------------------------------------------------------------------------
// apt-cdrom interface

void PyCdromProgress::Update(string text, int current)
{
   PyObject *arglist = Py_BuildValue("(si)", text.c_str(), current);
   setattr(callbackInst, "total_steps", "i", totalSteps);
   #ifdef COMPAT_0_7
   setattr(callbackInst, "totalSteps", "i", totalSteps);
   #endif
   RunSimpleCallback("update", arglist);
}

bool PyCdromProgress::ChangeCdrom()
{
   PyObject *arglist = Py_BuildValue("()");
   PyObject *result;
   if (PyObject_HasAttrString(callbackInst, "changeCdrom"))
      RunSimpleCallback("changeCdrom", arglist, &result);
   else
      RunSimpleCallback("change_cdrom", arglist, &result);

   bool res = true;
   if(!PyArg_Parse(result, "b", &res))
      std::cerr << "ChangeCdrom: result could not be parsed" << std::endl;

   return res;
}


bool PyCdromProgress::AskCdromName(string &Name)
{
   PyObject *arglist = Py_BuildValue("()");
   const char *new_name;
   bool res;
   PyObject *result;

   // Old style: (True, name) on success, (False, name) on failure.
   if (PyObject_HasAttrString(callbackInst, "askAdromName")) {
      RunSimpleCallback("askAdromName", arglist, &result);
      if(!PyArg_Parse(result, "(bs)", &res, &new_name))
         std::cerr << "AskCdromName: result could not be parsed" << std::endl;
      // set the new name
      Name = string(new_name);
      return res;
   }
   // New style: String on success, None on failure.
   else {
        RunSimpleCallback("ask_cdrom_name", arglist, &result);
        if(result == Py_None)
            return false;
        if(!PyArg_Parse(result, "s", &new_name))
            std::cerr << "ask_cdrom_name: result could not be parsed" << std::endl;
        else
            Name = string(new_name);
            return true;
  }
}
