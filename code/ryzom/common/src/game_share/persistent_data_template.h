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

/*
	**************************************
	NOTE: THIS IS NOT A NORMAL HEADER FILE
	**************************************

	This header file generates code for the following methods for a user-defined class

		void <user_class>::store(CPersistentDataRecord &pdr) const;
		void <user_class>::apply(CPersistentDataRecord &pdr);

	Note:
		Prototypes for these methods are also defined in the DECLARE_PERSISTENCE_METHODS macro in the  persistent_data.h file.

	Note:
		This header may be included several times in the same cpp file in order to instantiate
		store and apply methods for different classes


	USAGE:
		#define PERSISTENT_CLASS <class_name>
		#define PERSISTENT_DATA <data_list>

		#include "persistent_data_template.h"

		#undef PERSISTENT_CLASS
		#undef PERSISTENT_DATA


	EXAMPLE USAGE:
		////////////////////////////////////////////////////////

		#define PERSISTENT_CLASS CMyClass
		#define PERSISTENT_DATA\
			PROP(sint16,	Title,				getTitle(),				setTitle(val))\
			PROP(uint64,	VisualPropertyA,	getVisualPropertyA(),	setVisualPropertyA(val))

		#include "perstent_data_template.h"

		#undef PERSISTENT_CLASS
		#undef PERSISTENT_DATA

		////////////////////////////////////////////////////////

		#define PERSISTENT_CLASS CAnotherClass
		#define PERSISTENT_DATA\
			PROP_VECT(ucstring,IsIgnoredBy,_IsIgnoredBy,_IsIgnoredBy[i],_IsIgnoredBy[i]=val)\
			PROP_MAP(SP_TYPE,uint32,SkillPoints,_SpType,toString(key),val,setSp(fromString(key),val))\
			STRUCT(EntityBase,CEntityBaseCharacter::store(pdr),CEntityBaseCharacter::apply(pdr))\
			STRUCT_VECT(Pact,_Pact,_Pact[i].store(pdr),_Pact[i].apply(pdr))\
			STRUCT_MAP(TAIAlias,TMissionHistory,MissionHistories,_MissionHistories,key,val.store(pdr),_MissionHistories[key].apply(pdr))

		#include "persistent_data_template.h"

		#undef PERSISTENT_CLASS
		#undef PERSISTENT_DATA

		////////////////////////////////////////////////////////

	MACROS FOR PERSISTENT_DATA DEFINITION
		PROP(type,varName)
		PROP2(name,type,get,set)
		PROP3(name,type,logic,get,set)
		PROP_SET(type,varName)
		PROP_VECT(type,varName)
		PROP_MAP(keyType,valType,varName)
		PROP_MAP2(name,keyType,valType,logic,getKey,getVal,set)
		STRUCT(varName)
		STRUCT2(name,write,read)
		STRUCT3(name,logic,write,read)
		STRUCT_VECT(varName)
		STRUCT_MAP(keyType,valType,varName)
		STRUCT_PTR_VECT(type,varName)
		STRUCT_MAP2(name,keyType,logic,getKey,valWrite,read)

	Where:
		name		- the name stored in the persistent data record for this data element
		varName		- the name of a container (map or vector) to be iterated over

		type		- the data type of a variable
		keyType		- the data type of the key for a persistent map
		valType		- the data type of the value for a persistent map

		get			- code that returns a value to be stored in the persistent data
		write		- code to be executed for a structure at write time (equivalent to 'get' for a property)
		getKey		- code that returns a value for a map key to be stored in the persistent data
		getVal		- code that returns a value for a map value to be stored in the persistent data
		valWrite	- code to be executed for a map value structure at write time (equivalent to 'getVal' for a property)

		set			- code to store the value 'val' in persistent property at read time
		read		- code to be executed for a structure at read time (equivalent to 'get')

	Context notes:
		write		- 'pdr' contains the persistent data record currently being processed
		getKey		- 'it' is the map iterator
		getVal		- 'it' is the map iterator
		valWrite	- 'pdr' contains the persistent data record currently being processed
		set			- 'val' contains the value to apply
		read		- 'pdr' contains the persistent data record currently being processed
		read		- for maps: 'key' contains the map key
		read		- for property maps: 'val' contains the map entry value

		PROP_VECT	- 'i' is the vector index during all read and write operations
		STRUCT_VECT	- 'i' is the vector index during all read and write operations
*/

#ifndef PERSISTENT_CLASS
#error PERSISTENT_CLASS not defined
#endif

#ifndef PERSISTENT_DATA
#error PERSISTENT_DATA not defined
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAP_LOGIC(keyType,valType,varName) for(std::map<keyType,valType>::const_iterator it=varName.begin();it!=varName.end();++it)
#define SET_LOGIC(type,varName) for(std::set<type>::const_iterator it=varName.begin();it!=varName.end();++it)
#define LIST_LOGIC(type,varName) for(std::list<type>::const_iterator it=varName.begin();it!=varName.end();++it)
#define VECT_LOGIC(varName) ARRAY_LOGIC(varName.size())
#define ARRAY_LOGIC(size) for(uint32 i=0;i<size;++i)
#define DEFAULT_LOGIC {}

#define PROP(type,varName)											_PROP(__Tok##varName,#varName,type,DEFAULT_LOGIC,varName,varName=val)
#define NPROP(name,type,varName)									_PROP(__Tok##name,#name,type,DEFAULT_LOGIC,varName,varName=val)
#define PROP2(name,type,get,set)									_PROP(__Tok##name,#name,type,DEFAULT_LOGIC,get,set)
#define LPROP(type,varName,logic)									_PROP(__Tok##varName,#varName,type,logic,varName,varName=val)
#define LPROP2(name,type,logic,get,set)								_PROP(__Tok##name,#name,type,logic,get,set)
#define PROP_SET(type,varName)										_PROP(__Tok##varName,#varName,type,SET_LOGIC(type,varName),(*it),varName.insert(val))
#define PROP_LIST(type,varName)										_PROP(__Tok##varName,#varName,type,LIST_LOGIC(type,varName),(*it),varName.push_back(val))
#define PROP_VECT(type,varName)										_PROP(__Tok##varName,#varName,type,VECT_LOGIC(varName),varName[i],varName.push_back(val))
#define LPROP_VECT(type,varName,logic)								_PROP(__Tok##varName,#varName,type,logic,varName[i],varName.push_back(val))
#define LPROP_VECT2(name,type,logic,get,set)						_PROP(__Tok##name,#name,type,logic,get,set)
#define PROP_MAP(keyType,valType,varName)							_PROP_MAP(__Tok##varName,#varName,keyType,valType,MAP_LOGIC(keyType,valType,varName),it->first,it->second,varName[key]=val)
#define NPROP_MAP(name,keyType,valType,varName)						_PROP_MAP(__Tok##name,#name,keyType,valType,MAP_LOGIC(keyType,valType,varName),it->first,it->second,varName[key]=val)
#define PROP_MAP2(name,keyType,valType,getKey,getVal,set)			_PROP_MAP(__Tok##name,#name,keyType,valType,MAP_LOGIC(keyType,valType,varName),getKey,getVal,set)
#define PROP_ARRAY(type,varName,size)								_PROP_MAP(__Tok##varName,#varName,uint32,type,ARRAY_LOGIC(size),i,varName[i],if (key<size) varName[key]=val)
#define LPROP_MAP2(name,keyType,valType,logic,getKey,getVal,set)	_PROP_MAP(__Tok##name,#name,keyType,valType,logic,getKey,getVal,set)
#define STRUCT(varName)												_STRUCT(__Tok##varName,#varName,DEFAULT_LOGIC,(varName).store(pdr),(varName).apply(pdr))
#define STRUCT2(name,write,read)									_STRUCT(__Tok##name,#name,DEFAULT_LOGIC,write,read)
#define LSTRUCT(varName,logic)										_STRUCT(__Tok##varName,#varName,logic,(varName).store(pdr),(varName).apply(pdr))
#define LSTRUCT2(name,logic,write,read)								_STRUCT(__Tok##name,#name,logic,write,read)
#define STRUCT_LIST(type,varName)									_STRUCT(__Tok##varName,#varName,LIST_LOGIC(type,varName),(*it).store(pdr),listAppend(varName).apply(pdr))
#define STRUCT_VECT(varName)										_STRUCT(__Tok##varName,#varName,VECT_LOGIC(varName),varName[i].store(pdr),vectAppend(varName).apply(pdr))
#define LSTRUCT_VECT(varName,logic,write,read)						_STRUCT(__Tok##varName,#varName,logic,write,read)
#define STRUCT_PTR_VECT(type,varName)								_STRUCT(__Tok##varName,#varName,VECT_LOGIC(varName),if (varName[i]!=NULL)varName[i]->store(pdr),(varName.push_back(new type),varName.back())->apply(pdr))
#define STRUCT_SMRTPTR_VECT(type,varName)							_STRUCT(__Tok##varName,#varName,VECT_LOGIC(varName),if (varName[i]!=NULL)varName[i]->store(pdr),(varName.push_back(NLMISC::CSmartPtr<type>(new type)),varName.back())->apply(pdr))
#define STRUCT_INDEXED_VECT(varName)								_STRUCT_MAP(__Tok##varName,#varName,uint32,VECT_LOGIC(varName),i,varName[i].store(pdr),if (key>=varName.size()) varName.resize(key); varName[key].apply(pdr))
#define STRUCT_MAP(keyType,valType,varName)							_STRUCT_MAP(__Tok##varName,#varName,keyType,MAP_LOGIC(keyType,valType,varName),it->first,it->second.store(pdr),varName[key].apply(pdr))
#define LSTRUCT_MAP2(name,keyType,logic,getKey,valWrite,read)		_STRUCT_MAP(__Tok##name,#name,keyType,logic,getKey,valWrite,read)
#define STRUCT_ARRAY(varName,size)									_STRUCT_MAP(__Tok##varName,#varName,uint32,ARRAY_LOGIC(size),i,varName[i].store(pdr),if (key<size) varName[key].apply(pdr))
#define FLAG(name,applyCode)										_FLAG(__Tok##name,#name,DEFAULT_LOGIC,applyCode)
#define FLAG0(name,applyCode)										_FLAG(__Tok##name,#name,if(false),applyCode)
#define LFLAG(name,logic,applyCode)									_FLAG(__Tok##name,#name,logic,applyCode)


#ifndef PERSISTENT_GAME_CYCLE_INLINE_DEFINED
#define PERSISTENT_GAME_CYCLE_INLINE_DEFINED
#include "tick_event_handler.h"
#include "nel/misc/hierarchical_timer.h"
inline uint32 saveGameCycleToSecond(NLMISC::TGameCycle tick)
{
    // Evaluate the UTC of this event (with the current date of save). Suppose that 1 second==10 tick
    // NB: result should be positive since no event should have been launched before 1970!
    if (tick < CTickEventHandler::getGameCycle())
    {
        NLMISC::TGameCycle tick_dt = CTickEventHandler::getGameCycle() - tick;
        uint32 s_dt = tick_dt / 10;
        return NLMISC::CTime::getSecondsSince1970() - s_dt;
    }
    else
    {
        NLMISC::TGameCycle tick_dt = tick - CTickEventHandler::getGameCycle();
        uint32 s_dt = tick_dt / 10;
        return NLMISC::CTime::getSecondsSince1970() + s_dt;
    }
}
inline NLMISC::TGameCycle loadSecondToGameCycle(uint32 second)
{
    if (second < NLMISC::CTime::getSecondsSince1970())
    {
        uint32 s_dt = NLMISC::CTime::getSecondsSince1970() - second;
        NLMISC::TGameCycle tick_dt = s_dt * 10;
        return CTickEventHandler::getGameCycle() - tick_dt;
    }
    else
    {
        uint32 s_dt = second - NLMISC::CTime::getSecondsSince1970();
        NLMISC::TGameCycle tick_dt = s_dt * 10;
        return CTickEventHandler::getGameCycle() + tick_dt;
    }
}

/*inline uint32 saveGameCycleToSecond(NLMISC::TGameCycle tick)
{
	sint32 dt = CTickEventHandler::getGameCycle() - tick;


	// Evaluate the UTC of this event (with the current date of save). Suppose that 1 second==10 tick
	if (tick < CTickEventHandler::getGameCycle())
		return NLMISC::CTime::getSecondsSince1970();
	else
		return  NLMISC::CTime::getSecondsSince1970() + (tick - CTickEventHandler::getGameCycle())/10;
	// NB: result should be positive since no event should have been launched before 1970!
}

inline NLMISC::TGameCycle loadSecondToGameCycle(uint32 second)
{
	if (second < NLMISC::CTime::getSecondsSince1970())
		return 0;
	
	// Convert UTC of the event to game cycle. Suppose that 1 second==10 tick
	return CTickEventHandler::getGameCycle() + (second - NLMISC::CTime::getSecondsSince1970())*10;
}*/
#endif

// GameCycle property (saved as a UTC of the current game cycle, support server migration)
#define PROP_GAME_CYCLE(varName)\
	/* read and write, in UTC seconds. if result is negative, clamp to 0 */ \
	PROP2(UTC_##varName, uint32, saveGameCycleToSecond(varName), varName=loadSecondToGameCycle(val))

// GameCycle property with Backward compatibility (saved as a UTC of the current game cycle, support server migration)
#define PROP_GAME_CYCLE_COMP(varName)\
	/* don't write (old variable name), but direct read if in pdr */ \
	LPROP2(varName, NLMISC::TGameCycle, if(0), 0, varName=val) \
	/* read and write, in UTC seconds. if result is negative, clamp to 0 */ \
	PROP2(UTC_##varName, uint32, saveGameCycleToSecond(varName), varName=loadSecondToGameCycle(val))

// SideNotes:
// if the server where we load the game cycle is different from the one from where we saved, and if he is younger,
// then we can have situations where gamecycle<0. Since TGameCycle is actually a uint32, this will cause bugs.
// Hence we don't allow <0 values.


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
//
// NOTE: This method is declared in DECLARE_PERSISTENCE_METHODS macro in persistent_data.h
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef PERSISTENT_NO_STORE

#ifdef PERSISTENT_TOKEN_FAMILY

//#pragma message( "Using token family "NL_MACRO_TO_STR(PERSISTENT_TOKEN_FAMILY)" for persistent data class " NL_MACRO_TO_STR(PERSISTENT_CLASS) )

// setup some macros for constructing the classname and object name that we need for
// the 'persistent token family' constants class
#define _EVAL(b,c) b##c
#define _BUILD_TOKENS_CLASSNAME(c) _EVAL(c,__constants__class)
#define _BUILD_TOKENS_OBJNAME(c) _EVAL(c,__constants__obj)

// setup the class name and object name for the 'persistent token family' constants class and its instance
#ifdef PERSISTENT_TOKEN_CLASS
	#define _TOKENS_CLASSNAME _BUILD_TOKENS_CLASSNAME(PERSISTENT_TOKEN_CLASS)
	#define _TOKENS_OBJNAME	_BUILD_TOKENS_OBJNAME(PERSISTENT_TOKEN_CLASS)
#else
	#define _TOKENS_CLASSNAME _BUILD_TOKENS_CLASSNAME(PERSISTENT_CLASS)
	#define _TOKENS_OBJNAME	_BUILD_TOKENS_OBJNAME(PERSISTENT_CLASS)
#endif


// define the set of tokens required by this class
// adding them to some pre-defined token family
struct _TOKENS_CLASSNAME
{
	// the ctor is used to setup the values of the static tokens
	_TOKENS_CLASSNAME()
	{
		#define _ADD_TOKEN(token) CPdrTokenRegistry::getInstance()->addToken(NL_MACRO_TO_STR(PERSISTENT_TOKEN_FAMILY),token)

		__Tok__MapKey= _ADD_TOKEN("__Key__");
		__Tok__MapVal= _ADD_TOKEN("__Val__");
		#define _PROP(token,name,type,logic,get,set)							token= _ADD_TOKEN(name);
		#define _STRUCT(token,name,logic,write,read)							token= _ADD_TOKEN(name);
		#define _PROP_MAP(token,name,keyType,valType,logic,getKey,getVal,set)	token= _ADD_TOKEN(name);
		#define _STRUCT_MAP(token,name,keyType,logic,getKey,valWrite,read)		token= _ADD_TOKEN(name);
		#define _FLAG(token,name,logic,code)									token= _ADD_TOKEN(name);
		PERSISTENT_DATA
		#undef _PROP
		#undef _STRUCT
		#undef _PROP_MAP
		#undef _STRUCT_MAP
		#undef _FLAG

		#undef _ADD_TOKEN
	}

	// define the set of static variables to act as constants for stocking the set of tokens for this persistet data class
	uint16 __Tok__MapKey;
	uint16 __Tok__MapVal;
	#define _PROP(token,name,type,logic,get,set)							uint16 token;
	#define _STRUCT(token,name,logic,write,read)							uint16 token;
	#define _PROP_MAP(token,name,keyType,valType,logic,getKey,getVal,set)	uint16 token;
	#define _STRUCT_MAP(token,name,keyType,logic,getKey,valWrite,read)		uint16 token;
	#define _FLAG(token,name,logic,code)									uint16 token;
	PERSISTENT_DATA
	#undef _PROP
	#undef _STRUCT
	#undef _PROP_MAP
	#undef _STRUCT_MAP
	#undef _FLAG
};
static _TOKENS_CLASSNAME _TOKENS_OBJNAME;

#undef _TOKENS_CLASSNAME
#undef _TOKENS_OBJNAME

#else

#ifdef NL_OS_WINDOWS
	#pragma message( " ")
	#pragma message( "NON-OPTIMISED: Persistent data class " NL_MACRO_TO_STR(PERSISTENT_CLASS) " not using a token family")
	#pragma message( " ")
#endif

#endif

// define _PERSISTENT_STORE_ARGS as either (<empty>) or (","<extra args>) for use in adding args to the store() function prototype
#ifdef PERSISTENT_STORE_ARGS
#define _PERSISTENT_STORE_ARGS ,PERSISTENT_STORE_ARGS
#else
#define _PERSISTENT_STORE_ARGS
#endif

// store()
void PERSISTENT_CLASS::store(CPersistentDataRecord &pdr _PERSISTENT_STORE_ARGS) const
{
	// PDR debug helper
	#ifdef NL_DEBUG
		//nldebug("PDR:store:"NL_MACRO_TO_STR(PERSISTENT_CLASS)"(enter)");
	#  define _PDR_TRACE(name)	//nldebug("PDR:store:" NL_MACRO_TO_STR(PERSISTENT_CLASS) "." #name)
	#else
	#  define _PDR_TRACE(name)
	#endif

	#ifdef PERSISTENT_TOKEN_FAMILY

		#ifdef NL_DEBUG
			// this class has been configured to use a specific token family. This means that it is not possible
			// to store the class to a pdr that is not using the same token family.
			BOMB_IF(pdr.getTokenFamily()!=NL_MACRO_TO_STR(PERSISTENT_TOKEN_FAMILY),
				NL_MACRO_TO_STR(PERSISTENT_CLASS)"::store() requires token family '"NL_MACRO_TO_STR(PERSISTENT_TOKEN_FAMILY)"'"
				" but pdr is using token family '"+pdr.getTokenFamily()+"'",
				return);
		#endif

		#ifdef PERSISTENT_TOKEN_CLASS
			#define _TOKEN(token) _BUILD_TOKENS_OBJNAME(PERSISTENT_TOKEN_CLASS).token
		#else
			#define _TOKEN(token) _BUILD_TOKENS_OBJNAME(PERSISTENT_CLASS).token
		#endif

	#else
		// define the set of tokens - this makes sure that the tokens exist in the map and that we only look them up the once
		static uint16 __Tok__MapKey= (uint16)~0u; pdr.addString("__Key__",__Tok__MapKey);
		static uint16 __Tok__MapVal= (uint16)~0u; pdr.addString("__Val__",__Tok__MapVal);
		#define _PROP(token,name,type,logic,get,set)							static uint16 token= (uint16)~0u; pdr.addString(name,token);
		#define _STRUCT(token,name,logic,write,read)							static uint16 token= (uint16)~0u; pdr.addString(name,token);
		#define _PROP_MAP(token,name,keyType,valType,logic,getKey,getVal,set)	static uint16 token= (uint16)~0u; pdr.addString(name,token);
		#define _STRUCT_MAP(token,name,keyType,logic,getKey,valWrite,read)		static uint16 token= (uint16)~0u; pdr.addString(name,token);
		#define _FLAG(token,name,logic,code)									static uint16 token= (uint16)~0u; pdr.addString(name,token);
		PERSISTENT_DATA
		#undef _PROP
		#undef _STRUCT
		#undef _PROP_MAP
		#undef _STRUCT_MAP
		#undef _FLAG

		#define _TOKEN(token) token
	#endif

	// Add user-defined code
	#ifdef PERSISTENT_PRE_STORE
		PERSISTENT_PRE_STORE
	#endif

	// write the properties to the persistent data record
	#define _PROP(token,name,type,logic,get,set)\
		_PDR_TRACE(name);\
		logic { type val=get; pdr.push(_TOKEN(token),val); }

	#define _STRUCT(token,name,logic,write,read)\
		_PDR_TRACE(name);\
		logic { pdr.pushStructBegin(_TOKEN(token)); write; pdr.pushStructEnd(_TOKEN(token)); }

	#define _PROP_MAP(token,name,keyType,valType,logic,getKey,getVal,set)\
		_PDR_TRACE(name);\
		{\
			pdr.pushStructBegin(_TOKEN(token));\
			logic { keyType __k=getKey; valType __v=getVal; pdr.push(_TOKEN(__Tok__MapKey),__k); pdr.push(_TOKEN(__Tok__MapVal),__v); }\
			pdr.pushStructEnd(_TOKEN(token)); \
		}

	#define _STRUCT_MAP(token,name,keyType,logic,getKey,valWrite,read)\
		_PDR_TRACE(name);\
		{ \
			pdr.pushStructBegin(_TOKEN(token));\
			logic { keyType __k=getKey; pdr.push(_TOKEN(__Tok__MapKey),__k); pdr.pushStructBegin(_TOKEN(__Tok__MapVal)); valWrite; pdr.pushStructEnd(_TOKEN(__Tok__MapVal)); }\
			pdr.pushStructEnd(_TOKEN(token)); \
		}

	#define _FLAG(token,name,logic,code)\
		_PDR_TRACE(name);\
		logic { pdr.push(_TOKEN(token)); }

	PERSISTENT_DATA

	#undef _PROP
	#undef _STRUCT
	#undef _PROP_MAP
	#undef _STRUCT_MAP
	#undef _FLAG
	#undef _PDR_TRACE

	#undef _TOKEN


	// Add user-defined code
	#ifdef PERSISTENT_POST_STORE
		PERSISTENT_POST_STORE
	#endif
	#ifdef NL_DEBUG
		//nldebug("PDR:store:"NL_MACRO_TO_STR(PERSISTENT_CLASS)"(leave)");
	#endif
}

#endif // ndef PERSISTENT_NO_STORE

// define _PERSISTENT_APPLY_ARGS as either (<empty>) or (","<extra args>) for use in adding args to the apply() function prototype
#ifdef PERSISTENT_APPLY_ARGS
#define _PERSISTENT_APPLY_ARGS ,PERSISTENT_APPLY_ARGS
#else
#define _PERSISTENT_APPLY_ARGS
#endif

#ifndef PERSISTENT_NO_APPLY

// apply()
void PERSISTENT_CLASS::apply(CPersistentDataRecord &pdr _PERSISTENT_APPLY_ARGS)
{
	#define DECORATE(x) #x
	#define MY_H_AUTO(name) static NLMISC::CHTimer _timer(DECORATE(name)"_Apply"); NLMISC::CAutoTimer _auto(&_timer);
	MY_H_AUTO(PERSISTENT_CLASS)
	#undef MY_H_AUTO
	#undef DECORATE

	// PDR debug helper
	#ifdef NL_DEBUG
		//nldebug("PDR:apply:"NL_MACRO_TO_STR(PERSISTENT_CLASS)"(enter)");
	#  define _PDR_TRACE(name)	//nldebug("PDR:apply:" NL_MACRO_TO_STR(PERSISTENT_CLASS) "." #name)
	#else
	#  define _PDR_TRACE(name)
	#endif

	// define the set of tokens - this makes sure that the tokens exist in the map and that we only look them up the once
	static uint16 __Tok__MapKey= (uint16)~0u; pdr.addString("__Key__",__Tok__MapKey);
	static uint16 __Tok__MapVal= (uint16)~0u; pdr.addString("__Val__",__Tok__MapVal);
	#define _PROP(token,name,type,logic,get,set)							static uint16 token= (uint16)~0u; pdr.addString(name,token);
	#define _STRUCT(token,name,logic,write,read)							static uint16 token= (uint16)~0u; pdr.addString(name,token);
	#define _PROP_MAP(token,name,keyType,valType,logic,getKey,getVal,set)	static uint16 token= (uint16)~0u; pdr.addString(name,token);
	#define _STRUCT_MAP(token,name,keyType,logic,getKey,valWrite,read)		static uint16 token= (uint16)~0u; pdr.addString(name,token);
	#define _FLAG(token,name,logic,code)									static uint16 token= (uint16)~0u; pdr.addString(name,token);
	PERSISTENT_DATA
	#undef _PROP
	#undef _STRUCT
	#undef _PROP_MAP
	#undef _STRUCT_MAP
	#undef _FLAG


	// Add user-defined code at the start of the method
	#ifdef PERSISTENT_PRE_APPLY
		PERSISTENT_PRE_APPLY
	#endif

	// apply the properties from the persistent data record
	#define _PROP(token,name,type,logic,get,set)\
		_PDR_TRACE(name);\
		if (nextToken==token) { H_AUTO(token); type val; pdr.pop(token,val); set; continue; }

	#define _STRUCT(token,name,logic,write,read)\
		_PDR_TRACE(name);\
		if (nextToken==token) { H_AUTO(token##_STRUCT); pdr.popStructBegin(token); read; pdr.popStructEnd(token); continue; }

	#define _PROP_MAP(token,name,keyType,valType,logic,getKey,getVal,set)\
		_PDR_TRACE(name);\
		if (nextToken==token) { H_AUTO(token##_Key); pdr.popStructBegin(token);\
		while (!pdr.isEndOfStruct()) { H_AUTO(token##_Val); keyType key; pdr.pop(__Tok__MapKey,key); valType val; pdr.pop(__Tok__MapVal,val);set;}\
		pdr.popStructEnd(token); continue; }

	#define _STRUCT_MAP(token,name,keyType,logic,getKey,valWrite,read)\
		_PDR_TRACE(name);\
		if (nextToken==token) { H_AUTO(token##_Key); pdr.popStructBegin(token);\
		while (!pdr.isEndOfStruct()) { H_AUTO(token##_Val_STRUCT); keyType key; pdr.pop(__Tok__MapKey,key);pdr.popStructBegin(__Tok__MapVal);read;pdr.popStructEnd(__Tok__MapVal);}\
		pdr.popStructEnd(token); continue; }

	#define _FLAG(token,name,logic,code)\
		_PDR_TRACE(name);\
		if (nextToken==token) { H_AUTO(token##_FLAG); pdr.pop(token); code; continue; }

	while (!pdr.isEndOfStruct())
	{
		uint16 nextToken= pdr.peekNextToken();

		// try to match the next token to one of the persistent data elements and 'continue' on success
		PERSISTENT_DATA
//		nlwarning("Skipping unrecognised token: %s",pdr.peekNextTokenName().c_str());

		// if this is a structure then skip the whole thing
		std::vector<uint16> stack;
		do
		{
			H_AUTO(SkipUnknownStuff);

			if (pdr.isStartOfStruct())
			{
				stack.push_back(pdr.peekNextToken());
				pdr.popStructBegin(stack.back());
			}
			else if (pdr.isEndOfStruct())
			{
				pdr.popStructEnd(stack.back());
				if (!stack.empty())
					stack.pop_back();
			}
			else
			{
				pdr.popNextArg(pdr.peekNextToken());
			}
		}
		while (!stack.empty() && !pdr.isEndOfData());
	}

	#undef _PROP
	#undef _STRUCT
	#undef _PROP_MAP
	#undef _STRUCT_MAP
	#undef _FLAG
	#undef _PDR_TRACE


	// Add user-defined code
	#ifdef PERSISTENT_POST_APPLY
		PERSISTENT_POST_APPLY
	#endif
	#ifdef NL_DEBUG
		//nldebug("PDR:apply:"NL_MACRO_TO_STR(PERSISTENT_CLASS)"(leave)");
	#endif
}

#endif // ndef PERSISTENT_NO_APPLY


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef PROP
#undef PROP2
#undef LPROP
#undef LPROP2
#undef PROP_SET
#undef PROP_VECT
#undef LPROP_VECT
#undef LPROP_VECT2
#undef PROP_MAP
#undef PROP_MAP2
#undef LPROP_MAP2
#undef STRUCT
#undef STRUCT2
#undef LSTRUCT
#undef LSTRUCT2
#undef STRUCT_VECT
#undef LSTRUCT_VECT
#undef STRUCT_PTR_VECT
#undef STRUCT_MAP
#undef LSTRUCT_MAP2

#undef MAP_LOGIC
#undef SET_LOGIC
#undef VECT_LOGIC
#undef ARRAY_LOGIC
#undef DEFAULT_LOGIC

#undef _PERSISTENT_STORE_ARGS
#undef _PERSISTENT_APPLY_ARGS

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PERSISTENT_MACROS_AUTO_UNDEF

	#undef PERSISTENT_CLASS
	#undef PERSISTENT_DATA

	#ifdef PERSISTENT_TOKEN_CLASS
	#undef PERSISTENT_TOKEN_CLASS
	#endif

	#ifdef PERSISTENT_PRE_APPLY
	#undef PERSISTENT_PRE_APPLY
	#endif

	#ifdef PERSISTENT_POST_APPLY
	#undef PERSISTENT_POST_APPLY
	#endif

	#ifdef PERSISTENT_PRE_STORE
	#undef PERSISTENT_PRE_STORE
	#endif

	#ifdef PERSISTENT_POST_STORE
	#undef PERSISTENT_POST_STORE
	#endif

	#ifdef PERSISTENT_STORE_ARGS
	#undef PERSISTENT_STORE_ARGS
	#endif

	#ifdef PERSISTENT_APPLY_ARGS
	#undef PERSISTENT_APPLY_ARGS
	#endif

	#ifdef PERSISTENT_NO_STORE
	#undef PERSISTENT_NO_STORE
	#endif

	#ifdef PERSISTENT_NO_APPLY
	#undef PERSISTENT_NO_APPLY
	#endif

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
