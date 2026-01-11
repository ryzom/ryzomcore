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
#include "path_behaviors.h"
#include "ais_actions.h"
#include "continent.h"
#include <typeinfo>

extern bool simulateBug(int bugId);

using namespace RYAI_MAP_CRUNCH;
using namespace CAISActionEnums;


NLMISC::CVariable<bool>	ActivateStraightRepulsion("ai", "ActivateStraightRepulsion", "Activate the straight repulsion for follow route (only available with Ring shards for moment).", true, 0, true);
NLMISC::CVariable<bool>	LogTimeConsumingAStar("ai", "LogTimeConsumingAStar", "Activate logging of time consuming path finding operations", false, 0, true);


//////////////////////////////////////////////////////////////////////////////
// Actions used when parsing primitives                                     //
//////////////////////////////////////////////////////////////////////////////

DEFINE_ACTION(ContextGlobal,SETNOGO)
{
	// get hold of the parameters and check their validity
	float x,y;
	if (!getArgs(args,name(),x,y))
		return;

	const	RYAI_MAP_CRUNCH::CMapPosition	pos(CAIVector	(x,y));

	const RYAI_MAP_CRUNCH::CWorldMap	&worldMap=CWorldContainer::getWorldMap(/*0*/);
	const CSuperCell	*superCell=worldMap.getSuperCellCst(pos);

	if (!superCell)
	{
		nlwarning("Unable to set flags at this position %.3f %.3f",x,y);
		return;
	}
}

DEFINE_ACTION(ContextGlobal,SAFEZONE)
{
	// get hold of the parameters and check their validity
	float x,y,r;
	
	if (!getArgs(args,name(),x,y,r))
		return;
	
	const	RYAI_MAP_CRUNCH::CMapPosition	pos(CAIVector	(x,y));
	CWorldContainer::getWorldMapNoCst().setFlagOnPosAndRadius(pos,r,1);
}

//////////////////////////////////////////////////////////////////////////////
// CFollowPathContext                                                       //
//////////////////////////////////////////////////////////////////////////////

CFollowPathContext::CFollowPathContext(const char* contextName, uint32 maxSearchDepth, bool forceMaxDepth)
{
	if (contextName==NULL)
	{
//		nldebug("CFollowPathContext: NULL : first init");
		_PrevContext= NULL;
		_ContextName= "TopOfStack";
		_MaxSearchDepth= maxSearchDepth;
		_NextContext= NULL;
	}
	else
	{
//		nldebug("CFollowPathContext: %s",contextName);
		_PrevContext= CFollowPath::getInstance()->_TopFollowPathContext;
		nlassert(_PrevContext!=NULL);
		CFollowPath::getInstance()->_TopFollowPathContext= this;
		_PrevContext->_NextContext= this;
		_ContextName= contextName;
		_MaxSearchDepth= (forceMaxDepth)? maxSearchDepth: std::min(maxSearchDepth,_PrevContext->_MaxSearchDepth);
		_NextContext= NULL;
	}
}

CFollowPathContext::~CFollowPathContext()
{
	// if we're not the bottom of stack then fixup the previous stack entry
	if (_PrevContext!=NULL)
	{
		_PrevContext->_NextContext= _NextContext;
	}

	// if we're not at the top of stack then fixup the next stack entry
	// otherwise fixup the stack top pointer in the CFollowPath singleton
	if (_NextContext!=NULL)
	{
		_NextContext->_PrevContext= _PrevContext;
	}
	else
	{
		CFollowPath::getInstance()->_TopFollowPathContext= _PrevContext;
	}
}

void CFollowPathContext::buildContextName(std::string &result) const
{
	// concatenate previous stack entries
	if (_PrevContext!=NULL)
	{
		_PrevContext->buildContextName(result);
		result+=':';
	}
	else
	{
		result.clear();
	}

	// add our name to the result
	result+= _ContextName;
}


//////////////////////////////////////////////////////////////////////////////
// CFollowPath                                                              //
//////////////////////////////////////////////////////////////////////////////

CFollowPath* CFollowPath::_Instance = NULL;

CFollowPath* CFollowPath::getInstance()
{
	if (_Instance==NULL)
	{
		_Instance = new CFollowPath;
	}
	return _Instance;
}

void CFollowPath::destroyInstance()
{
	if (_Instance!=NULL)
	{
		delete _Instance;
		_Instance = NULL;
	}
}

CFollowPath::CFollowPath()
: _LastReason(NO_REASON)
, _LastFIASPReason(CWorldMap::FIASPR_NO_REASON)
{
	// initialise the _TopFollowPathContext stack
	_TopFollowPathContext= new CFollowPathContext(NULL);
}

NLMISC::CVariable<uint> AStarNbStepsLogThreshold( "ais", "AStarNbStepsLogThreshold", "", 1000, 0, true );
namespace RYAI_MAP_CRUNCH
{
	extern uint LastAStarNbSteps;
}

inline void logTimeConsumingAStar(CModEntityPhysical* bot, float dist, CPathCont& pathCont, CAIVector* realDestination )
{
	if (!LogTimeConsumingAStar.get())
		return;
	
	if (RYAI_MAP_CRUNCH::LastAStarNbSteps >= AStarNbStepsLogThreshold.get() )
	{
		nlinfo( "ASTAR: Bot %s (type %s) used %u steps src=%s dest=%s dist=%g a*flag=%s context=%s",
			bot ? bot->getPersistent().getOneLineInfoString().c_str() : "-",
			bot ? typeid(*bot).name() : "-",
			RYAI_MAP_CRUNCH::LastAStarNbSteps,
			bot ? bot->pos().toString().c_str() : "-",
			realDestination ? realDestination->toString().c_str() : pathCont.getDestination().toString().c_str(),
			dist,
			bot ? RYAI_MAP_CRUNCH::toString(bot->getAStarFlag()).c_str() : "-",
			CFollowPath::getInstance()->getContextName() );
	}
}

NLMISC::CMustConsume<CFollowPath::TFollowStatus> CFollowPath::followPath(
		CModEntityPhysical* bot,
		CPathPosition& pathPos,
		CPathCont& pathCont,
		float dist,
		float modecalageDist,
		float angleAmort,
		bool focusOnTargetDirection,
		CAIVector* realDestination,
		float repulsSpeed,
		bool updateOrient)
{
	H_AUTO(followPath);
	
	if	(!bot->canMove())
		return FOLLOW_BLOCKED;

	TFollowStatus returnStatus = FOLLOWING;
	CAngle motionAngle;
	bool isMotionAngleComputed = false;
	CAIVector add;
	CAIVector addCorrection(0.0,0.0);
	CWorldMap const& worldMap = CWorldContainer::getWorldMap();
	
	CAIPos const startPos = CAIPos(bot->pos());
#ifdef NL_DEBUG
	nlassert(angleAmort>=0.f && angleAmort<=1.f);
#endif
	if (!bot->wpos().isValid())
		return FOLLOWING;
	
	bool haveRestart = false;
	
	switch (pathPos._PathState)
	{
		case CPathPosition::NOT_INITIALIZED:
		{
			pathPos._Angle=bot->theta();
			
reStartFollowTopo:
			if	(!pathCont.getCalcPathForSource	(pathPos,bot->wpos()))
			{
				pathPos._PathState = CPathPosition::NOT_INITIALIZED;
				returnStatus = FOLLOW_NO_PATH;
				_LastReason = FNP_NOT_INITIALIZED;
				break;
			}
			logTimeConsumingAStar(bot, dist, pathCont, realDestination);
			pathPos._PathState = CPathPosition::FOLLOWING_TOPO;
		}
		// No break
		
		case CPathPosition::FOLLOWING_TOPO:
		{
			if (pathCont._TimeTopoChanged!=pathPos._TimeTopoChanged)
			{
				goto reStartFollowTopo;
			}
			
			// If we have changed our current topology, we have to take the next furthest topo in the same 'cell'.
			CTopology::TTopologyRef	botTopology(bot->wpos().getTopologyRef());
			while (	pathPos.haveNextTopology(2)
				&&	botTopology==pathPos.getNextTopology())
			{
				pathPos.nextTopology();
			}
			
			if (!pathPos.isFinished())
			{
				CGridDirectionLayer const* layer = worldMap.getGridDirectionLayer(bot->wpos(),pathPos.getNextTopology());
				
				if (!layer) // We are now to far to have a layer to lead us to the next topo.
				{
					goto reStartFollowTopo;
				}
				
				CDirection motion = layer->getDirection(bot->wpos());
				if (motion.isValid())
				{
					motionAngle = motion.getAngle();
					isMotionAngleComputed=true;
					break;
				}
			}
			pathPos._PathState = CPathPosition::FOLLOWING_INSIDE_TOPO;
		}
		// No break
		
		case CPathPosition::FOLLOWING_INSIDE_TOPO:
		{
			if (pathCont._TimeTopoChanged!=pathPos._TimeTopoChanged)
			{
				goto reStartFollowTopo;
			}
			
			if (pathCont._TimeDestChanged!=pathPos._TimeDestChanged)
			{
				pathPos._InsidePath=NULL;
			}
			
			if (pathPos._InsidePath.isNull())
			{
				CInsidePath* insidePath = new CInsidePath();
				pathPos._InsidePath = insidePath;
				
				if (!worldMap.findInsideAStarPath(bot->wpos(), pathCont.getDestPos(), insidePath->_DirectionPath, pathCont.denyFlags()))
				{
					// If findInsideAStarPath returned false we have a topology change
					pathPos._InsidePath=NULL;
					
				#ifdef NL_DEBUG
					nlassert(!haveRestart);
				#endif
					if (!haveRestart)
					{
						haveRestart = true;
						goto reStartFollowTopo;
					}
					returnStatus = FOLLOW_NO_PATH;
					_LastReason = FNP_NO_INSIDE_ASTAR_PATH;
					_LastFIASPReason = worldMap._LastFIASPReason;
					break;
				}
				
				pathPos._InsideIndex = 0;
				
				pathPos._TimeDestChanged = pathCont._TimeDestChanged;
				pathPos._TimeTopoChanged = pathCont._TimeTopoChanged;
				
				// If we reached path end
				if (pathPos._InsideIndex >= insidePath->_DirectionPath.size())
				{
					returnStatus = FOLLOW_ARRIVED;
					pathPos._InsidePath = NULL;
				}
				else
				{
					pathPos._InsideStartPos = bot->wpos();
					CDirection	direction = insidePath->_DirectionPath[pathPos._InsideIndex];
					pathPos._InsideDestPos = pathPos._InsideStartPos.step(direction.dx(), direction.dy());
				}
			}
			
			if (!pathPos._InsidePath.isNull())
			{
				CInsidePath* insidePath = pathPos._InsidePath;
				CMapPosition botWpos = bot->wpos();
				
				while (pathPos._InsideIndex < insidePath->_DirectionPath.size())
				{
					if (botWpos == pathPos._InsideStartPos)
						break;
					
					if (botWpos == pathPos._InsideDestPos)
					{
						++pathPos._InsideIndex;
						if (pathPos._InsideIndex >= insidePath->_DirectionPath.size())
						{
							returnStatus = FOLLOW_ARRIVED;
							pathPos._InsidePath = NULL;
						}
						else
						{
							pathPos._InsideStartPos = botWpos;
							CDirection	direction = insidePath->_DirectionPath[pathPos._InsideIndex];
							pathPos._InsideDestPos = pathPos._InsideStartPos.step(direction.dx(), direction.dy());
						}
						break;
					}
					else
					{
						pathPos._InsideIndex++;
					}
				}
				
				if (returnStatus!=FOLLOW_ARRIVED && pathPos._InsideIndex >= insidePath->_DirectionPath.size())
				{
					pathPos._InsidePath = NULL;
				}
				else
				{
					if (returnStatus!=FOLLOW_ARRIVED)
					{
						motionAngle = pathPos._InsidePath->_DirectionPath[pathPos._InsideIndex].getAngle();
						isMotionAngleComputed = true;
					}
					else
					{
						pathPos._PathState = CPathPosition::REACH_END;
					}
				}
			}
		}
		break;
		
		case CPathPosition::REACH_END:
		{
			if (pathCont._TimeDestChanged!=pathPos._TimeDestChanged)
			{
				goto reStartFollowTopo;
			}
			else
			{
				returnStatus=FOLLOW_ARRIVED;
			}
		}
		break;
		
		case CPathPosition::NO_PATH:
		break;
		
		default:		
		break;
	}
	
	if (isMotionAngleComputed)
	{
		CAngle thisAngle = pathPos._Angle;
		
		CAIVector deltaToDest = (realDestination!=NULL)?*realDestination:pathCont.getDestination();
		deltaToDest -= CAIVector(bot->pos());
		
		CAngle idealAngle = deltaToDest.asAngle();
		
		sint32 absDeltaIdealAngle = abs((sint16)(idealAngle.asRawSint16()-motionAngle.asRawSint16()));
		if (absDeltaIdealAngle>=32768)
		{
			absDeltaIdealAngle = abs((sint16)absDeltaIdealAngle-65536);
		}		
		
		bool idealIsValid = true;

		if (absDeltaIdealAngle>(32768*67.5/180))	// /*97*/ if a difference of slightly more than 95 degrees.
		{
			idealIsValid = false;
		}
		else
		{
			//////////////////////////////////////////////////////////////////////////		
			// verify that the direction is valid in the map at our position (very slow, to be optimized).
			CDirection	dir0, dir1;
			CDirection::getDirectionAround(idealAngle, dir0, dir1);
			CWorldPosition	testPos(bot->wpos());
			dir1.addStep(CDirection::HALF_TURN_RIGHT);
			idealIsValid=worldMap.customCheckDiagMove(testPos, dir1, pathCont.denyFlags());
		}
		
		if (idealIsValid)
		{
			motionAngle = idealAngle;
		}
		
		{
			sint16 deltaAngle = motionAngle.asRawSint16()-thisAngle.asRawSint16();
			sint32 absDeltaAngle = abs(deltaAngle);		
			// Take the smallest angle must be add in CAngle.
			if (absDeltaAngle >= 32768)
			{
				if (deltaAngle > 0)
					deltaAngle = (sint16)deltaAngle-65536;
				else
					deltaAngle = (sint16)65536-deltaAngle;
			}
			
			// Only if significative.
			if (absDeltaAngle>32)
			{
				deltaAngle = (sint16)((float)deltaAngle*angleAmort);
				thisAngle += deltaAngle;
			}
			else
			{
				thisAngle = motionAngle;
			}
		}
		
		pathPos._Angle = thisAngle;
		
		// Verify that dist is not too high !
		{
			float reachDist = (float)(bot->pos().quickDistTo(pathCont.getDestination())*0.5+0.1);
			if (dist > reachDist)
				dist = reachDist;
		}
		
		add = CAIVector(thisAngle.asVector2d()*dist);
		if	(idealIsValid)
		{
			deltaToDest.normalize(dist*1000);
			addCorrection = deltaToDest-add;
		}
	}
	
	CAIVector moveDecalage=bot->moveDecalage();
	bot->resetDecalage();
	moveDecalage.normalize(modecalageDist*1000);
	
	CAIVector rAdd;
	if (simulateBug(8))
	{
		rAdd = add;
		if (IsRingShard.get() && ActivateStraightRepulsion.get())
		{
			// straight repulsion
			CAIVector repulsion;
			if (bot->calcStraightRepulsion(startPos+rAdd, repulsion))
			{
				rAdd += moveDecalage;
				rAdd += repulsion;
			}
			else
			{
				return FOLLOW_BLOCKED;
			}
		}
		else
		{
			// repulsion
			CAIVector repulsion = bot->calcRepulsion(startPos+rAdd);
			rAdd += moveDecalage;
			repulsion.normalize((float)(std::max((float)rAdd.norm(),repulsSpeed)*710));	// minimum of 0.25m/s.
			rAdd += repulsion;
		}
	}
	else
	{
		rAdd = add + moveDecalage;
		if (IsRingShard.get() && ActivateStraightRepulsion.get())
		{
			// straight repulsion
			CAIVector repulsion;
			if (bot->calcStraightRepulsion(startPos+rAdd, repulsion))
			{
				rAdd += repulsion;
			}
			else
			{
				return FOLLOW_BLOCKED;
			}
		}
		else
		{
			// repulsion
			CAIVector repulsion = bot->calcRepulsion(startPos+rAdd);	
			repulsion.normalize((float)(std::max((float)rAdd.norm(),repulsSpeed)*710));	// minimum of 0.25m/s.
			rAdd += repulsion;
		}
	}
	
	if (!rAdd.isNull() && !bot->moveBy(rAdd, pathCont.denyFlags()))
	{
		if (rAdd==add || !bot->moveBy(add, pathCont.denyFlags())) // Try without correction / repulsion, etc .. (if there's any).
		{
			if (!isMotionAngleComputed || pathPos._Angle==motionAngle)
			{
				// try a 1 meter step
				add = add.normalize()*1000;
				if (!bot->moveBy(add, pathCont.denyFlags()))
				return FOLLOW_BLOCKED;
			}
			
			if (isMotionAngleComputed)
			{
				pathPos._Angle = motionAngle;
				if (updateOrient)
					bot->setTheta(motionAngle);
			}
			return	returnStatus;
		}
		else
		{
			if (updateOrient)
				bot->setTheta(pathPos._Angle);
		}
	}
	else
	{
		if (updateOrient && !rAdd.isNull())
			bot->setTheta(rAdd.asAngle());
	}
	return returnStatus;
}

const char* CFollowPath::getContextName() const
{
	static std::string name;

	// the following should never happen but it's quite unimportant
	if (_TopFollowPathContext==NULL)
		return "<no context stack>";

	// delegate to the top context to build the contextname
	_TopFollowPathContext->buildContextName(name);

	// we're allowed to return this value because 'name' is a static
	return name.c_str();
}


//////////////////////////////////////////////////////////////////////////////
// CPathCont                                                                //
//////////////////////////////////////////////////////////////////////////////

CPathCont::CPathCont(RYAI_MAP_CRUNCH::TAStarFlag denyFlags)
: _TimeTopoChanged(~0)
, _TimeDestChanged(~0)		
, _denyFlags(denyFlags)
{
}

CPathCont::CPathCont(CPathCont const& pathCont)	
: _denyFlags(pathCont._denyFlags)
{
}

void CPathCont::clearPaths()
{
	_SourcePaths.clear();
}

void CPathCont::setDestination(RYAI_MAP_CRUNCH::CWorldPosition const& destPos)
{
	RYAI_MAP_CRUNCH::CMapPosition mapPos(destPos);
	
	if (_DestinationMapPos==mapPos)
		return;
	
	_DestinationMapPos = mapPos;
	_Destination = destPos;
	_VerticalPos = AITYPES::vp_auto;
	uint32 gameCycle = CTimeInterface::gameCycle();
	
	RYAI_MAP_CRUNCH::CWorldPosition	tmpPos = destPos;
	
	if (tmpPos!=_DestPos)
	{
		_DestPos = tmpPos;
		_TimeDestChanged = gameCycle;
		
		if (!_DestPos.isValid())
		{
			clearPaths();
		}
		else
		{
			RYAI_MAP_CRUNCH::CTopology::TTopologyRef newTopo = _DestPos.getTopologyRef();
			if (((RYAI_MAP_CRUNCH::CTopology::TTopologyId)newTopo)!=_TopoRef )
			{
				_TimeTopoChanged = gameCycle;
				clearPaths();
				_TopoRef = newTopo;
			}
		}
	}
}

void CPathCont::setDestination(AITYPES::TVerticalPos verticalPos, CAIVector const& destPos)
{
	RYAI_MAP_CRUNCH::CMapPosition mapPos(destPos);
	
	if (_DestinationMapPos==mapPos)
		return;
	
	_DestinationMapPos = mapPos;
	_Destination = destPos;
	_VerticalPos = verticalPos;
	uint32 gameCycle = CTimeInterface::gameCycle();
	
	RYAI_MAP_CRUNCH::CWorldPosition	tmpPos;
	CWorldContainer::getWorldMap().setWorldPosition(verticalPos, tmpPos,destPos);
	
	if (tmpPos!=_DestPos)
	{
		_DestPos = tmpPos;
		_TimeDestChanged = gameCycle;
		
		if (!_DestPos.isValid())
		{
			clearPaths	();
		}
		else
		{
			RYAI_MAP_CRUNCH::CTopology::TTopologyRef newTopo = _DestPos.getTopologyRef();
			if (((RYAI_MAP_CRUNCH::CTopology::TTopologyId)newTopo)!=_TopoRef )
			{
				_TimeTopoChanged = gameCycle;
				clearPaths();
				_TopoRef = newTopo;
			}
		}
	}
}

bool CPathCont::getPathForSource(CPathPosition& pathPos, RYAI_MAP_CRUNCH::CWorldPosition const& startPos) const
{
	H_AUTO(getPathForSource);
	RYAI_MAP_CRUNCH::CTopology::TTopologyRef const startTopo = startPos.getTopologyRef();
	
	std::vector<NLMISC::CSmartPtr<CAIPath> >::const_iterator it = _SourcePaths.begin();
	const std::vector<NLMISC::CSmartPtr<CAIPath> >::const_iterator itEnd = _SourcePaths.end();
	
	while (it!=itEnd)
	{
		CAIPath	*path=*(it);
		
		std::vector<RYAI_MAP_CRUNCH::CTopology::TTopologyRef>::const_iterator const topoItBegin=path->topologiesPath().begin();
		std::vector<RYAI_MAP_CRUNCH::CTopology::TTopologyRef>::const_iterator const topoItEnd=path->topologiesPath().end();
		std::vector<RYAI_MAP_CRUNCH::CTopology::TTopologyRef>::const_iterator const topoItFind = std::find(topoItBegin,topoItEnd,startTopo);
		
		if (topoItFind!=topoItEnd)
		{
			pathPos._Index = (uint)(topoItFind-topoItBegin);
			pathPos._Path = *it;
			return true;
		}
		++it;
	}
	pathPos._Path = NULL;
	return false; // No path found.
}

bool CPathCont::calcPathForSource(CPathPosition& pathPos, RYAI_MAP_CRUNCH::CWorldPosition const& startPos)
{
	H_AUTO(calcPathForSource);
	
	RYAI_MAP_CRUNCH::CWorldMap const& worldMap = CWorldContainer::getWorldMap();
	
	{
		// Get a free path.
		std::vector<NLMISC::CSmartPtr<CAIPath> >::iterator it, itEnd = _SourcePaths.end();
		for (it = _SourcePaths.begin(); it!=itEnd; ++it)
			if ((*it)->getRefCount()==1) // one refcount is reserved for the current list.
				break;
		
		// If we didn't found a free path we have to extend the vector..
		if (it == itEnd)
		{
			_SourcePaths.push_back(new CAIPath());
			pathPos._Path = _SourcePaths.back();
		}
		else
		{
			pathPos._Path = (*it);
		}
	}
	
	pathPos._Index = 0;	//	current topology index.
	
	CAIPath& pathRef = *(pathPos._Path);
	
	// Check start position validity
	if (!startPos.isValid())
	{
		RYAI_MAP_CRUNCH::CWorldPosition newpos;
		CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, newpos, startPos, 6, 100, CWorldContainer::CPosValidatorDefault());
	#ifdef NL_DEBUG
		if (newpos.isValid() && LogAcceptablePos)
				nlwarning("Path pos error at position %s: an acceptable position could be %s", startPos.toString().c_str(), newpos.toString().c_str() );
		if (!newpos.isValid())
			nlwarning("Path pos error at position %s: no acceptable position found around", startPos.toString().c_str());
	#endif
		pathRef.setStartPos(newpos);
	}
	else
	{
		pathRef.setStartPos(startPos);
	}
	
	// Check dest position validity
	if (!_DestPos.isValid())
	{
		RYAI_MAP_CRUNCH::CWorldPosition newpos;
		CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, newpos, _DestPos, 6, 100, CWorldContainer::CPosValidatorDefault());
	#ifdef NL_DEBUG
		if (newpos.isValid() && LogAcceptablePos)
			nlwarning("Path pos error at position %s, an acceptable position could be %s", _DestPos.toString().c_str(), newpos.toString().c_str() );
		if (!newpos.isValid())
			nlwarning("Path pos error at position %s, no acceptable position found around", _DestPos.toString().c_str());
	#endif
		_DestPos = newpos;
	}
	
	pathRef.setEndPos(_DestPos);
	
	if (!worldMap.findAStarPath(pathRef.getStartPos(), _DestPos, pathRef.topologiesPathForCalc(), _denyFlags))
	{
		pathPos._Path = NULL;
		return false;
	}
	return true;
}

bool CPathCont::getCalcPathForSource(CPathPosition& pathPos, RYAI_MAP_CRUNCH::CWorldPosition const& startPos)
{
	H_AUTO(GetCalcPathForSource);
	static uint32 getCount = 0;
	static uint32 calcCount = 0;
	pathPos._TimeDestChanged = _TimeDestChanged;
	pathPos._TimeTopoChanged = _TimeTopoChanged;
	
	if (getPathForSource(pathPos, startPos))
	{
		++getCount;
		return true;
	}
	if (calcPathForSource(pathPos, startPos))
	{
		++calcCount;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
// CPathPosition                                                            //
//////////////////////////////////////////////////////////////////////////////

CPathPosition::CPathPosition(CAngle const& angle)
: _Angle(angle)
, _Path(NULL)
, _PathState(NOT_INITIALIZED)
{
}

bool CPathPosition::isPathValid() const
{		
	return !_Path.isNull();
}

RYAI_MAP_CRUNCH::CTopology::TTopologyRef const& CPathPosition::getTopology() const
{
#ifdef NL_DEBUG
	nlassert(_Index<_Path->topologiesPath().size());
#endif
	return _Path->topologiesPath()[_Index];
}

RYAI_MAP_CRUNCH::CTopology::TTopologyRef const& CPathPosition::getNextTopology() const
{
#ifdef NL_DEBUG
	nlassert((_Index+1)<_Path->topologiesPath().size());
#endif
	return _Path->topologiesPath()[_Index+1];
}

bool CPathPosition::isFinished() const
{
	uint32 size = (uint32)_Path->topologiesPath().size();
	return (size==0 || _Index==size-1);
}

bool CPathPosition::nextTopology()
{
	++_Index;
	return _Index<_Path->topologiesPath().size();
}

bool CPathPosition::haveNextTopology(uint nbTopo)
{
	return (_Index+nbTopo)<_Path->topologiesPath().size();
}	

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

