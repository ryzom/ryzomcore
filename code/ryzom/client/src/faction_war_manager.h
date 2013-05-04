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



#ifndef CL_FACTION_WAR_MANAGER_H
#define CL_FACTION_WAR_MANAGER_H

/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"
#include "game_share/pvp_clan.h"


///////////
// CLASS //
///////////
/**
 * All active faction wars
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2005
 */
class CFactionWarManager
{
public:
	/// singleton init
	static void init();
	/// singleton release
	static void release();
	/// instance accessor
	static CFactionWarManager * getInstance();

	/// add a faction war
	void addFactionWar(PVP_CLAN::CFactionWar fw);
	/// stop a faction war
	void stopFactionWar(PVP_CLAN::CFactionWar fw);

	/// true if there's a conflict between the two factions
	bool areFactionsInWar(PVP_CLAN::TPVPClan clan1, PVP_CLAN::TPVPClan clan2);

private:
	/// ctor
	CFactionWarManager(){}
	/// dtor
	~CFactionWarManager();

	/// unique instance
	static CFactionWarManager * _Instance;

	/// faction wars
	std::vector< PVP_CLAN::CFactionWar >	_FactionWarOccurs;

};

#endif // CL_FACTION_WAR_MANAGER_H

/* End of faction_war_manager.h */
