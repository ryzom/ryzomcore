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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////
#include "stdpch.h"
#include "database_plr.h"

	

TCDBBank CBankAccessor_PLR::BankTag;

		CBankAccessor_PLR::TGameTime	CBankAccessor_PLR::_GameTime;
CBankAccessor_PLR::TINTERFACES	CBankAccessor_PLR::_INTERFACES;
CBankAccessor_PLR::TUSER	CBankAccessor_PLR::_USER;
CBankAccessor_PLR::TDEFENSE	CBankAccessor_PLR::_DEFENSE;
CBankAccessor_PLR::TFLAGS	CBankAccessor_PLR::_FLAGS;
CBankAccessor_PLR::TTARGET	CBankAccessor_PLR::_TARGET;
CBankAccessor_PLR::TGROUP	CBankAccessor_PLR::_GROUP;
CBankAccessor_PLR::TDM_GIFT	CBankAccessor_PLR::_DM_GIFT;
CBankAccessor_PLR::TEXCHANGE	CBankAccessor_PLR::_EXCHANGE;
CBankAccessor_PLR::TINVENTORY	CBankAccessor_PLR::_INVENTORY;
CBankAccessor_PLR::TMODIFIERS	CBankAccessor_PLR::_MODIFIERS;
CBankAccessor_PLR::TDISABLE_CONSUMABLE	CBankAccessor_PLR::_DISABLE_CONSUMABLE;
CBankAccessor_PLR::TBOTCHAT	CBankAccessor_PLR::_BOTCHAT;
CBankAccessor_PLR::TASCENSOR	CBankAccessor_PLR::_ASCENSOR;
CBankAccessor_PLR::TCHOOSE_MISSIONS	CBankAccessor_PLR::_CHOOSE_MISSIONS;
CBankAccessor_PLR::TTRADING	CBankAccessor_PLR::_TRADING;
CBankAccessor_PLR::TBRICK_FAMILY	CBankAccessor_PLR::_BRICK_FAMILY;
CBankAccessor_PLR::TFABER_PLANS	CBankAccessor_PLR::_FABER_PLANS;
CBankAccessor_PLR::TMISSIONS	CBankAccessor_PLR::_MISSIONS;
CBankAccessor_PLR::TEXECUTE_PHRASE	CBankAccessor_PLR::_EXECUTE_PHRASE;
CBankAccessor_PLR::TCHARACTER_INFO	CBankAccessor_PLR::_CHARACTER_INFO;
CBankAccessor_PLR::TPACK_ANIMAL	CBankAccessor_PLR::_PACK_ANIMAL;
CBankAccessor_PLR::TDEBUG_INFO	CBankAccessor_PLR::_DEBUG_INFO;
CBankAccessor_PLR::TMP_EVAL	CBankAccessor_PLR::_MP_EVAL;
CBankAccessor_PLR::TCOMPASS	CBankAccessor_PLR::_COMPASS;
CBankAccessor_PLR::TFAME	CBankAccessor_PLR::_FAME;
CBankAccessor_PLR::TSTATIC_DATA	CBankAccessor_PLR::_STATIC_DATA;
CBankAccessor_PLR::TDYN_CHAT	CBankAccessor_PLR::_DYN_CHAT;
CBankAccessor_PLR::TPVP_EFFECTS	CBankAccessor_PLR::_PVP_EFFECTS;
CBankAccessor_PLR::TWEATHER	CBankAccessor_PLR::_WEATHER;

void CBankAccessor_PLR::init()
{
	static bool inited = false;
	if (!inited)
	{
		// retreive the bank structure
		CCDBStructBanks	*bank = CCDBStructBanks::instance();
		BankTag = CCDBStructBanks::readBankName("PLR");

		ICDBStructNode *node;

		// branch init
		
		node  = bank->getICDBStructNodeFromName( BankTag, "GameTime" );
		nlassert(node != NULL);
		// call sub branch init
		_GameTime.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "INTERFACES" );
		nlassert(node != NULL);
		// call sub branch init
		_INTERFACES.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "USER" );
		nlassert(node != NULL);
		// call sub branch init
		_USER.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "DEFENSE" );
		nlassert(node != NULL);
		// call sub branch init
		_DEFENSE.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "FLAGS" );
		nlassert(node != NULL);
		// call sub branch init
		_FLAGS.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "TARGET" );
		nlassert(node != NULL);
		// call sub branch init
		_TARGET.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "GROUP" );
		nlassert(node != NULL);
		// call sub branch init
		_GROUP.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "DM_GIFT" );
		nlassert(node != NULL);
		// call sub branch init
		_DM_GIFT.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "EXCHANGE" );
		nlassert(node != NULL);
		// call sub branch init
		_EXCHANGE.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "INVENTORY" );
		nlassert(node != NULL);
		// call sub branch init
		_INVENTORY.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "MODIFIERS" );
		nlassert(node != NULL);
		// call sub branch init
		_MODIFIERS.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "DISABLE_CONSUMABLE" );
		nlassert(node != NULL);
		// call sub branch init
		_DISABLE_CONSUMABLE.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "BOTCHAT" );
		nlassert(node != NULL);
		// call sub branch init
		_BOTCHAT.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "ASCENSOR" );
		nlassert(node != NULL);
		// call sub branch init
		_ASCENSOR.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "CHOOSE_MISSIONS" );
		nlassert(node != NULL);
		// call sub branch init
		_CHOOSE_MISSIONS.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "TRADING" );
		nlassert(node != NULL);
		// call sub branch init
		_TRADING.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "BRICK_FAMILY" );
		nlassert(node != NULL);
		// call sub branch init
		_BRICK_FAMILY.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "FABER_PLANS" );
		nlassert(node != NULL);
		// call sub branch init
		_FABER_PLANS.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "MISSIONS" );
		nlassert(node != NULL);
		// call sub branch init
		_MISSIONS.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "EXECUTE_PHRASE" );
		nlassert(node != NULL);
		// call sub branch init
		_EXECUTE_PHRASE.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "CHARACTER_INFO" );
		nlassert(node != NULL);
		// call sub branch init
		_CHARACTER_INFO.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "PACK_ANIMAL" );
		nlassert(node != NULL);
		// call sub branch init
		_PACK_ANIMAL.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "DEBUG_INFO" );
		nlassert(node != NULL);
		// call sub branch init
		_DEBUG_INFO.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "MP_EVAL" );
		nlassert(node != NULL);
		// call sub branch init
		_MP_EVAL.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "COMPASS" );
		nlassert(node != NULL);
		// call sub branch init
		_COMPASS.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "FAME" );
		nlassert(node != NULL);
		// call sub branch init
		_FAME.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "STATIC_DATA" );
		nlassert(node != NULL);
		// call sub branch init
		_STATIC_DATA.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "DYN_CHAT" );
		nlassert(node != NULL);
		// call sub branch init
		_DYN_CHAT.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "PVP_EFFECTS" );
		nlassert(node != NULL);
		// call sub branch init
		_PVP_EFFECTS.init(node);
		
		node  = bank->getICDBStructNodeFromName( BankTag, "WEATHER" );
		nlassert(node != NULL);
		// call sub branch init
		_WEATHER.init(node);
		

		inited = true;
	}
}
		
void CBankAccessor_PLR::TGameTime::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("Hours"), false );
	nlassert(node != NULL);
	_Hours = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TINTERFACES::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("FLAGS"), false );
	nlassert(node != NULL);
	_FLAGS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NB_BONUS_LANDMARKS"), false );
	nlassert(node != NULL);
	_NB_BONUS_LANDMARKS = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TUSER::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("HAIR_TYPE"), false );
	nlassert(node != NULL);
	_HAIR_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("HAIR_COLOR"), false );
	nlassert(node != NULL);
	_HAIR_COLOR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DEATH_XP_MALUS"), false );
	nlassert(node != NULL);
	_DEATH_XP_MALUS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("IN_DUEL"), false );
	nlassert(node != NULL);
	_IN_DUEL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("IN_PVP_CHALLENGE"), false );
	nlassert(node != NULL);
	_IN_PVP_CHALLENGE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MOUNT_WALK_SPEED"), false );
	nlassert(node != NULL);
	_MOUNT_WALK_SPEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MOUNT_RUN_SPEED"), false );
	nlassert(node != NULL);
	_MOUNT_RUN_SPEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEAM_MEMBER"), false );
	nlassert(node != NULL);
	_TEAM_MEMBER = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEAM_LEADER"), false );
	nlassert(node != NULL);
	_TEAM_LEADER = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("OUTPOST_ADMIN"), false );
	nlassert(node != NULL);
	_OUTPOST_ADMIN = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BERSERK"), false );
	nlassert(node != NULL);
	_BERSERK = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ACT_TSTART"), false );
	nlassert(node != NULL);
	_ACT_TSTART = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ACT_TEND"), false );
	nlassert(node != NULL);
	_ACT_TEND = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ACT_TYPE"), false );
	nlassert(node != NULL);
	_ACT_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ACT_NUMBER"), false );
	nlassert(node != NULL);
	_ACT_NUMBER = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ACT_REFUSED_NUM"), false );
	nlassert(node != NULL);
	_ACT_REFUSED_NUM = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ACT_CANCELED_NUM"), false );
	nlassert(node != NULL);
	_ACT_CANCELED_NUM = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SPEED_FACTOR"), false );
	nlassert(node != NULL);
	_SPEED_FACTOR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SKILL_POINTS"), false );
	nlassert(node != NULL);
	_SKILL_POINTS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("IS_NEWBIE"), false );
	nlassert(node != NULL);
	_IS_NEWBIE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("IS_TRIAL"), false );
	nlassert(node != NULL);
	_IS_TRIAL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DEFAULT_WEIGHT_HANDS"), false );
	nlassert(node != NULL);
	_DEFAULT_WEIGHT_HANDS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("IS_INVISIBLE"), false );
	nlassert(node != NULL);
	_IS_INVISIBLE = node;

	node  = parent->getNode( ICDBStructNode::CTextId("COUNTER"), false );
	nlassert(node != NULL);
	_COUNTER = node;
	

	// branch init
	
	for (uint i=0; i<4; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("SKILL_POINTS_%u", i)), false );
		nlassert(node != NULL);
		_SKILL_POINTS_[i].init(node, i);
	}
	
	for (uint i=0; i<6; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("FACTION_POINTS_%u", i)), false );
		nlassert(node != NULL);
		_FACTION_POINTS_[i].init(node, i);
	}
	
	for (uint i=0; i<6; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("RRPS_LEVELS%u", i)), false );
		nlassert(node != NULL);
		_RRPS_LEVELS[i].init(node, i);
	}
	
	node  = parent->getNode( ICDBStructNode::CTextId("NPC_CONTROL"), false );
	nlassert(node != NULL);
	_NPC_CONTROL.init(node);
	
}


void CBankAccessor_PLR::TUSER::TSKILL_POINTS_::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALUE"), false );
	nlassert(node != NULL);
	_VALUE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TUSER::TFACTION_POINTS_::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALUE"), false );
	nlassert(node != NULL);
	_VALUE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TUSER::TRRPS_LEVELS::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALUE"), false );
	nlassert(node != NULL);
	_VALUE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TUSER::TNPC_CONTROL::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RUN"), false );
	nlassert(node != NULL);
	_RUN = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WALK"), false );
	nlassert(node != NULL);
	_WALK = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TDEFENSE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("DEFENSE_MODE"), false );
	nlassert(node != NULL);
	_DEFENSE_MODE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PROTECTED_SLOT"), false );
	nlassert(node != NULL);
	_PROTECTED_SLOT = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SLOTS"), false );
	nlassert(node != NULL);
	_SLOTS.init(node);
	
}


void CBankAccessor_PLR::TDEFENSE::TSLOTS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<6; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TDEFENSE::TSLOTS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("MODIFIER"), false );
	nlassert(node != NULL);
	_MODIFIER = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TFLAGS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("CRITICAL"), false );
	nlassert(node != NULL);
	_CRITICAL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PARRY"), false );
	nlassert(node != NULL);
	_PARRY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DODGE"), false );
	nlassert(node != NULL);
	_DODGE = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("BRICK_TICK_RANGE"), false );
	nlassert(node != NULL);
	_BRICK_TICK_RANGE.init(node);
	
}


void CBankAccessor_PLR::TFLAGS::TBRICK_TICK_RANGE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<64; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TFLAGS::TBRICK_TICK_RANGE::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TICK_RANGE"), false );
	nlassert(node != NULL);
	_TICK_RANGE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TTARGET::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("CONTEXT_VAL"), false );
	nlassert(node != NULL);
	_CONTEXT_VAL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("AGGRESSIVE"), false );
	nlassert(node != NULL);
	_AGGRESSIVE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FORCE_RATIO"), false );
	nlassert(node != NULL);
	_FORCE_RATIO = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("BARS"), false );
	nlassert(node != NULL);
	_BARS.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("CONTEXT_MENU"), false );
	nlassert(node != NULL);
	_CONTEXT_MENU.init(node);
	
}


void CBankAccessor_PLR::TTARGET::TBARS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("UID"), false );
	nlassert(node != NULL);
	_UID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("HP"), false );
	nlassert(node != NULL);
	_HP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SAP"), false );
	nlassert(node != NULL);
	_SAP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("STA"), false );
	nlassert(node != NULL);
	_STA = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FOCUS"), false );
	nlassert(node != NULL);
	_FOCUS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PLAYER_LEVEL"), false );
	nlassert(node != NULL);
	_PLAYER_LEVEL = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TTARGET::TCONTEXT_MENU::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("PROGRAMMES"), false );
	nlassert(node != NULL);
	_PROGRAMMES = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WEB_PAGE_TITLE"), false );
	nlassert(node != NULL);
	_WEB_PAGE_TITLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WEB_PAGE_URL"), false );
	nlassert(node != NULL);
	_WEB_PAGE_URL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("OUTPOST"), false );
	nlassert(node != NULL);
	_OUTPOST = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("COUNTER"), false );
	nlassert(node != NULL);
	_COUNTER = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("MISSIONS_OPTIONS"), false );
	nlassert(node != NULL);
	_MISSIONS_OPTIONS.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("MISSION_RING"), false );
	nlassert(node != NULL);
	_MISSION_RING.init(node);
	
}


void CBankAccessor_PLR::TTARGET::TCONTEXT_MENU::TMISSIONS_OPTIONS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TTARGET::TCONTEXT_MENU::TMISSIONS_OPTIONS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TITLE"), false );
	nlassert(node != NULL);
	_TITLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PLAYER_GIFT_NEEDED"), false );
	nlassert(node != NULL);
	_PLAYER_GIFT_NEEDED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PRIORITY"), false );
	nlassert(node != NULL);
	_PRIORITY = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TTARGET::TCONTEXT_MENU::TMISSION_RING::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<4; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TTARGET::TCONTEXT_MENU::TMISSION_RING::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TITLE"), false );
	nlassert(node != NULL);
	_TITLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ID"), false );
	nlassert(node != NULL);
	_ID = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TGROUP::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("LEADER_INDEX"), false );
	nlassert(node != NULL);
	_LEADER_INDEX = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SUCCESSOR_INDEX"), false );
	nlassert(node != NULL);
	_SUCCESSOR_INDEX = node;
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
	node  = parent->getNode( ICDBStructNode::CTextId("MISSIONS"), false );
	nlassert(node != NULL);
	_MISSIONS.init(node);
	
}


void CBankAccessor_PLR::TGROUP::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("PRESENT"), false );
	nlassert(node != NULL);
	_PRESENT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("UID"), false );
	nlassert(node != NULL);
	_UID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAME"), false );
	nlassert(node != NULL);
	_NAME = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("HP"), false );
	nlassert(node != NULL);
	_HP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SAP"), false );
	nlassert(node != NULL);
	_SAP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("STA"), false );
	nlassert(node != NULL);
	_STA = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("POS"), false );
	nlassert(node != NULL);
	_POS = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TGROUP::TMISSIONS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<15; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TGROUP::TMISSIONS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TYPE"), false );
	nlassert(node != NULL);
	_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ICON"), false );
	nlassert(node != NULL);
	_ICON = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TITLE"), false );
	nlassert(node != NULL);
	_TITLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DETAIL_TEXT"), false );
	nlassert(node != NULL);
	_DETAIL_TEXT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BEGIN_DATE"), false );
	nlassert(node != NULL);
	_BEGIN_DATE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("END_DATE"), false );
	nlassert(node != NULL);
	_END_DATE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("OR_STEPS"), false );
	nlassert(node != NULL);
	_OR_STEPS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FINISHED"), false );
	nlassert(node != NULL);
	_FINISHED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ABANDONNABLE"), false );
	nlassert(node != NULL);
	_ABANDONNABLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SLEEP"), false );
	nlassert(node != NULL);
	_SLEEP = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("GOALS"), false );
	nlassert(node != NULL);
	_GOALS.init(node);
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("TARGET%u", i)), false );
		nlassert(node != NULL);
		_TARGET[i].init(node, i);
	}
	
	node  = parent->getNode( ICDBStructNode::CTextId("HISTO"), false );
	nlassert(node != NULL);
	_HISTO.init(node);
	
}


void CBankAccessor_PLR::TGROUP::TMISSIONS::TArray::TGOALS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<20; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TGROUP::TMISSIONS::TArray::TGOALS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEXT"), false );
	nlassert(node != NULL);
	_TEXT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NPC_ALIAS"), false );
	nlassert(node != NULL);
	_NPC_ALIAS = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TGROUP::TMISSIONS::TArray::TTARGET::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TITLE"), false );
	nlassert(node != NULL);
	_TITLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("X"), false );
	nlassert(node != NULL);
	_X = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("Y"), false );
	nlassert(node != NULL);
	_Y = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TGROUP::TMISSIONS::TArray::THISTO::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<30; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TGROUP::TMISSIONS::TArray::THISTO::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEXT"), false );
	nlassert(node != NULL);
	_TEXT = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TDM_GIFT::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEXT"), false );
	nlassert(node != NULL);
	_TEXT = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TEXCHANGE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEXT"), false );
	nlassert(node != NULL);
	_TEXT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ID"), false );
	nlassert(node != NULL);
	_ID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BEGUN"), false );
	nlassert(node != NULL);
	_BEGUN = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ACCEPTED"), false );
	nlassert(node != NULL);
	_ACCEPTED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MONEY"), false );
	nlassert(node != NULL);
	_MONEY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FORCE_REFUSE"), false );
	nlassert(node != NULL);
	_FORCE_REFUSE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("COUNTER"), false );
	nlassert(node != NULL);
	_COUNTER = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("GIVE"), false );
	nlassert(node != NULL);
	_GIVE.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("RECEIVE"), false );
	nlassert(node != NULL);
	_RECEIVE.init(node);
	
}


void CBankAccessor_PLR::TEXCHANGE::TGIVE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TEXCHANGE::TGIVE::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUALITY"), false );
	nlassert(node != NULL);
	_QUALITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUANTITY"), false );
	nlassert(node != NULL);
	_QUANTITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("USER_COLOR"), false );
	nlassert(node != NULL);
	_USER_COLOR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WEIGHT"), false );
	nlassert(node != NULL);
	_WEIGHT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAMEID"), false );
	nlassert(node != NULL);
	_NAMEID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("INFO_VERSION"), false );
	nlassert(node != NULL);
	_INFO_VERSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ENCHANT"), false );
	nlassert(node != NULL);
	_ENCHANT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_CLASS_TYPE"), false );
	nlassert(node != NULL);
	_RM_CLASS_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_FABER_STAT_TYPE"), false );
	nlassert(node != NULL);
	_RM_FABER_STAT_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PREREQUISIT_VALID"), false );
	nlassert(node != NULL);
	_PREREQUISIT_VALID = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TEXCHANGE::TRECEIVE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TEXCHANGE::TRECEIVE::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUALITY"), false );
	nlassert(node != NULL);
	_QUALITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUANTITY"), false );
	nlassert(node != NULL);
	_QUANTITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("USER_COLOR"), false );
	nlassert(node != NULL);
	_USER_COLOR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WEIGHT"), false );
	nlassert(node != NULL);
	_WEIGHT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAMEID"), false );
	nlassert(node != NULL);
	_NAMEID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("INFO_VERSION"), false );
	nlassert(node != NULL);
	_INFO_VERSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ENCHANT"), false );
	nlassert(node != NULL);
	_ENCHANT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_CLASS_TYPE"), false );
	nlassert(node != NULL);
	_RM_CLASS_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_FABER_STAT_TYPE"), false );
	nlassert(node != NULL);
	_RM_FABER_STAT_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PREREQUISIT_VALID"), false );
	nlassert(node != NULL);
	_PREREQUISIT_VALID = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TINVENTORY::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("MONEY"), false );
	nlassert(node != NULL);
	_MONEY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("COUNTER"), false );
	nlassert(node != NULL);
	_COUNTER = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("HAND"), false );
	nlassert(node != NULL);
	_HAND.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("EQUIP"), false );
	nlassert(node != NULL);
	_EQUIP.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEMP"), false );
	nlassert(node != NULL);
	_TEMP.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHARE"), false );
	nlassert(node != NULL);
	_SHARE.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROOM"), false );
	nlassert(node != NULL);
	_ROOM.init(node);
	
}


void CBankAccessor_PLR::TINVENTORY::THAND::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<2; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TINVENTORY::THAND::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("INDEX_IN_BAG"), false );
	nlassert(node != NULL);
	_INDEX_IN_BAG = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TINVENTORY::TEQUIP::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<19; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TINVENTORY::TEQUIP::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("INDEX_IN_BAG"), false );
	nlassert(node != NULL);
	_INDEX_IN_BAG = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TINVENTORY::TTEMP::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TYPE"), false );
	nlassert(node != NULL);
	_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ENABLE_TAKE"), false );
	nlassert(node != NULL);
	_ENABLE_TAKE = node;
	

	// branch init
	
	for (uint i=0; i<16; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TINVENTORY::TTEMP::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUALITY"), false );
	nlassert(node != NULL);
	_QUALITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUANTITY"), false );
	nlassert(node != NULL);
	_QUANTITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("USER_COLOR"), false );
	nlassert(node != NULL);
	_USER_COLOR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WEIGHT"), false );
	nlassert(node != NULL);
	_WEIGHT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAMEID"), false );
	nlassert(node != NULL);
	_NAMEID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("INFO_VERSION"), false );
	nlassert(node != NULL);
	_INFO_VERSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ENCHANT"), false );
	nlassert(node != NULL);
	_ENCHANT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_CLASS_TYPE"), false );
	nlassert(node != NULL);
	_RM_CLASS_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_FABER_STAT_TYPE"), false );
	nlassert(node != NULL);
	_RM_FABER_STAT_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PREREQUISIT_VALID"), false );
	nlassert(node != NULL);
	_PREREQUISIT_VALID = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TINVENTORY::TSHARE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SESSION"), false );
	nlassert(node != NULL);
	_SESSION = node;
	

	// branch init
	
	for (uint i=0; i<16; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("TM_%u", i)), false );
		nlassert(node != NULL);
		_TM_[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TINVENTORY::TSHARE::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUALITY"), false );
	nlassert(node != NULL);
	_QUALITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUANTITY"), false );
	nlassert(node != NULL);
	_QUANTITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("USER_COLOR"), false );
	nlassert(node != NULL);
	_USER_COLOR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WEIGHT"), false );
	nlassert(node != NULL);
	_WEIGHT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAMEID"), false );
	nlassert(node != NULL);
	_NAMEID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("INFO_VERSION"), false );
	nlassert(node != NULL);
	_INFO_VERSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ENCHANT"), false );
	nlassert(node != NULL);
	_ENCHANT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_CLASS_TYPE"), false );
	nlassert(node != NULL);
	_RM_CLASS_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_FABER_STAT_TYPE"), false );
	nlassert(node != NULL);
	_RM_FABER_STAT_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PREREQUISIT_VALID"), false );
	nlassert(node != NULL);
	_PREREQUISIT_VALID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NB_MEMBER"), false );
	nlassert(node != NULL);
	_NB_MEMBER = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WANTED"), false );
	nlassert(node != NULL);
	_WANTED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CHANCE"), false );
	nlassert(node != NULL);
	_CHANCE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TINVENTORY::TSHARE::TTM_::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAME"), false );
	nlassert(node != NULL);
	_NAME = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALID"), false );
	nlassert(node != NULL);
	_VALID = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TINVENTORY::TROOM::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SESSION"), false );
	nlassert(node != NULL);
	_SESSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BULK_MAX"), false );
	nlassert(node != NULL);
	_BULK_MAX = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MONEY"), false );
	nlassert(node != NULL);
	_MONEY = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TMODIFIERS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TOTAL_MALUS_EQUIP"), false );
	nlassert(node != NULL);
	_TOTAL_MALUS_EQUIP = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("BONUS"), false );
	nlassert(node != NULL);
	_BONUS.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("MALUS"), false );
	nlassert(node != NULL);
	_MALUS.init(node);
	
}


void CBankAccessor_PLR::TMODIFIERS::TBONUS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<12; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TMODIFIERS::TBONUS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DISABLED"), false );
	nlassert(node != NULL);
	_DISABLED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DISABLED_TIME"), false );
	nlassert(node != NULL);
	_DISABLED_TIME = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TMODIFIERS::TMALUS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<12; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TMODIFIERS::TMALUS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DISABLED"), false );
	nlassert(node != NULL);
	_DISABLED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DISABLED_TIME"), false );
	nlassert(node != NULL);
	_DISABLED_TIME = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TDISABLE_CONSUMABLE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<12; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TDISABLE_CONSUMABLE::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("FAMILY"), false );
	nlassert(node != NULL);
	_FAMILY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DISABLE_TIME"), false );
	nlassert(node != NULL);
	_DISABLE_TIME = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TBOTCHAT::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("PLAYER_GIFT"), false );
	nlassert(node != NULL);
	_PLAYER_GIFT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CREATE_GUILD"), false );
	nlassert(node != NULL);
	_CREATE_GUILD = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TRADE"), false );
	nlassert(node != NULL);
	_TRADE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CHOOSE_MISSION"), false );
	nlassert(node != NULL);
	_CHOOSE_MISSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DM_TITLE"), false );
	nlassert(node != NULL);
	_DM_TITLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DM_DESCRIPTION"), false );
	nlassert(node != NULL);
	_DM_DESCRIPTION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROLEMASTER_TYPE"), false );
	nlassert(node != NULL);
	_ROLEMASTER_TYPE = node;
	

	// branch init
	
	for (uint i=0; i<3; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("DM_CHOICE%u", i)), false );
		nlassert(node != NULL);
		_DM_CHOICE[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TBOTCHAT::TDM_CHOICE::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TITLE"), false );
	nlassert(node != NULL);
	_TITLE = node;
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TBOTCHAT::TDM_CHOICE::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEXT"), false );
	nlassert(node != NULL);
	_TEXT = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TASCENSOR::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SESSION"), false );
	nlassert(node != NULL);
	_SESSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PAGE_ID"), false );
	nlassert(node != NULL);
	_PAGE_ID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("HAS_NEXT"), false );
	nlassert(node != NULL);
	_HAS_NEXT = node;
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TASCENSOR::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("ICON"), false );
	nlassert(node != NULL);
	_ICON = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAME"), false );
	nlassert(node != NULL);
	_NAME = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHOOSE_MISSIONS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SESSION"), false );
	nlassert(node != NULL);
	_SESSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PAGE_ID"), false );
	nlassert(node != NULL);
	_PAGE_ID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("HAS_NEXT"), false );
	nlassert(node != NULL);
	_HAS_NEXT = node;
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TCHOOSE_MISSIONS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("ICON"), false );
	nlassert(node != NULL);
	_ICON = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEXT"), false );
	nlassert(node != NULL);
	_TEXT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DETAIL_TEXT"), false );
	nlassert(node != NULL);
	_DETAIL_TEXT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PREREQ_STATE"), false );
	nlassert(node != NULL);
	_PREREQ_STATE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TTRADING::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SESSION"), false );
	nlassert(node != NULL);
	_SESSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PAGE_ID"), false );
	nlassert(node != NULL);
	_PAGE_ID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("HAS_NEXT"), false );
	nlassert(node != NULL);
	_HAS_NEXT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROLEMASTER_FLAGS"), false );
	nlassert(node != NULL);
	_ROLEMASTER_FLAGS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROLEMASTER_RACE"), false );
	nlassert(node != NULL);
	_ROLEMASTER_RACE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BUILDING_LOSS_WARNING"), false );
	nlassert(node != NULL);
	_BUILDING_LOSS_WARNING = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RAW_MATERIAL_SELLER"), false );
	nlassert(node != NULL);
	_RAW_MATERIAL_SELLER = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ITEM_TYPE_SELLER_BITFILED_0_63"), false );
	nlassert(node != NULL);
	_ITEM_TYPE_SELLER_BITFILED_0_63 = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ITEM_TYPE_SELLER_BITFILED_64_127"), false );
	nlassert(node != NULL);
	_ITEM_TYPE_SELLER_BITFILED_64_127 = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FAME_PRICE_FACTOR"), false );
	nlassert(node != NULL);
	_FAME_PRICE_FACTOR = node;
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TTRADING::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUALITY"), false );
	nlassert(node != NULL);
	_QUALITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("QUANTITY"), false );
	nlassert(node != NULL);
	_QUANTITY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("USER_COLOR"), false );
	nlassert(node != NULL);
	_USER_COLOR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WEIGHT"), false );
	nlassert(node != NULL);
	_WEIGHT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAMEID"), false );
	nlassert(node != NULL);
	_NAMEID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("INFO_VERSION"), false );
	nlassert(node != NULL);
	_INFO_VERSION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ENCHANT"), false );
	nlassert(node != NULL);
	_ENCHANT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_CLASS_TYPE"), false );
	nlassert(node != NULL);
	_RM_CLASS_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RM_FABER_STAT_TYPE"), false );
	nlassert(node != NULL);
	_RM_FABER_STAT_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PREREQUISIT_VALID"), false );
	nlassert(node != NULL);
	_PREREQUISIT_VALID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CURRENCY"), false );
	nlassert(node != NULL);
	_CURRENCY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RRP_LEVEL"), false );
	nlassert(node != NULL);
	_RRP_LEVEL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MONEY_SHEET"), false );
	nlassert(node != NULL);
	_MONEY_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BASE_SKILL"), false );
	nlassert(node != NULL);
	_BASE_SKILL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FACTION_TYPE"), false );
	nlassert(node != NULL);
	_FACTION_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PRICE"), false );
	nlassert(node != NULL);
	_PRICE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PRICE_RETIRE"), false );
	nlassert(node != NULL);
	_PRICE_RETIRE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RESALE_TIME_LEFT"), false );
	nlassert(node != NULL);
	_RESALE_TIME_LEFT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("VENDOR_NAMEID"), false );
	nlassert(node != NULL);
	_VENDOR_NAMEID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FACTION_POINT_PRICE"), false );
	nlassert(node != NULL);
	_FACTION_POINT_PRICE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SLOT_TYPE"), false );
	nlassert(node != NULL);
	_SLOT_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SELLER_TYPE"), false );
	nlassert(node != NULL);
	_SELLER_TYPE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TBRICK_FAMILY::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<1024; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TBRICK_FAMILY::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("BRICKS"), false );
	nlassert(node != NULL);
	_BRICKS = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TFABER_PLANS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<64; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TFABER_PLANS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("KNOWN"), false );
	nlassert(node != NULL);
	_KNOWN = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TMISSIONS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<15; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TMISSIONS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TYPE"), false );
	nlassert(node != NULL);
	_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ICON"), false );
	nlassert(node != NULL);
	_ICON = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TITLE"), false );
	nlassert(node != NULL);
	_TITLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DETAIL_TEXT"), false );
	nlassert(node != NULL);
	_DETAIL_TEXT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BEGIN_DATE"), false );
	nlassert(node != NULL);
	_BEGIN_DATE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("END_DATE"), false );
	nlassert(node != NULL);
	_END_DATE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("OR_STEPS"), false );
	nlassert(node != NULL);
	_OR_STEPS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FINISHED"), false );
	nlassert(node != NULL);
	_FINISHED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ABANDONNABLE"), false );
	nlassert(node != NULL);
	_ABANDONNABLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SLEEP"), false );
	nlassert(node != NULL);
	_SLEEP = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("GOALS"), false );
	nlassert(node != NULL);
	_GOALS.init(node);
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("TARGET%u", i)), false );
		nlassert(node != NULL);
		_TARGET[i].init(node, i);
	}
	
	node  = parent->getNode( ICDBStructNode::CTextId("HISTO"), false );
	nlassert(node != NULL);
	_HISTO.init(node);
	
}


void CBankAccessor_PLR::TMISSIONS::TArray::TGOALS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<20; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TMISSIONS::TArray::TGOALS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEXT"), false );
	nlassert(node != NULL);
	_TEXT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NPC_ALIAS"), false );
	nlassert(node != NULL);
	_NPC_ALIAS = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TMISSIONS::TArray::TTARGET::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TITLE"), false );
	nlassert(node != NULL);
	_TITLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("X"), false );
	nlassert(node != NULL);
	_X = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("Y"), false );
	nlassert(node != NULL);
	_Y = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TMISSIONS::TArray::THISTO::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<30; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TMISSIONS::TArray::THISTO::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TEXT"), false );
	nlassert(node != NULL);
	_TEXT = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TEXECUTE_PHRASE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("PHRASE"), false );
	nlassert(node != NULL);
	_PHRASE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NEXT_COUNTER"), false );
	nlassert(node != NULL);
	_NEXT_COUNTER = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CYCLE_COUNTER"), false );
	nlassert(node != NULL);
	_CYCLE_COUNTER = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("LINK"), false );
	nlassert(node != NULL);
	_LINK.init(node);
	
}


void CBankAccessor_PLR::TEXECUTE_PHRASE::TLINK::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<10; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TEXECUTE_PHRASE::TLINK::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("PHRASE"), false );
	nlassert(node != NULL);
	_PHRASE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("COUNTER"), false );
	nlassert(node != NULL);
	_COUNTER = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("HP_COST"), false );
	nlassert(node != NULL);
	_HP_COST = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SAP_COST"), false );
	nlassert(node != NULL);
	_SAP_COST = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("STA_COST"), false );
	nlassert(node != NULL);
	_STA_COST = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TARGET_NAME"), false );
	nlassert(node != NULL);
	_TARGET_NAME = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TARGET_HP"), false );
	nlassert(node != NULL);
	_TARGET_HP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TARGET_SAP"), false );
	nlassert(node != NULL);
	_TARGET_SAP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TARGET_STA"), false );
	nlassert(node != NULL);
	_TARGET_STA = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("CHARACTERISTICS%u", i)), false );
		nlassert(node != NULL);
		_CHARACTERISTICS[i].init(node, i);
	}
	
	for (uint i=0; i<4; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("SCORES%u", i)), false );
		nlassert(node != NULL);
		_SCORES[i].init(node, i);
	}
	
	node  = parent->getNode( ICDBStructNode::CTextId("MAGIC_RESISTANCE"), false );
	nlassert(node != NULL);
	_MAGIC_RESISTANCE.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("MAGIC_PROTECTION"), false );
	nlassert(node != NULL);
	_MAGIC_PROTECTION.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("DODGE"), false );
	nlassert(node != NULL);
	_DODGE.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("PARRY"), false );
	nlassert(node != NULL);
	_PARRY.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("SKILLS"), false );
	nlassert(node != NULL);
	_SKILLS.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("XP_CATALYSER"), false );
	nlassert(node != NULL);
	_XP_CATALYSER.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("RING_XP_CATALYSER"), false );
	nlassert(node != NULL);
	_RING_XP_CATALYSER.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("PVP_FACTION_TAG"), false );
	nlassert(node != NULL);
	_PVP_FACTION_TAG.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("PVP_OUTPOST"), false );
	nlassert(node != NULL);
	_PVP_OUTPOST.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("SUCCESS_MODIFIER"), false );
	nlassert(node != NULL);
	_SUCCESS_MODIFIER.init(node);
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TCHARACTERISTICS::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALUE"), false );
	nlassert(node != NULL);
	_VALUE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TSCORES::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("Base"), false );
	nlassert(node != NULL);
	_Base = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("Max"), false );
	nlassert(node != NULL);
	_Max = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BaseRegen"), false );
	nlassert(node != NULL);
	_BaseRegen = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("Regen"), false );
	nlassert(node != NULL);
	_Regen = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TMAGIC_RESISTANCE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("MaxResistanceBonus"), false );
	nlassert(node != NULL);
	_MaxResistanceBonus = node;
	

	// branch init
	
	for (uint i=0; i<5; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TMAGIC_RESISTANCE::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALUE"), false );
	nlassert(node != NULL);
	_VALUE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TMAGIC_PROTECTION::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("MaxProtectionClampValue"), false );
	nlassert(node != NULL);
	_MaxProtectionClampValue = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MaxAbsorptionFactor"), false );
	nlassert(node != NULL);
	_MaxAbsorptionFactor = node;
	

	// branch init
	
	for (uint i=0; i<7; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TMAGIC_PROTECTION::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALUE"), false );
	nlassert(node != NULL);
	_VALUE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TDODGE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("Base"), false );
	nlassert(node != NULL);
	_Base = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("Current"), false );
	nlassert(node != NULL);
	_Current = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TPARRY::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("Base"), false );
	nlassert(node != NULL);
	_Base = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("Current"), false );
	nlassert(node != NULL);
	_Current = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TSKILLS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<225; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TSKILLS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SKILL"), false );
	nlassert(node != NULL);
	_SKILL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BaseSKILL"), false );
	nlassert(node != NULL);
	_BaseSKILL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PROGRESS_BAR"), false );
	nlassert(node != NULL);
	_PROGRESS_BAR = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TXP_CATALYSER::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("Level"), false );
	nlassert(node != NULL);
	_Level = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("Count"), false );
	nlassert(node != NULL);
	_Count = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TRING_XP_CATALYSER::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("Level"), false );
	nlassert(node != NULL);
	_Level = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("Count"), false );
	nlassert(node != NULL);
	_Count = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TPVP_FACTION_TAG::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("TAG_PVP"), false );
	nlassert(node != NULL);
	_TAG_PVP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ACTIVATION_TIME"), false );
	nlassert(node != NULL);
	_ACTIVATION_TIME = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FLAG_PVP_TIME_LEFT"), false );
	nlassert(node != NULL);
	_FLAG_PVP_TIME_LEFT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("COUNTER"), false );
	nlassert(node != NULL);
	_COUNTER = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TPVP_OUTPOST::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("FLAG_PVP"), false );
	nlassert(node != NULL);
	_FLAG_PVP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RIGHT_TO_BANISH"), false );
	nlassert(node != NULL);
	_RIGHT_TO_BANISH = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROUND_LVL_CUR"), false );
	nlassert(node != NULL);
	_ROUND_LVL_CUR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROUND_END_DATE"), false );
	nlassert(node != NULL);
	_ROUND_END_DATE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("FLAG_PVP_TIME_END"), false );
	nlassert(node != NULL);
	_FLAG_PVP_TIME_END = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TSUCCESS_MODIFIER::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("DODGE"), false );
	nlassert(node != NULL);
	_DODGE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PARRY"), false );
	nlassert(node != NULL);
	_PARRY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CRAFT"), false );
	nlassert(node != NULL);
	_CRAFT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MELEE"), false );
	nlassert(node != NULL);
	_MELEE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("RANGE"), false );
	nlassert(node != NULL);
	_RANGE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MAGIC"), false );
	nlassert(node != NULL);
	_MAGIC = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("ECO"), false );
	nlassert(node != NULL);
	_ECO.init(node);
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TSUCCESS_MODIFIER::TECO::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<7; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TCHARACTER_INFO::TSUCCESS_MODIFIER::TECO::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("FORAGE"), false );
	nlassert(node != NULL);
	_FORAGE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TPACK_ANIMAL::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<4; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("BEAST%u", i)), false );
		nlassert(node != NULL);
		_BEAST[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TPACK_ANIMAL::TBEAST::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("UID"), false );
	nlassert(node != NULL);
	_UID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TYPE"), false );
	nlassert(node != NULL);
	_TYPE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("STATUS"), false );
	nlassert(node != NULL);
	_STATUS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("HP"), false );
	nlassert(node != NULL);
	_HP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BULK_MAX"), false );
	nlassert(node != NULL);
	_BULK_MAX = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("POS"), false );
	nlassert(node != NULL);
	_POS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("HUNGER"), false );
	nlassert(node != NULL);
	_HUNGER = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DESPAWN"), false );
	nlassert(node != NULL);
	_DESPAWN = node;

	node  = parent->getNode( ICDBStructNode::CTextId("NAME"), false );
	nlassert(node != NULL);
	_NAME = node;

	// branch init
	
}


void CBankAccessor_PLR::TDEBUG_INFO::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("Ping"), false );
	nlassert(node != NULL);
	_Ping = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TMP_EVAL::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("COST"), false );
	nlassert(node != NULL);
	_COST = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("RESULT"), false );
	nlassert(node != NULL);
	_RESULT.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("RESULT_CRITICAL"), false );
	nlassert(node != NULL);
	_RESULT_CRITICAL.init(node);
	
}


void CBankAccessor_PLR::TMP_EVAL::TRESULT::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALID"), false );
	nlassert(node != NULL);
	_VALID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SMALL_SEED"), false );
	nlassert(node != NULL);
	_SMALL_SEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MEDIUM_SEED"), false );
	nlassert(node != NULL);
	_MEDIUM_SEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BIG_SEED"), false );
	nlassert(node != NULL);
	_BIG_SEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("VERY_BIG_SEED"), false );
	nlassert(node != NULL);
	_VERY_BIG_SEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("EXPIRY_DATE"), false );
	nlassert(node != NULL);
	_EXPIRY_DATE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TMP_EVAL::TRESULT_CRITICAL::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALID"), false );
	nlassert(node != NULL);
	_VALID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SMALL_SEED"), false );
	nlassert(node != NULL);
	_SMALL_SEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("MEDIUM_SEED"), false );
	nlassert(node != NULL);
	_MEDIUM_SEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BIG_SEED"), false );
	nlassert(node != NULL);
	_BIG_SEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("VERY_BIG_SEED"), false );
	nlassert(node != NULL);
	_VERY_BIG_SEED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("EXPIRY_DATE"), false );
	nlassert(node != NULL);
	_EXPIRY_DATE = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TCOMPASS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("HOME_POINT"), false );
	nlassert(node != NULL);
	_HOME_POINT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("BIND_POINT"), false );
	nlassert(node != NULL);
	_BIND_POINT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TARGET"), false );
	nlassert(node != NULL);
	_TARGET = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TFAME::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("CULT_ALLEGIANCE"), false );
	nlassert(node != NULL);
	_CULT_ALLEGIANCE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CIV_ALLEGIANCE"), false );
	nlassert(node != NULL);
	_CIV_ALLEGIANCE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("THRESHOLD_TRADE"), false );
	nlassert(node != NULL);
	_THRESHOLD_TRADE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("THRESHOLD_KOS"), false );
	nlassert(node != NULL);
	_THRESHOLD_KOS = node;
	

	// branch init
	
	for (uint i=0; i<6; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("PLAYER%u", i)), false );
		nlassert(node != NULL);
		_PLAYER[i].init(node, i);
	}
	
	for (uint i=0; i<53; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("TRIBE%u", i)), false );
		nlassert(node != NULL);
		_TRIBE[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TFAME::TPLAYER::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALUE"), false );
	nlassert(node != NULL);
	_VALUE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("THRESHOLD"), false );
	nlassert(node != NULL);
	_THRESHOLD = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TREND"), false );
	nlassert(node != NULL);
	_TREND = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TFAME::TTRIBE::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALUE"), false );
	nlassert(node != NULL);
	_VALUE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("THRESHOLD"), false );
	nlassert(node != NULL);
	_THRESHOLD = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TREND"), false );
	nlassert(node != NULL);
	_TREND = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TSTATIC_DATA::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("BAG_BULK_MAX"), false );
	nlassert(node != NULL);
	_BAG_BULK_MAX = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TDYN_CHAT::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<8; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("CHANNEL%u", i)), false );
		nlassert(node != NULL);
		_CHANNEL[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TDYN_CHAT::TCHANNEL::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAME"), false );
	nlassert(node != NULL);
	_NAME = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ID"), false );
	nlassert(node != NULL);
	_ID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WRITE_RIGHT"), false );
	nlassert(node != NULL);
	_WRITE_RIGHT = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TPVP_EFFECTS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("PVP_FACTION_POINTS"), false );
	nlassert(node != NULL);
	_PVP_FACTION_POINTS.init(node);
	
	for (uint i=0; i<59; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_PLR::TPVP_EFFECTS::TPVP_FACTION_POINTS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("CIV"), false );
	nlassert(node != NULL);
	_CIV = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CIV_POINTS"), false );
	nlassert(node != NULL);
	_CIV_POINTS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CULT"), false );
	nlassert(node != NULL);
	_CULT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CULT_POINTS"), false );
	nlassert(node != NULL);
	_CULT_POINTS = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TPVP_EFFECTS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("ID"), false );
	nlassert(node != NULL);
	_ID = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ISBONUS"), false );
	nlassert(node != NULL);
	_ISBONUS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PARAM"), false );
	nlassert(node != NULL);
	_PARAM = node;
	

	// branch init
	
}


void CBankAccessor_PLR::TWEATHER::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("VALUE"), false );
	nlassert(node != NULL);
	_VALUE = node;
	

	// branch init
	
}

