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

#include "game_share/fame.h"
#include "server_share/log_outpost_gen.h"

#include "guild_manager/guild_member.h"
#include "building_manager/building_physical.h"
#include "creature_manager/creature.h"
#include "player_manager/character.h"
#include "outpost_manager/outpost_manager.h"
#include "primitives_parser.h"

#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_outpost.h"

#include "guild_high_officer_module.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CGuildHighOfficerModule);

//----------------------------------------------------------------------------
bool CGuildHighOfficerModule::canAffectGrade(EGSPD::CGuildGrade::TGuildGrade grade)const
{
	return ( grade == EGSPD::CGuildGrade::Member || grade == EGSPD::CGuildGrade::Officer );
}

//----------------------------------------------------------------------------
bool CGuildHighOfficerModule::canTakeGuildItem()const
{
	return true;
}

//----------------------------------------------------------------------------
bool CGuildHighOfficerModule::canInvite()const
{
	CGuild * guild = MODULE_CAST<CGuild*>( _GuildMemberCore->getGuild() );
	MODULE_AST( guild );
	return !(guild->isProxy());
}

//----------------------------------------------------------------------------
void CGuildHighOfficerModule::buyGuildOption( const CStaticItem * form )
{
	nlassert(form);

	CGuildCharProxy proxy;
	getProxy( proxy );
	CGuild * guild = MODULE_CAST<CGuild*>( _GuildMemberCore->getGuild() );
	MODULE_AST( guild );

	if ( proxy.getMoney() < form->GuildOption->MoneyCost )
		return;
	
//	if ( guild->getXP() < form->GuildOption->XpCost )
//		return;
		
	if ( form->GuildOption->Type == GUILD_OPTION::GuildMainBuilding )
	{
		CCreature * bot = proxy.getInterlocutor();
		if ( !bot )
		{
			nlwarning("<BUILDING> char %s bot %s is invalid",proxy.getId().toString().c_str(), proxy.getInterlocutor()->getId().toString().c_str() );
			return;
		}	

		if ( bot->getGuildBuilding() == NULL )
		{
			nlwarning("<BUILDING> char %s bot %s has no building",proxy.getId().toString().c_str(), proxy.getInterlocutor()->getId().toString().c_str() );
			return;
		}	
			
		sint32 fame = CFameInterface::getInstance().getFameIndexed( guild->getEId(), bot->getForm()->getFaction());
		if ( fame < MinFameToBuyGuildBuilding )
		{
			SM_STATIC_PARAMS_2(params,STRING_MANAGER::integer,STRING_MANAGER::race);
			params[0].Int = MinFameToBuyGuildBuilding;
			params[1].Enum = bot->getRace();
			proxy.sendSystemMessage( "GUILD_BUILDING_BAD_FAME", params);
			return;
		}

		guild->setBuilding( bot->getGuildBuilding()->getAlias() );
	}
//	else if ( guild->getBuilding() != 0 )
//	{	
//		EGSPD::CSPType::TSPType type = GUILD_OPTION::toSPType( form->GuildOption->Type);
//		if ( type == EGSPD::CSPType::EndSPType )
//		{
//			nlwarning("<BUILDING> char %s, sheet %s invalid sp type",proxy.getId().toString().c_str(),form->SheetId.toString().c_str() );
//			return;
//		}
//		
//		if ( guild->hasRoleMaster( type ) )
//		{
//			proxy.sendSystemMessage( "GUILD_RM_ALREADY_BOUGHT" );
//			return;
//		}
//		guild->addRoleMaster( type );
//	}
//	guild->spendXP( form->GuildOption->XpCost );
	proxy.spendMoney( form->GuildOption->MoneyCost );
}

//----------------------------------------------------------------------------
bool CGuildHighOfficerModule::canBuyOutpostBuilding() const
{
	return true;
}

//----------------------------------------------------------------------------
void CGuildHighOfficerModule::buyOutpostBuilding(NLMISC::CSheetId sid)
{
	const CStaticOutpostBuilding *pSOB = CSheets::getOutpostBuildingForm(sid);
	nlassert(pSOB);

	CGuildCharProxy proxy;
	getProxy(proxy);
	CGuild *pGuild = MODULE_CAST<CGuild*>(_GuildMemberCore->getGuild());	
	MODULE_AST(pGuild);

	// This is an outpost building trade here
	CCreature *pBot = proxy.getInterlocutor();
	if (pBot == NULL)
	{
		nlwarning("<BUILDING> char %s no interlocutor", proxy.getId().toString().c_str());
		return;
	}
	COutpostBuilding *pOB = pBot->getOutpostBuilding();
	if (pOB == NULL)
	{
		nlwarning("<BUILDING> char %s : the interlocutor is not an outpost building", proxy.getId().toString().c_str());
		return;
	}
	if (pOB->isConstructing())
	{
		nlwarning("<BUILDING> char %s : the outpost building is already constructing", proxy.getId().toString().c_str());
		return;
	}
	// Verify the outpost holding the building belongs to the same guild as the player
	const COutpost *pO = pOB->getParent();
	if (pO == NULL)
	{
		nlwarning("<BUILDING> char %s : the interlocutor is an outpost building but not attached to an outpost", proxy.getId().toString().c_str());
		return;
	}
	if (!pO->isBelongingToAGuild())
	{
		nlwarning("<BUILDING> char %s : the outpost does not belongs to a guild", proxy.getId().toString().c_str());
		return;
	}
	if (pO->getOwnerGuild() != pGuild->getId())
	{
		nlwarning("<BUILDING> char %s : the outpost does not belongs to the player guild", proxy.getId().toString().c_str());
		return;
	}
	if (!pO->canConstructBuilding(sid, pOB))
	{
		nlwarning("<BUILDING> char %s : cannot construct building", proxy.getId().toString().c_str());
		return;
	}
	// Check the player rights
	if (!canBuyOutpostBuilding())
	{
		nlwarning("<BUILDING> char %s : no rights to build outpost buildings", proxy.getId().toString().c_str());
		return;
	}
	// Spend guild money
	if (pGuild->getMoney() < (uint64)pSOB->CostDapper)
	{
		nlwarning("<BUILDING> char %s : the guild has not enought money to build this", proxy.getId().toString().c_str());
		return;
	}
	pGuild->spendMoney(pSOB->CostDapper);
	// Construct the building
	pOB->construct(sid);
	log_Outpost_BuyOption(pO->getName(), pGuild->getName().toUtf8(), sid);
	proxy.endBotChat();
}

//----------------------------------------------------------------------------
bool CGuildHighOfficerModule::isOutpostAdmin() const
{
	return true;
}

extern CVariable<bool> UseProxyMoneyForOutpostCosts;

//----------------------------------------------------------------------------
COutpost::TChallengeOutpostErrors CGuildHighOfficerModule::challengeOutpost(NLMISC::CSheetId outpostSheet, bool simulate)
{
	CGuildCharProxy proxy;
	getProxy(proxy);
	CGuild * guild = MODULE_CAST<CGuild*>(_GuildMemberCore->getGuild());
	MODULE_AST(guild);

	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	if (outpost == NULL)
		return COutpost::InvalidOutpost;

	uint64 challengerMoney = guild->getMoney();
	if (UseProxyMoneyForOutpostCosts.get())
		challengerMoney += proxy.getMoney();

	if (challengerMoney < outpost->getChallengeCost())
	{
		if (!simulate)
			proxy.sendSystemMessage( "NEED_MORE_GUILD_MONEY" );
		return COutpost::NotEnoughMoney;
	}
	
	if (outpost->getAttackerGuild()!=0)
	{
		if (!simulate)
			proxy.sendSystemMessage( "GUILD_CHALLENGE_OUTPOST_ALREADY_ATTACKED" );
		// :TODO: Test whether attacking guild is us or not and return different errors.
		return COutpost::AlreadyAttacked;
	}
	if (outpost->getOwnerGuild()==guild->getId())
	{
		if (!simulate)
			nlwarning("character %s try to challenge outpost %s but already owns it",
				proxy.getId().toString().c_str(),
				CPrimitivesParser::aliasToString(outpost->getAlias()).c_str()
			);
		return COutpost::AlreadyOwned;
	}
	
	COutpost::TChallengeOutpostErrors ret = outpost->challengeOutpost(guild, simulate);
	
	if (!simulate && ret == COutpost::NoError)
	{
		uint32 remainingAmountToPay = outpost->getChallengeCost();
		uint32 amountPaidByGuild = remainingAmountToPay;
		if (guild->getMoney() < amountPaidByGuild)
			amountPaidByGuild = uint32( guild->getMoney() );

		remainingAmountToPay -= amountPaidByGuild;
		if (remainingAmountToPay > 0)
		{
			nlassert(UseProxyMoneyForOutpostCosts.get());
			uint32 amountPaidByPlayer = remainingAmountToPay;
			remainingAmountToPay -= amountPaidByPlayer;

			nlassert(amountPaidByPlayer <= proxy.getMoney());
			if (amountPaidByPlayer > 0)
				proxy.spendMoney(amountPaidByPlayer);
		}

		if (amountPaidByGuild > 0)
			guild->spendMoney(amountPaidByGuild);

		nlassert(remainingAmountToPay == 0);
	}
	
	return ret;
}

//----------------------------------------------------------------------------
void CGuildHighOfficerModule::giveupOutpost(NLMISC::CSheetId outpostSheet)
{
	CGuildCharProxy proxy;
	getProxy(proxy);
	CGuild* guild = MODULE_CAST<CGuild*>(_GuildMemberCore->getGuild());
	MODULE_AST(guild);

	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	if (outpost == NULL)
		return;

	// give up the outpost
	if (guild->getId()==outpost->getOwnerGuild())
	{
		if (!guild->canGiveUpOutpost())
			return;
		outpost->giveupOwnership();
	}
	else if (guild->getId()==outpost->getAttackerGuild())
	{
		// it is forbidden to give up a challenged outpost (against exploit)
		//outpost->giveupAttack();
		return;
	}
	else
	{
		nlwarning("Character %s try to giveup outpost %s but does not own it", proxy.getId().toString().c_str(), CPrimitivesParser::aliasToString(outpost->getAlias()).c_str());
		proxy.sendSystemMessage( "GUILD_GIVEUP_OUTPOST_ATTACK_NOT_ATTACKED" );
	}
}

//----------------------------------------------------------------------------
void CGuildHighOfficerModule::outpostSetSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot, uint32 shopSquadIndex)
{
	CGuildCharProxy proxy;
	getProxy(proxy);
	
	CGuild * guild = MODULE_CAST<CGuild*>(_GuildMemberCore->getGuild());
	MODULE_AST(guild);
	
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	if (outpost == NULL)
		return;

	OUTPOSTENUMS::TPVPSide side;
	if (proxy.getGuildId() == outpost->getOwnerGuild())
	{
		side = OUTPOSTENUMS::OutpostOwner;
	}
	else if (proxy.getGuildId() == outpost->getAttackerGuild())
	{
		side = OUTPOSTENUMS::OutpostAttacker;
	}
	else
	{
		nlwarning("Player %s is not allowed to edit the outpost '%s'",
			proxy.getId().toString().c_str(),
			outpost->getSheet().toString().c_str()
			);
		return;
	}

	// check editing concurrency
	if (!outpost->submitEditingAccess(side, proxy.getId(), COutpost::EditSquads))
		return;

	bool res = outpost->setSquad(side, squadSlot, shopSquadIndex);
	if (!res)
	{
		nlwarning("cannot set squad: player %s, squadSlot=%u, shopSquadIndex=%u",
			proxy.getId().toString().c_str(),
			squadSlot,
			shopSquadIndex
			);
	}
}

//----------------------------------------------------------------------------
void CGuildHighOfficerModule::outpostSetSquadSpawnZone(NLMISC::CSheetId outpostSheet, uint32 squadSlot, uint32 spawnZoneIndex)
{
	CGuildCharProxy proxy;
	getProxy(proxy);
	
	CGuild * guild = MODULE_CAST<CGuild*>(_GuildMemberCore->getGuild());
	MODULE_AST(guild);
	
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	if (outpost == NULL)
		return;

	OUTPOSTENUMS::TPVPSide side;
	if (proxy.getGuildId() == outpost->getOwnerGuild())
	{
		side = OUTPOSTENUMS::OutpostOwner;
	}
	else if (proxy.getGuildId() == outpost->getAttackerGuild())
	{
		side = OUTPOSTENUMS::OutpostAttacker;
	}
	else
	{
		nlwarning("Player %s is not allowed to edit the outpost '%s'",
			proxy.getId().toString().c_str(),
			outpost->getSheet().toString().c_str()
			);
		return;
	}

	// check editing concurrency
	if (!outpost->submitEditingAccess(side, proxy.getId(), COutpost::EditSquads))
		return;

	bool res = outpost->setSquadSpawnZone(side, squadSlot, spawnZoneIndex);
	if (!res)
	{
		nlwarning("cannot set squad spawn zone: player %s, squadSlot=%u, spawnZoneIndex=%u",
			proxy.getId().toString().c_str(),
			squadSlot,
			spawnZoneIndex
			);
	}
}

//----------------------------------------------------------------------------
void CGuildHighOfficerModule::outpostInsertSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot)
{
	CGuildCharProxy proxy;
	getProxy(proxy);
	
	CGuild * guild = MODULE_CAST<CGuild*>(_GuildMemberCore->getGuild());
	MODULE_AST(guild);
	
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	if (outpost == NULL)
		return;

	OUTPOSTENUMS::TPVPSide side;
	if (proxy.getGuildId() == outpost->getOwnerGuild())
	{
		side = OUTPOSTENUMS::OutpostOwner;
	}
	else if (proxy.getGuildId() == outpost->getAttackerGuild())
	{
		side = OUTPOSTENUMS::OutpostAttacker;
	}
	else
	{
		nlwarning("Player %s is not allowed to edit the outpost '%s'",
			proxy.getId().toString().c_str(),
			outpost->getSheet().toString().c_str()
			);
		return;
	}

	// check editing concurrency
	if (!outpost->submitEditingAccess(side, proxy.getId(), COutpost::EditSquads))
		return;

	bool res = outpost->insertDefaultSquad(side, squadSlot);
	if (!res)
	{
		nlwarning("cannot insert default squad: player %s, squadSlot=%u",
			proxy.getId().toString().c_str(),
			squadSlot
			);
	}
}

//----------------------------------------------------------------------------
void CGuildHighOfficerModule::outpostRemoveSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot)
{
	CGuildCharProxy proxy;
	getProxy(proxy);
	
	CGuild * guild = MODULE_CAST<CGuild*>(_GuildMemberCore->getGuild());
	MODULE_AST(guild);
	
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	if (outpost == NULL)
		return;

	OUTPOSTENUMS::TPVPSide side;
	if (proxy.getGuildId() == outpost->getOwnerGuild())
	{
		side = OUTPOSTENUMS::OutpostOwner;
	}
	else if (proxy.getGuildId() == outpost->getAttackerGuild())
	{
		side = OUTPOSTENUMS::OutpostAttacker;
	}
	else
	{
		nlwarning("Player %s is not allowed to edit the outpost '%s'",
			proxy.getId().toString().c_str(),
			outpost->getSheet().toString().c_str()
			);
		return;
	}

	// check editing concurrency
	if (!outpost->submitEditingAccess(side, proxy.getId(), COutpost::EditSquads))
		return;

	bool res = outpost->removeSquad(side, squadSlot);
	if (!res)
	{
		nlwarning("cannot remove squad: player %s, squadSlot=%u",
			proxy.getId().toString().c_str(),
			squadSlot
			);
	}
}

//----------------------------------------------------------------------------
void CGuildHighOfficerModule::outpostSetExpenseLimit(NLMISC::CSheetId outpostSheet, uint32 expenseLimit)
{
	CGuildCharProxy proxy;
	getProxy(proxy);
	
	CGuild * guild = MODULE_CAST<CGuild*>(_GuildMemberCore->getGuild());
	MODULE_AST(guild);
	
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	if (outpost == NULL)
		return;

	OUTPOSTENUMS::TPVPSide side;
	if (proxy.getGuildId() == outpost->getOwnerGuild())
	{
		side = OUTPOSTENUMS::OutpostOwner;
	}
	else if (proxy.getGuildId() == outpost->getAttackerGuild())
	{
		side = OUTPOSTENUMS::OutpostAttacker;
	}
	else
	{
		nlwarning("Player %s is not allowed to edit the outpost '%s'",
			proxy.getId().toString().c_str(),
			outpost->getSheet().toString().c_str()
			);
		return;
	}

	// check editing concurrency
	if (!outpost->submitEditingAccess(side, proxy.getId(), COutpost::EditExpenseLimit))
		return;

	if (side == OUTPOSTENUMS::OutpostOwner)
	{
		outpost->setOwnerExpenseLimit(expenseLimit);
	}
	else
	{
		outpost->setAttackerExpenseLimit(expenseLimit);
	}
}

//----------------------------------------------------------------------------
void CGuildHighOfficerModule::outpostSetDefensePeriod(NLMISC::CSheetId outpostSheet, uint8 hour)
{
	CGuildCharProxy proxy;
	getProxy(proxy);
	
	CGuild * guild = MODULE_CAST<CGuild*>(_GuildMemberCore->getGuild());
	MODULE_AST(guild);
	
	NLMISC::CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	if (outpost == NULL)
		return;
	
	if (proxy.getGuildId() != outpost->getOwnerGuild())
	{
		nlwarning("Player %s is not allowed to edit the outpost '%s'",
			proxy.getId().toString().c_str(),
			outpost->getSheet().toString().c_str()
			);
		return;
	}
	
	// check editing concurrency
	if (!outpost->submitEditingAccess(OUTPOSTENUMS::OutpostOwner, proxy.getId(), COutpost::EditDefenseHour))
		return;
	
	outpost->timeSetDefenseHour(hour);
}

