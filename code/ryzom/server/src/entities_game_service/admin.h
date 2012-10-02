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



#ifndef RY_ADMIN_H
#define RY_ADMIN_H

//
// Includes
//

#include <string>
#include <vector>

#include "server_share/log_character_gen.h"

struct CAdminCommand
{
	std::string	Name;
	bool		AddEId;
	std::string	Priv;
	std::string ForwardToservice;
	bool		Audit;
};

//
// Functions
//

void initAdmin ();
void initCommandsPrivileges(const std::string & fileName);
void initPositionFlags(const std::string & fileName);
void initSalt();
const std::string &getSalt();

CAdminCommand * findAdminCommand(const std::string & name);

extern void GET_CHARACTER_Helper(std::string& command, const NLMISC::CEntityId& id, const std::string& adminCommand);

#define GET_CHARACTER \
	if (args.size() < 1) { nlwarning ("Missing argument number 0 that should be the eid"); return false; } \
	CEntityId eid(args[0]); \
	if (eid == CEntityId::Unknown) \
		return true; \
	TLogContext_Character_AdminCommand commandContext(eid); \
	CCharacter *c = PlayerManager.getChar(eid); \
	if(c == 0) \
	{ \
		log.displayNL ("Unknown player '%s' (%s)", eid.toString().c_str(), args[0].c_str()); \
		goto offline; \
	} \
	if(!c->getEnterFlag()) \
	{ \
		log.displayNL ("'%s' is not entered", eid.toString().c_str()); \
		goto offline; \
	} \
	if(!TheDataset.isAccessible(c->getEntityRowId())) \
	{ \
		log.displayNL ("'%s' is not valid in mirror", eid.toString().c_str()); \
		goto offline; \
	} \
	goto end; \
	offline: \
	{\
		std::string command; \
		std::string adminCommand; \
		adminCommand = getName(); \
		for( uint i = 0; i < args.size(); i++ ) \
		{ \
			adminCommand += string(" \"") + args[i] + string("\""); \
		} \
		GET_CHARACTER_Helper( command, eid, adminCommand ); \
		return true; \
	} \
	end: 

//#define GET_CHARACTER1 \
//	if (args.size() < 2) { nlwarning ("Missing argument number 1 that should be the eid"); return false; } \
//	CEntityId eid = CEntityIdTranslator::getInstance()->getByEntity(args[1]); \
//	if (eid == CEntityId::Unknown) \
//	return true; \
//	CCharacter *c = PlayerManager.getChar(eid); \
//	if(c == 0) \
//{ \
//	log.displayNL ("Unknown player '%s' (%s)", eid.toString().c_str(), args[0].c_str()); \
//	goto offline2; \
//} \
//	if(!c->getEnterFlag()) \
//{ \
//	log.displayNL ("'%s' is not entered", eid.toString().c_str()); \
//	goto offline2; \
//} \
//	if(!TheDataset.isAccessible(c->getEntityRowId())) \
//{ \
//	log.displayNL ("'%s' is not valid in mirror", eid.toString().c_str()); \
//	goto offline2; \
//} \
//	goto end2; \
//offline2: \
//{\
//	std::string command; \
//	std::string adminCommand; \
//	adminCommand = getName(); \
//	for( uint i = 0; i < args.size(); i++ ) \
//{ \
//	adminCommand += string(" \"") + args[i] + string("\""); \
//} \
//	GET_CHARACTER_Helper( command, eid, adminCommand ); \
//	return true; \
//} \
//end2: 

#endif // RY_ADMIN_H

/* End of admin.h */
