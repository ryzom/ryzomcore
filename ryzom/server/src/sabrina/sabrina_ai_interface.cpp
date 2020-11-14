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




//-----------------------------------------------
// includes
//-----------------------------------------------
// nel
#include "nel/net/message.h"
// sabrina
#include "sabrina_ai_interface.h"
#include "sabrina_message_callbacks.h"


//-----------------------------------------------
// singleton data 
//-----------------------------------------------

std::vector<uint8>					CSabrinaAIInterface::_RegisteredServices;
CSabrinaAIInterface::TAIEventList	CSabrinaAIInterface::_AIEvents;
std::vector<CAiEventReport>			CSabrinaAIInterface::_AIEventReports;


//-----------------------------------------------
//			init()
//-----------------------------------------------
void CSabrinaAIInterface::init()
{
	//array of callback items
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "REGISTER_AI_EVENT_REPORTS",		SABRINA::cbRegisterServiceAI	},
		{ "UNREGISTER_AI_EVENT_REPORTS",	SABRINA::cbUnregisterServiceAI	},
	};

	NLNET::CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
}

//-----------------------------------------------
//			flushEvents()
//-----------------------------------------------
void CSabrinaAIInterface::flushEvents()
{
	// iterator for vector of registered services
	std::vector<uint8>::iterator it;

	if ( !_AIEvents.empty() )
	{
		// build the message to send
		NLNET::CMessage msgai("AI_EVENTS");
		uint16 size = _AIEvents.size();
		msgai.serial( size );

		TAIEventList::iterator eventIt;
		for (eventIt = _AIEvents.begin() ; eventIt !=  _AIEvents.end(); ++eventIt)
		{
			msgai.serial( *(*eventIt) );
			delete (*eventIt);
		}

		// dispatch the message to all registered services
		for (it = _RegisteredServices.begin() ; it != _RegisteredServices.end() ; ++it)
		{
			sendMessageViaMirror (*it, msgai);
//			INFOLOG("Send AI_EVENTS to AI service %d", (*it) );
		}

		// clear the event report buffer
		_AIEvents.clear();
	}

	if ( !_AIEventReports.empty() )
	{
		// build the message to send
		CBSAIEventReportMsg msgAI;
		const uint nbAiReports = _AIEventReports.size();
		for (uint i = 0 ; i < nbAiReports ; ++i )
		{
			msgAI.pushBack( _AIEventReports[i] );
		}

		// dispatch the message to all registered services
		for (it = _RegisteredServices.begin() ; it != _RegisteredServices.end() ; ++it)
		{
			msgAI.send (*it );
//			INFOLOG("Send EVENT_REPORTS to AI service %d", (*it) );
		}

		// clear the event report buffer
		_AIEventReports.clear();
	}
} 


//-----------------------------------------------
//			init()
//-----------------------------------------------
void CSabrinaAIInterface::release()
{
}

//--------------------------------------------------------------
//				registerAIService()  
//--------------------------------------------------------------
void CSabrinaAIInterface::registerAIService( uint8 serviceId ) 
{ 
	// make sure the service isn't already in our vector
	for (uint32 i=0;i<_RegisteredServices.size();++i)
		if (_RegisteredServices[i]==serviceId)
		{
			nlwarning("BUG: CSabrinaPhraseManager::unregisterAIService(): service '%d' not found in registerd AI service vector",serviceId);
			#ifdef NL_DEBUG
				nlstop
			#endif
			return;
		}
	// add the service to the vector
	_RegisteredServices.push_back( serviceId ); 
}

//--------------------------------------------------------------
//				unregisterAIService()  
//--------------------------------------------------------------
void CSabrinaAIInterface::unregisterAIService( uint8 serviceId ) 
{ 
	uint32 foundCount=0;
	uint32 i=0;
	// remove all occurences of the service from the services vector
	while (i<_RegisteredServices.size())
	{
		if (_RegisteredServices[i]==serviceId)
		{
			++foundCount;
			_RegisteredServices[i]=_RegisteredServices[_RegisteredServices.size()-1];
			_RegisteredServices.pop_back();
		}
		else
		{
			++i;
		}
	}
	// make sure the service was found and only found once in the vector
	if (foundCount==0)
	{
		nlwarning("BUG: CSabrinaPhraseManager::unregisterAIService(): service '%d' not found in registerd AI service vector",serviceId);
		#ifdef NL_DEBUG
			nlstop
		#endif
	}
	else if (foundCount>0)
	{
		nlwarning("BUG: CSabrinaPhraseManager::unregisterAIService(): found more than one ref to service '%d' in registerd AI service vector",serviceId);
		#ifdef NL_DEBUG
			nlstop
		#endif
	}
}

//--------------------------------------------------------------
//				addAiEventReport()  
//--------------------------------------------------------------
void CSabrinaAIInterface::addAiEventReport( const CAiEventReport &report )
{
	_AIEventReports.push_back(report);
}

//--------------------------------------------------------------
//				addAIEvent()  
//--------------------------------------------------------------
void CSabrinaAIInterface::addAIEvent( const IAIEvent *event )
{
	if (event != NULL)
		_AIEvents.push_back( const_cast<IAIEvent*>(event) );
}
