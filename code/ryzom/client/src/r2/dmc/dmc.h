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

#ifndef DMS_DMC_H
#define DMS_DMC_H

#include "nel/misc/types_nl.h"
#include "action_historic.h"
#include "game_share/r2_types.h"

#include "idmc.h"
#include <string>


extern "C"
{
	#include "nel/gui/lua_loadlib.h"
}


namespace NLNET
{
	class IModuleSocket;
}
namespace R2
{
	class CFrameActionsRecorder;
	class CScenario;
	class CObjectFactory;
	class CObject;
	class CComLuaModule;
	class CClientAdminModule;
	class CClientAnimationModule;
	class CClientEditionModule;
	class CPropertyAccessor;
	class CClientMessageAdventureUserConnection;
	enum TScenarioSessionType;
	class CDynamicMapClient : public IDynamicMapClient
	{
	public:
//		typedef uint32 TSessionId;
	public:
		void setInstantFeedBackFlag(bool instantFeedBackFlag);
		bool getInstantFeedBackFlag() const { return _InstantFeedBackFlag; }
		// If instant feedback flag has been set, flush the command queue (no-op else);
		void flushActions();

		/** \param luaState NULL If the lua environment should be built by this object
		  *        If the lua state is provided externally, then it won't be deleted by this object
		  */
		CDynamicMapClient(const std::string &eid, NLNET::IModuleSocket * clientGateway, lua_State *luaState);

		void loadFeatures();

		virtual ~CDynamicMapClient();

		void loadDefaultPalette();

		virtual void addPaletteElement(const std::string& attrName, CObject* paletteElement);

		virtual CObject* getPropertyValue(CObject* component, const std::string& attrName) const;

		virtual CObject* getPaletteElement(const std::string& key)const;

		virtual bool isInPalette(const std::string& key) const;

		virtual CObject* newComponent(const std::string& type) const;

		virtual void registerGenerator(CObject* classObject);

		virtual void show() const;

		virtual void requestTranslateFeatures();

		void requestUploadCurrentScenario();

		virtual void requestCreateScenario(CObject* scenario);

		virtual void requestUpdateRtScenario(CObject* rtScenario);

		// request commands -> add in undo / redo buffer
		void requestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value);
		void requestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value);
		void requestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position);
		void requestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);
		CActionHistoric &getActionHistoric() { return _ActionHistoric; }

		// request commands -> send commands for real (not buffered)
		virtual void doRequestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value);
		virtual void doRequestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value);
		virtual void doRequestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position);
		virtual void doRequestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);

		// access to action historic


		virtual void nodeSet(
			const std::string& instanceId, const std::string& attrName, CObject* value);

		virtual void nodeErased(	const std::string& instanceId, const std::string& attrName, sint32 position);

		virtual void nodeInserted( const std::string& instanceId, const std::string& attrName, sint32 position,
			const std::string& key, CObject* value);

		virtual void nodeMoved(
				const std::string& instanceId, const std::string& attrName, sint32 position,
				const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);

		virtual void scenarioUpdated(CObject* highLevel, bool willTP, uint32 startingActIndex);

		virtual void onEditionModeConnected( uint32 userSlotId, uint32 adventureId, CObject* highLevel, const std::string& versionName, bool willTP, uint32 initialActIndex);

		virtual void onAnimationModeConnected(const R2::CClientMessageAdventureUserConnection& connected);

		virtual void onAnimationModePlayConnected();

		virtual void onEditionModeDisconnected();

		virtual void onResetEditionMode();

		virtual void onTestModeConnected();

		virtual void onTestModeDisconnected(TSessionId sessionId, uint32 lasAct, TScenarioSessionType sessionType);

		virtual void onNpcAnimationTargeted(uint32 mode);


		virtual CObject* getPropertyList(CObject* component) const;

		void testConnectionAsCreator();

		void updateScenario(CObject* scenario, bool willTP);
		CObject *translateScenario(CObject* scenario);

		void save(const std::string& filename);

		// return true if loading was ok
		bool load(const std::string& filename);

		// return true if loading was ok
		bool loadAnimationFromBuffer(const std::string& data, const std::string& filename, std::string& errMsg, const CScenarioValidator::TValues& values);

		void doFile(const std::string& filename);

		void runLuaScript(const std::string& filename);

		// find an object in the scenario from its instance ID
		CObject *find(const std::string& instanceId, const std::string& attrName = "", sint32 position = -1, const std::string &key ="");
		virtual void disconnect();
		const CObject *getHighLevel() const;

		CPropertyAccessor &getPropertyAccessor() const;

		CClientEditionModule& getEditionModule() const;

		CComLuaModule& getComLuaModule() const;

		CScenario* getCurrentScenario() const;

		virtual void requestUploadScenario(CObject* scenario);

		virtual void release();

		virtual void init(lua_State *luaState);

		virtual void saveRtData(const std::string& filename);
		virtual void requestTalkAs(const std::string& npcname);
		virtual	CObject* getCurrentScenarioHighLevel();

		TSessionId getSessionId() const;

		void newAction(const ucstring &name);


	protected:

		CClientEditionModule* _EditionModule;

		CComLuaModule*	_ComLua;
		CActionHistoric _ActionHistoric;

		CFrameActionsRecorder *_FrameActionsRecorder;

		bool _InstantFeedBackFlag;
	};

} // namespace DMS
#endif //DMS_DMC_H
