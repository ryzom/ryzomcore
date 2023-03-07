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

#include "com_lua_module.h"

#include "nel/misc/debug.h"
#include "nel/misc/path.h"
//#include "nel/misc/string.h"

#include "simulated_dmc.h"
//#include "palette.h"
//#include "property_accessor.h"
#include "simulated_client_edition_module.h"
#include "simulated_client_animation_module.h"
//#include "client_admin_module.h"

#include "r2_share/object.h"
#include "r2_share/r2_lua.h"
#include "r2_share/scenario.h"

#include <iostream>
#include <assert.h>
#include <vector>
#include <sstream>
#include <string>

// TMP TMP
//#include "lua_helper.h"
//#include "lua_ihm.h"

using namespace R2;


std::map<lua_State*, CComLuaModule*> CComLuaModule::_Instance;

#define CHECK_LUA_ARG_COUNT(count, funcName) if (args != count) { nlwarning("%d args required for lua function %s.", count, funcName); return 0; }
#define CHECK_LUA_ARG_COUNT_MAX(count, funcName) if (args > count) { nlwarning("Lua function %s accept at most %d arguments.", funcName, count); return 0; }


CComLuaModule::CComLuaModule(CDynamicMapClient* client, lua_State *luaState /*= NULL*/)
{		
	_Client = client;

	if (!luaState)
	{
		#ifdef LUA_NEVRAX_VERSION
			_LuaState = lua_open(NULL, NULL);
		#else
			_LuaState = lua_open();
		#endif
		_LuaOwnerShip = false;
		luaopen_base(_LuaState);
		luaopen_table(_LuaState);
		luaopen_io(_LuaState);
		luaopen_string(_LuaState);
		luaopen_math(_LuaState);
		luaopen_debug(_LuaState);
		_LuaOwnerShip = true;
	}
	else
	{
		_LuaState = luaState;
		_LuaOwnerShip = false;
	}
	
	_Instance[_LuaState] = this;
	initLuaLib();	

	nlassert(_LuaState);			

}

void CComLuaModule::initLuaLib()
{
	const luaL_reg methods[] =
	{
		{"updateScenario", CComLuaModule::luaUpdateScenario},
		{"requestUpdateRtScenario", CComLuaModule::luaRequestUpdateRtScenario},
		{"requestCreateScenario", CComLuaModule::luaRequestCreateScenario},
		{"requestMapConnection", CComLuaModule::luaRequestMapConnection},
		{"requestReconnection", CComLuaModule::luaRequestReconnection},
		{"requestListAdventure", CComLuaModule::luaRequestListAdventure},

		{"requestInsertNode", CComLuaModule::luaRequestInsertNode},
		{"requestInsertGhostNode", CComLuaModule::luaRequestInsertGhostNode},
		{"requestSetNode", CComLuaModule::luaRequestSetNode},
		{"requestEraseNode", CComLuaModule::luaRequestEraseNode},
		{"requestMoveNode", CComLuaModule::luaRequestMoveNode},
		{"requestGoLive", CComLuaModule::luaRequestGoLive},
		{"requestStopLive", CComLuaModule::luaRequestStopLive},
		{"requestStartAct", CComLuaModule::luaRequestStartAct},
		{"requestStopAct", CComLuaModule::luaRequestStopAct},
		{"requestStopAct", CComLuaModule::luaRequestStopAct},
		{"requestCreatePrimitives", CComLuaModule::luaRequestCreatePrimitives},
		{"requestSetWeather", CComLuaModule::luaRequestSetWeather},
		
		{"requestTpPosition", CComLuaModule::luaRequestTpPosition},
		{"requestDespawnEntity", CComLuaModule::luaRequestDespawnEntity},
		{"requestDespawnGrp", CComLuaModule::luaRequestDespawnGrp},
				
		{"requestStopAct", CComLuaModule::luaRequestStopAct},		
		{"newComponent", CComLuaModule::luaNewComponent},		
		{"requestTranslateFeatures", CComLuaModule::luaRequestTranslateFeatures},
		{"registerGenerator", CComLuaModule::luaRegisterGenerator},
		{"show", CComLuaModule::luaShow},
		{"addPaletteElement", CComLuaModule::luaAddPaletteElement},
		{"getPaletteElement", CComLuaModule::luaGetPaletteElement},
		{"getPropertyValue", CComLuaModule::luaGetPropertyValue},
		{"getScenarioObj",CComLuaModule::luaGetScenarioObj},
		/*{"getPropertyList", CComLuaModule::luaGetPropertyList},*/
		{"doFile2", CComLuaModule::luaDoFile2},
		{"print", CComLuaModule::luaPrint},
		{"save", CComLuaModule::luaSave},
		{"load",CComLuaModule::luaLoad},		
		{"requestTalkAs",CComLuaModule::luaTalkAs},
		{"requestStopTalk",CComLuaModule::luaStopTalkAs},
		{"requestStringTable",CComLuaModule::luaRequestStringTable},
		{"requestSetStringValue", CComLuaModule::luaRequestSetStringValue},
		{"requestStringValue", CComLuaModule::luaRequestStringValue},
		{"requestIdList", CComLuaModule::luaRequestIdList},
		
		{"getNamespace", CComLuaModule::luaGetNamespace},
		{"getIslandsLocation", CComLuaModule::luaGetIslandsLocation},

		{0,0}

	};	
	int initialStackSize = lua_gettop(_LuaState);
	luaL_openlib(_LuaState, R2_LUA_PATH, methods, 0);
	lua_settop(_LuaState, initialStackSize);	
	// load r2 features & components	
	doFile( "r2_core.lua" );
}


void CComLuaModule::doFile(const std::string& filename)
{
	std::string filePath = NLMISC::CPath::lookup(filename, false, true);
	if (filePath.empty())
	{
		nlwarning("Can't find %s", filename.c_str());
		return;
	}
	std::string str = NLMISC::toString("dofile(\"%s\")", filePath.c_str() );
	std::string errorMsg;
	runLuaScript(str.c_str(), errorMsg);	
}

bool CComLuaModule::runLuaScript(const std::string& script, std::string& erromsg)
{
	const char *buff = script.c_str();
	size_t size = script.size();
	const char *name = script.c_str();
	
	int status = luaL_loadbuffer(_LuaState, buff, size, name);
	if (status == 0)
	{ 
		status = lua_pcall(_LuaState, 0, LUA_MULTRET, 0);  /* call main */
	}

	if (status != 0)
	{
		erromsg = NLMISC::toString("%s\n", lua_tostring(_LuaState, -1));
		lua_pop(_LuaState, 1);
		return false;
	}

	return true;
}
sint CComLuaModule::luaDoFile2(lua_State* state)
{
	sint args = lua_gettop(state);
	nlassert(args == 1);
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string key( lua_tostring(state, 1) );
	this2->doFile(key);	
	return  0;
}

//obsolete
void CComLuaModule::loadFeatures()
{
	doFile("r2_core.lua");
}

CComLuaModule* CComLuaModule::getInstance(lua_State* state)
{
	std::map<lua_State*, CComLuaModule*>::const_iterator found(_Instance.find(state));
	if (found != _Instance.end()) 
	{
		return found->second;
	}
	return 0;
}

void CComLuaModule::callTranslateFeatures(CObject* scenario)
{
	if (!scenario)
	{
		nlwarning("<CComLuaModule::callTranslateFeatures> called on NULL scenario");
		return;
	}	
	lua_getglobal(_LuaState, "r2");
	lua_pushstring(_LuaState, "translateFeatures");
	lua_gettable(_LuaState, -2);
	setObjectToLua(_LuaState, scenario);
	lua_call(_LuaState, 1, 0);
}
	

CObject* CComLuaModule::translateFeatures(CObject* hlScenario, std::string& errorMsg) const
{
	if (!hlScenario)
	{
		errorMsg = "<CComLuaModule::translateFeatures> called on NULL scenario";
		return 0;
	}
	lua_getglobal(_LuaState, "r2");
	lua_pushstring(_LuaState, "doTranslateFeatures");
	lua_gettable(_LuaState, -2);
	setObjectToLua(_LuaState, hlScenario);
	if ( lua_pcall(_LuaState, 1, 1, 0) !=0 ) 
	{
		errorMsg = NLMISC::toString( "error running function 'doTranslateFeatures': %s", lua_tostring(_LuaState, -1));
		return 0;
	}
	CObject* ret = getObjectFromLua(_LuaState, -1);	
	return ret;
}

sint CComLuaModule::luaAddPaletteElement(lua_State* state)
{
	
	luaL_checktype(state, 1, LUA_TSTRING);
	luaL_checktype(state, 2, LUA_TTABLE);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string key( lua_tostring(state, 1) );	
	CObject* object = this2->getObjectFromLua(state, 2);	
	this2->_Client->addPaletteElement(key, object);
	return  0;	
}

sint CComLuaModule::luaGetPropertyValue(lua_State* state)
{
	sint args = lua_gettop(state);
	nlassert(args == 2);
	luaL_checktype(state, 1, LUA_TTABLE);
	luaL_checktype(state, 2, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	CObject* object = this2->getObjectFromLua(state, 1);
	std::string attrName( lua_tostring(state, 2) );
	CObject* toRet = this2->_Client->getPropertyValue(object, attrName);
	this2->setObjectToLua(state, toRet);
	delete object;	
	return  1;	
}

/*sint CComLuaModule::luaGetPropertyList(lua_State* state)
{
	sint args = lua_gettop(state);
	nlassert(args == 1);
	luaL_checktype(state, 1, LUA_TTABLE);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	CObject* object = this2->getObjectFromLua(state, 1);	
	CObject* toRet = this2->_Client->getPropertyList(object);
	this2->setObjectToLua(state, toRet);
	delete toRet;
	return 1;	
}*/
sint CComLuaModule::luaGetPaletteElement(lua_State* state)
{

	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	assert(this2);
	std::string key( lua_tostring(state, 1) );
	CObject* object = this2->_Client->getPaletteElement(key);
	this2->setObjectToLua(state, object);	
	return  1;	
}

sint CComLuaModule::luaRequestTranslateFeatures(lua_State* state)
{
	
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestTranslateFeatures();
	return 0;
}

sint CComLuaModule::luaRequestGoLive(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestGoTest();
	return 0;
}


sint CComLuaModule::luaRequestCreatePrimitives(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestCreatePrimitives();
	return 0;
}


sint CComLuaModule::luaRequestSetWeather(lua_State* state)
{
	luaL_checktype(state, 1, LUA_TNUMBER);
	uint16 weatherValue = (uint16) lua_tonumber(state, 1);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getAnimationModule().requestSetWeather(weatherValue);
	return 0;	
}

sint CComLuaModule::luaRequestStopLive(lua_State* state)
{
	
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestStopTest();
	return 0;
}

sint CComLuaModule::luaRequestStartAct(lua_State* state)
{	
	luaL_checktype(state, 1, LUA_TNUMBER);
	uint32 actId(static_cast<uint32>(lua_tonumber(state, 1)));
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getAnimationModule().requestStartAct(actId);
	return 0;
}

sint CComLuaModule::luaRequestStopAct(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getAnimationModule().requestStopAct();
	return 0;
}

sint CComLuaModule::luaNewComponent(lua_State* state)
{
	
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	
	luaL_checktype(state, 1, LUA_TSTRING);
	std::string key(lua_tostring(state, 1));
	CObject* object = this2->_Client->newComponent(key);
	if (!object)
	{
		lua_pushnil(state);
		return 1;
	}
	setObjectToLua(state, object);
	delete object;
	return 1;
}

sint CComLuaModule::luaRegisterGenerator(lua_State* state)
{
	luaL_checktype(state, 1, LUA_TTABLE);
	CComLuaModule* this2 = getInstance(state);
	assert(this2);
	CObject* object = this2->getObjectFromLua(state, 1);
 	this2->_Client->registerGenerator(object);
	return  0;	
}

sint CComLuaModule::luaShow(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	assert(this2);
	this2->_Client->show();
	return  0;	
}

sint CComLuaModule::luaRequestUpdateRtScenario(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	assert(this2);
	CObject* object = this2->getObjectFromLua(state, 1);
	assert(object);
	this2->_Client->requestUpdateRtScenario(object);
	return 0;
}

sint CComLuaModule::luaRequestCreateScenario(lua_State* state)
{
	luaL_checktype(state, 1, LUA_TTABLE);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	CObject* object = this2->getObjectFromLua(state, 1);
	nlassert(object);
	this2->_Client->requestCreateScenario(object);
	delete object;
	return 0;
}

sint CComLuaModule::luaPrint(lua_State* state)
{
	
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "print");	

//	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	CObject* object = this2->getObjectFromLua(state, 1);
	if(!object)
	{
		nlinfo("nil");
		return 0;	
	}
	std::stringstream ss;
	std::string s;
	object->serialize(ss);
	while (std::getline(ss, s))
	{
		nlinfo("%s", s.c_str());
	}
	

	//this2->_Client->requestCreateScenario(object);
	delete object;
	return 0;
}



sint CComLuaModule::luaRequestMapConnection(lua_State* state)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "requestMapConnection");	
	luaL_checktype(state, 1, LUA_TNUMBER);

	uint32 adventureId = static_cast<uint32>( lua_tonumber(state, 1) );

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestMapConnection(adventureId, true);

	return 0;
}



sint CComLuaModule::luaRequestReconnection(lua_State* state)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "requestReconnection");	

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestReconnection();

	return 0;
}


sint CComLuaModule::luaRequestListAdventure(lua_State* state)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "requestListAdventure");	

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
//	AJM:	this2->_Client->getAdminModule().requestListAdventure();

	return 0;
}


sint CComLuaModule::requestInsertNode(lua_State* state, bool isGhost)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(5, "requestInsertNode");	
	luaL_checktype(state, 1, LUA_TSTRING);
	luaL_checktype(state, 2, LUA_TSTRING);
	luaL_checktype(state, 3, LUA_TNUMBER);
	luaL_checktype(state, 4, LUA_TSTRING);
	luaL_checkany(state, 5); //TODO just string, number and table

	std::string instanceId(lua_tostring(state, 1));
	std::string attrName(lua_tostring(state, 2));
	sint position(static_cast<sint>(lua_tonumber(state, 3)));
	std::string key(lua_tostring(state, 4));
	CObject* value = getObjectFromLua(state, 5);
//	value->setGhost(isGhost);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestInsertNode(
		instanceId, attrName, position, key, value);
	delete value;
	return 0;
}


sint CComLuaModule::luaRequestInsertNode(lua_State* state)
{
	return requestInsertNode(state, false);
}

sint CComLuaModule::luaRequestInsertGhostNode(lua_State* state)
{
	return requestInsertNode(state, true);
}

sint CComLuaModule::luaRequestSetNode(lua_State* state)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(3, "requestSetNode");	
	luaL_checktype(state, 1, LUA_TSTRING);
	luaL_checktype(state, 2, LUA_TSTRING);
	luaL_checkany(state, 3); //TODO just string, number and table

	std::string instanceId(lua_tostring(state, 1));
	std::string attrName(lua_tostring(state, 2));
	CObject* value = getObjectFromLua(state, 3);
	if (value == NULL)
	{
		nlwarning("requestSetNode : bad type for argument 3");
		return 0;
	}
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestSetNode(	instanceId, attrName, value);
	delete value;
	return 0;
}

sint CComLuaModule::luaRequestEraseNode(lua_State* state)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT_MAX(3, "requestEraseNode")	
	luaL_checktype(state, 1, LUA_TSTRING);
	if (args>1) { luaL_checktype(state, 2, LUA_TSTRING); }
	if (args>2) { luaL_checknumber(state, 3); }

	std::string instanceId(lua_tostring(state, 1));
	std::string attrName = "";
	sint position = -1;
	if (args>1){ attrName = lua_tostring(state, 2);}
	if (args>2){ position = static_cast<sint>(lua_tonumber(state, 3));} 
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestEraseNode(	instanceId, attrName, position);
	return 0;

}


sint CComLuaModule::luaRequestMoveNode(lua_State* state)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(6, "requestMoveNode");
	luaL_checktype(state, 1, LUA_TSTRING);
	luaL_checktype(state, 2, LUA_TSTRING);
	luaL_checktype(state, 3, LUA_TNUMBER);
	luaL_checktype(state, 4, LUA_TSTRING);
	luaL_checktype(state, 5, LUA_TSTRING);
	luaL_checktype(state, 6, LUA_TNUMBER);


	luaL_checkany(state, 3); //TODO just string, number and table

	std::string instanceId(lua_tostring(state, 1));
	std::string attrName(lua_tostring(state, 2));
	sint position = static_cast<sint>(lua_tonumber(state, 3));

	std::string instanceId2(lua_tostring(state, 4));
	std::string attrName2(lua_tostring(state, 5));
	sint position2 = static_cast<sint>(lua_tonumber(state, 6));
	
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestMoveNode(	instanceId, attrName, position, instanceId2, attrName2, position2);
	return 0;

}


sint CComLuaModule::luaRequestTpPosition(lua_State* state)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(2, "requestTpPosition");
	luaL_checktype(state, 1, LUA_TNUMBER);
	luaL_checktype(state, 2, LUA_TNUMBER);

	float x = static_cast<float>(lua_tonumber(state, 1));
	float y = static_cast<float>(lua_tonumber(state, 2));
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getAnimationModule().requestTpPosition(x, y);
	return 0;

}

sint CComLuaModule::luaRequestDespawnEntity(lua_State* state)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(2, "requestDespawnEntity");
	luaL_checktype(state, 1, LUA_TSTRING);

	std::string npcId = lua_tostring(state, 1);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getAnimationModule().requestDespawnEntity(npcId);
	return 0;

}

sint CComLuaModule::luaRequestDespawnGrp(lua_State* state)
{
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(2, "requestDespawnGrp");
	luaL_checktype(state, 1, LUA_TSTRING);
	
	std::string grpId = lua_tostring(state, 1);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getAnimationModule().requestDespawnEntity(grpId);
	return 0;
}

void CComLuaModule::setObjectToLua(lua_State* state, CObject* object)
{
	if (!object)
	{
		lua_pushnil(state);
		return;
	}
	if ( object->isNumber() )
	{
		lua_pushnumber(state, object->toNumber());
		return;
	}

	if (object->isRefId())
	{
		int initialStackSize = lua_gettop(state);
		
		lua_pushvalue(state, LUA_GLOBALSINDEX); // _G
		
		lua_pushstring(state, "r2");			// _G, "r2"
		lua_gettable(state, -2);                // G, r2
		lua_pushstring(state, "RefIdMetatable");		// _G, r2, "RefIdMetatable"
		lua_gettable(state, -2);                // _G, r2, RefIdMetatable
		lua_insert(state, -3);					// RefIdMetatable, _G, r2
		lua_pop(state, 2);

		nlassert(lua_gettop(state) == initialStackSize + 1);
		lua_newtable(state); // RefIdMetatable, {}
		lua_pushstring(state, "Value"); // RefIdMetatable, {}, "Value"
		lua_pushstring(state, object->toString().c_str()); // RefIdMetatable, {}, "Value", Value
		lua_settable(state, -3); // RefIdMetatable, { "Value" = Value }
		lua_insert(state, -2);   // { "Value" = Value }, RefIdMetatable
		lua_setmetatable(state, -2); // { "Value" = Value } + metatable
		int finalStackSize = lua_gettop(state);
		nlassert(finalStackSize == initialStackSize + 1);
		return;
	}
	
	if ( object->isString() )
	{
		lua_pushstring(state, object->toString().c_str());
		return;
	}
	
	if ( object->isTable() )
	{
		lua_newtable(state);
		uint32 first = 0;
		uint32 last = object->getSize();
		uint32 arraySize = 0;
		uint32 arrayIndex = 0;

		for ( ; first != last; ++first)
		{
			if (object->getKey(first).empty()) { ++arraySize;}
		}

		std::vector<std::string> keys;
		for (first=0 ; first != last; ++first)
		{
			std::string key = object->getKey(first);
			CObject *value =  object->getValue(first);
			if (!key.empty())
			{
				lua_pushstring(state, key.c_str());
				setObjectToLua(state, value);
				lua_settable(state, -3);
				keys.push_back(key);
			}
			else
			{
				++arrayIndex;
				setObjectToLua(state, value);
				lua_rawseti(state, -2, arrayIndex);
			}
		}

		if (0)
		{
		
			first = 0;
			last = keys.size();
	//				if (!keys.empty())
			{
				lua_pushstring(state, "Keys");
				lua_newtable(state);
				for (; first != last; ++first)
				{
					lua_pushstring(state, keys[first].c_str());
					lua_rawseti(state, -2, first+1);
				}
				luaL_setn(state, -1, last);
				lua_settable(state, -3);
				if (arraySize > 0)
				{
					luaL_setn(state, -1, arraySize);
				}
			}
		}
	}
	else
	{
		nlwarning("error! object is not a string, not a number, not a table!");
		lua_pushnil(state);
	}
}


CObject* CComLuaModule::getObjectFromLua(lua_State* state, sint idx)
{	
	lua_pushvalue(state, idx);

	// special case for RefID
	if (lua_type(state, -1) == LUA_TTABLE)
	{		
		if (lua_getmetatable(state, -1))
		{
			
			lua_pushvalue(state, LUA_GLOBALSINDEX); // obj, mt, _G
			
			lua_pushstring(state, "r2");					// obj, mt, _G, "r2"
			
			lua_gettable(state, -2);                    // obj, mt, _G, r2
			
			lua_pushstring(state, "RefIdMetatable");		// obj, mt, _G, r2, "RefIdMetatable"
			
			lua_gettable(state, -2);                    // obj, mt, _G, r2, RefIdMetatable
			bool equal = lua_rawequal(state, -1, -4) != 0;
			if (equal)
			{
				
				lua_pop(state, 4); // obj
				
				lua_pushstring(state, "Value"); // obj, "Value"
				
				lua_gettable(state, -2); // obj, value
				CObject *result = 0;
				if ( lua_isstring (state, -1) )
				{	const char* str = lua_tostring(state, -1);
					if (str)
					{
//						result = new CObjectRefIdClient(str);
					}
					else
					{
						nlwarning("RefId error invalid string");
						nlstop;
					}
					
				}
				else
				{
					nlwarning("RefId not a string");
					nlstop;
				}

			
				
				lua_pop(state, 2);
				return result;
			}
			else
			{
				
				lua_pop(state, 4); // obj
			}
			
		}		
	}	
	switch (lua_type(state, -1))
	{
		case LUA_TNUMBER:
		{
			double value = lua_tonumber(state, -1);
			lua_pop(state, 1);
			return new CObjectNumber(value);
		}
		break;
		
		case LUA_TSTRING:
		{
			std::string value = lua_tostring(state, -1);
			lua_pop(state, 1);
			return new CObjectString(value);
		}
		break;
				
		case LUA_TTABLE: 
		{
			CObjectTable* table = new CObjectTable();	//Client();

			lua_pushnil(state);
			while (lua_next(state, -2) != 0)
			{
				std::string key = "";
				if ( lua_type(state, -2) == LUA_TSTRING)
				{
					key = lua_tostring(state, -2);
				}
				CObject* object = getObjectFromLua(state, -1);
				if (object)
				{
					table->add(key, object);
				}
				lua_pop(state, 1);					
			}
			lua_pop(state, 1);
			table->sort();
			return table;
		}				
		break;
		default:
			lua_pop(state, 1);
			// other types such as functions are ignored
			return NULL;
		break;
	}
	return 0;
}


CObject* CComLuaModule::loadLocal(const std::string& filename)
{
	CObject* object = NULL;
	if (lua_dofile(_LuaState, filename.c_str()) == 0)
	{
		lua_getglobal(_LuaState, "scenario");
		
		if (lua_type(_LuaState, -1) == LUA_TTABLE)
		{
			object = getObjectFromLua(_LuaState);
		}
	}
	if (!object)
	{
		nlwarning("Error while loading %s", filename.c_str());
	}
	return object;
}


bool CComLuaModule::load(const std::string& filename)
{
	
	CObject* object = loadLocal(filename);
	if (!object)
	{
		nlwarning("Error while loading %s", filename.c_str());
		return false;
	}
	_Client->requestUploadScenario(object);		
	return true;
}





sint CComLuaModule::luaUpdateScenario(lua_State* state)
{
	luaL_checktype(state, 1, LUA_TTABLE);

//			lua_pushvalue(state, 1);
	CObject* object = getObjectFromLua(state);
	lua_pushliteral(state, "tmp1");
	lua_pushliteral(state, "tmp1");
	lua_pushliteral(state, "tmp1");

	lua_getglobal(state, "write_table");
	setObjectToLua(state, object);
	lua_call(state, 1, 0);
	
//			lua_pop(state, 1);


	std::cout << "updateScenario" << std::endl;
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2->_LuaState == state);

	return 0;
}

CComLuaModule::~CComLuaModule()
{
	if (_LuaOwnerShip)
	{
		lua_close(_LuaState);
	}
}



sint CComLuaModule::luaLoad(lua_State* state)
{
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string filename( lua_tostring(state, 1) );
	bool ok = this2->_Client->load(filename);
	if (!ok)
	{
		lua_pushnil(state);
	}
	else
	{
		lua_pushnumber(state, 1);
	}
	return 1;
}

 sint CComLuaModule::luaSave(lua_State* state)
{
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string filename( lua_tostring(state, 1) );
	this2->_Client->save(filename);
	return 0;
}

sint CComLuaModule::luaTalkAs(lua_State* state)
{
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string npcname( lua_tostring(state, 1) );
	nlinfo(("luaTalkAs:: "+npcname).c_str());
	this2->_Client->getAnimationModule().requestTalkAs(npcname);
	return 0;
}


sint CComLuaModule::luaStopTalkAs(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getAnimationModule().requestStopTalkAs();
	return 0;
}

sint CComLuaModule::luaRequestStringTable(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getAnimationModule().requestStringTable();
	return 0;
}

sint CComLuaModule::luaRequestSetStringValue(lua_State* state)
{
	luaL_checktype(state, 1, LUA_TSTRING);
	luaL_checktype(state, 2, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string localId( lua_tostring(state, 1) );
	std::string value(lua_tostring(state,2));
	this2->_Client->getAnimationModule().requestSetStringValue(localId,value);
	return 0;
}

sint CComLuaModule::luaRequestStringValue(lua_State* state)
{
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string localId( lua_tostring(state, 1) );
	this2->_Client->getAnimationModule().requestStringValue(localId);
	return 0;
}

sint CComLuaModule::luaRequestIdList(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getAnimationModule().requestIdList();
	return 0;
}



sint CComLuaModule::luaGetScenarioObj(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	CObject* obj = this2->_Client->getCurrentScenario()->getHighLevel();
	setObjectToLua(state,obj);
	return 1;
}

sint CComLuaModule::luaGetNamespace(lua_State* state)

{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	TSessionId sessionId = this2->_Client->getEditionModule().getCurrentAdventureId();
	std::string value= NLMISC::toString("r2_%04d_",sessionId);
	lua_pushstring(state, value.c_str());
	return 1;
}


sint CComLuaModule::luaGetIslandsLocation(lua_State* state)

{
	std::vector<std::string> locations;
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
//	AJM:	this2->_Client->getAdminModule().getIslandsLocation(locations);

	
		
	uint first(0), last( locations.size() ) ;
	
			
	//	lua_pushstring(state, "Keys");
	lua_newtable(state);
	for (; first != last; ++first)
	{
		lua_pushnumber(state, first+1);
		lua_pushstring(state, locations[first].c_str());
		lua_settable(state, -3);
	}
	
	return 1;
}




