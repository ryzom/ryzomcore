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

#ifndef RYAI_PATH_BEHAVIOR_H
#define RYAI_PATH_BEHAVIOR_H

#include "world_container.h"
#include "ai_entity_physical.h"
#include "ai_entity_physical_inline.h"
#include "ai_instance.h"
#include "time_interface.h"
#include "nel/misc/variable.h"

extern NLMISC::CVariable<bool>	LogAcceptablePos;

//////////////////////////////////////////////////////////////////////////////
// CAIPath                                                                  //
//////////////////////////////////////////////////////////////////////////////

class CAIPath : public NLMISC::CRefCount
{
public:
	/// @name Accessors
	std::vector<RYAI_MAP_CRUNCH::CTopology::TTopologyRef> const& topologiesPath() const { return _TopologiesPath; }
	std::vector<RYAI_MAP_CRUNCH::CTopology::TTopologyRef>& topologiesPathForCalc() { return _TopologiesPath; }
	
	RYAI_MAP_CRUNCH::CWorldPosition const& getStartPos() const { return _Start; }
	void setStartPos(RYAI_MAP_CRUNCH::CWorldPosition const& pos) { _Start = pos; }
	
	RYAI_MAP_CRUNCH::CWorldPosition const& getEndPos() const { return _End; }
	void setEndPos(RYAI_MAP_CRUNCH::CWorldPosition const& pos) { _End = pos; }
	//@}
	
private:
	RYAI_MAP_CRUNCH::CWorldPosition _Start;
	RYAI_MAP_CRUNCH::CWorldPosition _End;
	std::vector<RYAI_MAP_CRUNCH::CTopology::TTopologyRef> _TopologiesPath;
};

//////////////////////////////////////////////////////////////////////////////
// CInsidePath                                                              //
//////////////////////////////////////////////////////////////////////////////

class CInsidePath : public NLMISC::CRefCount
{
public:
	std::vector<RYAI_MAP_CRUNCH::CDirection> _DirectionPath;
};

//////////////////////////////////////////////////////////////////////////////
// CPathPosition                                                            //
//////////////////////////////////////////////////////////////////////////////

class CPathCont;

class CPathPosition
{
public:
	enum TPathPosState
	{
		NOT_INITIALIZED,
		FOLLOWING_TOPO,
		FOLLOWING_INSIDE_TOPO,
		REACH_END,
		NO_PATH
	};
	
	/// @name Constructor and destructor
	//@{
	CPathPosition(CAngle const& angle);
	virtual ~CPathPosition() { } // Defined coz no base class
	//@}
	
	/// @name Accessors
	//@{
	NLMISC::CSmartPtr<CAIPath> const& getPath() const { return _Path; }
	RYAI_MAP_CRUNCH::CTopology::TTopologyRef const& getTopology() const;
	RYAI_MAP_CRUNCH::CTopology::TTopologyRef const& getNextTopology() const;
	//@}
	
	bool isPathValid() const;
	bool isFinished() const;
	bool nextTopology();
	bool haveNextTopology(uint nbTopo = 1);
	
	NLMISC::CSmartPtr<CAIPath>		_Path;
	NLMISC::CSmartPtr<CInsidePath>	_InsidePath;
	RYAI_MAP_CRUNCH::CMapPosition	_InsideStartPos;
	RYAI_MAP_CRUNCH::CMapPosition	_InsideDestPos;
	uint			_InsideIndex;
	TPathPosState	_PathState;
	uint32			_TimeTopoChanged;
	uint32			_TimeDestChanged;
	CAngle			_Angle;
	
private:
	friend	class CPathCont;
	uint	_Index;
};

//////////////////////////////////////////////////////////////////////////////
// CPathCont                                                                //
//////////////////////////////////////////////////////////////////////////////

class CPathCont : public NLMISC::CDbgRefCount<CPathCont>
{
public:
	/// @name Constructors
	//@{
	CPathCont(RYAI_MAP_CRUNCH::TAStarFlag denyFlags);
	CPathCont(CPathCont const& pathCont);
	//@}
	
	/// @name Accessors
	//@{
	RYAI_MAP_CRUNCH::TAStarFlag const& denyFlags() const { return _denyFlags; }
	RYAI_MAP_CRUNCH::TAStarFlag const& getDenyFlags() const { return _denyFlags; }
	void setDenyFlags(RYAI_MAP_CRUNCH::TAStarFlag denyFlags) { _denyFlags = denyFlags; }
	
	CAIVector const& getDestination() const { return _Destination; }
	RYAI_MAP_CRUNCH::CWorldPosition const& getDestPos() const { return _DestPos; }
	void setDestination(RYAI_MAP_CRUNCH::CWorldPosition const& destPos);
	void setDestination(AITYPES::TVerticalPos verticalPos, CAIVector const& destPos);
	//@}
	
	void clearPaths();
	
	bool getPathForSource(CPathPosition& pathPos, RYAI_MAP_CRUNCH::CWorldPosition const& startPos) const;
	bool calcPathForSource(CPathPosition& pathPos, RYAI_MAP_CRUNCH::CWorldPosition const& startPos);
	bool getCalcPathForSource(CPathPosition& pathPos, RYAI_MAP_CRUNCH::CWorldPosition const& startPos);
	
	uint32 _TimeTopoChanged;
	uint32 _TimeDestChanged;
	
private:
	CAIVector						_Destination;
	RYAI_MAP_CRUNCH::CMapPosition	_DestinationMapPos;
	AITYPES::TVerticalPos			_VerticalPos;
	
	RYAI_MAP_CRUNCH::CTopology::TTopologyRef	_TopoRef;
	RYAI_MAP_CRUNCH::CWorldPosition				_DestPos;
	std::vector<NLMISC::CSmartPtr<CAIPath> >	_SourcePaths;
	
	RYAI_MAP_CRUNCH::TAStarFlag		_denyFlags;
};

//////////////////////////////////////////////////////////////////////////////
// CFollowPathContext                                                       //
//////////////////////////////////////////////////////////////////////////////

class CFollowPathContext
{
public:
	// ctor
	// - This method adds the new object to the top of the CFollowPath singleton's context stack
	// parameters:
	// - contextName	: an arbitrary string naming the context
	// - maxSearchDepth	: the value that the path finder search depth should be limitted to (default to std::numeric_limits<uint32>::max() meaning no limit)
	// - forceMaxDepth	: set this flag true to override previous limit with larger value
	// example:
	// - ... Before we begin ... CFollowPath::_MaxSearchDepth = std::numeric_limits<uint32>::max()
	// - CFollowPathContext context1("tata")			: CFollowPath::_MaxSearchDepth => std::numeric_limits<uint32>::max()
	// - CFollowPathContext context2("tete",456)		: CFollowPath::_MaxSearchDepth => 456
	// - CFollowPathContext context3("titi",123)		: CFollowPath::_MaxSearchDepth => 123
	// - CFollowPathContext context4("toto",456)		: CFollowPath::_MaxSearchDepth => 123
	// - CFollowPathContext context5("tutu")			: CFollowPath::_MaxSearchDepth => 123
	// - CFollowPathContext context6("dada",456,true)	: CFollowPath::_MaxSearchDepth => 456
	// - CFollowPathContext context6.~CFollowPathContext()	: CFollowPath::_MaxSearchDepth => 123
	// - CFollowPathContext context5.~CFollowPathContext()	: CFollowPath::_MaxSearchDepth => 123
	// - CFollowPathContext context4.~CFollowPathContext()	: CFollowPath::_MaxSearchDepth => 123
	// - CFollowPathContext context3.~CFollowPathContext()	: CFollowPath::_MaxSearchDepth => 456
	// - CFollowPathContext context2.~CFollowPathContext()	: CFollowPath::_MaxSearchDepth => std::numeric_limits<uint32>::max()
	// - CFollowPathContext context1.~CFollowPathContext()	: CFollowPath::_MaxSearchDepth => std::numeric_limits<uint32>::max()
	CFollowPathContext(const char* contextName, uint32 maxSearchDepth=std::numeric_limits<uint32>::max(), bool forceMaxDepth=false);

	// dtor
	// - This method removes the destroyed object from the CFollowPath singleton's context stack
	// - Out of order destruction is supported ok, but the _MaxSearchDepth values of higher stack entries are not recalculated
	~CFollowPathContext();

	// getter for _MaxSearchDepth
	uint32 getMaxSearchDepth() const { return _MaxSearchDepth; }

	// simple accessor for contructing the full context name of this object by concatenating own name with names of previous context stack entries
	void buildContextName(std::string &result) const;

private:
	CFollowPathContext* _PrevContext;
	CFollowPathContext* _NextContext;
	const char* _ContextName;
	uint32 _MaxSearchDepth;
};


//////////////////////////////////////////////////////////////////////////////
// CFollowPath                                                              //
//////////////////////////////////////////////////////////////////////////////

class CFollowPath
{
private:
	static CFollowPath* _Instance;
public:
	static CFollowPath* getInstance();
	static void destroyInstance();
	CFollowPath();

	enum TFollowStatus
	{
		FOLLOWING,		// normal
		FOLLOW_NO_PATH,	// no path found
		FOLLOW_ARRIVED,	// arrived
		FOLLOW_BLOCKED	// Is blocked by someone(s), cannot move.
	};
	enum TFollowReason
	{
		NO_REASON = -1,
		// FOLLOW_NO_PATH
		FNP_NOT_INITIALIZED,
		FNP_NO_INSIDE_ASTAR_PATH
	};
	
private:
	TFollowReason _LastReason;
	RYAI_MAP_CRUNCH::CWorldMap::TFindInsideAStarPathReason _LastFIASPReason;

public:
	static std::string toString(TFollowReason reason, RYAI_MAP_CRUNCH::CWorldMap::TFindInsideAStarPathReason fIASPReason = RYAI_MAP_CRUNCH::CWorldMap::FIASPR_NO_REASON)
	{
		switch (reason)
		{
		case FNP_NOT_INITIALIZED:
			return "FOLLOW_NO_PATH::NOT_INITIALIZED";
		case FNP_NO_INSIDE_ASTAR_PATH:
			return "FOLLOW_NO_PATH::NO_INSIDE_ASTAR_PATH::" + RYAI_MAP_CRUNCH::CWorldMap::toString(fIASPReason);
		default:
			return "UnknownReason(" + NLMISC::toString((int)reason) + ")";
		}
	}
	TFollowReason lastReason() { return _LastReason; }
	RYAI_MAP_CRUNCH::CWorldMap::TFindInsideAStarPathReason lastFIASPReason() { return _LastFIASPReason; }
	
public:
	NLMISC::CMustConsume<TFollowStatus>	followPath(
			CModEntityPhysical* bot,
			CPathPosition& pathPos,
			CPathCont& pathCont,
			float dist,
			float modecalageDist,
			float angleAmort = 1.f,
			bool focusOnTargetDirection = true,
			CAIVector* realDestination = NULL,
			float repulsSpeed = 0.025f,
			bool updateOrient = true);

private:
	friend class CFollowPathContext;
	CFollowPathContext* _TopFollowPathContext;
public:
	uint32 getMaxSearchDepth() const { return (_TopFollowPathContext==NULL)? std::numeric_limits<uint32>::max(): _TopFollowPathContext->getMaxSearchDepth(); }
	const char* getContextName() const;
};

#endif

