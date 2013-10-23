// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/misc/displayer.h"
#include "nel/misc/file.h"

#include "nel/3d/register_3d.h"

#include "build_surf.h"
#include "build_rbank.h"
#include "prim_checker.h"

#include "nel/pacs/global_retriever.h"
#include "nel/pacs/retriever_bank.h"
#include "nel/pacs/surface_quad.h"
#include "nel/pacs/local_retriever.h"
#include "nel/pacs/retriever_instance.h"

#include <string>
#include <deque>

#include <stdlib.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

#ifndef NL_BRB_CFG
#define NL_BRB_CFG "."
#endif // NL_BRB_CFG

#define LOG_ALL_INFO_TO_FILE



string												OutputRootPath;
string												OutputDirectory;
string												OutputPath;
string												TessellationPath;
string												IGBoxes;
uint												TessellateLevel;
bool												ReduceSurfaces;
bool												SmoothBorders;
bool												ComputeElevation;
bool												ComputeLevels;
vector<string>										ZoneNames;
string												ZoneExt;
string												ZoneNHExt = ".zonenhw";
string												ZoneLookUpPath;
bool												ProcessAllPasses;
bool												CheckPrims;
bool												TessellateZones;
bool												MoulineZones;
bool												TessellateAndMoulineZones;
bool												ProcessRetrievers;
string												PreprocessDirectory;
float												WaterThreshold = 0.0f;
bool												UseZoneSquare;
string												ZoneUL;
string												ZoneDR;
string												GlobalRetriever;
string												RetrieverBank;
string												GlobalUL;
string												GlobalDR;
bool												ProcessGlobal;
string												LevelDesignWorldPath;
string												IgLandPath;
string												IgVillagePath;
bool												Verbose = false;
bool												CheckConsistency = true;

CPrimChecker										PrimChecker;

/****************************************************************\
					initMoulinette
\****************************************************************/
int		getInt(CConfigFile &cf, const string &varName, int defaultValue=0)
{
	CConfigFile::CVar *var = cf.getVarPtr(varName);
	if (Verbose)
	{
		if (var)
			nlinfo("Read %s = %d", varName.c_str(), var->asInt());
		else
			nlinfo("Couldn't read %s, using default = %d", varName.c_str(), defaultValue);
	}
	return var ? var->asInt() : defaultValue;
}

float		getFloat(CConfigFile &cf, const string &varName, float defaultValue=0.0)
{
	CConfigFile::CVar *var = cf.getVarPtr(varName);
	if (Verbose)
	{
		if (var)
			nlinfo("Read %s = %f", varName.c_str(), var->asFloat());
		else
			nlinfo("Couldn't read %s, using default = %f", varName.c_str(), defaultValue);
	}
	return var ? var->asFloat() : defaultValue;
}

string	getString(CConfigFile &cf, const string &varName, const string &defaultValue="")
{
	CConfigFile::CVar *var = cf.getVarPtr(varName);
	if (Verbose)
	{
		if (var)
			nlinfo("Read %s = '%s'", varName.c_str(), var->asString().c_str());
		else
			nlinfo("Couldn't read %s, using default = '%s'", varName.c_str(), defaultValue.c_str());
	}
	return var ? var->asString() : defaultValue;
}

bool	getBool(CConfigFile &cf, const string &varName, bool defaultValue=false)
{
	CConfigFile::CVar *var = cf.getVarPtr(varName);
	if (Verbose)
	{
		if (var)
			nlinfo("Read %s = %s", varName.c_str(), (var->asInt()!=0 ? "true" : "false"));
		else
			nlinfo("Couldn't read %s, using default = '%s'", varName.c_str(), (defaultValue ? "true" : "false"));
	}
	return var ? (var->asInt() != 0) : defaultValue;
}



void	initMoulinette()
{
	registerSerial3d();

	try
	{
#ifdef NL_OS_UNIX
	        std::string homeDir = getenv("HOME");
        	NLMISC::CPath::addSearchPath( homeDir + "/.nel");
#endif // NL_OS_UNIX

	        NLMISC::CPath::addSearchPath(NL_BRB_CFG);

		CConfigFile cf;
		uint			i;

		cf.load("build_rbank.cfg");

		CConfigFile::CVar *verboseVar = cf.getVarPtr("Verbose");
		if (verboseVar && verboseVar->asInt() != 0)
			Verbose = true;


		// Read paths
		CConfigFile::CVar *cvPathes = cf.getVarPtr("Pathes");
		for (i=0; cvPathes != NULL && i<cvPathes->size(); ++i)
			CPath::addSearchPath(cvPathes->asString(i));

		ProcessAllPasses = getBool(cf, "ProcessAllPasses", false);
		CheckPrims = getBool(cf, "CheckPrims", false);
		TessellateZones = getBool(cf, "TessellateZones", false);
		MoulineZones = getBool(cf, "MoulineZones", false);
		TessellateAndMoulineZones = getBool(cf, "TessellateAndMoulineZones", false);
		ProcessRetrievers = getBool(cf, "ProcessRetrievers", false);
		ProcessGlobal = getBool(cf, "ProcessGlobal", false);

		OutputRootPath = getString(cf, "OutputRootPath");
		UseZoneSquare = getBool(cf, "UseZoneSquare", false);

		WaterThreshold = getFloat(cf, "WaterThreshold", 1.0);

		CheckConsistency = getBool(cf, "CheckConsistency", true);

		//if (TessellateZones || MoulineZones)
		{
			ZoneExt = getString(cf, "ZoneExt", ".zonew");
			ZoneNHExt = getString(cf, "ZoneNHExt", ".zonenhw");
			ZoneLookUpPath = getString(cf, "ZonePath", "./");
			CPath::addSearchPath(ZoneLookUpPath);

			TessellationPath = getString(cf, "TessellationPath");
			TessellateLevel = getInt(cf, "TessellateLevel");
		}

		//if (MoulineZones)
		{
			LevelDesignWorldPath = getString(cf, "LevelDesignWorldPath");
			IgLandPath = getString(cf, "IgLandPath");
			IgVillagePath = getString(cf, "IgVillagePath");
			IGBoxes = getString(cf, "IGBoxes", "./temp.bbox");
			ReduceSurfaces = getBool(cf, "ReduceSurfaces", true);
			ComputeElevation = getBool(cf, "ComputeElevation", false);
			ComputeLevels = getBool(cf, "ComputeLevels", true);
		}

		//if (MoulineZones || ProcessRetrievers || ProcessGlobal)
		{
			SmoothBorders = getBool(cf, "SmoothBorders", true);
			PreprocessDirectory = getString(cf, "PreprocessDirectory");

			if (SmoothBorders)
				OutputDirectory = getString(cf, "SmoothDirectory");
			else
				OutputDirectory = getString(cf, "RawDirectory");

			OutputPath = OutputRootPath+OutputDirectory;
		}

		//if (ProcessGlobal)
		{
			GlobalRetriever = getString(cf, "GlobalRetriever");
			RetrieverBank = getString(cf, "RetrieverBank");
			GlobalUL = getString(cf, "GlobalUL");
			GlobalDR = getString(cf, "GlobalDR");
		}


		ZoneNames.clear();
		if (UseZoneSquare)
		{
			ZoneUL = getString(cf, "ZoneUL");
			ZoneDR = getString(cf, "ZoneDR");

			uint	ul = getZoneIdByName(ZoneUL),
					dr = getZoneIdByName(ZoneDR);
			uint	x0 = ul%256, 
					y0 = ul/256,
					x1 = dr%256, 
					y1 = dr/256;
			uint	x, y;
			for (y=y0; y<=y1; ++y)
				for (x=x0; x<=x1; ++x)
					ZoneNames.push_back(getZoneNameById(x+y*256));
		}
		else
		{
			CConfigFile::CVar *cvZones = cf.getVarPtr("Zones");
			for (i=0; cvZones != NULL && i<cvZones->size(); i++)
				ZoneNames.push_back(cvZones->asString(i));
		}
	}
	catch (const EConfigFile &e)
	{
		nlwarning("Problem in config file : %s\n", e.what ());
	}
}

/****************************************************************\
					moulineZones
\****************************************************************/
void	moulineZones(vector<string> &zoneNames)
{
	uint	i;

	if (CheckPrims)
	{
		PrimChecker.build(LevelDesignWorldPath, IgLandPath, IgVillagePath);
	}

	PrimChecker.load();

	if (ProcessAllPasses)
	{

		for (i=0; i<zoneNames.size(); ++i)
		{
			nlinfo("Generate final .lr for zone %s", zoneNames[i].c_str());
			processAllPasses(zoneNames[i]);
		}

	}

	if (ProcessGlobal)
	{
		nlinfo("Process .gr and .rbank");
		processGlobalRetriever();
	}
}

/****************************************************************\
							MAIN
\****************************************************************/
int main(int argc, char **argv)
{
	// Filter addSearchPath
	NLMISC::createDebug();
	InfoLog->addNegativeFilter ("adding the path");

	CFileDisplayer fd(getLogDirectory() + "evallog.log", true);

#ifdef LOG_ALL_INFO_TO_FILE
	createDebug();
	DebugLog->addDisplayer (&fd);
	ErrorLog->addDisplayer (&fd);
	WarningLog->addDisplayer (&fd);
	InfoLog->addDisplayer (&fd);
	AssertLog->addDisplayer (&fd);

	ErrorLog->removeDisplayer("DEFAULT_MBD");
#endif

	try
	{
		// Init the moulinette
		initMoulinette();

		// Compute the zone surfaces
		TTime	before, after;

		uint	i;
		if (argc > 1)
		{
			ZoneNames.clear();
			for (i=1; i<(uint)argc; ++i)
			{
				if (argv[i][0] != '-')
				{
					ZoneNames.push_back(string(argv[i]));
				}
				else
				{
					switch (argv[i][1])
					{
					case 'C':
						CheckPrims = true;
						break;
					case 'c':
						CheckPrims = false;
						break;
					case 'P':
						ProcessAllPasses = true;
						break;
					case 'p':
						ProcessAllPasses = false;
						break;
					case 'G':
						ProcessGlobal = true;
						break;
					case 'g':
						ProcessGlobal = false;
						break;
					case 'T':
					case 't':
					case 'M':
					case 'm':
					case 'L':
					case 'l':
						nlwarning("Option %c is not more valid", argv[i][1]);
						break;
					}
				}
			}
		}

		before = CTime::getLocalTime();
		moulineZones(ZoneNames);
		after = CTime::getLocalTime();

		uint	totalSeconds = (uint)((after-before)/1000);
		uint	workDay = totalSeconds/86400,
				workHour = (totalSeconds-86400*workDay)/3600,
				workMinute = (totalSeconds-86400*workDay-3600*workHour)/60,
				workSecond = (totalSeconds-86400*workDay-3600*workHour-60*workMinute);
		if (Verbose)
			nlinfo("total computation time: %d days, %d hours, %d minutes and %d seconds", workDay, workHour, workMinute, workSecond);
	}
	catch (const Exception &e)
	{
		nlwarning ("main trapped an exception: '%s'\n", e.what ());
	}
#ifndef NL_DEBUG
/*	catch (...)
	{
		nlwarning("main trapped an unknown exception\n");
	}*/
#endif // NL_DEBUG

	return 0;
}
