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

#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "outpost_manager/outpost_manager.h"

#include "guild_member_module.h"
#include "guild_officer_module.h"
#include "guild_high_officer_module.h"
#include "guild_leader_module.h"
#include "guild_manager/guild_char_proxy.h"
#include "guild_manager/guild_manager.h"
#include "guild_invitation_module.h"
#include "guild_manager/guild_member.h"
#include "egs_utils.h"
#include "guild_manager/guild.h"
#include "building_manager/building_manager.h"
#include "building_manager/room_instance.h"
#include "building_manager/building_physical.h"
#include "guild_manager/fame_manager.h"
#include "zone_manager.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CGuildMember);
NL_INSTANCE_COUNTER_IMPL(CGuildMemberModule);

//----------------------------------------------------------------------------
CGuildMemberModule * CGuildMemberModule::createModule( CGuildCharProxy& proxy, CGuildMember* guildMember )
{
	nlassert( guildMember );
	switch( guildMember->getGrade() ) 
	{
	case EGSPD::CGuildGrade::Member:
		return new CGuildMemberModule(proxy,guildMember);
		break;
	case EGSPD::CGuildGrade::Officer:
		return new CGuildOfficerModule(proxy,guildMember);
		break;
	case EGSPD::CGuildGrade::HighOfficer:
		return new CGuildHighOfficerModule(proxy,guildMember);
		break;
	case EGSPD::CGuildGrade::Leader:
		return new CGuildLeaderModule(proxy,guildMember);
		break;
	}
	return NULL;
}

//----------------------------------------------------------------------------
CGuildMemberModule::CGuildMemberModule( CGuildCharProxy& proxy, CGuildMember* guildMember )
:IModule(&(proxy.getModuleParent()),guildMember),_GuildMemberCore(guildMember)
{
	nlassert( guildMember );
	MODULE_AST( guildMember->getGuild() );
	CGuild* guild = EGS_PD_CAST<CGuild*>( guildMember->getGuild() );
	EGS_PD_AST(guild);

	_LastGuildItemInfoVersionsSent.resize( INVENTORIES::NbGuildSlots, 0 );
}


bool CGuildMemberModule::isGuildProxy()
{
	CGuild * guild = EGS_PD_CAST<CGuild*>( _GuildMemberCore->getGuild() );
	EGS_PD_AST( guild );

	return guild->isProxy();
}

//----------------------------------------------------------------------------
void CGuildMemberModule::setLeader( uint16 index,uint8 session)
{
	CGuildCharProxy proxy;
	getProxy(proxy);
	proxy.cancelAFK();
	proxy.sendSystemMessage("GUILD_INSUFFICIENT_GRADE");
}

//----------------------------------------------------------------------------
void CGuildMemberModule::quitGuild()
{
	MODULE_AST( _GuildMemberCore );
	CGuild * guild = EGS_PD_CAST<CGuild*>( _GuildMemberCore->getGuild() );
	EGS_PD_AST( guild );
	SM_STATIC_PARAMS_1( params,STRING_MANAGER::player );
	params[0].setEIdAIAlias( _GuildMemberCore->getIngameEId(), CAIAliasTranslator::getInstance()->getAIAlias(_GuildMemberCore->getIngameEId()) );
	
	CFameManager::getInstance().clearPlayerGuild( _GuildMemberCore->getIngameEId() );

	CGuildCharProxy proxy;
	getProxy(proxy);
	proxy.cancelAFK();
	clearOnlineGuildProperties();
	guild->deleteMember( _GuildMemberCore );
	if ( guild->getMembersBegin() == guild->getMembersEnd() )
	{
		CGuildManager::getInstance()->deleteGuild(guild->getId());
		proxy.sendSystemMessage("GUILD_DESTROYED");
	}
	else
	{
		guild->sendMessageToGuildMembers("GUILD_QUIT", params);
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::string_id);
		params[0].StringId = guild->getNameId();
		proxy.sendSystemMessage("GUILD_YOU_QUIT", params);
	}
}

//----------------------------------------------------------------------------
void CGuildMemberModule::setGrade( uint16 index,uint8 session, EGSPD::CGuildGrade::TGuildGrade grade)const
{
	MODULE_AST( _GuildMemberCore );
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
	if ( _GuildMemberCore->getMemberIndex() == index )
	{
		nlwarning("<GUILD>%s tries to change its grade",proxy.getId().toString().c_str());
		return;
	}
	CGuildMember * member = guild->getMemberByIndex( index );
	if ( member == NULL )
	{
		nlwarning("<GUILD>%s set invalid member idx %u as leader",proxy.getId().toString().c_str(),index );
		return;
	}
	EGSPD::CGuildGrade::TGuildGrade oldGrade = member->getGrade();
	if ( !canAffectGrade( oldGrade ) )
	{
		proxy.sendSystemMessage("GUILD_INSUFFICIENT_GRADE");
		return;
	}

	if ( !CGuildManager::getInstance()->isGMGuild( guild->getId() ) && guild->getGradeCount(grade) >= guild->getMaxGradeCount(grade) )
	{
		SM_STATIC_PARAMS_1( paramFull, STRING_MANAGER::string_id );
		paramFull[0].StringId = CEntityIdTranslator::getInstance()->getEntityNameStringId(member->getIngameEId());
		proxy.sendSystemMessage("GUILD_GRADE_FULL",paramFull);
		return;
	}
	
	member->setMemberGrade(grade);
	guild->incGradeCount( grade );
	guild->decGradeCount( oldGrade );
	
	// send system message
	SM_STATIC_PARAMS_3(params,STRING_MANAGER::player,STRING_MANAGER::player,STRING_MANAGER::string_id);
	params[0].setEIdAIAlias( proxy.getId(), CAIAliasTranslator::getInstance()->getAIAlias(proxy.getId()) );
	params[1].setEIdAIAlias( member->getIngameEId(), CAIAliasTranslator::getInstance()->getAIAlias(member->getIngameEId()) );
	params[2].StringId = guild->getNameId();
	
	// If the player is online, the module must be recreated. Do as the reference was destroyed
	CGuildMemberModule * module = NULL;
	if ( member->getReferencingModule(module) )
	{
		CGuildCharProxy targetProxy;
		module->getProxy(targetProxy);
		member->removeReferencingModule(module);
		module->onReferencedDestruction();
		IModule * moduleTarget = createModule(targetProxy,member);
		guild->setMemberClientDB( member );
		MODULE_AST(moduleTarget);
	}
}

//----------------------------------------------------------------------------
void CGuildMemberModule::sendMessageToGuildMembers( const std::string &  msg, const TVectorParamCheck & params )const
{
	MODULE_AST( _GuildMemberCore );
	CGuild * guild = MODULE_CAST<CGuild*>( _GuildMemberCore->getGuild() );
	MODULE_AST( guild );
	guild->sendMessageToGuildMembers( msg,params );
}

//----------------------------------------------------------------------------
void CGuildMemberModule::inviteCharacterInGuild(CCharacter * invitedCharacter)const
{
	MODULE_AST( _GuildMemberCore );
	MODULE_AST( _GuildMemberCore->getGuild() );

	CGuildCharProxy invitor;
	getProxy( invitor );
	invitor.cancelAFK();

	if(invitedCharacter == 0 || invitedCharacter->getEnterFlag() == false )
	{
		CCharacter::sendDynamicSystemMessage( invitor.getRowId(), "GUILD_INVITED_CHARACTER_MUST_BE_ONLINE" );
		return;
	}

	CGuildCharProxy invited(invitedCharacter);
	_inviteCharacterInGuild(invitor, invited);
}

//----------------------------------------------------------------------------
void CGuildMemberModule::inviteTargetInGuild()const
{
	MODULE_AST( _GuildMemberCore );
	MODULE_AST( _GuildMemberCore->getGuild() );

	CGuildCharProxy invitor;
	getProxy( invitor );
	invitor.cancelAFK();
	CGuildCharProxy target;
	if ( !invitor.getTarget(target) )
	{
		nlwarning("<GUILD>%s has no target to invite",invitor.getId().toString().c_str() );
		return;
	}
	_inviteCharacterInGuild(invitor, target);
}

//----------------------------------------------------------------------------
void CGuildMemberModule::_inviteCharacterInGuild(CGuildCharProxy& invitor, CGuildCharProxy& target)const
{
	CGuild * guild = EGS_PD_CAST<CGuild*>( _GuildMemberCore->getGuild() );
	EGS_PD_AST( guild );

	SM_STATIC_PARAMS_1( params1, STRING_MANAGER::player );
	params1[0].setEIdAIAlias( target.getId(), CAIAliasTranslator::getInstance()->getAIAlias( target.getId()) );

	/// check invitor grade
	if ( !canInvite() )
	{
		CCharacter::sendDynamicSystemMessage( invitor.getRowId(), "GUILD_INSUFFICIENT_GRADE",params1 );
		return;
	}

	/// target must not have a guild
	CGuildMemberModule * guildModule;
	if ( target.getModule( guildModule ) )
	{
		invitor.sendSystemMessage("GUILD_ALREADY_MEMBER",params1);
		return;
	}

	/// target must not be invited
	CGuildInvitationModule * inviteModule;
	if ( target.getModule(inviteModule) )
	{
		CCharacter::sendDynamicSystemMessage( invitor.getRowId(), "GUILD_ALREADY_HAS_JOIN_PROPOSAL",params1 );
		return;		
	}

	/// the invitor must not be in the ignore list of the target
	{
		CCharacter * invitedChar = PlayerManager.getChar(target.getId());
		if( invitedChar == 0 ) return;
		if(invitedChar->hasInIgnoreList(invitor.getId()))
		{
			// Use the standard "player declines your offer". Don't use specific message because
			// maybe not a good idea to inform a player that someone ignores him
			CCharacter::sendDynamicSystemMessage( invitor.getRowId(), "GUILD_REFUSE_JOIN",params1 );
			return;
		}
	}

	/// target must not be an outpost pvp enemy of the invitor's guild
	CCharacter * invitorPlayer = PlayerManager.getChar( invitor.getEntityRowId() );
	if( invitorPlayer )
	{
		CEntityBase * targetEntity = CEntityBaseManager::getEntityBasePtr( target.getEntityRowId() );
		if(targetEntity)
		{
			if( targetEntity->getOutpostAlias() !=0 )
			{
				CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias( targetEntity->getOutpostAlias() );
				if( outpost )
				{
					if( (outpost->getOwnerGuild()==invitorPlayer->getGuildId() &&  targetEntity->getOutpostSide()==OUTPOSTENUMS::OutpostAttacker) ||
						(outpost->getAttackerGuild()==invitorPlayer->getGuildId() &&  targetEntity->getOutpostSide()==OUTPOSTENUMS::OutpostOwner) )
					{
						CCharacter::sendDynamicSystemMessage( invitor.getRowId(), "GUILD_CANT_INVITE_OUTPOST_ENEMY" );
						return;
					}
				}
				else
				{
					nlwarning("<CGuildMemberModule::inviteTargetInGuild> can't get outpost %d of target",targetEntity->getOutpostAlias() );
				}
			}
		}
		else
		{
			nlwarning("<CGuildMemberModule::inviteTargetInGuild> can't get target %s",target.getId().toString().c_str() );
		}
	}
	else
	{
		nlwarning("<CGuildMemberModule::inviteTargetInGuild> can't get char from invitor %s",invitor.getId().toString().c_str() );
	}

	/// check if there is room in the guild
	if (  guild->getMemberCount() >= GuildMaxMemberCount )
	{
		SM_STATIC_PARAMS_1(params,STRING_MANAGER::integer);
		params[0].Int = GuildMaxMemberCount;
		CCharacter::sendDynamicSystemMessage( invitor.getRowId(), "GUILD_MAX_MEMBER_COUNT", params);
		return;		
	}

	/// check guild and invited member allegiances compatibilities
	CGuild::TAllegiances guildAllegiance, invitedAllegiance;
	guildAllegiance = guild->getAllegiance();
	CCharacter * invitedChar = PlayerManager.getChar(target.getId());
	if( invitedChar == 0 ) return;
	invitedAllegiance = invitedChar->getAllegiance();
	if( invitedAllegiance.first != guildAllegiance.first && invitedAllegiance.first != PVP_CLAN::Neutral )
	{
		SM_STATIC_PARAMS_2( params, STRING_MANAGER::player, STRING_MANAGER::faction );
		params[0].setEIdAIAlias( target.getId(), CAIAliasTranslator::getInstance()->getAIAlias( target.getId()) );
		params[1].Enum = PVP_CLAN::getFactionIndex(invitedAllegiance.first);
		invitor.sendSystemMessage("GUILD_ICOMPATIBLE_ALLEGIANCE",params);
		return;
	}
	if( invitedAllegiance.second != guildAllegiance.second && invitedAllegiance.second != PVP_CLAN::Neutral )
	{
		SM_STATIC_PARAMS_2( params, STRING_MANAGER::player, STRING_MANAGER::faction );
		params[0].setEIdAIAlias( target.getId(), CAIAliasTranslator::getInstance()->getAIAlias( target.getId()) );
		params[1].Enum = PVP_CLAN::getFactionIndex(invitedAllegiance.second);
		invitor.sendSystemMessage("GUILD_ICOMPATIBLE_ALLEGIANCE",params);
		return;
	}

	// build a new invitation
	MODULE_AST(guild);
	CGuildInvitation *invitation = new CGuildInvitation( guild, invitor.getRowId() );
	CGuildManager::getInstance()->addInvitation( invitation );

	// build a module on it
	inviteModule = new CGuildInvitationModule(target,invitation);

	// tell client
	SM_STATIC_PARAMS_2(params2,STRING_MANAGER::player,STRING_MANAGER::string_id);
	params2[0].setEIdAIAlias( invitor.getId(), CAIAliasTranslator::getInstance()->getAIAlias( invitor.getId()) );
	params2[1].StringId = guild->getNameId();
	uint32 txt = STRING_MANAGER::sendStringToClient( target.getRowId(),"GUILD_JOIN_PROPOSAL",params2 );
	PlayerManager.sendImpulseToClient(target.getId(), "GUILD:JOIN_PROPOSAL", txt );
}

//----------------------------------------------------------------------------
void CGuildMemberModule::onParentDestructionHandler()
{
	MODULE_AST( _GuildMemberCore );
	CGuild* guild = EGS_PD_CAST<CGuild*>(_GuildMemberCore->getGuild());
	EGS_PD_AST( guild );
	_GuildMemberCore->removeReferencingModule(this);
	// once the guild member module is removed the member can be set offline
	guild->setMemberOffline( _GuildMemberCore );
}

//----------------------------------------------------------------------------
bool CGuildMemberModule::canTakeGuildItem()const
{
	return false;
}

//----------------------------------------------------------------------------
bool CGuildMemberModule::canInvite()const
{
	return false;
}

//----------------------------------------------------------------------------
void CGuildMemberModule::buyGuildOption( const CStaticItem * form )
{
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
}

//----------------------------------------------------------------------------
bool CGuildMemberModule::isOutpostAdmin() const
{
	return false;
}

//----------------------------------------------------------------------------
COutpost::TChallengeOutpostErrors CGuildMemberModule::challengeOutpost(NLMISC::CSheetId outpostSheet, bool simulate)
{
	if (!simulate)
	{
		CGuildCharProxy proxy;
		getProxy( proxy );
		proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
	}
	return COutpost::BadGuildGrade;
}

//----------------------------------------------------------------------------
void CGuildMemberModule::giveupOutpost(NLMISC::CSheetId outpostSheet)
{
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
}

//----------------------------------------------------------------------------
void CGuildMemberModule::outpostSetSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot, uint32 shopSquadIndex)
{
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
}

//----------------------------------------------------------------------------
void CGuildMemberModule::outpostSetSquadSpawnZone(NLMISC::CSheetId outpostSheet, uint32 squadSlot, uint32 spawnZoneIndex)
{
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
}

//----------------------------------------------------------------------------
void CGuildMemberModule::outpostInsertSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot)
{
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
}

//----------------------------------------------------------------------------
void CGuildMemberModule::outpostRemoveSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot)
{
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
}

//----------------------------------------------------------------------------
void CGuildMemberModule::outpostSetExpenseLimit(NLMISC::CSheetId outpostSheet, uint32 expenseLimit)
{
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
}

//----------------------------------------------------------------------------
void CGuildMemberModule::outpostSetDefensePeriod(NLMISC::CSheetId outpostSheet, uint8 hour)
{
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
}

//----------------------------------------------------------------------------
bool CGuildMemberModule::canBuyOutpostBuilding() const
{
	return false;
}

//----------------------------------------------------------------------------
void CGuildMemberModule::buyOutpostBuilding(NLMISC::CSheetId sid)
{
	CGuildCharProxy proxy;
	getProxy( proxy );
	proxy.sendSystemMessage( "GUILD_BUILDING_BAD_GRADE" );
}

//----------------------------------------------------------------------------
bool CGuildMemberModule::canAffectGrade(EGSPD::CGuildGrade::TGuildGrade)const
{
	return false;
}

//----------------------------------------------------------------------------
CMissionGuild * CGuildMemberModule::pickMission( TAIAlias alias )
{
	/// todo guild mission
	return NULL;
}

//----------------------------------------------------------------------------
void CGuildMemberModule::clearOnlineGuildProperties()
{
	CGuildCharProxy targetProxy;
	getProxy(targetProxy);
	targetProxy.setGuildId(0);
	targetProxy.updateTargetingChars();
	
	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, targetProxy.getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;		
	if ( CBuildingManager::getInstance()->isRoomCell(cell) )
	{	
		CRoomInstanceGuild * room = dynamic_cast<CRoomInstanceGuild *>( CBuildingManager::getInstance()->getRoomInstanceFromCell( cell ) );
		if ( room && room->getGuildId() && room->getBuilding() )
		{
			const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone( room->getBuilding()->getDefaultExitSpawn() );
			if ( zone )
			{
				sint32 x,y,z;
				float heading;
				zone->getRandomPoint(x,y,z,heading);
				targetProxy.tpWanted(x,y,z,true,heading);
			}
		}
	}
}

//----------------------------------------------------------------------------
void CGuildMemberModule::kickMember( uint16 index,uint8 session )const
{
	MODULE_AST( _GuildMemberCore );
	CGuild * guild = EGS_PD_CAST<CGuild*>( _GuildMemberCore->getGuild() );
	EGS_PD_AST( guild );
	CGuildCharProxy proxy;
	getProxy(proxy);
	proxy.cancelAFK();
	
	if ( guild->getMembersSession() != session )
	{
		proxy.sendSystemMessage( "GUILD_BAD_SESSION" );
		return;
	}
	if ( _GuildMemberCore->getMemberIndex() == index )
	{
		nlwarning("<GUILD>%s tries to kick himself",proxy.getId().toString().c_str());
		return;
	}
	
	CGuildMember * member = guild->getMemberByIndex( index );
	if ( member == NULL )
	{
		nlwarning("<GUILD>%s set invalid member idx %u as leader",proxy.getId().toString().c_str(),index );
		return;
	}
	EGSPD::CGuildGrade::TGuildGrade targetGrade = member->getGrade();
	if ( !canAffectGrade( targetGrade ) )
	{
		proxy.sendSystemMessage("GUILD_INSUFFICIENT_GRADE");
		return;
	}
	// if the user is online reset its guild id
	CGuildMemberModule * module = NULL;
	if ( member->getReferencingModule( module ) )
	{
		module->clearOnlineGuildProperties();
	}
	// send system message
	SM_STATIC_PARAMS_2(params,STRING_MANAGER::player,STRING_MANAGER::string_id);
	params[0].setEIdAIAlias( proxy.getId(), CAIAliasTranslator::getInstance()->getAIAlias(proxy.getId()) );
	params[1].StringId = CEntityIdTranslator::getInstance()->getEntityNameStringId(member->getIngameEId());
	sendMessageToGuildMembers("GUILD_KICK_MEMBER",params);
	
	guild->deleteMember( member );	
}

//----------------------------------------------------------------------------
void CGuildMemberModule::setMOTD(const std::string& motd)
{
	MODULE_AST( _GuildMemberCore );
	CGuild * guild = EGS_PD_CAST<CGuild*>( _GuildMemberCore->getGuild() );
	EGS_PD_AST( guild );
	CGuildCharProxy proxy;
	getProxy(proxy);
	guild->setMOTD(motd, proxy.getId());
}
