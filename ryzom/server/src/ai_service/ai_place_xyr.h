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



#ifndef RYAI_PLACE_XYR_H
#define RYAI_PLACE_XYR_H

#include "ai_place.h"
#include "ai.h"
#include "world_container.h"
#include "nel/misc/variable.h"

extern NLMISC::CVariable<bool>	LogAcceptablePos;


//	used to allow object to handle places.
class	CPlaceOwner
{
public:
	virtual std::string getFullName() const = 0;
	virtual	std::string	getIndexString()	const = 0;
};

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceXYR                                                              //
//////////////////////////////////////////////////////////////////////////////

/// This class implements places defined by a point and radius
class CAIPlaceXYR	
: public CAIPlace
, public CPlaceRandomPos
{
public:
	CAIPlaceXYR(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription = NULL);
	
	/// @name CAIPlace implementation
	//@{
	virtual bool atPlace(CAIVector const& pos) const;
	virtual bool atPlace(CAIVectorMirror const& pos) const;
	virtual bool atPlace(CAIEntityPhysical const* entity) const;
	virtual CAIPos const& midPos() const { return _pos; }
	
	virtual RYAI_MAP_CRUNCH::CWorldPosition const& worldValidPos() const { return _worldValidPos; }
	
	virtual float getRadius() const { return _radius; }
	
	virtual void display(CStringWriter& stringWriter) const;
	
	AITYPES::TVerticalPos getVerticalPos() const;
	void getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const;
	//@}
	
	/// radius in mm
	void setPosAndRadius(AITYPES::TVerticalPos verticalPos, CAIPos const& pos, uint32 radius);
	
	CAIPlaceXYR& operator =(CAIPlaceXYR const& other);	
	
	std::string getFullName() const { return std::string(getOwner()->getFullName() +":"+ getName()); }
	
private:
	bool calcRandomPos(CAIPos& pos) const;
	
	RYAI_MAP_CRUNCH::CWorldPosition _worldValidPos;
	CAIPos _pos;
	float _radius;
};

// base class for fauna places
class CFaunaGenericPlace
{
public:
	enum
	{
		INVALID_PLACE = INT_MAX
	};
	enum TFlag
	{
		FLAG_SPAWN = 0,
		FLAG_EAT,
		FLAG_REST,
		FLAG_COUNT
	};
	// ctor
	CFaunaGenericPlace();
	// activities flags
	void   setFlag(TFlag flag, bool on) { nlassert(flag < FLAG_COUNT); _Flags[flag] = on; }
	bool   getFlag(TFlag flag) const { nlassert(flag < FLAG_COUNT); return _Flags[flag]; }
	void   setMinStayTime(uint32 numTicks) { _MinStayTime = numTicks; }
	void   setMaxStayTime(uint32 numTicks) { _MaxStayTime = numTicks; }
	uint32 getMinStayTime() const { return _MinStayTime; }
	uint32 getMaxStayTime() const { return _MaxStayTime; }
	// index in graph
	void   setIndex(sint32 index) { _Index = index; }
	sint32 getIndex() const { return _Index; }
	void   setArcs(const std::vector<sint32> &arcs) { _Arcs = arcs; }
	const std::vector<sint32> &getArcs() const { return _Arcs; }
	void  setReachNext(bool enabled) { _ReachNext = enabled; }
	bool  getReachNext() const { return _ReachNext; }
	void  setActive(bool active) { nlassert(!_TimeDriven); _Active = active; }
	// See if that place is active in the fauna graph
	// If place is season driven then current time is tested against valid interval
	// to see if the place is activated.
	// Otherwise the 'active' field is returns
	bool  getActive() const;
	void  setTimeDriven(bool timeDriven) { _TimeDriven = timeDriven; }
	bool  getTimeDriven() const { return _TimeDriven; }
	//
	void  setDayInterval(const std::string &dayInterval) { _DayInterval = dayInterval; }
	const std::string &getDayInterval() const { return _DayInterval; }
	void  setTimeInterval(const std::string &TimeInterval) { _TimeInterval = TimeInterval; }
	const std::string &getTimeInterval() const { return _TimeInterval; }
	// backward compatibility : setup parameter from the zone name ("spawn, food, or rest")
	uint  setupFromOldName(const std::string &name);	
	//
private:	
	bool		_Active;
	bool		_TimeDriven;
	std::string _TimeInterval;
	std::string _DayInterval;
	bool	_Flags[FLAG_COUNT];
	uint32  _MinStayTime; // min stay time in this place (in ticks)
	uint32  _MaxStayTime; // man stay time in this place (in ticks)
	sint32  _Index;
	std::vector<sint32> _Arcs;
	bool				_ReachNext; // if arcs where not given, say if next greater index is reachable
};

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceXYRFauna                                                         //
//////////////////////////////////////////////////////////////////////////////
class CAIPlaceXYRFauna : public CAIPlaceXYR,
                         public CFaunaGenericPlace
{
public:	
	CAIPlaceXYRFauna(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription = NULL);		
	virtual std::string	getOneLineInfoString() const;	
	std::string getFullName() const { return std::string(getOwner()->getFullName() +":"+ getName()); }
};



//////////////////////////////////////////////////////////////////////////////
// CAIPlaceFastXYR                                                          //
//////////////////////////////////////////////////////////////////////////////

/// This class implements places defined by a point and radius
class CAIPlaceFastXYR	
: public CAIPlace
//, public CPlaceRandomPos
{
public:
	CAIPlaceFastXYR(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription = NULL);
	
	/// @name CAIPlace implementation
	//@{
	virtual bool atPlace(CAIVector const& pos) const;
	virtual bool atPlace(CAIVectorMirror const& pos) const;
	virtual bool atPlace(CAIEntityPhysical const* entity) const;
	virtual CAIPos const& midPos() const { return _Pos; }
	
	virtual RYAI_MAP_CRUNCH::CWorldPosition const& worldValidPos() const;
	
	virtual float getRadius() const { return _Radius; }
	
	virtual void display(CStringWriter& stringWriter) const;
	
	AITYPES::TVerticalPos getVerticalPos() const;
	void getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const;
	//@}
	
	/// radius in mm
	void setPosAndRadius(AITYPES::TVerticalPos verticalPos, CAIPos const& pos, uint32 radius);
	
	CAIPlaceFastXYR& operator =(CAIPlaceFastXYR const& other);
	
private:
	bool calcRandomPos(CAIPos& pos) const;
	
	CAIPos _Pos;
	AITYPES::TVerticalPos _VerticalPos;
	float _Radius;
	
	mutable bool _WorldPosInitialized;
	mutable RYAI_MAP_CRUNCH::CWorldPosition _WorldValidPos;
};

/****************************************************************************/
/* Inlined methods                                                          */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceXYR                                                              //
//////////////////////////////////////////////////////////////////////////////

inline
CAIPlaceXYR::CAIPlaceXYR(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription)
: CAIPlace(owner, aliasDescription)
{
}

inline
bool CAIPlaceXYR::atPlace(CAIVector const& pos) const
{
	return (pos-_pos).sqrnorm() <= ((double)_radius*(double)_radius);
}

inline
bool CAIPlaceXYR::atPlace(CAIVectorMirror const& pos) const
{
	return (pos-_pos).sqrnorm() <= ((double)_radius*(double)_radius);
}

inline
void CAIPlaceXYR::setPosAndRadius(AITYPES::TVerticalPos verticalPos, CAIPos const& pos, uint32 radius)
{
	_VerticalPos = verticalPos;
	_pos = pos;
	_radius = float(radius)/1000.0f;
#ifdef NL_DEBUG
	nlassert(_radius > 0);
	nlassert(pos.x()!=0||pos.y()!=0);
#endif
	if (pos.x()==0 && pos.y()==0)
	{
		nlwarning("Null place Position for %s", getAliasFullName().c_str());
	}
	
	if (!CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, _worldValidPos, _pos, _radius, 1000, CWorldContainer::CPosValidatorDefault()))
	{
		if (LogAcceptablePos)
			nlwarning("Unvalid place (no collision free position found) at %d %d ", _pos.x().asInt(), _pos.y().asInt());
	}
	
	buildRandomPos(_worldValidPos, _radius);
}

inline
CAIPlaceXYR& CAIPlaceXYR::operator =(CAIPlaceXYR const& other)
{
	_worldValidPos = other._worldValidPos;
	_pos = other._pos;
	_radius = other._radius;
	
	CPlaceRandomPos::operator =(other);
	return *this;
}

inline
AITYPES::TVerticalPos CAIPlaceXYR::getVerticalPos() const
{
	return CPlaceRandomPos::getVerticalPos();
}

inline
void CAIPlaceXYR::getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const
{
	CPlaceRandomPos::getRandomPos(pos);
}

inline
bool CAIPlaceXYR::calcRandomPos(CAIPos& pos) const
{
	double dx, dy;
	double const r = (double)_radius;
	double const rSquare = r * r;
	do
	{
		dx = CAIS::frandPlusMinus(r);
		dy = CAIS::frandPlusMinus(r);
	}
	while (dx*dx+dy*dy > rSquare);
	
	pos.setX(_pos.x()+dx);
	pos.setY(_pos.y()+dy);
	pos.setH(_pos.h());
	pos.setTheta(pos.angleTo(_pos)+CAngle(NLMISC::Pi/2));
	
	return true;
}


////////////////////////
// CFaunaGenericPlace //
////////////////////////
inline
CFaunaGenericPlace::CFaunaGenericPlace()
{

	_ReachNext = false;
	_Index = 0;
	_MinStayTime = _MaxStayTime = 100;
	std::fill(_Flags, _Flags + FLAG_COUNT, false);
	_Active = true;
	_TimeDriven = false;
}


//////////////////////////////////////////////////////////////////////////////
// CAIPlaceXYRFauna                                                         //
//////////////////////////////////////////////////////////////////////////////

inline
CAIPlaceXYRFauna::CAIPlaceXYRFauna(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription)
: CAIPlaceXYR(owner, aliasDescription)
{	
}


//////////////////////////////////////////////////////////////////////////////
// CAIPlaceFastXYR                                                          //
//////////////////////////////////////////////////////////////////////////////

inline
CAIPlaceFastXYR::CAIPlaceFastXYR(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription)
: CAIPlace(owner, aliasDescription)
{
}

inline
bool CAIPlaceFastXYR::atPlace(CAIVector const& pos) const
{
	return (float)((pos-_Pos).sqrnorm()) <= (_Radius*_Radius);
}

inline
bool CAIPlaceFastXYR::atPlace(CAIVectorMirror const& pos) const
{
	return (float)((pos-_Pos).sqrnorm()) <= ((double)_Radius*(double)_Radius);
}

inline
void CAIPlaceFastXYR::setPosAndRadius(AITYPES::TVerticalPos verticalPos, CAIPos const& pos, uint32 radius)
{
#ifdef NL_DEBUG
	nlassert(radius > 0);
	nlassert(pos.x()!=0||pos.y()!=0);
#endif
	_VerticalPos = verticalPos;
	_Pos = pos;
	_Radius = (float)radius / 1000.0f;
}

inline
CAIPlaceFastXYR& CAIPlaceFastXYR::operator =(CAIPlaceFastXYR const& other)
{
	_VerticalPos = other._VerticalPos;
	_Pos = other._Pos;
	_Radius = other._Radius;
	
	return *this;
}

inline
AITYPES::TVerticalPos CAIPlaceFastXYR::getVerticalPos() const
{
	return _VerticalPos;
}

inline
void CAIPlaceFastXYR::getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& wpos) const
{
	CAIPos pos;
	if (calcRandomPos(pos))
	{
		CWorldContainer::getWorldMap().setWorldPosition(_VerticalPos, wpos, pos);
	}
}

inline
bool CAIPlaceFastXYR::calcRandomPos(CAIPos& pos) const
{
	double dx, dy;
	double const r = (double)_Radius;
	double const rSquare = r * r;
	do
	{
		dx = CAIS::frandPlusMinus(r);
		dy = CAIS::frandPlusMinus(r);
	}
	while (dx*dx+dy*dy > rSquare);
	
	pos.setX(_Pos.x()+dx);
	pos.setY(_Pos.y()+dy);
	pos.setH(_Pos.h());
	pos.setTheta(pos.angleTo(_Pos)+CAngle(NLMISC::Pi/2));
	
	return true;
}

inline
RYAI_MAP_CRUNCH::CWorldPosition const& CAIPlaceFastXYR::worldValidPos() const
{
	if (!_WorldPosInitialized)
		_WorldPosInitialized = CWorldContainer::getWorldMap().setWorldPosition(_VerticalPos, _WorldValidPos, _Pos);
	return _WorldValidPos;
}

#endif
