
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "stdpch.h"
	
#include "nel_database_mapping.h"

namespace RSMGR
{

	void CNelUserPtr::linkPtr()
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

	void CNelUserPtr::unlinkPtr()
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


	CNelUser::TObjectCache		CNelUser::_ObjectCache;
	CNelUser::TReleasedObject	CNelUser::_ReleasedObject;


	// Destructor, delete any children
	CNelUser::~CNelUser()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CNelUserPtr *ptr = _PtrList;
			do
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_UserId != NOPE::INVALID_OBJECT_ID
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CNelUser @%p from cache with id %u", this, static_cast<uint32>(_UserId));
			nlverify(_ObjectCache.erase(_UserId) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_UserId) == _ObjectCache.end());
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

	void CNelUser::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CNelUser::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO user (";
		
		qs += "Login, State, Privilege, ExtendedPrivilege, GMId";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_LoginName), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_State), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Privilege), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_ExtendedPrivilege), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_GMId), connection)+"'";

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

	bool CNelUser::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE user SET ";
		
		qs += "Login = '"+MSW::escapeString(NLMISC::toString(_LoginName), connection)+"'";
		qs += ", ";
		qs += "State = '"+MSW::escapeString(NLMISC::toString(_State), connection)+"'";
		qs += ", ";
		qs += "Privilege = '"+MSW::escapeString(NLMISC::toString(_Privilege), connection)+"'";
		qs += ", ";
		qs += "ExtendedPrivilege = '"+MSW::escapeString(NLMISC::toString(_ExtendedPrivilege), connection)+"'";
		qs += ", ";
		qs += "GMId = '"+MSW::escapeString(NLMISC::toString(_GMId), connection)+"'";

		qs += " WHERE UId = '"+NLMISC::toString(_UserId)+"'";
	

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

	bool CNelUser::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM user ";
		
		qs += " WHERE UId = '"+NLMISC::toString(_UserId)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CNelUser::removeById(MSW::CConnection &connection, uint32 id)
	{
		CNelUser *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM user ";
		
		qs += " WHERE UId = '"+NLMISC::toString(id)+"'";
	

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
	CNelUser *CNelUser::loadFromCache(uint32 objectId, bool unrelease)
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
			CNelUser *object = it->second;

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
	uint32 CNelUser::cacheCmd(NOPE::TCacheCmd cmd)
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

	void CNelUser::dump()
	{
		nlinfo("  Cache info for class CNelUser :");
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

	void CNelUser::updateCache()
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
				CNelUser *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CNelUser::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CNelUser *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CNelUser::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CNelUser::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CNelUser::setFirstPtr(CNelUserPtr *ptr)
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
	void CNelUser::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CNelUser @%p in cache with id %u", this, static_cast<uint32>(_UserId));
			nlverify(_ObjectCache.insert(std::make_pair(_UserId, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_UserId) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CNelUser @%p in cache with id %u", this, static_cast<uint32>(_UserId));
			nlverify(_ObjectCache.erase(_UserId) == 1);
		}
	}


	CNelUserPtr CNelUser::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CNelUser *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CNelUserPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "UId, Login, State, Privilege, ExtendedPrivilege, GMId";

		qs += " FROM user";
		
		qs += " WHERE UId = '"+NLMISC::toString(id)+"'";
	CNelUserPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CNelUser, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_UserId);
			result->getField(1, ret->_LoginName);
			result->getField(2, ret->_State);
			result->getField(3, ret->_Privilege);
			result->getField(4, ret->_ExtendedPrivilege);
			result->getField(5, ret->_GMId);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	void CNelPermissionPtr::linkPtr()
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

	void CNelPermissionPtr::unlinkPtr()
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


	CNelPermission::TObjectCache		CNelPermission::_ObjectCache;
	CNelPermission::TReleasedObject	CNelPermission::_ReleasedObject;


	// Destructor, delete any children
	CNelPermission::~CNelPermission()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CNelPermissionPtr *ptr = _PtrList;
			do
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_PermissionId != NOPE::INVALID_OBJECT_ID
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CNelPermission @%p from cache with id %u", this, static_cast<uint32>(_PermissionId));
			nlverify(_ObjectCache.erase(_PermissionId) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_PermissionId) == _ObjectCache.end());
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

	void CNelPermission::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CNelPermission::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_PermissionId != 0);
		std::string qs;
		qs = "INSERT INTO permission (";
		
		qs += "PermissionId, UId, DomainId, ShardId, AccessPrivilege";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_PermissionId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_DomainId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_ShardId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_AccessPriv), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			return true;
		}

		return false;
	}

	bool CNelPermission::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE permission SET ";
		
		qs += "PermissionId = '"+MSW::escapeString(NLMISC::toString(_PermissionId), connection)+"'";
		qs += ", ";
		qs += "UId = '"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";
		qs += ", ";
		qs += "DomainId = '"+MSW::escapeString(NLMISC::toString(_DomainId), connection)+"'";
		qs += ", ";
		qs += "ShardId = '"+MSW::escapeString(NLMISC::toString(_ShardId), connection)+"'";
		qs += ", ";
		qs += "AccessPrivilege = '"+MSW::escapeString(NLMISC::toString(_AccessPriv), connection)+"'";

		qs += " WHERE PermissionId = '"+NLMISC::toString(_PermissionId)+"'";
	

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

	bool CNelPermission::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM permission ";
		
		qs += " WHERE PermissionId = '"+NLMISC::toString(_PermissionId)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CNelPermission::removeById(MSW::CConnection &connection, uint32 id)
	{
		CNelPermission *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM permission ";
		
		qs += " WHERE PermissionId = '"+NLMISC::toString(id)+"'";
	

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
	CNelPermission *CNelPermission::loadFromCache(uint32 objectId, bool unrelease)
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
			CNelPermission *object = it->second;

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
	uint32 CNelPermission::cacheCmd(NOPE::TCacheCmd cmd)
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

	void CNelPermission::dump()
	{
		nlinfo("  Cache info for class CNelPermission :");
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

	void CNelPermission::updateCache()
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
				CNelPermission *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CNelPermission::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CNelPermission *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CNelPermission::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CNelPermission::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CNelPermission::setFirstPtr(CNelPermissionPtr *ptr)
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
	void CNelPermission::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CNelPermission @%p in cache with id %u", this, static_cast<uint32>(_PermissionId));
			nlverify(_ObjectCache.insert(std::make_pair(_PermissionId, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_PermissionId) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CNelPermission @%p in cache with id %u", this, static_cast<uint32>(_PermissionId));
			nlverify(_ObjectCache.erase(_PermissionId) == 1);
		}
	}


	CNelPermissionPtr CNelPermission::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CNelPermission *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CNelPermissionPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "PermissionId, UId, DomainId, ShardId, AccessPrivilege";

		qs += " FROM permission";
		
		qs += " WHERE PermissionId = '"+NLMISC::toString(id)+"'";
	CNelPermissionPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CNelPermission, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_PermissionId);
			result->getField(1, ret->_UserId);
			result->getField(2, ret->_DomainId);
			result->getField(3, ret->_ShardId);
			result->getField(4, ret->_AccessPriv);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


}
