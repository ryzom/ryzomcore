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



//class CAIPlace;

#ifndef RYAI_PLACE_H
#define RYAI_PLACE_H

#include "nel/misc/variable.h"
#include "ai_route.h"
#include "ai_share/world_map.h"
#include "world_container.h"
#include "ai.h"

class CPlaceOwner;

// come from config file
//extern uint32 RandomPosMaxRetry;
extern NLMISC::CVariable<uint32>	RandomPosMaxRetry;
//extern bool	LogAcceptablePos;
extern NLMISC::CVariable<bool>	LogAcceptablePos;

const uint32	MAX_PLACE_RANDOM_POS = 64;

/** Base class for random generated pos inside a place.
 *	NB : this class as no idea of the place geometry it 
 *	only store random valid position.
 */
class	CPlaceRandomPos
		:public	NLMISC::CDbgRefCount<CPlaceRandomPos>
{
public:
	
	CPlaceRandomPos()
	{
		_MaxPlacesFound = 0;
		_VerticalPos = AITYPES::vp_auto;
	}

	void setVerticalPos(AITYPES::TVerticalPos verticalPos)
	{
		_VerticalPos = verticalPos;
	}

	AITYPES::TVerticalPos	getVerticalPos() const
	{
		return _VerticalPos;
	}


	uint	getRandomPosCount() const
	{
		return _MaxPlacesFound;
	}
	
	// return a random position within the place
	void	getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition &pos) const
	{
#ifdef	NL_DEBUG
		nlassert(_MaxPlacesFound>0);
#endif
		pos=_PosList[CAIS::rand16(_MaxPlacesFound)];
	}

	void buildRandomPos(const RYAI_MAP_CRUNCH::CWorldPosition &validPos, float radius)
	{
		_MaxPlacesFound = 0;

		RYAI_MAP_CRUNCH::CWorldPosition	worldValidPos;

		if	(!CWorldContainer::calcNearestWPosFromPosAnRadius	(_VerticalPos, worldValidPos, validPos, radius, 1000, CWorldContainer::CPosValidatorDefault()))
		{
			if (LogAcceptablePos)
				nlwarning("Unvalid place (no collision free position found) at %d %d", validPos.toAIVector().x().asInt(), validPos.toAIVector().y().asInt());
		}

		uint	maxTries= RandomPosMaxRetry;
		nlassert(MAX_PLACE_RANDOM_POS>0);
		_MaxPlacesFound = 0;
		_PosList[_MaxPlacesFound] = worldValidPos;
		_MaxPlacesFound++;
		
		while (_MaxPlacesFound<MAX_PLACE_RANDOM_POS)
		{
			CAIPos	dummyPos;
			bool	foundRandomPos=false;
			
			while (!foundRandomPos)
			{
				if (calcRandomPos(dummyPos))
					foundRandomPos=CWorldContainer::getWorldMap().setWorldPosition(_VerticalPos, _PosList[_MaxPlacesFound], dummyPos);
				maxTries--;
				if (maxTries<=0)
					break;
			}
			if (maxTries<=0)
				break;
			_MaxPlacesFound++;
		}
	}

	// virtual to overload to generate a random pos inside the place.
	virtual bool	calcRandomPos(CAIPos &pos)	const =0;

protected:
	AITYPES::TVerticalPos			_VerticalPos;
private:
	RYAI_MAP_CRUNCH::CWorldPosition	_PosList[MAX_PLACE_RANDOM_POS];
	uint32							_MaxPlacesFound;
};


/**	This virtual base class represents a place to which AIs may go and 
 *	contains route generator for pathfinding
*/
class CAIPlace:
	public NLMISC::CDbgRefCount<CAIPlace>,
	public CAliasChild<CPlaceOwner>,
	public virtual	NLMISC::CVirtualRefCount
{
public:
	// ctor & dtor
	CAIPlace(CPlaceOwner*	owner,	CAIAliasDescriptionNode *aliasDescription)	
		: CAliasChild<CPlaceOwner>(owner,aliasDescription)
	{
	}

	virtual ~CAIPlace()
	{
	}

	std::string getIndexString() const;

	/// @name Abstract interface
	//@{
	/// Test whether a position is 'at the place'
	virtual bool atPlace(const CAIVector &pos) const = 0;
	/// Test whether a position is 'at the place'
	virtual bool atPlace(const CAIVectorMirror &pos) const = 0;
	/// Test whether an entity is 'at the place'
	virtual bool atPlace(CAIEntityPhysical const* entity) const = 0;
	/// Return the position of the middle of the place
	virtual const CAIPos &midPos() const = 0;
	
	/// Return a valid world position in the place.
	virtual const RYAI_MAP_CRUNCH::CWorldPosition &worldValidPos() const = 0;
	
	/// Return the outer radius of the place
	virtual float getRadius() const = 0;
	
	virtual void display(CStringWriter	&stringWriter) const = 0;
	
	virtual AITYPES::TVerticalPos	getVerticalPos() const = 0;
	virtual void getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition &pos) const = 0;
	//@}

	virtual std::string getOneLineInfoString() const { return ""; }
};


#endif
