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

#ifndef SIM_DMC_H
#define SIM_DMC_H

#include "nel/misc/types_nl.h"

#include "simulated_idmc.h"
#include <string>

extern "C"
{
//	#include "lua_loadlib.h"

// interface_v3 file, replaced with:
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


namespace NLNET
{
	class IModuleSocket;
}
namespace R2
{

	class CScenario;
	class CObjectFactory;
	class CObject;
	class CComLuaModule;
//	class CClientAdminModule;
	class CSimClientAnimationModule;
	class CSimClientEditionModule;

	class CDynamicMapClient : public IDynamicMapClient
	{
	public:
		/** \param luaState NULL If the lua environment should be built by this object
		  *        If the lua state is provided externally, then it won't be deleted by this object
		  */
		CDynamicMapClient(const std::string &eid, NLNET::IModuleSocket * clientGateway, lua_State *luaState);
		~CDynamicMapClient();

		virtual void init( uint id, lua_State *luaState );
		virtual void release();
		virtual void disconnect();

		bool isSEMConnected();

		void save(const std::string& filename);
		bool load(const std::string& filename);
		CObject *loadScenario(const std::string& filename);
		void loadFeatures();
		void loadDefaultPalette();
		
		virtual void show() const;
	
		virtual void addPaletteElement(const std::string& attrName, CObject* paletteElement);
		
		virtual CObject* newComponent(const std::string& type) const;
		
		virtual void registerGenerator(CObject* classObject);


		virtual void requestGoTest();
		virtual void requestStartAct( uint actId );
		virtual void requestStopTest();
		virtual void requestTranslateFeatures();
		virtual void requestCreateScenario(CObject* scenario);
		virtual void requestUpdateRtScenario(CObject* rtScenario);
		virtual void requestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value);
		virtual void requestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value);
		virtual void requestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position);
		virtual void requestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);

		virtual void nodeSet(const std::string& instanceId, const std::string& attrName, CObject* value);
		virtual void nodeErased(const std::string& instanceId, const std::string& attrName, sint32 position);
		virtual void nodeInserted( const std::string& instanceId, const std::string& attrName, sint32 position,
			const std::string& key, CObject* value);
		virtual void nodeMoved(const std::string& instanceId, const std::string& attrName, sint32 position,
				const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);

		virtual void scenarioUpdated(CObject* highLevel);

		virtual void onEditionModeConnected( uint32 userSlotId, uint32 adventureId, CObject* highLevel);
		virtual void onEditionModeDisconnected();
		virtual void onResetEditionMode();
		virtual void onTestModeConnected();
		virtual void onTestModeDisconnected();

//		virtual void onNpcAnimationTargeted(uint32 mode);


		void testConnectionAsCreator();

		void updateScenario(CObject* scenario);

		void doFile(const std::string& filename);
		void runLuaScript(const std::string& filename);

		// find an object in the scenario from its instance ID
		CObject* find(const std::string& instanceId);		

		CSimClientEditionModule& getEditionModule() const;
		CSimClientAnimationModule& getAnimationModule() const;
//		CClientAdminModule& getAdminModule() const;

		CScenario* getCurrentScenario() const;
		virtual	CObject* getCurrentScenarioHighLevel();
		const CObject *getHighLevel() const;
		virtual CObject* getPropertyValue(CObject* component, const std::string& attrName) const;
		virtual CObject* getPropertyList(CObject* component) const;
		virtual CObject* getPaletteElement(const std::string& key)const;
//		CPropertyAccessor &getPropertyAccessor() const;
		
		virtual void requestUploadScenario(CObject* scenario);
		
		virtual void saveRtData(const std::string& filename);
//		virtual void requestTalkAs(const std::string& npcname);

	protected:
		uint						 _Id;
		NLNET::IModuleSocket		*_SocketGateway;

//		CClientAdminModule			*_AdminModule;
		CSimClientAnimationModule	*_AnimationModule;
		CSimClientEditionModule		*_EditionModule;

		CComLuaModule				* _ComLua;
	};

} // namespace R2
#endif //SIM_DMC_H
