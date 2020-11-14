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

#include "guild_invitation_module.h"
#include "guild_manager.h"
#include "guild_member_module.h"
#include "guild_member.h"
#include "guild.h"
#include "egs_utils.h"

NL_INSTANCE_COUNTER_IMPL(CGuildInvitation);
NL_INSTANCE_COUNTER_IMPL(CGuildInvitationModule);

//----------------------------------------------------------------------------
void CGuildInvitationModule::refuse()
{
	MODULE_AST(_Invitation);
	nlassert( _Invitation->getGuild() );
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.cancelAFK();

	SM_STATIC_PARAMS_1(params,STRING_MANAGER::player);
	params[0].setEIdAIAlias( proxy.getId(), CAIAliasTranslator::getInstance()->getAIAlias( proxy.getId()) );

	CCharacter::sendDynamicSystemMessage( _Invitation->getInvitor(), "GUILD_REFUSE_JOIN", params);
	CGuildManager::getInstance()->removeInvitation(_Invitation);
}

//----------------------------------------------------------------------------
void CGuildInvitationModule::accept()
{
	MODULE_AST(_Invitation);
	nlassert( _Invitation->getGuild() );
	CGuild * guild = EGS_PD_CAST< CGuild* > ( _Invitation->getGuild() );
	EGS_PD_AST(guild);

	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.cancelAFK();

	CGuildMemberModule * oldModule;
	proxy.getModule( oldModule );
	if ( oldModule )
	{
		nlwarning("<GUILD> user %s is already member of a guild",proxy.getId().toString().c_str() );
		CGuildManager::getInstance()->removeInvitation(_Invitation);
		return;
	}

	/// check if there is room in the guild
	if (  guild->getMemberCount() >= GuildMaxMemberCount )
	{
		nlwarning("<GUILD> guild max number member reached");
		SM_STATIC_PARAMS_1(params,STRING_MANAGER::integer);
		params[0].Int = GuildMaxMemberCount;
		proxy.sendSystemMessage("GUILD_MAX_MEMBER_COUNT_INVITE",params);
		CGuildManager::getInstance()->removeInvitation(_Invitation);
		return;
	}
	// send message to the guild
	SM_STATIC_PARAMS_1(params2,STRING_MANAGER::player);
	params2[0].setEIdAIAlias( proxy.getId(), CAIAliasTranslator::getInstance()->getAIAlias( proxy.getId()) );
	_Invitation->getGuild()->sendMessageToGuildMembers("GUILD_JOIN", params2);

	proxy.setGuildId(guild->getId());

	// inform user
	SM_STATIC_PARAMS_1(params1,STRING_MANAGER::string_id);
	params1[0].StringId = _Invitation->getGuild()->getNameId();
	proxy.sendSystemMessage("GUILD_YOU_JOIN",params1);

	// ask the client to open it's guild interface
	PlayerManager.sendImpulseToClient( proxy.getId(),"GUILD:OPEN_GUILD_WINDOW" );

	// create a new member core
	CGuildMember * memberCore = _Invitation->getGuild()->newMember( proxy.getId() );
	nlassert( memberCore );

// All the following already done by newMember()
	// retrieve the member module for the new member
//	CGuildMemberModule *module = NULL;
//	memberCore->getReferencingModule(module);
//	nlassert(memberCore != NULL);
//	memberCore = new CGuildMemberModule( proxy, memberCore );
//	memberCore->setMemberGrade( EGSPD::CGuildGrade::Member );
//	guild->setMemberOnline( memberCore, proxy.getId().getDynamicId() );

	//remove invitation
	CGuildManager::getInstance()->removeInvitation(_Invitation);
}

//----------------------------------------------------------------------------
void CGuildInvitationModule::onParentDestructionHandler()
{
	MODULE_AST(_Invitation);
	CGuildManager::getInstance()->removeInvitation(_Invitation);
}
