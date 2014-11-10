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

#include "com_lua_module.h"

#include <assert.h>
#include <vector>
#include <string>

#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/i18n.h"
#include "nel/misc/file.h"
//#include "nel/misc/string.h"

#include "dmc.h"
#include "palette.h"
#include "property_accessor.h"
#include "client_edition_module.h"
#include "../object_factory_client.h"
#include "../editor.h"
#include "../../net_manager.h"
#include "nel/gui/lua_ihm.h"
#include "../../interface_v3/lua_ihm_ryzom.h"

#include "game_share/object.h"
#include "../r2_lua.h"
#include "game_share/scenario.h"
#include "game_share/ring_access.h"

#include <assert.h>
#include <vector>
#include <string>


#include "nel/gui/lua_helper.h"
using namespace NLGUI;
#include "nel/gui/lua_ihm.h"

#include "../../entities.h"

using namespace NLMISC;
using namespace R2;



std::map<lua_State*, CComLuaModule*> CComLuaModule::_Instance;

#define CHECK_LUA_ARG_COUNT(count, funcName) if (args != count) { nlwarning("%d args required for lua function %s.", count, funcName); return 0; }
#define CHECK_LUA_ARG_COUNT_MAX(count, funcName) if (args > count) { nlwarning("Lua function %s accept at most %d arguments.", funcName, count); return 0; }


extern uint8 PlayerSelectedSlot;

CComLuaModule::CComLuaModule(CDynamicMapClient* client, lua_State *luaState /*= NULL*/)
{
	_Client = client;

	if (!luaState)
	{
		#ifdef LUA_NEVRAX_VERSION
			_LuaState = lua_open(NULL, NULL);
		#else
			_LuaState = luaL_newstate();
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
	//H_AUTO(R2_CComLuaModule_initLuaLib)
	const luaL_Reg methods[] =
	{
		{"updateScenario", CComLuaModule::luaUpdateScenario},
		{"requestUpdateRtScenario", CComLuaModule::luaRequestUpdateRtScenario},
		{"requestCreateScenario", CComLuaModule::luaRequestCreateScenario},
		{"requestUploadCurrentScenario", CComLuaModule::luaRequestUploadCurrentScenario},
		{"requestMapConnection", CComLuaModule::luaRequestMapConnection},
		{"requestReconnection", CComLuaModule::luaRequestReconnection},

		{"requestInsertNode", CComLuaModule::luaRequestInsertNode},
		{"requestInsertGhostNode", CComLuaModule::luaRequestInsertGhostNode},

		{"requestSetNode", CComLuaModule::luaRequestSetNode},
		{"requestSetGhostNode", CComLuaModule::luaRequestSetGhostNode},

		{"requestEraseNode", CComLuaModule::luaRequestEraseNode},
		{"requestMoveNode", CComLuaModule::luaRequestMoveNode},
		{"requestNewAction", CComLuaModule::luaRequestNewAction},
		{"requestNewMultiAction", CComLuaModule::luaRequestNewMultiAction},
		{"requestForceEndMultiAction", CComLuaModule::luaRequestForceEndMultiAction},
		{"requestNewPendingMultiAction", CComLuaModule::luaRequestNewPendingMultiAction},
		{"requestCancelAction", CComLuaModule::luaRequestCancelAction},
		{"clearActionHistoric", CComLuaModule::luaRequestClearActionHistoric},
		{"requestEndAction", CComLuaModule::luaRequestEndAction},
		{"requestNewPendingAction", CComLuaModule::luaRequestNewPendingAction},
		{"requestStopLive", CComLuaModule::luaRequestStopLive},
		{"requestStartAct", CComLuaModule::luaRequestStartAct},
		{"requestStopAct", CComLuaModule::luaRequestStopAct},
		{"requestStopAct", CComLuaModule::luaRequestStopAct},
		{"requestCreatePrimitives", CComLuaModule::luaRequestCreatePrimitives},
		{"requestSetWeather", CComLuaModule::luaRequestSetWeather},
		{"requestSetSeason", CComLuaModule::luaRequestSetSeason},

		{"requestTpPosition", CComLuaModule::luaRequestTpPosition},

		{"requestStopAct", CComLuaModule::luaRequestStopAct},
		{"newComponent", CComLuaModule::luaNewComponent},
		{"requestTranslateFeatures", CComLuaModule::luaRequestTranslateFeatures},
		{"registerGenerator", CComLuaModule::luaRegisterGenerator},
		{"show", CComLuaModule::luaShow},

		{"addPaletteElement", CComLuaModule::luaAddPaletteElement},
		{"getPaletteElement", CComLuaModule::luaGetPaletteElement},
		{"isInPalette", CComLuaModule::luaIsInPalette},

		{"getPropertyValue", CComLuaModule::luaGetPropertyValue},
		{"getScenarioObj",CComLuaModule::luaGetScenarioObj},
		/*{"getPropertyList", CComLuaModule::luaGetPropertyList},*/
		{"doFile2", CComLuaModule::luaDoFile2},
		{"print", CComLuaModule::luaPrint},
		{"save", CComLuaModule::luaSave},
		{"load",CComLuaModule::luaLoad},
		{"loadUserComponentFile",CComLuaModule::luaLoadUserComponent},
		{"saveUserComponent", CComLuaModule::luaSaveUserComponent},
		{"loadAnimation", CComLuaModule::luaLoadAnimation},

		{"requestTalkAs",CComLuaModule::luaTalkAs},
		{"requestStopTalk",CComLuaModule::luaStopTalkAs},
		{"requestStringTable",CComLuaModule::luaRequestStringTable},
		{"requestSetStringValue", CComLuaModule::luaRequestSetStringValue},
		{"requestStringValue", CComLuaModule::luaRequestStringValue},
		{"requestIdList", CComLuaModule::luaRequestIdList},

		{"getNamespace", CComLuaModule::luaGetNamespace},
		{"getIslandsLocation", CComLuaModule::luaGetIslandsLocation},
		{"objectToLua", CComLuaModule::luaObjectToLua},
		{"readUserComponentFile", CComLuaModule::luaReadUserComponentFile},
		{"registerUserComponent", CComLuaModule::luaRegisterUserComponent},
		{"updateUserComponentsInfo", CComLuaModule::luaUpdateUserComponentsInfo},
		{"saveUserComponentFile", CComLuaModule::luaSaveUserComponentFile},

		{"getUserTriggers", CComLuaModule::luaGetUserTriggers},
		{"getRuntimeActs", CComLuaModule::luaGetRuntimeActs},
		{"getCurrentActIndex", CComLuaModule::luaGetCurrentActIndex},
		{"triggerUserTrigger", CComLuaModule::luaTriggerUserTrigger},

		{"getMaxNpcs", CComLuaModule::luaGetMaxNpcs},
		{"getMaxStaticObjects",CComLuaModule::luaGetMaxStaticObjects},

		{"getSheetIdName", CComLuaModule::luaGetSheetIdName},
		{"getMaxId", CComLuaModule::luaGetMaxId},
		{"reserveIdRange", CComLuaModule::luaReserveIdRange},
		{"getUserSlot", CComLuaModule::luaGetUserSlot},

		{"getSheetIdName", CComLuaModule::luaGetSheetIdName},
		{"requestTpToEntryPoint", CComLuaModule::luaRequestTpToEntryPoint},
		{"requestSetStartingAct", CComLuaModule::luaRequestSetStartingAct},

		{"getEmoteBehaviorFromEmoteId", CComLuaModule::luaGetEmoteBehaviorFromEmoteId},

		{"setStartingActIndex", CComLuaModule::luaSetStartingActIndex},
		{"dssTarget", CComLuaModule::luaDssTarget},

		{"mustDisplayInfo", CComLuaModule::luaMustDisplayInfo},
		{"hasDisplayInfo", CComLuaModule::luaHasDisplayInfo},
		{"setDisplayInfo", CComLuaModule::luaSetDisplayInfo},
		{"resetDisplayInfo", CComLuaModule::luaResetDisplayInfo},

		{"getTalkingAsList", CComLuaModule::luaGetTalkingAsList},
		{"getIncarnatingList", CComLuaModule::luaGetIncarnatingList},
		{"getScenarioHeader", CComLuaModule::luaGetScenarioHeader},

		{"getSheetRingAccess", CComLuaModule::luaGetSheetRingAccess},
		{"getIslandRingAccess", CComLuaModule::luaGetIslandRingAccess},
		{"updateScenarioAck", CComLuaModule::luaUpdateScenarioAck},
		{"getCharacterRingAccess", CComLuaModule::luaGetCharacterRingAccess},
		{"getRingAccessAsMap", CComLuaModule::luaGetRingAccessAsMap},
		{"verifyRtScenario", CComLuaModule::luaVerifyRtScenario},
		{"checkRingAccess", CComLuaModule::luaCheckRingAccess},

		{"setScenarioUpToDate", CComLuaModule::luaSetScenarioUpToDate},
		{"isSessionOwner", CComLuaModule::luaIsSessionOwner},
		{"getEditSessionLink", CComLuaModule::luaGetEditSessionLink},
		{"getFileHeader", CComLuaModule::luaGetFileHeader},
		{"getMustVerifyRingAccessWhileLoadingAnimation", CComLuaModule::luaGetMustVerifyRingAccessWhileLoadingAnimation},
		{"getUseVerboseRingAccess", CComLuaModule::luaGetUseVerboseRingAccess},
		{"getIsAnimationSession", CComLuaModule::luaGetIsAnimationSession},
		{"resetNameGiver", CComLuaModule::luaResetNameGiver},
		{"getCharIdMd5", CComLuaModule::luaGetCharIdMd5},
		{"hasCharacterSameCharacterIdMd5", CComLuaModule::luaHasCharacterSameCharacterIdMd5},
		{"getScenarioSavePath", CComLuaModule::luaGetScenarioSavePath},

		{"isServerEditionModuleUp", CComLuaModule::luaCheckServerEditionModule},





		{0,0}

	};
	int initialStackSize = lua_gettop(_LuaState);
#if LUA_VERSION_NUM >= 502
	// luaL_newlib(_LuaState, methods);
	// lua_setglobal(_LuaState, R2_LUA_PATH);
	lua_getglobal(_LuaState, R2_LUA_PATH);
	if (lua_isnil(_LuaState, -1))
	{
	  lua_pop(_LuaState, 1);
	  lua_newtable(_LuaState);
	}
	luaL_setfuncs(_LuaState, methods, 0);
	lua_setglobal(_LuaState, R2_LUA_PATH);
#else
	luaL_openlib(_LuaState, R2_LUA_PATH, methods, 0);
#endif
	lua_settop(_LuaState, initialStackSize);
}


void CComLuaModule::doFile(const std::string& filename)
{
	//H_AUTO(R2_CComLuaModule_doFile)
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
	//H_AUTO(R2_CComLuaModule_runLuaScript)
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
	//H_AUTO(R2_CComLuaModule_luaDoFile2)
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
	//H_AUTO(R2_CComLuaModule_loadFeatures)
	doFile("r2_core.lua");
}


CComLuaModule* CComLuaModule::getInstance(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_getInstance)
	std::map<lua_State*, CComLuaModule*>::const_iterator found(_Instance.find(state));
	if (found != _Instance.end())
	{
		return found->second;
	}
	return 0;
}


void CComLuaModule::callTranslateFeatures(CObject* scenario)
{
	//H_AUTO(R2_CComLuaModule_callTranslateFeatures)
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
	//H_AUTO(R2_CComLuaModule_translateFeatures)
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


sint CComLuaModule::luaReadUserComponentFile(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaReadUserComponentFile)
	sint args = lua_gettop(state);
	nlassert(args == 1);
	luaL_checktype(state, 1, LUA_TSTRING);
	std::string filename( lua_tostring(state, 1) );
	CComLuaModule* this2 = getInstance(state);

	std::string ret = this2->_Client->getEditionModule().readUserComponentFile(filename);
	lua_pushstring(state, ret.c_str());
	return 1;
}


sint CComLuaModule::luaRegisterUserComponent(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRegisterUserComponent)
	sint args = lua_gettop(state);
	nlassert(args == 1);
	luaL_checktype(state, 1, LUA_TSTRING);
	std::string filename( lua_tostring(state, 1) );
	CComLuaModule* this2 = getInstance(state);

	this2->_Client->getEditionModule().registerUserComponent(filename);
	return 0;
}


sint CComLuaModule::luaSaveUserComponentFile(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaSaveUserComponentFile)
	sint args = lua_gettop(state);
	nlassert(args == 2);
	luaL_checktype(state, 1, LUA_TSTRING);
	luaL_checktype(state, 2, LUA_TNUMBER);
	std::string filename( lua_tostring(state, 1) );
	double mustCompress( lua_tonumber(state, 2) );

	CComLuaModule* this2 = getInstance(state);
	this2->_Client->getEditionModule().saveUserComponentFile(filename, mustCompress != 0);
	return 0;
}

sint CComLuaModule::luaUpdateUserComponentsInfo(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaUpdateUserComponentsInfo)
	sint args = lua_gettop(state);
	nlassert(args == 5);

	luaL_checktype(state, 1, LUA_TSTRING);
	luaL_checktype(state, 2, LUA_TSTRING);
	luaL_checktype(state, 3, LUA_TSTRING);
	luaL_checktype(state, 4, LUA_TSTRING);
	luaL_checktype(state, 5, LUA_TSTRING);

	std::string filename( lua_tostring(state, 1) );
	std::string name( lua_tostring(state, 2) );
	std::string description( lua_tostring(state, 3) );
	uint32 timestamp( static_cast<uint32>(lua_tonumber(state, 3) ));
	std::string md5Id( lua_tostring(state, 3) );

	CComLuaModule* this2 = getInstance(state);

	this2->_Client->getEditionModule().updateUserComponentsInfo(filename, name, description, timestamp, md5Id);
	return 0;
}

sint CComLuaModule::luaGetSheetIdName(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetSheetIdName)
	luaL_checktype(state, 1, LUA_TNUMBER);

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	uint32 sheetIdValue = static_cast<uint32>( lua_tonumber(state, 1) );

	NLMISC::CSheetId sheetId(sheetIdValue);
	if(sheetId != NLMISC::CSheetId::Unknown)
	{
			lua_pushstring(state, sheetId.toString().c_str());
	}
	else
	{
		lua_pushstring(state, "");
	}

	return 1;
}

sint CComLuaModule::luaAddPaletteElement(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaAddPaletteElement)
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
	//H_AUTO(R2_CComLuaModule_luaGetPropertyValue)
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


sint CComLuaModule::luaGetPaletteElement(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetPaletteElement)
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string key( lua_tostring(state, 1) );
	CObject* object = this2->_Client->getPaletteElement(key);
	this2->setObjectToLua(state, object);
	return  1;
}

sint CComLuaModule::luaIsInPalette(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaIsInPalette)
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string key( lua_tostring(state, 1) );
	bool found = this2->_Client->isInPalette(key);
	lua_pushboolean(state, found);

	return  1;
}


sint CComLuaModule::luaRequestTranslateFeatures(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestTranslateFeatures)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestTranslateFeatures();
	return 0;
}


sint CComLuaModule::luaRequestCreatePrimitives(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestCreatePrimitives)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestCreatePrimitives();
	return 0;
}


sint CComLuaModule::luaRequestSetWeather(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestSetWeather)
	luaL_checktype(state, 1, LUA_TNUMBER);
	uint16 weatherValue = (uint16) lua_tonumber(state, 1);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestSetWeather(weatherValue);
	return 0;
}


sint CComLuaModule::luaRequestSetSeason(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestSetSeason)
	luaL_checktype(state, 1, LUA_TNUMBER);
	uint8 season = (uint8) lua_tonumber(state, 1);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestSetSeason(season);
	return 0;
}


sint CComLuaModule::luaRequestStopLive(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestStopLive)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestStopTest();
	return 0;
}


sint CComLuaModule::luaRequestStartAct(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestStartAct)
	luaL_checktype(state, 1, LUA_TNUMBER);
	uint32 actId(static_cast<uint32>(lua_tonumber(state, 1)));
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestStartAct(actId);
	return 0;
}


sint CComLuaModule::luaRequestStopAct(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestStopAct)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestStopAct();
	return 0;
}


sint CComLuaModule::luaNewComponent(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaNewComponent)
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
	//H_AUTO(R2_CComLuaModule_luaRegisterGenerator)
	luaL_checktype(state, 1, LUA_TTABLE);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	CObject* object = this2->getObjectFromLua(state, 1);
 	this2->_Client->registerGenerator(object);
	return  0;
}


sint CComLuaModule::luaShow(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaShow)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->show();
	return  0;
}


sint CComLuaModule::luaRequestUpdateRtScenario(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestUpdateRtScenario)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	CObject* object = this2->getObjectFromLua(state, 1);
	nlassert(object);
	this2->_Client->requestUpdateRtScenario(object);
	delete object;	// AJM
	return 0;
}


sint CComLuaModule::luaRequestCreateScenario(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestCreateScenario)
	luaL_checktype(state, 1, LUA_TTABLE);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	CObject* object = this2->getObjectFromLua(state, 1);
	nlassert(object);
	this2->_Client->getActionHistoric().clear();
	this2->_Client->requestCreateScenario(object);
	delete object;
	return 0;
}

sint CComLuaModule::luaRequestUploadCurrentScenario(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestUploadCurrentScenario)

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestUploadCurrentScenario();

	return 0;
}

sint CComLuaModule::luaPrint(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaPrint)
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
	/*std::stringstream ss;
	std::string s;
	object->serialize(ss);
	while (std::getline(ss, s))
	{
		nlinfo("%s", s.c_str());
	}*/
	std::string ss;
	object->serialize(ss);
	std::vector<std::string> lines;
	NLMISC::splitString(ss, "\n", lines);
	uint first=0, last=(uint)lines.size();
	for (; first != last ; ++first) { nlinfo("%s", lines[first].c_str()); }

	//this2->_Client->requestCreateScenario(object);
	delete object;
	return 0;
}


sint CComLuaModule::luaRequestMapConnection(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestMapConnection)
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
	//H_AUTO(R2_CComLuaModule_luaRequestReconnection)
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "requestReconnection");

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestReconnection();

	return 0;
}




sint CComLuaModule::requestInsertNode(lua_State* state, bool isGhost)
{
	//H_AUTO(R2_CComLuaModule_requestInsertNode)
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
	value->setGhost(isGhost);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestInsertNode(
		instanceId, attrName, position, key, value);
	delete value;
	return 0;
}


sint CComLuaModule::luaRequestInsertNode(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestInsertNode)
	return requestInsertNode(state, false);
}


sint CComLuaModule::luaRequestInsertGhostNode(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestInsertGhostNode)
	return requestInsertNode(state, true);
}


sint CComLuaModule::requestSetNode(lua_State* state, bool isGhost)
{
	//H_AUTO(R2_CComLuaModule_requestSetNode)
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
	value->setGhost(isGhost);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->requestSetNode(	instanceId, attrName, value);
	delete value;
	return 0;
}


sint CComLuaModule::luaRequestSetNode(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestSetNode)
	static volatile bool dumpCallstack = false;
	if (dumpCallstack)
	{
		CLuaIHMRyzom::dumpCallStack();
	}
	return requestSetNode(state, false);
}


sint CComLuaModule::luaRequestSetGhostNode(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestSetGhostNode)
	return requestSetNode(state, true);
}


sint CComLuaModule::luaRequestEraseNode(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestEraseNode)
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
	//H_AUTO(R2_CComLuaModule_luaRequestMoveNode)
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

int CComLuaModule::luaRequestNewAction(lua_State* state, bool pending, uint count)
{
	//H_AUTO(R2_CComLuaModule_luaRequestNewAction)
	if (count == 0)
	{
		nlwarning("Bad action count");
		return 0;
	}
	sint args = lua_gettop(state);
	const char *funcName = pending ? "requestNewPendingAction" : "requestNewAction";
	CHECK_LUA_ARG_COUNT_MAX(1, funcName);
	ucstring actionName;
	if (lua_gettop(state) == 1)
	{
		if (lua_type(state, -1) == LUA_TSTRING)
		{
			actionName = lua_tostring(state, -1);
		}
		else
		{
			// try with ucstring
			CLuaState &ls = getEditor().getLua();
			nlassert(ls.getStatePointer() == state);
			if (!CLuaIHM::getUCStringOnStack(ls, -1, actionName))
			{
				nlwarning("<r2.%s> : ucstring or string expected as action name", funcName);
				return 0;
			}
		}
	}
	else
	{
		actionName = NLMISC::CI18N::get("uiR2EDUnamedAction");
	}
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	if (pending)
	{
		if (count > 1)
		{
			this2->_Client->getActionHistoric().newPendingMultiAction(actionName, count);
		}
		else
		{
			this2->_Client->getActionHistoric().newPendingAction(actionName);
		}
	}
	else
	{
		if (count > 1)
		{
			this2->_Client->getActionHistoric().newMultiAction(actionName, count);
		}
		else
		{
			this2->_Client->newAction(actionName);
		}
	}
	return 0;
}

sint CComLuaModule::luaRequestClearActionHistoric(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestClearActionHistoric)
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "onClearActionHistoric");
	CComLuaModule* this2 = getInstance(state);
	this2->_Client->getActionHistoric().clear();
	return 0;
}


sint CComLuaModule::luaRequestNewAction(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestNewAction)
	return luaRequestNewAction(state, false, 1);
}

sint CComLuaModule::luaRequestNewPendingAction(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestNewPendingAction)
	return luaRequestNewAction(state, true, 1);
}

sint CComLuaModule::luaRequestNewMultiAction(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestNewMultiAction)
	const char *funcName = "requestNewMultiAction";
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(2, funcName);
	luaL_checktype(state, 2, LUA_TNUMBER);
	uint count = (uint) lua_tonumber(state, 2);
	lua_pop(state, 1);
	return luaRequestNewAction(state, false, count);
}

sint CComLuaModule::luaRequestNewPendingMultiAction(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestNewPendingMultiAction)
	const char *funcName = "requestNewPendingMultiAction";
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(2, funcName);
	luaL_checktype(state, 2, LUA_TNUMBER);
	uint count = (uint) lua_tonumber(state, 2);
	lua_pop(state, 1);
	return luaRequestNewAction(state, true, count);
}

sint CComLuaModule::luaRequestCancelAction(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestCancelAction)
	const char *funcName = "requestCancelAction";
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, funcName);
	CComLuaModule* this2 = getInstance(state);
	this2->_Client->getActionHistoric().cancelAction();
	return 0;
}

sint CComLuaModule::luaRequestForceEndMultiAction(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestForceEndMultiAction)
	const char *funcName = "requestForceEndMultiAction";
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, funcName);
	CComLuaModule* this2 = getInstance(state);
	this2->_Client->getActionHistoric().forceEndMultiAction();
	return 0;
}



sint CComLuaModule::luaRequestEndAction(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestEndAction)
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "requestEndAction");
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getActionHistoric().endAction();
	return 0;
}



sint CComLuaModule::luaRequestTpPosition(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestTpPosition)
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(3, "requestTpPosition");
	luaL_checktype(state, 1, LUA_TNUMBER);
	luaL_checktype(state, 2, LUA_TNUMBER);
	luaL_checktype(state, 3, LUA_TNUMBER);

	float x = static_cast<float>(lua_tonumber(state, 1));
	float y = static_cast<float>(lua_tonumber(state, 2));
	float z = static_cast<float>(lua_tonumber(state, 3));
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestTpPosition(x, y, z);
	return 0;

}


void CComLuaModule::setObjectToLua(lua_State* state, CObject* object)
{
	//H_AUTO(R2_CComLuaModule_setObjectToLua)
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

#if LUA_VERSION_NUM >= 502
		lua_pushglobaltable(state); // _G
#else
		lua_pushvalue(state, LUA_GLOBALSINDEX); // _G
#endif

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

#if 0
		// okay!
		if (0)
		{

			first = 0;
			last = (uint32)keys.size();
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
#endif
	}
	else
	{
		nlwarning("error! object is not a string, not a number, not a table!");
		lua_pushnil(state);
	}
}


CObject* CComLuaModule::getObjectFromLua(lua_State* state, sint idx)
{
	//H_AUTO(R2_CComLuaModule_getObjectFromLua)
	lua_pushvalue(state, idx);

	// special case for RefID
	if (lua_type(state, -1) == LUA_TTABLE)
	{
		if (lua_getmetatable(state, -1))
		{
#if LUA_VERSION_NUM >= 502
			lua_pushglobaltable(state); // obj, mt, _G
#else
			lua_pushvalue(state, LUA_GLOBALSINDEX); // obj, mt, _G
#endif

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
						result = new CObjectRefIdClient(str);
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

		case LUA_TBOOLEAN:
		{
			double value = static_cast<double>(lua_toboolean(state, -1));
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
			CObjectTable* table = new CObjectTableClient();

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


CObject* CComLuaModule::loadLocal(const std::string& filename, const CScenarioValidator::TValues& values)
{
	CScenarioValidator::TValues::const_iterator first(values.begin()), last(values.end());
	std::string name = "";
	for (; first != last; ++first)
	{
		if (first->first == "Name" ) { name = first->second; }
	}

	//H_AUTO(R2_CComLuaModule_loadLocal)
	if (filename.empty()){ return 0; }
	CObject* object = NULL;
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
	if (luaL_dofile(_LuaState, filename.c_str()) == 0)
#else
	if (lua_dofile(_LuaState, filename.c_str()) == 0)
#endif
	{
		lua_getglobal(_LuaState, "scenario");

		if (lua_type(_LuaState, -1) == LUA_TTABLE)
		{
			object = getObjectFromLua(_LuaState);
			if (object && !name.empty())
			{
				if (object->getAttr("Ghost_Name"))
				{
					object->set("Ghost_Name", name);
				}
				else
				{
					object->add("Ghost_Name", name);
				}
			}
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
	//H_AUTO(R2_CComLuaModule_load)

	return _Client->getEditionModule().addToLoadList(filename, new CLoadScenarioSucceded(&_Client->getEditionModule()));
}


bool CComLuaModule::loadUserComponent(const std::string& filename)
{
	//H_AUTO(R2_CComLuaModule_load)

	return _Client->getEditionModule().addToUserComponentLoadList(filename, new CLoadUserComponentSucceeded(&_Client->getEditionModule()));
}




CObject* CComLuaModule::loadFromBuffer(const std::string& data, const std::string& filename, const CScenarioValidator::TValues& values)
{
	CScenarioValidator::TValues::const_iterator first(values.begin()), last(values.end());
	std::string name = "";
	for (; first != last; ++first)
	{
		if (first->first == "Name" ) { name = first->second; }
	}

	//H_AUTO(R2_CComLuaModule_loadFromBuffer)
	static volatile bool dump = false;
	if (dump)
	{
		COFile testNico("test_nico.lua");
		testNico.serialBuffer(const_cast<uint8 * >((const uint8 *) &data[0]), (uint)data.size());
	}
	CObject* object = NULL;
	// TMP TMP
	CLuaState &ls = getEditor().getLua();
	try
	{
		ls.executeScript(data, 0);
		lua_getglobal(_LuaState, "scenario");

		if (lua_type(_LuaState, -1) == LUA_TTABLE)
		{
			object = getObjectFromLua(_LuaState);
			if (object && !name.empty())
			{
				if (object->getAttr("Ghost_Name"))
				{
					object->set("Ghost_Name", name);
				}
				else
				{
					object->add("Ghost_Name", name);
				}
			}
		}
	}
	catch(const ELuaError &e)
	{
		nlwarning("%s", e.what());
	}
	/*t res =
	if (lua_dobuffer(_LuaState,data.c_str(), data.size(), filename.c_str()) == 0)
	{

	}*/
	if (!object)
	{
		nlwarning("Error while loading %s", filename.c_str());
		return 0;
	}
	return object;


}


sint CComLuaModule::luaUpdateScenario(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaUpdateScenario)
	luaL_checktype(state, 1, LUA_TTABLE);

	CObject* object = getObjectFromLua(state);
	lua_pushliteral(state, "tmp1");
	lua_pushliteral(state, "tmp1");
	lua_pushliteral(state, "tmp1");

	lua_getglobal(state, "write_table");
	setObjectToLua(state, object);
	lua_call(state, 1, 0);

	nlinfo("updateScenario");
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2->_LuaState == state);

	delete object;	// AJM
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
	//H_AUTO(R2_CComLuaModule_luaLoad)
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

sint CComLuaModule::luaLoadUserComponent(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaLoad)
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string filename( lua_tostring(state, 1) );

	this2->loadUserComponent(filename);
	return 0;
}


sint CComLuaModule::luaLoadAnimation(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaLoadAnimation)
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string filename( lua_tostring(state, 1) );
	std::string errMsg;

	bool ok = this2->_Client->getEditionModule().addToLoadList(filename, new CLoadAnimationSucceded(&this2->_Client->getEditionModule()));

	if (!ok)
	{
		lua_pushboolean(state, 0);
		lua_pushstring(state, errMsg.c_str());
	}
	else
	{
		lua_pushboolean(state, 1);
		lua_pushstring(state, errMsg.c_str());
	}
	return 2;
}


sint CComLuaModule::luaSave(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaSave)
	luaL_checktype(state, 1, LUA_TSTRING);


	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string filename( lua_tostring(state, 1) );

	std::vector< std::pair< std::string, std::string> > values;
	// second parameter == value table
	if (lua_type(state, 2) == LUA_TTABLE)
	{


		lua_pushnil(state);
		while (lua_next(state, 2) != 0)
		{

			if (   lua_type(state, -2) == LUA_TNUMBER
				&& lua_type(state, -1) == LUA_TTABLE)
			{

				lua_pushnil(state);
				while (lua_next(state, -2) != 0)
				{
					if (   lua_type(state, -2) == LUA_TSTRING
						&& lua_type(state, -1) == LUA_TSTRING)

					{
						std::string key( lua_tostring(state, -2) );
						std::string value( lua_tostring(state, -1) );
						values.push_back( std::pair<std::string, std::string>(key, value) );
					}
					lua_pop(state, 1);
				}

			}
			else
			{
				nlwarning("Error while saving file %s", filename.c_str());
			}
			lua_pop(state, 1);
		}



	}


	this2->_Client->getEditionModule().addToSaveList(filename, values);

	return 0;
}


sint CComLuaModule::luaSaveUserComponent(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaSave)
	luaL_checktype(state, 1, LUA_TSTRING); //filename
	luaL_checktype(state, 3, LUA_TSTRING); //file body

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string filename( lua_tostring(state, 1) );
	std::string body( lua_tostring(state, 3) );

	std::vector< std::pair< std::string, std::string> > values;
	// second parameter == value table
	if (lua_type(state, 2) == LUA_TTABLE)
	{


		lua_pushnil(state);
		while (lua_next(state, 2) != 0)
		{

			if (   lua_type(state, -2) == LUA_TNUMBER
				&& lua_type(state, -1) == LUA_TTABLE)
			{

				lua_pushnil(state);
				while (lua_next(state, -2) != 0)
				{
					if (   lua_type(state, -2) == LUA_TSTRING
						&& lua_type(state, -1) == LUA_TSTRING)

					{
						std::string key( lua_tostring(state, -2) );
						std::string value( lua_tostring(state, -1) );
						values.push_back( std::pair<std::string, std::string>(key, value) );
					}
					lua_pop(state, 1);
				}

			}
			else
			{
				nlwarning("Error while saving user component file %s", filename.c_str());
			}
			lua_pop(state, 1);
		}



	}


	this2->_Client->getEditionModule().addToUserComponentSaveList(filename, values, body);

	return 0;
}

sint CComLuaModule::luaTalkAs(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaTalkAs)
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string npcname( lua_tostring(state, 1) );
	nlinfo(("luaTalkAs:: "+npcname).c_str());
	this2->_Client->getEditionModule().requestTalkAs(npcname);
	return 0;
}


sint CComLuaModule::luaStopTalkAs(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaStopTalkAs)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestStopTalkAs();
	return 0;
}


sint CComLuaModule::luaRequestStringTable(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestStringTable)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestStringTable();
	return 0;
}


sint CComLuaModule::luaRequestSetStringValue(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestSetStringValue)
	luaL_checktype(state, 1, LUA_TSTRING);
	luaL_checktype(state, 2, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string localId( lua_tostring(state, 1) );
	std::string value(lua_tostring(state,2));
	this2->_Client->getEditionModule().requestSetStringValue(localId,value);
	return 0;
}


sint CComLuaModule::luaRequestStringValue(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestStringValue)
	luaL_checktype(state, 1, LUA_TSTRING);
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string localId( lua_tostring(state, 1) );
	this2->_Client->getEditionModule().requestStringValue(localId);
	return 0;
}


sint CComLuaModule::luaRequestIdList(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestIdList)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	this2->_Client->getEditionModule().requestIdList();
	return 0;
}


sint CComLuaModule::luaGetScenarioObj(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetScenarioObj)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	CObject* obj = this2->_Client->getCurrentScenario()->getHighLevel();
	setObjectToLua(state,obj);
	return 1;
}

sint CComLuaModule::luaGetNamespace(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetNamespace)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	TSessionId sessionId= this2->_Client->getEditionModule().getCurrentAdventureId();
	std::string value= NLMISC::toString("r2_%04d_",sessionId.asInt());
	lua_pushstring(state, value.c_str());
	return 1;
}



sint CComLuaModule::luaGetUserTriggers(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetUserTriggers)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	const TUserTriggerDescriptions&  triggers = this2->_Client->getEditionModule().getUserTriggers();

	uint32 first = 0, last = (uint32)triggers.size();
	lua_newtable(state);
	for ( ; first != last ; ++first)
	{
		lua_pushnumber(state, first+1);
		lua_newtable(state);

		lua_pushstring(state, "Name");
		lua_pushstring(state, triggers[first].Name.c_str());
		lua_settable(state, -3);

		lua_pushstring(state, "Act");
		lua_pushnumber(state, triggers[first].Act);
		lua_settable(state, -3);


		lua_pushstring(state, "Id");
		lua_pushnumber(state, triggers[first].Id);
		lua_settable(state, -3);

		lua_settable(state, -3);
	}
	return 1;
}


sint CComLuaModule::luaTriggerUserTrigger(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaTriggerUserTrigger)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	uint32 actId( static_cast<uint32>(lua_tonumber(state, 1) ) );
	uint32 id( static_cast<uint32>(lua_tonumber(state, 2) ));
	this2->_Client->getEditionModule().requestTriggerUserTrigger(actId, id);

	return 0;
}


sint CComLuaModule::luaGetRuntimeActs(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetRuntimeActs)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	const TActPositionDescriptions& actPositionDescriptions = this2->_Client->getEditionModule().getRuntimeActs();

	uint32 first = 0, last = (uint32)actPositionDescriptions.size();
	lua_newtable(state);
	for ( ; first != last ; ++first)
	{
		lua_pushnumber(state, first+1);
		lua_newtable(state);

		lua_pushstring(state, "Name");
		lua_pushstring(state, actPositionDescriptions[first].Name.c_str());
		lua_settable(state, -3);

		lua_pushstring(state, "Island");
		lua_pushstring(state, actPositionDescriptions[first].Island.c_str());
		lua_settable(state, -3);

		lua_pushstring(state, "Season");
		lua_pushnumber(state, actPositionDescriptions[first].Season);
		lua_settable(state, -3);


		lua_pushstring(state, "LocationId");
		lua_pushnumber(state, actPositionDescriptions[first].LocationId);
		lua_settable(state, -3);

		lua_settable(state, -3);
	}
	return 1;
}


sint CComLuaModule::luaGetCurrentActIndex(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetCurrentActIndex)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	uint32 currentActIndex =this2->_Client->getEditionModule().getCurrentActIndex();
	lua_pushnumber(state, currentActIndex);

	return 1;
}

sint CComLuaModule::luaGetIslandsLocation(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetIslandsLocation)
	std::vector<std::string> locations;

	//	lua_pushstring(state, "Keys");
	lua_newtable(state);

	return 1;
}

sint CComLuaModule::luaObjectToLua(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaObjectToLua)
	CObject* ret = getObjectFromLua(state, -1);
	if(!ret)
	{
		nlwarning("objectToLua : not an object");
		return 0;
	}
	setObjectToLua(state, ret);
	return 1;
}

sint CComLuaModule::luaGetMaxNpcs(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetMaxNpcs)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	uint32 maxNpcs =this2->_Client->getEditionModule().getMaxNpcs();
	lua_pushnumber(state, maxNpcs);

	return 1;


}

sint CComLuaModule::luaGetMaxStaticObjects(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetMaxStaticObjects)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	uint32 maxStaticObjects =this2->_Client->getEditionModule().getMaxStaticObjects();
	lua_pushnumber(state, maxStaticObjects);

	return 1;
}

sint CComLuaModule::luaGetMaxId(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetMaxId)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	uint32 maxId = this2->_Client->getEditionModule().getCurrentMaxId();
	lua_pushnumber(state, maxId);

	return 1;
}

sint CComLuaModule::luaReserveIdRange(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaReserveIdRange)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	//
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "reserveIdRange");
	luaL_checktype(state, 1, LUA_TNUMBER);

	uint32 range=static_cast<uint32>( lua_tonumber(state, 1) );

	this2->_Client->getEditionModule().reserveIdRange(range);

	return 1;
}

sint CComLuaModule::luaGetUserSlot(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetUserSlot)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	std::string eid = this2->_Client->getEditionModule().getEid();
	lua_pushstring(state, eid.c_str());
	return 1;
}


sint CComLuaModule::luaRequestTpToEntryPoint(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestTpToEntryPoint)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "requestTpToEntryPoint");
	luaL_checktype(state, 1, LUA_TNUMBER);

	uint32 actIndex = static_cast<uint32>( lua_tonumber(state, 1) );
	this2->_Client->getEditionModule().requestTpToEntryPoint(actIndex);

	return 0;
}


sint CComLuaModule::luaRequestSetStartingAct(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaRequestSetStartingAct)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "requestSetStartingAct");
	luaL_checktype(state, 1, LUA_TNUMBER);

	uint32 actIndex = static_cast<uint32>( lua_tonumber(state, 1) );
	this2->_Client->getEditionModule().requestSetStartingAct(actIndex);

	return 0;
}



sint CComLuaModule::luaGetEmoteBehaviorFromEmoteId(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetEmoteBehaviorFromEmoteId)

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "requestTpToEntryPoint");
	luaL_checktype(state, 1, LUA_TSTRING);
	std::string ret = this2->_Client->getEditionModule().getEmoteBehaviorFromEmoteId( lua_tostring(state, 1));
	lua_pushstring(state, ret.c_str());

	return 1;
}


sint CComLuaModule::luaMustDisplayInfo(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaMustDisplayInfo)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	//
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "mustDisplayInfo");
	luaL_checktype(state, 1, LUA_TSTRING);

	std::string formName( lua_tostring(state, 1) );

	//this2->_Client->getEditionModule().reserveIdRange(range);
	uint32 isDisplayed = this2->_Client->getEditionModule().mustDisplayInfo(formName);
	lua_pushnumber(state, isDisplayed);

	return 1;
}

sint CComLuaModule::luaHasDisplayInfo(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaHasDisplayInfo)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	//
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "hasDisplayInfo");
	luaL_checktype(state, 1, LUA_TSTRING);

	std::string formName( lua_tostring(state, 1) );

	uint32 hasDisplayInfo = this2->_Client->getEditionModule().hasDisplayInfo(formName);
	lua_pushnumber(state, hasDisplayInfo);

	return 1;
}


sint CComLuaModule::luaSetDisplayInfo(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaSetDisplayInfo)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	//
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(2, "setDisplayInfo");
	luaL_checktype(state, 1, LUA_TSTRING);
	luaL_checktype(state, 2, LUA_TNUMBER);


	std::string formName( lua_tostring(state, 1) );
	bool displayInfo = static_cast<bool>(lua_tonumber(state, 2) == 0);

	//this2->_Client->getEditionModule().reserveIdRange(range);
	this2->_Client->getEditionModule().setDisplayInfo(formName, displayInfo);

	return 1;
}

sint CComLuaModule::luaResetDisplayInfo(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaResetDisplayInfo)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	this2->_Client->getEditionModule().resetDisplayInfo();


	return 1;
}


sint CComLuaModule::luaSetStartingActIndex(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaSetStartingActIndex)

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "setStartingActIndex");
	luaL_checktype(state, 1, LUA_TNUMBER);
	this2->_Client->getEditionModule().setStartingActIndex( static_cast<uint32>(lua_tonumber(state, 1)) );
	return 0;
}

sint CComLuaModule::luaDssTarget(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaDssTarget)

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(2, "dssTarget");
	// arg1 is r2 but we don't care
	luaL_checktype(state, 2, LUA_TSTRING);
	std::string str(lua_tostring(state, 2));

	bool controlNpc = UserEntity->isInNpcControl();
	if(str=="CONTROL" && UserEntity && !controlNpc)
	{
		CEntityCL *entity = EntitiesMngr.entity(UserEntity->targetSlot());

		if(entity )
		{
			UserEntity->pos(entity->pos());
			UserEntity->dir(entity->dir());
			UserEntity->front(entity->front());
		}
	}

	std::vector<std::string> strsplit;
	NLMISC::splitString(str, " ", strsplit);

	this2->_Client->getEditionModule().dssTarget(strsplit);
	return 0;
}


sint CComLuaModule::luaGetTalkingAsList(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetTalkingAsList)


	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "getTalkingAsList");


	std::vector<uint32> l = this2->_Client->getEditionModule().getTalkingAsList();


	lua_newtable(state);

	double index = 0.0;
	std::vector<uint32>::const_iterator first(l.begin()), last(l.end());
	for ( ; first != last; ++first)
	{
		index += 1;

		lua_pushnumber(state, index);
		lua_pushnumber(state, double(*first));
		lua_settable(state, -3);
	}


	return 1;

}

sint CComLuaModule::luaGetScenarioHeader(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetScenarioHeader)

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "getScenarioHeader");

	const CClientEditionModule::TScenarioHeader& header = this2->_Client->getEditionModule().getScenarioHeader();

	lua_newtable(state);

	//double index = 0.0;
	CClientEditionModule::TScenarioHeader::const_iterator first(header.begin()), last(header.end());

	for ( ; first != last; ++first)
	{

		lua_pushstring(state, first->first.c_str());
		lua_pushstring(state, first->second.c_str());

		lua_settable(state, -3);
	}


	return 1;
}

sint CComLuaModule::luaGetIncarnatingList(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetIncarnatingList)


	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "getIncarnatingList");

	std::vector<uint32> l = this2->_Client->getEditionModule().getIncarnatingList();

	lua_newtable(state);

	double index = 0.0;
	std::vector<uint32>::const_iterator first(l.begin()), last(l.end());

	for ( ; first != last; ++first)
	{
		index += 1;

		lua_pushnumber(state, index);
		lua_pushnumber(state, double(*first));
		lua_settable(state, -3);
	}


	return 1;
}


sint CComLuaModule::luaGetSheetRingAccess(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetSheetRingAccess)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);

	//CHECK_LUA_ARG_COUNT(1, "getSheetRingAccess");
	luaL_checktype(state, 1, LUA_TSTRING);
	std::string sheetClient(lua_tostring(state, 1));
	std::string sheetServer;


	if (args == 2)
	{
		luaL_checktype(state, 2, LUA_TSTRING);
		sheetServer = lua_tostring(state, 2);
	}

	std::string ringAccess = CRingAccess::getInstance().getSheetAccessInfo(sheetClient, sheetServer);

	lua_pushstring(state, ringAccess.c_str());

	return 1;
}

sint CComLuaModule::luaGetIslandRingAccess(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetIslandRingAccess)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "getIslandRingAccess");

	luaL_checktype(state, 1, LUA_TSTRING);
	std::string island(lua_tostring(state, 1));

	std::string ringAccess = CRingAccess::getInstance().getIslandAccessInfo(island);

	lua_pushstring(state, ringAccess.c_str());

	return 1;
}


sint CComLuaModule::luaUpdateScenarioAck(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaUpdateScenarioAck)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(3, "updateScenarioAck");

	luaL_checktype(state, 1, LUA_TBOOLEAN);
	luaL_checktype(state, 2, LUA_TTABLE);
	luaL_checktype(state, 3, LUA_TSTRING);
	bool ok(lua_toboolean(state, 1) != 0.0);
	std::string  errMsg = lua_tostring(state, 3);

	CObject* object = this2->getObjectFromLua(state, 2);
	std::string str;
	if (object)
	{
		std::map<std::string, int> level;
		uint size = object->getSize();
		for(uint i = 0; i < size;  ++i)
		{
			std::string key = object->getKey(i);
			CObject* value = object->getValue(i);
			if (value->isNumber())
			{

				if (key.size() == 1)
				{
					level[key] = static_cast<int>(value->toNumber());
				}
			}
		}

		std::map<std::string, int>::const_iterator first(level.begin()), last(level.end());
		for (; first != last; ++first)
		{
			str += NLMISC::toString(":%s%d", first->first.c_str(), first->second);
		}
		if (!str.empty()) { str += ":"; }
		this2->_Client->getEditionModule().updateScenarioRingAccess(ok, str, errMsg);

	}



	return 0;
}


sint CComLuaModule::luaGetCharacterRingAccess(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetCharacterRingAccess)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "getCharacterRingAccess");

	std::string access = this2->_Client->getEditionModule().getCharacterRingAccess();
	lua_pushstring(state, access.c_str());


	return 1;
}

sint CComLuaModule::luaCheckRingAccess(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaCheckRingAccess)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "checkRingAccess");
	luaL_checktype(state, 1, LUA_TSTRING);

	std::string wantedAccess = lua_tostring(state, 1);

	std::string access = this2->_Client->getEditionModule().getCharacterRingAccess();

	bool ok = CRingAccess::getInstance().verifyRight(wantedAccess, access);

	lua_pushboolean(state, ok);

	return 1;
}


sint CComLuaModule::luaGetRingAccessAsMap(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetRingAccessAsMap)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "getRingAccessAsMap");

	luaL_checktype(state, 1, LUA_TSTRING);

	std::string access = lua_tostring(state, 1);
	std::map<std::string, int> accessMap;
	CRingAccess::getInstance().getRingAccessAsMap(access, accessMap);

	lua_newtable(state);
	std::map<std::string, int>::const_iterator first(accessMap.begin()), last(accessMap.end());
	for (; first != last; ++first)
	{
		lua_pushstring(state, first->first.c_str());
		lua_pushnumber(state, first->second);
		lua_settable(state,- 3);
	}



	return 1;

}
sint CComLuaModule::luaSetScenarioUpToDate(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaSetScenarioUpToDate)

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "setScenarioUpToDate");
	luaL_checktype(state, 1, LUA_TBOOLEAN);
	int ok = lua_toboolean(state, 1);

	this2->_Client->getEditionModule().setScenarioUpToDate(ok != 0);

	return 0;
}

sint CComLuaModule::luaIsSessionOwner(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaIsSessionOwner)

	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "isSessionOwner");
	bool ok = this2->_Client->getEditionModule().isSessionOwner();
	lua_pushboolean(state, static_cast<int>(ok));
	return 1;
}

sint CComLuaModule::luaVerifyRtScenario(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaVerifyRtScenario)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "verifyRtScenario");

	luaL_checktype(state, 1, LUA_TTABLE);


	CObject* object = this2->getObjectFromLua(state, 1);
	CVerfiyRightRtScenarioError* err;


	std::string charRingAccess = this2->_Client->getEditionModule().getCharacterRingAccess();
	bool ok = CRingAccess::getInstance().verifyRtScenario(object, charRingAccess, err);
	if (ClientCfg.Local) // TMP TMP
	{
		ok = true;
	}
	if ( ok ) { lua_pushboolean(state, static_cast<int>(ok));  return 1; };

	nlassert(err);


	lua_pushboolean(state, static_cast<int>(ok));

	lua_newtable(state);
	lua_pushstring(state, "Type");
	switch(err->Type)
	{
	case CVerfiyRightRtScenarioError::InvalidData: lua_pushstring(state, "InvalidData"); break;
		case CVerfiyRightRtScenarioError::InvalidIslandLevel: lua_pushstring(state, "InvalidIslandLevel"); break;
		case CVerfiyRightRtScenarioError::InvalidBotLevel: lua_pushstring(state, "InvalidBotLevel"); break;
		default: lua_pushstring(state, "None"); break;
	}

	lua_settable(state, -3);

	lua_pushstring(state, "EntityName");
	lua_pushstring(state, err->Name.c_str());
	lua_settable(state, -3);

	lua_pushstring(state, "Package");
	lua_pushstring(state, err->Package.c_str());
	lua_settable(state, -3);

	lua_pushstring(state, "EntityLevel");
	lua_pushnumber(state, err->Level);
	lua_settable(state, -3);

	lua_pushstring(state, "CharLevel");
	lua_pushnumber(state, err->CharLevel);
	lua_settable(state, -3);

	delete err;
	return 2;

}

sint CComLuaModule::luaGetEditSessionLink(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetEditSessionLink)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	TSessionId sessionId= this2->_Client->getEditionModule().getEditSessionLink();
	lua_pushnumber(state, sessionId.asInt());
	return 1;
}

sint CComLuaModule::luaGetFileHeader(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetFileHeader)
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "getFileHeader");
	luaL_checktype(state, 1, LUA_TSTRING);

	const std::string scenarioName( lua_tostring(state, 1) );
	std::string md5, signature;

	CClientEditionModule::TScenarioHeader header;

	R2::CScenarioValidator sv;
	sv.setScenarioToLoad(scenarioName, header, md5, signature, false);

	lua_newtable(state);

	CClientEditionModule::TScenarioHeader::const_iterator first(header.begin()), last(header.end());

	for ( ; first != last; ++first)
	{

		lua_pushstring(state, first->first.c_str());
		lua_pushstring(state, first->second.c_str());

		lua_settable(state, -3);
	}


	return 1;
}


sint CComLuaModule::luaGetMustVerifyRingAccessWhileLoadingAnimation(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetMustVerifyRingAccessWhileLoadingAnimation)
	lua_pushboolean(state, ClientCfg.R2EDMustVerifyRingAccessWhileLoadingAnimation);
	return 1;
}


sint CComLuaModule::luaGetUseVerboseRingAccess(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetUseVerboseRingAccess)
	lua_pushboolean(state, ClientCfg.R2EDUseVerboseRingAccess);
	return 1;
}


sint CComLuaModule::luaGetIsAnimationSession(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaGetIsAnimationSession)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);


	lua_pushboolean(state, this2->_Client->getEditionModule().getSessionType() == st_anim);
	return 1;
}

sint CComLuaModule::luaResetNameGiver(lua_State* state)
{
	//H_AUTO(R2_CComLuaModule_luaResetNameGiver)
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);

	this2->_Client->getEditionModule().resetNameGiver();
	return 0;
}

sint CComLuaModule::luaGetCharIdMd5(lua_State* state)
{
	uint32 charId = 0;
	if (!ClientCfg.Local) { charId = (NetMngr.getLoginCookie().getUserId()<< 4) + (uint32) PlayerSelectedSlot; }

	uint32 value = CRingAccess::getInstance().cypherCharId(charId);

	std::string ret = "00000000";


	for (uint32 i = 0 ; i < 8 ; ++i)
	{
		uint32 v = value  % 16;
		value /= 16;
		char & c = ret[7-i];
		if (v <= 9)
		{
			c = '0' + static_cast<char>(v);
		}
		else //  10 <=> 15
		{
			c = 'A' + static_cast<char>(v) - 10;
		}
	}

	lua_pushstring(state, ret.c_str())      ;

	return 1;
}


sint CComLuaModule::luaHasCharacterSameCharacterIdMd5(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(1, "hasCharacterSameCharacterIdMD5");
	luaL_checktype(state, 1, LUA_TSTRING);

	std::string charId( lua_tostring(state, 1) );

	bool ok = this2->_Client->getEditionModule().hasCharacterSameCharacterIdMd5(charId);
	lua_pushboolean(state, ok);
	return 1;
}


sint CComLuaModule::luaGetScenarioSavePath(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "luaGetScenarioSavePath");

	std::string path = ClientCfg.ScenarioSavePath;
	if (!path.empty())
	{
		if (path[path.size()-1] != '/')
		{
			path.push_back('/');
		}
	}
	lua_pushstring(state, path.c_str());
	return 1;
}

sint CComLuaModule::luaCheckServerEditionModule(lua_State* state)
{
	CComLuaModule* this2 = getInstance(state);
	nlassert(this2);
	sint args = lua_gettop(state);
	CHECK_LUA_ARG_COUNT(0, "luaCheckServerEditionModule");

	bool moduleUp = this2->_Client->getEditionModule().isServerEditionModuleUp();

	lua_pushboolean(state, moduleUp);
	return 1;
}
