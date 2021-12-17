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

#include "std_header.h"
//
#include "builder_config.h"
//
#include "nel/misc/config_file.h"

using namespace NLMISC;

//*************************************************************************
CBuilderConfig::CBuilderConfig()
{
	CachePath = "island_packed_zones_cache";
	CWMapCachePath = "island_cwmap_cache";
	OutputPath = "islands_col_meshes";
	CamSpeed = 100;
	Fly = false;
	HeightMapsAsTga = false;
	PixelPerMeter = 1.f;
	RefineThreshold = 8.f;
}

//*************************************************************************
void CBuilderConfig::build(NLMISC::CConfigFile &cf)
{
	CConfigFile::CVar *cachePathVar = cf.getVarPtr("CachePath");
	if (cachePathVar)
	{
		CachePath = cachePathVar->asString();
	}
	//
	CConfigFile::CVar *cwMapCachePathVar = cf.getVarPtr("CWMapCachePath");
	if (cwMapCachePathVar)
	{
		CWMapCachePath = cwMapCachePathVar->asString();
	}
	//
	CConfigFile::CVar *outputPathVar = cf.getVarPtr("OutputPath");
	if (outputPathVar)
	{
		OutputPath = outputPathVar->asString();
	}
	//
	CConfigFile::CVar *searchPathsVar = cf.getVarPtr("SearchPaths");
	if (searchPathsVar)
	{
		SearchPaths.resize(searchPathsVar->size());
		for(sint k = 0; k < (sint)searchPathsVar->size(); ++k)
		{
			SearchPaths[k] = searchPathsVar->asString(k);
		}
	}
	//
	CConfigFile::CVar *cwMapListVar = cf.getVarPtr("CWMapList");
	if (cwMapListVar)
	{
		CWMapList.resize(cwMapListVar->size());
		for(sint k = 0; k < (sint)cwMapListVar->size(); ++k)
		{
			CWMapList[k] = cwMapListVar->asString(k);
		}
	}
	//
	CConfigFile::CVar *camSpeedVar = cf.getVarPtr("CamSpeed");
	if (camSpeedVar)
	{
		CamSpeed = camSpeedVar->asFloat();
	}
	//
	CConfigFile::CVar *flyVar = cf.getVarPtr("Fly");
	if (flyVar)
	{
		Fly = flyVar->asBool();
	}	
	//
	CConfigFile::CVar *heightMapAsTgaVar = cf.getVarPtr("HeightMapsAsTga");
	if (heightMapAsTgaVar)
	{
		HeightMapsAsTga = heightMapAsTgaVar->asBool();
	}
	//
	CConfigFile::CVar *pixelPerMeterVar = cf.getVarPtr("PixelPerMeter");
	if (pixelPerMeterVar)
	{
		PixelPerMeter = pixelPerMeterVar->asFloat();
	}
	//
	CConfigFile::CVar *refineThresholdVar = cf.getVarPtr("RefineThreshold");
	if (refineThresholdVar)
	{
		RefineThreshold = refineThresholdVar->asFloat();
	}
}
