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


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "char_info_extractor_factory.h"
#include "character_scan_job.h"
#include "character.h"


//-------------------------------------------------------------------------------------------------
// INFO_EXTRACTOR Implementations
//-------------------------------------------------------------------------------------------------

INFO_EXTRACTOR(Name,"The character name","name")
{
	job->charTblSetEntry("name",c->EntityBase._Name);
}

INFO_EXTRACTOR(Money,"Cash in hand","money")
{
	job->charTblSetEntry("money",NLMISC::toString(c->_Money));
}

INFO_EXTRACTOR(PlayerRoomAndPets,"The Id of the room and number of pets owned by the character","room,petCount")
{
	job->charTblSetEntry("room",NLMISC::toString(c->_PlayerRoom.Building));
	job->charTblSetEntry("petCount",NLMISC::toString(c->_PlayerPets.size()));
}

INFO_EXTRACTOR(PlayerRoomStats,"The stats of number of players who have access to each room type","")
{
	job->freqTblAddEntry("room",NLMISC::toString(c->_PlayerRoom.Building));
}

INFO_EXTRACTOR(RespawnPointCount,"Number of respawn points","respawnPointCount")
{
	job->charTblSetEntry("respawnPointCount",NLMISC::toString(c->RespawnPoints.RespawnPoints.size()));
}

INFO_EXTRACTOR(RespawnPointStats,"Stats on number of players who have access to each of the respawn points","")
{
	for (uint32 i=0;i<c->RespawnPoints.RespawnPoints.size();++i)
	{
		job->freqTblAddEntry("RespawnPoints",c->RespawnPoints.RespawnPoints[i]);
	}
}

INFO_EXTRACTOR(StanzaStats,"Stats on number of players who know each stanza","")
{
	for (uint32 i=0;i<c->_KnownBricks.size();++i)
	{
		job->freqTblAddEntry("RespawnPoints",c->_KnownBricks[i].toString());
	}
}

INFO_EXTRACTOR(VisPropStats,"Stats on race and visual props","")
{
	job->freqTblAddEntry("race_sex",c->EntityBase._Race+(c->EntityBase._Gender==0?"_MALE":"_FEMALE"));
	job->freqTblAddEntry("hair",c->EntityBase._Race+(c->EntityBase._Gender==0?"_MALE_":"_FEMALE_")+NLMISC::toString(c->HairType));
	job->freqTblAddEntry("tattoo",c->EntityBase._Race+(c->EntityBase._Gender==0?"_MALE_":"_FEMALE_")+NLMISC::toString(c->Tattoo));
}
