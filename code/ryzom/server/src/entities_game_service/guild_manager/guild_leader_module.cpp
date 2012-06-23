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
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "guild_leader_module.h"
#include "guild_member.h"
#include "guild.h"
#include "egs_utils.h"
#include "nel/misc/eid_translator.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CGuildLeaderModule);

//----------------------------------------------------------------------------
void CGuildLeaderModule::setLeader( uint16 index,uint8 session)
{
	MODULE_AST( _GuildMemberCore );
	MODULE_AST( _GuildMemberCore->getGuild() );
	CGuild * guild = EGS_PD_CAST<CGuild*>(_GuildMemberCore->getGuild());
	EGS_PD_AST( guild );
	CGuildCharProxy proxy;
	getProxy(proxy);
	proxy.cancelAFK();
	if ( guild->getMembersSession() != session )
	{
		proxy.sendSystemMessage( "GUILD_BAD_SESSION" );
		return;
	}
	CGuildMember * memberPD = guild->getMemberByIndex( index );
	if ( memberPD == NULL )
	{
		nlwarning("<GUILD>%s set invalid member idx %u as leader",proxy.getId().toString().c_str(),index );
		return;
	}

	_GuildMemberCore->setMemberGrade(memberPD->getGrade());
	memberPD->setMemberGrade(EGSPD::CGuildGrade::Leader);

	// send system message
	SM_STATIC_PARAMS_3(params,STRING_MANAGER::player,STRING_MANAGER::string_id,STRING_MANAGER::string_id);
	params[0].setEIdAIAlias( proxy.getId(), CAIAliasTranslator::getInstance()->getAIAlias(proxy.getId()) );
	params[1].StringId = CEntityIdTranslator::getInstance()->getEntityNameStringId(memberPD->getIngameEId());
	params[2].StringId = guild->getNameId();
	sendMessageToGuildMembers("GUILD_SET_LEADER",params);

	CGuildMember * myself =  _GuildMemberCore;
	myself->removeReferencingModule(this);
	onReferencedDestruction();
	//guild->setMemberOffline( myself );
	IModule * moduleNew = createModule(proxy,myself);
	guild->setMemberClientDB( myself );
	MODULE_AST(moduleNew);

	// If the player is online, the module must be recreated. Do as the reference was destroyed
	CGuildMemberModule * module = NULL;
	if ( memberPD->getReferencingModule(module) )
	{
		CGuildCharProxy targetProxy;
		module->getProxy(targetProxy);
		memberPD->removeReferencingModule(module);
		module->onReferencedDestruction();
		IModule * moduleTarget = createModule(targetProxy,memberPD);
		guild->setMemberClientDB(memberPD);
		MODULE_AST(moduleTarget);
	}
}

//----------------------------------------------------------------------------
void CGuildLeaderModule::quitGuild()
{
	MODULE_AST( _GuildMemberCore );
	NLMISC::CRefPtr<CGuild> guild = EGS_PD_CAST<CGuild*>(_GuildMemberCore->getGuild());
	EGS_PD_AST(guild);

	// the leader quits : find a successor
	CGuildMember * successor = NULL;
	// best successor is the member with best grade. If more than 1 user fits, take the older in the guild 
	for (map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::iterator it = _GuildMemberCore->getGuild()->getMembersBegin();
		it != _GuildMemberCore->getGuild()->getMembersEnd();
		++it  )
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember*>( (*it).second );
		EGS_PD_AST( member );
		// ignore current leader
		if ( _GuildMemberCore->getId() == member->getId() )
			continue;

		// check if the current member is the successor
		if ( successor == NULL ||
			 member->getGrade() < successor->getGrade() ||
			 ( member->getGrade() == successor->getGrade() && member->getEnterTime() < successor->getEnterTime() ) )
		{
			successor = member;
		}
	}
		
	// if the guild is still valid, set the successor as leader
	if ( successor )
	{
		guild->decGradeCount( successor->getGrade() );
		guild->incGradeCount( EGSPD::CGuildGrade::Leader );
		successor->setMemberGrade( EGSPD::CGuildGrade::Leader );

		// tell guild
		SM_STATIC_PARAMS_1( params,STRING_MANAGER::string_id );
		params[0].StringId = CEntityIdTranslator::getInstance()->getEntityNameStringId(successor->getIngameEId());

		sendMessageToGuildMembers("GUILD_NEW_LEADER",params );

		// If the player is online, the module must be recreated. Do as the reference was destroyed
		CGuildMemberModule * successorModule = NULL;
		if ( successor->getReferencingModule(successorModule) )
		{
			CGuildCharProxy successorProxy;
			successorModule->getProxy(successorProxy);
			successor->removeReferencingModule(successorModule);
			successorModule->onReferencedDestruction();

			IModule * moduleTarget = createModule(successorProxy,successor);
			guild->setMemberClientDB( successor );
			MODULE_AST(moduleTarget);
		}
	}
	_GuildMemberCore->setMemberGrade(EGSPD::CGuildGrade::Member);
	guild->incGradeCount( EGSPD::CGuildGrade::Member );
	guild->decGradeCount( EGSPD::CGuildGrade::Leader );
	// quit the guild 
	CGuildMemberModule::quitGuild();
}

//----------------------------------------------------------------------------
bool CGuildLeaderModule::canAffectGrade(EGSPD::CGuildGrade::TGuildGrade)const
{
	return true;
}

//----------------------------------------------------------------------------
bool CGuildLeaderModule::canInvite()const
{
	CGuild * guild = MODULE_CAST<CGuild*>( _GuildMemberCore->getGuild() );
	MODULE_AST( guild );
	return !(guild->isProxy());
}

