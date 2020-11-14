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



// Nel Misc
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/algo.h"

// Local includes
#include "ags_test.h"
#include "actor.h"
#include "actor_manager.h"
#include "macro_manager.h"
#include "position_generator.h"
#include "sheets.h"
#include "mirrors.h"
#include "ai_mgr.h"

#include "nel/ligo/primitive.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

namespace AGS_TEST
{


//-------------------------------------------------------------------------
// Some global variables
uint32 GlobalActorCount=0;
uint32 GlobalActorUpdates=0;
uint32 GlobalActorMoves=0;

NLMISC_VARIABLE(uint32, GlobalActorCount,	"actors"); 
NLMISC_VARIABLE(uint32, GlobalActorUpdates,	"updates"); 
NLMISC_VARIABLE(uint32, GlobalActorMoves,	"moves"); 


//-------------------------------------------------------------------------
// some globals local to this file
static CActor::EActivity	DefaultActivity			= CActor::SQUARE;
static NLLIGO::CPrimZone *	DefaultPatatEvolution	= 0;
static float				DefaultOrientation		= 0.0f;
static float				DefaultMagnetRange		= 50.0f;
static float				DefaultMagnetDecay		= 10.0f;
static float				DefaultAttackDistance	= 0.0f;
static std::string			DefaultBehaviour;
static int					DefaultDialogue			= 0;
static float				DefaultMinSpawnTime		= 25.0f;
static float				DefaultMaxSpawnTime		= 25.0f;

static map<string,string>	PrimOrigin;		// map of prim points and zones to prim filename
static std::string			SpawnName;		// name or coordinates of the spawn point
static CAiMgrFile *			AIFile=0;		// the file to which ai managers are to be added
static CAiMgrInstance *		AIManager=0;	// the ai manager to which entities are to be added
static CAiMgrPopulation *	AIPopulation=0;	// the ai manager to which entities are to be added

//-------------------------------------------------------------------------
// some stuff for manageing the output of AI_MANAGER files
CAiMgrFile		*CurrentAiMgrFile;
CAiMgrInstance	*CurrentAiMgrInstance;


//-------------------------------------------------------------------------
// CLEANING AND INITIALISING THE SYSTEM

// Command for resetting the system
NLMISC_COMMAND(actorReset,"Remove all actors and reset the actor system","")
{
	COMMAND_MACRO_RECORD_TEST

	CActorManager::release();
	CActorManager::init();
	GlobalActorCount = 0;
	return true;
}

/*
// Command for removing actors
NLMISC_COMMAND(actorRemove,"Kill one or more actors","<actor name> [<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	// iterate through the actor names
	for (unsigned i=0;i<args.size();i++)
		CActorManager::killActor(args[i]);

	return true;
}
*/

// Command for loading .prim file
NLMISC_COMMAND(loadPrim,"load a .prim file","<file name>")
{
	if(args.size() !=1) return false;

	CIFile	f(CPath::lookup(args[0]));
	NLLIGO::CPrimRegion	region;
	CIXml	xml;
	xml.init(f);
	region.serial(xml);

	uint	i;
	// adding zones to move manager
	for (i=0; i<region.VZones.size(); ++i)
		CMoveManager::addPrimZone(region.VZones[i]);

	// adding points and zones to the position generator
	for (i=0; i<region.VZones.size(); ++i)
		CPositionGenerator::addSpawnZone(region.VZones[i]);

	for (i=0; i<region.VPoints.size(); ++i)
		CPositionGenerator::addSpawnPoint(region.VPoints[i]);

	// creating a map of point and zone names -> prim file
	for (i=0; i<region.VZones.size(); ++i)
		PrimOrigin[region.VZones[i].getName()]=args[0];

	for (i=0; i<region.VPoints.size(); ++i)
		PrimOrigin[region.VPoints[i].getName()]=args[0];

	return true;
}


//-------------------------------------------------------------------------
// ACTOR CREATION POSITION GENERATOR

// Command for configuring how actors are placed on landscape
NLMISC_COMMAND(actorCreatePattern,"Setup pattern for new actors","<pattern> [...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	if (args[0]==std::string("grid") && args.size()==1)
		{}
	else if (args[0]==std::string("line") && args.size()==1)
		{}
	else if (args[0]==std::string("line") && args.size()==2)
		CPositionGenerator::setSpacing(atoi(args[0].c_str()));
	else
		return false;

	CPositionGenerator::setPattern(args[0]);
	return true;
}

// Command for configuring how actors are placed on landscape
NLMISC_COMMAND(actorCreateSpacing,"Setup spacing for new actors","<spacing>")
{
	if(args.size() !=1) return false;
	COMMAND_MACRO_RECORD_TEST

	CPositionGenerator::setSpacing(atoi(args[0].c_str()));

	return true;
}

// Command for configuring how actors are placed on landscape
NLMISC_COMMAND(actorCreatePosition,"Setup start position for new actors","<x> <y> [<spacing>]")
{
	COMMAND_MACRO_RECORD_TEST

	switch(args.size())
	{
	case 0:		
		return false;
	case 1:
		CPositionGenerator::setPosition(args[0].c_str());
		SpawnName=args[0];
		break;
	case 3:
		CPositionGenerator::setSpacing(atoi(args[2].c_str()));
		// no break - drop through!
	case 2:
		CPositionGenerator::setPosition(atoi(args[0].c_str()), atoi(args[1].c_str()));
		SpawnName=args[0]+" "+args[1];
		break;
	default:
		return false;
	}
	
	return true;
}


//-------------------------------------------------------------------------
// CREATING ACTORS

static CActor *createActor(std::string type,std::string name)
{
	CActor *theNewActor=CActorManager::newActor(type,name);
	if (theNewActor)
	{
		theNewActor->setActivity		(DefaultActivity);
		theNewActor->setPrimZone		(DefaultPatatEvolution);
		theNewActor->setAngle			(DefaultOrientation);
		theNewActor->setMagnetRange		(DefaultMagnetRange,DefaultMagnetDecay);
		theNewActor->setBehaviour		(DefaultBehaviour);
		theNewActor->setAttackDistance	(DefaultAttackDistance);
		theNewActor->setChatSet			(DefaultDialogue);
	}
	return theNewActor;
}

void addPopulation(std::string type,std::string name,uint count)
{
	// if no file has been opened yet then create one
	if (AIFile==0)
	{
		AIFile=new CAiMgrFile;
		AIFile->setName(std::string("ags_test.ai_manager"));
		CAiMgr::addFile(AIFile);
	}

	// create a new ai_manager instance
	AIManager= new CAiMgrInstance;
	AIFile->addChild(AIManager);

	AIManager->setName(name);
	AIManager->setCreatureLimit(count);
	if (DefaultPatatEvolution!=0)
		AIManager->setBoundary(DefaultPatatEvolution->getName());

	// add a population record
	AIPopulation=new CAiMgrPopulation;
	AIManager->addPopulation(AIPopulation);

	AIPopulation->setName(name);
	AIPopulation->setOrientation(DefaultOrientation);
	AIPopulation->setQuantity(count);
	AIPopulation->setSpawnRate(int(DefaultMinSpawnTime*1000.0),int(DefaultMaxSpawnTime*1000.0));
	AIPopulation->setType(NLMISC::CSheetId(type));

	// add a spawn location
	CAiMgrSpawn *spawn=new CAiMgrSpawn;
	AIManager->addSpawn(spawn);

	spawn->setPlace(SpawnName);

	// check whether the spawn or evolution patats are in an unrefferenced prim file
	if (PrimOrigin.find(CPositionGenerator::getSpawnName())!=PrimOrigin.end())
	{
		if (AIFile->getPrim().empty())
			AIFile->setPrim(PrimOrigin[CPositionGenerator::getSpawnName()]);
		else
			if (AIFile->getPrim()!=PrimOrigin[CPositionGenerator::getSpawnName()])
				nlwarning("WARNING: Can only apply one prim file per AI manager - spawn '%s' in '%s' NOT '%s'",
					CPositionGenerator::getSpawnName().c_str(),
					PrimOrigin[CPositionGenerator::getSpawnName()].c_str(),
					AIFile->getPrim().c_str() );
	}

	if (DefaultPatatEvolution!=0)
		if (PrimOrigin.find(DefaultPatatEvolution->getName())!=PrimOrigin.end())
		{
			if (AIFile->getPrim().empty())
				AIFile->setPrim(PrimOrigin[DefaultPatatEvolution->getName()]);
			else
				if (AIFile->getPrim()!=PrimOrigin[DefaultPatatEvolution->getName()])
					nlwarning("WARNING: Can only apply one prim file per AI manager - spawn '%s' in '%s' NOT '%s'",
						DefaultPatatEvolution->getName().c_str(),
						PrimOrigin[DefaultPatatEvolution->getName()].c_str(),
						AIFile->getPrim().c_str() );
		}
}

// Command for creating actors
NLMISC_COMMAND(actorCreate,"Add one or more moving actors","<actor type> <actor name> [<actor name>...]")
{
	if(args.size() <2) return false;
	COMMAND_MACRO_RECORD_TEST

	// store the population information in an AI_MGR file record
	addPopulation(args[0],args[1],args.size()-1);

	// iterate through the actor names
	for (unsigned i=1;i<args.size();i++)
	{
		// create the named actor and add them to the GPMS and other services
/*
		CActor *theNewActor=CActorManager::newActor(args[0],args[i]);
		theNewActor->setActivity		(DefaultActivity);
		theNewActor->setPrimZone		(DefaultPatatEvolution);
		theNewActor->setAngle			(DefaultOrientation);
		theNewActor->setMagnetRange		(DefaultMagnetRange,DefaultMagnetDecay);
		theNewActor->setBehaviour		(DefaultBehaviour);
		theNewActor->setAttackDistance	(DefaultAttackDistance);
		theNewActor->setChatSet			(DefaultDialogue);
*/
		CActor *theNewActor=createActor(args[0],args[i]);
	}

	return true;
}

// Command for creating actors
NLMISC_COMMAND(actorCreateCustom,"Add one or more moving actors","<actor type> [ [-act <activity>] [-pos <posx> <posy>] [-ctop <color>] [-cbot <color>] [-chair <color>] [-rhand <int>] [-lhand <int>] ] <actor name> [<actor name>...]")
{
	if(args.size() <2) return false;
	COMMAND_MACRO_RECORD_TEST
/*
	uint	i;
	for (i=1; i<args.size(); ++i)
	{
	}

	// store the population information in an AI_MGR file record
	addPopulation(args[0],args[1],args.size()-1);

	// iterate through the actor names
	for (unsigned i=1;i<args.size();i++)
	{
		// create the named actor and add them to the GPMS and other services
		CActor *theNewActor=createActor(args[0],args[i]);
	}
*/
	return true;
}

// Command for creating actors
NLMISC_COMMAND(actorCreateGroup,"Add a group of actors all at once","<actor type> <group name> <count>")
{
	if(args.size()!=3 || atoi(args[2].c_str())<=0 || atoi(args[2].c_str())>1024) return false;
	COMMAND_MACRO_RECORD_TEST
	
	CActorGroup *theNewGroup=CActorManager::newActorGroup(args[1]);

	// store the population information in an AI_MGR file record
	addPopulation(args[0],args[1],atoi(args[2].c_str()));

	// iterate through the actor names
	for (int i=0;i<atoi(args[2].c_str());i++)
	{
		// create the named actor and add them to the GPMS and other services
/*
		CActor *theNewActor=CActorManager::newActor(args[0],args[1]+toString(theNewGroup->actorCount()));
		theNewActor->setActivity		(DefaultActivity);
		theNewActor->setPrimZone		(DefaultPatatEvolution);
		theNewActor->setAngle			(DefaultOrientation);
		theNewActor->setMagnetRange		(DefaultMagnetRange,DefaultMagnetDecay);
		theNewActor->setBehaviour		(DefaultBehaviour);
		theNewActor->setAttackDistance	(DefaultAttackDistance);
		theNewActor->setChatSet			(DefaultDialogue);
*/
		CActor *theNewActor=createActor(args[0],args[1]+toString(theNewGroup->actorCount()));

		if (theNewActor)
			theNewGroup->addActor(theNewActor);
	}

	return true;
}


// Command for creating actors
NLMISC_COMMAND(actorCreateMoving,"Add one or more static actors","<actor type> <actor name> [<actor name>...]")
{
	if(args.size() ==1) return false;
	COMMAND_MACRO_RECORD_TEST

	if (args.size()==0)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultActivity = CActor::SQUARE;
	}
	else
	{
		// store the population information in an AI_MGR file record
		addPopulation(args[0],args[1],args.size()-1);

		// iterate through the actor names
		for (unsigned i=1;i<args.size();i++)
		{
			// create the named actor and add them to the GPMS and other services
/*
			CActor *theNewActor= CActorManager::newActor(args[0],args[i]);
			theNewActor->setPrimZone		(DefaultPatatEvolution);
			theNewActor->setAngle			(DefaultOrientation);
			theNewActor->setMagnetRange		(DefaultMagnetRange,DefaultMagnetDecay);
			theNewActor->setBehaviour		(DefaultBehaviour);
			theNewActor->setAttackDistance	(DefaultAttackDistance);
			theNewActor->setChatSet			(DefaultDialogue);
*/
			CActor *theNewActor=createActor(args[0],args[i]);

			if (theNewActor)
			{
				theNewActor->doSquare();
				theNewActor->setActivity(CActor::SQUARE);
			}
		}
	}

	return true;
}


// Command for creating actors
NLMISC_COMMAND(actorCreateStationary,"Add one or more static actors","<actor type> [<actor name>...]")
{
	if(args.size() ==1) return false;
	COMMAND_MACRO_RECORD_TEST

	if (args.size()==0)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultActivity = CActor::NOTHING;
	}
	else
	{
		// store the population information in an AI_MGR file record
		addPopulation(args[0],args[1],args.size()-1);
								
		// iterate through the actor names
		for (unsigned i=1;i<args.size();i++)
		{
			// create the named actor and add them to the GPMS and other services
/*
			CActor *theNewActor= CActorManager::newActor(args[0],args[i]);
			theNewActor->setPrimZone		(DefaultPatatEvolution);
			theNewActor->setAngle			(DefaultOrientation);
			theNewActor->setMagnetRange		(DefaultMagnetRange,DefaultMagnetDecay);
			theNewActor->setBehaviour		(DefaultBehaviour);
			theNewActor->setAttackDistance	(DefaultAttackDistance);
			theNewActor->setChatSet			(DefaultDialogue);
*/
			CActor *theNewActor=createActor(args[0],args[i]);

			if (theNewActor)
			{
				theNewActor->doNothing();
				theNewActor->setActivity(CActor::NOTHING);
			}
		}
	}

	return true;
}


// Command for creating actors
NLMISC_COMMAND(actorCreateWandering,"Add one or more static actors","<actor type> <actor name> [<actor name>...]")
{
	if(args.size() ==1) return false;
	COMMAND_MACRO_RECORD_TEST

	if (args.size()==0)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultActivity = CActor::WANDER;
	}
	else
	{
		// store the population information in an AI_MGR file record
		addPopulation(args[0],args[1],args.size()-1);

		// iterate through the actor names
		for (unsigned i=1;i<args.size();i++)
		{
			// create the named actor and add them to the GPMS and other services
/*
			CActor *theNewActor= CActorManager::newActor(args[0],args[i]);
			theNewActor->setPrimZone		(DefaultPatatEvolution);
			theNewActor->setAngle			(DefaultOrientation);
			theNewActor->setMagnetRange		(DefaultMagnetRange,DefaultMagnetDecay);
			theNewActor->setBehaviour		(DefaultBehaviour);
			theNewActor->setAttackDistance	(DefaultAttackDistance);
			theNewActor->setChatSet			(DefaultDialogue);
*/
			CActor *theNewActor=createActor(args[0],args[i]);

			if (theNewActor)
			{
				theNewActor->doWander();
				theNewActor->setActivity(CActor::WANDER);
			}
		}
	}

	return true;
}


// Command for moving actor from a group to another
NLMISC_COMMAND(moveActorsToGroup,"Move actors from a group to another (source group is emptied). If no source specified, all actors from defaultGroup assumed.","[<source group>] <dest group>")
{
	COMMAND_MACRO_RECORD_TEST

	if (args.size() < 1)
		return false;

	if (args.size() > 2)
		return false;

	if (args.size() == 1)
		CActorManager::setActorsToGroup("defaultGroup", args[0]);
	else
		CActorManager::setActorsToGroup(args[0], args[1]);

	return true;
}


//-------------------------------------------------------------------------
// MANAGING EXISTING ACTORS

// Command for setting an actor PrimZone
NLMISC_COMMAND(actorSetPatatEvolution,"Set actor patat evolution","<patat name> [<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	NLLIGO::CPrimZone	*primZone = CMoveManager::getPrimZone(args[0]);
	if (primZone == NULL)
	{
		nlwarning("Can't find prim zone '%s'", args[0].c_str());
		return false;
	}

	if (args.size()==1)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultPatatEvolution = primZone;
	}
	else
	{
		// apply this evolution patat to the last manager stored in ai manager
		if (AIManager)
			AIManager->setBoundary(args[0]);

		// iterate through the actor names
		uint	i;
		for (i=1;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				actor->setPrimZone(primZone);
		}
	}

	return true;
}



// Command for setting actor orientations
NLMISC_COMMAND(actorSetOrientation,"Add one or more static actors","<direction> [<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	const char *angleNames[]= {"E", "ENE", "NE", "NNE", "N","NNW","NW","WNW","W","WSW","SW","SSW","S","SSE","SE","ESE"};
	unsigned i;
	float theta;

#ifndef NL_OS_WINDOWS
#define stricmp strcasecmp
#endif

	for (i=0; i<sizeof(angleNames)/sizeof(angleNames[0]) && 
		stricmp(args[0].c_str(),angleNames[i]); i++) 
		;
	if (i<(sizeof(angleNames)/sizeof(angleNames[0])))
		theta=3.14159265359f*2.0f*(float)i/(float)(sizeof(angleNames)/sizeof(angleNames[0]));
	else
	{
		NLMISC::fromString(args[0], theta);
		theta = theta / 360.0f*2.0f*3.14159265359f;
	}

	if (args.size()==1)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultOrientation = theta;
	}
	else
	{
		// apply this orientation to the last manager stored in ai manager
		if (AIPopulation)
			AIPopulation->setOrientation(theta);

		// iterate through the actor names
		for (i=1;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				actor->setAngle(theta);
		}
	}

	return true;
}


// Command for setting actor magnet range
NLMISC_COMMAND(actorSetMagnetRange,"Set the magnet properties for wandering behaviour","<range> [<decay>] [<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	float range;
	NLMISC::fromString(args[0], range);
	if (range<=0.0f) return false;

	unsigned start;
	float decay;
	NLMISC::fromString(args[0], decay);
	if (decay<0.0f) return false;
	if (decay==0.0f) 
	{
		decay=10.0f;
		start=1;
	}
	else
		start=2;
	if((sint)args.size() <(start+1)) return false;

	if (args.size()<start)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultMagnetRange = range;
		if (start>1)
			DefaultMagnetDecay = decay;
	}
	else
	{
		// iterate through the actor names
		for (unsigned i=start;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				actor->setMagnetRange(range,decay);
		}
	}

	return true;
}

// Command for setting actor attack distance
NLMISC_COMMAND(actorSetAttackDistance,"set actor attack distance","<distance> [<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	float distance;
	NLMISC::fromString(args[0], distance);
//	if (distance<=0.0f) return false;

	if (args.size()<2)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultAttackDistance = distance;
	}
	else
	{
		// iterate through the actor names
		for (unsigned i=1;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				actor->setAttackDistance(distance);
		}
	}

	return true;
}

// Command for setting actor mode
/*NLMISC_COMMAND(actorSetMode,"Set actor mode","<mode> <actor name>[...]")
{
	if(args.size() <2) return false;
	COMMAND_MACRO_RECORD_TEST

	// iterate through the named actors setting their mode
	for (unsigned i=1;i<args.size();i++)
	{
		CActor *actor=CActorManager::getActor(args[i]);
		if (actor==0)
			nlinfo("Unknown actor: '%s'",args[i].c_str());
		else
			actor->setMode(args[0]);
	}

	return true;
}*/

// Command for setting actor behaviour
NLMISC_COMMAND(actorSetBehaviour,"Set actor behaviour","<behaviour> [<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	if (args.size()<2)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultBehaviour = args[0];
	}
	else if (args.size() == 2 && args[1] == "all")
	{
		uint	num = CActorManager::numActors();
		uint	i;
		for (i=0; i<num; ++i)
		{
			CActor *actor=CActorManager::getActor(i);
			if (actor != NULL)
				actor->setBehaviour(args[0]);
		}
	}
	else
	{
		// iterate through the named actors setting their behaviour
		for (unsigned i=1;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				actor->setBehaviour(args[0]);
		}
	}

	return true;
}

// Command for setting actor dialogue for bot chat
NLMISC_COMMAND(actorSetDialogue,"Set actor bot-chat dialogue","<dialogue set> [<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	int dialogue=atoi(args[0].c_str());

	if (args.size()<2)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultDialogue = dialogue;
	}
	else
	{
		// iterate through the named actors setting their behaviour
		for (unsigned i=1;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				actor->setChatSet(dialogue);
		}
	}

	return true;
}


// Command for setting actor respawn time (when dead)
NLMISC_COMMAND(actorSetRespawnDelay,"set actor respawn delay","<min time> [<max time>] [<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	float min;
	NLMISC::fromString(args[0], min);
	if (min<=0.0f) return false;

	unsigned start;
	float max;
	NLMISC::fromString(args[0], max);
	if (max==0.0f) 
	{
		max=min;
		start=1;
	}
	else
	{
		if (max<min)
			return false;
		start=2;
	}
	if((sint)args.size() <(start+1)) return false;

	if (args.size()<start)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultMinSpawnTime = min;
		if (start>1)
			DefaultMaxSpawnTime = max;
	}
	else
	{
		// apply this orientation to the last manager stored in ai manager
		if (AIPopulation)
			AIPopulation->setSpawnRate(int(1000.0*min),int(1000.0*max));

		// iterate through the actor names
		for (unsigned i=start;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				actor->setSpawnTime(min,max);
		}
	}

	return true;
}


// Command for making actors walk in a square pattern
NLMISC_COMMAND(actorSetActivitySquare,"Make actors walk in a square shape","[<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	if (args.size()==0)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultActivity = CActor::SQUARE;
	}
	else if (args.size() == 1 && args[0] == "all")
	{
		uint	num = CActorManager::numActors();
		uint	i;
		for (i=0; i<num; ++i)
		{
			CActor *actor=CActorManager::getActor(i);
			if (actor != NULL)
				actor->setActivity(CActor::SQUARE);
		}
	}
	else
	{
		// iterate through the actor names
		for (unsigned i=0;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				CActorManager::getActor(args[i])->setActivity(CActor::SQUARE);
		}
	}

	return true;
}

// Command for making actor(s) wander about randomly
NLMISC_COMMAND(actorSetActivityWander,"Make actors walk in a square shape","[<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	if (args.size()==0)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultActivity = CActor::WANDER;
	}
	else if (args.size() == 1 && args[0] == "all")
	{
		uint	num = CActorManager::numActors();
		uint	i;
		for (i=0; i<num; ++i)
		{
			CActor *actor=CActorManager::getActor(i);
			if (actor != NULL)
				actor->setActivity(CActor::WANDER);
		}
	}
	else
	{
		// iterate through the actor names
		for (unsigned i=0;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				CActorManager::getActor(args[i])->setActivity(CActor::WANDER);
		}
	}

	return true;
}

// Command for making actor(s) stand still
NLMISC_COMMAND(actorSetActivityStationary,"Make actors stop walking/ fighting","[<actor name>...]")
{
	if(args.size() <1) return false;
	COMMAND_MACRO_RECORD_TEST

	if (args.size()==0)
	{
		// no actor names specified so setup the default value to apply to new actors
		DefaultActivity = CActor::NOTHING;
	}
	else if (args.size() == 1 && args[0] == "all")
	{
		uint	num = CActorManager::numActors();
		uint	i;
		for (i=0; i<num; ++i)
		{
			CActor *actor=CActorManager::getActor(i);
			if (actor != NULL)
				actor->setActivity(CActor::NOTHING);
		}
	}
	else
	{
		// iterate through the actor names
		for (unsigned i=0;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				nlinfo("Unknown actor: '%s'",args[i].c_str());
			else
				CActorManager::getActor(args[i])->setActivity(CActor::NOTHING);
		}
	}

	return true;
}

// Command for making actors attack entities
NLMISC_COMMAND(actorFight,"Make actors fight eachother","<actor or group name> <actor or group name>")
{
	if(args.size()!=2) return false;
	COMMAND_MACRO_RECORD_TEST

	CActor *actor0=CActorManager::getActor(args[0]);
	CActor *actor1=CActorManager::getActor(args[1]);
	CActorGroup *group0=CActorManager::getActorGroup(args[0]);
	CActorGroup *group1=CActorManager::getActorGroup(args[1]);


	if (actor0)
	{
		if (actor1)
		{
			actor0->doFight(actor1);
			nlinfo("ACTOR %s Attacking ACTOR: %s",args[0].c_str(),args[1].c_str());
//			actor1->doFight(actor0);
		}
		else if (group1)
		{
			actor0->doFight(group1);
			nlinfo("ACTOR %s Attacking GROUP: %s",args[0].c_str(),args[1].c_str());
//			group1->doFight(actor0);
		}
		else
		{
			CEntityId id;
			id.fromString( args[1].c_str() );
			actor0->doFight(id);
			nlinfo("ACTOR %s Attacking entity with SID: %s",args[0].c_str(),args[1].c_str());
		}
	}
	else if (group0)
	{
		if (actor1)
		{
			group0->doFight(actor1);
			nlinfo("GROUP %s Attacking ACTOR: %s",args[0].c_str(),args[1].c_str());
//			actor1->doFight(group0);
		}
		else if (group1)
		{
			group0->doFight(group1);
			nlinfo("GROUP %s Attacking GROUP: %s",args[0].c_str(),args[1].c_str());
//			group1->doFight(group0);
		}
		else
		{
			CEntityId id;
			id.fromString( args[1].c_str() );
			//group0->doFight(id);
			nlinfo("GROUP %s Cannot attack: %s (no actor or group of this name found in this service)",args[0].c_str(),args[1].c_str());
		}
	}
	else nlinfo("Actor or group doesn't exist: %s",args[0].c_str());

	return true;
}

// Command for displaying all of or named actors
NLMISC_COMMAND(actorDisplay,"List the actors","[<actor name>...]")
{
	COMMAND_MACRO_RECORD_TEST

	if(args.size() <1)
	{
		// list all the actors' stats
		for (unsigned i=0;i<CActorManager::numActors();++i)
			CActorManager::getActor(i)->display();
	}
	else
	{
		// iterate through the named actors listing their stats
		for (unsigned i=0;i<args.size();i++)
		{
			CActor *actor=CActorManager::getActor(args[i]);
			if (actor==0)
				log.displayNL("Unknown actor: '%s'",args[i].c_str());
			else
				actor->display(&log);
		}
	}

	return true;
}

//-------------------------------------------------------------------------
// MANAGING AI FILES

// command to display ai manager hierachy
NLMISC_COMMAND(aiDisplay,"Display the ai manager hierachy","")
{
	if(args.size()!=0) return false;
	COMMAND_MACRO_RECORD_TEST

	CAiMgr::display(0, &log);
	return true;
}

// command to set the output file for ai populations
NLMISC_COMMAND(aiFile,"Set the file name for the following ai populations","<file name>")
{
	if(args.size()!=1) return false;
	COMMAND_MACRO_RECORD_TEST

	AIFile=new CAiMgrFile;
	AIFile->setName(args[0]+std::string(".ai_manager"));
	CAiMgr::addFile(AIFile);
	return true;
}

// command to write ai manager hierachy to files
NLMISC_COMMAND(aiWrite,"Write the ai manager hierachy to disk","")
{
	if(args.size()!=0) return false;
	COMMAND_MACRO_RECORD_TEST

	CAiMgr::write();
	return true;
}

// command to read ai manager hierachy from files
NLMISC_COMMAND(aiRead,"Read the ai manager hierachy from disk","<path>")
{
	if(args.size()!=1) return false;
	COMMAND_MACRO_RECORD_TEST

	CAiMgr::read(args[0],true);
	return true;
}


//-------------------------------------------------------------------------
// MANAGING MACROS

// Command for creating a new macro
NLMISC_COMMAND(macroRecord,"Start recording a new macro","<macro name>")
{
	if(args.size() !=1)
		return false;

	CMacroManager::recordMacro(args[0]);

	return true;
}


// Command for adding a line to a macro
NLMISC_COMMAND(macroStopRecord,"stop macro recording","")
{
	if(args.size() !=0)
		return false;

	CMacroManager::stopRecord();

	return true;
}


// Command for executing a macro
NLMISC_COMMAND(macroExecute,"Execute one or more macros","<macro name> [<macro name>...]")
{
	if(args.size() <1)
		return false;

	for (unsigned i=0;i<args.size();++i)
		CMacroManager::execute(args[i],log);

	return true;
}


// Command for executing a macro
NLMISC_COMMAND(macroDisplay,"Display list of macros or contents of the given macros","")
{
	if(args.size()==0)
		CMacroManager::listMacros(&log);
	else
		for (unsigned i=0;i<args.size();++i)
			CMacroManager::displayMacro(args[i], &log);


	return true;
}

// Command for executing a macro
NLMISC_COMMAND(sheetDisplay,"Display list of sheets read","")
{
	CSheets::display(&log);
	return true;
}



//
NLMISC_COMMAND(generateScript, "Generate a sript that spawn a given type of creature", "<script_name> <spawn x> <spawn y> <spacing> <width> <extension> [<filter> ...]")
{
	if (args.size() < 6)
		return false;

	const string				&scriptName = args[0];
	double						spawnX;
	NLMISC::fromString(args[1], spawnX);
	double						spawnY;
	NLMISC::fromString(args[2], spawnY);
	double						spacing;
	NLMISC::fromString(args[3], spacing);
	uint						width;
	NLMISC::fromString(args[4], width);
	const string				&extension = args[5];
	vector<string>				filters;

	uint	i;
	for (i=6; i<args.size(); ++i)
		filters.push_back(args[i]);

	vector<string>				files;
	CPath::getFileList(extension, files);

	if (!filters.empty())
	{
		vector<string>::iterator	itr;
		for (itr=files.begin(); itr!=files.end(); )
		{
			bool	match = false;

			if (files[i][0] != '_')
			{
				for (i=0; !match && i<filters.size(); ++i)
					if (testWildCard(*itr, filters[i]))
						match = true;
			}

			if (match)
				++itr;
			else
				itr = files.erase(itr);
		}
	}

	if (files.empty())
		return false;

	char	buffer[2048];

	COFile						script;
	if (!script.open(scriptName))
		return false;

	smprintf(buffer, 2048, "! %s\n", scriptName.c_str());
	script.serialBuffer((uint8*)buffer, strlen(buffer));
	smprintf(buffer, 2048, "! Generated using args: %s %g %g %g %d %s\n\n", scriptName.c_str(), spawnX, spawnY, spacing, width, extension.c_str());
	script.serialBuffer((uint8*)buffer, strlen(buffer));

	if (!filters.empty())
	{
		smprintf(buffer, 2048, "! Filters are:");
		script.serialBuffer((uint8*)buffer, strlen(buffer));
		for (i=0; i<filters.size(); ++i)
		{
			smprintf(buffer, 2048, " %s", filters[i].c_str());
			script.serialBuffer((uint8*)buffer, strlen(buffer));
		}
		smprintf(buffer, 2048, "\n\n");
		script.serialBuffer((uint8*)buffer, strlen(buffer));
	}

	smprintf(buffer, 2048, "! %d actor sheets found\n\n", files.size());
	script.serialBuffer((uint8*)buffer, strlen(buffer));

	smprintf(buffer, 2048, "cmd actorCreateStationary\ncmd actorSetOrientation 0\n\n");
	script.serialBuffer((uint8*)buffer, strlen(buffer));

	double	cx = spawnX, cy = spawnY;
	uint	inLine = 0;

	for (i=0; i<files.size(); ++i)
	{
		string		fileName = CFile::getFilename(files[i]);
		string		name = CFile::getFilenameWithoutExtension(fileName);

		smprintf(buffer, 2048, "cmd actorCreatePosition %d %d\ncmd actorCreate %s %s\n", (int)cx, (int)cy, fileName.c_str(), name.c_str());
		script.serialBuffer((uint8*)buffer, strlen(buffer));

		++inLine;
		cx += spacing;

		if (inLine >= width)
		{
			cx = spawnX;
			cy += spacing;
			inLine = 0;
		}
	}

	script.close();

	return true;
}

//
NLMISC_COMMAND(generateAndRun, "Generate and run a sript that spawn a given type of creature", "<spawn x> <spawn y> <spacing> <width> <extension> [<filter> ...]")
{
	string	scriptfile = IService::getInstance()->WriteFilesDirectory.toString()+"ags_generated.script";
	string	commandLine = "generateScript "+scriptfile;

	uint	i;
	for (i=0; i<args.size(); ++i)
		commandLine += string(" ")+args[i];

	// remove file first
	CFile::deleteFile(scriptfile);

	// generate script
	ICommand::execute(commandLine, log);

	// run script
	ICommand::execute("runScript ags_generated "+scriptfile, log);

	return true;
}

//
NLMISC_COMMAND(checkGenerateAndRun, "Generate and run a sript that spawn a given type of creature", "<script_name> <spawn x> <spawn y> <spacing> <width> <extension> [<filter> ...]")
{
	if (args.size() < 6)
		return false;

	string	scriptName = args[0];
	string	filename = scriptName+".script";

	string	scriptfile = CPath::lookup(filename, false, false);

	if (scriptfile.empty())
	{
		if (!CFile::isExists("data_shard/"))
			CFile::createDirectory("data_shard/");
		if (!CFile::isExists("data_shard/ags_script/"))
			CFile::createDirectory("data_shard/ags_script/");

		scriptfile = string("data_shard/ags_script/")+filename;

		string	commandLine = "generateScript "+scriptfile;
		uint	i;
		for (i=1; i<args.size(); ++i)
			commandLine += string(" ")+args[i];

		// generate script
		ICommand::execute(commandLine, log);
	}

	// run script
	ICommand::execute("runScript "+scriptName+" "+scriptfile, log);

	return true;
}

//
//NLMISC_COMMAND

NLMISC_COMMAND(setTargets, "set targets for a multi cast", "<caster entityId> <cast_time> [<target1 entityId> ...]")
{
	if (args.size() < 2)
		return false;

	CEntityId		caster;
	list<CEntityId>	targets;

	caster.fromString(args[0].c_str());

	CMirrorPropValueList<TDataSetRow>	targetList(TheDataset, caster, "TargetList");

	targetList.clear();

	uint	i;
	for (i=args.size()-1; i>=2; --i)
	{
		CEntityId	target;
		target.fromString(args[i].c_str());
		TDataSetRow	targetIndex = CMirrors::DataSet->getDataSetRow(target);

		targetList.push_front(targetIndex);
	}

	TGameCycle		castTime = CTickEventHandler::getGameCycle() + atoi(args[1].c_str());
	targetList.push_front(*((TDataSetRow*)(&castTime)));

	return true;
}

//-------------------------------------------------------------------------
} // end of namespace AGS_TEST


