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
#include "database_outpost.h"

	

TCDBBank CBankAccessor_OUTPOST::BankTag;

		CBankAccessor_OUTPOST::TOUTPOST_SELECTED	CBankAccessor_OUTPOST::_OUTPOST_SELECTED;

void CBankAccessor_OUTPOST::init()
{
	static bool inited = false;
	if (!inited)
	{
		// retreive the bank structure
		CCDBStructBanks	*bank = CCDBStructBanks::instance();
		BankTag = CCDBStructBanks::readBankName("OUTPOST");

		ICDBStructNode *node;

		// branch init
		
		node  = bank->getICDBStructNodeFromName( BankTag, "OUTPOST_SELECTED" );
		nlassert(node != NULL);
		// call sub branch init
		_OUTPOST_SELECTED.init(node);
		

		inited = true;
	}
}
		
void CBankAccessor_OUTPOST::TOUTPOST_SELECTED::init(ICDBStructNode *parent)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	
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
	

	// branch init
	
	node  = parent->getNode( ICDBStructNode::CTextId("GUILD"), false );
	nlassert(node != NULL);
	_GUILD.init(node);
	
}


void CBankAccessor_OUTPOST::TOUTPOST_SELECTED::TGUILD::init(ICDBStructNode *parent)
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

