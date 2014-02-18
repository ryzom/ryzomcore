// NeL - MMORPG Framework <http://www.ryzomcore.org/>
// Copyright (C) 2014  by authors
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
// 
// Author: Jan Boon

#include <nel/misc/types_nl.h>

// STL includes
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

// NeL includes
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/log.h>
#include <nel/misc/path.h>
#include <nel/misc/sheet_id.h>
#include <nel/georges/load_form.h>
#include <game_share/data_set_base.h>
#include <input_output_service/string_manager.h>
#include <gpm_service/sheets.h>
#include <server_share/continent_container.h>
#include <entities_game_service/egs_sheets/egs_sheets.h>

// Project includes
// ...

namespace {

} /* anonymous namespace */

// EGS
NLMISC::CVariable<bool> EGSLight("egs","EGSLight", "Load EGS with a minimal set of feature loaded", false, 0, true);
NLMISC::CVariable<bool> LoadOutposts("egs", "LoadOutposts", "If false outposts won't be loaded", true, 0, true );
static std::string s_WriteDirectory;
std::string writeDirectory()
{
	return s_WriteDirectory;
}

////////////////////////////////////////////////////////////////////////
// note: *.packed_sheets files are placed in <build_packed_sheets>    //
//           and will need to be moved to the right location by       //
//           your build script system.                                //
////////////////////////////////////////////////////////////////////////

int main(int nNbArg, char **ppArgs)
{
	// create debug stuff
	NLMISC::createDebug();

	// verify all params
	if (nNbArg < 6)
	{
		// >sheets_packer_shard.exe L:\leveldesign L:\leveldesign\DFN R:\code\ryzom\server\data_shard\mirror_sheets T:\export\common\leveldesign\visual_slot_tab T:\test_shard
		nlinfo("ERROR : Wrong number of arguments\n");
		nlinfo("USAGE : sheets_packer_shard  <leveldesign> <dfn> <datasets> <tab> <build_packed_sheets>\n");
		nlinfo("<tab> : Directory containing visual_slots.tab");
		return EXIT_FAILURE;
	}
	std::string leveldesignDir = std::string(ppArgs[1]);
	if (!NLMISC::CFile::isDirectory(leveldesignDir))
	{
		nlerrornoex("Directory leveldesign '%s' does not exist", leveldesignDir.c_str());
		return EXIT_FAILURE;
	}
	std::string dfnDir = std::string(ppArgs[2]);
	if (!NLMISC::CFile::isDirectory(dfnDir))
	{
		nlerrornoex("Directory dfn '%s' does not exist", dfnDir.c_str());
		return EXIT_FAILURE;
	}
	std::string datasetsDir = std::string(ppArgs[3]);
	if (!NLMISC::CFile::isDirectory(datasetsDir))
	{
		nlerrornoex("Directory datasets '%s' does not exist", datasetsDir.c_str());
		return EXIT_FAILURE;
	}
	std::string tabDir = std::string(ppArgs[4]);
	if (!NLMISC::CFile::isDirectory(tabDir))
	{
		nlerrornoex("Directory tab '%s' does not exist", tabDir.c_str());
		return EXIT_FAILURE;
	}
	std::string exportDir = std::string(ppArgs[5]);
	if (!NLMISC::CFile::isDirectory(exportDir))
	{
		nlerrornoex("Directory build_packed_sheets '%s' does not exist", exportDir.c_str());
		return EXIT_FAILURE;
	}
	s_WriteDirectory = exportDir + "/";
	
	// add search paths
	NLMISC::CPath::addSearchPath(leveldesignDir, true, false);
	NLMISC::CPath::addSearchPath(dfnDir, true, false);
	NLMISC::CPath::addSearchPath(datasetsDir, false, false);
	NLMISC::CPath::addSearchPath(tabDir, false, false);
	
	// init sheet_id.bin
	NLMISC::CSheetId::init(false);

	// this here does the magic
	// MS
	{
		// Used by mirror_service.cpp
		// Used by range_mirror_manager.cpp
		// Used by mirror.cpp
		TSDataSetSheets sDataSetSheets;
		loadForm("dataset", exportDir + "/datasets.packed_sheets", sDataSetSheets);
	}

	// IOS
	{
		// Used by string_manager_parcer.cpp
		std::map<NLMISC::CSheetId, CStringManager::TSheetInfo> container;
		std::vector<std::string> exts;
		exts.push_back("creature");
		exts.push_back("race_stats");
		loadForm(exts, exportDir + "/ios_sheets.packed_sheets", container);
	}

	// GPMS
	{
		std::map<NLMISC::CSheetId, CGpmSheets::CSheet> container;
		std::vector<std::string> filters;
		filters.push_back("creature");
		filters.push_back("player");
		loadForm(filters, exportDir + "/gpms.packed_sheets", container);
	}

	// CContinentContainer
	{
		CContinentContainer continents;
		continents.buildSheets(s_WriteDirectory);
	}

	// EGS
	{
		CSheets::init();
	}
	
	// and that's all folks
	return EXIT_SUCCESS;
}

/* end of file */
