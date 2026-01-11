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

// game share
//#include "game_share/msg_combat_move_service.h"
//sabrina
#include "sabrina_message_callbacks.h"
//#include "phrase_utilities_functions.h"

/*
extern CMirror			Mirror;
extern CMirroredDataSet	*FeTempDataset;
*/

//--------------------------------------------------------------
//					CEGSExecuteMsg::callback()  
//--------------------------------------------------------------
void CEGSExecuteMsgImp::callback (const std::string &serviceName, NLNET::TServiceId serviceId)
{
	nlwarning("*** Received untreated message 'CEGSExecuteMsgImp' from service %s(%d)",serviceName.c_str(),serviceId);
//	if ( ! Mirror.mirrorIsReady() )
//	{
//		nlwarning("CEGSExecuteMsgImp::callback(): Message received from %s(%d) service but mirror not yet ready", serviceName.c_str(), sid );
//		return;
//	}
//
//	//PhraseManager->executePhrase( TheDataset.getEntityId(PlayerId), TheDataset.getEntityId(TargetId), Bricks/*, MPsSheet, MPsQty, MPsQuality*/, (BRICK_TYPE::EBrickType)Type, Index );
//	PhraseManager->executePhrase( ActorRowId, TargetRowId, BrickIds, Cyclic );
} // CEGSExecuteMsg::callback  //

//--------------------------------------------------------------
//				CBSAIEventReportMsg::callback()  
//--------------------------------------------------------------
void CBSAIDeathReport::callback(const std::string &serviceName, NLNET::TServiceId serviceId)
{
	nlwarning("*** Received untreated message 'CBSAIDeathReport' from service %s(%d)",serviceName.c_str(),serviceId);
	// Nothing to do.
}

//--------------------------------------------------------------
//				CBSAIEventReportMsg::callback()  
//--------------------------------------------------------------
void CBSAIEventReportMsg::callback(const std::string &serviceName, NLNET::TServiceId serviceId)
{
	nlwarning("*** Received untreated message 'CBSAIEventReportMsg' from service %s(%d)",serviceName.c_str(),serviceId);
	// empty , unused
} // CBSAIEventReportMsg::callback //


namespace SABRINA
{

	//--------------------------------------------------------------
	//					cbCancelPhrase()  
	//--------------------------------------------------------------
	void cbCancelPhrase( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbCancelPhrase' from service %s(%d)",serviceName.c_str(),serviceId);
	//	if ( ! Mirror.mirrorIsReady() )
	//	{
	//		nlwarning("<cbCancelPhrase> Received from %s service but mirror not yet ready", serviceName.c_str() );
	//		return;
	//	}
	//
	//	if (NLMISC::nlstricmp(serviceName.c_str(),"AIS")==0)
	//	{
	//		TDataSetRow	playerId;
	//		msgin.serial( playerId );
	//		
	//		BRICK_TYPE::EBrickType type = BRICK_TYPE::UNKNOWN;
	//		msgin.serialEnum( type );
	//		
	//		uint8 index;
	//		msgin.serial( index );
	//		
	//		//PhraseManager->cancelSentence( TheDataset.getEntityId(playerId), type, index );
	//	}
	//	else
	//	{
	//		NLMISC::CEntityId playerId;
	//		msgin.serial( playerId );
	//		
	//		BRICK_TYPE::EBrickType type = BRICK_TYPE::UNKNOWN;
	//		msgin.serialEnum( type );
	//		
	//		uint8 index;
	//		msgin.serial( index );
	//		
	//		//PhraseManager->cancelSentence( playerId, type, index );
	//	}
	} // cbCancelPhrase //



	//--------------------------------------------------------------
	//					cbCancelCurrentPhrase()  
	//--------------------------------------------------------------
	void cbCancelCurrentPhrase( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbCancelCurrentPhrase' from service %s(%d)",serviceName.c_str(),serviceId);
	//	if ( ! Mirror.mirrorIsReady() )
	//	{
	//		nlwarning("<cbCancelCurrentPhrase> Received from %s service but mirror not yet ready", serviceName.c_str() );
	//		return;
	//	}
	//
	//	if (NLMISC::nlstricmp(serviceName.c_str(),"AIS")==0)
	//	{
	//		TDataSetRow playerId;
	//		msgin.serial( playerId );
	//		
	////		PhraseManager->cancelCurrentSentence( TheDataset.getEntityId(playerId) );
	//	}
	//	else
	//	{
	//		NLMISC::CEntityId playerId;
	//		msgin.serial( playerId );
	//		
	////		PhraseManager->cancelCurrentSentence( playerId );
	//	}
	} // cbCancelCurrentPhrase //



	//--------------------------------------------------------------
	//					cbCancelAllPhrases()  
	//--------------------------------------------------------------
	void cbCancelAllPhrases( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbCancelAllPhrases' from service %s(%d)",serviceName.c_str(),serviceId);
	//	if ( ! Mirror.mirrorIsReady() )
	//	{
	//		nlwarning("<cbCancelAllPhrases> Received from %s service but mirror not yet ready", serviceName.c_str() );
	//		return;
	//	}
	//	
	//	if (NLMISC::nlstricmp(serviceName.c_str(),"AIS")==0)
	//	{
	//		TDataSetRow playerId;
	//		msgin.serial( playerId );
	//		
	////		PhraseManager->cancelAllSentences( TheDataset.getEntityId(playerId) );
	//	}
	//	else
	//	{
	//		NLMISC::CEntityId playerId;
	//		msgin.serial( playerId );
	//		
	////		PhraseManager->cancelAllSentences( playerId );
	//	}
	} // cbCancelAllPhrases //


	//--------------------------------------------------------------
	//					cbRegisterService()  
	//--------------------------------------------------------------
	void cbRegisterService( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbRegisterService' from service %s(%d)",serviceName.c_str(),serviceId);
	//	PhraseManager->registerService( serviceName );
	} // cbRegisterService //


	//--------------------------------------------------------------
	//					cbUnregisterService()  
	//--------------------------------------------------------------
	void cbUnregisterService( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbUnregisterService' from service %s(%d)",serviceName.c_str(),serviceId);
	//	PhraseManager->unregisterService( serviceName );
	} // cbUnregisterService //


	//--------------------------------------------------------------
	//					cbRegisterServiceAI()  
	//--------------------------------------------------------------
	void cbRegisterServiceAI( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbRegisterServiceAI' from service %s(%d)",serviceName.c_str(),serviceId);
	//	PhraseManager->registerServiceForAI( serviceName );
	} // cbRegisterServiceAI //


	//--------------------------------------------------------------
	//					cbUnregisterServiceAI()  
	//--------------------------------------------------------------
	void cbUnregisterServiceAI( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbUnregisterServiceAI' from service %s(%d)",serviceName.c_str(),serviceId);
	//	PhraseManager->unregisterServiceForAI( serviceName );
	} // cbUnregisterServiceAI //


	//--------------------------------------------------------------
	//					cbDisengageNotification()  
	//--------------------------------------------------------------
	void cbDisengageNotification( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbDisengageNotification' from service %s(%d)",serviceName.c_str(),serviceId);
	//	if ( ! Mirror.mirrorIsReady() )
	//	{
	//		nlwarning("<cbDisengageNotification> Received from %s service but mirror not yet ready", serviceName.c_str() );
	//		return;
	//	}
	//	
	//	if (NLMISC::nlstricmp(serviceName.c_str(),"AIS")==0)
	//	{
	//		TDataSetRow entityRowId;
	//		msgin.serial( entityRowId );
	//		
	////		INFOLOG("<cbDisengageNotification> received disengage notification for entity %s", TheDataset.getEntityId(entityRowId).toString().c_str());
	//		
	//		PhraseManager->disengage( entityRowId, true );
	//	}
	//	else
	//	{
	//		CEntityId entityId;
	//		msgin.serial( entityId );
	//		
	//		INFOLOG("<cbDisengageNotification> received disengage notification for entity %s", entityId.toString().c_str());
	//		
	//		PhraseManager->disengage( TheDataset.getDataSetRow(entityId), true );
	//	}
	} // cbDisengageNotification //


	//--------------------------------------------------------------
	//					cbDisengage()  
	//--------------------------------------------------------------
	void cbDisengage( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbDisengage' from service %s(%d)",serviceName.c_str(),serviceId);
	//
	//	if ( ! Mirror.mirrorIsReady() )
	//	{
	//		nlwarning("<cbDisengage> Received from %s service but mirror not yet ready", serviceName.c_str() );
	//		return;
	//	}
	//
	//	if (NLMISC::nlstricmp(serviceName.c_str(),"AIS")==0)
	//	{
	//		TDataSetRow entityRowId;
	//		msgin.serial( entityRowId );
	//		
	//		DEBUGLOG("<cbDisengage> AIS Disengage entity %s", TheDataset.getEntityId(entityRowId).toString().c_str() );
	//		
	//		PhraseManager->disengage( entityRowId, true /*chatMsg*/, true /*disengageCreature*/);
	//	}
	//	else
	//	{
	//		CEntityId entityId;
	//		msgin.serial( entityId );
	//
	//		DEBUGLOG("<cbDisengage> Service %s Disengage entity %s", serviceName.c_str(), entityId.toString().c_str() );
	//
	//		PhraseManager->disengage( TheDataset.getDataSetRow(entityId), true );
	//	}
	//	
	} // cbDisengage //




	//class CEntitiesSortFunctor
	//{
	//public:
	//	inline CEntitiesSortFunctor()
	//	{}
	//
	//	inline bool operator() (const pair<CEntityId, sint32> &arg1, const pair<CEntityId, sint32> &arg2)
	//	{
	//		return ( arg1.second < arg2.second);
	//	}
	//};


	//--------------------------------------------------------------
	//				cbVisionAnswer()  
	//--------------------------------------------------------------
	void cbVisionAnswer( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
	{
		nlwarning("*** Received untreated message 'cbVisionAnswer' from service %s(%d)",serviceName.c_str(),serviceId);
	//	if ( ! Mirror.mirrorIsReady() )
	//	{
	//		nlwarning("<cbVisionAnswer> Received from %s service but mirror not yet ready", serviceName.c_str() );
	//		return;
	//	}
	///*	
	//	if (NLMISC::nlstricmp(serviceName.c_str(),"AIS")==0)
	//	{
	//		//get the answer to the vision request
	//		sint32 rid;
	//		sint32 range;
	//		TDataSetRow	id;
	//		vector<pair<CEntityId, sint32> >	entities;
	//		
	//		msgin.serial(rid);
	//		const sint32 msgLength = (sint32)msgin.length(); 
	//		//while( msgin.getPos() <= msgin.length()-1 )
	//		while( msgin.getPos() < msgLength )
	//		{
	//			msgin.serial(id);
	//			msgin.serial(range);
	//			entities.push_back(make_pair(TheDataset.getEntityId(id),range));
	//		}
	//		
	//#ifdef NL_DEBUG
	//		DEBUGLOG("Received entities :");
	//		for (uint i = 0 ; i < entities.size() ; ++i)
	//		{
	//			DEBUGLOG("Entity %s, distance = %d", entities[i].first.toString().c_str(),entities[i].second);
	//		}
	//#endif
	//		
	//		// sort entities by the distance from the area center
	//		sort( entities.begin(), entities.end(), CEntitiesSortFunctor() );
	//		
	//#ifdef NL_DEBUG
	//		DEBUGLOG("After sorting :");
	//		for (uint j = 0 ; j < entities.size() ; ++j)
	//		{
	//			DEBUGLOG("Entity %s, distance = %d", entities[j].first.toString().c_str(),entities[j].second);
	//		}
	//#endif
	//		
	//		//get the functionalities which requested the vision
	//		map<sint32, pair<IAreaFunctionality*,CSentence*> >& functionalities = PhraseManager->getFunctionalitiesAwaitingArea();
	//		map<sint32, pair<IAreaFunctionality*,CSentence*> >::iterator it = functionalities.find(rid);
	//		if ( it != functionalities.end() )
	//		{
	//			IAreaFunctionality *areaFunc = (*it).second.first;
	//			CSentence *sentence = (*it).second.second;
	//			
	//			if ( areaFunc  && sentence )
	//			{
	//				areaFunc->processEntitiesInArea( entities, sentence );
	//			}
	//			functionalities.erase(it);
	//		}
	//		
	//	}
	//	else
	//	{
	//		//get the answer to the vision request
	//		sint32 rid;
	//		sint32 range;
	//		CEntityId id;
	//		vector<pair<CEntityId, sint32> >	entities;
	//		
	//		msgin.serial(rid);
	//		const sint32 msgLength = (sint32)msgin.length(); 
	//		//while( msgin.getPos() <= msgin.length()-1 )
	//		while( msgin.getPos() < msgLength )
	//		{
	//			msgin.serial(id);
	//			msgin.serial(range);
	//			entities.push_back(make_pair(id,range));
	//		}
	//		
	//#ifdef NL_DEBUG
	//		DEBUGLOG("Received entities :");
	//		for (uint i = 0 ; i < entities.size() ; ++i)
	//		{
	//			DEBUGLOG("Entity %s, distance = %d", entities[i].first.toString().c_str(),entities[i].second);
	//		}
	//#endif
	//		
	//		// sort entities by the distance from the area center
	//		sort( entities.begin(), entities.end(), CEntitiesSortFunctor() );
	//		
	//#ifdef NL_DEBUG
	//		DEBUGLOG("After sorting :");
	//		for (uint j = 0 ; j < entities.size() ; ++j)
	//		{
	//			DEBUGLOG("Entity %s, distance = %d", entities[j].first.toString().c_str(),entities[j].second);
	//		}
	//#endif
	//		
	//		//get the functionalities which requested the vision
	//		map<sint32, pair<IAreaFunctionality*,CSentence*> >& functionalities = PhraseManager->getFunctionalitiesAwaitingArea();
	//		map<sint32, pair<IAreaFunctionality*,CSentence*> >::iterator it = functionalities.find(rid);
	//		if ( it != functionalities.end() )
	//		{
	//			IAreaFunctionality *areaFunc = (*it).second.first;
	//			CSentence *sentence = (*it).second.second;
	//			
	//			if ( areaFunc  && sentence )
	//			{
	//				areaFunc->processEntitiesInArea( entities, sentence );
	//			}
	//			functionalities.erase(it);
	//		}
	//
	//	}
	//*/	
	} // cbVisionAnswer //

}