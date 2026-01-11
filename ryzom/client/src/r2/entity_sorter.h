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

#ifndef R2_ENTITY_SORTER_H
#define R2_ENTITY_SORTER_H



#include "displayer_visual_entity.h"

namespace R2
{




/** This class maintains a set of active entities and display the ones that are nearest from the player, to bypass
  * the max limit of 255 entities displayed simultaneously by the entity manager (thus allowing edition of mush bigger
  * scenarii in the editor)
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 10/2006
  */
class CEntitySorter
{
public:
	CEntitySorter();
	// register one entity displayer : allows to sort them by distance to display the nearest entities
	void registerEntityDisplayer(CDisplayerVisualEntity *dve);
	void unregisterEntityDisplayer(CDisplayerVisualEntity *dve);
	// Return max number of entities that can be displayed (visible) simultaneously
	uint getMaxEntityCount() const;
	// main update
	void clipEntitiesByDist();
	// test if a new entity is in the good range to be displayed (based on the max dist of the last display)
	bool isWithinDisplayRange(CDisplayerVisualEntity *displayer) const;
private:

	typedef sint TIndexInVisibleList; // -1 if not inserted
	typedef std::map<CDisplayerVisualEntity *, TIndexInVisibleList> TDisplayers;
	//=============================================================================================
	// temporary obj for sorting of entities by distance
	class CSortedEntity
	{
	public:
		CDisplayerVisualEntity *Displayer;
		double					Dist2;
		TIndexInVisibleList     IndexInVisibleList;
	public:
		CSortedEntity(CDisplayerVisualEntity *displayer = NULL, double dist2 = 0.f, TIndexInVisibleList indexInVisibleList = -1)
				   :  Displayer(displayer), Dist2(dist2), IndexInVisibleList(indexInVisibleList)
		{}
		bool operator<(const CSortedEntity &rhs) const
		{
			if (Dist2 == rhs.Dist2)
			{
				return Displayer->getEntity() < rhs.Displayer->getEntity();
			}
			return Dist2  < rhs.Dist2;
		}
	};
	//=============================================================================================
	// A Currently visible entity (possibly fading in/out)
	class CVisibleEntity
	{
	public:
		enum TState { Appear = 0, Disappear, StateCount };
		TState State;
		float BlendValue; // current transparency (0.f hidden -> 1.f visible)
		CDisplayerVisualEntity *Displayer;
	public:
		CVisibleEntity(CDisplayerVisualEntity *displayer = NULL) : State(Appear), BlendValue(0.f), Displayer(displayer) {}
	};
	//
	std::vector<CSortedEntity>			_SortTable;			// *Table temporarily used for the sort	(avoid reallocation of a new vector each time)
	CSortedEntity						_FarthestEntity;	// *Contain the farthest visible entity after sorting by distance
	std::vector<CVisibleEntity>			_VisibleList;		// *List of currently visible entities. may not match the current '_SortTable' array
															//  because an entity must completely fade out before being evicted	from '_VisibleList',
															//  thus letting room for a newly visible entity
	TDisplayers							_EntityDisplayers;	// *the registered entities, competing for visibility

	#ifdef NL_DEBUG
		typedef std::vector<NLMISC::CRefPtr<CDisplayerVisualEntity> > TAddRemoveDebug;
		TAddRemoveDebug _AddRemoveDebug; // set of refptr to see if an object has been removed without
																			   // being unregistered from this object
	#endif

private:
	void sortByEntitiesByDistance();
	void updateVisibleList();
	void updateFadeInOut();
	void eraseVisibleListEntry(sint index);
	void checkIntegrity();
};




};



#endif
