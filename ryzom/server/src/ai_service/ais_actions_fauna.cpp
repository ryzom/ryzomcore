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
#include "ai_grp_fauna.h"

#include "continent_inline.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace CAISActionEnums;
using namespace	AITYPES;


//---------------------------------------------------------------------------------------
// Stuff used for management of log messages

static bool VerboseLog=false;
#define LOG if (!VerboseLog) {} else nlinfo


//----------------------------------------------------------------------------
// The FAUNA_MGR context
//----------------------------------------------------------------------------

//	DEFINE_ACTION(ContextFaunaMgr,PLACEXYR)
DEFINE_ACTION(ContextFaunaGrp,PLACEXYR)
{
	// set the working place slot
	// args: string name float x, float y, sint r

	if (!CWorkPtr::grpFauna())
		return;

	// get hold of the parameters and check their validity
	float x,y;
	sint32 r;
	uint32 alias;
	uint32 verticalPos;
	if (!getArgs(args,name(), alias, x, y, r, verticalPos))
		return;

	// try to get a pointer to the place and create a new place if need be
	CAIPlaceXYR	*const	place=dynamic_cast<CAIPlaceXYR *>(CWorkPtr::grpFauna()->places().getChildByAlias(alias));
	if (!place)
	{
		nlwarning("Unable to set placeXYR coords (%.3f,%.3f,%d) as place alias %u not found in manager",x,y,r,alias);
		return;
	}

	// set the place coordinates
	place->setPosAndRadius((TVerticalPos)verticalPos, CAIPos(x,y,0,0.0f),r);
}

// faune places specifics
DEFINE_ACTION(ContextFaunaGrp,PLXYRFAF)
{
	// set the working place slot
	// args: string name float x, float y, sint r

	if (!CWorkPtr::grpFauna())
		return;

	// get hold of the parameters and check their validity
	uint32 alias;
	bool flagSpawn;
	bool flagRest;
	bool flagFood;	
	if (!getArgs(args,name(), alias, flagSpawn, flagRest, flagFood))
		return;

	// try to get a pointer to the place and create a new place if need be
	CAIPlaceXYRFauna *const	place=dynamic_cast<CAIPlaceXYRFauna *>(CWorkPtr::grpFauna()->places().getChildByAlias(alias));
	if (!place)
	{
		nlwarning("Unable to set placeXYRFauna flags as place alias %u not found in manager", alias);
		return;
	}

	// set the place coordinates
	place->setFlag(CAIPlaceXYRFauna::FLAG_SPAWN, flagSpawn);
	place->setFlag(CAIPlaceXYRFauna::FLAG_REST, flagRest);
	place->setFlag(CAIPlaceXYRFauna::FLAG_EAT, flagFood);	
}

DEFINE_ACTION(ContextFaunaGrp,PLXYRFAS)
{
	// set the working place slot
	// args: string name float x, float y, sint r

	if (!CWorkPtr::grpFauna())
		return;

	// get hold of the parameters and check their validity
	uint32 alias;
	uint32 stayTimeMin, stayTimeMax;
	if (!getArgs(args,name(), alias, stayTimeMin, stayTimeMax))
		return;

	// try to get a pointer to the place and create a new place if need be
	CAIPlaceXYRFauna *const	place=dynamic_cast<CAIPlaceXYRFauna *>(CWorkPtr::grpFauna()->places().getChildByAlias(alias));
	if (!place)
	{
		nlwarning("Unable to set placeXYRFauna stay times as place alias %u not found in manager", alias);
		return;
	}

	// set the place coordinates
	place->setMinStayTime(stayTimeMin);
	place->setMaxStayTime(stayTimeMax);
}

DEFINE_ACTION(ContextFaunaGrp,PLXYRFAI)
{
	// set the working place slot
	// args: string name float x, float y, sint r

	if (!CWorkPtr::grpFauna())
		return;

	// get hold of the parameters and check their validity
	uint32 alias;
	uint32 index;
	std::string indexNext;
	if (!getArgs(args,name(), alias, index, indexNext))
		return;
	// try to get a pointer to the place and create a new place if need be
	CAIPlaceXYRFauna *place = dynamic_cast<CAIPlaceXYRFauna *>(CWorkPtr::grpFauna()->places().getChildByAlias(alias));
	if (!place)
	{
		nlwarning("Unable to set placeXYRFauna arcs as place alias %u not found in manager", alias);
		return;
	}

	std::vector<std::string> reachableIndicesStr;
	NLMISC::explode(indexNext, string(","), reachableIndicesStr);
	std::set<sint32> arcs;
	for(uint k = 0; k < reachableIndicesStr.size(); ++k)
	{			
		if (NLMISC::nlstricmp(reachableIndicesStr[k], "next") == 0)
		{
			place->setReachNext(true);
		}
		int currentIndex;
		if (sscanf(reachableIndicesStr[k].c_str(), "%d", &currentIndex) != 1) continue;
		arcs.insert((sint32) currentIndex);
	}
	std::vector<sint32> arcVect(arcs.begin(), arcs.end());
	place->setIndex(index);
	place->setArcs(arcVect);		
}


// activation related parameters
DEFINE_ACTION(ContextFaunaGrp,PLXYRFAA)
{
	// set the working place slot
	// args: string name float x, float y, sint r

	if (!CWorkPtr::grpFauna())
		return;

	// get hold of the parameters and check their validity
	uint32 alias;
	bool active;
	bool timeDriven;
	std::string timeInterval;
	std::string dayInterval;
	if (!getArgs(args,name(), alias, active, timeDriven, timeInterval, dayInterval))
		return;

	// try to get a pointer to the place and create a new place if need be
	CAIPlaceXYRFauna *const	place=dynamic_cast<CAIPlaceXYRFauna *>(CWorkPtr::grpFauna()->places().getChildByAlias(alias));
	if (!place)
	{
		nlwarning("Unable to set placeXYRFauna stay times as place alias %u not found in manager", alias);
		return;
	}
	// set activation parameters
	place->setTimeDriven(timeDriven);
	if (!timeDriven)
	{
		place->setActive(active);
	}
	else
	{
		place->setTimeInterval(timeInterval);
		place->setDayInterval(dayInterval);
	}
}



DEFINE_ACTION(ContextFaunaMgr,GRPFAUNA)
{
	// set the working grp
	// args: uint slot_in_manager [, string name [, string type]]

	if (!CWorkPtr::mgrFauna())
		return;

	// get hold of the parameters and check their validity
	uint32 alias;
	std::string type;
	if	(!getArgs(args,name(),alias))
		return;

	// scan the manager for a group with the right alias
	for	(uint i=0;i<CWorkPtr::mgrFauna()->groups().size();++i)
	{
		CGroup*const grp	=	CWorkPtr::mgrFauna()->groups()[i];	//getGrpFauna(i);
		if	(grp->getAlias()!=alias)
			continue;

		// setup the working group pointer and exit
		CWorkPtr::grp(grp);
		// push the group context onto the context stack
		CContextStack::setContext(ContextFaunaGrp);
		return;
	}

	// no match found so throw a warning and invalidate grp pointer
	nlwarning("GRPFAUNA failed because no group with alias %u found in manager",alias);
	CWorkPtr::grp(NULL);
}

//----------------------------------------------------------------------------
// The FAUNA_GRP context
//----------------------------------------------------------------------------

DEFINE_ACTION(ContextFaunaGrp,AUTOSPWN)
{
	// set the feed and rest times
	// args: float time0, float time1

	if(!CWorkPtr::grpFauna())
		return;
	
	uint32 autoSpawn;
	if (!getArgs(args, name(), autoSpawn))
		return;
	CWorkPtr::grpFauna()->setAutoSpawn(autoSpawn != 0);
	LOG("AutoSpawn : %s", autoSpawn ? "true" : "false");
}

DEFINE_ACTION(ContextFaunaGrp,SETTIMES)
{
	// set the feed and rest times
	// args: float time0, float time1

	if(!CWorkPtr::grpFauna())
		return;

	// get hold of the parameters and check their validity
	double time0, time1;
	if (!getArgs(args,name(),time0,time1))
		return;

	time0*=10;	//	quick hack !
	time1*=10;	//	quick hack !
	
	CWorkPtr::grpFauna()->setTimer	(CGrpFauna::EAT_TIME,	(uint32) time0);
	CWorkPtr::grpFauna()->setTimer	(CGrpFauna::REST_TIME,	(uint32) time1);
	LOG("Set timers: eat=%i sleep=%i",(uint32) time0, (uint32) time1);
}

DEFINE_ACTION(ContextFaunaGrp, ASSIST)
{
	// set the assist mode for the group
	// args: bool assist

	if(!CWorkPtr::grpFauna())
		return;

	// get hold of the parameters and check their validity
	uint32 assist;
	if (!getArgs(args,name(),assist))
		return;

//	CWorkPtr::grpFauna()->setSolidarity(assist!=0);
}


DEFINE_ACTION(ContextFaunaGrp, STCYCLES)
{
	// set the respawn and corpse time
	// args: float time0, float time1
	
	if	(!CWorkPtr::grpFauna())
		return;
	
	// get hold of the parameters and check their validity
	std::string	cycles;
	if	(!getArgs(args,name(),cycles))
		return;
	
	CWorkPtr::grpFauna()->setCyles(cycles);
	
	LOG("Set Cycles: %s", cycles.c_str());
}


DEFINE_ACTION(ContextFaunaGrp, SPAWTIME)
{
	// set the respawn and corpse time
	// args: float time0, float time1

	if(!CWorkPtr::grpFauna())
		return;

	// get hold of the parameters and check their validity
	double time0, time1, time2= -1;
	if(args.size()>=3)
	{
		if	(!getArgs(args,name(),time0,time1,time2))
			return;
	}
	else
	{
		if	(!getArgs(args,name(),time0,time1))
			return;
	}

	CWorkPtr::grpFauna()->setTimer	( CGrpFauna::SPAWN_TIME,	(uint32) time0);
	CWorkPtr::grpFauna()->setTimer	( CGrpFauna::CORPSE_TIME,	(uint32) time1);
	// If the LevelDesigner has specified a correct respawn time
	if(time2>=0)
	{
		CWorkPtr::grpFauna()->setTimer	( CGrpFauna::RESPAWN_TIME,	(uint32) time2);
		LOG("Set spawn timers: spawn=%i corpse=%i respawn=%i",(uint32) time0, (uint32) time1, (uint32) time2);
	}
	else
	{
		LOG("Set spawn timers: spawn=%i corpse=%i respawn=45",(uint32) time0, (uint32) time1);
	}
}

DEFINE_ACTION(ContextFaunaGrp,POPVER)
{
	// add a population version for a group
	// args: uint32 alias, string spawn_type, uint weight, (string sheet, uint32 count)+

	if(!CWorkPtr::grpFauna())
		return;

	const uint32 fixedArgsCount=3;
	if (args.size()<fixedArgsCount+2 || ((args.size()-fixedArgsCount)&1)==1)
	{
		nlwarning("POPVER action FAILED due to bad number of arguments (%d)",args.size());
		return;
	}
	
	std::string		spawnTypeStr;
	std::string		cycles;
	uint32			alias;
	uint32			weight;
	TSpawnType	spawntype;

	if (	args[0].get(alias)==false
		||	args[1].get(spawnTypeStr)==false
		||	args[2].get(weight)==false)
	{
		nlwarning("POPVER FAILED due to failure to read alias, spawn type & weight at arguments 0, 1 & 2");
		return;
	}

	// convert the spawn type to an enum value and check validity
	getType(spawntype,spawnTypeStr.c_str());
	if (spawntype==SpawnTypeBadType)
	{
		nlwarning("POP_ADD action FAILED due to unknown spawn type: '%s'",spawnTypeStr.c_str());
		return;
	}

	CPopulation	*pop	=	new	CPopulation(CWorkPtr::grpFauna(),alias,"Population");
	pop->setSpawnType(spawntype);

	pop->setWeight(weight);
	
	// get hold of the parameters and check their validity
	for (uint i=fixedArgsCount;i+1<args.size();i+=2)
	{
		std::string	sheet;
		uint32	count;

		if	(	!args[i].get(sheet)
			||	!args[i+1].get(count))
		{
			nlwarning("POPVER Add Record FAILED due to bad arguments");
			continue;
		}
		
		CSheetId sheetId(sheet);
		if (sheetId==CSheetId::Unknown)
		{
			nlwarning("POPVER Add Record Invalid sheet: %s",sheet.c_str());
			continue;
		}
		
		AISHEETS::ICreatureCPtr const sheetPtr = AISHEETS::CSheets::getInstance()->lookup(sheetId);
		if (!sheetPtr)
		{
			nlwarning("POPVER Add Record Invalid sheet: %s",sheet.c_str());
			continue;
		}
		pop->addPopRecord(CPopulationRecord(sheetPtr,count));
	}
	
	// create the population
	CWorkPtr::grpFauna()->setPopulation(pop);
}

//---------------------------------------------------------------------------------------
// Control over verbose nature of logging
//---------------------------------------------------------------------------------------

NLMISC_COMMAND(verboseFaunaParseLog,"Turn on or off or check the state of verbose parser activity logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			VerboseLog=true;

		if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			VerboseLog=false;
	}

	nlinfo("verboseLogging is %s",VerboseLog?"ON":"OFF");
	return true;
}

//---------------------------------------------------------------------------------------
