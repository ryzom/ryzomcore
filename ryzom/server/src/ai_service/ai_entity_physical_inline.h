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




#ifndef RYAI_ENTITY_PHYSICAL_INLINE_H
#define RYAI_ENTITY_PHYSICAL_INLINE_H

#include "ai.h"
#include "game_share/fame.h"
/*

  The following base class is the common parent of bots, plrs and any other
  physical object that has an existance in the world

*/

//////////////////////////////////////////////////////////////////////////////
// CAIEntityPhysical                                                        //
//////////////////////////////////////////////////////////////////////////////

#ifdef NL_OS_WINDOWS
#pragma warning (push)
#pragma warning (disable : 4355)
#endif

inline
CAIEntityPhysical::CAIEntityPhysical(CPersistentOfPhysical& owner, TDataSetRow const& entityIndex, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag const& AStarFlags)
: CAIEntity()
, CSpawnable<CPersistentOfPhysical >(owner)
, _pos(entityIndex)
, _dataSetRow(entityIndex)
, _Stuned(0)
, _Rooted(0)
, _Blinded(0)
, _Feared(0)
//, _UnreachableTarget((CAIEntityPhysical*)NULL)
, _id(id)
, _radius(radius)
, _food(_radius)
, _Level(level)
, _AStarFlags(AStarFlags)
{
	_instanceNumber.init(*CMirrors::DataSet, entityIndex, DSPropertyAI_INSTANCE);
	_mode.init			(*CMirrors::DataSet, entityIndex, DSPropertyMODE);
	_behaviour.init		(*CMirrors::DataSet, entityIndex, DSPropertyBEHAVIOUR);
	_targetRow.init		(*CMirrors::DataSet, entityIndex, DSPropertyTARGET_ID);
	_ActionFlags.init	(*CMirrors::DataSet, entityIndex, DSPropertyACTION_FLAGS);

	_RunSpeed.init		(*CMirrors::DataSet, entityIndex, DSPropertyCURRENT_RUN_SPEED);
	_WalkSpeed.init		(*CMirrors::DataSet, entityIndex, DSPropertyCURRENT_WALK_SPEED);
	
	_CurrentHitPoint.init	(*CMirrors::DataSet, entityIndex, DSPropertyCURRENT_HIT_POINTS);
	_MaxHitPoint.init		(*CMirrors::DataSet, entityIndex, DSPropertyMAX_HIT_POINTS);
	_VisionCounter.init		(*CMirrors::DataSet, entityIndex, DSPropertyVISION_COUNTER);
	_InOutpostAlias.init	(*CMirrors::DataSet, entityIndex, DSPropertyIN_OUTPOST_ZONE_ALIAS);
	_InOutpostSide.init		(*CMirrors::DataSet, entityIndex, DSPropertyIN_OUTPOST_ZONE_SIDE);

#ifdef NL_DEBUG
	const NLMISC::CDbgRefCount<CAIEntityPhysical>	&ref=*(static_cast<NLMISC::CDbgRefCount<CAIEntityPhysical> * const>(this));
	NLMISC::CDbgPtr<CAIEntityPhysical>	dummyPtr;
	
	nlassert(ref.getDbgRef(dummyPtr)==0);
	nlassert(CAIS::instance()._CAIEntityByDataSetRow.find(entityIndex.getIndex())==CAIS::instance()._CAIEntityByDataSetRow.end());
	ref.setCheckMax(false);
#endif
	CAIS::instance()._CAIEntityByDataSetRow.insert(std::make_pair(entityIndex.getIndex(),this));	//	we don't want to register players.	
#ifdef NL_DEBUG
	ref.setCheckMax(true);
	nlassert(ref.getDbgRef(dummyPtr)==1);
	nlassert(CAIS::instance()._CAIEntityByDataSetRow.find(entityIndex.getIndex())!=CAIS::instance()._CAIEntityByDataSetRow.end());
#endif

	CAIEntityPhysicalLocator::getInstance()->addEntity(_dataSetRow, _id, this);
}

inline
CAIEntityPhysical::~CAIEntityPhysical()
{
	CAIEntityPhysicalLocator::getInstance()->delEntity(_dataSetRow, _id, this);
	
#ifdef NL_DEBUG
	const NLMISC::CDbgRefCount<CAIEntityPhysical>	&ref=*(static_cast<NLMISC::CDbgRefCount<CAIEntityPhysical> * const>(this));
	NLMISC::CDbgPtr<CAIEntityPhysical>	dummyPtr;	
#endif

	// Remove healers
	std::multiset<IAIEntityPhysicalHealer*>::iterator it, itEnd;
	for (it=_Healers.begin(), itEnd=_Healers.end(); it!=itEnd; ++it)
	{
		(*it)->healerRemoved(this);
	}
	
	detachFromTargeting();
	CAIS::instance()._CAIEntityByDataSetRow.erase(dataSetRow().getIndex());

#ifdef NL_DEBUG
	nlassert(ref.getDbgRef(dummyPtr)==0);
	nlassert(CAIS::instance()._CAIEntityByDataSetRow.find(dataSetRow().getIndex())==CAIS::instance()._CAIEntityByDataSetRow.end());
#endif
}

#ifdef NL_OS_WINDOWS
#pragma warning (pop)
#endif

inline
void CAIEntityPhysical::setActionFlags(RYZOMACTIONFLAGS::TActionFlag const& flag)
{
	_ActionFlags = (_ActionFlags.getValue() | flag );
}

inline
void CAIEntityPhysical::removeActionFlags(RYZOMACTIONFLAGS::TActionFlag const& flag)
{
	_ActionFlags = (_ActionFlags.getValue() & ~flag );
}

inline
float CAIEntityPhysical::getCollisionDist(float angTo) const
{
	return 0.5;
}

inline
bool CAIEntityPhysical::canMove() const
{
	return !isRooted() && (walkSpeed() != 0);
}

inline
float CAIEntityPhysical::walkSpeed() const
{
	return _WalkSpeed()*0.1f*SpeedFactor*getSpeedFactor();
}

inline
float CAIEntityPhysical::runSpeed() const
{
	return _RunSpeed()*0.1f*SpeedFactor*getSpeedFactor();
}

inline
void CAIEntityPhysical::setWPos(RYAI_MAP_CRUNCH::CWorldPosition const& pos)
{
	nlassert(pos.isValid());
	_wpos = pos;
}

//////////////////////////////////////////////////////////////////////////////
// CPersistentOfPhysical                                                    //
//////////////////////////////////////////////////////////////////////////////

inline
bool CPersistentOfPhysical::isAt16MetersPos(uint16 x, uint16 y)	const
{
	CAIEntityPhysical const* phys = getSpawnObj();
	if (!phys)
		return false;
	return (	(uint16)(phys->x().asInt16Meters())==x
		&&	(uint16)(phys->y().asInt16Meters())==y	);
}

inline
CAIEntityPhysical* CPersistentOfPhysical::getSpawnObj() const
{
	return	CPersistent<CAIEntityPhysical>::getSpawnObj();
}

//////////////////////////////////////////////////////////////////////////////
// CModEntityPhysical                                                       //
//////////////////////////////////////////////////////////////////////////////

inline
void CModEntityPhysical::setMode(MBEHAV::EMode m)
{
	if (_mode().Mode == MBEHAV::DEATH)	//	to remove (can be replaced with the above assertion in debug mode).
		return;
	
	if (_mode().Mode == m)
		return;
	_mode = MBEHAV::TMode(m, pos().x().asInt(), pos().y().asInt());
}

inline
void CModEntityPhysical::setTarget(CAIEntityPhysical* target)
{
	CTargetable<CAIEntityPhysical>::setTarget(target);
//	if (CTargetable<CAIEntityPhysical>::getTarget()==target)
	const CAIEntityPhysical* tgt=CTargetable<CAIEntityPhysical>::getTarget();
	if (tgt==target)
		_targetRow = target ? target->dataSetRow() : TDataSetRow();
}

inline
void CModEntityPhysical::setVisualTarget(CAIEntityPhysical* target)
{
	CTargetable<CAIEntityPhysical>::setVisualTarget(target);
//	if (CTargetable<CAIEntityPhysical>::getVisualTarget()==target)
	const CAIEntityPhysical* tgt=CTargetable<CAIEntityPhysical>::getVisualTarget();
	if (tgt==target)
		_targetRow = target ? target->dataSetRow() : TDataSetRow();
}

inline
void CModEntityPhysical::setUnreachableTarget(CAIEntityPhysical* target)
{
	CTargetable<CAIEntityPhysical>::setUnreachableTarget(target);
//	if (CTargetable<CAIEntityPhysical>::getUnreachableTarget()==target)
	const CAIEntityPhysical* tgt=CTargetable<CAIEntityPhysical>::getUnreachableTarget();
	if (tgt==target)
		_targetRow = target ? target->dataSetRow() : TDataSetRow();
}

inline
bool CModEntityPhysical::setPos(CAIPos const& pos)
{
	RYAI_MAP_CRUNCH::CWorldPosition	wpos;
	if	(!CWorldContainer::getWorldMap().setWorldPosition(AITYPES::vp_auto, wpos,pos))
		return	false;
	setPos(pos, wpos);
	return	true;
}	

inline
bool CModEntityPhysical::moveTo(CAIPos const& newPos, RYAI_MAP_CRUNCH::TAStarFlag const& denyFlags)
{
	if (!wpos().isValid())
		return false;
	
	RYAI_MAP_CRUNCH::CWorldPosition	tmpWPos(wpos());
	if (CWorldContainer::getWorldMap().move(tmpWPos, RYAI_MAP_CRUNCH::CMapPosition(newPos), denyFlags))
	{
#ifdef NL_DEBUG
		nlassert(RYAI_MAP_CRUNCH::CMapPosition(newPos)==tmpWPos);
#endif
		setPos(newPos, tmpWPos);
		return true;
	}
	else
	{
		//	a part of move has been done.
		if (tmpWPos!=wpos())
		{
			CAIPos partNewPos(tmpWPos.toAIVector(),newPos.h(),newPos.theta());
			setPos(partNewPos, tmpWPos);
			return true;
		}
		
	}
	//	no move at all..
	return false;
};

inline
void CModEntityPhysical::resetDecalage()
{
	_Decalage.setXY(0, 0);
}

//////////////////////////////////////////////////////////////////////////////
// CWorldMapLink                                                            //
//////////////////////////////////////////////////////////////////////////////

template <class T>
void CWorldMapLink<T>::linkEntityToMatrix(const CAICoord &x,const CAICoord &y,CAIEntityMatrix<T>& matrix)
{
	_matrixListLink.link(matrix[(uint8)(y.asInt16Meters())][(uint8)(x.asInt16Meters())]);
}

template <class T>
void CWorldMapLink<T>::linkEntityToMatrix(const CAIVectorMirror &pos,CAIEntityMatrix<T>& matrix)
{
	linkEntityToMatrix(pos.x(),pos.y(),matrix);
}

template <class T>
void CWorldMapLink<T>::linkEntityToMatrix(const CAIVector &pos,CAIEntityMatrix<T>& matrix)
{
	linkEntityToMatrix(pos.x(),pos.y(),matrix);
}

template <class T>
void CWorldMapLink<T>::linkToWorldMap(T *entityPtr, const	CAIVector	&pos,CAIEntityMatrix<T>& matrix)
{
	_matrixListLink.setEntity	(entityPtr);
	linkEntityToMatrix			(pos,matrix);
}

template <class T>
void CWorldMapLink<T>::linkToWorldMap(T *entityPtr, const	CAIVectorMirror	&pos,CAIEntityMatrix<T>& matrix)
{
	_matrixListLink.setEntity	(entityPtr);
	linkEntityToMatrix			(pos,matrix);
}

//////////////////////////////////////////////////////////////////////////////
// CTargetable                                                              //
//////////////////////////////////////////////////////////////////////////////

template <class T>
CTargetable<T>::CTargetable()
: _FightTargetersWeight(0.f)
, _FightTargetersValue(0.f)
{
	for	(size_t i=0; i<TARGET_TYPE_MAX; ++i)
	{
		_TargeterCount[i] = 0;
		_FirstTargeters[i] = (T*)NULL;
	}
	_Target = (T*)NULL;
	_TargetType = TARGET_TYPE_FIGHT;
	_NextTargeter = (T*)NULL;
}

template <class T>
CTargetable<T>::~CTargetable()
{
	// This should be done in Derived class dtor, but just in case of since we're instanciable
	detachFromTargeting();
}

template <class T>
void CTargetable<T>::detachFromTargeters()
{
	for	(size_t i=0; i<TARGET_TYPE_MAX; ++i)
	{
		while (!_FirstTargeters[i].isNULL())
		{
			TPtr targeter = _FirstTargeters[i];
			targeter->targetDied();
			// This method is a bit more intrusive than targeter->setTarget(NULL), but it avoids possible infinite loops
			_FirstTargeters[i] = targeter->_NextTargeter;
			unlinkTargeter((TTargetType)i, targeter);
		}
	}
}

template <class T>
void CTargetable<T>::detachFromTargeting()
{
	setTarget((T*)NULL);
	detachFromTargeters();
}

template <class T>
void CTargetable<T>::linkTargeter(TTargetType type, TPtr const& targeter, TPtr const& nextTargeter)
{
	++_TargeterCount[type];
	if (type==TARGET_TYPE_FIGHT)
	{
		_FightTargetersWeight += targeter->fightWeight();
		_FightTargetersValue += targeter->fightValue();
	}
	targeter->_Target = dynamic_cast<T*>(this);
	targeter->_TargetType = type;
	targeter->_NextTargeter = nextTargeter;
	// Some verifications to avoid algorithm bugs
//	nlassert(fightTargetersFreeWeight()>=-0.00001f);
	nlassert(!targeter->_Target.isNULL());
}

template <class T>
void CTargetable<T>::unlinkTargeter(TTargetType type, TPtr const& targeter)
{
#ifdef NL_DEBUG
	nlassert((T*)targeter->_Target == this);
#else
	if ((T*)targeter->_Target != this)
		nlwarning("Targeter's target is not me!");
#endif
	targeter->_Target = (T*)NULL;
	targeter->_NextTargeter = (T*)NULL;
	--_TargeterCount[type];
	if (type==TARGET_TYPE_FIGHT)
	{
		_FightTargetersWeight -= targeter->fightWeight();
		_FightTargetersValue -= targeter->fightValue();
	}
}

template <class T>
void CTargetable<T>::addTargeter(TTargetType type, TPtr const& targeter)
{
	linkTargeter(type, targeter, _FirstTargeters[type]);
	_FirstTargeters[type] = targeter;
}

template <class T>
void CTargetable<T>::removeTargeter(TTargetType type, TPtr const& targeter)
{
	if (targeter.isNULL())
		return;
	if (_FirstTargeters[type] == targeter)
	{
		_FirstTargeters[type] = targeter->_NextTargeter;
		unlinkTargeter(type, targeter);
		return;
	}
	
	NLMISC::CDbgPtr<T> current = _FirstTargeters[type];
	while (!current.isNULL())
	{
		NLMISC::CDbgPtr<T> next = current->_NextTargeter;
		if (next == targeter)
		{
			current->_NextTargeter = targeter->_NextTargeter;
			unlinkTargeter(type, targeter);
			break;
		}
		else
		{
			current = next;
		}
	}
}

template <class T>
void CTargetable<T>::tryToAddTargeter(TTargetType type, TPtr const& targeter)
{
	H_AUTO(CTargetable_addTargeter);

	if (type==TARGET_TYPE_FIGHT)
	{
	
	// Prepare data to optimize ----------------------------------------------
	size_t opponentCount = targeterCount();
	// Allocate arrays filled with 0.f
	float* weights = new float[opponentCount+1];
	float* values = new float[opponentCount+1];
	bool* take = new bool[opponentCount+1];
	for (size_t i=0; i<opponentCount+1; ++i)
	{
		weights[i] = 0.f;
		values[i] = 0.f;
		take[i] = false;
	}
	{
		H_AUTO(addTargeterInfoRetr);
		// Fill with the former opponents
		T* opponent = firstTargeter();
		size_t i = 0;
		while (opponent!=NULL)
		{
			weights[i] = opponent->fightWeight();
			values[i] = opponent->fightValue();
			opponent = opponent->nextTargeter();
			take[i] = true;
			++i;
		}
		// Add the new targeter
		weights[opponentCount] = targeter->fightWeight();
		values[opponentCount] = targeter->fightValue();
		take[opponentCount] = false;
	}
	// Create the context
	CKnapsackContext context(opponentCount+1, weights, values);
	// Configure and run the solver ------------------------------------------
	CKnapsackSolver solver(&context, take);
	{
		H_AUTO(addTargeterOptimize);
		solver.optimize(fightTargetersWeightMax(), _TargeterChoiceAlgorithm);
	}
	// Save the data ---------------------------------------------------------
	// If the new set is better than the old one
	if (solver.totalValue()>fightTargetersValue())
	{
		// Unlink the old ones if needed
		T* opponent = _FirstTargeters[type];
		TPtr* previousPtr = &_FirstTargeters[type];
		size_t i = 0;
		while (opponent!=NULL)
		{
			if (!solver.take(i))
			{
				*previousPtr = opponent->_NextTargeter;
				unlinkTargeter(type, opponent);
			}
			else
			{
				previousPtr = &opponent->_NextTargeter;
				opponent = opponent->_NextTargeter;
			}
			++i;
		}
		// Link the new targeter if needed
		if (solver.take(opponentCount))
		{
			addTargeter(type, targeter);
		}
#ifdef NL_DEBUG
		// Debug verifications
		{
			opponent = _FirstTargeters[type];
			float value = 0.f;
			float weight = 0.f;
			while (opponent!=NULL)
			{
				value += opponent->fightValue();
				weight += opponent->fightWeight();
				opponent = opponent->_NextTargeter;
			}
			// Verify linking system
			nlassert(fabs(fightTargetersValue() - value) < 0.0001);
			nlassert(fabs(fightTargetersWeight() - weight) < 0.0001);
			
			// Verify solver behaviour
			nlassert(fabs(fightTargetersValue() - solver.totalValue()) < 0.0001);
			nlassert(fabs(fightTargetersWeight() - solver.totalWeight()) < 0.0001);
		}
#endif
	}
	delete [] take;
	delete [] values;
	delete [] weights;
	
	}
	else
	{
		// Just link the new targeter (no limit on visual targeters)
		addTargeter(type, targeter);
	}
}

template <class T>
void CTargetable<T>::setTarget(T* target)
{
	setTarget(TARGET_TYPE_FIGHT, target);
}

template <class T>
void CTargetable<T>::setVisualTarget(T* target)
{
	setTarget(TARGET_TYPE_VISUAL, target);
}

template <class T>
void CTargetable<T>::setUnreachableTarget(T* target)
{
	setTarget(TARGET_TYPE_UNREACHABLE, target);
}

template <class T>
void CTargetable<T>::setTarget(TTargetType type, TPtr const& target)
{
	if (target==_Target && type==_TargetType)
		return;
	
	if (!_Target.isNULL())
	{
		_Target->removeTargeter(_TargetType, dynamic_cast<T*>(this));
	}
	if (!target.isNULL())
	{
		target->tryToAddTargeter(type, dynamic_cast<T*>(this));
	}
}

template <class T>
typename CTargetable<T>::TPtr CTargetable<T>::getTarget() const
{
	return getTarget(TARGET_TYPE_FIGHT);
}

template <class T>
typename CTargetable<T>::TPtr CTargetable<T>::getVisualTarget() const
{
	return getTarget(TARGET_TYPE_VISUAL);
}

template <class T>
typename CTargetable<T>::TPtr CTargetable<T>::getUnreachableTarget() const
{
	return getTarget(TARGET_TYPE_UNREACHABLE);
}

template <class T>
typename CTargetable<T>::TPtr CTargetable<T>::getTarget(TTargetType type) const
{
	if (_TargetType==type)
		return _Target;
	else
		return (T*)NULL;
}

template <class T>
uint32 CTargetable<T>::totalTargeterCount() const
{
	uint32 count = 0;
	for (size_t i = 0; i<TARGET_TYPE_MAX; ++i)
		count += _TargeterCount[i];
	return count;
}

//----------------------------------------------------------------------------

#endif

