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




#ifndef GD_ACTOR_GROUP_H
#define GD_ACTOR_GROUP_H

// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

// Game share
#include "game_share/ryzom_entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/mode_and_behaviour.h"

namespace AGS_TEST
{
// external refs
class CActor;

// the class
class CActorGroup
{
public:
	// Constructor - must have a name for the group
	CActorGroup(const std::string &name): _name(name), _MagnetActors(false) {}

	// adding actors to the group
	void addActor(CActor *actor)		{ _actors.push_back(actor); }

	// Remove actor from the group (ignore if not found)
	void removeActor(CActor *actor);

	// some basic read accessors
	std::string getName()				{ return _name; }
	unsigned actorCount()				{ return (unsigned)_actors.size(); }
	CActor *operator[](unsigned index)	{ return _actors[index]; }

	// displaying the actor's current state
	void display();

	// changeing the actor's activity
	void doNothing();
	void doSquare();
	void doFight(CActor *target);
	void doFight(CActorGroup *target);
	void stopFight();

	//
	void	update();

	// magnet moving/attraction scripting setup
	void	magnetActors(bool control=true) { _MagnetActors = control; }
	void	setMagnet(const NLMISC::CVector &pos, float distance, float decay) { _Magnet = pos; _MagnetDistance = distance; _MagnetDecay = decay; _ToFade = false; }
	void	fadeMagnet(const NLMISC::CVector &pos, float distance, float decay, NLMISC::TGameCycle endTime);
	bool	fadeComplete() const { return !_ToFade; }
	const NLMISC::CVector &getMagnetPos() const { return _Magnet; }
	float	getMagnetDistance() const { return _MagnetDistance; }
	float	getMagnetDecay() const { return _MagnetDecay; }

private:
	CActorGroup();	// forbid default constructor

	std::string _name;
	std::vector<CActor *>	_actors;

	NLMISC::CVector			_Magnet;
	float					_MagnetDistance;
	float					_MagnetDecay;
	bool					_MagnetActors;

	NLMISC::CVector			_ToStartMagnet;
	float					_ToStartMagnetDistance;
	float					_ToStartMagnetDecay;
	NLMISC::CVector			_ToStopMagnet;
	float					_ToStopMagnetDistance;
	float					_ToStopMagnetDecay;
	NLMISC::TGameCycle		_ToStartCycle, _ToStopCycle;
	bool					_ToFade;
};


} // end of namespace AGS_TEST

#endif // GD_ACTOR_GROUP_H
/* End of actor_group.h */
