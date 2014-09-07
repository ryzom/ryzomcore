
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef NEL_DATABASE_MAPPING
#define NEL_DATABASE_MAPPING
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "nel/misc/string_common.h"
#include "server_share/mysql_wrapper.h"

#include "ring_session_manager.h"
	
#include "game_share/ring_session_manager_itf.h"
	
namespace RSMGR
{
	
	class CNelUser;

	class CNelUserPtr;
	class CNelPermission;

	class CNelPermissionPtr;



	class CNelUserPtr
	{
		friend class CNelUser;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CNelUserPtr	*_NextPtr;
		CNelUserPtr	*_PrevPtr;

		CNelUser	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CNelUserPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CNelUserPtr(const CNelUserPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CNelUserPtr(const CNelUserPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CNelUserPtr(CNelUser *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;

			linkPtr();
		}

		CNelUserPtr &assign(const CNelUserPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CNelUserPtr()
		{
			unlinkPtr();
		}

		CNelUserPtr &assign(CNelUser *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CNelUserPtr &operator =(const CNelUserPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CNelUser *operator ->()
		{
			return _Ptr;
		}
		const CNelUser *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CNelUserPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CNelUserPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CNelUser *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CNelUser *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CNelUserPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CNelUserPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CNelPermissionPtr
	{
		friend class CNelPermission;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CNelPermissionPtr	*_NextPtr;
		CNelPermissionPtr	*_PrevPtr;

		CNelPermission	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CNelPermissionPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CNelPermissionPtr(const CNelPermissionPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CNelPermissionPtr(const CNelPermissionPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CNelPermissionPtr(CNelPermission *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;

			linkPtr();
		}

		CNelPermissionPtr &assign(const CNelPermissionPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CNelPermissionPtr()
		{
			unlinkPtr();
		}

		CNelPermissionPtr &assign(CNelPermission *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CNelPermissionPtr &operator =(const CNelPermissionPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CNelPermission *operator ->()
		{
			return _Ptr;
		}
		const CNelPermission *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CNelPermissionPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CNelPermissionPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CNelPermission *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CNelPermission *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CNelPermissionPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CNelPermissionPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};


	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CNelUser
	{
	protected:
		// 
		uint32	_UserId;
		// 
		std::string	_LoginName;
		// 
		std::string	_State;
		// 
		std::string	_Privilege;
		// 
		std::string	_ExtendedPrivilege;
		// 
		uint32	_GMId;
	public:
		// 
		const std::string &getLoginName() const
		{
			return _LoginName;
		}



		void setLoginName(const std::string &value)
		{

			if (_LoginName != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_LoginName = value;

				
			}

		}
			// 
		const std::string &getState() const
		{
			return _State;
		}



		void setState(const std::string &value)
		{

			if (_State != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_State = value;

				
			}

		}
			// 
		const std::string &getPrivilege() const
		{
			return _Privilege;
		}



		void setPrivilege(const std::string &value)
		{

			if (_Privilege != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Privilege = value;

				
			}

		}
			// 
		const std::string &getExtendedPrivilege() const
		{
			return _ExtendedPrivilege;
		}



		void setExtendedPrivilege(const std::string &value)
		{

			if (_ExtendedPrivilege != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_ExtendedPrivilege = value;

				
			}

		}
			// 
		uint32 getGMId() const
		{
			return _GMId;
		}

		void setGMId(uint32 value)
		{

			if (_GMId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_GMId = value;

			}

		}
	
		bool operator == (const CNelUser &other) const
		{
			return _UserId == other._UserId
				&& _LoginName == other._LoginName
				&& _State == other._State
				&& _Privilege == other._Privilege
				&& _ExtendedPrivilege == other._ExtendedPrivilege
				&& _GMId == other._GMId;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CNelUser()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_UserId(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CNelUser();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CNelUserPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CNelUserPtr(new CNelUser(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CNelUserPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);



	private:
	
	private:
		friend class CPersistentCache;
		friend class CNelUserPtr;

		typedef std::map<uint32, CNelUser*>	TObjectCache;
		typedef std::set<CNelUser*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CNelUserPtr		*_PtrList;

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CNelUser *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CNelUserPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CNelUserPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:

		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _UserId;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_UserId == NOPE::INVALID_OBJECT_ID);
			_UserId = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CNelPermission
	{
	protected:
		// 
		uint32	_PermissionId;
		// 
		uint32	_UserId;
		// 
		uint32	_DomainId;
		// 
		uint32	_ShardId;
		// 
		std::string	_AccessPriv;
	public:
		// 
		uint32 getUserId() const
		{
			return _UserId;
		}

		void setUserId(uint32 value)
		{

			if (_UserId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_UserId = value;

			}

		}
			// 
		uint32 getDomainId() const
		{
			return _DomainId;
		}

		void setDomainId(uint32 value)
		{

			if (_DomainId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_DomainId = value;

			}

		}
			// 
		uint32 getShardId() const
		{
			return _ShardId;
		}

		void setShardId(uint32 value)
		{

			if (_ShardId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ShardId = value;

			}

		}
			// 
		const std::string &getAccessPriv() const
		{
			return _AccessPriv;
		}



		void setAccessPriv(const std::string &value)
		{

			if (_AccessPriv != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_AccessPriv = value;

				
			}

		}
	
		bool operator == (const CNelPermission &other) const
		{
			return _PermissionId == other._PermissionId
				&& _UserId == other._UserId
				&& _DomainId == other._DomainId
				&& _ShardId == other._ShardId
				&& _AccessPriv == other._AccessPriv;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CNelPermission()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_PermissionId(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CNelPermission();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CNelPermissionPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CNelPermissionPtr(new CNelPermission(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CNelPermissionPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);



	private:
	
	private:
		friend class CPersistentCache;
		friend class CNelPermissionPtr;

		typedef std::map<uint32, CNelPermission*>	TObjectCache;
		typedef std::set<CNelPermission*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CNelPermissionPtr		*_PtrList;

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CNelPermission *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CNelPermissionPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CNelPermissionPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:

		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			return _PermissionId;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_PermissionId == NOPE::INVALID_OBJECT_ID);
			_PermissionId = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


	
}
	
#endif
