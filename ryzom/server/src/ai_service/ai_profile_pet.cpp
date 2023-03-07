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
#include "ai_profile_pet.h"

#include "ai_bot_pet.h"			// for CSpawnBotPet
#include "ai_grp_pet.h"			// for CSpawnGroupPet
#include "server_share/animal_hunger.h"	// for CSpeedLimit

using namespace NLMISC;
using namespace NLNET;
using namespace RYAI_MAP_CRUNCH;
using namespace	AITYPES;

/****************************************************************************/
/* Local classes                                                            */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CTopoPosValidator                                                        //
//////////////////////////////////////////////////////////////////////////////

class CTopoPosValidator
: public CWorldContainer::CPosValidator
{
public:
	CTopoPosValidator(CWorldPosition const& startPos, TAStarFlag denyFlags);
	bool check(CWorldPosition const& wpos) const;
	
private:
	CWorldPosition _StartPos;
	TAStarFlag     _DenyFlags;
};

/****************************************************************************/
/* Methods definitions                                                      */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CAIPetProfileStand                                                       //
//////////////////////////////////////////////////////////////////////////////

CAIPetProfileStand::CAIPetProfileStand(CSpawnBotPet* bot)
: CAIBaseProfile()
, _Bot(bot)
{
#ifndef NL_DEBUG
	nlassert(bot);
#endif
}

//////////////////////////////////////////////////////////////////////////////
// CAIPetProfileFollowPlayer                                                //
//////////////////////////////////////////////////////////////////////////////

CAIPetProfileFollowPlayer::CAIPetProfileFollowPlayer(CSpawnBotPet* bot, TDataSetRow const& playerRow)
: CAIBaseProfile()
, _Bot(bot)
, _PlayerRow(playerRow)
{
}

void CAIPetProfileFollowPlayer::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(PetFollowPlayer);
	
	// Is the pet stucked by something?
	if (!_Bot->canMove())
		return;
	
	// Need to wait for a correct position before moving?
	CAIVector const& dest = _Bot->spawnGrp().getPathCont().getDestination();
	if (dest.x()==0 || dest.y()==0)
		return;
	
	CPathCont& pathCont = _Bot->spawnGrp().getPathCont();
	if ((pathCont.getDestination()-_Bot->wpos().toAIVector()).quickNorm()>6.f) // follow only if > 6 meters.
	{
		// Handle the hunger of the animal
		CSpeedLimit speedLimit( TheDataset, _Bot->dataSetRow() );
		float speedToUse = speedLimit.getSpeedLimit( _Bot->walkSpeed(), _Bot->runSpeed() );
		
		// Move
		float const dist = speedToUse * ticksSinceLastUpdate;
		CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
			_Bot,
			_Bot->pathPos(),
			pathCont,
			dist,
			0,
			.5f);
		if (status==CFollowPath::FOLLOW_NO_PATH)
		{
			nlwarning("problem with pet following behavior and ground properties like (Water, Nogo)");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CAIPetProfileGotoPoint                                                   //
//////////////////////////////////////////////////////////////////////////////

CAIPetProfileGotoPoint::CAIPetProfileGotoPoint(CSpawnBotPet* bot, CAIPos const& position, TAStarFlag denyFlags, bool despawn)
: CAIBaseProfile()
, _Bot(bot)
, _Pos(position)
, _Despawn(despawn)
, _Valid(false)
, _PathCont(denyFlags)
{
#ifndef NL_DEBUG
	nlassert(bot);
#endif
	CTopoPosValidator const posValidator(bot->wpos(), denyFlags);
	
	CWorldPosition gotoPos;
	if (!CWorldContainer::calcNearestWPosFromPosAnRadius(vp_auto, gotoPos, _Pos, 16, 300, posValidator))
	{
#ifdef NL_DEBUG
		nldebug("position problem CAIPetProfileGotoPoint");
#endif
		return;
	}
	_Pos.setXY(gotoPos.toAIVector());
	_PathCont.setDestination((TVerticalPos)position.h(), _Pos);
	_Valid = true;
}

void CAIPetProfileGotoPoint::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(PetGotoPoint);
	if (!_Bot->canMove())
		return;
	
	CAIVector botPos = _Bot->wpos().toAIVector();
	float dist = _Bot->runSpeed() * ticksSinceLastUpdate;
	
	if ((_PathCont.getDestination()-botPos).quickNorm()>3.f) // follow only if > 6 meters.
	{
		CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
			_Bot,
			_Bot->pathPos(),
			_PathCont,
			dist,
			0.f,
			.5f);
		if (status==CFollowPath::FOLLOW_NO_PATH)
		{
			nlwarning("PetGotoPoint problem with destination properties (Water, Nogo)");
		}
	}
	botPos -= _Bot->wpos().toAIVector();
	
	if (botPos.quickNorm()<(dist*0.1))
	{
		if (_Despawn)
		{
			_Bot->getPersistent().setDespawn();
			return;
		}
		_Bot->setAIProfile(new CAIPetProfileStand(_Bot));
		return;
	}
}

TProfiles CAIPetProfileGotoPoint::getAIProfileType() const
{
	if (_Despawn)
		return PET_GOTO_AND_DESPAWN;
	else
		return PET_GOTO;
}

//////////////////////////////////////////////////////////////////////////////
// CTopoPosValidator                                                        //
//////////////////////////////////////////////////////////////////////////////

CTopoPosValidator::CTopoPosValidator(CWorldPosition const& startPos, TAStarFlag denyFlags)
: _StartPos(startPos)
, _DenyFlags(denyFlags)
{
}

bool CTopoPosValidator::check(CWorldPosition const& wpos) const
{
	CCompatibleResult res;
	areCompatiblesWithoutStartRestriction(_StartPos, wpos, _DenyFlags, res, true);
	return res.isValid();
}
