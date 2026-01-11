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

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include "stdpch.h"

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild_member_module.h"
#include "outpost_manager/outpost.h"
#include "outpost_manager/outpost_manager.h"


//----------------------------------------------------------------------------
// namespaces
//----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//----------------------------------------------------------------------------
// helpers
//----------------------------------------------------------------------------

#define GET_CHAR( _id_ ) \
	CCharacter * user = PlayerManager.getChar( _id_ ); \
	if (!user){ \
	OUTPOST_WRN("<OUTPOST>'%s' is not a valid char. Cant process callback",_id_.toString().c_str()); \
	return; }
	
#define GET_GUILD_MODULE( _id_ ) \
	GET_CHAR( _id_ ) \
	CGuildMemberModule * module = NULL; \
	if ( !user->getModuleParent().getModule( module ) ){ \
	OUTPOST_WRN("<OUTPOST>'%s' has no valid guild module Cant process callback",_id_.toString().c_str()); \
	return; }


//----------------------------------------------------------------------------
// client callbacks
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void cbClientOutpostSideChosen( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eid;
	msgin.serial(eid);
	
	bool neutral;
	uint8 side; 
	msgin.serial( neutral );
	msgin.serial( side );
	
	CCharacter * character = PlayerManager.getChar(eid);
	if( character )
	{
		character->outpostSideChosen( neutral, (OUTPOSTENUMS::TPVPSide)side );
	}
	else
	{
		OUTPOST_WRN("<cbClientOutpostSideChosen> unknown character %s",eid.toString().c_str());
	}
}

//----------------------------------------------------------------------------
void cbClientOutpostSelect(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostSelect);
	
	CEntityId eId;
	CSheetId outpostSheet;
	
	msgin.serial(eId, outpostSheet);

	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	CCharacter* character = PlayerManager.getChar(eId);
	if (outpost && character)
	{
		character->setSelectedOutpost(outpost->getAlias());
		outpost->addOutpostDBRecipient(eId);
	}
}

//----------------------------------------------------------------------------
void cbClientOutpostUnselect(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostUnselect);
	
	CEntityId eId;
	msgin.serial(eId);
	
	CCharacter* character = PlayerManager.getChar(eId);
	if (character)
	{
		TAIAlias outpostAlias = character->getSelectedOutpost();
		NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(outpostAlias);
		if (outpost)
			outpost->removeOutpostDBRecipient(eId);
	}
}

//----------------------------------------------------------------------------
void cbClientOutpostWarStart(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostWarStart);
	
	CEntityId eid;
	CSheetId outpostSheet;
	uint8 hour;
	msgin.serial(eid, outpostSheet, hour);

	uint32 attackTime = COutpost::s_computeEstimatedAttackTimeForClient(hour);

	COutpost::TChallengeOutpostErrors error = COutpost::UnknownError;
	CCharacter * user = PlayerManager.getChar(eid);
	if (user != NULL)
	{
		CGuildMemberModule * module;
		if ( user->getModuleParent().getModule( module ) )
		{
			error = module->challengeOutpost(outpostSheet, true);
		}
		else
		{
			OUTPOST_WRN( "GuildModule not found for %s", eid.toString().c_str() );
			error = COutpost::NoGuildModule;
		}
	}
	else
	{
		OUTPOST_WRN("Invalid char %s", eid.toString().c_str());
		error = COutpost::InvalidUser;
	}

	bool ok = (error == COutpost::NoError);
	uint32 textId;
	textId = STRING_MANAGER::sendStringToClient(TheDataset.getDataSetRow(eid), COutpost::getErrorString(error), TVectorParamCheck());
	PlayerManager.sendImpulseToClient(eid, std::string("OUTPOST:DECLARE_WAR_ACK"), ok, textId, attackTime );
}

//----------------------------------------------------------------------------
void cbClientOutpostWarValidate(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostWarValidate);
	
	CEntityId eid;
	CSheetId outpostSheet;
	uint8 hour;
	uint32 estimatedAttackTime;
	msgin.serial(eid, outpostSheet, hour, estimatedAttackTime);

	uint32 attackTime = COutpost::s_computeEstimatedAttackTimeForClient(hour);

	COutpost::TChallengeOutpostErrors error = COutpost::UnknownError;
	CCharacter * user = PlayerManager.getChar(eid);
	if (user != NULL)
	{
		CGuildMemberModule * module;
		if ( user->getModuleParent().getModule( module ) )
		{
			if (attackTime == estimatedAttackTime)
				error = module->challengeOutpost(outpostSheet, false);
			else
				error = COutpost::TimePeriodEstimationChanged;
		}
		else
		{
			OUTPOST_WRN( "GuildModule not found for %s", eid.toString().c_str() );
			error = COutpost::NoGuildModule;
		}
	}
	else
	{
		OUTPOST_WRN("Invalid char %s", eid.toString().c_str());
		error = COutpost::InvalidUser;
	}

	bool ok = (error == COutpost::NoError);
	if (ok)
	{
		COutpost* outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
		if (outpost)
			outpost->timeSetAttackHour(hour);
	}
	uint32 textId;
	textId = STRING_MANAGER::sendStringToClient(TheDataset.getDataSetRow(eid), COutpost::getErrorString(error), TVectorParamCheck());
	PlayerManager.sendImpulseToClient(eid, std::string("OUTPOST:DECLARE_WAR_ACK"), ok, textId, attackTime );
}

//----------------------------------------------------------------------------
void cbClientOutpostGiveup(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostGiveup);
	
	CEntityId eId;
	CSheetId outpostSheet;
	
	msgin.serial(eId, outpostSheet);
	
	GET_GUILD_MODULE(eId);
	
	module->giveupOutpost(outpostSheet);
}

//----------------------------------------------------------------------------
void cbClientOutpostBanishPlayer(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostBanishPlayer);

	CEntityId eId;
	msgin.serial( eId );
	
	CCharacter * actor = PlayerManager.getChar( eId );
	if( actor )
	{
		// actor must be in a guild
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( actor->getGuildId() );
		if( guild != NULL )
		{
			uint32 outpostAlias = actor->getOutpostAlias();
			if( outpostAlias != 0 )
			{
				CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias( outpostAlias );
				if( outpost )
				{
					// actor's guild must be part of the outpost conflict
					if( outpost->getAttackerGuild() == guild->getId() || outpost->getOwnerGuild() == guild->getId() )
					{
						CGuildMember * member = guild->getMemberFromEId( eId );
						if( member != NULL )
						{
							// actor must be at least officer
							if( member->getGrade() < EGSPD::CGuildGrade::Member )
							{
								CEntityId targetId = actor->getTarget();
								CCharacter * target = PlayerManager.getChar( targetId );
								if( target )
								{
									if( outpostAlias == target->getOutpostAlias() )
									{
										// actor and target must be outpost ally
										if( actor->getOutpostSide() == target->getOutpostSide() )
										{
											if(actor->getOutpostSide() == OUTPOSTENUMS::OutpostAttacker)
												outpost->banishPlayerForAttack( targetId );
											else
												outpost->banishPlayerForDefense( targetId );
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		OUTPOST_WRN("<cbClientOutpostBanishPlayer> cant find char with entity id %s",eId.toString().c_str());
	}
}

//----------------------------------------------------------------------------
void cbClientOutpostBanishGuild(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostBanishGuild);
	
	CEntityId eId;
	msgin.serial( eId );
	
	CCharacter * actor = PlayerManager.getChar( eId );
	if( actor )
	{
		uint32 outpostAlias = actor->getOutpostAlias();
		// actor must take part in the outpost conflict
		if( outpostAlias != 0 )
		{
			CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias( outpostAlias );
			if( outpost )
			{
				// actor must be in a guild
				CGuild * guild = CGuildManager::getInstance()->getGuildFromId( actor->getGuildId() );
				if( guild != NULL )
				{
					// actor's Guild must take part in the outpost conflict
					if( outpost->getAttackerGuild() == guild->getId() || outpost->getOwnerGuild() == guild->getId() )
					{
						CGuildMember * member = guild->getMemberFromEId( eId );
						if( member != NULL )
						{
							// actor must be at least officer
							if( member->getGrade() < EGSPD::CGuildGrade::Member )
							{
								CEntityId targetId = actor->getTarget();
								CCharacter * target = PlayerManager.getChar( targetId );
								if( target )
								{
									// target must not be in the same guild as actor
									if( actor->getGuildId() != target->getGuildId() )
									{	
										if( outpostAlias == target->getOutpostAlias() )
										{
											// actor and target must be outpost ally
											if( actor->getOutpostSide() == target->getOutpostSide() )
											{
												// target must have a guild
												CGuild * targetGuild = CGuildManager::getInstance()->getGuildFromId( target->getGuildId() );
												if( targetGuild != NULL )
												{
													if(target->getOutpostSide() == OUTPOSTENUMS::OutpostAttacker)
														outpost->banishGuildForAttack( target->getGuildId() );
													else
														outpost->banishGuildForDefense( target->getGuildId() );
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				OUTPOST_WRN("<cbClientOutpostBanishGuild> cant find outpost %d",outpostAlias);
			}
		}
	}
	else
	{
		OUTPOST_WRN("<cbClientOutpostBanishGuild> cant find char with entity id %s",eId.toString().c_str());
	}
}

//----------------------------------------------------------------------------
void cbClientOutpostSetSquad(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostSetSquad);

	CEntityId eId;
	CSheetId outpostSheet;
	uint8 squadSlot;
	uint8 shopSquadIndex;

	msgin.serial(eId, outpostSheet, squadSlot, shopSquadIndex);

	GET_GUILD_MODULE(eId);

	module->outpostSetSquad(outpostSheet, squadSlot, shopSquadIndex);
}

//----------------------------------------------------------------------------
void cbClientOutpostSetSquadSpawnZone(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostSetSquadSpawnZone);

	CEntityId eId;
	CSheetId outpostSheet;
	uint8 squadSlot;
	uint8 spawnZoneIndex;

	msgin.serial(eId, outpostSheet, squadSlot, spawnZoneIndex);

	GET_GUILD_MODULE(eId);

	module->outpostSetSquadSpawnZone(outpostSheet, squadSlot, spawnZoneIndex);
}

//----------------------------------------------------------------------------
void cbClientOutpostInsertSquad(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostInsertSquad);

	CEntityId eId;
	CSheetId outpostSheet;
	uint8 squadSlot;

	msgin.serial(eId, outpostSheet, squadSlot);

	GET_GUILD_MODULE(eId);

	module->outpostInsertSquad(outpostSheet, squadSlot);
}

//----------------------------------------------------------------------------
void cbClientOutpostRemoveSquad(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostRemoveSquad);

	CEntityId eId;
	CSheetId outpostSheet;
	uint8 squadSlot;

	msgin.serial(eId, outpostSheet, squadSlot);

	GET_GUILD_MODULE(eId);

	module->outpostRemoveSquad(outpostSheet, squadSlot);
}

//----------------------------------------------------------------------------
void cbClientOutpostSetExpenseLimit(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostSetExpenseLimit);

	CEntityId eId;
	CSheetId outpostSheet;
	uint32 squadCapital;

	msgin.serial(eId, outpostSheet, squadCapital);

	GET_GUILD_MODULE(eId);

	module->outpostSetExpenseLimit(outpostSheet, squadCapital);
}

//----------------------------------------------------------------------------
void cbClientOutpostSetDefensePeriod(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientOutpostSetExpenseLimit);
	
	CEntityId eId;
	CSheetId outpostSheet;
	uint8 hour;
	
	msgin.serial(eId, outpostSheet, hour);
	
	GET_GUILD_MODULE(eId);
	
	module->outpostSetDefensePeriod(outpostSheet, hour);
}

