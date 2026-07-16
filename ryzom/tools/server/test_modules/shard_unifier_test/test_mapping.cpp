
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "test_mapping.h"

namespace TM
{

	void CRootTablePtr::linkPtr()
	{
		nlassert(_NextPtr == NULL);
		nlassert(_PrevPtr == NULL);
		if (_Ptr != NULL)
		{
			_NextPtr = _Ptr->getFirstPtr();
			if (_NextPtr != NULL)
			{
				_PrevPtr = _NextPtr->_PrevPtr;
				_PrevPtr->_NextPtr = this;
				_NextPtr->_PrevPtr = this;
			}
			else
			{
				_NextPtr = this;
				_PrevPtr = this;
				_Ptr->setFirstPtr(this);
			}
		}
	}

	void CRootTablePtr::unlinkPtr()
	{
		if (_NextPtr == NULL)
		{
			nlassert(_PrevPtr == NULL);
			return;
		}

		if (_Ptr != NULL)
		{
			if (_NextPtr == this)
			{
				nlassert(_PrevPtr == this);
				// last pointer !
				_Ptr->setFirstPtr(NULL);
			}
			else
			{
				if (_Ptr->getFirstPtr() == this)
				{
					// the first ptr is the current one, we need to switch to next one
					_Ptr->setFirstPtr(_NextPtr);
				}
			}

		}
		if (_NextPtr != this)
		{
			nlassert(_PrevPtr != this);

			_NextPtr->_PrevPtr = _PrevPtr;
			_PrevPtr->_NextPtr = _NextPtr;
		}
		_NextPtr = NULL;
		_PrevPtr = NULL;
	}


	CRootTable::TObjectCache		CRootTable::_ObjectCache;
	CRootTable::TReleasedObject	CRootTable::_ReleasedObject;


	// Destructor, delete any children
	CRootTable::~CRootTable()
	{
		// release childs reference
			if (_MapChilds != NULL)
						delete _MapChilds;
			if (_VectorChilds != NULL)
						delete _VectorChilds;


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CRootTablePtr *ptr = _PtrList;
			do 
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_Id != NOPE::INVALID_OBJECT_ID 
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CRootTable @%p from cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_Id) == _ObjectCache.end());
		}
		if (_ObjectState == NOPE::os_released)
		{
			removeFromReleased();
		}
		else
		{
			TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
			if (it != _ReleasedObject.end())
			{
				nlassert(it->second.find(this) == it->second.end());
			}
		}
	}

	void CRootTable::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CRootTable::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO root_table (";
		
		qs += "";
		qs += ") VALUES (";
		
		qs += ")";

		if (connection.query(qs))
		{
			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			return true;
		}

		return false;
	}

	bool CRootTable::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE root_table SET ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				setPersistentState(NOPE::os_clean);
				return true;
			}
		}

		return false;
	}

	bool CRootTable::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM root_table ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{

				{
					// cascading deletion for map child MapChilds
					nlassert(loadMapChilds(connection, __FILE__, __LINE__));

					const std::map < uint32, CMapChildPtr > & childs = getMapChilds();

					while (!childs.empty())
					{
						getMapChildsById(childs.begin()->first)->remove(connection);
					}
				}

				{
					// cascading deletion for single child OneChild
					nlassert(loadOneChild(connection, __FILE__, __LINE__));
					
					if (getOneChild() != NULL)
						getOneChild()->remove(connection);
				}

				{
					// unreference (and update) for vector child VectorChilds
					nlassert(loadVectorChilds(connection, __FILE__, __LINE__));

					const std::vector < CVectorChildPtr > & childs = getVectorChilds();

					for (uint i=0; i < childs.size(); ++i)
					{
						
						getVectorChildsByIndex(i)->setParentId(0);
						getVectorChildsByIndex(i)->update(connection);
					}
				}


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CRootTable::removeById(MSW::CConnection &connection, uint32 id)
	{
		CRootTable *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM root_table ";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				// ok, the row is removed
				return true;
			}
		}

		return false;
	}


	// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
	CRootTable *CRootTable::loadFromCache(uint32 objectId, bool unrelease)
	{
		// look in the cache
		TObjectCache::iterator it(_ObjectCache.find(objectId));
		if (it == _ObjectCache.end())
		{
			// not found, return null
			return NULL;
		}
		else
		{
			CRootTable *object = it->second;

			if (object->_ObjectState == NOPE::os_released)
			{
				if (unrelease)
				{
					// we need to remove this object from the released object set.
					object->removeFromReleased();
					object->_ObjectState = NOPE::os_clean;
				}
			}

			return it->second;
		}
	}
	// Receive and execute command from the cache manager.
	uint32 CRootTable::cacheCmd(NOPE::TCacheCmd cmd)
	{
		if (cmd == NOPE::cc_update)
		{
			updateCache();
		}
		else if (cmd == NOPE::cc_clear)
		{
			clearCache();
		}
		else if (cmd == NOPE::cc_dump)
		{
			dump();
		}
		else if (cmd == NOPE::cc_instance_count)
		{
			return _ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CRootTable::dump()
	{
		nlinfo("  Cache info for class CRootTable :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CRootTable::updateCache()
	{
		if (_ReleasedObject.empty())
			return;

		// 30 s hold in cache
		const time_t MAX_CACHE_OLD_TIME = 30;

		time_t now = NLMISC::CTime::getSecondsSince1970();

		// look for object set older than MAX_CACHE_OLD_TIME and delete them
		while (!_ReleasedObject.empty() && _ReleasedObject.begin()->first < now-MAX_CACHE_OLD_TIME)
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CRootTable *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CRootTable::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CRootTable *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CRootTable::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CRootTable::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CRootTable::setFirstPtr(CRootTablePtr *ptr)
	{
		_PtrList = ptr;

		if (ptr == NULL)
		{
			// this is the last pointer !
			if (_ObjectState == NOPE::os_transient
				|| _ObjectState == NOPE::os_removed)
			{
				// not a persistent object, or removed object, just delet it
				delete this;
			}
			else if (_ObjectState != NOPE::os_removed)
			{
				setPersistentState(NOPE::os_released);
			}
		}
	}

	// Set the persistent state of the object and do some house keeping
	void CRootTable::setPersistentState(NOPE::TObjectState state)
	{
		nlassert(NOPE::AllowedTransition[_ObjectState][state] == true);

		if(_ObjectState == NOPE::os_released && state == NOPE::os_removed)
		{
			// a release object gets removed (e.g. by remove by id)
		
			// delete the object
			delete this;

			// no more to do
			return;
		}

		if (_ObjectState == NOPE::os_transient && state != NOPE::os_transient)
		{
			nldebug("NOPE: inserting CRootTable @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.insert(std::make_pair(_Id, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_Id) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CRootTable @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CRootTablePtr CRootTable::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CRootTable *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CRootTablePtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id";

		qs += " FROM root_table";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CRootTablePtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CRootTable, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CRootTable::loadMapChilds(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_MapChilds != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_MapChilds = new std::map < uint32,  CMapChildPtr >;
		
		// load the childs
		ret &= CMapChild::loadChildrenOfCRootTable(connection, getObjectId(), *_MapChilds, filename, lineNum);
		return ret;
	}


	const std::map<uint32, CMapChildPtr> &CRootTable::getMapChilds() const
	{
		nlassert(_MapChilds != NULL);
		return *_MapChilds;
	}

	CMapChildPtr &CRootTable::getMapChildsById(uint32 id) const
	{
		nlassert(_MapChilds != NULL);
		std::map<uint32, CMapChildPtr>::const_iterator it(_MapChilds->find(id));

		if (it == _MapChilds->end())
		{
			// no object with this id, return a null pointer
			static CMapChildPtr nil;
			return nil;
		}

		return const_cast< CMapChildPtr & >(it->second);
	}

	
	bool CRootTable::loadVectorChilds(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_VectorChilds != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_VectorChilds = new std::vector < CVectorChildPtr >;
		
		// load the childs
		ret &= CVectorChild::loadChildrenOfCRootTable(connection, getObjectId(), *_VectorChilds, filename, lineNum);
		return ret;
	}


	const std::vector<CVectorChildPtr> &CRootTable::getVectorChilds() const
	{
		nlassert(_VectorChilds != NULL);
		return *_VectorChilds;
	}

	CVectorChildPtr &CRootTable::getVectorChildsByIndex(uint32 index) const
	{
		nlassert(_VectorChilds != NULL);
		nlassert(index < _VectorChilds->size());
		return const_cast< CVectorChildPtr & >(_VectorChilds->operator[](index));
	}
	
	CVectorChildPtr &CRootTable::getVectorChildsById(uint32 id) const
	{
		nlassert(_VectorChilds != NULL);
		std::vector<CVectorChildPtr >::const_iterator first(_VectorChilds->begin()), last(_VectorChilds->end());
		for (; first != last; ++first)
		{
			const CVectorChildPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CVectorChildPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CVectorChildPtr nil;

		return nil;
	}

	
	bool CRootTable::loadOneChild(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		if (_OneChildLoaded)
		{
			// the child is already loaded, just return true
			return true;
		}
 		bool ret = COneChild::loadChildOfCRootTable(connection, getObjectId(), _OneChild, filename, lineNum);
		_OneChildLoaded = true;
		return ret;
	}

	/** Return the one child object (or null if not) */
	COneChildPtr CRootTable::getOneChild()
	{
		nlassert(_OneChildLoaded);
		return _OneChild;
	}


	void COneChildPtr::linkPtr()
	{
		nlassert(_NextPtr == NULL);
		nlassert(_PrevPtr == NULL);
		if (_Ptr != NULL)
		{
			_NextPtr = _Ptr->getFirstPtr();
			if (_NextPtr != NULL)
			{
				_PrevPtr = _NextPtr->_PrevPtr;
				_PrevPtr->_NextPtr = this;
				_NextPtr->_PrevPtr = this;
			}
			else
			{
				_NextPtr = this;
				_PrevPtr = this;
				_Ptr->setFirstPtr(this);
			}
		}
	}

	void COneChildPtr::unlinkPtr()
	{
		if (_NextPtr == NULL)
		{
			nlassert(_PrevPtr == NULL);
			return;
		}

		if (_Ptr != NULL)
		{
			if (_NextPtr == this)
			{
				nlassert(_PrevPtr == this);
				// last pointer !
				_Ptr->setFirstPtr(NULL);
			}
			else
			{
				if (_Ptr->getFirstPtr() == this)
				{
					// the first ptr is the current one, we need to switch to next one
					_Ptr->setFirstPtr(_NextPtr);
				}
			}

		}
		if (_NextPtr != this)
		{
			nlassert(_PrevPtr != this);

			_NextPtr->_PrevPtr = _PrevPtr;
			_PrevPtr->_NextPtr = _NextPtr;
		}
		_NextPtr = NULL;
		_PrevPtr = NULL;
	}


	COneChild::TObjectCache		COneChild::_ObjectCache;
	COneChild::TReleasedObject	COneChild::_ReleasedObject;


	// Destructor, delete any children
	COneChild::~COneChild()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			COneChildPtr *ptr = _PtrList;
			do 
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_Id != NOPE::INVALID_OBJECT_ID 
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing COneChild @%p from cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_Id) == _ObjectCache.end());
		}
		if (_ObjectState == NOPE::os_released)
		{
			removeFromReleased();
		}
		else
		{
			TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
			if (it != _ReleasedObject.end())
			{
				nlassert(it->second.find(this) == it->second.end());
			}
		}
	}

	void COneChild::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool COneChild::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_Id != 0);
		std::string qs;
		qs = "INSERT INTO one_child (";
		
		qs += "Id";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_Id), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_Id != 0)
			{
				// need to update the parent class child if it is in the cache
				CRootTable *parent = CRootTable::loadFromCache(_Id, false);
				if (parent && parent->_OneChildLoaded)
				{
					nlassert(parent->_OneChild == NULL);
					parent->_OneChild = COneChildPtr(this, __FILE__, __LINE__);
				}
			}

			return true;
		}

		return false;
	}

	bool COneChild::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE one_child SET ";
		
		qs += "Id = '"+MSW::escapeString(NLMISC::toString(_Id), connection)+"'";

		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				setPersistentState(NOPE::os_clean);
				return true;
			}
		}

		return false;
	}

	bool COneChild::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM one_child ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				// need to remove ref from parent (if any)
				
				{
					CRootTablePtr parent(CRootTable::loadFromCache(_Id, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_OneChildLoaded)
					{
						// assign a new NULL pointer
						parent->_OneChild.assign(COneChildPtr(), __FILE__, __LINE__);
					}
				}
				

				return true;
			}
		}
		return false;
	}

	bool COneChild::removeById(MSW::CConnection &connection, uint32 id)
	{
		COneChild *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM one_child ";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				// ok, the row is removed
				return true;
			}
		}

		return false;
	}


	// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
	COneChild *COneChild::loadFromCache(uint32 objectId, bool unrelease)
	{
		// look in the cache
		TObjectCache::iterator it(_ObjectCache.find(objectId));
		if (it == _ObjectCache.end())
		{
			// not found, return null
			return NULL;
		}
		else
		{
			COneChild *object = it->second;

			if (object->_ObjectState == NOPE::os_released)
			{
				if (unrelease)
				{
					// we need to remove this object from the released object set.
					object->removeFromReleased();
					object->_ObjectState = NOPE::os_clean;
				}
			}

			return it->second;
		}
	}
	// Receive and execute command from the cache manager.
	uint32 COneChild::cacheCmd(NOPE::TCacheCmd cmd)
	{
		if (cmd == NOPE::cc_update)
		{
			updateCache();
		}
		else if (cmd == NOPE::cc_clear)
		{
			clearCache();
		}
		else if (cmd == NOPE::cc_dump)
		{
			dump();
		}
		else if (cmd == NOPE::cc_instance_count)
		{
			return _ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void COneChild::dump()
	{
		nlinfo("  Cache info for class COneChild :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void COneChild::updateCache()
	{
		if (_ReleasedObject.empty())
			return;

		// 30 s hold in cache
		const time_t MAX_CACHE_OLD_TIME = 30;

		time_t now = NLMISC::CTime::getSecondsSince1970();

		// look for object set older than MAX_CACHE_OLD_TIME and delete them
		while (!_ReleasedObject.empty() && _ReleasedObject.begin()->first < now-MAX_CACHE_OLD_TIME)
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				COneChild *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void COneChild::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				COneChild *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void COneChild::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&COneChild::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void COneChild::setFirstPtr(COneChildPtr *ptr)
	{
		_PtrList = ptr;

		if (ptr == NULL)
		{
			// this is the last pointer !
			if (_ObjectState == NOPE::os_transient
				|| _ObjectState == NOPE::os_removed)
			{
				// not a persistent object, or removed object, just delet it
				delete this;
			}
			else if (_ObjectState != NOPE::os_removed)
			{
				setPersistentState(NOPE::os_released);
			}
		}
	}

	// Set the persistent state of the object and do some house keeping
	void COneChild::setPersistentState(NOPE::TObjectState state)
	{
		nlassert(NOPE::AllowedTransition[_ObjectState][state] == true);

		if(_ObjectState == NOPE::os_released && state == NOPE::os_removed)
		{
			// a release object gets removed (e.g. by remove by id)
		
			// delete the object
			delete this;

			// no more to do
			return;
		}

		if (_ObjectState == NOPE::os_transient && state != NOPE::os_transient)
		{
			nldebug("NOPE: inserting COneChild @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.insert(std::make_pair(_Id, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_Id) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing COneChild @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	COneChildPtr COneChild::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		COneChild *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return COneChildPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id";

		qs += " FROM one_child";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	COneChildPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new COneChild, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}

	bool COneChild::loadChildOfCRootTable(MSW::CConnection &connection, uint32 parentId, COneChildPtr &childPtr, const char *filename, uint32 lineNum)
	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id";

		qs += " FROM one_child";
		qs += " WHERE Id = '"+NLMISC::toString(parentId)+"'";

		COneChildPtr ret;
		if (!connection.query(qs))
		{
			childPtr = COneChildPtr();
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		// check that the data description is consistent with database content
		nlassert(result->getNumRows() <= 1);

		if (result->getNumRows() == 1)
		{
			COneChild *object = new COneChild;
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, object->_Id);
					COneChild *inCache = loadFromCache(object->_Id, true);
			if (inCache != NULL)
			{
				ret.assign(inCache, filename, lineNum);
				// no more needed
				delete object;
			}
			else
			{
				object->setPersistentState(NOPE::os_clean);
				ret.assign(object, filename, lineNum);
			}

			childPtr = ret;
			return true;
		}
	
		// no result, but no error
		childPtr = COneChildPtr();
		return true;
	}

	void CMapChildPtr::linkPtr()
	{
		nlassert(_NextPtr == NULL);
		nlassert(_PrevPtr == NULL);
		if (_Ptr != NULL)
		{
			_NextPtr = _Ptr->getFirstPtr();
			if (_NextPtr != NULL)
			{
				_PrevPtr = _NextPtr->_PrevPtr;
				_PrevPtr->_NextPtr = this;
				_NextPtr->_PrevPtr = this;
			}
			else
			{
				_NextPtr = this;
				_PrevPtr = this;
				_Ptr->setFirstPtr(this);
			}
		}
	}

	void CMapChildPtr::unlinkPtr()
	{
		if (_NextPtr == NULL)
		{
			nlassert(_PrevPtr == NULL);
			return;
		}

		if (_Ptr != NULL)
		{
			if (_NextPtr == this)
			{
				nlassert(_PrevPtr == this);
				// last pointer !
				_Ptr->setFirstPtr(NULL);
			}
			else
			{
				if (_Ptr->getFirstPtr() == this)
				{
					// the first ptr is the current one, we need to switch to next one
					_Ptr->setFirstPtr(_NextPtr);
				}
			}

		}
		if (_NextPtr != this)
		{
			nlassert(_PrevPtr != this);

			_NextPtr->_PrevPtr = _PrevPtr;
			_PrevPtr->_NextPtr = _NextPtr;
		}
		_NextPtr = NULL;
		_PrevPtr = NULL;
	}


	CMapChild::TObjectCache		CMapChild::_ObjectCache;
	CMapChild::TReleasedObject	CMapChild::_ReleasedObject;


	// Destructor, delete any children
	CMapChild::~CMapChild()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CMapChildPtr *ptr = _PtrList;
			do 
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_Id != NOPE::INVALID_OBJECT_ID 
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CMapChild @%p from cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_Id) == _ObjectCache.end());
		}
		if (_ObjectState == NOPE::os_released)
		{
			removeFromReleased();
		}
		else
		{
			TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
			if (it != _ReleasedObject.end())
			{
				nlassert(it->second.find(this) == it->second.end());
			}
		}
	}

	void CMapChild::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CMapChild::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_Id != 0);
		std::string qs;
		qs = "INSERT INTO map_child (";
		
		qs += "Id, parent_id";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_Id), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_ParentId), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_ParentId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CRootTable *parent = CRootTable::loadFromCache(_ParentId, false);
				if (parent && parent->_MapChilds != NULL)
				{

						nlverify(parent->_MapChilds->insert(std::make_pair(getObjectId(), CMapChildPtr(this, __FILE__, __LINE__))).second);
 
				}
			}

			return true;
		}

		return false;
	}

	bool CMapChild::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE map_child SET ";
		
		qs += "Id = '"+MSW::escapeString(NLMISC::toString(_Id), connection)+"'";
		qs += ", ";
		qs += "parent_id = '"+MSW::escapeString(NLMISC::toString(_ParentId), connection)+"'";

		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				setPersistentState(NOPE::os_clean);
				return true;
			}
		}

		return false;
	}

	bool CMapChild::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM map_child ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CRootTablePtr parent(CRootTable::loadFromCache(_ParentId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_MapChilds != NULL)
					{

						parent->_MapChilds->erase(getObjectId());
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CMapChild::removeById(MSW::CConnection &connection, uint32 id)
	{
		CMapChild *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM map_child ";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				// ok, the row is removed
				return true;
			}
		}

		return false;
	}


	// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
	CMapChild *CMapChild::loadFromCache(uint32 objectId, bool unrelease)
	{
		// look in the cache
		TObjectCache::iterator it(_ObjectCache.find(objectId));
		if (it == _ObjectCache.end())
		{
			// not found, return null
			return NULL;
		}
		else
		{
			CMapChild *object = it->second;

			if (object->_ObjectState == NOPE::os_released)
			{
				if (unrelease)
				{
					// we need to remove this object from the released object set.
					object->removeFromReleased();
					object->_ObjectState = NOPE::os_clean;
				}
			}

			return it->second;
		}
	}
	// Receive and execute command from the cache manager.
	uint32 CMapChild::cacheCmd(NOPE::TCacheCmd cmd)
	{
		if (cmd == NOPE::cc_update)
		{
			updateCache();
		}
		else if (cmd == NOPE::cc_clear)
		{
			clearCache();
		}
		else if (cmd == NOPE::cc_dump)
		{
			dump();
		}
		else if (cmd == NOPE::cc_instance_count)
		{
			return _ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CMapChild::dump()
	{
		nlinfo("  Cache info for class CMapChild :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CMapChild::updateCache()
	{
		if (_ReleasedObject.empty())
			return;

		// 30 s hold in cache
		const time_t MAX_CACHE_OLD_TIME = 30;

		time_t now = NLMISC::CTime::getSecondsSince1970();

		// look for object set older than MAX_CACHE_OLD_TIME and delete them
		while (!_ReleasedObject.empty() && _ReleasedObject.begin()->first < now-MAX_CACHE_OLD_TIME)
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CMapChild *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CMapChild::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CMapChild *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CMapChild::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CMapChild::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CMapChild::setFirstPtr(CMapChildPtr *ptr)
	{
		_PtrList = ptr;

		if (ptr == NULL)
		{
			// this is the last pointer !
			if (_ObjectState == NOPE::os_transient
				|| _ObjectState == NOPE::os_removed)
			{
				// not a persistent object, or removed object, just delet it
				delete this;
			}
			else if (_ObjectState != NOPE::os_removed)
			{
				setPersistentState(NOPE::os_released);
			}
		}
	}

	// Set the persistent state of the object and do some house keeping
	void CMapChild::setPersistentState(NOPE::TObjectState state)
	{
		nlassert(NOPE::AllowedTransition[_ObjectState][state] == true);

		if(_ObjectState == NOPE::os_released && state == NOPE::os_removed)
		{
			// a release object gets removed (e.g. by remove by id)
		
			// delete the object
			delete this;

			// no more to do
			return;
		}

		if (_ObjectState == NOPE::os_transient && state != NOPE::os_transient)
		{
			nldebug("NOPE: inserting CMapChild @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.insert(std::make_pair(_Id, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_Id) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CMapChild @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CMapChildPtr CMapChild::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CMapChild *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CMapChildPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id, parent_id";

		qs += " FROM map_child";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CMapChildPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CMapChild, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getField(1, ret->_ParentId);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CMapChild::loadChildrenOfCRootTable(MSW::CConnection &connection, uint32 parentId, std::map < uint32, CMapChildPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, parent_id";

		qs += " FROM map_child";
		qs += " WHERE parent_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CMapChild *ret = new CMapChild();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_ParentId);
					CMapChild *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.insert(std::make_pair(inCache->getObjectId(), CMapChildPtr(inCache, filename, lineNum)));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.insert(std::make_pair(ret->getObjectId(), CMapChildPtr(ret, filename, lineNum)));

			}
		}

		return true;
	}

	void CVectorChildPtr::linkPtr()
	{
		nlassert(_NextPtr == NULL);
		nlassert(_PrevPtr == NULL);
		if (_Ptr != NULL)
		{
			_NextPtr = _Ptr->getFirstPtr();
			if (_NextPtr != NULL)
			{
				_PrevPtr = _NextPtr->_PrevPtr;
				_PrevPtr->_NextPtr = this;
				_NextPtr->_PrevPtr = this;
			}
			else
			{
				_NextPtr = this;
				_PrevPtr = this;
				_Ptr->setFirstPtr(this);
			}
		}
	}

	void CVectorChildPtr::unlinkPtr()
	{
		if (_NextPtr == NULL)
		{
			nlassert(_PrevPtr == NULL);
			return;
		}

		if (_Ptr != NULL)
		{
			if (_NextPtr == this)
			{
				nlassert(_PrevPtr == this);
				// last pointer !
				_Ptr->setFirstPtr(NULL);
			}
			else
			{
				if (_Ptr->getFirstPtr() == this)
				{
					// the first ptr is the current one, we need to switch to next one
					_Ptr->setFirstPtr(_NextPtr);
				}
			}

		}
		if (_NextPtr != this)
		{
			nlassert(_PrevPtr != this);

			_NextPtr->_PrevPtr = _PrevPtr;
			_PrevPtr->_NextPtr = _NextPtr;
		}
		_NextPtr = NULL;
		_PrevPtr = NULL;
	}


	CVectorChild::TObjectCache		CVectorChild::_ObjectCache;
	CVectorChild::TReleasedObject	CVectorChild::_ReleasedObject;


	// Destructor, delete any children
	CVectorChild::~CVectorChild()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CVectorChildPtr *ptr = _PtrList;
			do 
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_Id != NOPE::INVALID_OBJECT_ID 
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CVectorChild @%p from cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_Id) == _ObjectCache.end());
		}
		if (_ObjectState == NOPE::os_released)
		{
			removeFromReleased();
		}
		else
		{
			TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
			if (it != _ReleasedObject.end())
			{
				nlassert(it->second.find(this) == it->second.end());
			}
		}
	}

	void CVectorChild::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CVectorChild::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_Id != 0);
		std::string qs;
		qs = "INSERT INTO vector_child (";
		
		qs += "Id, parent_id, info";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_Id), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_ParentId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Info), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_ParentId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CRootTable *parent = CRootTable::loadFromCache(_ParentId, false);
				if (parent && parent->_VectorChilds != NULL)
				{

						nlassert(std::find(parent->_VectorChilds->begin(), parent->_VectorChilds->end(), CVectorChildPtr(this, __FILE__, __LINE__)) == parent->_VectorChilds->end());
						parent->_VectorChilds->push_back(CVectorChildPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CVectorChild::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE vector_child SET ";
		
		qs += "Id = '"+MSW::escapeString(NLMISC::toString(_Id), connection)+"'";
		qs += ", ";
		qs += "parent_id = '"+MSW::escapeString(NLMISC::toString(_ParentId), connection)+"'";
		qs += ", ";
		qs += "info = '"+MSW::escapeString(NLMISC::toString(_Info), connection)+"'";

		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				setPersistentState(NOPE::os_clean);
				return true;
			}
		}

		return false;
	}

	bool CVectorChild::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM vector_child ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CRootTablePtr parent(CRootTable::loadFromCache(_ParentId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_VectorChilds != NULL)
					{

						std::vector < CVectorChildPtr >::iterator it = std::find(parent->_VectorChilds->begin(), parent->_VectorChilds->end(), this);
						if (it != parent->_VectorChilds->end())
						{
							parent->_VectorChilds->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CVectorChild::removeById(MSW::CConnection &connection, uint32 id)
	{
		CVectorChild *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM vector_child ";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				// ok, the row is removed
				return true;
			}
		}

		return false;
	}


	// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
	CVectorChild *CVectorChild::loadFromCache(uint32 objectId, bool unrelease)
	{
		// look in the cache
		TObjectCache::iterator it(_ObjectCache.find(objectId));
		if (it == _ObjectCache.end())
		{
			// not found, return null
			return NULL;
		}
		else
		{
			CVectorChild *object = it->second;

			if (object->_ObjectState == NOPE::os_released)
			{
				if (unrelease)
				{
					// we need to remove this object from the released object set.
					object->removeFromReleased();
					object->_ObjectState = NOPE::os_clean;
				}
			}

			return it->second;
		}
	}
	// Receive and execute command from the cache manager.
	uint32 CVectorChild::cacheCmd(NOPE::TCacheCmd cmd)
	{
		if (cmd == NOPE::cc_update)
		{
			updateCache();
		}
		else if (cmd == NOPE::cc_clear)
		{
			clearCache();
		}
		else if (cmd == NOPE::cc_dump)
		{
			dump();
		}
		else if (cmd == NOPE::cc_instance_count)
		{
			return _ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CVectorChild::dump()
	{
		nlinfo("  Cache info for class CVectorChild :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CVectorChild::updateCache()
	{
		if (_ReleasedObject.empty())
			return;

		// 30 s hold in cache
		const time_t MAX_CACHE_OLD_TIME = 30;

		time_t now = NLMISC::CTime::getSecondsSince1970();

		// look for object set older than MAX_CACHE_OLD_TIME and delete them
		while (!_ReleasedObject.empty() && _ReleasedObject.begin()->first < now-MAX_CACHE_OLD_TIME)
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CVectorChild *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CVectorChild::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CVectorChild *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CVectorChild::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CVectorChild::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CVectorChild::setFirstPtr(CVectorChildPtr *ptr)
	{
		_PtrList = ptr;

		if (ptr == NULL)
		{
			// this is the last pointer !
			if (_ObjectState == NOPE::os_transient
				|| _ObjectState == NOPE::os_removed)
			{
				// not a persistent object, or removed object, just delet it
				delete this;
			}
			else if (_ObjectState != NOPE::os_removed)
			{
				setPersistentState(NOPE::os_released);
			}
		}
	}

	// Set the persistent state of the object and do some house keeping
	void CVectorChild::setPersistentState(NOPE::TObjectState state)
	{
		nlassert(NOPE::AllowedTransition[_ObjectState][state] == true);

		if(_ObjectState == NOPE::os_released && state == NOPE::os_removed)
		{
			// a release object gets removed (e.g. by remove by id)
		
			// delete the object
			delete this;

			// no more to do
			return;
		}

		if (_ObjectState == NOPE::os_transient && state != NOPE::os_transient)
		{
			nldebug("NOPE: inserting CVectorChild @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.insert(std::make_pair(_Id, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_Id) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CVectorChild @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CVectorChildPtr CVectorChild::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CVectorChild *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CVectorChildPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id, parent_id, info";

		qs += " FROM vector_child";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CVectorChildPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CVectorChild, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getField(1, ret->_ParentId);
			result->getField(2, ret->_Info);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CVectorChild::loadChildrenOfCRootTable(MSW::CConnection &connection, uint32 parentId, std::vector < CVectorChildPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, parent_id, info";

		qs += " FROM vector_child";
		qs += " WHERE parent_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CVectorChild *ret = new CVectorChild();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_ParentId);
					
			result->getField(2, ret->_Info);
					CVectorChild *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CVectorChildPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CVectorChildPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

}
