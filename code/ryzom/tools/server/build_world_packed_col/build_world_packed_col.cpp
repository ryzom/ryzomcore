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
#include "packed_world_builder.h"
#include "builder_config.h"
#include "zone_util.h"
//
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/command.h"
#include "nel/misc/debug.h"
#include "nel/misc/array_2d.h"
//
#include "nel/3d/packed_world.h"
#include "nel/3d/register_3d.h"
//
#include "game_share/scenario_entry_points.h"


using namespace NL3D;
using namespace NLMISC;
using namespace R2;

NLMISC::CLog	g_log;

class CPackedWorldHolder : public NLMISC::CRefCount
{
public:
	CPackedWorld PW;
};

typedef NLMISC::CSmartPtr<CPackedWorldHolder> CPackedWorldSmartPtr;

// get all the islands that are inside the given rect
static void getIslandsInside(sint32 xmin, sint32 xmax, sint32 ymin, sint32 ymax, std::vector<const CScenarioEntryPoints::CCompleteIsland *> &dest)
{
	dest.clear();	
	R2::CScenarioEntryPoints &sep = R2::CScenarioEntryPoints::getInstance();	
	uint numIslands = (uint)sep.getCompleteIslands().size();	
	for(uint l = 0; l < numIslands; ++l)
	{				
		const R2::CScenarioEntryPoints::CCompleteIsland &ci = sep.getCompleteIslands()[l];
		bool outside = (ci.XMin >= xmax || ci.XMax <= xmin ||
						 ci.YMin >= ymax || ci.YMax <= ymin);
		if (!outside)
		{
			dest.push_back(&ci);
		}
	}
}

int main(int argc, char* argv[])
{	
	//////////////////
	createDebug();
	registerSerial3d();	
	
	if (argc != 2)
	{
		printf("usage : %s config_file_name.cfg\n", CFile::getFilename(argv[0]).c_str());
		return -1;
	}	
	CConfigFile cf;
	CBuilderConfig builderConfig;
	//	
	try
	{
		cf.load(argv[1]);		
	}
	catch(const EStream &)
	{
		nlwarning("Error while reading config file\n");
		return -1;
	}
	builderConfig.build(cf);
	// build the cache / output directories if needed
	if (!CFile::isExists(builderConfig.CWMapCachePath))
	{
		CFile::createDirectory(builderConfig.CWMapCachePath);
	}
	//
	if (!CFile::isExists(builderConfig.CachePath))
	{
		CFile::createDirectory(builderConfig.CachePath);
	}
	//
	if (!CFile::isExists(builderConfig.OutputPath))
	{
		CFile::createDirectory(builderConfig.OutputPath);
	}	
	//
	for(uint k = 0; k < builderConfig.SearchPaths.size(); ++k)
	{
		CPath::addSearchPath(builderConfig.SearchPaths[k], true, false);
	}
	CPath::remapExtension("dds", "tga", true);
	CPath::remapExtension("dds", "png", true);
	//
	R2::CScenarioEntryPoints &sep = R2::CScenarioEntryPoints::getInstance();
	try
	{
		sep.loadCompleteIslands();
	}
	catch(const NLMISC::EStream &)
	{
		return -1;
	}
	if (sep.getCompleteIslands().empty())
	{
		nlwarning("No entry points found or read error in");
		return -1;
	}
	//	
	CPackedWorldBuilder builder;
	std::vector<CPackedWorldSmartPtr> islands;
	std::vector<CVector>              startPositions;
	uint numIslands = (uint)sep.getCompleteIslands().size();		
	//	
	for(uint k = 0; k < numIslands; ++k)
	{
		std::vector<std::string> zoneNames;
		const R2::CScenarioEntryPoints::CCompleteIsland &ep = sep.getCompleteIslands()[k];
		std::string zoneMin = posToZoneName((float) ep.XMin, (float) ep.YMin);
		std::string zoneMax = posToZoneName((float) ep.XMax, (float) ep.YMax);
		sint zoneMinX, zoneMinY;
		sint zoneMaxX, zoneMaxY;
		nlverify(getZonePos(zoneMin, zoneMinX, zoneMinY));
		nlverify(getZonePos(zoneMax, zoneMaxX, zoneMaxY));
		for (sint y = zoneMinY; y <= zoneMaxY; ++y)
		{
			for (sint x = zoneMinX; x <= zoneMaxX; ++x)
			{
				zoneNames.push_back(posToZoneName(x * 160.f + 80.f, y * 160.f + 80.f) + ".zonel");				
			}
		}
		// If the packed island already exists, load its header to know from which zones it was built
		// This way, if the zones found in an island change it will get rebuilt
		bool mustRebuild = true;
		std::string islandPath = builderConfig.OutputPath + "/" + ep.Island + ".packed_island";
		CPackedWorld tmpPW;		
		if (CFile::fileExists(islandPath))
		{			
			try
			{
				CIFile f(islandPath);
				tmpPW.serialZoneNames(f);
			}
			catch(const EStream &)
			{
				tmpPW.ZoneNames.clear();
			}
		}		
		// now, see which zones really should be found in that island
		std::vector<std::string> presentZoneNames;
		std::vector<std::string> presentZonePathes;
		for(uint l = 0; l < zoneNames.size(); ++l)
		{
			std::string zonePath = CPath::lookup(zoneNames[l], false, false);
			if (!zonePath.empty())
			{
				presentZonePathes.push_back(zonePath);
				presentZoneNames.push_back(toLower(zoneNames[l]));
			}
		}
		//
		std::sort(tmpPW.ZoneNames.begin(), tmpPW.ZoneNames.end());
		std::sort(presentZoneNames.begin(), presentZoneNames.end());
		//
		if (tmpPW.ZoneNames.size() == presentZoneNames.size() &&
			std::equal(tmpPW.ZoneNames.begin(), tmpPW.ZoneNames.end(), presentZoneNames.begin()))
		{
			// the same zones were found -> now check their dates against the island one
			uint32 packedIslandDate = CFile::getFileModificationDate(islandPath);
			mustRebuild = false;
			for(uint l = 0; l < presentZonePathes.size(); ++l)
			{				
				if (CFile::getFileModificationDate(presentZonePathes[l]) > packedIslandDate)
				{
					mustRebuild = true;
				}
				
			}
		}		
		//
		CPackedWorldSmartPtr packedIsland = new CPackedWorldHolder;
		if (!mustRebuild)
		{
			try
			{
				nlinfo("Loading island %s from cache (%d / %d)", ep.Island.c_str(), (int) k, (int) numIslands);
				CIFile f(islandPath);
				f.serial(packedIsland->PW);
			}
			catch(const EStream &)
			{
				mustRebuild = true; // damaged file or bad version ? -> force rebuild
				delete packedIsland; // remove whatever was serialized
				packedIsland = new CPackedWorldHolder;				
			}
		}

		if (mustRebuild)
		{
			builder.build(zoneNames, builderConfig.CachePath, false, packedIsland->PW, builderConfig.RefineThreshold);
			packedIsland->PW.ZoneNames = presentZoneNames;
			// save the result
			try
			{
				COFile f(islandPath);
				f.serial(packedIsland->PW);
			}
			catch(const EStream &)
			{
				nlwarning("Island %s not saved.", ep.Island.c_str());
			}
		}
		islands.push_back(packedIsland);
		CVector rayStart;
		if (!ep.EntryPoints.empty())
		{
			rayStart.set((float) ep.EntryPoints[0].X, (float) ep.EntryPoints[0].Y, 10000.f);
		}
		else
		{
			rayStart.set(0.5f * (ep.XMin + ep.XMax), 0.5f * (ep.YMin + ep.YMax), 10000.f);
		}
		//CVector rayEnd((float) ep.X, (float) ep.Y, -10000);
		//CVector inter;
		//if (packedIsland->PW.raytrace(rayStart, rayEnd, inter))
		//{
			//startPositions.push_back(inter);
		//}
		//else
		//{
		rayStart.z = 300.f;
		startPositions.push_back(rayStart);
		//}
		// TODO: add serialization here ...
		// TODO: add a check to see if no zones are more recent -> in this case, do nothing.
		// except if fly is wanted, in which case the packed version of the island should be reloaded.
	}		

	// For all cwmap, keep a file with its coordinates (we need to serialize them because else we must read the whole cwmap
	// to retrieve those				
	for(uint k = 0; k < builderConfig.CWMapList.size(); ++k)
	{
		// here, the rebuild require that we reload the whole map
		std::string cwMapPath = CPath::lookup(builderConfig.CWMapList[k], false, false);
		if (cwMapPath.empty())
		{
			nlwarning("Can't find %s, island height maps wont be updated");
			continue;
		}
		uint32 cwMapDate = CFile::getFileModificationDate(cwMapPath);

		std::string shortname = CFile::getFilenameWithoutExtension(builderConfig.CWMapList[k]);
		bool mustRebuild = false;
		sint32 xmin, xmax;
		sint32 ymin, ymax;
		try
		{
			CIFile f(builderConfig.CWMapCachePath + "/" + shortname + ".cw_height");
			f.serialCheck(NELID("OBSI"));
			f.serial(xmin);
			f.serial(xmax);
			f.serial(ymin);
			f.serial(ymax);
		}
		catch (const EStream &)
		{
			mustRebuild = true;
		}
		//									
		std::vector<const CScenarioEntryPoints::CCompleteIsland *> completeIslands;
		if (!mustRebuild)
		{
			getIslandsInside(xmin, xmax, ymin, ymax, completeIslands);
			for(uint l = 0; l < completeIslands.size(); ++l)
			{
				uint32 modifDate = CFile::getFileModificationDate(builderConfig.OutputPath + "/" + completeIslands[l]->Island + ".island_hm");
				if (modifDate < cwMapDate) 
				{
					mustRebuild = true;
					break;
				}
			}			
		}
		//		
		if (mustRebuild)
		{
			try
			{					
				std::vector<std::string> args(1);
				nlassert(ICommand::LocalCommands);
				// set the output path
				ICommand::TCommand::iterator commandIt;
				commandIt = ICommand::LocalCommands->find("setOutputPath");
				//nlassert(commandIt != ICommand::LocalCommands->end());
				args[0] = builderConfig.CWMapCachePath + "/";
				commandIt->second->execute("", args, g_log, true);
				// build the height map
				commandIt = ICommand::LocalCommands->find("pacsBuildHeightMap16");
				nlassert(commandIt != ICommand::LocalCommands->end());				
				args[0] = cwMapPath;
				commandIt->second->execute("", args, g_log, true);
				// now extract each island height
				// read back coordinates
				CIFile f(builderConfig.CWMapCachePath + "/" + shortname + ".cw_height");
				f.serialCheck(NELID("OBSI"));
				f.serial(xmin);
				f.serial(xmax);
				f.serial(ymin);
				f.serial(ymax);
				CArray2D<sint16> cwHeightMap;
				f.serial(cwHeightMap);
				//
				getIslandsInside(xmin, xmax, ymin, ymax, completeIslands);
				for(uint l = 0; l < completeIslands.size(); ++l)
				{		
					nlwarning("Building heightmap for entry point %s", completeIslands[l]->Island.c_str());
					const R2::CScenarioEntryPoints::CCompleteIsland &ep = *completeIslands[l];
					if (ep.XMax == ep.XMin || ep.YMax == ep.YMin)
					{
						nlwarning("Bad dimensions for island %s, not building heightmap", ep.Island.c_str());
					}
					else
					{
						CArray2D<sint16> island;
						island.init(ep.XMax - ep.XMin, ep.YMax - ep.YMin, 0x7fff);
						// get result from global heightmap						
						island.blit(cwHeightMap, ep.XMin - xmin, ep.YMin - ymin, island.getWidth(), island.getHeight(), 0, 0);
						// basic format for now ...
						try
						{
							COFile f(builderConfig.OutputPath + "/" + completeIslands[l]->Island + ".island_hm");
							f.serialCheck(NELID("MHSI"));
							f.serial(island);
							// export tga for check
							if (builderConfig.HeightMapsAsTga)
							{
								CBitmap tgaHM;
								tgaHM.resize(island.getWidth(), island.getHeight());
								sint16 hMax = -32768;
								sint16 hMin= 32767;
								uint numPix = island.getWidth() * island.getHeight();
								for(CArray2D<sint16>::iterator it = island.begin(); it != island.end(); ++it)	
								{
									if (*it != 32767)
									{
										hMax = std::max(hMax, *it);
										hMin = std::min(hMin, *it);
									}
								}								
								float scale = 255.f / favoid0((float) (hMax - hMin));
								float bias = (float) - hMin;
								CRGBA *dest = (CRGBA *) &tgaHM.getPixels()[0];
								for(CArray2D<sint16>::iterator it = island.begin(); it != island.end(); ++it)
								{
									if (*it == 0x7fff)
									{
										*dest++ = CRGBA::Magenta;
									}
									else
									{
										float height = scale * ((*it) + bias);
										clamp(height, 0.f, 255.f);
										*dest++ = CRGBA((uint8) height, (uint8) height, (uint8) height);
									}
								}	
								//
								COFile f(builderConfig.OutputPath + "/" + completeIslands[l]->Island + "_height_map.tga");
								tgaHM.writeTGA(f, 0, true);
							}
						}
						catch(const EStream &e)
						{
							e; // avoid compile warning
							nlwarning(e.what());
						}
					}
					
				}
			}
			catch (const Exception &e)
			{
				e; // avoid compile warning
				nlwarning(e.what());
			}
		}
	}		
	//
	if (builderConfig.Fly)
	{
		std::vector<CPackedWorldBuilder::CIslandInfo> islandInfos;	
		for(uint k = 0; k < islands.size(); ++k)
		{
			CPackedWorldBuilder::CIslandInfo islandInfo;
			const R2::CScenarioEntryPoints::CCompleteIsland &ci = sep.getCompleteIslands()[k];
			islandInfo.CornerMin.set((float) ci.XMin, (float) ci.YMin, 0.f);
			islandInfo.CornerMax.set((float) ci.XMax, (float) ci.YMax, 0.f);
			islandInfo.TexName = ci.Island + "_sp.tga";
			islandInfo.StartPosition = startPositions[k];
			islandInfo.PW = &(islands[k]->PW);
			// The map is cropped because of power of 2 texture size limitation
			// -> change UVScale accordingly
			uint texWidth = (uint) ceilf(builderConfig.PixelPerMeter * (ci.XMax - ci.XMin));
			uint texHeight = (uint) ceilf(builderConfig.PixelPerMeter * (ci.YMax - ci.YMin));
			islandInfo.UScale = (float) texWidth / 	raiseToNextPowerOf2(texWidth);
			islandInfo.VScale = (float) texHeight / raiseToNextPowerOf2(texHeight);
			//
			islandInfos.push_back(islandInfo);
		}
		builder.fly(islandInfos, builderConfig.CamSpeed);
	}
	
}



