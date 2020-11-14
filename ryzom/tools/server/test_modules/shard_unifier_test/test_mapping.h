
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef TEST_MAPPING
#define TEST_MAPPING
#include "nel/misc/types_nl.h"
#ifdef NL_COMP_VC8
  #include <memory>
#endif
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "nel/misc/string_common.h"
#include "game_share/mysql_wrapper.h"

namespace TM
{
	
	class CRootTable;

	class CRootTablePtr;
	class COneChild;

	class COneChildPtr;
	class CMapChild;

	class CMapChildPtr;
	class CVectorChild;

	class CVectorChildPtr;



	class CRootTablePtr
	{
		friend class CRootTable;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CRootTablePtr	*_NextPtr;
		CRootTablePtr	*_PrevPtr;

		CRootTable	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CRootTablePtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CRootTablePtr(const CRootTablePtr &other, const char *filename, uint32 lineNum)
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

		CRootTablePtr(const CRootTablePtr &other)
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

		CRootTablePtr(CRootTable *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CRootTablePtr &assign(const CRootTablePtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CRootTablePtr()
		{
			unlinkPtr();
		}

		CRootTablePtr &assign(CRootTable *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CRootTablePtr &operator =(const CRootTablePtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CRootTable *operator ->()
		{
			return _Ptr;
		}
		const CRootTable *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CRootTablePtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CRootTablePtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CRootTable *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CRootTable *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CRootTablePtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CRootTablePtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class COneChildPtr
	{
		friend class COneChild;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		COneChildPtr	*_NextPtr;
		COneChildPtr	*_PrevPtr;

		COneChild	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		COneChildPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		COneChildPtr(const COneChildPtr &other, const char *filename, uint32 lineNum)
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

		COneChildPtr(const COneChildPtr &other)
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

		COneChildPtr(COneChild *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		COneChildPtr &assign(const COneChildPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~COneChildPtr()
		{
			unlinkPtr();
		}

		COneChildPtr &assign(COneChild *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		COneChildPtr &operator =(const COneChildPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		COneChild *operator ->()
		{
			return _Ptr;
		}
		const COneChild *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const COneChildPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const COneChildPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const COneChild *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const COneChild *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const COneChildPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		COneChildPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CMapChildPtr
	{
		friend class CMapChild;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CMapChildPtr	*_NextPtr;
		CMapChildPtr	*_PrevPtr;

		CMapChild	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CMapChildPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CMapChildPtr(const CMapChildPtr &other, const char *filename, uint32 lineNum)
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

		CMapChildPtr(const CMapChildPtr &other)
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

		CMapChildPtr(CMapChild *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CMapChildPtr &assign(const CMapChildPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CMapChildPtr()
		{
			unlinkPtr();
		}

		CMapChildPtr &assign(CMapChild *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CMapChildPtr &operator =(const CMapChildPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CMapChild *operator ->()
		{
			return _Ptr;
		}
		const CMapChild *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CMapChildPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CMapChildPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CMapChild *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CMapChild *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CMapChildPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CMapChildPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CVectorChildPtr
	{
		friend class CVectorChild;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CVectorChildPtr	*_NextPtr;
		CVectorChildPtr	*_PrevPtr;

		CVectorChild	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CVectorChildPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CVectorChildPtr(const CVectorChildPtr &other, const char *filename, uint32 lineNum)
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

		CVectorChildPtr(const CVectorChildPtr &other)
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

		CVectorChildPtr(CVectorChild *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CVectorChildPtr &assign(const CVectorChildPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CVectorChildPtr()
		{
			unlinkPtr();
		}

		CVectorChildPtr &assign(CVectorChild *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CVectorChildPtr &operator =(const CVectorChildPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CVectorChild *operator ->()
		{
			return _Ptr;
		}
		const CVectorChild *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CVectorChildPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CVectorChildPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CVectorChild *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CVectorChild *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CVectorChildPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CVectorChildPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};


	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRootTable
	{
	protected:
		// 
		uint32	_Id;

		friend class CMapChild;

		std::map < uint32,  CMapChildPtr >	*_MapChilds;

		friend class CVectorChild;

		std::vector < CVectorChildPtr >	*_VectorChilds;
		friend class COneChild;
		bool								_OneChildLoaded;
		COneChildPtr	_OneChild;
	public:

		/** Return the one child object (or null if not) */
		COneChildPtr getOneChild();

		/** Return a const reference to the map of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following method who return non const pointer
		 *	on contained elements.
		 */
		const std::map<uint32, CMapChildPtr> &getMapChilds() const;
		/** Return the identified element by looking in the map
		 *	If no element match the id, NULL pointer is returned
		 */
		CMapChildPtr &getMapChildsById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CVectorChildPtr> &getVectorChilds() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CVectorChildPtr &getVectorChildsByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CVectorChildPtr &getVectorChildsById(uint32 id) const;

	
		bool operator == (const CRootTable &other) const
		{
			return _Id == other._Id;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CRootTable()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{
			_MapChilds = NULL;
			_VectorChilds = NULL;
			_OneChildLoaded = false;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CRootTable();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CRootTablePtr createTransient(const char *filename, uint32 lineNum)
		{
			return CRootTablePtr(new CRootTable(), filename, lineNum);
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
		static CRootTablePtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/// Load OneChild child(ren) object(s).
		bool loadOneChild(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load MapChilds child(ren) object(s).
		bool loadMapChilds(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load VectorChilds child(ren) object(s).
		bool loadVectorChilds(MSW::CConnection &connection, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CRootTablePtr;

		typedef std::map<uint32, CRootTable*>	TObjectCache;
		typedef std::set<CRootTable*>			TObjectSet;
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
		CRootTablePtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CRootTable *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CRootTablePtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CRootTablePtr *getFirstPtr()
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
			return _Id;
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
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
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
	class COneChild
	{
	protected:
		// 
		uint32	_Id;
	public:

		bool operator == (const COneChild &other) const
		{
			return _Id == other._Id;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		COneChild()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~COneChild();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static COneChildPtr createTransient(const char *filename, uint32 lineNum)
		{
			return COneChildPtr(new COneChild(), filename, lineNum);
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
		static COneChildPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load the object child of CRootTable and
		 *	return true if no error, false in case of error (in SQL maybe).
		 *	If no such object is found, fill the child pointer with NULL.
		 */
		static bool loadChildOfCRootTable(MSW::CConnection &connection, uint32 parentId, COneChildPtr &childPtr, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class COneChildPtr;

		typedef std::map<uint32, COneChild*>	TObjectCache;
		typedef std::set<COneChild*>			TObjectSet;
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
		COneChildPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static COneChild *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(COneChildPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		COneChildPtr *getFirstPtr()
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

			return _Id;
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
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
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
	class CMapChild
	{
	protected:
		// 
		uint32	_Id;
		// 
		uint32	_ParentId;
	public:
		// 
		uint32 getParentId() const
		{
			return _ParentId;
		}

		void setParentId(uint32 value)
		{

			if (_ParentId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ParentId = value;

			}

		}
	
		bool operator == (const CMapChild &other) const
		{
			return _Id == other._Id
				&& _ParentId == other._ParentId;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CMapChild()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CMapChild();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CMapChildPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CMapChildPtr(new CMapChild(), filename, lineNum);
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
		static CMapChildPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CRootTable and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCRootTable(MSW::CConnection &connection, uint32 parentId, std::map < uint32, CMapChildPtr > &children, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CMapChildPtr;

		typedef std::map<uint32, CMapChild*>	TObjectCache;
		typedef std::set<CMapChild*>			TObjectSet;
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
		CMapChildPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CMapChild *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CMapChildPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CMapChildPtr *getFirstPtr()
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

			return _Id;
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
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
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
	class CVectorChild
	{
	protected:
		// 
		uint32	_Id;
		// 
		uint32	_ParentId;
		// 
		sint32	_Info;
	public:
		// 
		uint32 getParentId() const
		{
			return _ParentId;
		}

		void setParentId(uint32 value)
		{

			if (_ParentId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ParentId = value;

			}

		}
			// 
		sint32 getInfo() const
		{
			return _Info;
		}

		void setInfo(sint32 value)
		{

			if (_Info != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Info = value;

			}

		}
	
		bool operator == (const CVectorChild &other) const
		{
			return _Id == other._Id
				&& _ParentId == other._ParentId
				&& _Info == other._Info;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CVectorChild()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CVectorChild();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CVectorChildPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CVectorChildPtr(new CVectorChild(), filename, lineNum);
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
		static CVectorChildPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CRootTable and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCRootTable(MSW::CConnection &connection, uint32 parentId, std::vector < CVectorChildPtr > &children, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CVectorChildPtr;

		typedef std::map<uint32, CVectorChild*>	TObjectCache;
		typedef std::set<CVectorChild*>			TObjectSet;
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
		CVectorChildPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CVectorChild *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CVectorChildPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CVectorChildPtr *getFirstPtr()
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

			return _Id;
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
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
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
