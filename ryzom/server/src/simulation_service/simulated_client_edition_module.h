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

#ifndef R2_SIM_CLIENT_EDITON_MODULE_H
#define R2_SIM_CLIENT_EDITON_MODULE_H

#include "nel/misc/types_nl.h"

#include "nel/net/module.h"

namespace R2
{
	class CObject;
//	class CDynamicMapService;
	class CDynamicMapClient;
//	class CPalette;
//	class CTranslationModule;
	class CScenario;
	class CObjectFactory;

//	class CPropertyAccessor;


	class CSimClientEditionModule : public NLNET::CModuleBase
	{
	public:
		CSimClientEditionModule();
		~CSimClientEditionModule();

		void init( uint id, NLNET::IModuleSocket *clientGW, CDynamicMapClient* client);
		void release();

		void updateScenario(CObject* scenario);

		// message handlers
		void onModuleUp(NLNET::IModuleProxy *moduleProxy);
		void onModuleDown(NLNET::IModuleProxy *moduleProxy);
		void onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message);
		void onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy);
		void onAdventureConnected(uint32 userSlotId, uint32 scenarioId, uint32 connectedAs, CObject* scenario );
		// stubbed
		void onServiceUp(const std::string &serviceName, uint16 serviceId) {}
		void onServiceDown(const std::string &serviceName, uint16 serviceId) {}
		void onModuleUpdate() {}
		void onApplicationExit() {}
		void onModuleSocketEvent(NLNET::IModuleSocket *moduleSocket, TModuleSocketEvent eventType) {}

		void requestCreatePrimitives();
		// request handlers
		void requestCreateScenario(CObject* scenario);
		void requestUploadScenario(CObject* scenario);
		void requestUpdateRtScenario( CObject* scenario);
		void requestGoTest();
		void requestStopTest();
		void requestStartAct(uint32 actId);
		void requestStopAct();
		void requestMapConnection( uint32 scenarioId, bool mustTp = true );
		void requestReconnection();
//		void requestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value);
//		void requestSetNodeNoTest(const std::string& instanceId, const std::string& attrName, CObject* value);
//		void requestEraseNode( 
//				const std::string& instanceId, const std::string& attrName, sint32 position);
//		void requestInsertNode( 
//				const std::string& instanceId, const std::string& attrName, sint32 position,
//				const std::string& key, CObject* value);
//		void requestMoveNode( 
//				const std::string& instanceId, const std::string& attrName, sint32 position,
//				const std::string& desInstanceId, const std::string& destAttrName, sint32 destPosition);

		// accessors
		CScenario* getCurrentScenario() const	{ return _Scenario; }
//		CObject* getPropertyValue(const std::string& instanceId, const std::string& attrName) const;
//		CObject* getPropertyValue(CObject* component, const std::string& attrName) const;
//		CObject* getPropertyList(CObject* component) const;
//		CObject* getPaletteElement(const std::string& key)const;
//		CObject* newComponent(const std::string& type) const;
//		CPropertyAccessor& getPropertyAccessor() const;
		std::string getEid() const { return _Eid; }
		uint32 getCurrentAdventureId() const { return _AdventureId; }
		bool isSEMConnected() const	{ return (_ServerEditionProxy != NULL); }

		// settors
		void setEid(const std::string& eid);
		void setCurrentAdventureId( uint32 adventureId) { _AdventureId = adventureId; }
//		void addPaletteElement(const std::string& attrName, CObject* paletteElement);

//		void registerGenerator(CObject* classObject);

	private:
		uint _Id;

//		CTranslationModule* _TranslationModule;
		CScenario* _Scenario;
		CObjectFactory* _Factory;
//		CPalette* _Palette;		

//		CPropertyAccessor* _PropertyAccessor;
//		NLNET::IModuleSocket* _ClientGw;
//		NLNET::TModuleId _ServerEditionModuleId;
		NLNET::TModuleProxyPtr _ServerEditionProxy;
		CDynamicMapClient* _Client;
		std::string _Eid;
		uint32 _AdventureId;
	};

} //namespace R2

#endif // R2_SIM_CLIENT_EDITON_MODULE_H



