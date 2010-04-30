// Description								/*{{{*/
// $Id: progress.cc,v 1.5 2003/06/03 03:03:23 mvo Exp $
/* ######################################################################

   Progress - Wrapper for the progress related functions

   ##################################################################### */

#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <utility>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/acquire-worker.h>
#include "generic.h"
#include "progress.h"

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
   PyObject *result = PyEval_CallObject(method, arglist);
   
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
   PyObject *o;
   o = Py_BuildValue("s", Op.c_str());
   PyObject_SetAttrString(callbackInst, "op", o);
   Py_XDECREF(o);
   o = Py_BuildValue("s", SubOp.c_str());
   PyObject_SetAttrString(callbackInst, "subOp", o);
   Py_XDECREF(o);
   o = Py_BuildValue("b", MajorChange);
   PyObject_SetAttrString(callbackInst, "majorChange", o);
   Py_XDECREF(o);

   // CheckChange takes a time delta argument how often we
   // should run update - for interactive UIs it makes sense
   // to run ~25/sec
   if(CheckChange(0.04))
   {
      PyObject *arglist = Py_BuildValue("(f)", Percent);
      RunSimpleCallback("update", arglist);
   }
};

void PyOpProgress::Done()
{
   RunSimpleCallback("done");
}



// fetcher interface



// apt interface

bool PyFetchProgress::MediaChange(string Media, string Drive)
{
   PyCbObj_END_ALLOW_THREADS
   //std::cout << "MediaChange" << std::endl;
   PyObject *arglist = Py_BuildValue("(ss)", Media.c_str(), Drive.c_str());
   PyObject *result;
   RunSimpleCallback("mediaChange", arglist, &result);

   bool res = true;
   if(!PyArg_Parse(result, "b", &res))
      std::cerr << "result could not be parsed" << std::endl;

   // FIXME: find out what it should return usually
   //std::cerr << "res is: " << res << std::endl;

   PyCbObj_BEGIN_ALLOW_THREADS
   return res;
}

void PyFetchProgress::UpdateStatus(pkgAcquire::ItemDesc &Itm, int status)
{
   //std::cout << "UpdateStatus: " << Itm.URI << " " << status << std::endl;

   // Added object file size and object partial size to
   // parameters that are passed to updateStatus.
   // -- Stephan
   PyCbObj_END_ALLOW_THREADS
   PyObject *arglist = Py_BuildValue("(sssikk)", Itm.URI.c_str(), 
				     Itm.Description.c_str(), 
				     Itm.ShortDesc.c_str(), 
				     status,
				     Itm.Owner->FileSize,
				     Itm.Owner->PartialSize);

   RunSimpleCallback("update_status_full", arglist);

   // legacy version of the interface
   arglist = Py_BuildValue("(sssi)", Itm.URI.c_str(), 
				     Itm.Description.c_str(), 
				     Itm.ShortDesc.c_str(), 
                                     status);
   RunSimpleCallback("updateStatus", arglist);
   PyCbObj_BEGIN_ALLOW_THREADS

}

void PyFetchProgress::IMSHit(pkgAcquire::ItemDesc &Itm)
{
   UpdateStatus(Itm, DLHit);
}

void PyFetchProgress::Fetch(pkgAcquire::ItemDesc &Itm)
{
   UpdateStatus(Itm, DLQueued);
}

void PyFetchProgress::Done(pkgAcquire::ItemDesc &Itm)
{
   UpdateStatus(Itm, DLDone);
}

void PyFetchProgress::Fail(pkgAcquire::ItemDesc &Itm)
{
   // Ignore certain kinds of transient failures (bad code)
   if (Itm.Owner->Status == pkgAcquire::Item::StatIdle)
      return;

   if (Itm.Owner->Status == pkgAcquire::Item::StatDone)
   {
      UpdateStatus(Itm, DLIgnored);
   }

   UpdateStatus(Itm, DLFailed);
}

void PyFetchProgress::Start()
{
   //std::cout << "Start" << std::endl;
   pkgAcquireStatus::Start();

   // These attributes should be initialized before the first callback (start)
   // is invoked.
   // -- Stephan
   PyObject *o;

   o = Py_BuildValue("f", 0.0f);
   PyObject_SetAttrString(callbackInst, "currentCPS", o);
   Py_XDECREF(o);
   o = Py_BuildValue("f", 0.0f);
   PyObject_SetAttrString(callbackInst, "currentBytes", o);
   Py_XDECREF(o);
   o = Py_BuildValue("i", 0);
   PyObject_SetAttrString(callbackInst, "currentItems", o);
   Py_XDECREF(o);
   o = Py_BuildValue("i", 0);
   PyObject_SetAttrString(callbackInst, "totalItems", o);
   Py_XDECREF(o);
   o = Py_BuildValue("f", 0.0f);
   PyObject_SetAttrString(callbackInst, "totalBytes", o);
   Py_XDECREF(o);

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
   if(callbackInst == 0)
      return false;

   // set stats
   PyObject *o;
   o = Py_BuildValue("f", CurrentCPS);
   PyObject_SetAttrString(callbackInst, "currentCPS", o);
   Py_XDECREF(o);
   o = Py_BuildValue("f", CurrentBytes);
   PyObject_SetAttrString(callbackInst, "currentBytes", o);
   Py_XDECREF(o);
   o = Py_BuildValue("i", CurrentItems);
   PyObject_SetAttrString(callbackInst, "currentItems", o);
   Py_XDECREF(o);
   o = Py_BuildValue("i", TotalItems);
   PyObject_SetAttrString(callbackInst, "totalItems", o);
   Py_XDECREF(o);
   o = Py_BuildValue("f", TotalBytes);
   PyObject_SetAttrString(callbackInst, "totalBytes", o);
   Py_XDECREF(o);

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

   RunSimpleCallback("pulse_items", arglist, &result);
   if (result != NULL && PyArg_Parse(result, "b", &res) && res == false) {
      // the user returned a explicit false here, stop
      PyCbObj_BEGIN_ALLOW_THREADS
      return false;
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
}



// install progress

void PyInstallProgress::StartUpdate()
{
   RunSimpleCallback("startUpdate");
   PyCbObj_BEGIN_ALLOW_THREADS
}

void PyInstallProgress::UpdateInterface()
{
   PyCbObj_END_ALLOW_THREADS
   RunSimpleCallback("updateInterface");
   PyCbObj_BEGIN_ALLOW_THREADS
}

void PyInstallProgress::FinishUpdate()
{
   PyCbObj_END_ALLOW_THREADS
   RunSimpleCallback("finishUpdate");
}

pkgPackageManager::OrderResult PyInstallProgress::Run(pkgPackageManager *pm)
{
   void *dummy;
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
      PyObject *result = PyEval_CallObject(method, arglist);
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
   if(PyObject_HasAttrString(callbackInst, "waitChild")) {
      PyObject *method = PyObject_GetAttrString(callbackInst, "waitChild");
      //std::cerr << "custom waitChild found" << std::endl;
      PyObject *arglist = Py_BuildValue("(i)",child_id);
      PyObject *result = PyEval_CallObject(method, arglist);
      Py_DECREF(arglist);
      if (result == NULL) {
	 std::cerr << "waitChild method invalid" << std::endl;
	 PyErr_Print();
	 return pkgPackageManager::Failed;
      }
      int child_res;
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

   PyObject *o = Py_BuildValue("i", totalSteps);
   PyObject_SetAttrString(callbackInst, "totalSteps", o);
   Py_XDECREF(o);

   RunSimpleCallback("update", arglist);
}

bool PyCdromProgress::ChangeCdrom()
{
   PyObject *arglist = Py_BuildValue("()");
   PyObject *result;
   RunSimpleCallback("changeCdrom", arglist, &result);

   bool res = true;
   if(!PyArg_Parse(result, "b", &res))
      std::cerr << "ChangeCdrom: result could not be parsed" << std::endl;

   return res;
}


bool PyCdromProgress::AskCdromName(string &Name)
{
   PyObject *arglist = Py_BuildValue("()");
   PyObject *result;
   RunSimpleCallback("askCdromName", arglist, &result);

   const char *new_name;
   bool res;
   if(!PyArg_Parse(result, "(bs)", &res, &new_name))
      std::cerr << "AskCdromName: result could not be parsed" << std::endl;

   //std::cerr << "got: " << res << " " << "name: " << new_name << std::endl;

   // set the new name
   Name = string(new_name);

   return res;
}
