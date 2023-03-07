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

#ifndef EGS_CHARACTER_RESPAWN_POINTS_H
#define EGS_CHARACTER_RESPAWN_POINTS_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
#include "game_share/persistent_data.h"
#include "game_share/continent.h"
#include "game_share/far_position.h"

//-----------------------------------------------------------------------------

class CCharacter;

/**
 * CCharacterRespawnPoints
 * This class contains code and data relative to the player respawn points.
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005
 */
class CCharacterRespawnPoints
{
	NL_INSTANCE_COUNTER_DECL(CCharacterRespawnPoints);

public:
	/// respawn point id
	/// WARNING: if you want to change this type check every dependencies in the whole code
	typedef uint16 TRespawnPoint;

	CCharacterRespawnPoints(CCharacter &c);

	DECLARE_PERSISTENCE_METHODS

	/// add a regular respawn point
	void addRespawnPoint(TRespawnPoint respawnPoint);

	/// add the default respawn point of the given continent
	void addDefaultRespawnPoint(CONTINENT::TContinent continent);

	/// set mission respawn points of the given continent
	/// these respawn points will be seen by the player when he dies in the given continent
	/// also they will be cleared when player will leave the given continent
	/// NOTE: the given respawn points can (or even should) be located in another continent than 'continent'
	/// WARNING: all the given respawn points must be located in the same continent
	void setMissionRespawnPoints(CONTINENT::TContinent continent, const std::vector<TRespawnPoint> & respawnPoints, bool hideOthers);

	/// set Ring adventure respawn point
	/// will been seen by player if he exist when player dies, other re-spawn point are not displayed in this case.
	/// in other case, normal behaviour operate and display mission/regular re-spawn point
	/// return true if ring adventure re-spawn point are changed
	bool setRingAdventureRespawnpoint(const CFarPosition &farPos);

	/// remove Ring adventure respawn point
	void clearRingRespawnpoint();

	/// return true and fill coordinate if r-spawn point is a ring adventure re-spawn point, else return false
	bool getRingAdventuresRespawnPoint( sint32 &x, sint32 &y ) const;

	/// get the respawn points that player can use in the given continent
	void getUsableRespawnPoints(CONTINENT::TContinent continent, std::vector<TRespawnPoint> & respawnPoints) const;

	/// Build a container with 
	CONTINENT::TRespawnPointCounters buildRingPoints() const;

	/// clear all respawn points (regular and mission)
	void clearRespawnPoints();

	/// called when client is ready
	/// WARNING: call this after player is registered in fame manager because of a dependency
	void cbClientReady();

	/// called when player continent changed
	void cbContinentChanged(CONTINENT::TContinent previousContinent);

	/// dump all respawn points of the player
	void dumpRespawnPoints(NLMISC::CLog & log) const;

	/// DO NOT USE THIS
	/// this old serial method is only here to load old save game
//	void legacyLoad(NLMISC::IStream & f);

	/// reset user db (resend all respawn points of the current continent to the client)
	void resetUserDb() const;

private:
	/// vector of mission respawn points
	class CMissionRespawnPoints : public std::vector<TRespawnPoint>
	{
	public:
		DECLARE_PERSISTENCE_METHODS_WITH_ARG(const CCharacter& c)

		/// ctor
		CMissionRespawnPoints()
		: _HideOthers(true)
		{
		}

		bool getHideOthers() const			{ return _HideOthers;		}
		void setHideOthers(bool hideOthers)	{ _HideOthers = hideOthers;	}

	private:
		/// if true these mission respawn points will hide the other respawn points
		bool _HideOthers;
	};

	/// get mission respawn points of the given continent
	/// \return false if continent has no event respawn point
	const CMissionRespawnPoints * getMissionRespawnPoints(CONTINENT::TContinent continent) const;

	/// return true if player can use the given regular respawn point in the given continent
	bool isUsableRegularRespawnPoint(CONTINENT::TContinent continent, TRespawnPoint respawnPoint) const;

	/// add the given respawn point to user db (send it to the client)
	void addRespawnPointToUserDb(TRespawnPoint respawnPoint) const;

private:
	/// the parent class
	CCharacter &_Char;

	/// regular respawn points validated by the player
	std::vector<TRespawnPoint> _RegularRespawnPoints;

	/// mission respawn points of the player by continent
	std::map<sint32, CMissionRespawnPoints> _MissionRespawnPointsByContinent;

	/// Ring adventure re-spawn point
	CFarPosition	_RingRespawnPoint;
};

#endif // EGS_CHARACTER_RESPAWN_POINTS_H
