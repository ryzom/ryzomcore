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
#include "server_share/r2_variables.h"
#include "ai_mgr.h"
#include "ai_entity_physical.h"
#include "ai_entity_physical_inline.h"
#include "ai_instance.h"


//	critical routine, get backward of weak technical design legacy.

const	float	repulsCoef=1.f/6.f;	// arbitrary

int CAIEntityPhysical::_PlayerVisibilityDistance = 64;
bool CAIEntityPhysical::havePlayersAround() const
{
  // todo Sadge: fixme! The correct fix is to handle the vision count mirror field in GPMS
  // until then, ring shard swill do much more work than required moving NPCs about
  if (IsRingShard)
  {
#ifdef NL_DEBUG
	nlwarning("FIXME: Quick hack to work round drunken NPC problem");
#endif
	return true;
  }

  TYPE_VISION_COUNTER n = 255-currentVisionCounter();
  ++AISStat::VisionCtr[n];
  if (n==255)
    return false;

  return n <= _PlayerVisibilityDistance;
}

template <> CKnapsackSolver::Algorithm CTargetable<CAIEntityPhysical>::_TargeterChoiceAlgorithm = CKnapsackSolver::FastSingleReplace;
template <> float const CTargetable<CAIEntityPhysical>::_DefaultFightTargetersWeightMax = 6.f;
template <> float const CTargetable<CAIEntityPhysical>::_DefaultFightWeight = .98f;
template <> float const CTargetable<CAIEntityPhysical>::_DefaultFightValue = 1.f;

CAIEntityPhysicalLocator* CAIEntityPhysicalLocator::_Instance = NULL;
CAIEntityPhysicalLocator* CAIEntityPhysicalLocator::getInstance()
{
	if (!_Instance)
		_Instance = new CAIEntityPhysicalLocator();
	return _Instance;
}

CAIEntityPhysical* CAIEntityPhysicalLocator::getEntity(TDataSetRow const& row) const
{
	std::map<TDataSetRow, CAIEntityPhysical*>::const_iterator it = _EntitiesByRow.find(row);
	if (it!=_EntitiesByRow.end())
		return it->second;
	else
		return NULL;
}

CAIEntityPhysical* CAIEntityPhysicalLocator::getEntity(NLMISC::CEntityId const& id) const
{
	std::map<NLMISC::CEntityId, CAIEntityPhysical*>::const_iterator it = _EntitiesById.find(id);
	if (it!=_EntitiesById.end())
		return it->second;
	else
		return NULL;
}

void CAIEntityPhysicalLocator::addEntity(TDataSetRow const& row, NLMISC::CEntityId const& id, CAIEntityPhysical* entity)
{
	_EntitiesByRow.insert(std::make_pair(row, entity));
	_EntitiesById.insert(std::make_pair(id, entity));
}

void CAIEntityPhysicalLocator::delEntity(TDataSetRow const& row, NLMISC::CEntityId const& id, CAIEntityPhysical* entity)
{
	_EntitiesById.erase(id);
	_EntitiesByRow.erase(row);
}

sint32 CAIEntityPhysical::getFameIndexed(uint32 factionIndex, bool modulated, bool returnUnknownValue) const
{
	return CFameInterface::getInstance().getFameIndexed(getEntityId(), factionIndex, modulated, returnUnknownValue);
}

sint32 CAIEntityPhysical::getFame(std::string const& faction, bool modulated, bool returnUnknownValue) const
{
	return CFameInterface::getInstance().getFame(getEntityId(), NLMISC::CStringMapper::map(faction), modulated, returnUnknownValue);
}

//////////////////////////////////////////////////////////////////////////////

CModEntityPhysical::CModEntityPhysical(CPersistentOfPhysical& owner, TDataSetRow const& entityIndex, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag const& AStarFlags)
: CAIEntityPhysical(owner, entityIndex, id, radius, level, AStarFlags)
, _Decalage(0, 0)
{
	setMode			(MBEHAV::NORMAL);
	setBehaviour	(MBEHAV::IDLE);
}

inline
CAIVector	CModEntityPhysical::calcRepulsionFrom(const CAIVector& pos, const std::vector<const CAIEntityPhysical*>& entities) const
{
	H_AUTO(CalcRepulsionFrom)

	const	double	thisDist=radius()+0.5;
	CAIVector	repulse;

	//	collide with entities
	FOREACHC(it, std::vector<const CAIEntityPhysical*>, entities)
	{
		const	CAIEntityPhysical*	entityPhysical = *it;
		nlassert(entityPhysical != NULL);

		CAIVector	deltaPos=pos;
		deltaPos	-=entityPhysical->pos();

		const	double	norm	=	deltaPos.quickNorm();
		const	double	cmpDist	=	thisDist+entityPhysical->radius();
		if	(norm<cmpDist)
		{
			double	coef;
			if	(norm<0.001f)
			{
				coef=1;
				deltaPos	=	CAIVector(0.1f,0);
			}
			else
			{
				coef=(cmpDist-norm)/norm;
			}
			repulse	+=	deltaPos*(coef*coef*repulsCoef);
		}
	}
	return	repulse;
}

inline
bool	CModEntityPhysical::calcStraightRepulsionFrom(const CAIVector& pos, const std::vector<const CAIEntityPhysical*>& entities, CAIVector& repulsion) const
{
	H_AUTO(CalcStraightRepulsionFrom)

	// init the repulsion to (0,0)
	repulsion.setXY(0.0, 0.0);

	const CAIVector	move		= pos - this->pos();
	const double	moveNorm	= move.quickNorm();
	const double	moveAngle	= move.asAngle().asRadians();
	const double	orientation	= theta().asRadians();
	
	// collide with entities
	FOREACHC(it, std::vector<const CAIEntityPhysical*>, entities)
	{
		const	CAIEntityPhysical*	entityPhysical = *it;
		nlassert(entityPhysical != NULL);

		CAIVector	deltaPos = pos;
		deltaPos	-= entityPhysical->pos();
		const double	norm	=	deltaPos.quickNorm();
		const double	cmpDist	=	radius() + entityPhysical->radius() + 0.5;

		// if the entity is colliding us
		if	(norm < cmpDist)
		{
			// first do a perpendicular repulsion
			deltaPos.normalize(float( 1000.0 * (cmpDist - norm) ));
			repulsion += deltaPos;

			// then add an oriented repulsion
			double	speedFactor = 1.0;
			double	repulsionAngle = deltaPos.asAngle().asRadians();
			double	angle = computeShortestAngle( moveAngle, (-deltaPos).asAngle().asRadians() );
			if (angle > -NLMISC::Pi/2 && angle < NLMISC::Pi/2)
			{
				const double	angle1 =  NLMISC::Pi/2 - angle;
				const double	angle2 = -NLMISC::Pi/2 - angle;
				const double	deviation1 = computeShortestAngle(orientation, repulsionAngle + angle1);
				const double	deviation2 = computeShortestAngle(orientation, repulsionAngle + angle2);
				if (fabs(deviation1) < fabs(deviation2))
				{
					angle = angle1;
				}
				else
				{
					angle = angle2;
				}
				speedFactor = 1.0 - fabs(angle)/NLMISC::Pi;
				repulsionAngle += angle;
				deltaPos = CAngle(repulsionAngle).asVector2d() * norm;
			}
			deltaPos.normalize(float( 1000.0 * speedFactor * (cmpDist - norm) ));
			repulsion += deltaPos;
		}
	}

	// now check that the repulsed position does not collide any entity
	const CAIVector repulsedPos = pos + repulsion;
	FOREACHC(it, std::vector<const CAIEntityPhysical*>, entities)
	{
		const	CAIEntityPhysical*	entityPhysical = *it;
		nlassert(entityPhysical != NULL);

		CAIVector	deltaPos = repulsedPos;
		deltaPos	-= entityPhysical->pos();

		const double	norm	=	deltaPos.quickNorm();
		const double	cmpDist	=	radius() + entityPhysical->radius();
		if	(norm < cmpDist)
		{
			return false;
		}
	}

	// ok the repulsion is successful
	return	true;
}

CAIVector	CModEntityPhysical::calcRepulsion(const	CAIPos&	pos)	const
{
	H_AUTO(CalcRepulsion)

	const	double	thisDist=radius()+0.5;
	const	double	botDist=(double)thisDist+5;		//	worth case scenario FOR BOTS.
	const	double	humanDist=(double)thisDist+2;	//	worth case scenario FOR HUMANS.

	std::vector<const CAIEntityPhysical*> nearbyEntities;
	{
		CAIEntityMatrix<CPersistentOfPhysical>::CEntityIteratorLinear it;

		//	get nearby bots
		for (it=getAIInstance()->botMatrix().beginEntities(CAIS::instance().bestLinearMatrixIteratorTbl((uint32) botDist),pos);!it.end();++it)
		{
			const	CAIEntityPhysical*	entityPhysical=(*it).getSpawnObj();
			if (	entityPhysical
				&&	entityPhysical->isAlive()
				&&	entityPhysical!=this)
			{
				nearbyEntities.push_back(entityPhysical);
			}
		}

		// get nearby players
		for (it=getAIInstance()->playerMatrix().beginEntities(CAIS::instance().bestLinearMatrixIteratorTbl((uint32) humanDist),pos);!it.end();++it)
		{
			const	CAIEntityPhysical*	entityPhysical=(*it).getSpawnObj();
			if (	entityPhysical
				&&	entityPhysical->isAlive()
				&&	entityPhysical!=this)
			{
				nearbyEntities.push_back(entityPhysical);
			}
		}
	}

	return	calcRepulsionFrom(pos, nearbyEntities);
}

bool	CModEntityPhysical::calcStraightRepulsion(CAIPos const& pos, CAIVector& repulsion)	const
{
	H_AUTO(CalcStraightRepulsion)

	const CAIVector	move		= pos - this->pos();
	const double	moveNorm	= move.quickNorm();
	const CAngle	moveAngle	= move.asAngle();

	const double	thisDist=radius()+0.5;
	const double	botDist=(double)thisDist+5;		//	worth case scenario FOR CREATURES.
	const double	humanDist=(double)thisDist+2;	//	worth case scenario FOR HUMANS.

	std::vector<const CAIEntityPhysical*> nonTraversableBots;
	std::vector<const CAIEntityPhysical*> nearbyEntities;
	{
		CAIEntityMatrix<CPersistentOfPhysical>::CEntityIteratorLinear it;

		// get nearby bots
		for (it=getAIInstance()->botMatrix().beginEntities(CAIS::instance().bestLinearMatrixIteratorTbl((uint32) botDist),pos);!it.end();++it)
		{
			const CBot*	bot = NLMISC::safe_cast<const CBot*>(&*it);
			const CAIEntityPhysical*	entityPhysical = bot->getSpawnObj();
			if (	entityPhysical
				&&	entityPhysical->isAlive()
				&&	entityPhysical!=this)
			{
				// if it is a bot object with a significant radius
				// TODO: kxu: add a sheet param to recognize these bots that MUST not be crossed by other bots
				if (	bot->getSheet()->NotTraversable()
					&&	entityPhysical->radius() > 1.f
					&&	entityPhysical->walkSpeed() == 0.f
					&&	entityPhysical->runSpeed() == 0.f)
				{
					nonTraversableBots.push_back(entityPhysical);
				}
				else
				{
					nearbyEntities.push_back(entityPhysical);
				}
			}
		}

		// get nearby players
		for (it=getAIInstance()->playerMatrix().beginEntities(CAIS::instance().bestLinearMatrixIteratorTbl((uint32) humanDist),pos);!it.end();++it)
		{
			const	CAIEntityPhysical*	entityPhysical=(*it).getSpawnObj();
			if (	entityPhysical
				&&	entityPhysical->isAlive()
				&&	entityPhysical!=this)
			{
				nearbyEntities.push_back(entityPhysical);
			}
		}
	}

	repulsion = calcRepulsionFrom(pos, nearbyEntities);
	// scale the repulsion
	// repulsion speed is 71% of movement speed
	repulsion.normalize(float( 710.0 * std::max(moveNorm, 0.025) ));
	if (nonTraversableBots.empty())
	{
		return true;
	}

	CAIVector	straightRepulsion;
	if (calcStraightRepulsionFrom(repulsion+pos, nonTraversableBots, straightRepulsion))
	{
		repulsion += straightRepulsion;
		return true;
	}

	// did not find a repulsion without collision
	return false;
}

void	CModEntityPhysical::setPos(const CAIPos &pos, const RYAI_MAP_CRUNCH::CWorldPosition &wpos)
{
	//	coherence test.
#ifdef NL_DEBUG
	nlassert(RYAI_MAP_CRUNCH::CMapPosition(pos)==wpos);
#endif
	
	_pos.setXY(pos);
	_wpos	=	wpos;		//		setWPos(wpos); coz no test.
	_pos.setH((sint32)(_wpos.getRootCell()?_wpos.getRootCell()->getMetricHeight(_wpos) : 0));
	
	// if we're not linked to the world map then nothing to do
	CPersistentOfPhysical	&persOfPhys=getPersistent();
	if	(persOfPhys.isLinkedToWorldMap())
	{
		CAIInstance* aii = getAIInstance();
		persOfPhys.linkEntityToMatrix(pos, aii->botMatrix());
	}
}

//////////////////////////////////////////////////////////////////////////////

NLMISC_COMMAND(targeterChoiceAlgorithm,"Set algorithm used to select targeters","[list|<algorithm name>]")
{
	if(args.size()>1)
		return false;
	
	if(args.size()==1)
	{
		if (args[0]=="list")
		{
			log.displayNL("Possible algorithms are:");
			CKnapsackSolver::Algorithm algorithms[] = {
				CKnapsackSolver::Optimal,
					//				CKnapsackSolver::FullAddCheck,
					//				CKnapsackSolver::AddCheck,
					CKnapsackSolver::FastAddCheck,
					//				CKnapsackSolver::FullSingleReplace,
					//				CKnapsackSolver::SingleReplace,
					CKnapsackSolver::FastSingleReplace,
					CKnapsackSolver::VeryFastSingleReplace
			};
			size_t algorithmCount = sizeof(algorithms)/sizeof(algorithms[0]);
			for (size_t i=0; i<algorithmCount; ++i)
				log.displayNL(" - %s", CKnapsackSolver::toString(algorithms[i]).c_str());
		}
		else
		{
			CKnapsackSolver::Algorithm algorithm = CKnapsackSolver::fromString(args[0]);
			if (algorithm!=CKnapsackSolver::UndefinedAlgorithm)
				CTargetable<CAIEntityPhysical>::_TargeterChoiceAlgorithm = algorithm;
		}
	}
	
	log.displayNL("targeterChoiceAlgorithm is %s", CKnapsackSolver::toString(CTargetable<CAIEntityPhysical>::_TargeterChoiceAlgorithm).c_str());
	return true;
}

//////////////////////////////////////////////////////////////////////////////

NLMISC_COMMAND(entityPlayerVisibilityDistance,"Set distance (0 to 255) at which entities consider they see a player","")
{
	if(args.size()>1)
		return false;
	
	if(args.size()==1)
	{
		sint n;
		NLMISC::fromString(args[0], n);
		if (n>=0 && n<=255)
			CAIEntityPhysical::_PlayerVisibilityDistance = n;
	}
	
	log.displayNL("entityPlayerVisibilityDistance is %d",CAIEntityPhysical::_PlayerVisibilityDistance);
	return true;
}
