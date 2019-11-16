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

#ifndef RY_PVP_CLAN_H
#define RY_PVP_CLAN_H

#include <string>
#include "nel/misc/sheet_id.h"
#include "people_pd.h"

namespace PVP_CLAN
{
	// If you change this enum do not forget to update database.xml and local_database.xml
	enum TPVPClan
	{
		None,
		Neutral,

		BeginClans, // begin of "real" clans (None and Neutral are excluded)
		BeginCults = BeginClans, // begin the cults section

			Kami = BeginClans,
			Karavan,

		EndCults = Karavan, // end of cults
		BeginCivs, // begin of civilizations

			Fyros = BeginCivs,
			Matis,
			Tryker,
			Zorai,

		EndCivs = Zorai, // end of civs
		EndClans = Zorai, // end of clans

		Unknown,
		NbClans = Unknown,
		NbBits = 4 // number of bits needed to store all valid values (all but Unknown)
	};

	TPVPClan fromString(const std::string & str);
	const std::string & toString(TPVPClan clan);
	std::string toLowerString(TPVPClan clan);

	uint32 getFactionIndex(TPVPClan clan);
	TPVPClan getClanFromIndex(uint32 theIndex);
	NLMISC::CSheetId getFactionSheetId(TPVPClan clan);
	TPVPClan getClanFromPeople( EGSPD::CPeople::TPeople people );

	/// For Client Interface, return the define name of the type (eg: "pvp_faction_icon_Kami")
	std::string toIconDefineString( TPVPClan c );


	/**
 	 *	CFactionWar
	 */
	struct CFactionWar
	{
		TPVPClan Clan1;
		TPVPClan Clan2;

		CFactionWar()
		{
			Clan1 = None;
			Clan2 = None;
		}

		void setFactionWar( TPVPClan clan1, TPVPClan clan2 )
		{
			if( clan1 != clan2 )
			{
				if( clan1 < BeginClans || clan1 > EndClans )
				{
					nlwarning("<CFactionWar::setFactionWar> clan1 is invalid : %d", clan1 );
					return;
				}
				if( clan2 < BeginClans || clan2 > EndClans )
				{
					nlwarning("<CFactionWar::setFactionWar> clan2 is invalid : %d", clan2 );
					return;
				}
				Clan1 = clan1;
				Clan2 = clan2;
			}
			else
			{
				nlwarning("<CFactionWar::setFactionWar> A clan can't be in war with him");
				return;
			}
		}

		bool inPvPFaction( TPVPClan clan1, TPVPClan clan2 ) const
		{
			return ( (clan1 == Clan1 && clan2 == Clan2) || (clan2 == Clan1 && clan1 == Clan2) );
		}

		void serial( NLMISC::IStream & f )
		{
			f.serialEnum( Clan1 );
			f.serialEnum( Clan2 );
		}
	};

} // namespace PVP_CLAN

#endif // RY_PVP_CLAN_H
