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
#include "entity_sorter.h"
#include "displayer_visual_entity.h"
#include "../user_entity.h"
#include "../time_client.h"

using namespace NLMISC;

namespace R2
{


#ifdef NL_DEBUG
	#define CHECK_INTEGRITY checkIntegrity();
#else
	#define CHECK_INTEGRITY
#endif

// *********************************************************************************************************
CEntitySorter::CEntitySorter()
{
}

// *********************************************************************************************************
void CEntitySorter::registerEntityDisplayer(CDisplayerVisualEntity *dve)
{
	nlassert(dve);
	CHECK_INTEGRITY
	#ifdef NL_DEBUG
		nlassert(!_EntityDisplayers.count(dve));
		nlassert(std::find(_AddRemoveDebug.begin(), _AddRemoveDebug.end(), NLMISC::CRefPtr<CDisplayerVisualEntity>(dve)) == _AddRemoveDebug.end());
		_AddRemoveDebug.push_back(dve);
	#endif
	_EntityDisplayers[dve] = -1;
	CHECK_INTEGRITY
}

// *********************************************************************************************************
void CEntitySorter::unregisterEntityDisplayer(CDisplayerVisualEntity *dve)
{
	nlassert(dve);
	CHECK_INTEGRITY
	TDisplayers::iterator it = _EntityDisplayers.find(dve);
	nlassert(it != _EntityDisplayers.end());
	eraseVisibleListEntry(it->second /* index in visible list */);
	_EntityDisplayers.erase(it);
	#ifdef NL_DEBUG
		nlassert(!_EntityDisplayers.count(dve));
		TAddRemoveDebug::iterator debugIt = std::find(_AddRemoveDebug.begin(), _AddRemoveDebug.end(), NLMISC::CRefPtr<CDisplayerVisualEntity>(dve));
		nlassert(debugIt != _AddRemoveDebug.end());
		_AddRemoveDebug.erase(debugIt);
	#endif
	CHECK_INTEGRITY
}

// *********************************************************************************************************
uint CEntitySorter::getMaxEntityCount() const
{
	static volatile uint MAX_NUM_VISIBLE_ENTITIES = 253;
	return MAX_NUM_VISIBLE_ENTITIES;
}

// *********************************************************************************************************
bool CEntitySorter::isWithinDisplayRange(CDisplayerVisualEntity *displayer) const
{
	if (!displayer) return false;
	if (!_FarthestEntity.Displayer) return true;
	return CSortedEntity(displayer, (UserEntity->pos() - displayer->getWorldPos()).sqrnorm()) < _FarthestEntity;
}

// *********************************************************************************************************
void CEntitySorter::sortByEntitiesByDistance()
{
	CHECK_INTEGRITY
	nlassert(_SortTable.empty());
	CVectorD playerPos = UserEntity->pos();
	for(TDisplayers::iterator it = _EntityDisplayers.begin(); it != _EntityDisplayers.end(); ++it)
	{
		CSortedEntity ed;
		ed.Displayer = it->first;
		ed.Dist2 = (playerPos - ed.Displayer->getWorldPos()).sqrnorm();
		ed.IndexInVisibleList = it->second;
		_SortTable.push_back(ed);
	}
	std::sort(_SortTable.begin(), _SortTable.end());
	CHECK_INTEGRITY
}


// *********************************************************************************************************
void CEntitySorter::updateFadeInOut()
{
	CHECK_INTEGRITY
	sint k = 0;
	float delta = ClientCfg.R2EDClippedEntityBlendTime;
	delta = delta != 0.f ? 1.f / delta : 10000.f;
	while (k < (sint) _VisibleList.size())
	{
		CVisibleEntity &entity =  _VisibleList[k];
		if (entity.State == CVisibleEntity::Appear)
		{
			// fade in
			NLMISC::incrementalBlend(entity.BlendValue, 1.f, delta * DT);
			entity.Displayer->setClipBlend(_VisibleList[k].BlendValue);
			++ k;
		}
		else
		{
			// fade out
			nlassert(entity.State == CVisibleEntity::Disappear);
			if (entity.Displayer->getEntity() && entity.Displayer->getEntity()->getLastClip())
			{
				entity.BlendValue = 0.f;
			}
			else
			{
				NLMISC::incrementalBlend(entity.BlendValue, 0.f, delta * DT);
				entity.Displayer->setClipBlend(_VisibleList[k].BlendValue);
			}
			if (entity.BlendValue == 0.f)
			{
				CHECK_INTEGRITY
				// fade out finished -> remove from list
				eraseVisibleListEntry(k);
				//
				CHECK_INTEGRITY
			}
			else
			{
				++k;
			}
		}
	}
	CHECK_INTEGRITY
}

// *********************************************************************************************************
void CEntitySorter::eraseVisibleListEntry(sint index)
{
	if (index == -1) return;
	nlassert(index >= 0 && index < (sint) _VisibleList.size());
	CVisibleEntity &lastEntity  =  _VisibleList.back();
	CVisibleEntity &entity		=  _VisibleList[index];
	// order doesn't matter for us, so copy the last entry at our position
	#ifdef NL_DEBUG
		nlassert(_EntityDisplayers.count(entity.Displayer));
		nlassert((sint) _EntityDisplayers[entity.Displayer] == index);
		nlassert((uint) _EntityDisplayers[lastEntity.Displayer] == _VisibleList.size() - 1);
	#endif
	_EntityDisplayers[lastEntity.Displayer] = index; // remap index to the new position
	_EntityDisplayers[entity.Displayer] = -1;	// should set to -1 last for the case where &lastEntity == &entity
	 entity = lastEntity;
	_VisibleList.pop_back();
}

// *********************************************************************************************************
void CEntitySorter::updateVisibleList()
{
	CHECK_INTEGRITY
	uint maxEntityCount = getMaxEntityCount();
	//
	uint k;
	uint candidateCount = std::min(maxEntityCount, uint(_SortTable.size()));
	// while there is remaining places in thevisible list, insert visible entities
	for(k = 0; k < candidateCount; ++k)
	{
		if (_SortTable[k].IndexInVisibleList == -1) // not inserted in visible list ?
		{
			if (_VisibleList.size() < maxEntityCount)
			{
				CDisplayerVisualEntity *dve = _SortTable[k].Displayer;
				_VisibleList.push_back(CVisibleEntity(dve));
				if (dve->isCreatedThisFrame())
				{
					_VisibleList.back().BlendValue = 1.f; // directly visible if just created
				}
				#ifdef NL_DEBUG
					nlassert(_EntityDisplayers.count(dve));
				#endif
				_EntityDisplayers[dve] = (sint)_VisibleList.size() - 1;

			}
		}
		else
		{
			_VisibleList[_SortTable[k].IndexInVisibleList].State = CVisibleEntity::Appear;
		}
	}
	if (!_SortTable.empty())
	{
		_FarthestEntity = _SortTable.back();
	}
	else
	{
		_FarthestEntity = CSortedEntity();
	}
	// hide remaining entities
	for (; k < _SortTable.size(); ++k)
	{
		if (_SortTable[k].IndexInVisibleList != -1)
		{
			#ifdef NL_DEBUG
				nlassert(_SortTable[k].IndexInVisibleList < (sint) _VisibleList.size());
			#endif
			// I'm always in the visible list, mark me as 'dissapearing'
			// until i'm removed from the visible list for real during 'updateFadeInOut'
			_VisibleList[_SortTable[k].IndexInVisibleList].State = CVisibleEntity::Disappear;
		}
		else
		{
			_SortTable[k].Displayer->setClipBlend(0.f);
		}
	}
	_SortTable.clear();
	CHECK_INTEGRITY
}

// *********************************************************************************************************
void CEntitySorter::clipEntitiesByDist()
{
	CHECK_INTEGRITY
	sint64 startTime = 0;
	static volatile bool bench = false;
	if (bench)
	{
		startTime = CTime::getPerformanceTime();
	}
	sortByEntitiesByDistance();
	updateVisibleList();
	updateFadeInOut();
	if (bench)
	{
		sint64 endTime = CTime::getPerformanceTime();
		nlwarning("clip time = %.2f ms", 1000.f * CTime::ticksToSecond(endTime - startTime));
	}
	CHECK_INTEGRITY
}

// *********************************************************************************************************
void CEntitySorter::checkIntegrity()
{
	#ifdef NL_DEBUG
		for(TAddRemoveDebug::iterator it = _AddRemoveDebug.begin(); it != _AddRemoveDebug.end(); ++it)
		{
			nlassert(*it); // if this assert fire, then a displayer was registered using 'registerEntityDisplayer", but deleted
						   // without calling unregisterEntityDisplayer
		}
		nlassert(_VisibleList.size() <= getMaxEntityCount());
		for(TDisplayers::iterator it = _EntityDisplayers.begin(); it != _EntityDisplayers.end(); ++it)
		{
			TIndexInVisibleList indexInVisibleList = it->second;
			if (indexInVisibleList != -1)
			{
				nlassert(indexInVisibleList >= 0 && indexInVisibleList < (TIndexInVisibleList) _VisibleList.size());
				const CVisibleEntity &visibleEntity = _VisibleList[indexInVisibleList];
				nlassert(visibleEntity.Displayer == it->first);
				nlassert(visibleEntity.BlendValue >= 0.f && visibleEntity.BlendValue <= 1.f);
				nlassert((uint) visibleEntity.State < CVisibleEntity::StateCount);
			}
		}
		for(uint k = 0; k < _VisibleList.size(); ++k)
		{
			nlassert(_VisibleList[k].Displayer != NULL);
			nlassert(_EntityDisplayers.count(_VisibleList[k].Displayer));
			nlassert((uint) _EntityDisplayers[_VisibleList[k].Displayer] == k);
		}
	#endif
}




} //