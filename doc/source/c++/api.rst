Python APT C++ API
==================
The C++ API provides functions to create Python objects from C++ objects and
to retrieve the C++ object stored in the Python object. An object may have
another Python object as its owner and keeps its owner alive for its
lifetime. Some objects require an owner of a specific type, while others
require none. Refer to the sections below for details.

The C++ API names use the name of the class in apt_pkg and are prefixed with
Py. For each supported class, there is a _Type object, a _Check() function,
a _CheckExact() function, a _FromCpp() and a _ToCpp() function.

.. note::

    This API is experimental and should not be used in stable program
    releases.

Acquire (pkgAcquire)
--------------------
.. cvar:: PyTypeObject PyAcquire_Type

    The type object for :class:`apt_pkg.Acquire` objects.

.. cfunction:: int PyAcquire_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Acquire` object, or
    a subclass thereof.

.. cfunction:: int PyAcquire_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Acquire` object and no
    subclass thereof.

.. cfunction:: PyObject* PyAcquire_FromCpp(pkgAcquire *acquire, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Acquire` object from the :ctype:`pkgAcquire`
    pointer given by the parameter *acquire*. If the parameter *delete* is
    true, the object pointed to by *acquire* will be deleted when the refcount
    of the return value reaches 0.

.. cfunction:: pkgAcquire* PyAcquire_ToCpp(PyObject *acquire)

    Return the :ctype:`pkgAcquire` pointer contained in the Python object
    *acquire*.


AcquireFile (pkgAcqFile)
------------------------
.. cvar:: PyTypeObject PyAcquireFile_Type

    The type object for :class:`apt_pkg.AcquireFile` objects.

.. cfunction:: int PyAcquireFile_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.AcquireFile` object, or
    a subclass thereof.

.. cfunction:: int PyAcquireFile_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.AcquireFile` object
    and no subclass thereof.

.. cfunction:: PyObject* PyAcquireFile_FromCpp(pkgAcqFile *file, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.AcquireFile` object from the :ctype:`pkgAcqFile`
    pointer given by the parameter *file*. If the parameter *delete* is
    true, the object pointed to by *file* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* should point
    to a :class:`apt_pkg.Acquire` object.

.. cfunction:: pkgAcqFile* PyAcquireFile_ToCpp(PyObject *acquire)

    Return the :ctype:`pkgAcqFile` pointer contained in the Python object
    *acquire*.

AcquireItem (pkgAcquire::Item)
------------------------------
.. cvar:: PyTypeObject PyAcquireItem_Type

    The type object for :class:`apt_pkg.AcquireItem` objects.

.. cfunction:: int PyAcquireItem_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.AcquireItem` object, or
    a subclass thereof.

.. cfunction:: int PyAcquireItem_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.AcquireItem` object
    and no subclass thereof.

.. cfunction:: PyObject* PyAcquireItem_FromCpp(pkgAcquire::Item *item, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.AcquireItem` object from the :ctype:`pkgAcquire::Item`
    pointer given by the parameter *item*. If the parameter *delete* is
    true, the object pointed to by *item* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* should point
    to a :class:`apt_pkg.Acquire` object.

.. cfunction:: pkgAcquire::Item* PyAcquireItem_ToCpp(PyObject *object)

    Return the :ctype:`pkgAcquire::Item` pointer contained in the Python object
    *object*.

AcquireItemDesc (pkgAcquire::ItemDesc)
--------------------------------------
.. cvar:: PyTypeObject PyAcquireItemDesc_Type

    The type object for :class:`apt_pkg.AcquireItemDesc` objects.

.. cfunction:: int PyAcquireItemDesc_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.AcquireItemDesc` object, or
    a subclass thereof.

.. cfunction:: int PyAcquireItemDesc_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.AcquireItemDesc` object
    and no subclass thereof.

.. cfunction:: PyObject* PyAcquireItemDesc_FromCpp(pkgAcquire::ItemDesc *desc, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.AcquireItemDesc` object from the :ctype:`pkgAcquire::ItemDesc`
    pointer given by the parameter *desc*. If the parameter *delete* is
    true, the object pointed to by *desc* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* should point
    to a :class:`apt_pkg.AcquireItem` object.

.. cfunction:: pkgAcquire::ItemDesc* PyAcquireItemDesc_ToCpp(PyObject *object)

    Return the :ctype:`pkgAcquire::ItemDesc` pointer contained in the Python object
    *object*.

AcquireWorker (pkgAcquire::Worker)
----------------------------------
.. cvar:: PyTypeObject PyAcquireWorker_Type

    The type object for :class:`apt_pkg.AcquireWorker` objects.

.. cfunction:: int PyAcquireWorker_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.AcquireWorker` object, or
    a subclass thereof.

.. cfunction:: int PyAcquireWorker_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.AcquireWorker` object
    and no subclass thereof.

.. cfunction:: PyObject* PyAcquireWorker_FromCpp(pkgAcquire::Worker *worker, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.AcquireWorker` object from the :ctype:`pkgAcquire::Worker`
    pointer given by the parameter *worker*. If the parameter *delete* is
    true, the object pointed to by *worker* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* should point
    to a :class:`apt_pkg.Acquire` object.

.. cfunction:: pkgAcquire::Worker* PyAcquireWorker_ToCpp(PyObject *object)

    Return the :ctype:`pkgAcquire::Worker` pointer contained in the Python object
    *object*.

ActionGroup (pkgDepCache::ActionGroup)
--------------------------------------
.. cvar:: PyTypeObject PyActionGroup_Type

    The type object for :class:`apt_pkg.ActionGroup` objects.

.. cfunction:: int PyActionGroup_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.ActionGroup` object, or
    a subclass thereof.

.. cfunction:: int PyActionGroup_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.ActionGroup` object
    and no subclass thereof.

.. cfunction:: PyObject* PyActionGroup_FromCpp(pkgDepCache::ActionGroup *agroup, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.ActionGroup` object from the :ctype:`pkgDepCache::ActionGroup`
    pointer given by the parameter *agroup*. If the parameter *delete* is
    true, the object pointed to by *agroup* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* should point
    to a :class:`apt_pkg.DepCache` object.

.. cfunction:: pkgDepCache::ActionGroup* PyActionGroup_ToCpp(PyObject *object)

    Return the :ctype:`pkgDepCache::ActionGroup` pointer contained in the
    Python object *object*.

Cache (pkgCache)
------------------------
.. cvar:: PyTypeObject PyCache_Type

    The type object for :class:`apt_pkg.Cache` objects.

.. cfunction:: int PyCache_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Cache` object, or
    a subclass thereof.

.. cfunction:: int PyCache_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Cache` object
    and no subclass thereof.

.. cfunction:: PyObject* PyCache_FromCpp(pkgCache *cache, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Cache` object from the :ctype:`pkgCache`
    pointer given by the parameter *cache*. If the parameter *delete* is
    true, the object pointed to by *cache* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* shall point
    to a object of the type :cdata:`PyCacheFile_Type`.

.. cfunction:: pkgCache* PyCache_ToCpp(PyObject *object)

    Return the :ctype:`pkgCache` pointer contained in the Python object
    *object*.


CacheFile (pkgCacheFile)
------------------------
.. cvar:: PyTypeObject PyCacheFile_Type

    The type object for CacheFile. This type is internal and not exported to
    Python anywhere.

.. cfunction:: int PyCacheFile_Check(PyObject *object)

    Check that the object *object* is of the type :cdata:`PyCacheFile_Type` or
    a subclass thereof.

.. cfunction:: int PyCacheFile_CheckExact(PyObject *object)

    Check that the object *object* is of the type :cdata:`PyCacheFile_Type` and
    no subclass thereof.

.. cfunction:: PyObject* PyCacheFile_FromCpp(pkgCacheFile *file, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.CacheFile` object from the :ctype:`pkgCacheFile`
    pointer given by the parameter *file* If the parameter *delete* is
    true, the object pointed to by *file* will be deleted when the reference
    count of the returned object reaches 0.

.. cfunction:: pkgCacheFile* PyCacheFile_ToCpp(PyObject *object)

    Return the :ctype:`pkgCacheFile` pointer contained in the Python object
    *object*.

Cdrom (pkgCdrom)
------------------------
.. cvar:: PyTypeObject PyCdrom_Type

    The type object for :class:`apt_pkg.Cdrom` objects.

.. cfunction:: int PyCdrom_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Cdrom` object, or
    a subclass thereof.

.. cfunction:: int PyCdrom_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Cdrom` object
    and no subclass thereof.

.. cfunction:: PyObject* PyCdrom_FromCpp(pkgCdrom &cdrom, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Cdrom` object from the :ctype:`pkgCdrom`
    reference given by the parameter *cdrom*. If the parameter *delete* is
    true, *cdrom* will be deleted when the reference count of the returned
    object reaches 0.

.. cfunction:: pkgCdrom& PyCdrom_ToCpp(PyObject *object)

    Return the :ctype:`pkgCdrom` reference contained in the Python object
    *object*.

Configuration (Configuration)
-------------------------------
.. cvar:: PyTypeObject PyConfiguration_Type

    The type object for :class:`apt_pkg.Configuration` objects.

.. cfunction:: int PyConfiguration_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Configuration` object, or
    a subclass thereof.

.. cfunction:: int PyConfiguration_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Configuration` object
    and no subclass thereof.

.. cfunction:: PyObject* PyConfiguration_FromCpp(Configuration *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Configuration` object from the :ctype:`Configuration`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* may refer to
    a parent object (e.g. when exposing a sub tree of a configuration object).

.. cfunction:: Configuration* PyConfiguration_ToCpp(PyObject *object)

    Return the :ctype:`Configuration` pointer contained in the Python object
    *object*.

DepCache (pkgDepCache)
------------------------
.. cvar:: PyTypeObject PyDepCache_Type

    The type object for :class:`apt_pkg.DepCache` objects.

.. cfunction:: int PyDepCache_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.DepCache` object, or
    a subclass thereof.

.. cfunction:: int PyDepCache_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.DepCache` object
    and no subclass thereof.

.. cfunction:: PyObject* PyDepCache_FromCpp(pkgDepCache *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.DepCache` object from the :ctype:`pkgDepCache`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* must be
    a PyObject of the type :cdata:`PyCache_Type`.

.. cfunction:: pkgDepCache* PyDepCache_ToCpp(PyObject *object)

    Return the :ctype:`pkgDepCache` pointer contained in the Python object
    *object*.

Dependency (pkgCache::DepIterator)
----------------------------------
.. cvar:: PyTypeObject PyDependency_Type

    The type object for :class:`apt_pkg.Dependency` objects.

.. cfunction:: int PyDependency_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Dependency` object, or
    a subclass thereof.

.. cfunction:: int PyDependency_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Dependency` object
    and no subclass thereof.

.. cfunction:: PyObject* PyDependency_FromCpp(pkgCache::DepIterator &cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Dependency` object from the :ctype:`pkgCache::DepIterator`
    reference given by the parameter *cpp*. If the parameter *delete* is
    true, *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* must be
    a PyObject of the type :cdata:`PyPackage_Type`.

.. cfunction:: pkgCache::DepIterator& PyDependency_ToCpp(PyObject *object)

    Return the :ctype:`pkgCache::DepIterator` reference contained in the
    Python object *object*.

Description (pkgCache::DescIterator)
------------------------------------
.. cvar:: PyTypeObject PyDescription_Type

    The type object for :class:`apt_pkg.Description` objects.

.. cfunction:: int PyDescription_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Description` object, or
    a subclass thereof.

.. cfunction:: int PyDescription_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Description` object
    and no subclass thereof.

.. cfunction:: PyObject* PyDescription_FromCpp(pkgCache::DescIterator &cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Description` object from the :ctype:`pkgCache::DescIterator`
    reference given by the parameter *cpp*. If the parameter *delete* is
    true, *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* must be
    a PyObject of the type :cdata:`PyPackage_Type`.

.. cfunction:: pkgCache::DescIterator& PyDescription_ToCpp(PyObject *object)

    Return the :ctype:`pkgCache::DescIterator` reference contained in the
    Python object *object*.

Hashes (Hashes)
----------------------------------
.. cvar:: PyTypeObject PyHashes_Type

    The type object for :class:`apt_pkg.Hashes` objects.

.. cfunction:: int PyHashes_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Hashes` object, or
    a subclass thereof.

.. cfunction:: int PyHashes_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Hashes` object
    and no subclass thereof.

.. cfunction:: PyObject* PyHashes_FromCpp(Hashes &cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Hashes` object from the :ctype:`Hashes`
    reference given by the parameter *cpp*. If the parameter *delete* is
    true, *cpp* will be deleted when the reference count of the returned
    object reaches 0.

.. cfunction:: Hashes& PyHashes_ToCpp(PyObject *object)

    Return the :ctype:`Hashes` reference contained in the
    Python object *object*.

HashString (HashString)
------------------------
.. cvar:: PyTypeObject PyHashString_Type

    The type object for :class:`apt_pkg.HashString` objects.

.. cfunction:: int PyHashString_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.HashString` object, or
    a subclass thereof.

.. cfunction:: int PyHashString_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.HashString` object
    and no subclass thereof.

.. cfunction:: PyObject* PyHashString_FromCpp(HashString *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.HashString` object from the :ctype:`HashString`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0.

.. cfunction:: HashString* PyHashString_ToCpp(PyObject *object)

    Return the :ctype:`HashString` pointer contained in the Python object
    *object*.

IndexRecords (indexRecords)
----------------------------
.. cvar:: PyTypeObject PyIndexRecords_Type

    The type object for :class:`apt_pkg.IndexRecords` objects.

.. cfunction:: int PyIndexRecords_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.IndexRecords` object, or
    a subclass thereof.

.. cfunction:: int PyIndexRecords_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.IndexRecords` object
    and no subclass thereof.

.. cfunction:: PyObject* PyIndexRecords_FromCpp(indexRecords *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.IndexRecords` object from the :ctype:`indexRecords`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0.

.. cfunction:: indexRecords* PyIndexRecords_ToCpp(PyObject *object)

    Return the :ctype:`indexRecords` pointer contained in the Python object
    *object*.


MetaIndex (metaIndex)
------------------------
.. cvar:: PyTypeObject PyMetaIndex_Type

    The type object for :class:`apt_pkg.MetaIndex` objects.

.. cfunction:: int PyMetaIndex_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.MetaIndex` object, or
    a subclass thereof.

.. cfunction:: int PyMetaIndex_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.MetaIndex` object
    and no subclass thereof.

.. cfunction:: PyObject* PyMetaIndex_FromCpp(metaIndex *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.MetaIndex` object from the :ctype:`metaIndex`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* should be
    a PyObject of the type :cdata:`PySourceList_Type`.

.. cfunction:: metaIndex* PyMetaIndex_ToCpp(PyObject *object)

    Return the :ctype:`metaIndex` pointer contained in the Python object
    *object*.

Package (pkgCache::PkgIterator)
----------------------------------
.. cvar:: PyTypeObject PyPackage_Type

    The type object for :class:`apt_pkg.Package` objects.

.. cfunction:: int PyPackage_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Package` object, or
    a subclass thereof.

.. cfunction:: int PyPackage_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Package` object
    and no subclass thereof.

.. cfunction:: PyObject* PyPackage_FromCpp(pkgCache::PkgIterator &cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Package` object from the :ctype:`pkgCache::PkgIterator`
    reference given by the parameter *cpp*. If the parameter *delete* is
    true, *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* should be
    a PyObject of the type :cdata:`PyCache_Type`.

.. cfunction:: pkgCache::PkgIterator& PyPackage_ToCpp(PyObject *object)

    Return the :ctype:`pkgCache::PkgIterator` reference contained in the
    Python object *object*.

PackageFile (pkgCache::PkgFileIterator)
----------------------------------------
.. cvar:: PyTypeObject PyPackageFile_Type

    The type object for :class:`apt_pkg.PackageFile` objects.

.. cfunction:: int PyPackageFile_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.PackageFile` object, or
    a subclass thereof.

.. cfunction:: int PyPackageFile_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.PackageFile` object
    and no subclass thereof.

.. cfunction:: PyObject* PyPackageFile_FromCpp(pkgCache::PkgFileIterator &cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.PackageFile` object from the :ctype:`pkgCache::PkgFileIterator`
    reference given by the parameter *cpp*. If the parameter *delete* is
    true, *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* should be
    a PyObject of the type :cdata:`PyCache_Type`.

.. cfunction:: pkgCache::PkgFileIterator& PyPackageFile_ToCpp(PyObject *object)

    Return the :ctype:`pkgCache::PkgFileIterator` reference contained in the
    Python object *object*.

IndexFile (pkgIndexFile)
--------------------------------------
.. cvar:: PyTypeObject PyIndexFile_Type

    The type object for :class:`apt_pkg.IndexFile` objects.

.. cfunction:: int PyIndexFile_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.IndexFile` object, or
    a subclass thereof.

.. cfunction:: int PyIndexFile_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.IndexFile` object
    and no subclass thereof.

.. cfunction:: PyObject* PyIndexFile_FromCpp(pkgIndexFile *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.IndexFile` object from the :ctype:`pkgIndexFile`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* should be
    a PyObject of the type :cdata:`PyMetaIndex_Type`.

.. cfunction:: pkgIndexFile* PyIndexFile_ToCpp(PyObject *object)

    Return the :ctype:`pkgIndexFile` pointer contained in the Python object
    *object*.


PackageManager (pkgPackageManager)
----------------------------------
.. cvar:: PyTypeObject PyPackageManager_Type

    The type object for :class:`apt_pkg.PackageManager` objects.

.. cfunction:: int PyPackageManager_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.PackageManager` object, or
    a subclass thereof.

.. cfunction:: int PyPackageManager_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.PackageManager` object
    and no subclass thereof.

.. cfunction:: PyObject* PyPackageManager_FromCpp(pkgPackageManager *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.PackageManager` object from the :ctype:`pkgPackageManager`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0.

.. cfunction:: pkgPackageManager* PyPackageManager_ToCpp(PyObject *object)

    Return the :ctype:`pkgPackageManager` pointer contained in the Python object
    *object*.


Policy (pkgPolicy)
------------------
.. cvar:: PyTypeObject PyPolicy_Type

    The type object for :class:`apt_pkg.Policy` objects.

.. cfunction:: int PyPolicy_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Policy` object, or
    a subclass thereof.

.. cfunction:: int PyPolicy_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Policy` object
    and no subclass thereof.

.. cfunction:: PyObject* PyPolicy_FromCpp(pkgPolicy *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Policy` object from the :ctype:`pkgPolicy`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* must be
    a PyObject of the type :cdata:`PyCache_Type`.

.. cfunction:: pkgPolicy* PyPolicy_ToCpp(PyObject *object)

    Return the :ctype:`pkgPolicy` pointer contained in the Python object
    *object*.


ProblemResolver (pkgProblemResolver)
--------------------------------------
.. cvar:: PyTypeObject PyProblemResolver_Type

    The type object for :class:`apt_pkg.ProblemResolver` objects.

.. cfunction:: int PyProblemResolver_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.ProblemResolver` object, or
    a subclass thereof.

.. cfunction:: int PyProblemResolver_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.ProblemResolver` object
    and no subclass thereof.

.. cfunction:: PyObject* PyProblemResolver_FromCpp(pkgProblemResolver *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.ProblemResolver` object from the :ctype:`pkgProblemResolver`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* must be
    a PyObject of the type :cdata:`PyDepCache_Type`.

.. cfunction:: pkgProblemResolver* PyProblemResolver_ToCpp(PyObject *object)

    Return the :ctype:`pkgProblemResolver` pointer contained in the Python object
    *object*.



SourceList (pkgSourceList)
---------------------------
.. cvar:: PyTypeObject PySourceList_Type

    The type object for :class:`apt_pkg.SourceList` objects.

.. cfunction:: int PySourceList_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.SourceList` object, or
    a subclass thereof.

.. cfunction:: int PySourceList_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.SourceList` object
    and no subclass thereof.

.. cfunction:: PyObject* PySourceList_FromCpp(pkgSourceList *cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.SourceList` object from the :ctype:`pkgSourceList`
    pointer given by the parameter *cpp*. If the parameter *delete* is
    true, the object pointed to by *cpp* will be deleted when the reference
    count of the returned object reaches 0.

.. cfunction:: pkgSourceList* PySourceList_ToCpp(PyObject *object)

    Return the :ctype:`pkgSourceList` pointer contained in the Python object
    *object*.


TagFile (pkgTagFile)
----------------------------------
.. cvar:: PyTypeObject PyTagFile_Type

    The type object for :class:`apt_pkg.TagFile` objects.

.. cfunction:: int PyTagFile_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.TagFile` object, or
    a subclass thereof.

.. cfunction:: int PyTagFile_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.TagFile` object
    and no subclass thereof.

.. cfunction:: PyObject* PyTagFile_FromCpp(pkgTagFile &cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.TagFile` object from the :ctype:`pkgTagFile`
    reference given by the parameter *cpp*. If the parameter *delete* is
    true, *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* may be any
    Python object.

.. cfunction:: pkgTagFile& PyTagFile_ToCpp(PyObject *object)

    Return the :ctype:`pkgTagFile` reference contained in the
    Python object *object*.

TagSection (pkgTagSection)
----------------------------------
.. cvar:: PyTypeObject PyTagSection_Type

    The type object for :class:`apt_pkg.TagSection` objects.

.. cfunction:: int PyTagSection_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.TagSection` object, or
    a subclass thereof.

.. cfunction:: int PyTagSection_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.TagSection` object
    and no subclass thereof.

.. cfunction:: PyObject* PyTagSection_FromCpp(pkgTagSection &cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.TagSection` object from the :ctype:`pkgTagSection`
    reference given by the parameter *cpp*. If the parameter *delete* is
    true, *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* may be
    a PyObject of the type :cdata:`PyTagFile_Type`.

.. cfunction:: pkgTagSection& PyTagSection_ToCpp(PyObject *object)

    Return the :ctype:`pkgTagSection` reference contained in the
    Python object *object*.

Version (pkgCache::VerIterator)
----------------------------------
.. cvar:: PyTypeObject PyVersion_Type

    The type object for :class:`apt_pkg.Version` objects.

.. cfunction:: int PyVersion_Check(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Version` object, or
    a subclass thereof.

.. cfunction:: int PyVersion_CheckExact(PyObject *object)

    Check that the object *object* is an :class:`apt_pkg.Version` object
    and no subclass thereof.

.. cfunction:: PyObject* PyVersion_FromCpp(pkgCache::VerIterator &cpp, bool delete, PyObject *owner)

    Create a new :class:`apt_pkg.Version` object from the :ctype:`pkgCache::VerIterator`
    reference given by the parameter *cpp*. If the parameter *delete* is
    true, *cpp* will be deleted when the reference
    count of the returned object reaches 0. The parameter *owner* must be
    a PyObject of the type :cdata:`PyPackage_Type`.

.. cfunction:: pkgCache::VerIterator& PyVersion_ToCpp(PyObject *object)

    Return the :ctype:`pkgCache::VerIterator` reference contained in the
    Python object *object*.
