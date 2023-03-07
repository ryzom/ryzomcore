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

#ifndef RYAI_COMMANDS_H
#define RYAI_COMMANDS_H

#include <string>

//////////////////////////////////////////////////////////////////////////////
// CStringFilter                                                            //
//////////////////////////////////////////////////////////////////////////////

/// Match name according '*' and '?' DOS properties.
class CStringFilter
{
public:
	CStringFilter(std::string const& filter)
		: _Filter(filter)
	{
	}
	virtual ~CStringFilter() { }
	
	bool operator !=(std::string const& other) const { return !operator ==(other); }
	bool operator ==(std::string const& other) const { return NLMISC::testWildCard(other, _Filter); }
	
private:
	std::string _Filter;
};

//////////////////////////////////////////////////////////////////////////////
// Entity filters                                                           //
//////////////////////////////////////////////////////////////////////////////

/*

AIS hierarchy:

CAIS
+- CAIInstance
   +- COutpostSquadFamily
   |  \- CGroupDesc<COutpostSquadFamily>
   |     \- CBotDesc<COutpostSquadFamily>
   +- CContinent
   |  +- CRegion
   |  |  +- CCellZone
   |  |  |  \- CCell
   |  |  |     +- CFaunaZone
   |  |  |     \- CRoad
   |  |  |        \- CRoadTrigger
   |  |  +- CGroupFamily
   |  |  |  \- CGroupDesc<CGroupFamily>
   |  |  |     \- CBotDesc<CGroupFamily>
   |  |  \- CFamilyBehavior
   |  |     +- CMgrNpc : CManager
   |  |     |  \- CGroup
   |  |     |     \- CBot
   |  |     \- CMgrFauna : CManager
   |  |        \- CGroup
   |  |           \- CBot
   |  \- COutpost
   |     +- COutpostSpawnZone
   |     \- COutpostManager : CManager
   |        \- CGroup
   |           \- CBot
   \- CManager
      \- CGroup
         \- CBot

*/

//- Fetchers -----------------------------------------------------------------

template <class TContainer>
void buildInstanceList(TContainer& container)
{
	FOREACH(itAIInstance, CCont<CAIInstance>, CAIS::instance().AIList())
	{
		CAIInstance* aiinstance = *itAIInstance;
		container.push_back(aiinstance);
	}
}

template <class TContainer>
void buildContinentList(TContainer& container)
{
	std::deque<CAIInstance*> aiinstances;
	buildInstanceList(aiinstances);
	FOREACH(itAIInstance, std::deque<CAIInstance*>, aiinstances)
	{
		CAIInstance* aiinstance = *itAIInstance;
		if (aiinstance == NULL)
			continue;
		FOREACH(itContinent, CCont<CContinent>, aiinstance->continents())
		{
			CContinent* continent = *itContinent;
			container.push_back(continent);
		}
	}
}

template <class TContainer>
void buildRegionList(TContainer& container)
{
	std::deque<CContinent*> continents;
	buildContinentList(continents);
	FOREACH(itContinent, std::deque<CContinent*>, continents)
	{
		CContinent* continent = *itContinent;
		if (continent == NULL)
			continue;
		FOREACH(itRegion, CCont<CRegion>, continent->regions())
		{
			CRegion* region = *itRegion;
			container.push_back(region);
		}
	}
}

template <class TContainer>
void buildCellZoneList(TContainer& container)
{
	std::deque<CRegion*> regions;
	buildRegionList(regions);
	FOREACH(itRegion, std::deque<CRegion*>, regions)
	{
		CRegion* region = *itRegion;
		if (region == NULL)
			continue;
		FOREACH(itCellZone, CCont<CCellZone>, region->cellZones())
		{
			CCellZone* cellZone = *itCellZone;
			container.push_back(cellZone);
		}
	}
}

template <class TContainer>
void buildFamilyBehaviorList(TContainer& container)
{
	std::deque<CCellZone*> cellZones;
	buildCellZoneList(cellZones);
	FOREACH(itCellZone, std::deque<CCellZone*>, cellZones)
	{
		CCellZone* cellZone = *itCellZone;
		if (cellZone == NULL)
			continue;
		FOREACH(itFamilyBehavior, CCont<CFamilyBehavior>, cellZone->familyBehaviors())
		{
			CFamilyBehavior* familyBehavior = *itFamilyBehavior;
			container.push_back(familyBehavior);
		}
	}
}

template <class TContainer>
void buildOutpostList(TContainer& container)
{
	std::deque<CContinent*> continents;
	buildContinentList(continents);
	FOREACH(itContinent, std::deque<CContinent*>, continents)
	{
		CContinent* continent = *itContinent;
		if (continent == NULL)
			continue;
		FOREACH(itOutpost, CCont<COutpost>, continent->outposts())
		{
			COutpost* outpost = *itOutpost;
			container.push_back(outpost);
		}
	}
}

template <class TContainer>
void buildManagerList(TContainer& container)
{
	std::deque<CFamilyBehavior*> familyBehaviors;
	buildFamilyBehaviorList(familyBehaviors);
	FOREACH(itFamilyBehavior, std::deque<CFamilyBehavior*>, familyBehaviors)
	{
		CFamilyBehavior* familyBehavior = *itFamilyBehavior;
		if (familyBehavior == NULL)
			continue;
		container.push_back(static_cast<CManager*>(familyBehavior->mgrNpc()));
		container.push_back(static_cast<CManager*>(familyBehavior->mgrFauna()));
	}
	std::deque<CAIInstance*> aiinstances;
	buildInstanceList(aiinstances);
	FOREACH(itAIInstance, std::deque<CAIInstance*>, aiinstances)
	{
		CAIInstance* aiinstance = *itAIInstance;
		if (aiinstance == NULL)
			continue;
		FOREACH(itManager, CCont<CManager>, aiinstance->managers())
		{
			CManager* manager = *itManager;
			container.push_back(manager);
		}
	}
	std::deque<COutpost*> outposts;
	buildOutpostList(outposts);
	FOREACH(itOutpost, std::deque<COutpost*>, outposts)
	{
		COutpost* outpost = *itOutpost;
		if (outpost == NULL)
			continue;
		FOREACH(itManager, CCont<COutpostManager>, outpost->managers())
		{
			CManager* manager = static_cast<CManager*>(*itManager);
			container.push_back(manager);
		}
	}
}

template <class TContainer>
void buildGroupList(TContainer& container)
{
	std::deque<CManager*> managers;
	buildManagerList(managers);
	FOREACH(itManager, std::deque<CManager*>, managers)
	{
		CManager* manager = *itManager;
		if (manager == NULL)
			continue;
		FOREACH(itGroup, CCont<CGroup>, manager->groups())
		{
			CGroup* group = *itGroup;
			container.push_back(group);
		}
	}
}

template <class TContainer>
bool buildFilteredGroupList(TContainer& container, std::string filterString)
{
	typedef typename TContainer::value_type TValue;
	std::deque<TValue> _container;
	buildGroupList(_container);
	FOREACHC(it, typename std::deque<TValue>, _container)
	{
		typename TContainer::value_type value = *it;
		if (value == NULL)
			continue;
		CStringFilter filter(filterString);
		if (!isIdentifiedAs(*value, filter))
			continue;
		container.push_back(value);
	}
	return true;
}

template <class TContainer>
void buildBotList(TContainer& container)
{
	std::deque<CGroup*> groups;
	buildGroupList(groups);
	FOREACH(itGroup, std::deque<CGroup*>, groups)
	{
		CGroup* group = *itGroup;
		if (group == NULL)
			continue;
		FOREACH(itBot, CCont<CBot>, group->bots())
		{
			CBot* bot = *itBot;
			container.push_back(bot);
		}
	}
}

template <class TContainer>
bool buildFilteredBotList(TContainer& container, std::string filterString)
{
	typedef typename TContainer::value_type TValue;
	std::deque<TValue> _container;
	buildBotList(_container);
	FOREACHC(it, typename std::deque<TValue>, _container)
	{
		typename TContainer::value_type value = *it;
		if (value == NULL)
			continue;
		CStringFilter filter(filterString);
		if (!isIdentifiedAs(*value, filter))
			continue;
		container.push_back(value);
	}
	return true;
}

template <class TContainer>
void buildPlayerList(TContainer& container)
{
	std::deque<CAIInstance*> aiinstances;
	buildInstanceList(aiinstances);
	FOREACH(itAIInstance, std::deque<CAIInstance*>, aiinstances)
	{
		CAIInstance* aiinstance = *itAIInstance;
		if (!aiinstance)
			continue;
		CManagerPlayer* manager = aiinstance->getPlayerMgr();
		if (!manager)
			continue;
		FOREACH(itPlayer, CManagerPlayer::TPlayerMap, manager->playerList())
		{
			CBotPlayer* player = itPlayer->second;
			container.push_back(player);
		}
	}
}


template <class TContainer>
void buildFaunaPlaceList(TContainer& container)
{
	std::deque<CGroup *> groups;
	buildGroupList(groups);
	FOREACH(itGroup, std::deque<CGroup*>, groups)
	{
		CGrpFauna *grpFauna = dynamic_cast<CGrpFauna *>(*itGroup);
		if (!grpFauna)
			continue;		
		FOREACH(itPlaces, CAliasCont<CAIPlace>, grpFauna->places())
		{			
			container.push_back(*itPlaces);
		}
	}
}

//- Selector -----------------------------------------------------------------

template <class TValue>
bool isIdentifiedAs(TValue const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	return false;
}

template <>
inline
bool isIdentifiedAs<CBot>(CBot const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	if (filter==value.getEntityIdString())
		return true;
	if (filter==value.getAliasString())
		return true;
	if (filter==value.getName())
		return true;
	if (filter==value.getFullName())
		return true;
	return false;
}

template <>
inline
bool isIdentifiedAs<CGroup>(CGroup const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	if (filter==value.getAliasString())
		return true;
	if (filter==value.getName())
		return true;
	if (filter==value.getFullName())
		return true;
	return false;
}

template <>
inline
bool isIdentifiedAs<CManager>(CManager const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	if (filter==value.getAliasString())
		return true;
	if (filter==value.getName())
		return true;
	if (filter==value.getFullName())
		return true;
	return false;
}

template <>
inline
bool isIdentifiedAs<CBotPlayer>(CBotPlayer const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	if (filter==value.getEntityIdString())
		return true;
	return false;
}

template <>
inline
bool isIdentifiedAs<CAIInstance>(CAIInstance const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	if (filter==value.getContinentName())
		return true;
	return false;
}

template <>
inline
bool isIdentifiedAs<CContinent>(CContinent const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	if (filter==value.getName())
		return true;
	return false;
}

template <>
inline
bool isIdentifiedAs<CRegion>(CRegion const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	if (filter==value.getName())
		return true;
	return false;
}

template <>
inline
bool isIdentifiedAs<CCellZone>(CCellZone const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	if (filter==value.getName())
		return true;
	return false;
}

template <>
inline
bool isIdentifiedAs<CFamilyBehavior>(CFamilyBehavior const& value, CStringFilter const& filter)
{
	if (filter==value.getIndexString())
		return true;
	if (filter==value.grpFamily()->getName())
		return true;
	return false;
}

//- Functors -----------------------------------------------------------------

template <class TValue>
void display(TValue& value, NLMISC::CLog& log)
{
	log.displayNL("%s %s", value.getIndexString().c_str(), value.getOneLineInfoString().c_str());
}

template <class TValue>
void displayEx(TValue& value, NLMISC::CLog& log)
{
	std::vector<std::string> strings = value.getMultiLineInfoString();
	FOREACHC(itString, std::vector<std::string>, strings)
		log.displayNL("%s", itString->c_str());
}

template <class TValue>
void spawn(TValue& value)
{
	value.spawn();
}

template <class TValue>
void despawn(TValue& value)
{
	value.despawn();
}

//- Specialized functors -----------------------------------------------------

template <>
inline
void despawn(CGroup& value)
{
	value.despawnGrp();
}

template <>
inline
void despawn(CManager& value)
{
	value.despawnMgr();
}

// CFaunaGenericPlace specialization
inline
void spawn(CAIPlace &value)
{
	CFaunaGenericPlace *fgp = dynamic_cast<CFaunaGenericPlace *>(&value);
	if (fgp)
	{
		if (!fgp->getTimeDriven())
		{
			fgp->setActive(true);
		}
		else
		{
			nlwarning("Places %s is not time driven, cannot activate it", value.getIndexString().c_str());
		}
	}
}

inline
void despawn(CAIPlace &value)
{
	CFaunaGenericPlace *fgp = dynamic_cast<CFaunaGenericPlace *>(&value);
	if (fgp)
	{
		if (!fgp->getTimeDriven())
		{
			fgp->setActive(false);
		}
		else
		{
			nlwarning("Places %s is not time driven, cannot deactivate it", value.getIndexString().c_str());
		}		
	}
}


//- Top-level functions ------------------------------------------------------

template <class TContainer>
void displayList(NLMISC::CLog& log, TContainer const& container, std::string filterString = std::string("*"))
{
	FOREACHC(it, typename TContainer, container)
	{
		typename TContainer::value_type value = *it;
		if (value == NULL)
			continue;
		CStringFilter filter(filterString);
		if (!isIdentifiedAs(*value, filter))
			continue;
		display(*value, log);
	}
}

template <class TContainer>
void displayListEx(NLMISC::CLog& log, TContainer const& container, std::string filter = std::string("*"))
{
	CLogStringWriter stringWriter(&log);
	FOREACHC(it, typename TContainer, container)
	{
		typename TContainer::value_type value = *it;
		if (value == NULL)
			continue;
		if (!isIdentifiedAs(*value, filter))
			continue;
		displayEx(*value, log);
	}
}

template <class TContainer>
void spawnList(NLMISC::CLog& log, TContainer const& container, std::string filterString = std::string("*"))
{
	FOREACHC(it, typename TContainer, container)
	{
		typename TContainer::value_type value = *it;
		if (value == NULL)
			continue;
		CStringFilter filter(filterString);
		if (!isIdentifiedAs(*value, filter))
			continue;
		spawn(*value);
	}
}

template <class TContainer>
void despawnList(NLMISC::CLog& log, TContainer const& container, std::string filterString = std::string("*"))
{
	FOREACHC(it, typename TContainer, container)
	{
		typename TContainer::value_type value = *it;
		if (value == NULL)
			continue;
		CStringFilter filter(filterString);
		if (!isIdentifiedAs(*value, filter))
			continue;
		despawn(*value);
	}
}

//- NeL commands -------------------------------------------------------------

#define RYAI_TEMPLATED_COMMAND(__name,__help,__args,__type,__fetchmethod,__functor) \
NLMISC_COMMAND(__name,__help,__args)			\
{												\
	std::deque<__type*> container;				\
	__fetchmethod(container);					\
	if (args.size()>0)							\
		__functor(log, container, args[0]);		\
	else										\
		__functor(log, container);				\
	return true;								\
}

#endif
