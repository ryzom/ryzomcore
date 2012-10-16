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
// Misc.
#include "nel/misc/debug.h"
#include "nel/misc/displayer.h"
#include "nel/misc/path.h"
#include "nel/misc/log.h"
#include "nel/misc/sheet_id.h"

#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"
// Application
#include "sheets_packer_init.h"
#include "sheets_packer_cfg.h"
#include "sheet_manager.h"

#include "continent_manager_build.h"

///////////
// USING //
///////////
using namespace NLMISC;
//using namespace NL3D;
using namespace std;


/////////////
// GLOBALS //
/////////////
CFileDisplayer  *fd = NULL;

NLLIGO::CLigoConfig LigoConfig;

///////////////
// FUNCTIONS //
///////////////
//---------------------------------------------------
// init :
// Initialize the application.
// Return false if the init fails.
//---------------------------------------------------
bool init()
{
	// Add a displayer for Debug Infos.
	createDebug();

	fd = new CFileDisplayer(getLogDirectory() + "sheets_packer.log", true, "SHEETS_PACKER.LOG");

	// register ligo 'standard' class
	NLLIGO::Register();

	DebugLog->addDisplayer (fd);
	InfoLog->addDisplayer (fd);
	WarningLog->addDisplayer (fd);
	ErrorLog->addDisplayer (fd);
	AssertLog->addDisplayer (fd);

	// Load the application configuration.
	nlinfo("Loading config file...");
	AppCfg.init(ConfigFileName);

	// Define the root path that contains all data needed for the application.
	nlinfo("Adding search paths...");
	for(uint i = 0; i < AppCfg.DataPath.size(); i++)
		CPath::addSearchPath(AppCfg.DataPath[i], true, false);

	// Initialize Sheet IDs.
	nlinfo("Init SheetId...");
	CSheetId::init(true);

	// load packed sheets	
	nlinfo("Loading sheets...");
	IProgressCallback callback;
	SheetMngr.setOutputDataPath(AppCfg.OutputDataPath);
	SheetMngr.load (callback, true, true);

	// Make the lmconts.packed file
	if (!LigoConfig.readPrimitiveClass (AppCfg.LigoPrimitiveClass.c_str(), false))
		nlwarning ("Can't load primitive class file %s", AppCfg.LigoPrimitiveClass.c_str());
	NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &LigoConfig;
	buildLMConts(AppCfg.WorldSheet, AppCfg.PrimitivesPath, AppCfg.OutputDataPath);

	// The init is a success.
	return true;
}// init //

//---------------------------------------------------
// release :
// Release all the memory.
//---------------------------------------------------
void release()
{	
	DebugLog->removeDisplayer ("SHEETS_PACKER.LOG");
	InfoLog->removeDisplayer ("SHEETS_PACKER.LOG");
	WarningLog->removeDisplayer ("SHEETS_PACKER.LOG");
	ErrorLog->removeDisplayer ("SHEETS_PACKER.LOG");
	AssertLog->removeDisplayer ("SHEETS_PACKER.LOG");

	delete fd;
	fd = NULL;
}// release //

void outputSomeDebugInfoForPackedSheetCrash()
{
}
