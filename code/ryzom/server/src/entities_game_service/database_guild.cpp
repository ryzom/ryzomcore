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
#include "database_guild.h"

	

TCDBBank CBankAccessor_GUILD::BankTag;

		CBankAccessor_GUILD::TGUILD	CBankAccessor_GUILD::_GUILD;

void CBankAccessor_GUILD::init()
{
	static bool inited = false;
	if (!inited)
	{
		// retreive the bank structure
		CCDBStructBanks	*bank = CCDBStructBanks::instance();
		BankTag = CCDBStructBanks::readBankName("GUILD");

		ICDBStructNode *node;

		// branch init
		
		node  = bank->getICDBStructNodeFromName( BankTag, "GUILD" );
		nlassert(node != NULL);
		// call sub branch init
		_GUILD.init(node);
		

		inited = true;
	}
}
		
void CBankAccessor_GUILD::TGUILD::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("COUNTER"), false );
	nlassert(node != NULL);
	_COUNTER = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PROXY"), false );
	nlassert(node != NULL);
	_PROXY = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAME"), false );
	nlassert(node != NULL);
	_NAME = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DESCRIPTION"), false );
	nlassert(node != NULL);
	_DESCRIPTION = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ICON"), false );
	nlassert(node != NULL);
	_ICON = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("XP"), false );
	nlassert(node != NULL);
	_XP = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CHARGE_POINTS"), false );
	nlassert(node != NULL);
	_CHARGE_POINTS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("VILLAGE"), false );
	nlassert(node != NULL);
	_VILLAGE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("PEOPLE"), false );
	nlassert(node != NULL);
	_PEOPLE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("CREATION_DATE"), false );
	nlassert(node != NULL);
	_CREATION_DATE = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("FAME"), false );
	nlassert(node != NULL);
	_FAME.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("MEMBERS"), false );
	nlassert(node != NULL);
	_MEMBERS.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("INVENTORY"), false );
	nlassert(node != NULL);
	_INVENTORY.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("OUTPOST"), false );
	nlassert(node != NULL);
	_OUTPOST.init(node);
	
}


void CBankAccessor_GUILD::TGUILD::TFAME::init(ICDBStructNode *parent)
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
	

	// branch init
	
	for (uint i=0; i<6; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_GUILD::TGUILD::TFAME::TArray::init(ICDBStructNode *parent, uint index)
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


void CBankAccessor_GUILD::TGUILD::TMEMBERS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<256; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_GUILD::TGUILD::TMEMBERS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAME"), false );
	nlassert(node != NULL);
	_NAME = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("GRADE"), false );
	nlassert(node != NULL);
	_GRADE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ONLINE"), false );
	nlassert(node != NULL);
	_ONLINE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ENTER_DATE"), false );
	nlassert(node != NULL);
	_ENTER_DATE = node;
	

	// branch init
	
}


void CBankAccessor_GUILD::TGUILD::TINVENTORY::init(ICDBStructNode *parent)
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


void CBankAccessor_GUILD::TGUILD::TOUTPOST::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("CANDEL"), false );
	nlassert(node != NULL);
	_CANDEL = node;
	

	// branch init
	
	for (uint i=0; i<16; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("O%u", i)), false );
		nlassert(node != NULL);
		_O[i].init(node, i);
	}
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("OWNED"), false );
	nlassert(node != NULL);
	_OWNED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("LEVEL"), false );
	nlassert(node != NULL);
	_LEVEL = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("STATUS"), false );
	nlassert(node != NULL);
	_STATUS = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("STATE_END_DATE"), false );
	nlassert(node != NULL);
	_STATE_END_DATE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("DISPLAY_CRASH"), false );
	nlassert(node != NULL);
	_DISPLAY_CRASH = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("WARCOST"), false );
	nlassert(node != NULL);
	_WARCOST = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROUND_LVL_THRESHOLD"), false );
	nlassert(node != NULL);
	_ROUND_LVL_THRESHOLD = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROUND_LVL_MAX_ATT"), false );
	nlassert(node != NULL);
	_ROUND_LVL_MAX_ATT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROUND_LVL_MAX_DEF"), false );
	nlassert(node != NULL);
	_ROUND_LVL_MAX_DEF = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROUND_LVL_CUR"), false );
	nlassert(node != NULL);
	_ROUND_LVL_CUR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROUND_ID_CUR"), false );
	nlassert(node != NULL);
	_ROUND_ID_CUR = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ROUND_ID_MAX"), false );
	nlassert(node != NULL);
	_ROUND_ID_MAX = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TIME_RANGE_DEF_WANTED"), false );
	nlassert(node != NULL);
	_TIME_RANGE_DEF_WANTED = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TIME_RANGE_DEF"), false );
	nlassert(node != NULL);
	_TIME_RANGE_DEF = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TIME_RANGE_ATT"), false );
	nlassert(node != NULL);
	_TIME_RANGE_ATT = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TIME_RANGE_LENGTH"), false );
	nlassert(node != NULL);
	_TIME_RANGE_LENGTH = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SQUAD_CAPITAL"), false );
	nlassert(node != NULL);
	_SQUAD_CAPITAL = node;
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("GUILD"), false );
	nlassert(node != NULL);
	_GUILD.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("SQUAD_SPAWN_ZONE"), false );
	nlassert(node != NULL);
	_SQUAD_SPAWN_ZONE.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("SQUAD_SHOP"), false );
	nlassert(node != NULL);
	_SQUAD_SHOP.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("SQUADS"), false );
	nlassert(node != NULL);
	_SQUADS.init(node);
	
	node  = parent->getNode( ICDBStructNode::CTextId("BUILDINGS"), false );
	nlassert(node != NULL);
	_BUILDINGS.init(node);
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TGUILD::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAME"), false );
	nlassert(node != NULL);
	_NAME = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("ICON"), false );
	nlassert(node != NULL);
	_ICON = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("TRIBE"), false );
	nlassert(node != NULL);
	_TRIBE = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("NAME_ATT"), false );
	nlassert(node != NULL);
	_NAME_ATT = node;
	

	// branch init
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TSQUAD_SPAWN_ZONE::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<16; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TSQUAD_SPAWN_ZONE::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("X"), false );
	nlassert(node != NULL);
	_X = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("Y"), false );
	nlassert(node != NULL);
	_Y = node;
	

	// branch init
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TSQUAD_SHOP::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<16; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("%u", i)), false );
		nlassert(node != NULL);
		_Array[i].init(node, i);
	}
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TSQUAD_SHOP::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	

	// branch init
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TSQUADS::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	

	// branch init
	
	for (uint i=0; i<24; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("SP%u", i)), false );
		nlassert(node != NULL);
		__SP[i].init(node, i);
	}
	
	for (uint i=0; i<24; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("T%u", i)), false );
		nlassert(node != NULL);
		_T[i].init(node, i);
	}
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TSQUADS::TSP::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	

	// branch init
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TSQUADS::TT::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	
	node  = parent->getNode( ICDBStructNode::CTextId("SPAWN"), false );
	nlassert(node != NULL);
	_SPAWN = node;
	

	// branch init
	
}


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TBUILDINGS::init(ICDBStructNode *parent)
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


void CBankAccessor_GUILD::TGUILD::TOUTPOST::TO::TBUILDINGS::TArray::init(ICDBStructNode *parent, uint index)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
	node  = parent->getNode( ICDBStructNode::CTextId("SHEET"), false );
	nlassert(node != NULL);
	_SHEET = node;
	

	// branch init
	
}

