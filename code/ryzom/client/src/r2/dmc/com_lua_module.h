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

#ifndef  DMS_COMLUAMODULE_H
#define  DMS_COMLUAMODULE_H

#include "nel/misc/types_nl.h"


extern "C"
{
	#include "nel/gui/lua_loadlib.h"
}





#include <map>
#include <string>

#include "game_share/scenario.h"

namespace R2
{
	class CObject;
	class CDynamicMapClient;

	class CComLuaModule
	{
		public:
		/** Build the communication module
		  * \param luaState NULL If the lua environment should be built by this object
		  *        If the lua state is provided externally, then it won't be deleted by this object
		  */

		CComLuaModule(CDynamicMapClient* client, lua_State *luaState = NULL);

		void doFile(const std::string& filename);

		bool runLuaScript(const std::string& filename, std::string& errorMsg);

		CObject* translateFeatures(CObject* hlScenario, std::string& errorMsg) const;

		void loadFeatures();

		static CComLuaModule* getInstance(lua_State* state);

		void callTranslateFeatures(CObject* scenario);


		//Edition
		static sint luaAddPaletteElement(lua_State* state);
		static sint luaIsInPalette(lua_State* state);

		static sint luaGetPropertyValue(lua_State* state);
		static sint luaPrint(lua_State* state);

		static sint luaGetPaletteElement(lua_State* state);

		static sint luaRequestTranslateFeatures(lua_State* state);

		static sint luaNewComponent(lua_State* state);

		static sint luaNewGhostComponent(lua_State* state);

		static sint luaRegisterGenerator(lua_State* state);

		static sint luaShow(lua_State* state);

		static sint luaRequestInsertNode(lua_State* state);

		static sint luaRequestInsertGhostNode(lua_State* state);

		static sint luaRequestSetNode(lua_State* state);

		static sint luaRequestSetGhostNode(lua_State* state);

		static sint luaRequestEraseNode(lua_State* state);

		static sint luaRequestMoveNode(lua_State* state);

		static sint luaRequestNewAction(lua_State* state);

		static sint luaRequestNewMultiAction(lua_State* state);

		static sint luaRequestCancelAction(lua_State* state);

		static sint luaRequestForceEndMultiAction(lua_State* state);

		static sint luaRequestNewPendingMultiAction(lua_State* state);

		static sint luaRequestClearActionHistoric(lua_State* state);

		static sint luaRequestNewPendingAction(lua_State* state);

		static sint luaRequestEndAction(lua_State* state);

		static sint luaRequestUpdateRtScenario(lua_State* state);

		static sint luaSetScenarioUpToDate(lua_State* state);



		//Admin

		static sint luaRequestCreatePrimitives(lua_State* state);

		static sint luaRequestStopLive(lua_State* state);

		static sint luaRequestStartAct(lua_State* state);

		static sint luaRequestStopAct(lua_State* state);

		static sint luaRequestMapConnection(lua_State* state);

		static sint luaRequestReconnection(lua_State* state);

		static sint luaRequestUploadCurrentScenario(lua_State* state);

		static sint luaRequestCreateScenario(lua_State* state);

		static sint luaRequestSetWeather(lua_State* state);

		static sint luaRequestSetSeason(lua_State* state);

		static void setObjectToLua(lua_State* state, CObject* object);

		static CObject* getObjectFromLua(lua_State* state, sint idx=-1);


		CObject* loadFromBuffer(const std::string& data, const std::string& filename, const CScenarioValidator::TValues& values);
		bool load(const std::string& filename);
		bool loadUserComponent(const std::string& filename);


		CObject* loadLocal(const std::string& filename, const CScenarioValidator::TValues& values);

		static sint luaUpdateScenario(lua_State* state);

		static sint luaLoadUserComponent(lua_State* state);
		static sint luaSaveUserComponent(lua_State* state);

		static sint luaLoad(lua_State* state);
		static sint luaLoadAnimation(lua_State* state);

		static sint luaSave(lua_State* state);
		static sint luaDoFile2(lua_State* state);
		static sint luaTalkAs(lua_State* state);
		static sint luaStopTalkAs(lua_State* state);
		static sint luaRequestTpPosition(lua_State* state);
		static sint luaRequestDespawnEntity(lua_State* state);
		static sint luaRequestDespawnGrp(lua_State* state);
		static sint luaRequestIdList(lua_State* state);
		static sint luaRequestStringTable(lua_State* state);
		static sint luaRequestSetStringValue(lua_State* state);
		static sint luaRequestStringValue(lua_State* state);
		static sint luaGetScenarioObj(lua_State* state);
		static sint luaGetNamespace(lua_State* state);
		static sint luaGetIslandsLocation(lua_State* state);
		static sint luaObjectToLua(lua_State* state);
		static sint luaIsSessionOwner(lua_State* state);

		//Component
		static sint luaReadUserComponentFile(lua_State* state);
		static sint luaRegisterUserComponent(lua_State* state);
		static sint luaUpdateUserComponentsInfo(lua_State* state);
		static sint luaSaveUserComponentFile(lua_State* state);

		//Acts
		static sint luaGetRuntimeActs(lua_State* state);
		static sint luaGetUserTriggers(lua_State* state);
		static sint luaGetCurrentActIndex(lua_State* state);
		static sint luaTriggerUserTrigger(lua_State* state);
		static sint luaRequestTpToEntryPoint(lua_State* state);
		static sint luaRequestSetStartingAct(lua_State* state);
		static sint luaSetStartingActIndex(lua_State* state);



		//Quotas
		static sint luaGetMaxNpcs(lua_State* state);
		static sint luaGetMaxStaticObjects(lua_State* state);

		static sint luaGetSheetIdName(lua_State* state);

		//Scenario Component exporter
		static sint luaGetMaxId(lua_State* state);
		static sint luaReserveIdRange(lua_State* state);
		static sint luaGetUserSlot(lua_State* state);

		static sint luaGetEmoteBehaviorFromEmoteId(lua_State* state);

		//features tutorial form display
		static sint luaMustDisplayInfo(lua_State* state);
		static sint luaHasDisplayInfo(lua_State* state);
		static sint luaSetDisplayInfo(lua_State* state);
		static sint luaResetDisplayInfo(lua_State* state);


		static sint luaDssTarget(lua_State* state);

		// DM functions

		static sint luaGetTalkingAsList(lua_State* state);
		static sint luaGetIncarnatingList(lua_State* state);
		static sint luaGetScenarioHeader(lua_State* state);

		// Ring Access
		static sint luaGetRingAccessAsMap(lua_State* state);
		static sint luaGetSheetRingAccess(lua_State* state);
		static sint luaGetIslandRingAccess(lua_State* state);
		static sint luaUpdateScenarioAck(lua_State* statet);
		static sint luaGetCharacterRingAccess(lua_State* statet);
		static sint luaVerifyRtScenario(lua_State* statet);
		static sint luaCheckRingAccess(lua_State* state);


		static sint luaGetEditSessionLink(lua_State* statet);

		//Scenario files header access
		static sint luaGetFileHeader(lua_State* state);
		static sint luaGetMustVerifyRingAccessWhileLoadingAnimation(lua_State* state);

		static sint luaGetUseVerboseRingAccess(lua_State* state);
		static sint luaGetIsAnimationSession(lua_State* state);

		static sint luaResetNameGiver(lua_State* state);
		static sint luaGetCharIdMd5(lua_State* state);
		static sint luaHasCharacterSameCharacterIdMd5(lua_State* state);
		static sint luaGetScenarioSavePath(lua_State* state);


		static sint luaCheckServerEditionModule(lua_State* state);


		//static sint luaGetPropertyList(lua_State* state);
		~CComLuaModule();
	private:
		lua_State* _LuaState;
		bool	   _LuaOwnerShip;

		CDynamicMapClient* _Client;

		static std::map<lua_State*, CComLuaModule*> _Instance;

		// register the com lib into lua
		void initLuaLib();


		static sint requestInsertNode(lua_State* state, bool ghost);
		static sint requestSetNode(lua_State* state, bool ghost);


		static int luaRequestNewAction(lua_State* state, bool pending, uint count);

	};



} // namespace DMS
#endif // DMS_COMLUAMODULE_H
