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




//----------------------------------------------------------------------------

#include "stdpch.h"
#include "ais_actions.h"
#include "family_behavior.h"
#include "ai_instance.h"

#include "continent_inline.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace CAISActionEnums;
using	namespace	AITYPES;

extern CAIInstance *currentInstance;

DEFINE_ACTION(ContextGlobal,DYNSYS)
{
/*	{ // init the ai instance if necessaray
		CAIS &ais = CAIS::instance();

		CAIInstance	*aiInstance=NULL;
		
		if (0<ais.AIList().size())
		{
			aiInstance=ais.AIList()[0];
		}
		
		if	(!aiInstance)
		{
			aiInstance=ais.AIList().addChild(new CAIInstance(*((CAIS*)NULL)),	0);	//	Arggghhh !!!
			if (!aiInstance)
			{
				return;
			}

		}
		CWorkPtr::instance(aiInstance);	// set the current AIInstance.
	}
*/
	nlassertex(currentInstance != NULL, ("No AIInstance created !"));
	CWorkPtr::aiInstance(currentInstance);
	CAIInstance *aii = CWorkPtr::aiInstance();

//	uint32	alias;
//	string	name, mapName;
	string contName, mapName;
	if	(!getArgs(args,"DYNSYS", contName, mapName))
		return;

	CWorkPtr::continent(NULL);
	// look for a continent with this name.
	CCont<CContinent> &continents = aii->continents();
	for (uint i=0; i<continents.size(); ++i)
	{
		if (!continents[i])
			continue;
		if (continents[i]->getName() == contName)
		{
			// ok, we found it
			CWorkPtr::continent(continents[i]);
			break;
		}
	}

	if (CWorkPtr::continent() == NULL)
	{
		// continent not found, need to create one
		CContinent	*const	cont = new	CContinent(aii);
		aii->continents().addChild(cont);
		cont->setName(contName);
		CWorkPtr::continent(cont);
	}

	CContextStack::setContext(ContextContinent);

}

bool dumpContinentImp(uint instanceIndex, const std::string &continentName, NLMISC::CLog &log);

DEFINE_ACTION(ContextGlobal,DYN_END)	//	ContextContinent
{
	CContinent *continent = CWorkPtr::continent();
	if	(!continent)
		return;

//	dumpContinentImp(0, continent->ContinentName, *NLMISC::DebugLog);

	continent->pushLazyProcess(new CRebuildContinentAndOutPost(continent));
}

DEFINE_ACTION(ContextContinent,DYNREG)
{
	CContinent *const	continent = CWorkPtr::continent();
	if	(!continent)
		return;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	string	filename;
	if	(!getArgs(args,name(),aliasTree, filename))
		return;

	// see whether the region is already created
	CRegion	*region	=	CWorkPtr::continent()->regions().getChildByName(aliasTree->getName());

	// not found so create it
	if	(!region)
	{
		region = new CRegion(CWorkPtr::continent(), aliasTree->getAlias(), aliasTree->getName(), filename);
		
		CWorkPtr::continent()->regions().addAliasChild(region);
		CWorkPtr::region(region);
	}
	else
	{
		region->registerForFile(filename);
	}

	// clear delete flag (if any)
//	region->clearDeleteFlag();
	CWorkPtr::region(region);
	CContextStack::setContext(ContextRegion);
}

DEFINE_ACTION(ContextRegion,IDTREE)
{
	// set the id tree for the region (results in creation or update of region's object tree)
	// args: aliasTree

	if (!CWorkPtr::region())
		return;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args,name(),aliasTree))
		return;

		
	// have the manager update it's structure from the id tree
	nlinfo("ACTION IDTREE: Applying new tree to region[%u]: '%s'%s in continent '%s'", 
		CWorkPtr::region()->getChildIndex(), 
		CWorkPtr::region()->getName().c_str(),
		CWorkPtr::region()->getAliasString().c_str(), 
		CWorkPtr::region()->getOwner()->getName().c_str()
		);
	
	if (aliasTree && CWorkPtr::region())
		CWorkPtr::region()->updateAliasTree(*aliasTree);

//	dumpContinentImp(0, CWorkPtr::continent()->ContinentName, *NLMISC::DebugLog);

}


DEFINE_ACTION(ContextRegion,CELLZNE)
{
	CRegion *region= CWorkPtr::region();
	if (!region)
		return;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args,name(),aliasTree))
		return;

	// see whether the region is already loaded
	CCellZone *czone= region->cellZones().getChildByAlias(aliasTree->getAlias());
	if (!czone)
		return;

	CWorkPtr::cellZone(czone);
	CContextStack::setContext(ContextCellZone);
}




//DEFINE_ACTION(ContextCellZone,CZ_NRJ)
//{
//	return;
//
//	CCellZone *czone=CWorkPtr::cellZone();
//	if (!czone || args.size()!=5)
//		return;
//
//	std::string	family;
//	args[0].get(family);
//	if	(family.empty())
//		return;	
//
//	// read the alias tree from the argument list	
//	CLevelEnergy	&levelEnergy=czone->_FamiliesLevelEnergy[CPropertyId::create(family)];
//
//	for	(uint32 i=0;i<4;i++)
//	{
//		std::string	str;
//		double	value=1;
//		if (args[i+1].get(str))
//		{
//			if (!str.empty())
//				value=atof(str.c_str());
//		}
//		levelEnergy.setLevelEnergyValue(value, i);
//	}
//
//}


DEFINE_ACTION(ContextGroupFamily,CZ_NRJ)
{
	CGroupFamily *const	groupFamily = CWorkPtr::groupFamily();
	if	(!groupFamily)
		return;


	for	(uint32 i=0;i<4;i++)
	{
		std::string	str;
		double	value=1;
		if (args[i].get(str))
		{
			if (!str.empty())
				value=atof(str.c_str());
		}
		groupFamily->setLevelEnergyValue(value, i);
	}
	
//	groupFamily->setLevelEnergy();
//
//
//	CCellZone *czone=CWorkPtr::cellZone();
//	if (!czone || args.size()!=5)
//		return;
//	
//	std::string	family;
//	args[0].get(family);
//	if	(family.empty())
//		return;	
//	
//	// read the alias tree from the argument list	
//	CLevelEnergy	&levelEnergy=czone->_FamiliesLevelEnergy[CPropertyId::create(family)];
//	
//	for	(uint32 i=0;i<4;i++)
//	{
//		std::string	str;
//		double	value=1;
//		if (args[i+1].get(str))
//		{
//			if (!str.empty())
//				value=atof(str.c_str());
//		}
//		levelEnergy.setLevelEnergyValue(value, i);
//	}
	
}

DEFINE_ACTION(ContextCellZone,CELL)
{
	CCellZone *cellZone= CWorkPtr::cellZone();
	if (!cellZone)
		return;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args,name(),aliasTree))
		return;

	// see whether the region is already loaded
	CCell *cell= cellZone->cells().getChildByAlias(aliasTree->getAlias());
	if (!cell)
		return;

	CWorkPtr::cell(cell);
	CContextStack::setContext(ContextCell);
}

DEFINE_ACTION(ContextCell,CELLFLG)
{
//	CCell *cell= CWorkPtr::cell();
//	if (!cell)
//		return;
//
//	cell->_FamilyFlags.clearFamily();
//
//	for (uint i=0; i<args.size(); ++i)
//	{
//		string flag;
//		args[i].get(flag);
//		TPopulationFamily fam(flag);
//		if (fam.FamilyTag != family_bad)
//			cell->_FamilyFlags.addFamily(flag);
//	}
//	cell->_FamilyFlags.addFamily(TPopulationFamily("kitin_invasion"));	//	Arghh !! to much hard code!

	CCell *cell= CWorkPtr::cell();
	if	(!cell)
		return;

	for (uint i=0; i<args.size(); ++i)
	{
		string property;
		args[i].get(property);
		cell->properties().addProperty(property);
	}
	cell->properties().addProperty(string("kitin_invasion"));	//	Arghh !! to much hard code!

//	cell->_FamilyFlags.addFamily(TPopulationFamily("kitin_invasion"));	//	Arghh !! to much hard code!
}

DEFINE_ACTION(ContextCell,CELLGEO)
{
	CCell *cell= CWorkPtr::cell();
	if (!cell)
		return;

	cell->_Coords.clear();
	for (uint i=0; i<args.size(); i+=2)
	{
		double x, y;
		args[i].get(x);
		args[i+1].get(y);

		cell->_Coords.push_back(CAIVector(x, y));
	}
}



DEFINE_ACTION(ContextCell,DYNFZ)
{
	CCell *cell= CWorkPtr::cell();
	if	(!cell)
		return;

	float x, y, r;
//	uint32 activities;
	uint32 verticalPos;
	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args,name(),aliasTree, x, y, r, /*activities,*/ verticalPos))
		return;

	// see whether the region is already loaded
	CFaunaZone *faunaZone = cell->faunaZones().getChildByAlias(aliasTree->getAlias());
	if (!faunaZone)
		return;

	faunaZone->setPosAndRadius((TVerticalPos)verticalPos, CAIPos(x, y,0,0.0f), uint32(r*1000));

//	faunaZone->setFaunaActivity(activities);

	CWorkPtr::faunaZone(faunaZone);
	CContextStack::setContext(ContextFaunaZone);
}

DEFINE_ACTION(ContextFaunaZone,ACT_PARM)
{
	CFaunaZone	*faunaZone=CWorkPtr::faunaZone();
	if	(!faunaZone)
		return;

	for (uint i=0; i<args.size(); ++i)
	{
		string activityProp;
		args[i].get(activityProp);
		faunaZone->initialActivities().addProperty(CPropertyId::create(activityProp));
	}

}

DEFINE_ACTION(ContextCell,DYNNZ)
{
	CCell *cell= CWorkPtr::cell();
	if (!cell)
		return;

	float x, y, r;
	uint32 verticalPos;
	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args,name(),aliasTree, x, y, r, verticalPos))
		return;

	// see whether the region is already loaded
	CNpcZonePlace *npcZone = cell->npcZonePlaces().getChildByAlias(aliasTree->getAlias());
	if (!npcZone)
		return;

	npcZone->setPosAndRadius((TVerticalPos)verticalPos, CAIPos(x, y,0,0.0f), uint32(r*1000));
//	npcZone->setNpcActivity(activities);

	CWorkPtr::npcZone(npcZone);
	CContextStack::setContext(ContextNpcZone);
}

DEFINE_ACTION(ContextCell,DYNNZSHP)
{
	CCell *cell= CWorkPtr::cell();
	if (!cell || args.size()<2)
		return;
	
	std::vector<CAIVector> points;
	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	uint32 verticalPos;
	args[0].get(aliasTree);
	args[1].get(verticalPos);
	for (uint i=3;i<args.size();i+=2)
	{
		double x,y;
		args[i-1].get(x);
		args[i].get(y);
		points.push_back(CAIVector(x,y));
	}
	
	// see whether the region is already loaded
	CNpcZoneShape *npcZone = cell->npcZoneShapes().getChildByAlias(aliasTree->getAlias());
	if (!npcZone)
		return;

	npcZone->setPatat((TVerticalPos)verticalPos, points);
//	npcZone->setNpcActivity(activities);

	CWorkPtr::npcZone(npcZone);
	CContextStack::setContext(ContextNpcZone);
}

DEFINE_ACTION(ContextNpcZone,DYNNZPRM)
{
	CNpcZone	*npcZone=CWorkPtr::npcZone();
	if	(!npcZone)
		return;

	for (uint i=0; i<args.size(); ++i)
	{
		string activityProp;
		args[i].get(activityProp);
		npcZone->properties().addProperty(CPropertyId::create(activityProp));
	}

}

DEFINE_ACTION(ContextRegion,DYNROAD)
{
	CCell *cell= CWorkPtr::cell();
	if (!cell)
		return;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	float difficulty;
	uint32 verticalPos;
	if (!getArgs(args,name(),aliasTree, difficulty, verticalPos))
		return;

	// see whether the region is already loaded
	CRoad *road = cell->roads().getChildByAlias(aliasTree->getAlias());
	if (!road)
		return;

	road->setDifficulty(difficulty);
	road->setVerticalPos((TVerticalPos)verticalPos);

	CWorkPtr::road(road);
	CContextStack::setContext(ContextRoad);
}

DEFINE_ACTION(ContextRoad,ROADGEO)
{
	CRoad *road= CWorkPtr::road();
	if	(	!road
		||	args.empty())
		return;

	vector<CAIVector>	points;

	road->clearCoords();

	for	(uint i=1; i<args.size(); i+=2)
	{
		double x, y;
		args[i-1].get(x);
		args[i].get(y);

		points.push_back(CAIVector(x, y));
	}

	road->setPathPoints(road->verticalPos(), points);;
}

DEFINE_ACTION(ContextRoad,TRIGGER)
{
	CRoad *road= CWorkPtr::road();
	if (!road)
		return;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args,name(),aliasTree))
		return;

	// see whether the region is already loaded
	CRoadTrigger *trigger = road->triggers().getChildByAlias(aliasTree->getAlias());
	if (!trigger)
		return;

	CWorkPtr::roadTrigger(trigger);
	CContextStack::setContext(ContextRoadTrigger);
}

DEFINE_ACTION(ContextRoadTrigger,TRIGT1)
{
	CRoadTrigger *trigger= CWorkPtr::roadTrigger();
	if (!trigger)
		return;

	float x, y, r;

	if (!getArgs(args, name(), x, y, r))
		return;

	trigger->_Trigger1 = CAICircle(CAIVector(x, y), r);
}
DEFINE_ACTION(ContextRoadTrigger,TRIGT2)
{
	CRoadTrigger *trigger= CWorkPtr::roadTrigger();
	if (!trigger)
		return;

	float x, y, r;

	if (!getArgs(args, name(), x, y, r))
		return;

	trigger->_Trigger2 = CAICircle(CAIVector(x, y), r);
}
DEFINE_ACTION(ContextRoadTrigger,TRIGSP)
{
	CRoadTrigger *trigger= CWorkPtr::roadTrigger();
	if (!trigger)
		return;

	float x, y, r;

	if (!getArgs(args, name(), x, y, r))
		return;

	trigger->_Spawn = CAICircle(CAIVector(x, y), r);
}
DEFINE_ACTION(ContextRoadTrigger,TRIGFLG)
{
//	CRoadTrigger *trigger= CWorkPtr::roadTrigger();
//	if (!trigger)
//		return;
//
//	string flag;
//
//	trigger->_FamilyFlags.clearFamily();
//
//	for (uint i=0; i<args.size(); ++i)
//	{
//		string flag;
//		args[i].get(flag);
//		TPopulationFamily fam(flag);
//		if (fam.FamilyTag != family_bad)
//			trigger->_FamilyFlags.addFamily(flag);
//	}
}


 DEFINE_ACTION(BaseContextState, GRPFAM)
{
	string	family;
	CAIAliasDescriptionNode *
		aliasTree;
	std::string grpFamName;	
	uint32 logicActionAlias;
	if	(!getArgs(args,name(),aliasTree, family, grpFamName, logicActionAlias))
		return;		
	IAILogicAction *action = CWorkPtr::getLogicActionFromAlias(logicActionAlias);
	if (!action) return;
	
	CSmartPtr<CGroupFamily> grpFam = new CGroupFamily(NULL, aliasTree->getAlias(), grpFamName);
	
	grpFam->updateAliasTree(*aliasTree);

	action->addGroupFamily(grpFam);
		
	CWorkPtr::groupFamily(grpFam);
	CContextStack::setContext(ContextGroupFamily);
}

DEFINE_ACTION(ContextRegion,GRPFAM)
{
	string	family;
	CAIAliasDescriptionNode *aliasTree;
	if	(!getArgs(args,name(),aliasTree, family))
		return;		
	CGroupFamily *groupFamily;	
	CRegion *const	region = CWorkPtr::region();
	if	(!region)
		return;
	// see whether the region is already loaded
	groupFamily= region->groupFamilies().getChildByAlias(aliasTree->getAlias());	
	
	if	(!groupFamily)
		return;	
	groupFamily->setFamilyTag(family);
	CWorkPtr::groupFamily(groupFamily);
	CContextStack::setContext(ContextGroupFamily);
}

DEFINE_ACTION(ContextGroupFamily,TMPPRFF)
{	
	CGroupFamily *const	groupFamily = CWorkPtr::groupFamily();
	if	(!groupFamily)
		return;

	groupFamily->setProfileName("groupFamilyProfileFauna");
}

DEFINE_ACTION(ContextGroupFamily,TMPPRFT)
{	
	CGroupFamily *const	groupFamily = CWorkPtr::groupFamily();
	if	(!groupFamily)
		return;

	if (args.size()!=1)
		return;

	CAggroGroupContainer *aggroGroupContainer= new CAggroGroupContainer();
	string aggroGroupString;
	args[0].get(aggroGroupString);	
	AISHEETS::CCreature::getGroupStr(aggroGroupContainer->aggroGroupIds, aggroGroupString);
	
	groupFamily->setProfileName("groupFamilyProfileTribe");
	groupFamily->setProfileParams("aggro_groups", aggroGroupContainer);
}

DEFINE_ACTION(ContextGroupFamily,TMPPRFN)
{	
	CGroupFamily *const	groupFamily = CWorkPtr::groupFamily();
	if	(!groupFamily)
		return;

	groupFamily->setProfileName("groupFamilyProfileNpc");
}

DEFINE_ACTION(ContextGroupFamily,TMPPRFNF)	//	Npc flags
{	
	CGroupFamily *const	groupFamily = CWorkPtr::groupFamily();
	if	(!groupFamily)
		return;

	AITYPES::CPropertySet	actSet;

	for (uint i=0; i<args.size(); ++i)
	{
		string activityProp;
		args[i].get(activityProp);
		actSet.addProperty(CPropertyId::create(activityProp));
	}
	
	groupFamily->addProfileProperty(string("npc"), actSet);
}
DEFINE_ACTION(ContextGroupFamily,TMPPRFFF)	//	Food
{	
	CGroupFamily *const	groupFamily = CWorkPtr::groupFamily();
	if	(!groupFamily)
		return;

	AITYPES::CPropertySet	actSet;

	for (uint i=0; i<args.size(); ++i)
	{
		string activityProp;
		args[i].get(activityProp);
		actSet.addProperty(CPropertyId::create(activityProp));
	}
	
	groupFamily->addProfileProperty(string("food"), actSet);
}
DEFINE_ACTION(ContextGroupFamily,TMPPRFFR)	//	Rest
{	
	CGroupFamily *const	groupFamily = CWorkPtr::groupFamily();
	if	(!groupFamily)
		return;

	AITYPES::CPropertySet	actSet;
	
	for (uint i=0; i<args.size(); ++i)
	{
		string activityProp;
		args[i].get(activityProp);
		actSet.addProperty(CPropertyId::create(activityProp));
	}
	
	groupFamily->addProfileProperty(string("rest"), actSet);
}



DEFINE_ACTION(ContextGroupFamily,GRPTMPL)
{
	CGroupFamily *const	groupFamily = CWorkPtr::groupFamily();
	if	(!groupFamily)
		return;

	string	grpFamily;
	uint32	botCount;
	bool	countMultipliedBySheet;
	bool	multiLevel;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if	(!getArgs(args, name(), aliasTree, grpFamily, botCount, countMultipliedBySheet, multiLevel))
		return;

	CGroupDesc<CGroupFamily> *const	groupDesc= groupFamily->groupDescs().getChildByAlias(aliasTree->getAlias());
	if	(!groupDesc)
		return;

	
	groupDesc->setBaseBotCount	(botCount);
	groupDesc->setCountMultiplierFlag(countMultipliedBySheet);
	groupDesc->setMultiLevel(multiLevel);

	CWorkPtr::groupDesc(groupDesc);
	CContextStack::setContext(ContextGroupDesc);
}

DEFINE_ACTION(ContextGroupFamily,GF_FAM)
{
	CGroupFamily *const	groupFamily = CWorkPtr::groupFamily();
	if	(!groupFamily)
		return;

}

//////////////////////////////////////////////////////////////////////////////
// ContextGroupDesc actions instances                                       //
//////////////////////////////////////////////////////////////////////////////

// C for continent
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_SHEE,C,CGroupFamily);

//static CActionHandler_GT_SHEE<ContextGroupDesc, CGroupFamily> ActionHandler_ContextGroupDesc_GT_SHEE("GT_SHEE""C");

DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_LVLD,C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_SEAS,C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_ACT, C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_APRM,C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_NRG, C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_EQUI,C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_GPRM,C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,BOTTMPL,C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextBotDesc,BT_EQUI,C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextBotDesc,BT_LVLD,C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_GNRJ,C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,POPVER, C,CGroupFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextGroupDesc,GT_END, C,CGroupFamily);
