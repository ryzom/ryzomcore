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



/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
#include "faction_war_manager.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace std;


////////////
// METHOD //
////////////
CFactionWarManager * CFactionWarManager::_Instance = NULL;


//----------------------------------------------------------------------------
void CFactionWarManager::addFactionWar(PVP_CLAN::CFactionWar fw)
{
	for( uint i=0; i<_FactionWarOccurs.size(); ++i )
	{
		if( _FactionWarOccurs[i].inPvPFaction(fw.Clan1, fw.Clan2) )
		{
			return;
		}
	}
	nldebug("<CFactionWarManager::addFactionWar> Adding faction war %s vs %s",PVP_CLAN::toString(fw.Clan1).c_str(),PVP_CLAN::toString(fw.Clan2).c_str());
	_FactionWarOccurs.push_back(fw);
}

//----------------------------------------------------------------------------
void CFactionWarManager::stopFactionWar(PVP_CLAN::CFactionWar fw)
{
	vector< PVP_CLAN::CFactionWar >::iterator it;
	for( it=_FactionWarOccurs.begin(); it!=_FactionWarOccurs.end(); ++it )
	{
		if( (*it).inPvPFaction(fw.Clan1, fw.Clan2) )
		{
			nldebug("<CFactionWarManager::stopFactionWar> Stopping faction war %s vs %s",PVP_CLAN::toString(fw.Clan1).c_str(),PVP_CLAN::toString(fw.Clan2).c_str());
			_FactionWarOccurs.erase(it);
			return;
		}
	}
}

//----------------------------------------------------------------------------
bool CFactionWarManager::areFactionsInWar(PVP_CLAN::TPVPClan clan1, PVP_CLAN::TPVPClan clan2)
{
	for( uint i=0; i<_FactionWarOccurs.size(); ++i )
	{
		if( _FactionWarOccurs[i].inPvPFaction(clan1, clan2) )
		{
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
void CFactionWarManager::init()
{
	if( _Instance != 0 )
		return;

	_Instance = new CFactionWarManager();
}

//----------------------------------------------------------------------------
CFactionWarManager * CFactionWarManager::getInstance()
{
	if(CFactionWarManager::_Instance == 0)
	{
		CFactionWarManager::init();
	}
	return CFactionWarManager::_Instance;
}

//----------------------------------------------------------------------------
void CFactionWarManager::release()
{
	if( _Instance != 0 )
	{
		delete _Instance;
		_Instance = 0;
	}
}

//----------------------------------------------------------------------------
CFactionWarManager::~CFactionWarManager()
{
	_FactionWarOccurs.clear();
}

