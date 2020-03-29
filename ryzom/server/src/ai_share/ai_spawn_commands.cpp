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
#include "ai_spawn_commands.h"

// Fix the stupid Visual 6 Warning
void foo_ai_spawn_commands() {};


using namespace NLMISC;
using namespace std;


namespace AI_SHARE 
{

//-------------------------------------------------------------------------
// SPAWNING	COMMANDS

//NLMISC_COMMAND(spawnGrps,"spawn one of the versions of the group","<group id>")
//{
//	if(args.size() <1) return false;
//
//	for (uint i=0;i<args.size();++i)
//		CAISpawnCtrl::spawnGrp(args[i]);
//	
//	return true;
//}

//NLMISC_COMMAND(spawnMgrs,"spawn the population of one or more mgrs","[<mgr id>[...]]")
//{
//	if(args.size() <1) return false;
//
//	for (uint i=0;i<args.size();++i)
//		CAISpawnCtrl::spawnMgr(args[i]);
//	
//	return true;
//}

//NLMISC_COMMAND(spawnMaps,"spawn the population of one or more continents","[<continent name>[...]]")
//{
//	if(args.size() <1) return false;
//
//	for (uint i=0;i<args.size();++i)
//		CAISpawnCtrl::spawnMap(args[i]);
//	
//	return true;
//}

//NLMISC_COMMAND(spawnAll,"spawn the populations of all managers","")
//{
//	if(args.size() !=0) return false;
//	
//	CAISpawnCtrl::spawnAll();
//	
//	return true;
//}


//-------------------------------------------------------------------------
// DESPAWNING COMMANDS

//NLMISC_COMMAND(despawnGrps,"despawn populations of one or more groups","<group id> [<group id>[...]]")
//{
//	if(args.size() <1) return false;
//
//	for (uint i=0;i<args.size();++i)
//		CAISpawnCtrl::despawnGrp(args[i]);
//
//	return true;
//}
//
//NLMISC_COMMAND(despawnMgrs,"despawn the populations of one or more mgrs","[<mgr id>[...]]")
//{
//	if(args.size() <1) return false;
//
//	for (uint i=0;i<args.size();++i)
//		CAISpawnCtrl::despawnMgr(args[i]);
//	
//	return true;
//}
//
//NLMISC_COMMAND(despawnMaps,"despawn the population of one or more continents","[<continent id>[...]]")
//{
//	if(args.size() <1) return false;
//
//	for (uint i=0;i<args.size();++i)
//		CAISpawnCtrl::despawnMap(args[i]);
//	
//	return true;
//}
//
//NLMISC_COMMAND(despawnAll,"despawn the populations of all managers in service","")
//{
//	if(args.size() !=0) return false;
//
//	CAISpawnCtrl::despawnAll();
//
//	return true;
//}
//
// THIS LINE EXISTS TO MAKE SURE THE LINKER DOESN'T THROW OUT THIS MODULE AT LINK TIME!!!
bool LinkWithAiSpawnCommands=false;

} // end of namespace
