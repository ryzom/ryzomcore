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


#include "stdpch.h"
#include "nel/gui/reflect.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	// Yoyo: Act like a singleton, else registerClass may crash.
	CReflectSystem::TClassMap *CReflectSystem::_ClassMap= NULL;

	// hack to register the root class at startup
	static const struct CRootReflectableClassRegister
	{
		CRootReflectableClassRegister()
		{
			TReflectedProperties props;
			CReflectSystem::registerClass("CReflectable", "", props);
		}
	} _RootReflectableClassRegisterInstance;


	//===================================================================================
	// release memory
	void CReflectSystem::release()
	{
		delete _ClassMap;
		_ClassMap = NULL;
	}

	//===================================================================================
	void CReflectSystem::registerClass(const std::string &className, const std::string &parentName, const TReflectedProperties properties)
	{
		if(!_ClassMap)	_ClassMap= new TClassMap;

		TClassMap::const_iterator it = _ClassMap->find(className);
		if (it != _ClassMap->end())
		{
			nlerror("CReflectSystem::registerClass : Class registered twice : %s!", className.c_str());
		}
		CClassInfo &ci = (*_ClassMap)[className];
		ci.Properties = properties;
		ci.ClassName = className;
		for(uint k = 0; k < ci.Properties.size(); ++k)
		{
			ci.Properties[k].ParentClass = &ci;
		}
		if (parentName.empty())
		{
			ci.ParentClass = NULL;
		}
		else
		{
			it = _ClassMap->find(parentName);
			if (it == _ClassMap->end())
			{
				nlerror("CReflectSystem::registerClass : Parent class %s not found", parentName.c_str());
			}
			ci.ParentClass = &(it->second);
		}
	}

	//===================================================================================
	const CReflectedProperty *CReflectSystem::getProperty(const std::string &className, const std::string &propertyName, bool dspWarning)
	{
		if(!_ClassMap)	_ClassMap= new TClassMap;

		TClassMap::const_iterator it = _ClassMap->find(className);
		if (it == _ClassMap->end())
		{
			nlwarning("CReflectSystem::getProperty : Unkwown class : %s", className.c_str());
			return NULL;
		}
		const CClassInfo *ci = &it->second;
		while (ci)
		{
			// Linear search should suffice for now
			for(uint k = 0; k < ci->Properties.size(); ++k)
			{
				if (ci->Properties[k].Name == propertyName)
				{
					return &(ci->Properties[k]);
				}
			}
			// search in parent
			ci = ci->ParentClass;
		}
		//\ TODO nico : possible optimization : instead of going up in the parents when
		//              searching for a property, it would be simpler to concatenate properties
		//              from parent class at registration.
		//              All that would be left at the end would be a hash_map of properties ...

		if(dspWarning)
			nlwarning("CReflectSystem::getProperty : %s is not a property of class : %s", propertyName.c_str(), className.c_str());
		return NULL;
	}


	//===================================================================================
	const CClassInfo *CReflectable::getClassInfo()
	{
		if (!CReflectSystem::getClassMap()) return NULL;
		// TODO nico : a possible optimization would be to use the address of the static function
		// 'getReflectedProperties' as a key into the CClassInfo map. This pointer uniquely identify
		// classes that export properties
		CReflectSystem::TClassMap::const_iterator it = CReflectSystem::getClassMap()->find(this->getReflectedClassName());
		if (it == CReflectSystem::getClassMap()->end())
		{
			return NULL;
		}
		return &(it->second);
	}

	//===================================================================================
	const CReflectedProperty *CReflectable::getReflectedProperty(const std::string &propertyName, bool dspWarning) const
	{
		return 	CReflectSystem::getProperty(this->getReflectedClassName(), propertyName, dspWarning);
	}

}

#include "nel/gui/lua_manager.h"

namespace NLGUI
{
	CReflectableRefPtrTarget::~CReflectableRefPtrTarget()
	{
		CLuaState *lua= CLuaManager::getInstance().getLuaState();
		if(!lua)
			return;
		CLuaStackChecker lsc(lua);
		// remove from the lua registry if i'm in
		lua->pushLightUserData((void *) this);
		lua->getTable(LUA_REGISTRYINDEX);
		if (!lua->isNil(-1))
		{
			lua->pop();
			lua->pushLightUserData((void *) this);
			lua->pushNil();
			lua->setTable(LUA_REGISTRYINDEX);
		}
		else
		{
			lua->pop();
		}
	}

	/**
	  * Data structure pushed in lua (a userdata) to access CReflectableRefPtrTarget derived objects
	  * These includes element of the GUI.
	  * if holds a pointer to the reflectable object, and
	  * a cache to its CClassInfo for fast access to exported properties
	  * \see reflect.h
	  */

	//

	inline const CClassInfo &CReflectableLuaRef::getClassInfo() const
	{
		nlassert(Ptr); // class info should not be accessed for a null ptr
		if (_ClassInfo) return *_ClassInfo;
		_ClassInfo = Ptr->getClassInfo();
		return *_ClassInfo;
	}

	const CReflectedProperty *CReflectableLuaRef::getProp(const char *luaStringPtr) const
	{
		const CClassInfo &ci = getClassInfo();
		CClassInfo::TLuaStrToPropMap::const_iterator it = ci.LuaStrToProp.find(luaStringPtr);
		if (it != ci.LuaStrToProp.end())
		{
			return it->second.Prop;
		}
		// slowly retrieve property, and store in cache
		// NB nico : this could also be done at startup...
		const CReflectedProperty *prop = CReflectSystem::getProperty(ci.ClassName, luaStringPtr, false);
		if (!prop) return NULL;
		CLuaIndexedProperty lip;
		lip.Id = CLuaString(luaStringPtr); // keep a ref on the lua string to ensure that its pointer always remains valid
		lip.Prop = prop;
		ci.LuaStrToProp[luaStringPtr] = lip;
		return prop;
	}

}
