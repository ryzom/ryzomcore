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

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "npc_description_messages.h"
#include "../../server/src/ai_share/ai_share.h"

using namespace NLMISC;
using namespace std;


//---------------------------------------------------------------------------------------
// Stuff used for management of log messages

bool VerboseNpcDescriptionMsgLog=false;
#define LOG if (!VerboseNpcDescriptionMsgLog) {} else nlinfo

CNpcChatProfile::CNpcChatProfile(const CNpcChatProfile &other0, const CNpcChatProfile &other1)
{
	uint i,j;

	// run through shop types in other0 - trying to add to output shop type list
	for (i=0;i<other0.getShopTypes().size();++i)
	{
		// not found in negator list so add to output list
		_ShopTypes.push_back(other0._ShopTypes[i]);
	}

	// run through shop types in other1 - trying to add to output shop type list
	for (i=0;i<other1._ShopTypes.size();++i)
	{
		// make sure shop type isn't already in output list
		for (j=0;j<_ShopTypes.size();++j)
			if (other1._ShopTypes[i]==_ShopTypes[j])
				break;
		if (j<_ShopTypes.size())
			continue;

		// not found in negator list or existing output list so add it now
		_ShopTypes.push_back(other0._ShopTypes[i]);
	}

	// run through shop item types in other0 - trying to add to output shop item type list
	for (i=0;i<other0._ExplicitSales.size();++i)
	{
		_ExplicitSales.push_back(other0._ExplicitSales[i]);
	}

	// run through shop item types in other1 - trying to add to output shop item type list
	for (i=0;i<other1._ExplicitSales.size();++i)
	{
		// make sure shop item type isn't already in output list
		for (j=0;j<_ExplicitSales.size();++j)
			if (other1._ExplicitSales[i]==_ExplicitSales[j])
				break;
		if (j<_ExplicitSales.size())
			continue;

		// not found in negator list or existing output list so add it now
		_ExplicitSales.push_back(other1._ExplicitSales[i]);
	}

	// run through missions in other0 - trying to add to output mission list
	_Missions=other0._Missions;
	for (i=0;i<other1._Missions.size();++i)
	{
		// make sure shop type isn't already in output list
		for (j=0;j<_Missions.size();++j)
			if (other1._Missions[i]==_Missions[j])
				break;
		if (j<_Missions.size())
			continue;

		// not found in existing output list so add it now
		_Missions.push_back(other1._Missions[i]);
	}
}

void CCustomElementId::serial(NLMISC::IStream &f)
{
	f.serial(PrimAlias);
	f.serial(Id);
}

void CScriptData::serial(NLMISC::IStream &f)
{
	uint16 size;
	if (f.isReading())
	{
		Scripts.clear();
		f.serial(size);

		uint32 i = 0;
		for (; i < size; ++i)
		{
			//std::string tmpKey;
			CCustomElementId tmpKey;
			std::vector<std::string> tmpVal;
			f.serial(tmpKey);
			f.serialCont(tmpVal);
			Scripts.insert(make_pair(tmpKey,tmpVal));
		}
	}
	else
	{
		size = (uint16)Scripts.size();
		f.serial(size);	
		for (TScripts::iterator it = Scripts.begin(); it != Scripts.end(); ++it)
		{
			//std::string tmp = it->first;			
			nlWrite(f, serial, it->first);
			nlWrite(f, serialCont, it->second);			
		}
	}
}

void CCustomLootTable::serial(NLMISC::IStream &f)
{
	f.serial(LootSets);
	f.serial(MoneyFactor);
	f.serial(MoneyProba);
	f.serial(MoneyBase);
}

void CCustomLootTableManager::serial(NLMISC::IStream &f)
{
	uint16 size;
	if (f.isReading())
	{	
		Tables.clear();
		f.serial(size);

		uint32 i = 0;
		for (; i < size; ++i)
		{
			//std::string tmpKey;
			CCustomElementId tmpKey;
			CCustomLootTable tmpVal;
			f.serial(tmpKey);
			f.serial(tmpVal);
			Tables.insert(make_pair(tmpKey,tmpVal));
		}
	}
	else
	{
		size = (uint16)Tables.size();
		f.serial(size);	
		for (TCustomLootTable::iterator it = Tables.begin(); it != Tables.end(); ++it)
		{
			nlWrite(f, serial, it->first);
			nlWrite(f, serial, it->second);
		}
	}
}


//---------------------------------------------------------------------------------------
// Control over verbose nature of logging
//---------------------------------------------------------------------------------------

NLMISC_COMMAND(verboseNpcDescriptionMsgLog,"Turn on or off or check the state of verbose AI->EGS NPC description message logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			VerboseNpcDescriptionMsgLog=true;

		if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			VerboseNpcDescriptionMsgLog=false;
	}

	nlinfo("verbose Logging is %s",VerboseNpcDescriptionMsgLog?"ON":"OFF");
	return true;
}
