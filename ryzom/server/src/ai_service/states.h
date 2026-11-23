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



#ifndef RYAI_STATE_H
#define RYAI_STATE_H

#include "state_profil.h"
#include "state_chat.h"
#include "keyword_owner.h"
#include "event_reaction.h"
#include "profile_in_state.h"
#include "ai_share/ai_vector.h"
#include "ai_place_xyr.h"

class CStateMachine;

class CAIState : 
	public	NLMISC::CDbgRefCount<CAIState>,
	public	CKeyWordOwner, 
	public	CAliasChild<CStateMachine>, 
	public	NLMISC::CRefCount,
	public	CProfileInState
{
public:
	
	// ctor & dtor ------------------------------------------------------
	CAIState(const CStateMachine	*container,CAIAliasDescriptionNode *aliasTree)
		: CKeyWordOwner(), 
		CAliasChild<CStateMachine>(const_cast<CStateMachine*>(container),aliasTree)
	{}

	CAIState(CStateMachine const* container, uint32 alias, std::string const& name)
	: CKeyWordOwner()
	, CAliasChild<CStateMachine>(const_cast<CStateMachine*>(container), alias, name)
	{}
	
	virtual ~CAIState() NL_OVERRIDE
	{}

	std::string	getIndexString	()	const;

	virtual bool isPositional	()	const	=0;

	// Profile property accessors --------------------------------------

	const CAliasCont<CAIStateProfile>&	profiles() const
	{
		return	_Profiles;
	}

	const CAliasCont<CAIStateChat>&		chats() const
	{
		return	_Chats;
	}
	
	IAliasCont*			getAliasCont		(AITYPES::TAIType	type) NL_OVERRIDE;
	CAliasTreeOwner*	createChild			(IAliasCont	*cont,	CAIAliasDescriptionNode	*aliasTree) NL_OVERRIDE;
	void				updateDependencies	(const	CAIAliasDescriptionNode	&aliasTree, CAliasTreeOwner *aliasTreeOwner) NL_OVERRIDE;

protected:
	CAliasCont<CAIStateProfile>	_Profiles;
	CAliasCont<CAIStateChat>	_Chats;
};

class CAIStatePunctual: public CAIState  
{
public:
	virtual	bool	isPositional	()	const NL_OVERRIDE
	{
		return false;
	}
	
	CAIStatePunctual(const CStateMachine	*container,CAIAliasDescriptionNode *aliasTree)
		: CAIState(container,aliasTree) 
	{}

private:
};

class CShape
:public CPlaceRandomPos
{
public:
	CShape(bool acceptInvalidPos=false)
		: _GeometryType(NONE)
		, _AcceptInvalidPos(acceptInvalidPos)
	{}
	virtual	~CShape()
	{}
	
	typedef	RYAI_MAP_CRUNCH::CWorldPosition	TPosition;
	
	// clang-format off
	// methods for managing geometry
	bool	setPath(AITYPES::TVerticalPos verticalPos, const std::vector <CAIVector> &points);
	
	bool	setPatat(AITYPES::TVerticalPos verticalPos, const std::vector <CAIVector> &points);

	/// The heights in millimetres of the points the next setPath or setPatat gets, one for each point: the shape then
	/// puts its points and its random positions on the layer nearest to them (the states of a Ring scenario)
	void	setNextHeights(const std::vector<sint32> &heights) { m_NextHeights = heights; }
	/// The name of the state the shape belongs to, for its warnings
	void	setOwnerName(const std::string &name) { m_OwnerName = name; }
	
	// dumb little geometry type reporting accessors
	bool	hasPath		()	const	{ return _GeometryType==PATH && hasPoints(); } 
	bool	hasPatat	()	const	{ return _GeometryType==PATAT && hasPoints(); } 
	bool	hasPoints	()	const	{ return _Geometry.size()!=0; } 
	
	// accessors for path & patat geometry - whichever of the 2 exists
	uint32 numPoints()						{ return (uint32)_Geometry.size(); }
	const TPosition *point(uint32 idx)		{ return idx<_Geometry.size()? &_Geometry[idx]: NULL; }
	
	const std::vector<TPosition>	&getGeometry()	const { return	_Geometry; }
	
	bool	contains	(const	CAIVector	&pos)	const;

	bool	calcRandomPos(CAIPos &pos)	const NL_OVERRIDE;	
	bool	placePos(RYAI_MAP_CRUNCH::CWorldPosition &wpos, const CAIVector &pos) const NL_OVERRIDE;
	
protected:
private:
	enum { NONE, PATH, PATAT  }	_GeometryType;
	std::vector<TPosition>		_Geometry;
	
	void	takeHeights(const std::vector<CAIVector> &points);
	/// warns of each point whose height no layer at its place is near
	void	warnHeightsOffLayers() const;
	bool	placePoint(RYAI_MAP_CRUNCH::CWorldPosition &wpos, uint32 index, const CAIVector &pos, float radius, uint32 tries) const;
	/// the height at pos from those of the points, and the distance in metres to the nearest point
	sint32	heightAt(const CAIVector &pos, double &distance) const;

	/// the name of the state for the warnings
	const char *ownerName() const { return m_OwnerName.empty() ? "(no state)" : m_OwnerName.c_str(); }

	/// the heights for the next geometry, and those of the points of this one (empty without heights)
	std::vector<sint32>		m_NextHeights;
	std::vector<sint32>		m_Heights;
	std::vector<CAIVector>	m_HeightPoints;
	std::string				m_OwnerName;

	/// bouding box info
	CAIVector	_VMin, _VMax;
	bool	_AcceptInvalidPos;
	// clang-format on
};

class CAIStatePositional
:public	NLMISC::CDbgRefCount<const CAIStatePositional>
,public	NLMISC::CDbgRefCount<CAIStatePositional>
,public CAIState
{
public:
	CAIStatePositional (const CStateMachine	*container,CAIAliasDescriptionNode *aliasTree) 
	: CAIState(container,aliasTree)
	{
	}
	CAIStatePositional(CStateMachine const* container, uint32 alias, std::string const& name) 
	: CAIState(container, alias, name)
	{
	}
	
	virtual	bool	isPositional	()	const NL_OVERRIDE
	{
		return true;
	}
	const std::vector<CShape::TPosition>	&getGeometry()	const
	{
		return	_Shape.getGeometry();
	}
	bool	contains	(const	CAIVector	&pos)	const
	{
		return	_Shape.contains(pos);
	}

	CShape	&shape()
	{
		return	_Shape;
	}

	const	CShape	&shape()	const
	{
		return	_Shape;
	}
	
private:
	CShape	_Shape;
};

#endif

