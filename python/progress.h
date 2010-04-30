// Description								/*{{{*/
// $Id: progress.h,v 1.5 2003/06/03 03:03:23 mdz Exp $
/* ######################################################################

   Progress - Wrapper for the progress related functions

   ##################################################################### */

#ifndef PROGRESS_H
#define PROGRESS_H

#include <apt-pkg/progress.h>
#include <apt-pkg/acquire.h>
#include <apt-pkg/packagemanager.h>
#include <apt-pkg/cdrom.h>
#include <Python.h>

/* PyCbObj_BEGIN_ALLOW_THREADS and PyCbObj_END_ALLOW_THREADS are sligthly
 * modified versions of Py_BEGIN_ALLOW_THREADS and Py_END_ALLOW_THREADS.
 * Instead of storing the thread state in a function-local variable these
 * use a class attribute (with the same) name, allowing blocking and 
 * unblocking from different class methods.
 * Py_BLOCK_THREADS and Py_UNBLOCK_THREADS do not define their own
 * local variable but use the one provided by PyCbObj_BEGIN_ALLOW_THREADS
 * and thus are the same as Py_BLOCK_THREADS and Py_UNBLOCK_THREADS.
 */
#define PyCbObj_BEGIN_ALLOW_THREADS \
  _save = PyEval_SaveThread();
#define PyCbObj_END_ALLOW_THREADS \
  PyEval_RestoreThread(_save); \
  _save = NULL;
#define PyCbObj_BLOCK_THREADS Py_BLOCK_THREADS
#define PyCbObj_UNBLOCK_THREADS Py_UNBLOCK_THREADS

class PyCallbackObj {
 protected:
   PyObject *callbackInst;
   PyThreadState *_save;

 public:
   void setCallbackInst(PyObject *o) {
      Py_INCREF(o);
      callbackInst = o;
   }

   bool RunSimpleCallback(const char *method, PyObject *arglist=NULL,
			  PyObject **result=NULL);

   PyCallbackObj() : callbackInst(0) {};
   ~PyCallbackObj()  {Py_DECREF(callbackInst); };
};

struct PyOpProgress : public OpProgress, public PyCallbackObj
{

   virtual void Update();
   virtual void Done();

   PyOpProgress() : OpProgress(), PyCallbackObj() {};
};


struct PyFetchProgress : public pkgAcquireStatus, public PyCallbackObj
{
   enum {
      DLDone, DLQueued, DLFailed, DLHit, DLIgnored
   };

   void UpdateStatus(pkgAcquire::ItemDesc & Itm, int status);

   virtual bool MediaChange(string Media, string Drive);

   /* apt stuff */
   virtual void IMSHit(pkgAcquire::ItemDesc &Itm);
   virtual void Fetch(pkgAcquire::ItemDesc &Itm);
   virtual void Done(pkgAcquire::ItemDesc &Itm);
   virtual void Fail(pkgAcquire::ItemDesc &Itm);
   virtual void Start();
   virtual void Stop();

   bool Pulse(pkgAcquire * Owner);
   PyFetchProgress() : PyCallbackObj() {};
};

struct PyInstallProgress : public PyCallbackObj
{
   void StartUpdate();
   void UpdateInterface();
   void FinishUpdate();

   pkgPackageManager::OrderResult Run(pkgPackageManager *pm);

   PyInstallProgress() : PyCallbackObj() {};
};

struct PyCdromProgress : public pkgCdromStatus, public PyCallbackObj
{
   // update steps, will be called regularly as a "pulse"
   virtual void Update(string text="", int current=0);
   // ask for cdrom insert
   virtual bool ChangeCdrom();
   // ask for cdrom name
   virtual bool AskCdromName(string &Name);

   PyCdromProgress() : PyCallbackObj() {};
};


#endif
