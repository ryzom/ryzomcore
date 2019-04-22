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




//////////////
// INCLUDES //
//////////////
#include "stdpch.h"

// Application
#include "sheets_packer_cfg.h"
#include "sheet_manager.h"
#include "continent_manager_build.h"


///////////
// USING //
///////////
using namespace NLMISC;


/////////////
// GLOBALS //
/////////////
NLLIGO::CLigoConfig LigoConfig;


///////////////
// FUNCTIONS //
///////////////


//---------------------------------------------------
// MAIN :
// Entry for the Apllication.
//---------------------------------------------------
int main(int argc, char **argv)
{
	CApplicationContext applicationContext;

	// Parse Command Line.
	NLMISC::CCmdArgs args;

	args.setDescription("Pack all sheets needed by client. All parameters must be defined in sheets_packer.cfg and there is no parameters from command-line.");

	if (!args.parse(argc, argv)) return 1;

	CFileDisplayer  *fd = NULL;

	/////////////////////////////////
	// Initialize the application. //
	try
	{
		// Add a displayer for Debug Infos, disable log.log.
		createDebug(NULL, false);

		CLog::setProcessName("sheets_packer");

		fd = new CFileDisplayer(getLogDirectory() + "sheets_packer.log", true, "SHEETS_PACKER.LOG");

		// register ligo 'standard' class
		NLLIGO::Register();

		DebugLog->addDisplayer(fd);
		InfoLog->addDisplayer(fd);
		WarningLog->addDisplayer(fd);
		ErrorLog->addDisplayer(fd);
		AssertLog->addDisplayer(fd);

		// Load the application configuration.
		nlinfo("Loading config file...");
		AppCfg.init(ConfigFileName);

		// Define the root path that contains all data needed for the application.
		nlinfo("Adding search paths...");
		for (uint i = 0; i < AppCfg.DataPath.size(); i++)
			CPath::addSearchPath(NLMISC::expandEnvironmentVariables(AppCfg.DataPath[i]), true, false);

		// Initialize Sheet IDs.
		nlinfo("Init SheetId...");
		CSheetId::init(true);

		// load packed sheets	
		nlinfo("Loading sheets...");
		IProgressCallback callback;
		SheetMngr.setOutputDataPath(NLMISC::expandEnvironmentVariables(AppCfg.OutputDataPath));
		SheetMngr.load(callback, true, true, AppCfg.DumpVisualSlotsIndex);

		// Make the lmconts.packed file
		if (!LigoConfig.readPrimitiveClass(AppCfg.LigoPrimitiveClass.c_str(), false))
			nlwarning("Can't load primitive class file %s", AppCfg.LigoPrimitiveClass.c_str());
		NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &LigoConfig;
		buildLMConts(AppCfg.WorldSheet, NLMISC::expandEnvironmentVariables(AppCfg.PrimitivesPath), NLMISC::expandEnvironmentVariables(AppCfg.OutputDataPath));
	}
	catch(const EFatalError &) { return EXIT_FAILURE; /* nothing to do */ }
	catch(const Exception &e)
	{
		try
		{
			nlerror ("Initialization of the application failed : %s", e.what());
		}
		catch(const EFatalError &)
		{
			// nothing to do
		}

		// Failure -> Exit.
		return EXIT_FAILURE;
	}

	/////////////////////////////
	// Release all the memory. //
	try
	{
		DebugLog->removeDisplayer("SHEETS_PACKER.LOG");
		InfoLog->removeDisplayer("SHEETS_PACKER.LOG");
		WarningLog->removeDisplayer("SHEETS_PACKER.LOG");
		ErrorLog->removeDisplayer("SHEETS_PACKER.LOG");
		AssertLog->removeDisplayer("SHEETS_PACKER.LOG");

		if (fd) delete fd;
		fd = NULL;
	}
	catch(const EFatalError &) { return EXIT_FAILURE; /* nothing to do */ }
	catch(const Exception &e)
	{
		try
		{
			nlerror ("Release of the application failed: %s", e.what());
		}
		catch(const EFatalError &)
		{
			// nothing to do
		}

		// Failure -> Exit.
		return EXIT_FAILURE;
	}


	// EXIT of the Application.
	return EXIT_SUCCESS;
}// main //
