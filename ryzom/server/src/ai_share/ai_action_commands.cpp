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

#include "ai_actions.h"

using namespace NLMISC;
//using namespace NLNET;
using namespace std;


namespace AI_SHARE 
{

//-------------------------------------------------------------------------
// GENERATING BOT MANAGERS & GROUPS

NLMISC_COMMAND(setMap,"set the active map","<name>")
{
	if (args.size() !=1) return false;

	CAIActions::exec("SET_MAP",args[0]);

	return true;
}

NLMISC_COMMAND(setMgr,"set the active manager within a map","<name>|<slot>")
{
	if (args.size() !=1) return false;

	// see if we have a number or a name
	uint slot;
	NLMISC::fromString(args[0], slot);

	if (toString(slot)!=args[0])
		CAIActions::exec("SET_MGR",slot);
	else
		CAIActions::exec("SET_MGR",-1,args[0]);

	return true;
}

NLMISC_COMMAND(setGrp,"set the active group within a manager","<name>|<slot>")
{
	if (args.size() !=1) return false;

	// see if we have a number or a name
	uint slot;
	NLMISC::fromString(args[0], slot);

	if (toString(slot)!=args[0])
		CAIActions::exec("SET_GRP",slot);
	else
		CAIActions::exec("SET_GRP",-1,args[0]);

	return true;
}


NLMISC_COMMAND(newFaunaManager,"create the fauna manager for a region","<name> [<slot>]")
{
	uint slot;
	switch (args.size())
	{
	case 1:	// <name>
		{
			slot=~0;
			break;
		}

	case 2:	// <name> <slot>
		{
			// the slot id is explicit so make sure its a number
			NLMISC::fromString(args[1], slot);
			if (toString(slot)!=args[1])
				return false;
			break;
		}

	default:
		return false;
	}

	CAIActions::exec("SET_MGR",slot,args[0],"FAUNA");

	return true;
}

NLMISC_COMMAND(newPlaceXYR,"","<place name> <x> <y> <r> <verticalPos>")
{
	if(args.size() !=5)
		return false;

	sint x, y;
	NLMISC::fromString(args[1], x);
	NLMISC::fromString(args[2], y);
	sint r=10; // default value.
	uint32 vp = AITYPES::vp_auto;
	if (!args[3].empty())
	{
		NLMISC::fromString(args[3], r);
	}
	vp = AITYPES::verticalPosFromString(args[4]);

	if (toString(x)+toString(y)+toString(r)!=args[1]+args[2]+args[3])
	{
		nlinfo("Cumulated numeric parameter test failed: '%s' != '%s'",(toString(x)+toString(y)+toString(r)).c_str(),(args[1]+args[2]+args[3]).c_str());
		return false;
	}

	CAIActions::exec("PLACEXYR",args[0],x*1000,y*1000,r*1000, vp);

	return true;
}

NLMISC_COMMAND(newHerbivoreGroup,"","<group name> ")
{
	if(args.size() !=4) return false;

	CAIActions::exec("SET_GRP",-1,args[0],"HERBIVORE");

	return true;
}

NLMISC_COMMAND(newPredatorGroup,"","<group name> <spawn point> <eat point> <rest point>")
{
	if(args.size() !=4) return false;

	CAIActions::exec("SET_GRP",-1,args[0],"PREDATOR");

	return true;
}

NLMISC_COMMAND(newPlantGroup,"","<group name> <spawn point> <eat point> <rest point>")
{
	if(args.size() !=4) return false;

	CAIActions::exec("SET_GRP",-1,args[0],"PLANT");

	return true;
}

NLMISC_COMMAND(newAnimatGroup,"","<group name> <spawn point> <eat point> <rest point>")
{
	if(args.size() !=4) return false;
	
	CAIActions::exec("SET_GRP",-1,args[0],"ANIMAT");
	
	return true;
}

NLMISC_COMMAND(newScavengerGroup,"","<group name> <spawn point> <eat point> <rest point>")
{
	if(args.size() !=4) return false;

	CAIActions::exec("SET_GRP",-1,args[0],"SCAVENGER");

	return true;
}

NLMISC_COMMAND(addHerbivorePopulation,"add a population version to the current group","<creature type> <qty> [<type> <qty>] [...] [...]")
{
	if ( (args.size()&1) || args.size()>8)
		return false;

	std::vector <CAIActions::CArg> executeArgs;

	// push the weight
	executeArgs.push_back(100);
	
	// check that every second argument is a number
	for (uint i=0;i<args.size();i+=2)
	{
		uint32 count;
		NLMISC::fromString(args[i+1], count);
		if (toString(count)!=args[i+1])
			return false;
		executeArgs.push_back(CAIActions::CArg(args[i]));
		executeArgs.push_back(count);
	}
	CAIActions::execute("POP_ADD",executeArgs);

	return true;
}



NLMISC_COMMAND(newUrbanManager,"create the urban npc manager for a sttlement","<name> [<slot>]")
{
	uint slot;
	switch (args.size())
	{
	case 1:	// <name>
		{
			slot=~0;
			break;
		}

	case 2:	// <name> <slot>
		{
			// the slot id is explicit so make sure its a number
			NLMISC::fromString(args[1], slot);
			if (toString(slot)!=args[1])
				return false;
			break;
		}

	default:
		return false;
	}

	CAIActions::exec("SET_MGR",slot,args[0],"URBAN");

	return true;
}

NLMISC_COMMAND(newStationaryGroup,"","<group name>")
{
	if(args.size() !=1) return false;

	CAIActions::exec("SET_GRP",-1,args[0],"STATIONARY");

	return true;
}

NLMISC_COMMAND(npcAddGrpPatrol,"add a group to the manager","<name>")
{
	if(args.size() !=1) return false;

	CAIActions::exec("SET_GRP",-1,args[0],"PATROL");

	return true;
}

NLMISC_COMMAND(npcAddGrpGuard,"add a group to the manager","<name>")
{
	if(args.size() !=1) return false;

	CAIActions::exec("SET_GRP",-1,args[0],"GUARD");

	return true;
}

NLMISC_COMMAND(npcAddGrpWandering,"add a group to the manager","<name>")
{
	if(args.size() !=1) return false;

	CAIActions::exec("SET_GRP",-1,args[0],"WANDER");

	return true;
}

NLMISC_COMMAND(npcGrpSetRoute,"setup a patrol route","<route name>")
{
	if ( args.size()!=1)
		return false;

	CAIActions::exec("SETROUTE",args[0]);

	return true;
}

NLMISC_COMMAND(npcMgrDefaults,"setup the manager dummy bot","")
{
	if ( args.size()!=0)
		return false;

	CAIActions::exec("SET_BOT");

	return true;
}

NLMISC_COMMAND(npcGrpDefaults,"setup the manager dummy bot","")
{
	if ( args.size()!=0)
		return false;

	CAIActions::exec("SET_BOT");

	return true;
}

NLMISC_COMMAND(addNpc,"create a new npc ","<x> <y> <theta>")
{
	if ( args.size()!=3)
		return false;

	uint x=(uint)(1000.0*atof(args[0].c_str()));
	uint y=(uint)(1000.0*atof(args[1].c_str()));
	float theta=(float)atof(args[2].c_str());

	CAIActions::exec("SET_BOT",-1,"npc",x,y,theta);

	return true;
}

NLMISC_COMMAND(npcSetColours,"set the npc clothing colour scheme","<colour scheme>")
{
	if ( args.size()!=1)
		return false;

	CAIActions::exec("COLOURS");

	return true;
}

NLMISC_COMMAND(npcSetName,"set npc name","<name>")
{
	if ( args.size()!=1)
		return false;

	// concatenate the names on the command line into a single string
	std::string s=args[0];
	for (uint i=1;i<args.size();++i)
		s=s+' '+args[i];

	CAIActions::exec("NAME",s);

	return true;
}

NLMISC_COMMAND(npcSetGameForm,"set the .CREATURE sheet to use for game stats","<sheet name>")
{
	if ( args.size()!=1)
		return false;

	CAIActions::exec("GAMEFORM",args[0]);

	return true;
}

NLMISC_COMMAND(npcSetLookForm,"set the .CREATURE sheet to use for look on client","<sheet name>")
{
	if ( args.size()!=1)
		return false;

	CAIActions::exec("LOOKFORM",args[0]);

	return true;
}

NLMISC_COMMAND(npcSetLookType,"set the possible look types","<keyword>")
{
	if ( args.size()!=1)
		return false;

	CAIActions::exec("LOOKTYPE",args[0]);

	return true;
}

NLMISC_COMMAND(npcSetChat,"set the npc chat parameters","<keyword>[...]")
{
	if ( args.size()<1)
		return false;

	// prepare the arguments from the command line for the 'execute' command
	std::vector <CAIActions::CArg> executeArgs;
	for (uint i=0;i<args.size();++i)
		executeArgs.push_back(CAIActions::CArg(args[i]));

	CAIActions::execute("CHATTYPE",executeArgs);

	return true;
}


// THIS LINE EXISTS TO MAKE SURE THE LINKER DOESN'T THROW OUT THIS MODULE AT LINK TIME!!!
bool LinkWithAiActionCommands=false;

} // end of namespace
