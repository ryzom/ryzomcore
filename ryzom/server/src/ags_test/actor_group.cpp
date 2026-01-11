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



// Nel Misc
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/net/message.h"
#include "nel/net/unified_network.h"

// Game share
#include "game_share/tick_event_handler.h"
//#include "game_share/msg_brick_service.h"

// Local includes
#include "actor.h"
#include "actor_group.h"
#include "actor_manager.h"
#include "sheets.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

namespace AGS_TEST
{

//----------------------------------------------------------------------------------
void CActorGroup::removeActor(CActor *actor)
{
	if (this==0) return;

	for (int i=_actors.size();i--;)
		if (_actors[i]==actor)
		{
			_actors[i]=_actors[_actors.size()-1];
			_actors.pop_back();
		}
}

//----------------------------------------------------------------------------------
// displaying the actor's current state
void CActorGroup::display()
{
	if (this==0) return;

	nlinfo("");
	nlinfo("ActorGroup: %s",_name.c_str());

	// diaply the group's actors' stats
	for (int i=_actors.size();i--;)
		_actors[i]->display();
}

void CActorGroup::doFight(CActorGroup *target)
{
	if (this==0) return;
	if (target->actorCount()==0) return;

	// tell each actor in the group to fight the opposing group member with matching ID
	for (int i=_actors.size();i--;)
		_actors[i]->doFight((*target)[i%target->actorCount()]);
}

void CActorGroup::doFight(CActor *target)
{
	if (this==0) return;

	// tell all the actors in the group to attack the target chap
	for (int i=_actors.size();i--;)
		_actors[i]->doFight(target);
}

void CActorGroup::stopFight()
{
	if (this==0) return;

	for (int i=_actors.size();i--;)
		_actors[i]->stopFight();
}

//----------------------------------------------------------------------------------
// magnet state control
void	CActorGroup::fadeMagnet(const NLMISC::CVector &pos, float distance, float decay, NLMISC::TGameCycle endTime)
{
	_ToStartMagnet = _Magnet;
	_ToStartMagnetDistance = _MagnetDistance;
	_ToStartMagnetDecay = _MagnetDecay;
	_ToStopMagnet = pos;
	_ToStopMagnetDistance = distance;
	_ToStopMagnetDecay= decay;
	_ToStartCycle = CTickEventHandler::getGameCycle();
	_ToStopCycle = endTime;
	_ToFade = true;
}


//----------------------------------------------------------------------------------
// overall actors update
void	CActorGroup::update()
{
	if (_ToFade)
	{
		TGameCycle	cycle = CTickEventHandler::getGameCycle();
		if (cycle > _ToStopCycle)
		{
			cycle = _ToStopCycle;
			_ToFade = false;
		}

		float	ratio = (float)(cycle-_ToStartCycle)/(float)(_ToStopCycle-_ToStartCycle);

		_Magnet = _ToStartMagnet*(1.0f-ratio) + _ToStopMagnet*ratio;
		_MagnetDistance = _ToStartMagnetDistance*(1.0f-ratio) + _ToStopMagnetDistance*ratio;
		_MagnetDecay = _ToStartMagnetDecay*(1.0f-ratio) + _ToStopMagnetDecay*ratio;
	}

	if (_MagnetActors)
	{
		uint	i;
		for (i=0; i<_actors.size(); ++i)
		{
			_actors[i]->setMagnetPos(_Magnet);
			_actors[i]->setMagnetRange(_MagnetDistance, _MagnetDecay);
		}
	}
}


} // end of namespace AGS_TEST
