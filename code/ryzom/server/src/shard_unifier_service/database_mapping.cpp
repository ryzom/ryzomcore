// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "stdpch.h"
	
#include "database_mapping.h"

namespace RSMGR
{

	void CKnownUserPtr::linkPtr()
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

	void CKnownUserPtr::unlinkPtr()
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


	CKnownUser::TObjectCache		CKnownUser::_ObjectCache;
	CKnownUser::TReleasedObject	CKnownUser::_ReleasedObject;


	// Destructor, delete any children
	CKnownUser::~CKnownUser()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CKnownUserPtr *ptr = _PtrList;
			do 
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_RelationId != NOPE::INVALID_OBJECT_ID 
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CKnownUser @%p from cache with id %u", this, static_cast<uint32>(_RelationId));
			nlverify(_ObjectCache.erase(_RelationId) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_RelationId) == _ObjectCache.end());
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

	void CKnownUser::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CKnownUser::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO known_users (";
		
		qs += "owner, targer_user, targer_character, relation_type, comments";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_OwnerId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_TargetUser), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_TargetCharacter), connection)+"'";
		qs += ", ";
		qs += "'"+_Relation.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Comments), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{
			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_OwnerId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CRingUser *parent = CRingUser::loadFromCache(_OwnerId, false);
				if (parent && parent->_KnownUsers != NULL)
				{

						nlassert(std::find(parent->_KnownUsers->begin(), parent->_KnownUsers->end(), CKnownUserPtr(this, __FILE__, __LINE__)) == parent->_KnownUsers->end());
						parent->_KnownUsers->push_back(CKnownUserPtr(this, __FILE__, __LINE__));
 
				}
			}

			if (_TargetUser != 0)
			{
				// need to update the parent class child list if it is in the cache
				CCharacter *parent = CCharacter::loadFromCache(_TargetUser, false);
				if (parent && parent->_KnownBy != NULL)
				{

						nlassert(std::find(parent->_KnownBy->begin(), parent->_KnownBy->end(), CKnownUserPtr(this, __FILE__, __LINE__)) == parent->_KnownBy->end());
						parent->_KnownBy->push_back(CKnownUserPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CKnownUser::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE known_users SET ";
		
		qs += "owner = '"+MSW::escapeString(NLMISC::toString(_OwnerId), connection)+"'";
		qs += ", ";
		qs += "targer_user = '"+MSW::escapeString(NLMISC::toString(_TargetUser), connection)+"'";
		qs += ", ";
		qs += "targer_character = '"+MSW::escapeString(NLMISC::toString(_TargetCharacter), connection)+"'";
		qs += ", ";
		qs += "relation_type = '"+_Relation.toString()+"'";
		qs += ", ";
		qs += "comments = '"+MSW::escapeString(NLMISC::toString(_Comments), connection)+"'";

		qs += " WHERE Id = '"+NLMISC::toString(_RelationId)+"'";
	

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

	bool CKnownUser::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM known_users ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_RelationId)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CRingUserPtr parent(CRingUser::loadFromCache(_OwnerId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_KnownUsers != NULL)
					{

						std::vector < CKnownUserPtr >::iterator it = std::find(parent->_KnownUsers->begin(), parent->_KnownUsers->end(), this);
						if (it != parent->_KnownUsers->end())
						{
							parent->_KnownUsers->erase(it);
						}
 
					}
				}
				
				{
					CCharacterPtr parent(CCharacter::loadFromCache(_TargetUser, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_KnownBy != NULL)
					{

						std::vector < CKnownUserPtr >::iterator it = std::find(parent->_KnownBy->begin(), parent->_KnownBy->end(), this);
						if (it != parent->_KnownBy->end())
						{
							parent->_KnownBy->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CKnownUser::removeById(MSW::CConnection &connection, uint32 id)
	{
		CKnownUser *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM known_users ";
		
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
	CKnownUser *CKnownUser::loadFromCache(uint32 objectId, bool unrelease)
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
			CKnownUser *object = it->second;

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
	uint32 CKnownUser::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CKnownUser::dump()
	{
		nlinfo("  Cache info for class CKnownUser :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CKnownUser::updateCache()
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
				CKnownUser *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CKnownUser::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CKnownUser *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CKnownUser::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CKnownUser::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CKnownUser::setFirstPtr(CKnownUserPtr *ptr)
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
	void CKnownUser::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CKnownUser @%p in cache with id %u", this, static_cast<uint32>(_RelationId));
			nlverify(_ObjectCache.insert(std::make_pair(_RelationId, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_RelationId) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CKnownUser @%p in cache with id %u", this, static_cast<uint32>(_RelationId));
			nlverify(_ObjectCache.erase(_RelationId) == 1);
		}
	}


	CKnownUserPtr CKnownUser::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CKnownUser *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CKnownUserPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id, owner, targer_user, targer_character, relation_type, comments";

		qs += " FROM known_users";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CKnownUserPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CKnownUser, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_RelationId);
			result->getField(1, ret->_OwnerId);
			result->getField(2, ret->_TargetUser);
			result->getField(3, ret->_TargetCharacter);
			{
				std::string s;
				result->getField(4, s);
				ret->_Relation = TKnownUserRelation(s);
			}
			result->getField(5, ret->_Comments);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CKnownUser::loadChildrenOfCRingUser(MSW::CConnection &connection, uint32 parentId, std::vector < CKnownUserPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, owner, targer_user, targer_character, relation_type, comments";

		qs += " FROM known_users";
		qs += " WHERE owner = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CKnownUser *ret = new CKnownUser();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_RelationId);
					
			result->getField(1, ret->_OwnerId);
					
			result->getField(2, ret->_TargetUser);
					
			result->getField(3, ret->_TargetCharacter);
					
			{
				std::string s;
				result->getField(4, s);
				ret->_Relation = TKnownUserRelation(s);
			}
					
			result->getField(5, ret->_Comments);
					CKnownUser *inCache = loadFromCache(ret->_RelationId, true);
			if (inCache != NULL)
			{

				container.push_back(CKnownUserPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CKnownUserPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	bool CKnownUser::loadChildrenOfCCharacter(MSW::CConnection &connection, uint32 parentId, std::vector < CKnownUserPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, owner, targer_user, targer_character, relation_type, comments";

		qs += " FROM known_users";
		qs += " WHERE targer_user = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CKnownUser *ret = new CKnownUser();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_RelationId);
					
			result->getField(1, ret->_OwnerId);
					
			result->getField(2, ret->_TargetUser);
					
			result->getField(3, ret->_TargetCharacter);
					
			{
				std::string s;
				result->getField(4, s);
				ret->_Relation = TKnownUserRelation(s);
			}
					
			result->getField(5, ret->_Comments);
					CKnownUser *inCache = loadFromCache(ret->_RelationId, true);
			if (inCache != NULL)
			{

				container.push_back(CKnownUserPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CKnownUserPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	void CSessionParticipantPtr::linkPtr()
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

	void CSessionParticipantPtr::unlinkPtr()
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


	CSessionParticipant::TObjectCache		CSessionParticipant::_ObjectCache;
	CSessionParticipant::TReleasedObject	CSessionParticipant::_ReleasedObject;


	// Destructor, delete any children
	CSessionParticipant::~CSessionParticipant()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CSessionParticipantPtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CSessionParticipant @%p from cache with id %u", this, static_cast<uint32>(_Id));
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

	void CSessionParticipant::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CSessionParticipant::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO session_participant (";
		
		qs += "session_id, char_id, status, kicked";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_SessionId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_CharId), connection)+"'";
		qs += ", ";
		qs += "'"+_Status.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Kicked), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{
			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_CharId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CCharacter *parent = CCharacter::loadFromCache(_CharId, false);
				if (parent && parent->_SessionParticipants != NULL)
				{

						nlassert(std::find(parent->_SessionParticipants->begin(), parent->_SessionParticipants->end(), CSessionParticipantPtr(this, __FILE__, __LINE__)) == parent->_SessionParticipants->end());
						parent->_SessionParticipants->push_back(CSessionParticipantPtr(this, __FILE__, __LINE__));
 
				}
			}

			if (_SessionId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CSession *parent = CSession::loadFromCache(_SessionId, false);
				if (parent && parent->_SessionParticipants != NULL)
				{

						nlassert(std::find(parent->_SessionParticipants->begin(), parent->_SessionParticipants->end(), CSessionParticipantPtr(this, __FILE__, __LINE__)) == parent->_SessionParticipants->end());
						parent->_SessionParticipants->push_back(CSessionParticipantPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CSessionParticipant::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE session_participant SET ";
		
		qs += "session_id = '"+MSW::escapeString(NLMISC::toString(_SessionId), connection)+"'";
		qs += ", ";
		qs += "char_id = '"+MSW::escapeString(NLMISC::toString(_CharId), connection)+"'";
		qs += ", ";
		qs += "status = '"+_Status.toString()+"'";
		qs += ", ";
		qs += "kicked = '"+MSW::escapeString(NLMISC::toString(_Kicked), connection)+"'";

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

	bool CSessionParticipant::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM session_participant ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CCharacterPtr parent(CCharacter::loadFromCache(_CharId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_SessionParticipants != NULL)
					{

						std::vector < CSessionParticipantPtr >::iterator it = std::find(parent->_SessionParticipants->begin(), parent->_SessionParticipants->end(), this);
						if (it != parent->_SessionParticipants->end())
						{
							parent->_SessionParticipants->erase(it);
						}
 
					}
				}
				
				{
					CSessionPtr parent(CSession::loadFromCache(_SessionId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_SessionParticipants != NULL)
					{

						std::vector < CSessionParticipantPtr >::iterator it = std::find(parent->_SessionParticipants->begin(), parent->_SessionParticipants->end(), this);
						if (it != parent->_SessionParticipants->end())
						{
							parent->_SessionParticipants->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CSessionParticipant::removeById(MSW::CConnection &connection, uint32 id)
	{
		CSessionParticipant *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM session_participant ";
		
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
	CSessionParticipant *CSessionParticipant::loadFromCache(uint32 objectId, bool unrelease)
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
			CSessionParticipant *object = it->second;

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
	uint32 CSessionParticipant::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CSessionParticipant::dump()
	{
		nlinfo("  Cache info for class CSessionParticipant :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CSessionParticipant::updateCache()
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
				CSessionParticipant *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CSessionParticipant::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CSessionParticipant *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CSessionParticipant::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CSessionParticipant::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CSessionParticipant::setFirstPtr(CSessionParticipantPtr *ptr)
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
	void CSessionParticipant::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CSessionParticipant @%p in cache with id %u", this, static_cast<uint32>(_Id));
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
			nldebug("NOPE: erasing CSessionParticipant @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CSessionParticipantPtr CSessionParticipant::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CSessionParticipant *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CSessionParticipantPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id, session_id, char_id, status, kicked";

		qs += " FROM session_participant";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CSessionParticipantPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CSessionParticipant, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getField(1, ret->_SessionId);
			result->getField(2, ret->_CharId);
			{
				std::string s;
				result->getField(3, s);
				ret->_Status = TSessionPartStatus(s);
			}
			result->getField(4, ret->_Kicked);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CSessionParticipant::loadChildrenOfCCharacter(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionParticipantPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, session_id, char_id, status, kicked";

		qs += " FROM session_participant";
		qs += " WHERE char_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CSessionParticipant *ret = new CSessionParticipant();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_SessionId);
					
			result->getField(2, ret->_CharId);
					
			{
				std::string s;
				result->getField(3, s);
				ret->_Status = TSessionPartStatus(s);
			}
					
			result->getField(4, ret->_Kicked);
					CSessionParticipant *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CSessionParticipantPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CSessionParticipantPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	bool CSessionParticipant::loadChildrenOfCSession(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionParticipantPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, session_id, char_id, status, kicked";

		qs += " FROM session_participant";
		qs += " WHERE session_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CSessionParticipant *ret = new CSessionParticipant();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_SessionId);
					
			result->getField(2, ret->_CharId);
					
			{
				std::string s;
				result->getField(3, s);
				ret->_Status = TSessionPartStatus(s);
			}
					
			result->getField(4, ret->_Kicked);
					CSessionParticipant *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CSessionParticipantPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CSessionParticipantPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	void CCharacterPtr::linkPtr()
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

	void CCharacterPtr::unlinkPtr()
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


	CCharacter::TObjectCache		CCharacter::_ObjectCache;
	CCharacter::TReleasedObject	CCharacter::_ReleasedObject;


	// Destructor, delete any children
	CCharacter::~CCharacter()
	{
		// release childs reference
			if (_Sessions != NULL)
						delete _Sessions;
			if (_SessionParticipants != NULL)
						delete _SessionParticipants;
			if (_KnownBy != NULL)
						delete _KnownBy;
			if (_PlayerRatings != NULL)
						delete _PlayerRatings;


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CCharacterPtr *ptr = _PtrList;
			do 
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_CharId != NOPE::INVALID_OBJECT_ID 
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CCharacter @%p from cache with id %u", this, static_cast<uint32>(_CharId));
			nlverify(_ObjectCache.erase(_CharId) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_CharId) == _ObjectCache.end());
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

	void CCharacter::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CCharacter::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_CharId != 0);
		std::string qs;
		qs = "INSERT INTO characters (";
		
		qs += "char_id, char_name, user_id, guild_id, best_combat_level, home_mainland_session_id, ring_access, race, civilisation, cult, current_session, rrp_am, rrp_masterless, rrp_author, newcomer, creation_date, last_played_date";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_CharId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_CharName), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_GuildId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_BestCombatLevel), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_HomeMainlandSessionId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RingAccess), connection)+"'";
		qs += ", ";
		qs += "'"+_Race.toString()+"'";
		qs += ", ";
		qs += "'"+_Civilisation.toString()+"'";
		qs += ", ";
		qs += "'"+_Cult.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_CurrentSession), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RRPAM), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RRPMasterless), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RRPAuthor), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Newcomer), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::encodeDate(_CreationDate)+"'";
		qs += ", ";
		qs += "'"+MSW::encodeDate(_LastPlayedDate)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_UserId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CRingUser *parent = CRingUser::loadFromCache(_UserId, false);
				if (parent && parent->_Characters != NULL)
				{

						nlverify(parent->_Characters->insert(std::make_pair(getObjectId(), CCharacterPtr(this, __FILE__, __LINE__))).second);
 
				}
			}

			if (_GuildId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CGuild *parent = CGuild::loadFromCache(_GuildId, false);
				if (parent && parent->_Characters != NULL)
				{

						nlassert(std::find(parent->_Characters->begin(), parent->_Characters->end(), CCharacterPtr(this, __FILE__, __LINE__)) == parent->_Characters->end());
						parent->_Characters->push_back(CCharacterPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CCharacter::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE characters SET ";
		
		qs += "char_id = '"+MSW::escapeString(NLMISC::toString(_CharId), connection)+"'";
		qs += ", ";
		qs += "char_name = '"+MSW::escapeString(NLMISC::toString(_CharName), connection)+"'";
		qs += ", ";
		qs += "user_id = '"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";
		qs += ", ";
		qs += "guild_id = '"+MSW::escapeString(NLMISC::toString(_GuildId), connection)+"'";
		qs += ", ";
		qs += "best_combat_level = '"+MSW::escapeString(NLMISC::toString(_BestCombatLevel), connection)+"'";
		qs += ", ";
		qs += "home_mainland_session_id = '"+MSW::escapeString(NLMISC::toString(_HomeMainlandSessionId), connection)+"'";
		qs += ", ";
		qs += "ring_access = '"+MSW::escapeString(NLMISC::toString(_RingAccess), connection)+"'";
		qs += ", ";
		qs += "race = '"+_Race.toString()+"'";
		qs += ", ";
		qs += "civilisation = '"+_Civilisation.toString()+"'";
		qs += ", ";
		qs += "cult = '"+_Cult.toString()+"'";
		qs += ", ";
		qs += "current_session = '"+MSW::escapeString(NLMISC::toString(_CurrentSession), connection)+"'";
		qs += ", ";
		qs += "rrp_am = '"+MSW::escapeString(NLMISC::toString(_RRPAM), connection)+"'";
		qs += ", ";
		qs += "rrp_masterless = '"+MSW::escapeString(NLMISC::toString(_RRPMasterless), connection)+"'";
		qs += ", ";
		qs += "rrp_author = '"+MSW::escapeString(NLMISC::toString(_RRPAuthor), connection)+"'";
		qs += ", ";
		qs += "newcomer = '"+MSW::escapeString(NLMISC::toString(_Newcomer), connection)+"'";
		qs += ", ";
		qs += "creation_date = '"+MSW::encodeDate(_CreationDate)+"'";
		qs += ", ";
		qs += "last_played_date = '"+MSW::encodeDate(_LastPlayedDate)+"'";

		qs += " WHERE char_id = '"+NLMISC::toString(_CharId)+"'";
	

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

	bool CCharacter::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM characters ";
		
		qs += " WHERE char_id = '"+NLMISC::toString(_CharId)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{

				{
					// cascading deletion for vector child SessionParticipants
					nlassert(loadSessionParticipants(connection, __FILE__, __LINE__));

					const std::vector < CSessionParticipantPtr > & childs = getSessionParticipants();

					while (!childs.empty())
					{
						getSessionParticipantsByIndex((uint32)childs.size()-1)->remove(connection);
					}
				}


				{
					// cascading deletion for vector child KnownBy
					nlassert(loadKnownBy(connection, __FILE__, __LINE__));

					const std::vector < CKnownUserPtr > & childs = getKnownBy();

					while (!childs.empty())
					{
						getKnownByByIndex((uint32)childs.size()-1)->remove(connection);
					}
				}


				{
					// unreference (and update) for vector child PlayerRatings
					nlassert(loadPlayerRatings(connection, __FILE__, __LINE__));

					const std::vector < CPlayerRatingPtr > & childs = getPlayerRatings();

					for (uint i=0; i < childs.size(); ++i)
					{
						
						getPlayerRatingsByIndex(i)->setAuthor(0);
						getPlayerRatingsByIndex(i)->update(connection);
					}
				}


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CRingUserPtr parent(CRingUser::loadFromCache(_UserId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_Characters != NULL)
					{

						parent->_Characters->erase(getObjectId());
 
					}
				}
				
				{
					CGuildPtr parent(CGuild::loadFromCache(_GuildId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_Characters != NULL)
					{

						std::vector < CCharacterPtr >::iterator it = std::find(parent->_Characters->begin(), parent->_Characters->end(), this);
						if (it != parent->_Characters->end())
						{
							parent->_Characters->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CCharacter::removeById(MSW::CConnection &connection, uint32 id)
	{
		CCharacter *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM characters ";
		
		qs += " WHERE char_id = '"+NLMISC::toString(id)+"'";
	

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
	CCharacter *CCharacter::loadFromCache(uint32 objectId, bool unrelease)
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
			CCharacter *object = it->second;

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
	uint32 CCharacter::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CCharacter::dump()
	{
		nlinfo("  Cache info for class CCharacter :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CCharacter::updateCache()
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
				CCharacter *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CCharacter::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CCharacter *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CCharacter::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CCharacter::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CCharacter::setFirstPtr(CCharacterPtr *ptr)
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
	void CCharacter::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CCharacter @%p in cache with id %u", this, static_cast<uint32>(_CharId));
			nlverify(_ObjectCache.insert(std::make_pair(_CharId, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_CharId) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CCharacter @%p in cache with id %u", this, static_cast<uint32>(_CharId));
			nlverify(_ObjectCache.erase(_CharId) == 1);
		}
	}


	CCharacterPtr CCharacter::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CCharacter *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CCharacterPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "char_id, char_name, user_id, guild_id, best_combat_level, home_mainland_session_id, ring_access, race, civilisation, cult, current_session, rrp_am, rrp_masterless, rrp_author, newcomer, creation_date, last_played_date";

		qs += " FROM characters";
		
		qs += " WHERE char_id = '"+NLMISC::toString(id)+"'";
	CCharacterPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CCharacter, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_CharId);
			result->getField(1, ret->_CharName);
			result->getField(2, ret->_UserId);
			result->getField(3, ret->_GuildId);
			result->getField(4, ret->_BestCombatLevel);
			result->getField(5, ret->_HomeMainlandSessionId);
			result->getField(6, ret->_RingAccess);
			{
				std::string s;
				result->getField(7, s);
				ret->_Race = CHARSYNC::TRace(s);
			}
			{
				std::string s;
				result->getField(8, s);
				ret->_Civilisation = CHARSYNC::TCivilisation(s);
			}
			{
				std::string s;
				result->getField(9, s);
				ret->_Cult = CHARSYNC::TCult(s);
			}
			result->getField(10, ret->_CurrentSession);
			result->getField(11, ret->_RRPAM);
			result->getField(12, ret->_RRPMasterless);
			result->getField(13, ret->_RRPAuthor);
			result->getField(14, ret->_Newcomer);
			result->getDateField(15, ret->_CreationDate);
			result->getDateField(16, ret->_LastPlayedDate);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CCharacter::loadChildrenOfCRingUser(MSW::CConnection &connection, uint32 parentId, std::map < uint32, CCharacterPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "char_id, char_name, user_id, guild_id, best_combat_level, home_mainland_session_id, ring_access, race, civilisation, cult, current_session, rrp_am, rrp_masterless, rrp_author, newcomer, creation_date, last_played_date";

		qs += " FROM characters";
		qs += " WHERE user_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CCharacter *ret = new CCharacter();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_CharId);
					
			result->getField(1, ret->_CharName);
					
			result->getField(2, ret->_UserId);
					
			result->getField(3, ret->_GuildId);
					
			result->getField(4, ret->_BestCombatLevel);
					
			result->getField(5, ret->_HomeMainlandSessionId);
					
			result->getField(6, ret->_RingAccess);
					
			{
				std::string s;
				result->getField(7, s);
				ret->_Race = CHARSYNC::TRace(s);
			}
					
			{
				std::string s;
				result->getField(8, s);
				ret->_Civilisation = CHARSYNC::TCivilisation(s);
			}
					
			{
				std::string s;
				result->getField(9, s);
				ret->_Cult = CHARSYNC::TCult(s);
			}
					
			result->getField(10, ret->_CurrentSession);
					
			result->getField(11, ret->_RRPAM);
					
			result->getField(12, ret->_RRPMasterless);
					
			result->getField(13, ret->_RRPAuthor);
					
			result->getField(14, ret->_Newcomer);
					
			result->getDateField(15, ret->_CreationDate);
					
			result->getDateField(16, ret->_LastPlayedDate);
					CCharacter *inCache = loadFromCache(ret->_CharId, true);
			if (inCache != NULL)
			{

				container.insert(std::make_pair(inCache->getObjectId(), CCharacterPtr(inCache, filename, lineNum)));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.insert(std::make_pair(ret->getObjectId(), CCharacterPtr(ret, filename, lineNum)));

			}
		}

		return true;
	}

	bool CCharacter::loadChildrenOfCGuild(MSW::CConnection &connection, uint32 parentId, std::vector < CCharacterPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "char_id, char_name, user_id, guild_id, best_combat_level, home_mainland_session_id, ring_access, race, civilisation, cult, current_session, rrp_am, rrp_masterless, rrp_author, newcomer, creation_date, last_played_date";

		qs += " FROM characters";
		qs += " WHERE guild_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CCharacter *ret = new CCharacter();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_CharId);
					
			result->getField(1, ret->_CharName);
					
			result->getField(2, ret->_UserId);
					
			result->getField(3, ret->_GuildId);
					
			result->getField(4, ret->_BestCombatLevel);
					
			result->getField(5, ret->_HomeMainlandSessionId);
					
			result->getField(6, ret->_RingAccess);
					
			{
				std::string s;
				result->getField(7, s);
				ret->_Race = CHARSYNC::TRace(s);
			}
					
			{
				std::string s;
				result->getField(8, s);
				ret->_Civilisation = CHARSYNC::TCivilisation(s);
			}
					
			{
				std::string s;
				result->getField(9, s);
				ret->_Cult = CHARSYNC::TCult(s);
			}
					
			result->getField(10, ret->_CurrentSession);
					
			result->getField(11, ret->_RRPAM);
					
			result->getField(12, ret->_RRPMasterless);
					
			result->getField(13, ret->_RRPAuthor);
					
			result->getField(14, ret->_Newcomer);
					
			result->getDateField(15, ret->_CreationDate);
					
			result->getDateField(16, ret->_LastPlayedDate);
					CCharacter *inCache = loadFromCache(ret->_CharId, true);
			if (inCache != NULL)
			{

				container.push_back(CCharacterPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CCharacterPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	bool CCharacter::loadSessions(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_Sessions != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_Sessions = new std::vector < CSessionPtr >;
		
		// load the childs
		ret &= CSession::loadChildrenOfCCharacter(connection, getObjectId(), *_Sessions, filename, lineNum);
		return ret;
	}


	const std::vector<CSessionPtr> &CCharacter::getSessions() const
	{
		nlassert(_Sessions != NULL);
		return *_Sessions;
	}

	CSessionPtr &CCharacter::getSessionsByIndex(uint32 index) const
	{
		nlassert(_Sessions != NULL);
		nlassert(index < _Sessions->size());
		return const_cast< CSessionPtr & >(_Sessions->operator[](index));
	}
	
	CSessionPtr &CCharacter::getSessionsById(uint32 id) const
	{
		nlassert(_Sessions != NULL);
		std::vector<CSessionPtr >::const_iterator first(_Sessions->begin()), last(_Sessions->end());
		for (; first != last; ++first)
		{
			const CSessionPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CSessionPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CSessionPtr nil;

		return nil;
	}

	
	bool CCharacter::loadSessionParticipants(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_SessionParticipants != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_SessionParticipants = new std::vector < CSessionParticipantPtr >;
		
		// load the childs
		ret &= CSessionParticipant::loadChildrenOfCCharacter(connection, getObjectId(), *_SessionParticipants, filename, lineNum);
		return ret;
	}


	const std::vector<CSessionParticipantPtr> &CCharacter::getSessionParticipants() const
	{
		nlassert(_SessionParticipants != NULL);
		return *_SessionParticipants;
	}

	CSessionParticipantPtr &CCharacter::getSessionParticipantsByIndex(uint32 index) const
	{
		nlassert(_SessionParticipants != NULL);
		nlassert(index < _SessionParticipants->size());
		return const_cast< CSessionParticipantPtr & >(_SessionParticipants->operator[](index));
	}
	
	CSessionParticipantPtr &CCharacter::getSessionParticipantsById(uint32 id) const
	{
		nlassert(_SessionParticipants != NULL);
		std::vector<CSessionParticipantPtr >::const_iterator first(_SessionParticipants->begin()), last(_SessionParticipants->end());
		for (; first != last; ++first)
		{
			const CSessionParticipantPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CSessionParticipantPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CSessionParticipantPtr nil;

		return nil;
	}

	
	bool CCharacter::loadKnownBy(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_KnownBy != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_KnownBy = new std::vector < CKnownUserPtr >;
		
		// load the childs
		ret &= CKnownUser::loadChildrenOfCCharacter(connection, getObjectId(), *_KnownBy, filename, lineNum);
		return ret;
	}


	const std::vector<CKnownUserPtr> &CCharacter::getKnownBy() const
	{
		nlassert(_KnownBy != NULL);
		return *_KnownBy;
	}

	CKnownUserPtr &CCharacter::getKnownByByIndex(uint32 index) const
	{
		nlassert(_KnownBy != NULL);
		nlassert(index < _KnownBy->size());
		return const_cast< CKnownUserPtr & >(_KnownBy->operator[](index));
	}
	
	CKnownUserPtr &CCharacter::getKnownByById(uint32 id) const
	{
		nlassert(_KnownBy != NULL);
		std::vector<CKnownUserPtr >::const_iterator first(_KnownBy->begin()), last(_KnownBy->end());
		for (; first != last; ++first)
		{
			const CKnownUserPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CKnownUserPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CKnownUserPtr nil;

		return nil;
	}

	
	bool CCharacter::loadPlayerRatings(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_PlayerRatings != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_PlayerRatings = new std::vector < CPlayerRatingPtr >;
		
		// load the childs
		ret &= CPlayerRating::loadChildrenOfCCharacter(connection, getObjectId(), *_PlayerRatings, filename, lineNum);
		return ret;
	}


	const std::vector<CPlayerRatingPtr> &CCharacter::getPlayerRatings() const
	{
		nlassert(_PlayerRatings != NULL);
		return *_PlayerRatings;
	}

	CPlayerRatingPtr &CCharacter::getPlayerRatingsByIndex(uint32 index) const
	{
		nlassert(_PlayerRatings != NULL);
		nlassert(index < _PlayerRatings->size());
		return const_cast< CPlayerRatingPtr & >(_PlayerRatings->operator[](index));
	}
	
	CPlayerRatingPtr &CCharacter::getPlayerRatingsById(uint32 id) const
	{
		nlassert(_PlayerRatings != NULL);
		std::vector<CPlayerRatingPtr >::const_iterator first(_PlayerRatings->begin()), last(_PlayerRatings->end());
		for (; first != last; ++first)
		{
			const CPlayerRatingPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CPlayerRatingPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CPlayerRatingPtr nil;

		return nil;
	}

	
	void CRingUserPtr::linkPtr()
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

	void CRingUserPtr::unlinkPtr()
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


	CRingUser::TObjectCache		CRingUser::_ObjectCache;
	CRingUser::TReleasedObject	CRingUser::_ReleasedObject;


	// Destructor, delete any children
	CRingUser::~CRingUser()
	{
		// release childs reference
			if (_KnownUsers != NULL)
						delete _KnownUsers;
			if (_Characters != NULL)
						delete _Characters;
			if (_Folders != NULL)
						delete _Folders;
			if (_FolderAccess != NULL)
						delete _FolderAccess;


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CRingUserPtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CRingUser @%p from cache with id %u", this, static_cast<uint32>(_UserId));
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

	void CRingUser::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CRingUser::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_UserId != 0);
		std::string qs;
		qs = "INSERT INTO ring_users (";
		
		qs += "user_id, user_name, current_char, current_session, current_activity, current_status, public_level, account_type, content_access_level, description, lang, cookie, current_domain_id, add_privileges";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_UserName), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_CurrentCharacter), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_CurrentSession), connection)+"'";
		qs += ", ";
		qs += "'"+_CurrentActivity.toString()+"'";
		qs += ", ";
		qs += "'"+_CurrentStatus.toString()+"'";
		qs += ", ";
		qs += "'"+_PublicLevel.toString()+"'";
		qs += ", ";
		qs += "'"+_AccountType.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_ContentAccessLevel), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Description), connection)+"'";
		qs += ", ";
		qs += "'"+_Lang.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Cookie), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_CurrentDomainId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_AddedPrivileges), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			return true;
		}

		return false;
	}

	bool CRingUser::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE ring_users SET ";
		
		qs += "user_id = '"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";
		qs += ", ";
		qs += "user_name = '"+MSW::escapeString(NLMISC::toString(_UserName), connection)+"'";
		qs += ", ";
		qs += "current_char = '"+MSW::escapeString(NLMISC::toString(_CurrentCharacter), connection)+"'";
		qs += ", ";
		qs += "current_session = '"+MSW::escapeString(NLMISC::toString(_CurrentSession), connection)+"'";
		qs += ", ";
		qs += "current_activity = '"+_CurrentActivity.toString()+"'";
		qs += ", ";
		qs += "current_status = '"+_CurrentStatus.toString()+"'";
		qs += ", ";
		qs += "public_level = '"+_PublicLevel.toString()+"'";
		qs += ", ";
		qs += "account_type = '"+_AccountType.toString()+"'";
		qs += ", ";
		qs += "content_access_level = '"+MSW::escapeString(NLMISC::toString(_ContentAccessLevel), connection)+"'";
		qs += ", ";
		qs += "description = '"+MSW::escapeString(NLMISC::toString(_Description), connection)+"'";
		qs += ", ";
		qs += "lang = '"+_Lang.toString()+"'";
		qs += ", ";
		qs += "cookie = '"+MSW::escapeString(NLMISC::toString(_Cookie), connection)+"'";
		qs += ", ";
		qs += "current_domain_id = '"+MSW::escapeString(NLMISC::toString(_CurrentDomainId), connection)+"'";
		qs += ", ";
		qs += "add_privileges = '"+MSW::escapeString(NLMISC::toString(_AddedPrivileges), connection)+"'";

		qs += " WHERE user_id = '"+NLMISC::toString(_UserId)+"'";
	

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

	bool CRingUser::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM ring_users ";
		
		qs += " WHERE user_id = '"+NLMISC::toString(_UserId)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{

				{
					// cascading deletion for vector child KnownUsers
					nlassert(loadKnownUsers(connection, __FILE__, __LINE__));

					const std::vector < CKnownUserPtr > & childs = getKnownUsers();

					while (!childs.empty())
					{
						getKnownUsersByIndex((uint32)childs.size()-1)->remove(connection);
					}
				}


				{
					// cascading deletion for map child Characters
					nlassert(loadCharacters(connection, __FILE__, __LINE__));

					const std::map < uint32, CCharacterPtr > & childs = getCharacters();

					while (!childs.empty())
					{
						getCharactersById(childs.begin()->first)->remove(connection);
					}
				}

				{
					// cascading deletion for single child GMStatus
					nlassert(loadGMStatus(connection, __FILE__, __LINE__));
					
					if (getGMStatus() != NULL)
						getGMStatus()->remove(connection);
				}

				{
					// unreference (and update) for vector child Folders
					nlassert(loadFolders(connection, __FILE__, __LINE__));

					const std::vector < CFolderPtr > & childs = getFolders();

					for (uint i=0; i < childs.size(); ++i)
					{
						
						getFoldersByIndex(i)->setAuthor(0);
						getFoldersByIndex(i)->update(connection);
					}
				}

				{
					// unreference (and update) for vector child FolderAccess
					nlassert(loadFolderAccess(connection, __FILE__, __LINE__));

					const std::vector < CFolderAccessPtr > & childs = getFolderAccess();

					for (uint i=0; i < childs.size(); ++i)
					{
						
						getFolderAccessByIndex(i)->setUserId(0);
						getFolderAccessByIndex(i)->update(connection);
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

	bool CRingUser::removeById(MSW::CConnection &connection, uint32 id)
	{
		CRingUser *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM ring_users ";
		
		qs += " WHERE user_id = '"+NLMISC::toString(id)+"'";
	

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
	CRingUser *CRingUser::loadFromCache(uint32 objectId, bool unrelease)
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
			CRingUser *object = it->second;

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
	uint32 CRingUser::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CRingUser::dump()
	{
		nlinfo("  Cache info for class CRingUser :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CRingUser::updateCache()
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
				CRingUser *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CRingUser::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CRingUser *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CRingUser::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CRingUser::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CRingUser::setFirstPtr(CRingUserPtr *ptr)
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
	void CRingUser::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CRingUser @%p in cache with id %u", this, static_cast<uint32>(_UserId));
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
			nldebug("NOPE: erasing CRingUser @%p in cache with id %u", this, static_cast<uint32>(_UserId));
			nlverify(_ObjectCache.erase(_UserId) == 1);
		}
	}


	CRingUserPtr CRingUser::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CRingUser *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CRingUserPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "user_id, user_name, current_char, current_session, current_activity, current_status, public_level, account_type, content_access_level, description, lang, cookie, current_domain_id, add_privileges";

		qs += " FROM ring_users";
		
		qs += " WHERE user_id = '"+NLMISC::toString(id)+"'";
	CRingUserPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CRingUser, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_UserId);
			result->getField(1, ret->_UserName);
			result->getField(2, ret->_CurrentCharacter);
			result->getField(3, ret->_CurrentSession);
			{
				std::string s;
				result->getField(4, s);
				ret->_CurrentActivity = TCurrentActivity(s);
			}
			{
				std::string s;
				result->getField(5, s);
				ret->_CurrentStatus = TCurrentStatus(s);
			}
			{
				std::string s;
				result->getField(6, s);
				ret->_PublicLevel = TPublicLevel(s);
			}
			{
				std::string s;
				result->getField(7, s);
				ret->_AccountType = TAccountType(s);
			}
			result->getField(8, ret->_ContentAccessLevel);
			result->getField(9, ret->_Description);
			{
				std::string s;
				result->getField(10, s);
				ret->_Lang = TLanguage(s);
			}
			result->getField(11, ret->_Cookie);
			result->getField(12, ret->_CurrentDomainId);
			result->getField(13, ret->_AddedPrivileges);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CRingUser::loadKnownUsers(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_KnownUsers != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_KnownUsers = new std::vector < CKnownUserPtr >;
		
		// load the childs
		ret &= CKnownUser::loadChildrenOfCRingUser(connection, getObjectId(), *_KnownUsers, filename, lineNum);
		return ret;
	}


	const std::vector<CKnownUserPtr> &CRingUser::getKnownUsers() const
	{
		nlassert(_KnownUsers != NULL);
		return *_KnownUsers;
	}

	CKnownUserPtr &CRingUser::getKnownUsersByIndex(uint32 index) const
	{
		nlassert(_KnownUsers != NULL);
		nlassert(index < _KnownUsers->size());
		return const_cast< CKnownUserPtr & >(_KnownUsers->operator[](index));
	}
	
	CKnownUserPtr &CRingUser::getKnownUsersById(uint32 id) const
	{
		nlassert(_KnownUsers != NULL);
		std::vector<CKnownUserPtr >::const_iterator first(_KnownUsers->begin()), last(_KnownUsers->end());
		for (; first != last; ++first)
		{
			const CKnownUserPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CKnownUserPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CKnownUserPtr nil;

		return nil;
	}

	
	bool CRingUser::loadCharacters(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_Characters != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_Characters = new std::map < uint32,  CCharacterPtr >;
		
		// load the childs
		ret &= CCharacter::loadChildrenOfCRingUser(connection, getObjectId(), *_Characters, filename, lineNum);
		return ret;
	}


	const std::map<uint32, CCharacterPtr> &CRingUser::getCharacters() const
	{
		nlassert(_Characters != NULL);
		return *_Characters;
	}

	CCharacterPtr &CRingUser::getCharactersById(uint32 id) const
	{
		nlassert(_Characters != NULL);
		std::map<uint32, CCharacterPtr>::const_iterator it(_Characters->find(id));

		if (it == _Characters->end())
		{
			// no object with this id, return a null pointer
			static CCharacterPtr nil;
			return nil;
		}

		return const_cast< CCharacterPtr & >(it->second);
	}

	
	bool CRingUser::loadFolders(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_Folders != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_Folders = new std::vector < CFolderPtr >;
		
		// load the childs
		ret &= CFolder::loadChildrenOfCRingUser(connection, getObjectId(), *_Folders, filename, lineNum);
		return ret;
	}


	const std::vector<CFolderPtr> &CRingUser::getFolders() const
	{
		nlassert(_Folders != NULL);
		return *_Folders;
	}

	CFolderPtr &CRingUser::getFoldersByIndex(uint32 index) const
	{
		nlassert(_Folders != NULL);
		nlassert(index < _Folders->size());
		return const_cast< CFolderPtr & >(_Folders->operator[](index));
	}
	
	CFolderPtr &CRingUser::getFoldersById(uint32 id) const
	{
		nlassert(_Folders != NULL);
		std::vector<CFolderPtr >::const_iterator first(_Folders->begin()), last(_Folders->end());
		for (; first != last; ++first)
		{
			const CFolderPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CFolderPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CFolderPtr nil;

		return nil;
	}

	
	bool CRingUser::loadFolderAccess(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_FolderAccess != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_FolderAccess = new std::vector < CFolderAccessPtr >;
		
		// load the childs
		ret &= CFolderAccess::loadChildrenOfCRingUser(connection, getObjectId(), *_FolderAccess, filename, lineNum);
		return ret;
	}


	const std::vector<CFolderAccessPtr> &CRingUser::getFolderAccess() const
	{
		nlassert(_FolderAccess != NULL);
		return *_FolderAccess;
	}

	CFolderAccessPtr &CRingUser::getFolderAccessByIndex(uint32 index) const
	{
		nlassert(_FolderAccess != NULL);
		nlassert(index < _FolderAccess->size());
		return const_cast< CFolderAccessPtr & >(_FolderAccess->operator[](index));
	}
	
	CFolderAccessPtr &CRingUser::getFolderAccessById(uint32 id) const
	{
		nlassert(_FolderAccess != NULL);
		std::vector<CFolderAccessPtr >::const_iterator first(_FolderAccess->begin()), last(_FolderAccess->end());
		for (; first != last; ++first)
		{
			const CFolderAccessPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CFolderAccessPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CFolderAccessPtr nil;

		return nil;
	}

	
	bool CRingUser::loadGMStatus(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		if (_GMStatusLoaded)
		{
			// the child is already loaded, just return true
			return true;
		}
 		bool ret = CGmStatus::loadChildOfCRingUser(connection, getObjectId(), _GMStatus, filename, lineNum);
		_GMStatusLoaded = true;
		return ret;
	}

	/** Return the one child object (or null if not) */
	CGmStatusPtr CRingUser::getGMStatus()
	{
		nlassert(_GMStatusLoaded);
		return _GMStatus;
	}


	void CSessionPtr::linkPtr()
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

	void CSessionPtr::unlinkPtr()
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


	CSession::TObjectCache		CSession::_ObjectCache;
	CSession::TReleasedObject	CSession::_ReleasedObject;


	// Destructor, delete any children
	CSession::~CSession()
	{
		// release childs reference
			if (_SessionParticipants != NULL)
						delete _SessionParticipants;
			if (_GuildInvites != NULL)
						delete _GuildInvites;
			if (_JournalEntries != NULL)
						delete _JournalEntries;


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CSessionPtr *ptr = _PtrList;
			do 
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_SessionId != NOPE::INVALID_OBJECT_ID 
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CSession @%p from cache with id %u", this, static_cast<uint32>(_SessionId));
			nlverify(_ObjectCache.erase(_SessionId) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_SessionId) == _ObjectCache.end());
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

	void CSession::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CSession::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO sessions (";
		
		qs += "session_type, title, owner, plan_date, start_date, description, orientation, level, rule_type, access_type, state, host_shard_id, subscription_slots, reserved_slots, estimated_duration, final_duration, folder_id, lang, icone, anim_mode, race_filter, religion_filter, guild_filter, shard_filter, level_filter, subscription_closed, newcomer";
		qs += ") VALUES (";
		
		qs += "'"+_SessionType.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Title), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_OwnerId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::encodeDate(_PlanDate)+"'";
		qs += ", ";
		qs += "'"+MSW::encodeDate(_StartDate)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Description), connection)+"'";
		qs += ", ";
		qs += "'"+_Orientation.toString()+"'";
		qs += ", ";
		qs += "'"+_Level.toString()+"'";
		qs += ", ";
		qs += "'"+_RuleType.toString()+"'";
		qs += ", ";
		qs += "'"+_AccessType.toString()+"'";
		qs += ", ";
		qs += "'"+_State.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_HostShardId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_SubscriptionSlots), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_ReservedSlots), connection)+"'";
		qs += ", ";
		qs += "'"+_EstimatedDuration.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_FinalDuration), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_FolderId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Lang), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Icone), connection)+"'";
		qs += ", ";
		qs += "'"+_AnimMode.toString()+"'";
		qs += ", ";
		qs += "'"+_RaceFilter.toString()+"'";
		qs += ", ";
		qs += "'"+_ReligionFilter.toString()+"'";
		qs += ", ";
		qs += "'"+_GuildFilter.toString()+"'";
		qs += ", ";
		qs += "'"+_ShardFilter.toString()+"'";
		qs += ", ";
		qs += "'"+_LevelFilter.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_SubscriptionClosed), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Newcomer), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{
			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_OwnerId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CCharacter *parent = CCharacter::loadFromCache(_OwnerId, false);
				if (parent && parent->_Sessions != NULL)
				{

						nlassert(std::find(parent->_Sessions->begin(), parent->_Sessions->end(), CSessionPtr(this, __FILE__, __LINE__)) == parent->_Sessions->end());
						parent->_Sessions->push_back(CSessionPtr(this, __FILE__, __LINE__));
 
				}
			}

			if (_FolderId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CFolder *parent = CFolder::loadFromCache(_FolderId, false);
				if (parent && parent->_Sessions != NULL)
				{

						nlassert(std::find(parent->_Sessions->begin(), parent->_Sessions->end(), CSessionPtr(this, __FILE__, __LINE__)) == parent->_Sessions->end());
						parent->_Sessions->push_back(CSessionPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CSession::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE sessions SET ";
		
		qs += "session_type = '"+_SessionType.toString()+"'";
		qs += ", ";
		qs += "title = '"+MSW::escapeString(NLMISC::toString(_Title), connection)+"'";
		qs += ", ";
		qs += "owner = '"+MSW::escapeString(NLMISC::toString(_OwnerId), connection)+"'";
		qs += ", ";
		qs += "plan_date = '"+MSW::encodeDate(_PlanDate)+"'";
		qs += ", ";
		qs += "start_date = '"+MSW::encodeDate(_StartDate)+"'";
		qs += ", ";
		qs += "description = '"+MSW::escapeString(NLMISC::toString(_Description), connection)+"'";
		qs += ", ";
		qs += "orientation = '"+_Orientation.toString()+"'";
		qs += ", ";
		qs += "level = '"+_Level.toString()+"'";
		qs += ", ";
		qs += "rule_type = '"+_RuleType.toString()+"'";
		qs += ", ";
		qs += "access_type = '"+_AccessType.toString()+"'";
		qs += ", ";
		qs += "state = '"+_State.toString()+"'";
		qs += ", ";
		qs += "host_shard_id = '"+MSW::escapeString(NLMISC::toString(_HostShardId), connection)+"'";
		qs += ", ";
		qs += "subscription_slots = '"+MSW::escapeString(NLMISC::toString(_SubscriptionSlots), connection)+"'";
		qs += ", ";
		qs += "reserved_slots = '"+MSW::escapeString(NLMISC::toString(_ReservedSlots), connection)+"'";
		qs += ", ";
		qs += "estimated_duration = '"+_EstimatedDuration.toString()+"'";
		qs += ", ";
		qs += "final_duration = '"+MSW::escapeString(NLMISC::toString(_FinalDuration), connection)+"'";
		qs += ", ";
		qs += "folder_id = '"+MSW::escapeString(NLMISC::toString(_FolderId), connection)+"'";
		qs += ", ";
		qs += "lang = '"+MSW::escapeString(NLMISC::toString(_Lang), connection)+"'";
		qs += ", ";
		qs += "icone = '"+MSW::escapeString(NLMISC::toString(_Icone), connection)+"'";
		qs += ", ";
		qs += "anim_mode = '"+_AnimMode.toString()+"'";
		qs += ", ";
		qs += "race_filter = '"+_RaceFilter.toString()+"'";
		qs += ", ";
		qs += "religion_filter = '"+_ReligionFilter.toString()+"'";
		qs += ", ";
		qs += "guild_filter = '"+_GuildFilter.toString()+"'";
		qs += ", ";
		qs += "shard_filter = '"+_ShardFilter.toString()+"'";
		qs += ", ";
		qs += "level_filter = '"+_LevelFilter.toString()+"'";
		qs += ", ";
		qs += "subscription_closed = '"+MSW::escapeString(NLMISC::toString(_SubscriptionClosed), connection)+"'";
		qs += ", ";
		qs += "newcomer = '"+MSW::escapeString(NLMISC::toString(_Newcomer), connection)+"'";

		qs += " WHERE session_id = '"+NLMISC::toString(_SessionId)+"'";
	

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

	bool CSession::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM sessions ";
		
		qs += " WHERE session_id = '"+NLMISC::toString(_SessionId)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{

				{
					// cascading deletion for vector child SessionParticipants
					nlassert(loadSessionParticipants(connection, __FILE__, __LINE__));

					const std::vector < CSessionParticipantPtr > & childs = getSessionParticipants();

					while (!childs.empty())
					{
						getSessionParticipantsByIndex((uint32)childs.size()-1)->remove(connection);
					}
				}


				{
					// cascading deletion for vector child GuildInvites
					nlassert(loadGuildInvites(connection, __FILE__, __LINE__));

					const std::vector < CGuildInvitePtr > & childs = getGuildInvites();

					while (!childs.empty())
					{
						getGuildInvitesByIndex((uint32)childs.size()-1)->remove(connection);
					}
				}


				{
					// cascading deletion for vector child JournalEntries
					nlassert(loadJournalEntries(connection, __FILE__, __LINE__));

					const std::vector < CJournalEntryPtr > & childs = getJournalEntries();

					while (!childs.empty())
					{
						getJournalEntriesByIndex((uint32)childs.size()-1)->remove(connection);
					}
				}



				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CCharacterPtr parent(CCharacter::loadFromCache(_OwnerId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_Sessions != NULL)
					{

						std::vector < CSessionPtr >::iterator it = std::find(parent->_Sessions->begin(), parent->_Sessions->end(), this);
						if (it != parent->_Sessions->end())
						{
							parent->_Sessions->erase(it);
						}
 
					}
				}
				
				{
					CFolderPtr parent(CFolder::loadFromCache(_FolderId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_Sessions != NULL)
					{

						std::vector < CSessionPtr >::iterator it = std::find(parent->_Sessions->begin(), parent->_Sessions->end(), this);
						if (it != parent->_Sessions->end())
						{
							parent->_Sessions->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CSession::removeById(MSW::CConnection &connection, uint32 id)
	{
		CSession *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM sessions ";
		
		qs += " WHERE session_id = '"+NLMISC::toString(id)+"'";
	

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
	CSession *CSession::loadFromCache(uint32 objectId, bool unrelease)
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
			CSession *object = it->second;

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
	uint32 CSession::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CSession::dump()
	{
		nlinfo("  Cache info for class CSession :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CSession::updateCache()
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
				CSession *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CSession::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CSession *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CSession::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CSession::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CSession::setFirstPtr(CSessionPtr *ptr)
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
	void CSession::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CSession @%p in cache with id %u", this, static_cast<uint32>(_SessionId));
			nlverify(_ObjectCache.insert(std::make_pair(_SessionId, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_SessionId) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CSession @%p in cache with id %u", this, static_cast<uint32>(_SessionId));
			nlverify(_ObjectCache.erase(_SessionId) == 1);
		}
	}


	CSessionPtr CSession::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CSession *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CSessionPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "session_id, session_type, title, owner, plan_date, start_date, description, orientation, level, rule_type, access_type, state, host_shard_id, subscription_slots, reserved_slots, estimated_duration, final_duration, folder_id, lang, icone, anim_mode, race_filter, religion_filter, guild_filter, shard_filter, level_filter, subscription_closed, newcomer";

		qs += " FROM sessions";
		
		qs += " WHERE session_id = '"+NLMISC::toString(id)+"'";
	CSessionPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CSession, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_SessionId);
			{
				std::string s;
				result->getField(1, s);
				ret->_SessionType = TSessionType(s);
			}
			result->getField(2, ret->_Title);
			result->getField(3, ret->_OwnerId);
			result->getDateField(4, ret->_PlanDate);
			result->getDateField(5, ret->_StartDate);
			result->getField(6, ret->_Description);
			{
				std::string s;
				result->getField(7, s);
				ret->_Orientation = TSessionOrientation(s);
			}
			{
				std::string s;
				result->getField(8, s);
				ret->_Level = R2::TSessionLevel(s);
			}
			{
				std::string s;
				result->getField(9, s);
				ret->_RuleType = TRuleType(s);
			}
			{
				std::string s;
				result->getField(10, s);
				ret->_AccessType = TAccessType(s);
			}
			{
				std::string s;
				result->getField(11, s);
				ret->_State = TSessionState(s);
			}
			result->getField(12, ret->_HostShardId);
			result->getField(13, ret->_SubscriptionSlots);
			result->getField(14, ret->_ReservedSlots);
			{
				std::string s;
				result->getField(15, s);
				ret->_EstimatedDuration = TEstimatedDuration(s);
			}
			result->getField(16, ret->_FinalDuration);
			result->getField(17, ret->_FolderId);
			result->getField(18, ret->_Lang);
			result->getField(19, ret->_Icone);
			{
				std::string s;
				result->getField(20, s);
				ret->_AnimMode = TAnimMode(s);
			}
			{
				std::string s;
				result->getField(21, s);
				ret->_RaceFilter = TRaceFilter(s);
			}
			{
				std::string s;
				result->getField(22, s);
				ret->_ReligionFilter = TReligionFilter(s);
			}
			{
				std::string s;
				result->getField(23, s);
				ret->_GuildFilter = TGuildFilter(s);
			}
			{
				std::string s;
				result->getField(24, s);
				ret->_ShardFilter = TShardFilter(s);
			}
			{
				std::string s;
				result->getField(25, s);
				ret->_LevelFilter = TLevelFilter(s);
			}
			result->getField(26, ret->_SubscriptionClosed);
			result->getField(27, ret->_Newcomer);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CSession::loadChildrenOfCCharacter(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "session_id, session_type, title, owner, plan_date, start_date, description, orientation, level, rule_type, access_type, state, host_shard_id, subscription_slots, reserved_slots, estimated_duration, final_duration, folder_id, lang, icone, anim_mode, race_filter, religion_filter, guild_filter, shard_filter, level_filter, subscription_closed, newcomer";

		qs += " FROM sessions";
		qs += " WHERE owner = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CSession *ret = new CSession();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_SessionId);
					
			{
				std::string s;
				result->getField(1, s);
				ret->_SessionType = TSessionType(s);
			}
					
			result->getField(2, ret->_Title);
					
			result->getField(3, ret->_OwnerId);
					
			result->getDateField(4, ret->_PlanDate);
					
			result->getDateField(5, ret->_StartDate);
					
			result->getField(6, ret->_Description);
					
			{
				std::string s;
				result->getField(7, s);
				ret->_Orientation = TSessionOrientation(s);
			}
					
			{
				std::string s;
				result->getField(8, s);
				ret->_Level = R2::TSessionLevel(s);
			}
					
			{
				std::string s;
				result->getField(9, s);
				ret->_RuleType = TRuleType(s);
			}
					
			{
				std::string s;
				result->getField(10, s);
				ret->_AccessType = TAccessType(s);
			}
					
			{
				std::string s;
				result->getField(11, s);
				ret->_State = TSessionState(s);
			}
					
			result->getField(12, ret->_HostShardId);
					
			result->getField(13, ret->_SubscriptionSlots);
					
			result->getField(14, ret->_ReservedSlots);
					
			{
				std::string s;
				result->getField(15, s);
				ret->_EstimatedDuration = TEstimatedDuration(s);
			}
					
			result->getField(16, ret->_FinalDuration);
					
			result->getField(17, ret->_FolderId);
					
			result->getField(18, ret->_Lang);
					
			result->getField(19, ret->_Icone);
					
			{
				std::string s;
				result->getField(20, s);
				ret->_AnimMode = TAnimMode(s);
			}
					
			{
				std::string s;
				result->getField(21, s);
				ret->_RaceFilter = TRaceFilter(s);
			}
					
			{
				std::string s;
				result->getField(22, s);
				ret->_ReligionFilter = TReligionFilter(s);
			}
					
			{
				std::string s;
				result->getField(23, s);
				ret->_GuildFilter = TGuildFilter(s);
			}
					
			{
				std::string s;
				result->getField(24, s);
				ret->_ShardFilter = TShardFilter(s);
			}
					
			{
				std::string s;
				result->getField(25, s);
				ret->_LevelFilter = TLevelFilter(s);
			}
					
			result->getField(26, ret->_SubscriptionClosed);
					
			result->getField(27, ret->_Newcomer);
					CSession *inCache = loadFromCache(ret->_SessionId, true);
			if (inCache != NULL)
			{

				container.push_back(CSessionPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CSessionPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	bool CSession::loadChildrenOfCFolder(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "session_id, session_type, title, owner, plan_date, start_date, description, orientation, level, rule_type, access_type, state, host_shard_id, subscription_slots, reserved_slots, estimated_duration, final_duration, folder_id, lang, icone, anim_mode, race_filter, religion_filter, guild_filter, shard_filter, level_filter, subscription_closed, newcomer";

		qs += " FROM sessions";
		qs += " WHERE folder_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CSession *ret = new CSession();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_SessionId);
					
			{
				std::string s;
				result->getField(1, s);
				ret->_SessionType = TSessionType(s);
			}
					
			result->getField(2, ret->_Title);
					
			result->getField(3, ret->_OwnerId);
					
			result->getDateField(4, ret->_PlanDate);
					
			result->getDateField(5, ret->_StartDate);
					
			result->getField(6, ret->_Description);
					
			{
				std::string s;
				result->getField(7, s);
				ret->_Orientation = TSessionOrientation(s);
			}
					
			{
				std::string s;
				result->getField(8, s);
				ret->_Level = R2::TSessionLevel(s);
			}
					
			{
				std::string s;
				result->getField(9, s);
				ret->_RuleType = TRuleType(s);
			}
					
			{
				std::string s;
				result->getField(10, s);
				ret->_AccessType = TAccessType(s);
			}
					
			{
				std::string s;
				result->getField(11, s);
				ret->_State = TSessionState(s);
			}
					
			result->getField(12, ret->_HostShardId);
					
			result->getField(13, ret->_SubscriptionSlots);
					
			result->getField(14, ret->_ReservedSlots);
					
			{
				std::string s;
				result->getField(15, s);
				ret->_EstimatedDuration = TEstimatedDuration(s);
			}
					
			result->getField(16, ret->_FinalDuration);
					
			result->getField(17, ret->_FolderId);
					
			result->getField(18, ret->_Lang);
					
			result->getField(19, ret->_Icone);
					
			{
				std::string s;
				result->getField(20, s);
				ret->_AnimMode = TAnimMode(s);
			}
					
			{
				std::string s;
				result->getField(21, s);
				ret->_RaceFilter = TRaceFilter(s);
			}
					
			{
				std::string s;
				result->getField(22, s);
				ret->_ReligionFilter = TReligionFilter(s);
			}
					
			{
				std::string s;
				result->getField(23, s);
				ret->_GuildFilter = TGuildFilter(s);
			}
					
			{
				std::string s;
				result->getField(24, s);
				ret->_ShardFilter = TShardFilter(s);
			}
					
			{
				std::string s;
				result->getField(25, s);
				ret->_LevelFilter = TLevelFilter(s);
			}
					
			result->getField(26, ret->_SubscriptionClosed);
					
			result->getField(27, ret->_Newcomer);
					CSession *inCache = loadFromCache(ret->_SessionId, true);
			if (inCache != NULL)
			{

				container.push_back(CSessionPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CSessionPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	bool CSession::loadSessionParticipants(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_SessionParticipants != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_SessionParticipants = new std::vector < CSessionParticipantPtr >;
		
		// load the childs
		ret &= CSessionParticipant::loadChildrenOfCSession(connection, getObjectId(), *_SessionParticipants, filename, lineNum);
		return ret;
	}


	const std::vector<CSessionParticipantPtr> &CSession::getSessionParticipants() const
	{
		nlassert(_SessionParticipants != NULL);
		return *_SessionParticipants;
	}

	CSessionParticipantPtr &CSession::getSessionParticipantsByIndex(uint32 index) const
	{
		nlassert(_SessionParticipants != NULL);
		nlassert(index < _SessionParticipants->size());
		return const_cast< CSessionParticipantPtr & >(_SessionParticipants->operator[](index));
	}
	
	CSessionParticipantPtr &CSession::getSessionParticipantsById(uint32 id) const
	{
		nlassert(_SessionParticipants != NULL);
		std::vector<CSessionParticipantPtr >::const_iterator first(_SessionParticipants->begin()), last(_SessionParticipants->end());
		for (; first != last; ++first)
		{
			const CSessionParticipantPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CSessionParticipantPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CSessionParticipantPtr nil;

		return nil;
	}

	
	bool CSession::loadGuildInvites(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_GuildInvites != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_GuildInvites = new std::vector < CGuildInvitePtr >;
		
		// load the childs
		ret &= CGuildInvite::loadChildrenOfCSession(connection, getObjectId(), *_GuildInvites, filename, lineNum);
		return ret;
	}


	const std::vector<CGuildInvitePtr> &CSession::getGuildInvites() const
	{
		nlassert(_GuildInvites != NULL);
		return *_GuildInvites;
	}

	CGuildInvitePtr &CSession::getGuildInvitesByIndex(uint32 index) const
	{
		nlassert(_GuildInvites != NULL);
		nlassert(index < _GuildInvites->size());
		return const_cast< CGuildInvitePtr & >(_GuildInvites->operator[](index));
	}
	
	CGuildInvitePtr &CSession::getGuildInvitesById(uint32 id) const
	{
		nlassert(_GuildInvites != NULL);
		std::vector<CGuildInvitePtr >::const_iterator first(_GuildInvites->begin()), last(_GuildInvites->end());
		for (; first != last; ++first)
		{
			const CGuildInvitePtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CGuildInvitePtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CGuildInvitePtr nil;

		return nil;
	}

	
	bool CSession::loadJournalEntries(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_JournalEntries != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_JournalEntries = new std::vector < CJournalEntryPtr >;
		
		// load the childs
		ret &= CJournalEntry::loadChildrenOfCSession(connection, getObjectId(), *_JournalEntries, filename, lineNum);
		return ret;
	}


	const std::vector<CJournalEntryPtr> &CSession::getJournalEntries() const
	{
		nlassert(_JournalEntries != NULL);
		return *_JournalEntries;
	}

	CJournalEntryPtr &CSession::getJournalEntriesByIndex(uint32 index) const
	{
		nlassert(_JournalEntries != NULL);
		nlassert(index < _JournalEntries->size());
		return const_cast< CJournalEntryPtr & >(_JournalEntries->operator[](index));
	}
	
	CJournalEntryPtr &CSession::getJournalEntriesById(uint32 id) const
	{
		nlassert(_JournalEntries != NULL);
		std::vector<CJournalEntryPtr >::const_iterator first(_JournalEntries->begin()), last(_JournalEntries->end());
		for (; first != last; ++first)
		{
			const CJournalEntryPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CJournalEntryPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CJournalEntryPtr nil;

		return nil;
	}

	
	void CShardPtr::linkPtr()
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

	void CShardPtr::unlinkPtr()
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


	CShard::TObjectCache		CShard::_ObjectCache;
	CShard::TReleasedObject	CShard::_ReleasedObject;


	// Destructor, delete any children
	CShard::~CShard()
	{
		// release childs reference
			if (_Guilds != NULL)
						delete _Guilds;


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CShardPtr *ptr = _PtrList;
			do 
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_ShardId != NOPE::INVALID_OBJECT_ID 
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CShard @%p from cache with id %u", this, static_cast<uint32>(_ShardId));
			nlverify(_ObjectCache.erase(_ShardId) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_ShardId) == _ObjectCache.end());
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

	void CShard::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CShard::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_ShardId != 0);
		std::string qs;
		qs = "INSERT INTO shard (";
		
		qs += "shard_id, WSOnline, RequiredState, MOTD";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_ShardId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_WSOnline), connection)+"'";
		qs += ", ";
		qs += "'"+_RequiredState.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_MOTD), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			return true;
		}

		return false;
	}

	bool CShard::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE shard SET ";
		
		qs += "shard_id = '"+MSW::escapeString(NLMISC::toString(_ShardId), connection)+"'";
		qs += ", ";
		qs += "WSOnline = '"+MSW::escapeString(NLMISC::toString(_WSOnline), connection)+"'";
		qs += ", ";
		qs += "RequiredState = '"+_RequiredState.toString()+"'";
		qs += ", ";
		qs += "MOTD = '"+MSW::escapeString(NLMISC::toString(_MOTD), connection)+"'";

		qs += " WHERE shard_id = '"+NLMISC::toString(_ShardId)+"'";
	

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

	bool CShard::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM shard ";
		
		qs += " WHERE shard_id = '"+NLMISC::toString(_ShardId)+"'";
	

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

	bool CShard::removeById(MSW::CConnection &connection, uint32 id)
	{
		CShard *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM shard ";
		
		qs += " WHERE shard_id = '"+NLMISC::toString(id)+"'";
	

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
	CShard *CShard::loadFromCache(uint32 objectId, bool unrelease)
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
			CShard *object = it->second;

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
	uint32 CShard::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CShard::dump()
	{
		nlinfo("  Cache info for class CShard :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CShard::updateCache()
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
				CShard *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CShard::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CShard *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CShard::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CShard::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CShard::setFirstPtr(CShardPtr *ptr)
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
	void CShard::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CShard @%p in cache with id %u", this, static_cast<uint32>(_ShardId));
			nlverify(_ObjectCache.insert(std::make_pair(_ShardId, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_ShardId) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CShard @%p in cache with id %u", this, static_cast<uint32>(_ShardId));
			nlverify(_ObjectCache.erase(_ShardId) == 1);
		}
	}


	CShardPtr CShard::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CShard *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CShardPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "shard_id, WSOnline, RequiredState, MOTD";

		qs += " FROM shard";
		
		qs += " WHERE shard_id = '"+NLMISC::toString(id)+"'";
	CShardPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CShard, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_ShardId);
			result->getField(1, ret->_WSOnline);
			{
				std::string s;
				result->getField(2, s);
				ret->_RequiredState = TAccessLevel(s);
			}
			result->getField(3, ret->_MOTD);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CShard::loadGuilds(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_Guilds != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_Guilds = new std::map < uint32,  CGuildPtr >;
		
		// load the childs
		ret &= CGuild::loadChildrenOfCShard(connection, getObjectId(), *_Guilds, filename, lineNum);
		return ret;
	}


	const std::map<uint32, CGuildPtr> &CShard::getGuilds() const
	{
		nlassert(_Guilds != NULL);
		return *_Guilds;
	}

	CGuildPtr &CShard::getGuildsById(uint32 id) const
	{
		nlassert(_Guilds != NULL);
		std::map<uint32, CGuildPtr>::const_iterator it(_Guilds->find(id));

		if (it == _Guilds->end())
		{
			// no object with this id, return a null pointer
			static CGuildPtr nil;
			return nil;
		}

		return const_cast< CGuildPtr & >(it->second);
	}

	
	void CGuildPtr::linkPtr()
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

	void CGuildPtr::unlinkPtr()
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


	CGuild::TObjectCache		CGuild::_ObjectCache;
	CGuild::TReleasedObject	CGuild::_ReleasedObject;


	// Destructor, delete any children
	CGuild::~CGuild()
	{
		// release childs reference
			if (_Characters != NULL)
						delete _Characters;
			if (_Invites != NULL)
						delete _Invites;


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CGuildPtr *ptr = _PtrList;
			do 
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_GuildId != NOPE::INVALID_OBJECT_ID 
			&& _ObjectState != NOPE::os_removed
			&& _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing CGuild @%p from cache with id %u", this, static_cast<uint32>(_GuildId));
			nlverify(_ObjectCache.erase(_GuildId) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_GuildId) == _ObjectCache.end());
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

	void CGuild::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CGuild::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_GuildId != 0);
		std::string qs;
		qs = "INSERT INTO guilds (";
		
		qs += "guild_id, guild_name, shard_id";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_GuildId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_GuildName), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_ShardId), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_ShardId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CShard *parent = CShard::loadFromCache(_ShardId, false);
				if (parent && parent->_Guilds != NULL)
				{

						nlverify(parent->_Guilds->insert(std::make_pair(getObjectId(), CGuildPtr(this, __FILE__, __LINE__))).second);
 
				}
			}

			return true;
		}

		return false;
	}

	bool CGuild::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE guilds SET ";
		
		qs += "guild_id = '"+MSW::escapeString(NLMISC::toString(_GuildId), connection)+"'";
		qs += ", ";
		qs += "guild_name = '"+MSW::escapeString(NLMISC::toString(_GuildName), connection)+"'";
		qs += ", ";
		qs += "shard_id = '"+MSW::escapeString(NLMISC::toString(_ShardId), connection)+"'";

		qs += " WHERE guild_id = '"+NLMISC::toString(_GuildId)+"'";
	

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

	bool CGuild::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM guilds ";
		
		qs += " WHERE guild_id = '"+NLMISC::toString(_GuildId)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{

				{
					// cascading deletion for vector child Invites
					nlassert(loadInvites(connection, __FILE__, __LINE__));

					const std::vector < CGuildInvitePtr > & childs = getInvites();

					while (!childs.empty())
					{
						getInvitesByIndex((uint32)childs.size()-1)->remove(connection);
					}
				}


				{
					// unreference (and update) for vector child Characters
					nlassert(loadCharacters(connection, __FILE__, __LINE__));

					const std::vector < CCharacterPtr > & childs = getCharacters();

					for (uint i=0; i < childs.size(); ++i)
					{
						
						getCharactersByIndex(i)->setGuildId(0);
						getCharactersByIndex(i)->update(connection);
					}
				}


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CShardPtr parent(CShard::loadFromCache(_ShardId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_Guilds != NULL)
					{

						parent->_Guilds->erase(getObjectId());
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CGuild::removeById(MSW::CConnection &connection, uint32 id)
	{
		CGuild *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM guilds ";
		
		qs += " WHERE guild_id = '"+NLMISC::toString(id)+"'";
	

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
	CGuild *CGuild::loadFromCache(uint32 objectId, bool unrelease)
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
			CGuild *object = it->second;

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
	uint32 CGuild::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CGuild::dump()
	{
		nlinfo("  Cache info for class CGuild :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CGuild::updateCache()
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
				CGuild *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CGuild::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CGuild *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CGuild::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CGuild::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CGuild::setFirstPtr(CGuildPtr *ptr)
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
	void CGuild::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CGuild @%p in cache with id %u", this, static_cast<uint32>(_GuildId));
			nlverify(_ObjectCache.insert(std::make_pair(_GuildId, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_GuildId) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing CGuild @%p in cache with id %u", this, static_cast<uint32>(_GuildId));
			nlverify(_ObjectCache.erase(_GuildId) == 1);
		}
	}


	CGuildPtr CGuild::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CGuild *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CGuildPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "guild_id, guild_name, shard_id";

		qs += " FROM guilds";
		
		qs += " WHERE guild_id = '"+NLMISC::toString(id)+"'";
	CGuildPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CGuild, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_GuildId);
			result->getField(1, ret->_GuildName);
			result->getField(2, ret->_ShardId);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CGuild::loadChildrenOfCShard(MSW::CConnection &connection, uint32 parentId, std::map < uint32, CGuildPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "guild_id, guild_name, shard_id";

		qs += " FROM guilds";
		qs += " WHERE shard_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CGuild *ret = new CGuild();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_GuildId);
					
			result->getField(1, ret->_GuildName);
					
			result->getField(2, ret->_ShardId);
					CGuild *inCache = loadFromCache(ret->_GuildId, true);
			if (inCache != NULL)
			{

				container.insert(std::make_pair(inCache->getObjectId(), CGuildPtr(inCache, filename, lineNum)));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.insert(std::make_pair(ret->getObjectId(), CGuildPtr(ret, filename, lineNum)));

			}
		}

		return true;
	}

	bool CGuild::loadCharacters(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_Characters != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_Characters = new std::vector < CCharacterPtr >;
		
		// load the childs
		ret &= CCharacter::loadChildrenOfCGuild(connection, getObjectId(), *_Characters, filename, lineNum);
		return ret;
	}


	const std::vector<CCharacterPtr> &CGuild::getCharacters() const
	{
		nlassert(_Characters != NULL);
		return *_Characters;
	}

	CCharacterPtr &CGuild::getCharactersByIndex(uint32 index) const
	{
		nlassert(_Characters != NULL);
		nlassert(index < _Characters->size());
		return const_cast< CCharacterPtr & >(_Characters->operator[](index));
	}
	
	CCharacterPtr &CGuild::getCharactersById(uint32 id) const
	{
		nlassert(_Characters != NULL);
		std::vector<CCharacterPtr >::const_iterator first(_Characters->begin()), last(_Characters->end());
		for (; first != last; ++first)
		{
			const CCharacterPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CCharacterPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CCharacterPtr nil;

		return nil;
	}

	
	bool CGuild::loadInvites(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_Invites != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_Invites = new std::vector < CGuildInvitePtr >;
		
		// load the childs
		ret &= CGuildInvite::loadChildrenOfCGuild(connection, getObjectId(), *_Invites, filename, lineNum);
		return ret;
	}


	const std::vector<CGuildInvitePtr> &CGuild::getInvites() const
	{
		nlassert(_Invites != NULL);
		return *_Invites;
	}

	CGuildInvitePtr &CGuild::getInvitesByIndex(uint32 index) const
	{
		nlassert(_Invites != NULL);
		nlassert(index < _Invites->size());
		return const_cast< CGuildInvitePtr & >(_Invites->operator[](index));
	}
	
	CGuildInvitePtr &CGuild::getInvitesById(uint32 id) const
	{
		nlassert(_Invites != NULL);
		std::vector<CGuildInvitePtr >::const_iterator first(_Invites->begin()), last(_Invites->end());
		for (; first != last; ++first)
		{
			const CGuildInvitePtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CGuildInvitePtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CGuildInvitePtr nil;

		return nil;
	}

	
	void CGuildInvitePtr::linkPtr()
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

	void CGuildInvitePtr::unlinkPtr()
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


	CGuildInvite::TObjectCache		CGuildInvite::_ObjectCache;
	CGuildInvite::TReleasedObject	CGuildInvite::_ReleasedObject;


	// Destructor, delete any children
	CGuildInvite::~CGuildInvite()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CGuildInvitePtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CGuildInvite @%p from cache with id %u", this, static_cast<uint32>(_Id));
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

	void CGuildInvite::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CGuildInvite::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO guild_invites (";
		
		qs += "guild_id, session_id";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_GuildId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_SessionId), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{
			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_GuildId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CGuild *parent = CGuild::loadFromCache(_GuildId, false);
				if (parent && parent->_Invites != NULL)
				{

						nlassert(std::find(parent->_Invites->begin(), parent->_Invites->end(), CGuildInvitePtr(this, __FILE__, __LINE__)) == parent->_Invites->end());
						parent->_Invites->push_back(CGuildInvitePtr(this, __FILE__, __LINE__));
 
				}
			}

			if (_SessionId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CSession *parent = CSession::loadFromCache(_SessionId, false);
				if (parent && parent->_GuildInvites != NULL)
				{

						nlassert(std::find(parent->_GuildInvites->begin(), parent->_GuildInvites->end(), CGuildInvitePtr(this, __FILE__, __LINE__)) == parent->_GuildInvites->end());
						parent->_GuildInvites->push_back(CGuildInvitePtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CGuildInvite::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE guild_invites SET ";
		
		qs += "guild_id = '"+MSW::escapeString(NLMISC::toString(_GuildId), connection)+"'";
		qs += ", ";
		qs += "session_id = '"+MSW::escapeString(NLMISC::toString(_SessionId), connection)+"'";

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

	bool CGuildInvite::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM guild_invites ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CGuildPtr parent(CGuild::loadFromCache(_GuildId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_Invites != NULL)
					{

						std::vector < CGuildInvitePtr >::iterator it = std::find(parent->_Invites->begin(), parent->_Invites->end(), this);
						if (it != parent->_Invites->end())
						{
							parent->_Invites->erase(it);
						}
 
					}
				}
				
				{
					CSessionPtr parent(CSession::loadFromCache(_SessionId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_GuildInvites != NULL)
					{

						std::vector < CGuildInvitePtr >::iterator it = std::find(parent->_GuildInvites->begin(), parent->_GuildInvites->end(), this);
						if (it != parent->_GuildInvites->end())
						{
							parent->_GuildInvites->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CGuildInvite::removeById(MSW::CConnection &connection, uint32 id)
	{
		CGuildInvite *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM guild_invites ";
		
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
	CGuildInvite *CGuildInvite::loadFromCache(uint32 objectId, bool unrelease)
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
			CGuildInvite *object = it->second;

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
	uint32 CGuildInvite::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CGuildInvite::dump()
	{
		nlinfo("  Cache info for class CGuildInvite :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CGuildInvite::updateCache()
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
				CGuildInvite *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CGuildInvite::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CGuildInvite *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CGuildInvite::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CGuildInvite::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CGuildInvite::setFirstPtr(CGuildInvitePtr *ptr)
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
	void CGuildInvite::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CGuildInvite @%p in cache with id %u", this, static_cast<uint32>(_Id));
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
			nldebug("NOPE: erasing CGuildInvite @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CGuildInvitePtr CGuildInvite::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CGuildInvite *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CGuildInvitePtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id, guild_id, session_id";

		qs += " FROM guild_invites";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CGuildInvitePtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CGuildInvite, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getField(1, ret->_GuildId);
			result->getField(2, ret->_SessionId);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CGuildInvite::loadChildrenOfCGuild(MSW::CConnection &connection, uint32 parentId, std::vector < CGuildInvitePtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, guild_id, session_id";

		qs += " FROM guild_invites";
		qs += " WHERE guild_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CGuildInvite *ret = new CGuildInvite();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_GuildId);
					
			result->getField(2, ret->_SessionId);
					CGuildInvite *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CGuildInvitePtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CGuildInvitePtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	bool CGuildInvite::loadChildrenOfCSession(MSW::CConnection &connection, uint32 parentId, std::vector < CGuildInvitePtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, guild_id, session_id";

		qs += " FROM guild_invites";
		qs += " WHERE session_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CGuildInvite *ret = new CGuildInvite();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_GuildId);
					
			result->getField(2, ret->_SessionId);
					CGuildInvite *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CGuildInvitePtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CGuildInvitePtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	void CPlayerRatingPtr::linkPtr()
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

	void CPlayerRatingPtr::unlinkPtr()
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


	CPlayerRating::TObjectCache		CPlayerRating::_ObjectCache;
	CPlayerRating::TReleasedObject	CPlayerRating::_ReleasedObject;


	// Destructor, delete any children
	CPlayerRating::~CPlayerRating()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CPlayerRatingPtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CPlayerRating @%p from cache with id %u", this, static_cast<uint32>(_Id));
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

	void CPlayerRating::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CPlayerRating::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO player_rating (";
		
		qs += "scenario_id, author, rate_fun, rate_difficulty, rate_accessibility, rate_originality, rate_direction";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_ScenarioId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Author), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RateFun), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RateDifficulty), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RateAccessibility), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RateOriginality), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RateDirection), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{
			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_ScenarioId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CScenario *parent = CScenario::loadFromCache(_ScenarioId, false);
				if (parent && parent->_PlayerRatings != NULL)
				{

						nlassert(std::find(parent->_PlayerRatings->begin(), parent->_PlayerRatings->end(), CPlayerRatingPtr(this, __FILE__, __LINE__)) == parent->_PlayerRatings->end());
						parent->_PlayerRatings->push_back(CPlayerRatingPtr(this, __FILE__, __LINE__));
 
				}
			}

			if (_Author != 0)
			{
				// need to update the parent class child list if it is in the cache
				CCharacter *parent = CCharacter::loadFromCache(_Author, false);
				if (parent && parent->_PlayerRatings != NULL)
				{

						nlassert(std::find(parent->_PlayerRatings->begin(), parent->_PlayerRatings->end(), CPlayerRatingPtr(this, __FILE__, __LINE__)) == parent->_PlayerRatings->end());
						parent->_PlayerRatings->push_back(CPlayerRatingPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CPlayerRating::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE player_rating SET ";
		
		qs += "scenario_id = '"+MSW::escapeString(NLMISC::toString(_ScenarioId), connection)+"'";
		qs += ", ";
		qs += "author = '"+MSW::escapeString(NLMISC::toString(_Author), connection)+"'";
		qs += ", ";
		qs += "rate_fun = '"+MSW::escapeString(NLMISC::toString(_RateFun), connection)+"'";
		qs += ", ";
		qs += "rate_difficulty = '"+MSW::escapeString(NLMISC::toString(_RateDifficulty), connection)+"'";
		qs += ", ";
		qs += "rate_accessibility = '"+MSW::escapeString(NLMISC::toString(_RateAccessibility), connection)+"'";
		qs += ", ";
		qs += "rate_originality = '"+MSW::escapeString(NLMISC::toString(_RateOriginality), connection)+"'";
		qs += ", ";
		qs += "rate_direction = '"+MSW::escapeString(NLMISC::toString(_RateDirection), connection)+"'";

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

	bool CPlayerRating::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM player_rating ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CScenarioPtr parent(CScenario::loadFromCache(_ScenarioId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_PlayerRatings != NULL)
					{

						std::vector < CPlayerRatingPtr >::iterator it = std::find(parent->_PlayerRatings->begin(), parent->_PlayerRatings->end(), this);
						if (it != parent->_PlayerRatings->end())
						{
							parent->_PlayerRatings->erase(it);
						}
 
					}
				}
				
				{
					CCharacterPtr parent(CCharacter::loadFromCache(_Author, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_PlayerRatings != NULL)
					{

						std::vector < CPlayerRatingPtr >::iterator it = std::find(parent->_PlayerRatings->begin(), parent->_PlayerRatings->end(), this);
						if (it != parent->_PlayerRatings->end())
						{
							parent->_PlayerRatings->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CPlayerRating::removeById(MSW::CConnection &connection, uint32 id)
	{
		CPlayerRating *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM player_rating ";
		
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
	CPlayerRating *CPlayerRating::loadFromCache(uint32 objectId, bool unrelease)
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
			CPlayerRating *object = it->second;

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
	uint32 CPlayerRating::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CPlayerRating::dump()
	{
		nlinfo("  Cache info for class CPlayerRating :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CPlayerRating::updateCache()
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
				CPlayerRating *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CPlayerRating::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CPlayerRating *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CPlayerRating::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CPlayerRating::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CPlayerRating::setFirstPtr(CPlayerRatingPtr *ptr)
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
	void CPlayerRating::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CPlayerRating @%p in cache with id %u", this, static_cast<uint32>(_Id));
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
			nldebug("NOPE: erasing CPlayerRating @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CPlayerRatingPtr CPlayerRating::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CPlayerRating *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CPlayerRatingPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id, scenario_id, author, rate_fun, rate_difficulty, rate_accessibility, rate_originality, rate_direction";

		qs += " FROM player_rating";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CPlayerRatingPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CPlayerRating, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getField(1, ret->_ScenarioId);
			result->getField(2, ret->_Author);
			result->getField(3, ret->_RateFun);
			result->getField(4, ret->_RateDifficulty);
			result->getField(5, ret->_RateAccessibility);
			result->getField(6, ret->_RateOriginality);
			result->getField(7, ret->_RateDirection);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CPlayerRating::loadChildrenOfCScenario(MSW::CConnection &connection, uint32 parentId, std::vector < CPlayerRatingPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, scenario_id, author, rate_fun, rate_difficulty, rate_accessibility, rate_originality, rate_direction";

		qs += " FROM player_rating";
		qs += " WHERE scenario_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CPlayerRating *ret = new CPlayerRating();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_ScenarioId);
					
			result->getField(2, ret->_Author);
					
			result->getField(3, ret->_RateFun);
					
			result->getField(4, ret->_RateDifficulty);
					
			result->getField(5, ret->_RateAccessibility);
					
			result->getField(6, ret->_RateOriginality);
					
			result->getField(7, ret->_RateDirection);
					CPlayerRating *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CPlayerRatingPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CPlayerRatingPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	bool CPlayerRating::loadChildrenOfCCharacter(MSW::CConnection &connection, uint32 parentId, std::vector < CPlayerRatingPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, scenario_id, author, rate_fun, rate_difficulty, rate_accessibility, rate_originality, rate_direction";

		qs += " FROM player_rating";
		qs += " WHERE author = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CPlayerRating *ret = new CPlayerRating();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_ScenarioId);
					
			result->getField(2, ret->_Author);
					
			result->getField(3, ret->_RateFun);
					
			result->getField(4, ret->_RateDifficulty);
					
			result->getField(5, ret->_RateAccessibility);
					
			result->getField(6, ret->_RateOriginality);
					
			result->getField(7, ret->_RateDirection);
					CPlayerRating *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CPlayerRatingPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CPlayerRatingPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	void CJournalEntryPtr::linkPtr()
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

	void CJournalEntryPtr::unlinkPtr()
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


	CJournalEntry::TObjectCache		CJournalEntry::_ObjectCache;
	CJournalEntry::TReleasedObject	CJournalEntry::_ReleasedObject;


	// Destructor, delete any children
	CJournalEntry::~CJournalEntry()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CJournalEntryPtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CJournalEntry @%p from cache with id %u", this, static_cast<uint32>(_Id));
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

	void CJournalEntry::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CJournalEntry::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO journal_entry (";
		
		qs += "session_id, author, type, text, time_stamp";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_SessionId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Author), connection)+"'";
		qs += ", ";
		qs += "'"+_Type.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Text), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::encodeDate(_TimeStamp)+"'";

		qs += ")";

		if (connection.query(qs))
		{
			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_SessionId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CSession *parent = CSession::loadFromCache(_SessionId, false);
				if (parent && parent->_JournalEntries != NULL)
				{

						nlassert(std::find(parent->_JournalEntries->begin(), parent->_JournalEntries->end(), CJournalEntryPtr(this, __FILE__, __LINE__)) == parent->_JournalEntries->end());
						parent->_JournalEntries->push_back(CJournalEntryPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CJournalEntry::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE journal_entry SET ";
		
		qs += "session_id = '"+MSW::escapeString(NLMISC::toString(_SessionId), connection)+"'";
		qs += ", ";
		qs += "author = '"+MSW::escapeString(NLMISC::toString(_Author), connection)+"'";
		qs += ", ";
		qs += "type = '"+_Type.toString()+"'";
		qs += ", ";
		qs += "text = '"+MSW::escapeString(NLMISC::toString(_Text), connection)+"'";
		qs += ", ";
		qs += "time_stamp = '"+MSW::encodeDate(_TimeStamp)+"'";

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

	bool CJournalEntry::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM journal_entry ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CSessionPtr parent(CSession::loadFromCache(_SessionId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_JournalEntries != NULL)
					{

						std::vector < CJournalEntryPtr >::iterator it = std::find(parent->_JournalEntries->begin(), parent->_JournalEntries->end(), this);
						if (it != parent->_JournalEntries->end())
						{
							parent->_JournalEntries->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CJournalEntry::removeById(MSW::CConnection &connection, uint32 id)
	{
		CJournalEntry *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM journal_entry ";
		
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
	CJournalEntry *CJournalEntry::loadFromCache(uint32 objectId, bool unrelease)
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
			CJournalEntry *object = it->second;

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
	uint32 CJournalEntry::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CJournalEntry::dump()
	{
		nlinfo("  Cache info for class CJournalEntry :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CJournalEntry::updateCache()
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
				CJournalEntry *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CJournalEntry::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CJournalEntry *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CJournalEntry::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CJournalEntry::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CJournalEntry::setFirstPtr(CJournalEntryPtr *ptr)
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
	void CJournalEntry::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CJournalEntry @%p in cache with id %u", this, static_cast<uint32>(_Id));
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
			nldebug("NOPE: erasing CJournalEntry @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CJournalEntryPtr CJournalEntry::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CJournalEntry *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CJournalEntryPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id, session_id, author, type, text, time_stamp";

		qs += " FROM journal_entry";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CJournalEntryPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CJournalEntry, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getField(1, ret->_SessionId);
			result->getField(2, ret->_Author);
			{
				std::string s;
				result->getField(3, s);
				ret->_Type = TJournalEntryType(s);
			}
			result->getField(4, ret->_Text);
			result->getDateField(5, ret->_TimeStamp);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CJournalEntry::loadChildrenOfCSession(MSW::CConnection &connection, uint32 parentId, std::vector < CJournalEntryPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, session_id, author, type, text, time_stamp";

		qs += " FROM journal_entry";
		qs += " WHERE session_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CJournalEntry *ret = new CJournalEntry();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_SessionId);
					
			result->getField(2, ret->_Author);
					
			{
				std::string s;
				result->getField(3, s);
				ret->_Type = TJournalEntryType(s);
			}
					
			result->getField(4, ret->_Text);
					
			result->getDateField(5, ret->_TimeStamp);
					CJournalEntry *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CJournalEntryPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CJournalEntryPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	void CFolderPtr::linkPtr()
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

	void CFolderPtr::unlinkPtr()
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


	CFolder::TObjectCache		CFolder::_ObjectCache;
	CFolder::TReleasedObject	CFolder::_ReleasedObject;


	// Destructor, delete any children
	CFolder::~CFolder()
	{
		// release childs reference
			if (_FolderAccess != NULL)
						delete _FolderAccess;
			if (_Sessions != NULL)
						delete _Sessions;


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CFolderPtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CFolder @%p from cache with id %u", this, static_cast<uint32>(_Id));
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

	void CFolder::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CFolder::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO folder (";
		
		qs += "author, title, comments";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_Author), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Title), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Comments), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{
			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_Author != 0)
			{
				// need to update the parent class child list if it is in the cache
				CRingUser *parent = CRingUser::loadFromCache(_Author, false);
				if (parent && parent->_Folders != NULL)
				{

						nlassert(std::find(parent->_Folders->begin(), parent->_Folders->end(), CFolderPtr(this, __FILE__, __LINE__)) == parent->_Folders->end());
						parent->_Folders->push_back(CFolderPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CFolder::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE folder SET ";
		
		qs += "author = '"+MSW::escapeString(NLMISC::toString(_Author), connection)+"'";
		qs += ", ";
		qs += "title = '"+MSW::escapeString(NLMISC::toString(_Title), connection)+"'";
		qs += ", ";
		qs += "comments = '"+MSW::escapeString(NLMISC::toString(_Comments), connection)+"'";

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

	bool CFolder::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM folder ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{

				{
					// cascading deletion for vector child FolderAccess
					nlassert(loadFolderAccess(connection, __FILE__, __LINE__));

					const std::vector < CFolderAccessPtr > & childs = getFolderAccess();

					while (!childs.empty())
					{
						getFolderAccessByIndex((uint32)childs.size()-1)->remove(connection);
					}
				}


				{
					// unreference (and update) for vector child Sessions
					nlassert(loadSessions(connection, __FILE__, __LINE__));

					const std::vector < CSessionPtr > & childs = getSessions();

					for (uint i=0; i < childs.size(); ++i)
					{
						
						getSessionsByIndex(i)->setFolderId(0);
						getSessionsByIndex(i)->update(connection);
					}
				}


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CRingUserPtr parent(CRingUser::loadFromCache(_Author, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_Folders != NULL)
					{

						std::vector < CFolderPtr >::iterator it = std::find(parent->_Folders->begin(), parent->_Folders->end(), this);
						if (it != parent->_Folders->end())
						{
							parent->_Folders->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CFolder::removeById(MSW::CConnection &connection, uint32 id)
	{
		CFolder *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM folder ";
		
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
	CFolder *CFolder::loadFromCache(uint32 objectId, bool unrelease)
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
			CFolder *object = it->second;

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
	uint32 CFolder::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CFolder::dump()
	{
		nlinfo("  Cache info for class CFolder :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CFolder::updateCache()
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
				CFolder *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CFolder::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CFolder *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CFolder::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CFolder::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CFolder::setFirstPtr(CFolderPtr *ptr)
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
	void CFolder::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CFolder @%p in cache with id %u", this, static_cast<uint32>(_Id));
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
			nldebug("NOPE: erasing CFolder @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CFolderPtr CFolder::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CFolder *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CFolderPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id, author, title, comments";

		qs += " FROM folder";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CFolderPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CFolder, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getField(1, ret->_Author);
			result->getField(2, ret->_Title);
			result->getField(3, ret->_Comments);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CFolder::loadChildrenOfCRingUser(MSW::CConnection &connection, uint32 parentId, std::vector < CFolderPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, author, title, comments";

		qs += " FROM folder";
		qs += " WHERE author = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CFolder *ret = new CFolder();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_Author);
					
			result->getField(2, ret->_Title);
					
			result->getField(3, ret->_Comments);
					CFolder *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CFolderPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CFolderPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	bool CFolder::loadFolderAccess(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_FolderAccess != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_FolderAccess = new std::vector < CFolderAccessPtr >;
		
		// load the childs
		ret &= CFolderAccess::loadChildrenOfCFolder(connection, getObjectId(), *_FolderAccess, filename, lineNum);
		return ret;
	}


	const std::vector<CFolderAccessPtr> &CFolder::getFolderAccess() const
	{
		nlassert(_FolderAccess != NULL);
		return *_FolderAccess;
	}

	CFolderAccessPtr &CFolder::getFolderAccessByIndex(uint32 index) const
	{
		nlassert(_FolderAccess != NULL);
		nlassert(index < _FolderAccess->size());
		return const_cast< CFolderAccessPtr & >(_FolderAccess->operator[](index));
	}
	
	CFolderAccessPtr &CFolder::getFolderAccessById(uint32 id) const
	{
		nlassert(_FolderAccess != NULL);
		std::vector<CFolderAccessPtr >::const_iterator first(_FolderAccess->begin()), last(_FolderAccess->end());
		for (; first != last; ++first)
		{
			const CFolderAccessPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CFolderAccessPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CFolderAccessPtr nil;

		return nil;
	}

	
	bool CFolder::loadSessions(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_Sessions != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_Sessions = new std::vector < CSessionPtr >;
		
		// load the childs
		ret &= CSession::loadChildrenOfCFolder(connection, getObjectId(), *_Sessions, filename, lineNum);
		return ret;
	}


	const std::vector<CSessionPtr> &CFolder::getSessions() const
	{
		nlassert(_Sessions != NULL);
		return *_Sessions;
	}

	CSessionPtr &CFolder::getSessionsByIndex(uint32 index) const
	{
		nlassert(_Sessions != NULL);
		nlassert(index < _Sessions->size());
		return const_cast< CSessionPtr & >(_Sessions->operator[](index));
	}
	
	CSessionPtr &CFolder::getSessionsById(uint32 id) const
	{
		nlassert(_Sessions != NULL);
		std::vector<CSessionPtr >::const_iterator first(_Sessions->begin()), last(_Sessions->end());
		for (; first != last; ++first)
		{
			const CSessionPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CSessionPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CSessionPtr nil;

		return nil;
	}

	
	void CFolderAccessPtr::linkPtr()
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

	void CFolderAccessPtr::unlinkPtr()
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


	CFolderAccess::TObjectCache		CFolderAccess::_ObjectCache;
	CFolderAccess::TReleasedObject	CFolderAccess::_ReleasedObject;


	// Destructor, delete any children
	CFolderAccess::~CFolderAccess()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CFolderAccessPtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CFolderAccess @%p from cache with id %u", this, static_cast<uint32>(_Id));
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

	void CFolderAccess::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CFolderAccess::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO folder_access (";
		
		qs += "folder_id, user_id";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_FolderId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{
			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_UserId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CRingUser *parent = CRingUser::loadFromCache(_UserId, false);
				if (parent && parent->_FolderAccess != NULL)
				{

						nlassert(std::find(parent->_FolderAccess->begin(), parent->_FolderAccess->end(), CFolderAccessPtr(this, __FILE__, __LINE__)) == parent->_FolderAccess->end());
						parent->_FolderAccess->push_back(CFolderAccessPtr(this, __FILE__, __LINE__));
 
				}
			}

			if (_FolderId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CFolder *parent = CFolder::loadFromCache(_FolderId, false);
				if (parent && parent->_FolderAccess != NULL)
				{

						nlassert(std::find(parent->_FolderAccess->begin(), parent->_FolderAccess->end(), CFolderAccessPtr(this, __FILE__, __LINE__)) == parent->_FolderAccess->end());
						parent->_FolderAccess->push_back(CFolderAccessPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CFolderAccess::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE folder_access SET ";
		
		qs += "folder_id = '"+MSW::escapeString(NLMISC::toString(_FolderId), connection)+"'";
		qs += ", ";
		qs += "user_id = '"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";

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

	bool CFolderAccess::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM folder_access ";
		
		qs += " WHERE Id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CRingUserPtr parent(CRingUser::loadFromCache(_UserId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_FolderAccess != NULL)
					{

						std::vector < CFolderAccessPtr >::iterator it = std::find(parent->_FolderAccess->begin(), parent->_FolderAccess->end(), this);
						if (it != parent->_FolderAccess->end())
						{
							parent->_FolderAccess->erase(it);
						}
 
					}
				}
				
				{
					CFolderPtr parent(CFolder::loadFromCache(_FolderId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_FolderAccess != NULL)
					{

						std::vector < CFolderAccessPtr >::iterator it = std::find(parent->_FolderAccess->begin(), parent->_FolderAccess->end(), this);
						if (it != parent->_FolderAccess->end())
						{
							parent->_FolderAccess->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CFolderAccess::removeById(MSW::CConnection &connection, uint32 id)
	{
		CFolderAccess *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM folder_access ";
		
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
	CFolderAccess *CFolderAccess::loadFromCache(uint32 objectId, bool unrelease)
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
			CFolderAccess *object = it->second;

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
	uint32 CFolderAccess::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CFolderAccess::dump()
	{
		nlinfo("  Cache info for class CFolderAccess :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CFolderAccess::updateCache()
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
				CFolderAccess *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CFolderAccess::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CFolderAccess *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CFolderAccess::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CFolderAccess::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CFolderAccess::setFirstPtr(CFolderAccessPtr *ptr)
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
	void CFolderAccess::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CFolderAccess @%p in cache with id %u", this, static_cast<uint32>(_Id));
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
			nldebug("NOPE: erasing CFolderAccess @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CFolderAccessPtr CFolderAccess::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CFolderAccess *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CFolderAccessPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "Id, folder_id, user_id";

		qs += " FROM folder_access";
		
		qs += " WHERE Id = '"+NLMISC::toString(id)+"'";
	CFolderAccessPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CFolderAccess, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getField(1, ret->_FolderId);
			result->getField(2, ret->_UserId);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CFolderAccess::loadChildrenOfCRingUser(MSW::CConnection &connection, uint32 parentId, std::vector < CFolderAccessPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, folder_id, user_id";

		qs += " FROM folder_access";
		qs += " WHERE user_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CFolderAccess *ret = new CFolderAccess();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_FolderId);
					
			result->getField(2, ret->_UserId);
					CFolderAccess *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CFolderAccessPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CFolderAccessPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	bool CFolderAccess::loadChildrenOfCFolder(MSW::CConnection &connection, uint32 parentId, std::vector < CFolderAccessPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "Id, folder_id, user_id";

		qs += " FROM folder_access";
		qs += " WHERE folder_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CFolderAccess *ret = new CFolderAccess();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_FolderId);
					
			result->getField(2, ret->_UserId);
					CFolderAccess *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CFolderAccessPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CFolderAccessPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	void CScenarioPtr::linkPtr()
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

	void CScenarioPtr::unlinkPtr()
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


	CScenario::TObjectCache		CScenario::_ObjectCache;
	CScenario::TReleasedObject	CScenario::_ReleasedObject;


	// Destructor, delete any children
	CScenario::~CScenario()
	{
		// release childs reference
			if (_SessionLogs != NULL)
						delete _SessionLogs;
			if (_PlayerRatings != NULL)
						delete _PlayerRatings;


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CScenarioPtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CScenario @%p from cache with id %u", this, static_cast<uint32>(_Id));
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

	void CScenario::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CScenario::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		std::string qs;
		qs = "INSERT INTO scenario (";
		
		qs += "md5, title, description, author, rrp_total, anim_mode, language, orientation, level, allow_free_trial";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(_MD5.toString(), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Title), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Description), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Author), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RRPTotal), connection)+"'";
		qs += ", ";
		qs += "'"+_AnimMode.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Language), connection)+"'";
		qs += ", ";
		qs += "'"+_Orientation.toString()+"'";
		qs += ", ";
		qs += "'"+_Level.toString()+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_AllowFreeTrial), connection)+"'";

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

	bool CScenario::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE scenario SET ";
		
		qs += "md5 = '"+MSW::escapeString(_MD5.toString(), connection)+"'";
		qs += ", ";
		qs += "title = '"+MSW::escapeString(NLMISC::toString(_Title), connection)+"'";
		qs += ", ";
		qs += "description = '"+MSW::escapeString(NLMISC::toString(_Description), connection)+"'";
		qs += ", ";
		qs += "author = '"+MSW::escapeString(NLMISC::toString(_Author), connection)+"'";
		qs += ", ";
		qs += "rrp_total = '"+MSW::escapeString(NLMISC::toString(_RRPTotal), connection)+"'";
		qs += ", ";
		qs += "anim_mode = '"+_AnimMode.toString()+"'";
		qs += ", ";
		qs += "language = '"+MSW::escapeString(NLMISC::toString(_Language), connection)+"'";
		qs += ", ";
		qs += "orientation = '"+_Orientation.toString()+"'";
		qs += ", ";
		qs += "level = '"+_Level.toString()+"'";
		qs += ", ";
		qs += "allow_free_trial = '"+MSW::escapeString(NLMISC::toString(_AllowFreeTrial), connection)+"'";

		qs += " WHERE id = '"+NLMISC::toString(_Id)+"'";
	

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

	bool CScenario::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM scenario ";
		
		qs += " WHERE id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{

				{
					// cascading deletion for vector child SessionLogs
					nlassert(loadSessionLogs(connection, __FILE__, __LINE__));

					const std::vector < CSessionLogPtr > & childs = getSessionLogs();

					while (!childs.empty())
					{
						getSessionLogsByIndex((uint32)childs.size()-1)->remove(connection);
					}
				}


				{
					// cascading deletion for vector child PlayerRatings
					nlassert(loadPlayerRatings(connection, __FILE__, __LINE__));

					const std::vector < CPlayerRatingPtr > & childs = getPlayerRatings();

					while (!childs.empty())
					{
						getPlayerRatingsByIndex((uint32)childs.size()-1)->remove(connection);
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

	bool CScenario::removeById(MSW::CConnection &connection, uint32 id)
	{
		CScenario *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM scenario ";
		
		qs += " WHERE id = '"+NLMISC::toString(id)+"'";
	

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
	CScenario *CScenario::loadFromCache(uint32 objectId, bool unrelease)
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
			CScenario *object = it->second;

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
	uint32 CScenario::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CScenario::dump()
	{
		nlinfo("  Cache info for class CScenario :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CScenario::updateCache()
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
				CScenario *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CScenario::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CScenario *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CScenario::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CScenario::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CScenario::setFirstPtr(CScenarioPtr *ptr)
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
	void CScenario::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CScenario @%p in cache with id %u", this, static_cast<uint32>(_Id));
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
			nldebug("NOPE: erasing CScenario @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CScenarioPtr CScenario::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CScenario *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CScenarioPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "id, md5, title, description, author, rrp_total, anim_mode, language, orientation, level, allow_free_trial";

		qs += " FROM scenario";
		
		qs += " WHERE id = '"+NLMISC::toString(id)+"'";
	CScenarioPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CScenario, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getMD5Field(1, ret->_MD5);
			result->getField(2, ret->_Title);
			result->getField(3, ret->_Description);
			result->getField(4, ret->_Author);
			result->getField(5, ret->_RRPTotal);
			{
				std::string s;
				result->getField(6, s);
				ret->_AnimMode = TAnimMode(s);
			}
			result->getField(7, ret->_Language);
			{
				std::string s;
				result->getField(8, s);
				ret->_Orientation = TSessionOrientation(s);
			}
			{
				std::string s;
				result->getField(9, s);
				ret->_Level = R2::TSessionLevel(s);
			}
			result->getField(10, ret->_AllowFreeTrial);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CScenario::loadSessionLogs(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_SessionLogs != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_SessionLogs = new std::vector < CSessionLogPtr >;
		
		// load the childs
		ret &= CSessionLog::loadChildrenOfCScenario(connection, getObjectId(), *_SessionLogs, filename, lineNum);
		return ret;
	}


	const std::vector<CSessionLogPtr> &CScenario::getSessionLogs() const
	{
		nlassert(_SessionLogs != NULL);
		return *_SessionLogs;
	}

	CSessionLogPtr &CScenario::getSessionLogsByIndex(uint32 index) const
	{
		nlassert(_SessionLogs != NULL);
		nlassert(index < _SessionLogs->size());
		return const_cast< CSessionLogPtr & >(_SessionLogs->operator[](index));
	}
	
	CSessionLogPtr &CScenario::getSessionLogsById(uint32 id) const
	{
		nlassert(_SessionLogs != NULL);
		std::vector<CSessionLogPtr >::const_iterator first(_SessionLogs->begin()), last(_SessionLogs->end());
		for (; first != last; ++first)
		{
			const CSessionLogPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CSessionLogPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CSessionLogPtr nil;

		return nil;
	}

	
	bool CScenario::loadPlayerRatings(MSW::CConnection &connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_PlayerRatings != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
		_PlayerRatings = new std::vector < CPlayerRatingPtr >;
		
		// load the childs
		ret &= CPlayerRating::loadChildrenOfCScenario(connection, getObjectId(), *_PlayerRatings, filename, lineNum);
		return ret;
	}


	const std::vector<CPlayerRatingPtr> &CScenario::getPlayerRatings() const
	{
		nlassert(_PlayerRatings != NULL);
		return *_PlayerRatings;
	}

	CPlayerRatingPtr &CScenario::getPlayerRatingsByIndex(uint32 index) const
	{
		nlassert(_PlayerRatings != NULL);
		nlassert(index < _PlayerRatings->size());
		return const_cast< CPlayerRatingPtr & >(_PlayerRatings->operator[](index));
	}
	
	CPlayerRatingPtr &CScenario::getPlayerRatingsById(uint32 id) const
	{
		nlassert(_PlayerRatings != NULL);
		std::vector<CPlayerRatingPtr >::const_iterator first(_PlayerRatings->begin()), last(_PlayerRatings->end());
		for (; first != last; ++first)
		{
			const CPlayerRatingPtr &child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast< CPlayerRatingPtr & >(child);
			}
		}

		// no object with this id, return a null pointer
		static CPlayerRatingPtr nil;

		return nil;
	}

	
	void CSessionLogPtr::linkPtr()
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

	void CSessionLogPtr::unlinkPtr()
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


	CSessionLog::TObjectCache		CSessionLog::_ObjectCache;
	CSessionLog::TReleasedObject	CSessionLog::_ReleasedObject;


	// Destructor, delete any children
	CSessionLog::~CSessionLog()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CSessionLogPtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CSessionLog @%p from cache with id %u", this, static_cast<uint32>(_Id));
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

	void CSessionLog::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CSessionLog::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_Id != 0);
		std::string qs;
		qs = "INSERT INTO session_log (";
		
		qs += "id, scenario_id, rrp_scored, scenario_point_scored, time_taken, participants, launch_date, owner, guild_name";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_Id), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_ScenarioId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_RRPScored), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_ScenarioPointScored), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_TimeTaken), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Participants), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::encodeDate(_LaunchDate)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Owner), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_GuildName), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_ScenarioId != 0)
			{
				// need to update the parent class child list if it is in the cache
				CScenario *parent = CScenario::loadFromCache(_ScenarioId, false);
				if (parent && parent->_SessionLogs != NULL)
				{

						nlassert(std::find(parent->_SessionLogs->begin(), parent->_SessionLogs->end(), CSessionLogPtr(this, __FILE__, __LINE__)) == parent->_SessionLogs->end());
						parent->_SessionLogs->push_back(CSessionLogPtr(this, __FILE__, __LINE__));
 
				}
			}

			return true;
		}

		return false;
	}

	bool CSessionLog::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE session_log SET ";
		
		qs += "id = '"+MSW::escapeString(NLMISC::toString(_Id), connection)+"'";
		qs += ", ";
		qs += "scenario_id = '"+MSW::escapeString(NLMISC::toString(_ScenarioId), connection)+"'";
		qs += ", ";
		qs += "rrp_scored = '"+MSW::escapeString(NLMISC::toString(_RRPScored), connection)+"'";
		qs += ", ";
		qs += "scenario_point_scored = '"+MSW::escapeString(NLMISC::toString(_ScenarioPointScored), connection)+"'";
		qs += ", ";
		qs += "time_taken = '"+MSW::escapeString(NLMISC::toString(_TimeTaken), connection)+"'";
		qs += ", ";
		qs += "participants = '"+MSW::escapeString(NLMISC::toString(_Participants), connection)+"'";
		qs += ", ";
		qs += "launch_date = '"+MSW::encodeDate(_LaunchDate)+"'";
		qs += ", ";
		qs += "owner = '"+MSW::escapeString(NLMISC::toString(_Owner), connection)+"'";
		qs += ", ";
		qs += "guild_name = '"+MSW::escapeString(NLMISC::toString(_GuildName), connection)+"'";

		qs += " WHERE id = '"+NLMISC::toString(_Id)+"'";
	

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

	bool CSessionLog::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM session_log ";
		
		qs += " WHERE id = '"+NLMISC::toString(_Id)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				{
					CScenarioPtr parent(CScenario::loadFromCache(_ScenarioId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_SessionLogs != NULL)
					{

						std::vector < CSessionLogPtr >::iterator it = std::find(parent->_SessionLogs->begin(), parent->_SessionLogs->end(), this);
						if (it != parent->_SessionLogs->end())
						{
							parent->_SessionLogs->erase(it);
						}
 
					}
				}
				
				// need to remove ref from parent (if any)
				

				return true;
			}
		}
		return false;
	}

	bool CSessionLog::removeById(MSW::CConnection &connection, uint32 id)
	{
		CSessionLog *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM session_log ";
		
		qs += " WHERE id = '"+NLMISC::toString(id)+"'";
	

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
	CSessionLog *CSessionLog::loadFromCache(uint32 objectId, bool unrelease)
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
			CSessionLog *object = it->second;

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
	uint32 CSessionLog::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CSessionLog::dump()
	{
		nlinfo("  Cache info for class CSessionLog :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CSessionLog::updateCache()
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
				CSessionLog *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CSessionLog::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CSessionLog *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CSessionLog::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CSessionLog::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CSessionLog::setFirstPtr(CSessionLogPtr *ptr)
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
	void CSessionLog::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CSessionLog @%p in cache with id %u", this, static_cast<uint32>(_Id));
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
			nldebug("NOPE: erasing CSessionLog @%p in cache with id %u", this, static_cast<uint32>(_Id));
			nlverify(_ObjectCache.erase(_Id) == 1);
		}
	}


	CSessionLogPtr CSessionLog::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CSessionLog *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CSessionLogPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "id, scenario_id, rrp_scored, scenario_point_scored, time_taken, participants, launch_date, owner, guild_name";

		qs += " FROM session_log";
		
		qs += " WHERE id = '"+NLMISC::toString(id)+"'";
	CSessionLogPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CSessionLog, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_Id);
			result->getField(1, ret->_ScenarioId);
			result->getField(2, ret->_RRPScored);
			result->getField(3, ret->_ScenarioPointScored);
			result->getField(4, ret->_TimeTaken);
			result->getField(5, ret->_Participants);
			result->getDateField(6, ret->_LaunchDate);
			result->getField(7, ret->_Owner);
			result->getField(8, ret->_GuildName);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}


	bool CSessionLog::loadChildrenOfCScenario(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionLogPtr > & container, const char *filename, uint32 lineNum)

	{
		std::string qs;
		qs = "SELECT ";

		qs += "id, scenario_id, rrp_scored, scenario_point_scored, time_taken, participants, launch_date, owner, guild_name";

		qs += " FROM session_log";
		qs += " WHERE scenario_id = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			CSessionLog *ret = new CSessionLog();
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, ret->_Id);
					
			result->getField(1, ret->_ScenarioId);
					
			result->getField(2, ret->_RRPScored);
					
			result->getField(3, ret->_ScenarioPointScored);
					
			result->getField(4, ret->_TimeTaken);
					
			result->getField(5, ret->_Participants);
					
			result->getDateField(6, ret->_LaunchDate);
					
			result->getField(7, ret->_Owner);
					
			result->getField(8, ret->_GuildName);
					CSessionLog *inCache = loadFromCache(ret->_Id, true);
			if (inCache != NULL)
			{

				container.push_back(CSessionLogPtr(inCache, filename, lineNum));

				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);

				container.push_back(CSessionLogPtr(ret, filename, lineNum));

			}
		}

		return true;
	}

	void CGmStatusPtr::linkPtr()
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

	void CGmStatusPtr::unlinkPtr()
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


	CGmStatus::TObjectCache		CGmStatus::_ObjectCache;
	CGmStatus::TReleasedObject	CGmStatus::_ReleasedObject;


	// Destructor, delete any children
	CGmStatus::~CGmStatus()
	{
		// release childs reference


		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			CGmStatusPtr *ptr = _PtrList;
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
			nldebug("NOPE: clearing CGmStatus @%p from cache with id %u", this, static_cast<uint32>(_UserId));
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

	void CGmStatus::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool CGmStatus::create(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);

		nlassert(_UserId != 0);
		std::string qs;
		qs = "INSERT INTO gm_status (";
		
		qs += "user_id, available";
		qs += ") VALUES (";
		
		qs += "'"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";
		qs += ", ";
		qs += "'"+MSW::escapeString(NLMISC::toString(_Available), connection)+"'";

		qs += ")";

		if (connection.query(qs))
		{


			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any

			if (_UserId != 0)
			{
				// need to update the parent class child if it is in the cache
				CRingUser *parent = CRingUser::loadFromCache(_UserId, false);
				if (parent && parent->_GMStatusLoaded)
				{
					nlassert(parent->_GMStatus == NULL);
					parent->_GMStatus = CGmStatusPtr(this, __FILE__, __LINE__);
				}
			}

			return true;
		}

		return false;
	}

	bool CGmStatus::update(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE gm_status SET ";
		
		qs += "user_id = '"+MSW::escapeString(NLMISC::toString(_UserId), connection)+"'";
		qs += ", ";
		qs += "available = '"+MSW::escapeString(NLMISC::toString(_Available), connection)+"'";

		qs += " WHERE user_id = '"+NLMISC::toString(_UserId)+"'";
	

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

	bool CGmStatus::remove(MSW::CConnection &connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM gm_status ";
		
		qs += " WHERE user_id = '"+NLMISC::toString(_UserId)+"'";
	

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{


				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				
				// need to remove ref from parent (if any)
				
				{
					CRingUserPtr parent(CRingUser::loadFromCache(_UserId, true), __FILE__, __LINE__);
					if (parent != NULL && parent->_GMStatusLoaded)
					{
						// assign a new NULL pointer
						parent->_GMStatus.assign(CGmStatusPtr(), __FILE__, __LINE__);
					}
				}
				

				return true;
			}
		}
		return false;
	}

	bool CGmStatus::removeById(MSW::CConnection &connection, uint32 id)
	{
		CGmStatus *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM gm_status ";
		
		qs += " WHERE user_id = '"+NLMISC::toString(id)+"'";
	

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
	CGmStatus *CGmStatus::loadFromCache(uint32 objectId, bool unrelease)
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
			CGmStatus *object = it->second;

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
	uint32 CGmStatus::cacheCmd(NOPE::TCacheCmd cmd)
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
			return (uint32)_ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void CGmStatus::dump()
	{
		nlinfo("  Cache info for class CGmStatus :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += (uint32)first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void CGmStatus::updateCache()
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
				CGmStatus *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CGmStatus::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				CGmStatus *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void CGmStatus::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&CGmStatus::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void CGmStatus::setFirstPtr(CGmStatusPtr *ptr)
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
	void CGmStatus::setPersistentState(NOPE::TObjectState state)
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
			nldebug("NOPE: inserting CGmStatus @%p in cache with id %u", this, static_cast<uint32>(_UserId));
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
			nldebug("NOPE: erasing CGmStatus @%p in cache with id %u", this, static_cast<uint32>(_UserId));
			nlverify(_ObjectCache.erase(_UserId) == 1);
		}
	}


	CGmStatusPtr CGmStatus::load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum)
	{
		CGmStatus *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return CGmStatusPtr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		
		qs += "user_id, available";

		qs += " FROM gm_status";
		
		qs += " WHERE user_id = '"+NLMISC::toString(id)+"'";
	CGmStatusPtr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() <= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new CGmStatus, filename, lineNum);
			// ok, we have an object
			result->fetchRow();

			result->getField(0, ret->_UserId);
			result->getField(1, ret->_Available);


			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}

	bool CGmStatus::loadChildOfCRingUser(MSW::CConnection &connection, uint32 parentId, CGmStatusPtr &childPtr, const char *filename, uint32 lineNum)
	{
		std::string qs;
		qs = "SELECT ";

		qs += "user_id, available";

		qs += " FROM gm_status";
		qs += " WHERE user_id = '"+NLMISC::toString(parentId)+"'";

		CGmStatusPtr ret;
		if (!connection.query(qs))
		{
			childPtr = CGmStatusPtr();
			return false;
		}

		std::auto_ptr<MSW::CStoreResult> result = connection.storeResult();

		// check that the data description is consistent with database content
		nlassert(result->getNumRows() <= 1);

		if (result->getNumRows() == 1)
		{
			CGmStatus *object = new CGmStatus;
			// ok, we have an object
			result->fetchRow();
			
			result->getField(0, object->_UserId);
					
			result->getField(1, object->_Available);
					CGmStatus *inCache = loadFromCache(object->_UserId, true);
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
		childPtr = CGmStatusPtr();
		return true;
	}

}
