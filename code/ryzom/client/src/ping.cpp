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

#include <nel/misc/types_nl.h>
#include "ping.h"

#include "interface_v3/interface_manager.h"
#include "time_client.h"

using namespace NLMISC;
using namespace NLGUI;

void CPing::init()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if(pIM)
	{
		CCDBNodeLeaf *pNodeLeaf = CDBManager::getInstance()->getDbProp("SERVER:DEBUG_INFO:Ping", false);
		if(pNodeLeaf)
		{
			ICDBNode::CTextId textId;
			pNodeLeaf->addObserver(this, textId);
			//	nlwarning("CPing: cannot add the observer");
		}
		else
			nlwarning("CPing: 'SERVER:DEBUG_INFO:Ping' does not exist.");
	}
}

void CPing::release()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if(pIM)
	{
		CCDBNodeLeaf *pNodeLeaf = CDBManager::getInstance()->getDbProp("SERVER:DEBUG_INFO:Ping", false);
		if(pNodeLeaf)
		{
			ICDBNode::CTextId textId;
			pNodeLeaf->removeObserver(this, textId);
		}
		else
			nlwarning("CPing: 'SERVER:DEBUG_INFO:Ping' does not exist.");
	}
}

void CPing::update(NLMISC::ICDBNode* node)
{
	CCDBNodeLeaf *leaf = safe_cast<CCDBNodeLeaf *>(node);
	uint32 before = (uint32)leaf->getValue32();
	uint32 current = (uint32)(0xFFFFFFFF & ryzomGetLocalTime());
	if(before > current)
	{
		//nlwarning("DB PING Pb before '%u' after '%u'.", before, current);
		if(ClientCfg.Check)
			nlstop;
	}
	_Ping = current - before;
	_RdyToPing = true;
}

/* end of file */