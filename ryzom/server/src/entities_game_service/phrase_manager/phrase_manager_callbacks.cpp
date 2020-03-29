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

#include "phrase_manager/phrase_manager_callbacks.h"
#include "phrase_manager/phrase_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;



//--------------------------------------------------------------
//					CEGSExecuteMsg::callback()  
//--------------------------------------------------------------
void CEGSExecuteMsgImp::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
	H_AUTO(CEGSExecuteMsgImpCallback);
	if ( ! Mirror.mirrorIsReady() )
	{
		nlwarning("<CEGSExecuteMsg::callback> Received from %s service but mirror not yet ready", serviceName.c_str() );
		return;
	}

	CPhraseManager::getInstance().executePhrase( ActorRowId, TargetRowId, BrickIds, Cyclic );
} // CEGSExecuteMsg::callback  //


//--------------------------------------------------------------
//					cbRegisterService()  
//--------------------------------------------------------------
void cbRegisterService( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CPhraseManager::getInstance().registerService( serviceId );
} // cbRegisterService //


//--------------------------------------------------------------
//					cbUnregisterService()  
//--------------------------------------------------------------
void cbUnregisterService( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CPhraseManager::getInstance().unregisterService( serviceId );
} // cbUnregisterService //


//--------------------------------------------------------------
//					cbRegisterServiceAI()  
//--------------------------------------------------------------
void cbRegisterServiceAI( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CPhraseManager::getInstance().registerServiceForAI( serviceId );
} // cbRegisterServiceAI //


//--------------------------------------------------------------
//					cbUnregisterServiceAI()  
//--------------------------------------------------------------
void cbUnregisterServiceAI( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CPhraseManager::getInstance().unregisterServiceForAI( serviceId );
} // cbUnregisterServiceAI //


//--------------------------------------------------------------
//					cbDisengageNotification()  
//--------------------------------------------------------------
void cbDisengageNotification( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbDisengageNotification);
	
	if ( ! Mirror.mirrorIsReady() )
	{
		nlwarning("<cbDisengageNotification> Received from %s service but mirror not yet ready", serviceName.c_str() );
		return;
	}
	
	if (NLMISC::nlstricmp(serviceName.c_str(),"AIS")==0)
	{
		TDataSetRow entityRowId;
		msgin.serial( entityRowId );
		
		INFOLOG("<cbDisengageNotification> received disengage notification for entity %s", TheDataset.getEntityId(entityRowId).toString().c_str());
		
		CPhraseManager::getInstance().disengage( entityRowId, true );
	}
	else
	{
		CEntityId entityId;
		msgin.serial( entityId );
		
		INFOLOG("<cbDisengageNotification> received disengage notification for entity %s", entityId.toString().c_str());
		
		CPhraseManager::getInstance().disengage( TheDataset.getDataSetRow(entityId), true );
	}
	
} // cbDisengageNotification //


//--------------------------------------------------------------
//					cbDisengage()  
//--------------------------------------------------------------
void cbDisengage( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbDisengage);
	
	if ( ! Mirror.mirrorIsReady() )
	{
		nlwarning("<cbDisengage> Received from %s service but mirror not yet ready", serviceName.c_str() );
		return;
	}

	if (serviceName == "AIS")
	{
		TDataSetRow entityRowId;
		msgin.serial( entityRowId );
		
		DEBUGLOG("<cbDisengage> AIS Disengage entity %s", TheDataset.getEntityId(entityRowId).toString().c_str() );
		
		CPhraseManager::getInstance().disengage( entityRowId, true /*chatMsg*/, true /*disengageCreature*/);
	}
	else
	{
		CEntityId entityId;
		msgin.serial( entityId );

		DEBUGLOG("<cbDisengage> Service %s Disengage entity %s", serviceName.c_str(), entityId.toString().c_str() );

		CPhraseManager::getInstance().disengage( TheDataset.getDataSetRow(entityId), true );
	}
	
} // cbDisengage //


//--------------------------------------------------------------
//				CBSAIEventReportMsg::callback()  
//--------------------------------------------------------------
void CBSAIDeathReport::callback(const string &, NLNET::TServiceId )
{
	// Nothing to do.
}

//--------------------------------------------------------------
//				CBSAIEventReportMsg::callback()  
//--------------------------------------------------------------
void CBSAIEventReportMsg::callback(const string &, NLNET::TServiceId )
{
	// empty , unused
} // CBSAIEventReportMsg::callback //


//--------------------------------------------------------------
//				CEGSExecutePhraseMsg::callback()  
//--------------------------------------------------------------
void CEGSExecutePhraseMsg::callback(const string &serviceName, NLNET::TServiceId sid)
{
	H_AUTO(CEGSExecutePhraseMsg_callback);
	CPhraseManager::getInstance().executePhrase( ActorRowId, TargetRowId, PhraseId, false );
} // CEGSExecutePhraseMsg::callback //


//--------------------------------------------------------------
//				CEGSExecuteAiActionMsgImp::callback()  
//--------------------------------------------------------------
void CEGSExecuteAiActionMsgImp::callback(const string &serviceName, NLNET::TServiceId sid)
{	
	H_AUTO(CEGSExecuteAiActionMsgImp_callback);
	CPhraseManager::getInstance().executeAiAction(ActorRowId, TargetRowId, ActionId, DamageCoef, DamageSpeedCoef);
} // CEGSExecuteAiActionMsgImp::callback //
