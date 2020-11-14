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



#ifndef EGS_PHRASE_MANAGER_CALLBACKS_H
#define EGS_PHRASE_MANAGER_CALLBACKS_H

//
#include "server_share/msg_brick_service.h"

/**
 * Implementation of the phrase execution description Transport class
 */
class CEGSExecuteMsgImp : public CEGSExecuteMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId sid);
};

/**
 * Implementation of the Ai Action execution description Transport class
 */
class CEGSExecuteAiActionMsgImp : public CEGSExecuteAiActionMsg
{
public:
	virtual void callback(const std::string &serviceName, NLNET::TServiceId sid);
};

/// register a service for event reports
void cbRegisterService( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

/// unregister a service for event reports
void cbUnregisterService( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

/// register a service for AI event reports
void cbRegisterServiceAI( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

/// unregister a service for AI event reports
void cbUnregisterServiceAI( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

/// receive a disengage notification for specified entity (from CMS typically)
void cbDisengageNotification( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

/// receive a Disengage command
void cbDisengage( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

#endif // EGS_PHRASE_MANAGER_CALLBACKS_H

/* End of brick_manager_callbacks.h */
