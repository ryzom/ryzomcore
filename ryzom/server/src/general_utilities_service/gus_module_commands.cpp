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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

//nel
#include "nel/misc/command.h"

// game share
#include "game_share/utils.h"

// local
#include "gus_module_manager.h"
#include "gus_module_factory.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace GUS;


//-----------------------------------------------------------------------------
// CModuleManager Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(GUS,modulesAdd,"add a module to the GUS service","<module_name>[<args>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()<1)
		return false;

	NLMISC::CSString rawArgs= rawCommandString;
	rawArgs.strtok(" \t");
	NLMISC::CSString moduleType= rawArgs.strtok(" \t");
	CModuleManager::getInstance()->addModule(moduleType,rawArgs);

	return true;
}

NLMISC_CATEGORISED_COMMAND(GUS,modulesRemove,"remove a module from the GUS service","<module_id>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()<1)
		return false;

	NLMISC::CSString rawArgs= rawCommandString;
	rawArgs.strtok(" \t");
	uint32 moduleId= atoi(rawArgs.c_str());
	if (NLMISC::toString("%u",moduleId)==rawArgs)
	{
		// try to find the module by numeric id
		TModulePtr module= CModuleManager::getInstance()->lookupModuleById(moduleId);
		DROP_IF(module==NULL,"Module not found: "+rawArgs,return true);
		nlinfo("Removing module %3d: %s %s",moduleId,module->getName().c_str(),module->getParameters().c_str());
		CModuleManager::getInstance()->removeModule(moduleId);
		return true;
	}

	// try to find the module by name or name & params
	CModuleManager::TModuleVector modules;
	CModuleManager::getInstance()->getModules(modules);
	for (uint32 i=modules.size();i--;)
	{
		CSString moduleDescription= CSString()+modules[i]->getName()+" "+modules[i]->getParameters();
		if (rawArgs==moduleDescription.left(rawArgs.size()))
		{
			uint32 moduleId= CModuleManager::getInstance()->getModuleId(modules[i]);
			nlinfo("Removing module %3d: %s %s",moduleId,modules[i]->getName().c_str(),modules[i]->getParameters().c_str());
			CModuleManager::getInstance()->removeModule(moduleId);
			return true;
		}
	}

	return false;
}

NLMISC_CATEGORISED_COMMAND(GUS,modulesDisplay,"display the states of the currently active modules","[<id>]")
{
	CNLSmartLogOverride logOverride(&log);

	switch (args.size())
	{
	case 0:
		// display a list of active modules
		CModuleManager::getInstance()->displayModules();
		return true;

	case 1:
		{
			// treat the case where we have a numeric module ID
			uint32 moduleId= atoi(args[0].c_str());
			if (NLMISC::toString(moduleId)==args[0])
			{
				// try to find the module by numeric id
				TModulePtr theModule= CModuleManager::getInstance()->lookupModuleById(moduleId);
				DROP_IF(theModule==NULL,"Module not found: "+args[0],return true);
				theModule->displayModule();
				return true;
			}
		}
		break;
	}

	// get a list of active modules from the module manager
	CModuleManager::TModuleVector modules;
	CModuleManager::getInstance()->getModules(modules);

	NLMISC::CSString rawArgs= rawCommandString;
	rawArgs.strtok(" \t");

	// iterate over all modules looking for matches against our args
	bool found=false;
	for (uint32 i=modules.size();i--;)
	{
		CSString moduleDescription= CSString()+modules[i]->getName()+" "+modules[i]->getParameters();
		if (rawArgs==moduleDescription.left(rawArgs.size()))
		{
			// display a matching module's details
			modules[i]->displayModule();
			found=true;
		}
	}

	WARN_IF(!found,"Module not found: "+rawArgs);
	return true;
}


//-----------------------------------------------------------------------------
// CModuleFactory Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(GUS,modulesListTypes,"display the list of valid module types","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CModuleFactory::getInstance()->display();

	return true;
}


//-----------------------------------------------------------------------------
