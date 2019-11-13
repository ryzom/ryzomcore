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

#ifndef RZ_LUA_OBJECT
#define RZ_LUA_OBJECT


#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/gui/lua_helper.h"

namespace NLGUI
{

	class CLuaEnumeration;

	/**
	  * Wrapper to a lua value
	  *
	  * Useful to navigate through lua tables without having to deal with the stack.
	  *
	  * The following types are tracked by reference :
	  * - lua table
	  * - lua user data
	  * - lua functions
	  *
	  * The following types are kept by value :
	  *
	  * - lua numbers
	  * - lua strings ?
	  * - lua boolean
	  * - lua light user datas
	  * - lua 'pointers'
	  *
	  * Each reference object has an id giving its path in order to track bugs more easily
	  */
	class CLuaObject
	{
	public:
		CLuaObject() {}
		~CLuaObject();
		// Build this object by popping it from the given lua state
		CLuaObject(CLuaState &state, const char *id ="");
		CLuaObject(CLuaState &state, const std::string &id);
		// Build this object from another object
		CLuaObject(const CLuaObject &other);
		// Copy refrence to another lua object
		CLuaObject &operator=(const CLuaObject &other);
		// Get id for that object
		const std::string &getId() const { return _Id; }
		// Set id for that object
		void  setId(const std::string &id) { _Id = id; }
		// See if the obj
		bool            isValid() const;
		// Pop a new value for this lua object from the top of the stack. The stack must not be empty
		void            pop(CLuaState &luaState, const char *id ="");
		// Push the object that is being referenced on the stack
		// An assertion is raised if 'pop' hasn't been called or
		// if the lua state has been destroyed
		void            push() const;
		// Get the lua state in which the object resides.
		CLuaState      *getLuaState() const;
		// Release the object. 'pop' must be called to make the object valid again
		void            release();
		// type queries
		int             type() const;
		const char		*getTypename() const;
		bool            isNil() const;
		bool            isNumber() const;
		bool            isInteger() const;
		bool            isBoolean() const;
		bool            isString() const;
		bool            isFunction() const;
		bool            isCFunction() const;
		bool            isTable() const;
		bool	        isUserData() const;
		bool	        isLightUserData() const;
		bool			isRGBA() const;
		// equality
		bool			rawEqual(const CLuaObject &other) const;
		// conversions (no throw) : the actual value of object is not modified!!
		NLMISC::CRGBA	toRGBA() const; // default to black if not a crgba
		bool			toBoolean() const;
		lua_Number		toNumber() const;
		lua_Integer		toInteger() const;
		std::string 	toString() const;
		lua_CFunction	toCFunction() const;
		void			*toUserData() const;
		const void		*toPointer() const;
		// implicit conversions (no throw)
		operator bool() const;
		operator float() const;
		operator double() const;
		operator sint32() const;
		operator sint64() const;
		operator std::string() const;
		/** create a sub table for this object, with a string as a key
		  * This object must be a table or an exception if thrown
		  */
		CLuaObject newTable(const char *tableName);


		/** Set a value in a table.
		  * If this object is not a table then an exception is thrown
		  * NB : value should came from the same lua environment
		  * \TODO other type of keys
		  */
		void       setValue(const char *key, const CLuaObject &value);
		void	   setValue(const std::string &key, const CLuaObject &value) { setValue(key.c_str(), value); }
		void       setValue(const char *key, const std::string &value);
		void       setValue(const char *key, const char *value);
		void       setValue(const char *key, bool value);
		void       setValue(const char *key, TLuaWrappedFunction value);
		void       setValue(const char *key, double value);
		void       setValue(const char *key, uint32 value);
		void       setValue(const char *key, sint32 value);
		void       setValue(const char *key, sint64 value);
		void	   setValue(const std::string &key, const std::string &value) { setValue(key.c_str(), value); }
		void       setNil(const char *key);
		void       setNil(const std::string &key) { setNil(key.c_str()); }
		/** Erase a value in a table by its key.
		  * If this object is not a table then an exception is thrown.
		  * \TODO other type of keys
		  */
		void       eraseValue(const char *key);
		void       eraseValue(const std::string &key) { eraseValue(key.c_str()); }
		// test is this object is enumerable
		bool	   isEnumerable() const;
		// Enumeration of a table. If the object is not a table, an exception is thrown.
		CLuaEnumeration enumerate();
		// retrieve metatable of an object (or nil if object has no metatable)
		CLuaObject getMetaTable() const;
		// set metatable for this object
		bool setMetaTable(CLuaObject &metatable);
		/** Access to a sub element of a table (no throw).
		  * if the element is not a table, then 'nil' is returned
		  * TODO nico : add other key types if needed
		  * TODO nico : version that takes destination object as a reference in its parameter to avoid an object copy
		  */
		CLuaObject      operator[](double key) const;
		CLuaObject      operator[](const char *key) const;
		CLuaObject      operator[](const std::string &key) const { return operator[](key.c_str()); }
		/** Checked access to a sub element of a table. An exception is thrown is the element is not a table.
		  */
		CLuaObject      at(const char *key) const;
		CLuaObject      at(const std::string &key) const { return at(key.c_str()); }

		// Test is that table has the given key. The object must be a table or an exception is thrown
		bool            hasKey(const char *key) const;

		/** debug : recursively get value (useful for table)
		  * \param maxDepth (0 for no limit)
		  * \param alreadySeen pointer to lua tables that have already been displayed by the command (to avoid infinite recursion when a cycluic graph is encountered)
		  */
		std::string     toStringRecurse(uint depth = 0, uint maxDepth = 20, std::set<const void *> *alreadySeen = NULL) const;

		/** dump the value in the log (includes tables)
		  * \param alreadySeen pointer to lua tables that have already been displayed by the command (to avoid infinite recursion when a cycluic graph is encountered)
		  */
		void		    dump(uint maxDepth = 20, std::set<const void *> *alreadySeen = NULL) const;
		// concatenate identifiers, adding a dot between them if necessary. If right is a number then brackets are added
		static std::string concatId(const std::string &left, const std::string &right);
		// If this object is a function, then call it and return true on success
		bool callNoThrow(int numArgs, int numRet);
		// Call a method of this table by name (no throw version)
		bool callMethodByNameNoThrow(const char *name, int numArgs, int numRet);
	private:
		NLMISC::CRefPtr<CLuaState> _LuaState;
		std::string		           _Id;
	};


	/** enumeration of the content of a lua table
	  *
	  * Example of use :
	  *
	  *\code
		CLuaObject table;
		table.pop(luaState); // retrieve table from the top of a lua stack
		CLuaEnumeration enueration = table.enumerate();
		while (enumeration.hasNext())
		{
			nlinfo('key   = %s", enumeration.nextKey().toString().c_str());
			nlinfo('value = %s", enumeration.nextValue().toString().c_str());
			enumeration.next();
		};
	  \endcode
	  *
	  * There is a macro called 'ENUM_LUA_TABLE' to automate that process.
	  * Previous code would then be written as follow :
	  *
	  *
	  *\code
		CLuaObject table;
		table.pop(luaState); // retrieve table from the top of a lua stack
		ENUM_LUA_TABLE(table, enumeration);
		{
			nlinfo('key   = %s", enumeration.nextKey().toString().c_str());
			nlinfo('value = %s", enumeration.nextValue().toString().c_str());
		};
	  \endcode
	  *
	  */
	class CLuaEnumeration
	{
	public:
		// is there a next key,value pair in the table
		bool		       hasNext() { return _HasNext; }
		// Return next key. Assertion if 'hasNext' is false
		const CLuaObject  &nextKey() const;
		// Return next value. Assertion if 'hasNext' is false
		CLuaObject        &nextValue();
		// Go to the next value. Assertion if there's no such value
		void next();
	private:
		friend class CLuaObject;
		// current value & key
		CLuaObject _Table;
		CLuaObject _Key;
		CLuaObject _Value;
		CLuaObject _NextFunction; // pointer to the global 'next' function
		bool       _HasNext;
		// construction from a table on the stack
		CLuaEnumeration(CLuaObject &table);
	};




	/** macro to ease lua table enumeration
	  * \param object A CLuaObject which must be a table, and on which enumeration is done. An exception will be thrown as 'CLuaObject::enumerate' is
	  *               called if this is not the case
	  * \param enumerator The enumerator object
	  */
	#define ENUM_LUA_TABLE(object, enumerator) for(CLuaEnumeration enumerator = (object).enumerate();  enumerator.hasNext(); enumerator.next())


	//opitmized lua string for fast comparison
	class CLuaString
	{
	public:
		explicit CLuaString(const char *value = "")
		{
			nlassert( value != NULL );
			_Str = value;
		}
		const std::string& getStr() const{ return _Str; }
	private:
		std::string _Str;
		mutable CLuaState::TRefPtr _LuaState;	// ref ptr so that statics get rebuilt on lua restart
		mutable CLuaObject _InLua;
	};

	inline bool operator==(const char* lh, const CLuaString& rh)
	{
		return std::string(lh) == rh.getStr();
	}

	inline bool operator==( const CLuaString& lh, const CLuaString& rh)
	{
		return lh.getStr() == rh.getStr();
	}

	inline bool operator==(const CLuaString& lh, const char* rh)
	{
		return std::string(rh) == lh.getStr();
	}

	inline bool operator==( const CLuaString& lh, const std::string& rh)
	{
		return lh.getStr() == rh;
	}

	inline bool operator==(const std::string& lh, const CLuaString& rh)
	{
		return lh == rh.getStr();
	}

	class CLuaHashMapTraits
	{
	public:
		enum { bucket_size = 4, min_buckets = 8, };
		CLuaHashMapTraits()
		{}

		// hasher for lua string  -> they are unique pointers for each string, so just hash a pointer instead of a string...
		size_t operator()(const char *value) const { return ((size_t) value) >> 3; }

		// equality for lua string for hash_map -> they are unique pointer -> compare pointers instead of string content
		bool operator()(const char *lhs, const char *rhs) const { return lhs < rhs; }
	};


}

#endif
