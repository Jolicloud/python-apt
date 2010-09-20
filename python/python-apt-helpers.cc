/*
 * python-apt.h - Common object creation functions for the public API
 *
 * Copyright 2010 Julian Andres Klode <jak@debian.org>
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

#include "apt_pkgmodule.h"

/**
 * Short macro to be used for the generic FromCpp functions.
 *
 */
#define NEW_FROM(NAME,TYPE,Cpp) \
PyObject* NAME(Cpp const &obj, bool Delete, PyObject *Owner) \
{ \
    CppPyObject<Cpp> *Obj = CppPyObject_NEW<Cpp>(Owner, TYPE, obj); \
    Obj->NoDelete = (!Delete); \
    return Obj; \
}

// Create all those functions
NEW_FROM(PyAcquireFile_FromCpp,&PyAcquireFile_Type,pkgAcqFile*)
NEW_FROM(PyAcquireItem_FromCpp,&PyAcquireItem_Type,pkgAcquire::Item*)
NEW_FROM(PyAcquireItemDesc_FromCpp,&PyAcquireItemDesc_Type,pkgAcquire::ItemDesc*)
NEW_FROM(PyAcquireWorker_FromCpp,&PyAcquireWorker_Type,pkgAcquire::Worker*)
NEW_FROM(PyActionGroup_FromCpp,&PyActionGroup_Type,pkgDepCache::ActionGroup*)
NEW_FROM(PyCache_FromCpp,&PyCache_Type,pkgCache*)
NEW_FROM(PyCacheFile_FromCpp,&PyCacheFile_Type,pkgCacheFile*)
NEW_FROM(PyCdrom_FromCpp,&PyCdrom_Type,pkgCdrom)
NEW_FROM(PyConfiguration_FromCpp,&PyConfiguration_Type,Configuration*)
NEW_FROM(PyDepCache_FromCpp,&PyDepCache_Type,pkgDepCache*)
NEW_FROM(PyDependency_FromCpp,&PyDependency_Type,pkgCache::DepIterator)
//NEW_FROM(PyDependencyList_FromCpp,&PyDependencyList_Type,RDepListStruct)
NEW_FROM(PyDescription_FromCpp,&PyDescription_Type,pkgCache::DescIterator)
NEW_FROM(PyHashes_FromCpp,&PyHashes_Type,Hashes)
NEW_FROM(PyHashString_FromCpp,&PyHashString_Type,HashString*)
NEW_FROM(PyIndexRecords_FromCpp,&PyIndexRecords_Type,indexRecords*)
NEW_FROM(PyMetaIndex_FromCpp,&PyMetaIndex_Type,metaIndex*)
NEW_FROM(PyPackage_FromCpp,&PyPackage_Type,pkgCache::PkgIterator)
NEW_FROM(PyIndexFile_FromCpp,&PyIndexFile_Type,pkgIndexFile*)
NEW_FROM(PyPackageFile_FromCpp,&PyPackageFile_Type,pkgCache::PkgFileIterator)
//NEW_FROM(PyPackageList_FromCpp,&PyPackageList_Type,PkgListStruct)
NEW_FROM(PyPackageManager_FromCpp,&PyPackageManager_Type,pkgPackageManager*)
//NEW_FROM(PyPackageRecords_FromCpp,&PyPackageRecords_Type,PkgRecordsStruct)
NEW_FROM(PyPolicy_FromCpp,&PyPolicy_Type,pkgPolicy*)
NEW_FROM(PyProblemResolver_FromCpp,&PyProblemResolver_Type,pkgProblemResolver*)
NEW_FROM(PySourceList_FromCpp,&PySourceList_Type,pkgSourceList*)
//NEW_FROM(PySourceRecords_FromCpp,&PySourceRecords_Type,PkgSrcRecordsStruct)
NEW_FROM(PyTagFile_FromCpp,&PyTagFile_Type,pkgTagFile)
NEW_FROM(PyTagSection_FromCpp,&PyTagSection_Type,pkgTagSection)
NEW_FROM(PyVersion_FromCpp,&PyVersion_Type,pkgCache::VerIterator)

#undef NEW_FROM
