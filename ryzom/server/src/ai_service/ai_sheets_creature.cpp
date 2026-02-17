// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// AI-service-specific sheets overrides.
// These depend on ai_script_comp.cpp (CFightScriptCompReader,
// CFightSelectFilter) and are kept separate from the base CCreature
// so the base class can be used independently (e.g. by sheets_packer_shard).

#include "stdpch.h"

#include "ai_sheets_creature.h"
#include "ai_script_comp.h"

#include "nel/net/service.h"
#include "nel/georges/load_form.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern char const* AISPackedSheetsFilename;
extern char const* AISPackedFightConfigSheetsFilename;
extern char const* AISPackedActionSheetsFilename;
extern char const* AISPackedRaceStatsSheetsFilename;

void AISHEETS::CCreatureAI::onScriptComp(const std::string &scriptCompStr)
{
	CFightScriptComp* scriptComp;
	try
	{
		scriptComp = CFightScriptCompReader::createScriptComp(scriptCompStr);
		registerScriptComp(scriptComp);
	}
	catch (const ReadFightActionException& ex)
	{
		nlwarning("script read error (ignored): %s", ex.what());
	}
}

void AISHEETS::CCreatureAI::registerScriptComp(CFightScriptComp* scriptComp)
{
	_ScriptCompList.push_back(scriptComp);

	CFightSelectFilter* filter = dynamic_cast<CFightSelectFilter*>(scriptComp);
	if (!filter)
		return;

	std::string const& param = filter->getParam();
	if (param == "ON_UPDATE")
		_UpdateScriptList.push_back(scriptComp);
	if (param == "ON_DEATH")
		_DeathScriptList.push_back(scriptComp);
	if (param == "ON_BIRTH")
		_BirthScriptList.push_back(scriptComp);
}

void AISHEETS::CSheetsAI::initInstance()
{
	setInstance(new CSheetsAI);
}

void AISHEETS::CSheetsAI::packSheets(const std::string &writeFilesDirectoryName)
{
	CConfigFile::CVar *varPtr = IService::isServiceInitialized() ? IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths")) : NULL;

	// Use CCreatureAI map so script comps are processed during loading
	std::map<CSheetId, CCreatureAIPtr> sheetsAI;

	// if config file variable 'GeorgePaths' exists then only do a minimal loadForms otherwise do the full works
	if (varPtr != NULL)
	{
		bool addSearchPath = false;

		loadForm2("aiaction", writeFilesDirectoryName + AISPackedActionSheetsFilename, _ActionSheets, false, false);
		if (_ActionSheets.empty())
		{
			if (!addSearchPath)
			{
				addSearchPath = true;
				for (uint32 i = 0; i < varPtr->size(); ++i)
					CPath::addSearchPath(NLMISC::expandEnvironmentVariables(varPtr->asString(i)), true, false);
			}
			loadForm2("aiaction", writeFilesDirectoryName + AISPackedActionSheetsFilename, _ActionSheets, true);
		}

		loadForm("actionlist", writeFilesDirectoryName + AISPackedFightConfigSheetsFilename, _ActionListSheets, false, false);
		if (_ActionListSheets.empty())
		{
			if (!addSearchPath)
			{
				addSearchPath = true;
				for (uint32 i = 0; i < varPtr->size(); ++i)
					CPath::addSearchPath(NLMISC::expandEnvironmentVariables(varPtr->asString(i)), true, false);
			}
			loadForm("actionlist", writeFilesDirectoryName + AISPackedFightConfigSheetsFilename, _ActionListSheets, true);
		}

		loadForm2("creature", writeFilesDirectoryName + AISPackedSheetsFilename, sheetsAI, false, false);
		if (sheetsAI.empty())
		{
			if (!addSearchPath)
			{
				addSearchPath = true;
				for (uint32 i = 0; i < varPtr->size(); ++i)
					CPath::addSearchPath(NLMISC::expandEnvironmentVariables(varPtr->asString(i)), true, false);
			}
			loadForm2("creature", writeFilesDirectoryName + AISPackedSheetsFilename, sheetsAI, true);
		}

		loadForm2("race_stats", writeFilesDirectoryName + AISPackedRaceStatsSheetsFilename, _RaceStatsSheets, false, false);
		if (_RaceStatsSheets.empty())
		{
			if (!addSearchPath)
			{
				addSearchPath = true;
				for (uint32 i = 0; i < varPtr->size(); ++i)
					CPath::addSearchPath(NLMISC::expandEnvironmentVariables(varPtr->asString(i)), true, false);
			}
			loadForm2("race_stats", writeFilesDirectoryName + AISPackedRaceStatsSheetsFilename, _RaceStatsSheets, true);
		}
	}
	else
	{
		loadForm2("aiaction", writeFilesDirectoryName + AISPackedActionSheetsFilename, _ActionSheets, true);
		loadForm("actionlist", writeFilesDirectoryName + AISPackedFightConfigSheetsFilename, _ActionListSheets, true);
		loadForm2("creature", writeFilesDirectoryName + AISPackedSheetsFilename, sheetsAI, true);
		loadForm2("race_stats", writeFilesDirectoryName + AISPackedRaceStatsSheetsFilename, _RaceStatsSheets, true);
	}

	// Transfer CCreatureAI pointers into the base _Sheets map
	for (std::map<CSheetId, CCreatureAIPtr>::iterator it = sheetsAI.begin(); it != sheetsAI.end(); ++it)
	{
		_Sheets.insert(std::make_pair(it->first, CCreaturePtr(it->second)));
	}
}
