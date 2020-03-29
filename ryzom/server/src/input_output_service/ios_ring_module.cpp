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
#include "nel/net/module_builder_parts.h"

#include "server_share/r2_variables.h"
#include "game_share/r2_share_itf.h"

#include "string_manager.h"
#include "input_output_service.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace R2;

class CIOSRingModule : 
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CIOSRingItfSkel
{

	typedef vector<CEntityId>	TPendingUniverseChars;
	// character that are waiting to enter the ring universe channel
	TPendingUniverseChars		_PendingUniverseChars;

public:

	CIOSRingModule()
	{
		CIOSRingItfSkel::init(this);
	}

	virtual void onModuleDown(IModuleProxy *proxy)
	{
		if (proxy->getModuleClassName() == "ServerEditionModule" && IsRingShard)
		{
			CChatManager &cm = IOS->getChatManager();
			// remove any universe clients
			TGroupId gid(RYZOMID::chatGroup,0);
			CChatGroup &cg = cm.getGroup(gid);
			while (!cg.Members.empty())
			{
				cm.removeFromGroup(gid, *(cg.Members.begin()));
			}
		}
	}

	virtual void onModuleUpdate()
	{
		// check for pending character in universe
		for (uint i=0; i<_PendingUniverseChars.size(); ++i)
		{
			if (TheDataset.getDataSetRow(_PendingUniverseChars[i]).isValid())
			{
				nldebug("CIOSRingModule::onModuleUp : char %s is now available, subscribing in universe chat", _PendingUniverseChars[i].toString().c_str());
				// ok, this one is valid now
				IOS->getChatManager().subscribeCharacterInRingUniverse(_PendingUniverseChars[i]);
				_PendingUniverseChars.erase(_PendingUniverseChars.begin()+i);
				--i;
			}
		}
	}

	////////////////////////////////////////////////////////////////
	// Virtual overrides from char name mapper
	////////////////////////////////////////////////////////////////
	// DSS send a list of ring names user item with a AI instance
	virtual void storeItemNamesForAIInstance(NLNET::IModuleProxy *sender, uint32 aiInstance, const std::vector < TCharMappedInfo > &itemInfo)
	{
		SM->storeItemNamesForAIInstance(aiInstance, itemInfo);
	}

	// DSS ask to put a character in the ring universe channel
	// This is for editors and animator characters only
//	virtual void subscribeCharacterInRingUniverse(NLNET::IModuleProxy *sender, uint32 charId)
//	{
//#pragma message (NL_LOC_WRN "Deprecated")
//		CEntityId eid(RYZOMID::player, charId, 0, 0);
//		if (!TheDataset.getDataSetRow(eid).isValid())
//		{
//			nldebug("subscribeCharacterInRingUniverse : char %s is not available here, retrying later", eid.toString().c_str());
//			// the character is not available in the mirror, wait a little
//			_PendingUniverseChars.push_back(eid);
//		}
//		else
//		{
//			IOS->getChatManager().subscribeCharacterInRingUniverse(eid);
//		}
//	}

	// DSS ask to remove a character from the ring universe channel
	// This is for editors and animator characters only
//	virtual void unsubscribeCharacterInRingUniverse(NLNET::IModuleProxy *sender, uint32 charId)
//	{
//#pragma message (NL_LOC_WRN "Deprecated")
//		CEntityId eid(RYZOMID::player, charId, 0, 0);
//		IOS->getChatManager().unsubscribeCharacterInRingUniverse(eid);
//
//		// remove it from the pending universe chars if needed
//		_PendingUniverseChars.erase(remove(_PendingUniverseChars.begin(), _PendingUniverseChars.end(), eid), _PendingUniverseChars.end());
//	}


};


NLNET_REGISTER_MODULE_FACTORY(CIOSRingModule, "IOSRingModule");
