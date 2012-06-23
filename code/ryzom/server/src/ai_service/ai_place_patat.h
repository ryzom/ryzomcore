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



#ifndef RYAI_PLACE_SHAPE_H
#define RYAI_PLACE_SHAPE_H

#include "nel/misc/types_nl.h"
#include "ai_place.h"
#include "states.h"

//#include "ai_place.h"
//#include "ai.h"
//#include "world_container.h"
//#include "nel/misc/variable.h"

// used to allow object to handle places.
/*
class CPlaceOwner
{
public:
	virtual std::string getFullName() const = 0;
	virtual	std::string	getIndexString() const = 0;
};
*/

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceShape                                                            //
//////////////////////////////////////////////////////////////////////////////

///	This class implements places defined by a shape
class CAIPlaceShape
: public CAIPlace
, public CPlaceRandomPos
{
public:
	CAIPlaceShape(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription = NULL, bool warnOnInvalidPosition=false);
	
	/// @name CAIPlace implementation
	//@{
	virtual bool atPlace(CAIVector const& pos) const;
	virtual bool atPlace(const CAIVectorMirror &pos) const;
	virtual bool atPlace(CAIEntityPhysical const* entity) const;
	virtual CAIPos const& midPos() const;
	virtual RYAI_MAP_CRUNCH::CWorldPosition const& worldValidPos() const;
	virtual float getRadius() const;
	virtual void display(CStringWriter& stringWriter) const { }
	virtual AITYPES::TVerticalPos getVerticalPos() const;
	virtual void getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const;
	//@}
	
	/// @name CPlaceRandomPos implementation
	//@{
	virtual bool calcRandomPos(CAIPos& pos) const;
	//@}
	
	bool setPatat(AITYPES::TVerticalPos verticalPos, std::vector<CAIVector> const& points);
	
private:
	RYAI_MAP_CRUNCH::CWorldPosition	_WorldValidPos;
	CAIPos	_MidPos;
	
	CShape	_Shape;
};

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceShape                                                            //
//////////////////////////////////////////////////////////////////////////////

///	This class implements places defined by an outpost (it takes into account delays)
class CAIPlaceOutpost
: public CAIPlaceShape
{
public:
	CAIPlaceOutpost(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription = NULL);
	
	/// @name CAIPlace implementation
	//@{
	virtual bool atPlace(CAIEntityPhysical const* entity) const;
	//@}

	void setOutpostAlias(uint32 outpostAlias);
	
private:
	uint32	_OutpostAlias;
};

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceIntersect                                                        //
//////////////////////////////////////////////////////////////////////////////

// :KLUDGE: Intersection is not respected for some methods (and anyway a NULL
// intersection is theorically possible, although empty place is not allowed
// by CAIPlace interface specification.
class CAIPlaceIntersect
: public CAIPlace
, public CPlaceRandomPos
{
public:
	CAIPlaceIntersect(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription = NULL);
	
	/// @name CAIPlace implementation
	//@{
	virtual bool atPlace(CAIVector const& pos) const;
	virtual bool atPlace(const CAIVectorMirror &pos) const;
	virtual bool atPlace(CAIEntityPhysical const* entity) const;
	virtual CAIPos const& midPos() const;
	virtual RYAI_MAP_CRUNCH::CWorldPosition const& worldValidPos() const;
	virtual float getRadius() const;
	virtual void display(CStringWriter& stringWriter) const { }
	virtual AITYPES::TVerticalPos getVerticalPos() const;
	virtual void getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const;
	//@}
	
	/// @name CPlaceRandomPos implementation
	//@{
	virtual bool calcRandomPos(CAIPos& pos) const;
	//@}
	
	void setPlace1(NLMISC::CSmartPtr<CAIPlace const> const& place);
	void setPlace2(NLMISC::CSmartPtr<CAIPlace const> const& place);
	
private:
	NLMISC::CSmartPtr<CAIPlace const>	_Place1;
	NLMISC::CSmartPtr<CAIPlace const>	_Place2;
	// :TODO: Make _DummyMidPos and _DummyValidPos static as soon as there's a cpp file for this class.
	/*static */CAIPos	_DummyMidPos;
	/*static */RYAI_MAP_CRUNCH::CWorldPosition	_DummyValidPos;
};

#endif
