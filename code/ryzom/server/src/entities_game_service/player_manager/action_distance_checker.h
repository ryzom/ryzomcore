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



#ifndef RY_ACTION_DISTANCE_CHECKER_H
#define RY_ACTION_DISTANCE_CHECKER_H

#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "game_share/constants.h"

/**
 * Singleton used to cancel actions ( or states ) that are valid only with a specified range
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CActionDistanceChecker
{
public:

	/// init the singleton
	static void init();

	/// release the singleton
	static void release();

	///\return the singleton instance
	static inline CActionDistanceChecker * getInstance();

	/**
	 * add a player to thi manager. Distance from player to target will be periodically checked
	 * \param player: the player to check
	 * \param target: target entity, used to compute the distance
	 */
	inline void addPlayer( const TDataSetRow& player, const TDataSetRow& target );

	/**
	 * remove a player from this manager
	 * \param player: player to remove
	 */
	inline void removePlayer( const TDataSetRow& player );

	/// update called at each tick
	inline void tickUpdate();

private:

	/// Constructor
	CActionDistanceChecker();
	
	/// struct representing a distance check
	struct SCheck
	{
		SCheck(const TDataSetRow& player, const TDataSetRow& target)
			:Player(player),Target(target){}
		TDataSetRow Player;
		TDataSetRow Target;
	};
	/// the checks to do
	std::vector< SCheck >		_Checks;

	/// id of the last check done
	uint						_LastCheck;
	
	/// max number of checks to perform each tick
	const uint					_NbCheckPerTick;

	/// instance of the singleton
	static CActionDistanceChecker* _Instance;
};



//---------------------------------------------------
// CActionDistanceChecker getInstance
//---------------------------------------------------
inline CActionDistanceChecker * CActionDistanceChecker::getInstance()
{
	nlassert(_Instance);
	return _Instance;
}// CActionDistanceChecker getInstance

//---------------------------------------------------
// CActionDistanceChecker getInstance
//---------------------------------------------------
inline void CActionDistanceChecker::addPlayer( const TDataSetRow& player, const TDataSetRow& target )
{
	// WARNING I assume there won't be a lot of this kind of actions, so a vector fits but we'll have to modify that if there are too muxh actions
	for (uint i = 0; i < _Checks.size(); i++ )
	{
		if ( _Checks[i].Player == player )
		{
			nlwarning("<CActionDistanceChecker addPlayer> player %d already added", player.getIndex());
			return;
		}
	}
	_Checks.push_back( SCheck(player,target) );
}// CActionDistanceChecker getInstance

//---------------------------------------------------
// CActionDistanceChecker removePlayer
//---------------------------------------------------
void CActionDistanceChecker::removePlayer( const TDataSetRow& player )
{
	// WARNING I assume there won't be a lot of this kind of actions, so a vector fits but we'll have to modify that if there are too many actions
	for (uint i = 0; i < _Checks.size(); i++ )
	{
		if ( _Checks[i].Player == player )
		{
			_Checks[i] = _Checks.back();
			_Checks.pop_back();
			return;
		}
	}
	nlwarning("<CActionDistanceChecker removePlayer> player %d not found", player.getIndex());
}// CActionDistanceChecker removePlayer

//---------------------------------------------------
// CActionDistanceChecker tickUpdate
//---------------------------------------------------
void CActionDistanceChecker::tickUpdate()
{
	H_AUTO(ActionDistanceCheckerUpdate);

	if ( _Checks.empty() )
		return;
	if ( _LastCheck >= _Checks.size() )
		_LastCheck = 0;
	
	// compute the higher check index that will be done this tick
	uint max = (uint)_Checks.size();
	if ( max > _LastCheck + _NbCheckPerTick )
		max = _LastCheck + _NbCheckPerTick;

	// iterate through the check that will be performed
	for ( uint i = _LastCheck; i < max; i++ )
	{
		// get player and target
		uint32 distSquare = ~0;
		CCharacter * player = PlayerManager.getChar( _Checks[i].Player );
		CEntityBase * entity = CEntityBaseManager::getEntityBasePtr( _Checks[i].Target );
		if ( player && entity )
			distSquare = (uint32) pow( ( player->getState().X - entity->getState().X )/1000.0 ,2 ) + (uint32)pow( ( player->getState().Y - entity->getState().Y )/1000.0,2);

		if ( distSquare < MaxTalkingDistSquare )
			i++;
		else
		{
			max--;
			if (player != NULL)
				player->cancelExchangeInvitation();
			_Checks[i] = _Checks.back();
			_Checks.pop_back();
			continue;
		}
	}
}// CActionDistanceChecker tickUpdate

#endif // RY_ACTION_DISTANCE_CHECKER_H

/* End of action_distance_checker.h */
