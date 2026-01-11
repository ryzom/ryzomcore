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

/*#include "std_header.h"
//
// test_col_world.cpp : Defines the entry point for the console application.
//
#include "zone_util.h"
#include "village.h"
//
#include "nel/misc/array_2d.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/triangle.h"
#include "nel/misc/polygon.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/quad.h"
//
#include "nel/3d/frustum.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/stream.h"
//
#include "3d/landscape.h"
#include "3d/zone.h"
#include "3d/quad_grid.h"
#include "3d/event_mouse_listener.h"
#include "3d/vertex_buffer.h"
#include "3d/material.h"
#include "3d/register_3d.h"
#include "3d/nelu.h"
#include "3d/scene_group.h"
#include "3d/shape_info.h"
#include "3d/packed_zone.h"
#include "3d/packed_world.h"
//
#include <string>
//

#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"


using namespace NLMISC;
using namespace NL3D;



static inline void pushVBQuad(NLMISC::CVector *&dest, const NLMISC::CQuad &quad)
{
	*dest++ = quad.V0;
	*dest++ = quad.V1;
	*dest++ = quad.V2;
	*dest++ = quad.V3;	
}


class CZoneRefCount : public CRefCount
{
public:
	CZone		Zone;
	bool		Loaded;
	std::string Path;
	std::string IGPath;
	sint		X;
	sint		Y;
public:
	CZoneRefCount() : Loaded(false) {}
};


static void viewportToScissor(const CViewport &vp, CScissor &scissor)
{
	scissor.X = vp.getX();
	scissor.Y = vp.getY();
	scissor.Width = vp.getWidth();
	scissor.Height = vp.getHeight();
}




// load list of continent from the .world sheet
static bool loadContinentList(const std::string &worldSheet, std::vector<std::string> &continentList)
{
	NLGEORGES::UFormLoader *loader = NLGEORGES::UFormLoader::createLoader();
	//
	std::string path  = CPath::lookup(worldSheet, false, false);
	if (path.empty())
	{
		NLGEORGES::UFormLoader::releaseLoader(loader);
		nlwarning("Path not found for %s.", worldSheet.c_str());
		return false;
	}
	NLGEORGES::UForm *worldForm;
	worldForm = loader->loadForm(path.c_str());
	if (worldForm == NULL)
	{
		NLGEORGES::UFormLoader::releaseLoader(loader);
		return false;
	}
	
	uint size;
	NLGEORGES::UFormElm *pElt;
	nlverify (worldForm->getRootNode().getNodeByName (&pElt, "continents list"));
	if(!pElt)
	{
		nlwarning("node 'continents list' not found in a .world");
		NLGEORGES::UFormLoader::releaseLoader(loader);
		return false;
	}
	else
	{		
		nlverify (pElt->getArraySize (size));
		for (uint32 i = 0; i <size; ++i)
		{
			NLGEORGES::UFormElm *pEltOfList;

			// Get the continent
			if (pElt->getArrayNode (&pEltOfList, i) && pEltOfList)
			{
				std::string continentName;
				pEltOfList->getValueByName (continentName, "continent_name");
				if (CFile::getExtension(continentName).empty())
				{
					continentName += ".continent";
				}
				continentList.push_back(continentName);				
			}
		}		
	}
	NLGEORGES::UFormLoader::releaseLoader(loader);
	return true;
}


class CRemoveBorderLeavePred
{
public:
	CRemoveBorderLeavePred(uint16 zoneId) : ZoneId(zoneId) {}
	bool operator()(const CTessFace *tf) const
	{
		return tf->Patch->getZone()->getZoneId() != ZoneId;
	}
	uint16 ZoneId;
};


static bool parseCamPos(CConfigFile &cf, CVector &camPos)
{
	CConfigFile::CVar *camPosVar = cf.getVarPtr("CamPos");
	if (camPosVar)
	{
		if (camPosVar->size() == 3)
		{
			camPos.x = camPosVar->asFloat(0);
			camPos.y = camPosVar->asFloat(1);
			camPos.z = camPosVar->asFloat(2);
			return true;
		}
		else
		{
			// it is a zone name
			sint zoneX, zoneY;
			if (getZonePos(camPosVar->asString(), zoneX, zoneY))
			{
				camPos.x = 160.f * zoneX + 80.f;
				camPos.y = 160.f * zoneY + 80.f;
				camPos.z = 0.f;
				return true;
			}				
		}
	}
	return false;
}

static bool parseCamSpeed(CConfigFile &cf, float &camSpeed)
{
	CConfigFile::CVar *camSpeedVar = cf.getVarPtr("CamSpeed");
	if (camSpeedVar)
	{
		camSpeed = camSpeedVar->asFloat();
		return true;
	}
	return false;
}

static bool newCamSpeedWanted = false;
static float newCamSpeed = FLT_MIN;
static bool newCamPosWanted = false;

static CVector newCamPos(FLT_MAX, FLT_MAX, FLT_MAX);

static void configFileChanged(const std::string &filename)
{
	CConfigFile cf;
	try
	{
		cf.load(filename);
		CVector camPos;
		if (parseCamPos(cf, camPos))
		{
			if (camPos!= newCamPos)
			{
				newCamPos = camPos;
				newCamPosWanted = true;
			}
		}
		float camSpeed;
		if (parseCamSpeed(cf, camSpeed))
		{
			if (camSpeed != newCamSpeed)
			{
				newCamSpeed= camSpeed;
				newCamSpeedWanted = true;
			}
		}
	}
	catch(const EStream &)
	{
		printf("Error while reading config file\n");		
	}
}

int main(int argc, char* argv[])
{		
	registerSerial3d();	
	
	if (argc != 2)
	{
		printf("usage : %s config_file_name.cfg\n", CFile::getFilename(argv[0]).c_str());
		return -1;
	}	
	//
	std::string				 worldSheetName;
	//
	std::string				 cachePath;
	// load all landscape zones
	std::vector<std::string> zoneNames;	
	CConfigFile cf;
	CVector camPos(18927.f, -24382.f, 0.f);

	bool addLandscapeIG = false;

	float camSpeed = 35.f;
	try
	{
		cf.load(argv[1]);				
		parseCamPos(cf, camPos);
		newCamPos = camPos;
		parseCamSpeed(cf, camSpeed);
		newCamSpeed = camSpeed;
		CConfigFile::CVar &paths = cf.getVar("ZonesPaths");
		for(uint k = 0; k < (uint) paths.size(); ++k)
		{
			std::vector<std::string> files;
			CPath::getPathContent(paths.asString(k), true, false, true, files);
			for(uint l = 0; l < files.size(); ++l)
			{
				std::string ext = CFile::getExtension(files[l]);
				if (nlstricmp(ext, "zonel") == 0)
				{
					zoneNames.push_back(files[l]);
				}
			}
		}
		cachePath = cf.getVar("CachePath").asString();
		CConfigFile::CVar *searchPaths = cf.getVarPtr("SearchPaths");
		for(uint k = 0; k < (uint) searchPaths->size(); ++k)
		{
			CPath::addSearchPath(searchPaths->asString(k), true, false);
		}
		CConfigFile::CVar *addLandscapeIGVarPtr = cf.getVarPtr("AddLandscapeIG");
		if (addLandscapeIGVarPtr)
		{
			addLandscapeIG = addLandscapeIGVarPtr->asInt() != 0;
		}
	}
	catch(const EStream &)
	{
		printf("Error while reading config file\n");
		return -1;
	}
	catch(const EConfigFile &e)
	{
		printf(e.what());
		return -1;
	}
	//
	CFile::addFileChangeCallback(argv[1], configFileChanged);
	//
	std::vector<CSmartPtr<CZoneRefCount> >   zones;
	zones.reserve(zoneNames.size());	
	sint zoneMinX, zoneMaxX;
	sint zoneMinY, zoneMaxY;
	bool firstZoneCorner = true;
	for(uint k = 0; k < zoneNames.size(); ++k)
	{
		CSmartPtr<CZoneRefCount> zoneRef = new CZoneRefCount;
		zoneRef->Path = zoneNames[k];
		if (addLandscapeIG)
		{
			std::string igFileName = CFile::getFilenameWithoutExtension(zoneRef->Path) + ".ig";
			zoneRef->IGPath = CPath::lookup(igFileName, false, false);
			if (zoneRef->IGPath.empty())
			{
				nlwarning("Couldn't find ig %s. Maybe there's no ig for that zone.", igFileName.c_str());
			}
		}
		//printf("Loading zone %d / %d \n", (int) k + 1, (int) zoneNames.size());
		CVector2f zoneCornerMin, zoneCornerMax;		
		if (getZonePos(zoneNames[k], zoneRef->X, zoneRef->Y))
		{
			if (firstZoneCorner)
			{
				zoneMinX = zoneMaxX = zoneRef->X;
				zoneMinY = zoneMaxY = zoneRef->Y;
				firstZoneCorner = false;
			}
			else
			{
				zoneMinX = std::min(zoneMinX, zoneRef->X);
				zoneMaxX = std::max(zoneMaxX, zoneRef->X);
				zoneMinY = std::min(zoneMinY, zoneRef->Y);
				zoneMaxY = std::max(zoneMaxY, zoneRef->Y);
			}			
			zones.push_back(zoneRef);
		}				
	}
	if (zones.empty())
	{
		printf("No zones loaded \n");
		return -1;
	}
	uint gridWidth  = zoneMaxX - zoneMinX + 1;
	uint gridHeight = zoneMaxY - zoneMinY + 1;
	// build a grid of zones
	CArray2D<CZoneRefCount *> zoneGrid;
	CArray2D<TPackedZoneBaseSPtr> packedZoneGrid;
	zoneGrid.init(gridWidth, gridHeight, NULL);
	packedZoneGrid.init(gridWidth, gridHeight);
	for(uint k = 0; k < zones.size(); ++k)
	{
		sint x = zones[k]->X - zoneMinX;
		sint y = zones[k]->Y - zoneMinY;		
		zoneGrid(x, y) = zones[k];
	}	
	// build grid for tris
	CVector2f cornerMin, cornerMax;		
	cornerMin.set(zoneMinX * 160.f, zoneMinY * 160.f);
	cornerMax.set((zoneMaxX  + 1) * 160.f, (zoneMaxY  + 1) * 160.f);
	const float CELL_SIZE = 4.f; // resolution of tri grid	

	CVillageGrid vg;
	vg.init(gridWidth, gridHeight, zoneMinX, zoneMinY);


	std::vector<std::string> continentList;
	CConfigFile::CVar *worldSheetPtr = cf.getVarPtr("WorldSheet");
	if (!(worldSheetPtr && loadContinentList(worldSheetPtr->asString(), continentList)))
	{
		CConfigFile::CVar *continentSheetNames = cf.getVarPtr("ContinentSheetNames");
		std::vector<CSmartPtr<CInstanceGroup> > igs;
		if (!continentSheetNames)
		{
			nlwarning("No villages added to collisions");
		}
		else
		{		
			for(uint k = 0; k < (uint) continentSheetNames->size(); ++k)
			{
				continentList.push_back(continentSheetNames->asString(k));
			}
		}	
	}	

	bool addVillages = true;
	CConfigFile::CVar *addVillagesVar = cf.getVarPtr("AddVillages");
	if (addVillagesVar)
	{
		addVillages = addVillagesVar->asBool();
	}
	if (addVillages)
	{
		for(uint k = 0; k < (uint) continentList.size(); ++k)
		{		
			vg.addVillagesFromContinent(continentList[k]);
		}
	}	

	TShapeCache shapeCache;	

	

	// build each zone separatly
	static std::vector<const CTessFace*> leaves;
	uint currZoneIndex = 0;
	for (sint y = 0; y < (sint) zoneGrid.getHeight(); ++y)
	{
		for (sint x = 0; x < (sint) zoneGrid.getWidth(); ++x)
		{
			if (zoneGrid(x, y))
			{					
				std::string cacheFilename = CPath::standardizePath(cachePath) + CFile::getFilenameWithoutExtension(zoneGrid(x, y)->Path) + ".packed_zone";				
				bool mustRebuild = false;

				// if there's any village on this zone that is more recent than the zone, then must rebuild the zone
				{
					const std::list<uint> &villageList = vg.VillageGrid(x, y);
					for(std::list<uint>::const_iterator it = villageList.begin(); it != villageList.end(); ++it)
					{
						uint32 modifDate = vg.Villages[*it].FileModificationDate;
						uint32 cacheDate = CFile::getFileModificationDate(cacheFilename);
						if (modifDate > cacheDate)
						{
							mustRebuild = true;
							break;
						}
					}
				}
								
				// if landscape ig is reclaimed, see if it is more recent
				if (!zoneGrid(x, y)->IGPath.empty())
				{
					if (CFile::getFileModificationDate(zoneGrid(x, y)->IGPath) >= CFile::getFileModificationDate(cacheFilename))
					{
						mustRebuild = true;	
					}
				}				
				
				if (!mustRebuild)
				{
					mustRebuild = true;
					// see if zone is present in cache and is valid
					if (CFile::getFileModificationDate(cacheFilename) >= CFile::getFileModificationDate(zoneGrid(x, y)->Path))
					{
						// try to retrieve file from cache
						try
						{
							CIFile f;
							f.open(cacheFilename);
							CPackedZoneBase *pb;
							f.serialPolyPtr(pb);	
							packedZoneGrid(x, y) = pb;
							mustRebuild = false;
							printf("Retrieving zone %d / %d from cache\n", (int) currZoneIndex + 1, (int) zones.size());
						}
						catch(const EStream &)
						{
						}
					}
				}

				if (mustRebuild)
				{
					printf("Rebuilding zone %d / %d \n", (int) currZoneIndex + 1, (int) zones.size());
					CLandscape *landscape = new CLandscape;
					landscape->init();
					landscape->setThreshold (8.f / (1000.f * 1000.f));
					//landscape->setThreshold(0);
					landscape->setTileMaxSubdivision (0);
					// add wanted zone & all zones around for continuity
					for (sint ly = y - 1; ly <= y + 1; ++ly)
					{
						if (ly < 0)           continue;
						if (ly >= (sint) gridHeight) break;
						for (sint lx = x - 1; lx <= x + 1; ++lx)
						{
							if (lx < 0)           continue;
							if (lx >= (sint) gridWidth)  break;						
							if (zoneGrid(lx, ly))
							{
								if (!zoneGrid(lx, ly)->Loaded)
								{
									CIFile stream;
									try
									{										
										stream.open(zoneGrid(lx, ly)->Path);
										zoneGrid(lx, ly)->Zone.serial(stream);										
										zoneGrid(lx, ly)->Loaded = true;
									}
									catch(const EStream &)
									{
										printf("Error while loading zone %s : zone not loaded \n");
									}									
								}
								if (zoneGrid(lx, ly)->Loaded)
								{
									landscape->addZone(zoneGrid(lx, ly)->Zone);
								}
							}
						}
					}				
					//
					landscape->refineAll(CVector((x + zoneMinX) * 160.f + 80.f, (y + zoneMinY) * 160.f + 80.f, 1000.f));
					//
					// Dump tesselated triangles	
					leaves.clear();
					landscape->getTessellationLeaves(leaves);
					leaves.erase(std::remove_if(leaves.begin(), leaves.end(), CRemoveBorderLeavePred(zoneGrid(x, y)->Zone.getZoneId())), leaves.end());
					// gather and load list of villages ig
					std::vector<CInstanceGroup *> igs;
					if (addVillages)
					{
						const std::list<uint> &villageList = vg.VillageGrid(x, y);
						for(std::list<uint>::const_iterator it = villageList.begin(); it != villageList.end(); ++it)
						{
							CVillage &village = vg.Villages[*it];
							village.load(shapeCache); // load if needed
							for(uint k = 0; k < village.IG.size(); ++k)
							{
								if (village.IG[k].IG)
								{
									igs.push_back(village.IG[k].IG);
								}
							}
						}
					}					
					//
					CAABBox baseZoneBBox;
					CVector zoneMinCorner((x + zoneMinX) * 160.f, (y + zoneMinY) * 160.f, 0.f);
					baseZoneBBox.setMinMax(zoneMinCorner, zoneMinCorner + CVector(160.f, 160.f, 1.f));
					CSmartPtr<CPackedZone32> pz32 = new CPackedZone32;										
					//
					pz32->build(leaves, CELL_SIZE, igs, shapeCache, baseZoneBBox, (sint32) (x + zoneMinX), (sint32) (y + zoneMinY));					
					//					
					// try to convert to 16 bit indices to save some place
					CSmartPtr<CPackedZone16> pz16 = pz32->buildPackedZone16();					
					if (pz16)
					{
						packedZoneGrid(x, y) = pz16;
					}
					else					
					{
						packedZoneGrid(x, y) = pz32;
					}										
					// write result in cache
					try
					{
						if (!CFile::isExists(cachePath))
						{
							CFile::createDirectoryTree(cachePath);
						}
						COFile f;
						f.open(cacheFilename);
						CPackedZoneBase *pb = packedZoneGrid(x, y);
						f.serialPolyPtr(pb);						
					}
					catch(const EStream &e)
					{
						printf("Error while writing packed zone to cache : \n %s \n", e.what());
					}
					//
					delete landscape;
				}
				++ currZoneIndex;
			}
		}
	}
	zoneGrid.clear();
	shapeCache.clear();

	
	std::vector<TPackedZoneBaseSPtr> packedZonesArray;
	for (sint y = 0; y < (sint) packedZoneGrid.getHeight(); ++y)
	{
		for (sint x = 0; x < (sint) packedZoneGrid.getWidth(); ++x)
		{
			if (packedZoneGrid(x, y))
			{				
				packedZonesArray.push_back(packedZoneGrid(x, y));
			}
		}
	}

	// build packed world
	CPackedWorld pw;
	pw.build(packedZonesArray);	


	bool fly = true;
	CConfigFile::CVar *flyVar = cf.getVarPtr("Fly");
	if (flyVar)
	{
		fly = flyVar->asBool();
	}
	
	if (!fly) return 0;

	// fly into scene	
	try
	{	
		CNELU::init(1024, 768, CViewport(), 32, true, EmptyWindow, false, true);
	}
	catch(const Exception &e)
	{		
		puts(e.what());
		getchar();
		return -1;	
	}
	//
	CFrustum frust;
	frust.init(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 150.f, 1);
	//
	
	CMatrix camMat;
	camMat.identity();
	camMat.setPos(camPos);
	//
	IDriver *driver = CNELU::Driver;
	CEvent3dMouseListener mouseListener;
	mouseListener.setMatrix(camMat);
	mouseListener.setMouseMode(U3dMouseListener::firstPerson);
	mouseListener.setFrustrum(frust);
	mouseListener.addToServer(CNELU::EventServer);
	mouseListener.setSpeed(camSpeed);
	CVertexBuffer vb;
	vb.setVertexFormat(CVertexBuffer::PositionFlag);
	vb.setPreferredMemory(CVertexBuffer::AGPVolatile, false);
	//
	CMaterial material;
	material.initUnlit();
	material.setDoubleSided(true);
	material.setZFunc(CMaterial::lessequal);
	//
	do
	{
		printf("zone = %s \n", posToZoneName(camMat.getPos().x, camMat.getPos().y).c_str());
		//
		CFile::checkFileChange();
		if (newCamPosWanted)
		{
			camMat.setPos(newCamPos);
			mouseListener.setMatrix(camMat);
			newCamPosWanted = false;
		}
		if (newCamSpeedWanted)
		{
			mouseListener.setSpeed(newCamSpeed);						
			newCamSpeedWanted = false;
		}
		const CRGBA clearColor = CRGBA(0, 0, 127, 0);
		driver->enableFog(true);
		driver->setupFog(frust.Far * 0.8f, frust.Far, clearColor);		
		CViewport vp;
		vp.init(0.f, 0.f, 1.f, 1.f);
		driver->setupViewport(vp);
		CScissor scissor;
		viewportToScissor(vp, scissor);		
		driver->setupScissor(scissor);

		CNELU::EventServer.pump();
		camMat = mouseListener.getViewMatrix();
		//
		driver->clear2D(clearColor);
		driver->clearZBuffer();
		//
		driver->setFrustum(frust.Left, frust.Right, frust.Bottom, frust.Top, frust.Near, frust.Far, frust.Perspective);
		driver->setupViewMatrix(camMat.inverted());
		driver->setupModelMatrix(CMatrix::Identity);
		//		
		//		
		const CVector localFrustCorners[8] = 
		{
			CVector(frust.Left, frust.Near, frust.Top),
			CVector(frust.Right, frust.Near, frust.Top),
			CVector(frust.Right, frust.Near, frust.Bottom),
			CVector(frust.Left, frust.Near, frust.Bottom),
			CVector(frust.Left  * frust.Far / frust.Near, frust.Far, frust.Top * frust.Far / frust.Near),
			CVector(frust.Right * frust.Far / frust.Near, frust.Far, frust.Top * frust.Far / frust.Near),
			CVector(frust.Right * frust.Far / frust.Near, frust.Far, frust.Bottom * frust.Far / frust.Near),
			CVector(frust.Left  * frust.Far / frust.Near, frust.Far, frust.Bottom * frust.Far / frust.Near)
		};
		// roughly compute covered zones		
		//
		sint frustZoneMinX = INT_MAX;
		sint frustZoneMaxX = INT_MIN;
		sint frustZoneMinY = INT_MAX;
		sint frustZoneMaxY = INT_MIN;
		for(uint k = 0; k < sizeofarray(localFrustCorners); ++k)
		{
			CVector corner = camMat * localFrustCorners[k];
			sint zoneX = (sint) (corner.x / 160.f) - zoneMinX;
			sint zoneY = (sint) floorf(corner.y / 160.f) - zoneMinY;
			frustZoneMinX = std::min(frustZoneMinX, zoneX);
			frustZoneMinY = std::min(frustZoneMinY, zoneY);
			frustZoneMaxX = std::max(frustZoneMaxX, zoneX);
			frustZoneMaxY = std::max(frustZoneMaxY, zoneY);
		}			

		const uint TRI_BATCH_SIZE = 10000; // batch size for rendering
		
		
		//for (sint y = frustZoneMinY; y <= frustZoneMaxY; ++y)
		//{
			//if (y < 0) continue;
			//if (y >= (sint) gridHeight) break;
			//for (sint x = frustZoneMinX; x <= frustZoneMaxX; ++x)
			//{
				//if (x < 0) continue;
				//if (x >= (sint) gridWidth) break;
				//if (packedZoneGrid(x, y))
				//{
					//packedZoneGrid(x, y)->render(vb, *driver, material, camMat, TRI_BATCH_SIZE, localFrustCorners);
				//}
			//}
		//}
				

		for (sint y = 0; y <= (sint) gridHeight; ++y)
		{
			if (y < 0) continue;
			if (y >= (sint) gridHeight) break;
			for (sint x = 0; x <= (sint) gridWidth; ++x)
			{
				if (x < 0) continue;
				if (x >= (sint) gridWidth) break;
				if (packedZoneGrid(x, y))
				{
					packedZoneGrid(x, y)->render(vb, *driver, material, camMat, TRI_BATCH_SIZE, localFrustCorners);
				}
			}
		}
		
		driver->setPolygonMode(IDriver::Filled);
		material.setColor(CRGBA::Green);
		// compute intersection with landscape & display a dot at that position
		CVector lookAtPos = camMat.getPos() + 1000.f * camMat.getJ();
		CVector inter;
		static std::vector<CTriangle> triList;		
		triList.clear();
		bool interFound = pw.raytrace(camMat.getPos(), lookAtPos, inter, &triList);
		if (!triList.empty())
		{
			vb.setNumVertices(3 * triList.size());
			CVertexBufferReadWrite vba;
			vb.lock(vba);
			CVector *dest = vba.getVertexCoordPointer(0);
			memcpy(dest, &triList[0], sizeof(CTriangle) * triList.size());
			vba.unlock();
			driver->activeVertexBuffer(vb);
			driver->renderRawTriangles(material, 0, triList.size());
		}				
		if (interFound)
		{
			material.setColor(CRGBA::Magenta);
			CQuad q;
			q.V0 = inter - camMat.getI() + camMat.getK();
			q.V1 = inter + camMat.getI() + camMat.getK();
			q.V2 = inter + camMat.getI() - camMat.getK();
			q.V3 = inter - camMat.getI() - camMat.getK();
			{
				vb.setNumVertices(4);
				CVertexBufferReadWrite vba;
				vb.lock(vba);
				CVector *dest = vba.getVertexCoordPointer(0);
				pushVBQuad(dest, q);
				vba.unlock();
			}
			driver->activeVertexBuffer(vb);
			driver->renderRawQuads(material, 0, 1);
		}		
		//		
//		for(uint k = 0; k < sizeofarray(frustCorners); ++k)
//		{
//			frustCorners[k] = camMat * frustCorners[k];
//			frustCorners[k].x -= cornerMin.x;
//			frustCorners[k].y -= cornerMin.y;
//		}
//		// project frustum on x/y plane to see where to test polys		
//		sint minY = INT_MAX;
//		CPolygon2D::TRasterVect silhouette;		
//		addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[2], frustCorners[3], silhouette, minY, CELL_SIZE);		
//		addQuadToSilhouette(frustCorners[1], frustCorners[5], frustCorners[6], frustCorners[2], silhouette, minY, CELL_SIZE);
//		addQuadToSilhouette(frustCorners[4], frustCorners[5], frustCorners[6], frustCorners[7], silhouette, minY, CELL_SIZE);
//		addQuadToSilhouette(frustCorners[0], frustCorners[4], frustCorners[7], frustCorners[3], silhouette, minY, CELL_SIZE);		
//		addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[5], frustCorners[4], silhouette, minY, CELL_SIZE);		
//		addQuadToSilhouette(frustCorners[3], frustCorners[7], frustCorners[6], frustCorners[2], silhouette, minY, CELL_SIZE);		
//		//	
//		driver->setPolygonMode(IDriver::Line);		
//		//
//		material.setColor(CRGBA::White);
//		//		
//		vb.setNumVertices(TRI_BATCH_SIZE * 3);		
//		{
//			CVertexBufferReadWrite vba;
//			vb.lock(vba);
//			CVector *dest = vba.getVertexCoordPointer(0);
//			const CVector *endDest = dest + TRI_BATCH_SIZE * 3;
//			for(sint y = 0; y < (sint) silhouette.size(); ++y)
//			{
//				sint gridY = y + minY;
//				if (gridY < 0) continue;
//				if (gridY >= triGridHeight) continue;
//				sint minX = silhouette[y].first;
//				sint maxX = silhouette[y].second;
//				for (sint x = minX; x <= maxX; ++x)
//				{
//					if (x < 0) continue;
//					if (x >= triGridWidth) break;
//					sint triRefIndex = triGrid(x, gridY);
//					while (triRefIndex != -1)
//					{
//						CTriangle tri = tris[triRefs[triRefIndex].TriIndex];
//						triRefIndex = triRefs[triRefIndex].NextTriRef;						
//						tri.V0.x += cornerMin.x;
//						tri.V0.y += cornerMin.y;
//						tri.V1.x += cornerMin.x;
//						tri.V1.y += cornerMin.y;
//						tri.V2.x += cornerMin.x;
//						tri.V2.y += cornerMin.y;
//						*dest++ = tri.V0;
//						*dest++ = tri.V1;
//						*dest++ = tri.V2;
//						if (dest == endDest)
//						{
//							// flush batch
//							vba.unlock();
//							material.setColor(CRGBA(100, 100, 100));
//							driver->setPolygonMode(IDriver::Filled);
//							driver->renderRawTriangles(material, 0, TRI_BATCH_SIZE);
//							material.setColor(CRGBA::White);
//							driver->setPolygonMode(IDriver::Line);
//							driver->renderRawTriangles(material, 0, TRI_BATCH_SIZE);
//							// reclaim a new batch
//							vb.lock(vba);
//							dest = vba.getVertexCoordPointer(0);
//							endDest = dest + TRI_BATCH_SIZE * 3;
//						}
//					}
//				}
//			}
//			vba.unlock();
//			uint numRemainingTris = TRI_BATCH_SIZE - ((endDest - dest) / 3);
//			if (numRemainingTris)
//			{
//				material.setColor(CRGBA(100, 100, 100));
//				driver->setPolygonMode(IDriver::Filled);
//				driver->activeVertexBuffer(vb);
//				driver->renderRawTriangles(material, 0, numRemainingTris);
//				material.setColor(CRGBA::White);
//				driver->setPolygonMode(IDriver::Line);
//				driver->renderRawTriangles(material, 0, numRemainingTris);
//			}
//		}
		
		// prepare 2D view
		
//		const uint GRID_EXTENT = 20;
//		sint currPosX = (sint) (camMat.getPos().x / CELL_SIZE);
//		sint currPosY = (sint) (camMat.getPos().y / CELL_SIZE);
//		driver->setFrustum( - (float) (GRID_EXTENT - 1) * CELL_SIZE,  (float) (GRID_EXTENT - 1) * CELL_SIZE,
//				                - (float) (GRID_EXTENT - 1) * CELL_SIZE,  (float) (GRID_EXTENT - 1) * CELL_SIZE, 0.f, 1.f, false);		
//		vp.init(0.1f, 0.1f, 0.4f ,0.4f);
//		driver->setupViewport(vp);		
//		viewportToScissor(vp, scissor);		
//		driver->setupScissor(scissor);			
//		//			
//		driver->clear2D(CRGBA(0, 127, 0, 0));
//		driver->clearZBuffer();
//		CMatrix viewMatrix;
//		viewMatrix.setRot(CVector::I, -CVector::K, CVector::J);
//		viewMatrix.setPos(CVector(camMat.getPos().x, camMat.getPos().y, 0.f));
//		viewMatrix.invert();
//		driver->setupViewMatrix(viewMatrix);
//		driver->setupModelMatrix(CMatrix::Identity);
//
//		driver->setPolygonMode(IDriver::Filled);
		
//		// draw covered portion of the grid
//		material.setColor(CRGBA(127, 0, 0));
//		{
//				CVertexBufferReadWrite vba;
//				vb.lock(vba);		
//				CVector *dest = vba.getVertexCoordPointer(0);
//				sint numQuads = 0;				
//				for(sint y = 0; y < (sint) silhouette.size(); ++y)
//				{
//					if (silhouette[y].first > silhouette[y].second) continue;
//					CQuad q;
//					q.V0.x = silhouette[y].first * CELL_SIZE + cornerMin.x;
//					q.V0.y = (y + minY) * CELL_SIZE + cornerMin.y;
//					q.V1.x = (silhouette[y].second + 1) * CELL_SIZE + cornerMin.x;
//					++ numQuads;
//					q.V1.y = q.V0.y;
//					q.V2.x = q.V1.x;
//					q.V2.y = q.V1.y + CELL_SIZE;
//					q.V3.x = q.V0.x;
//					q.V3.y = q.V2.y;
//					pushVBQuad2D(dest, q);
//				}
//				nlassert(numQuads * 4 < TRI_BATCH_SIZE * 3);
//				vba.unlock();
//				driver->renderRawQuads(material, 0, numQuads);
//		}
//		*/
		//
		/*
//		driver->setPolygonMode(IDriver::Line);
//		// draw grid around & frustum
//		{						
//			//						
//			material.setColor(CRGBA(127, 127, 127));
//			{
//				CVertexBufferReadWrite vba;
//				vb.lock(vba);
//				CVector *dest = vba.getVertexCoordPointer(0);				
//				for(sint x = currPosX - GRID_EXTENT; x <= currPosX + (sint) GRID_EXTENT; ++x)
//				{
//					pushVBLine2D(dest, CVector(x * CELL_SIZE, (currPosY + (sint) GRID_EXTENT) * CELL_SIZE, 0.f), 
//						               CVector(x * CELL_SIZE, (currPosY - (sint) GRID_EXTENT) * CELL_SIZE, 0.f));
//				}
//				for(sint y = currPosY - GRID_EXTENT; y <= currPosY + (sint) GRID_EXTENT; ++y)
//				{
//					pushVBLine2D(dest, CVector((currPosX - (sint) GRID_EXTENT) * CELL_SIZE, y * CELL_SIZE, 0.f), 
//						               CVector((currPosX + (sint) GRID_EXTENT) * CELL_SIZE, y * CELL_SIZE, 0.f));
//				}
//				vba.unlock();
//				uint numTri = 2 * (2 * GRID_EXTENT + 1);
//				nlassert(numTri <= TRI_BATCH_SIZE);
//				driver->renderRawTriangles(material, 0, numTri);
//			}			
//			material.setColor(CRGBA::Red);
//			{
//				CVertexBufferReadWrite vba;
//				vb.lock(vba);
//				CVector *dest = vba.getVertexCoordPointer(0);
//				for(uint k = 0; k < sizeofarray(localFrustCorners); ++k)
//				{
//					frustCorners[k].x += cornerMin.x;
//					frustCorners[k].y += cornerMin.y;					
//				}
//				pushVBLine2D(dest, frustCorners[1], frustCorners[5]);
//				pushVBLine2D(dest, frustCorners[5], frustCorners[6]);
//				pushVBLine2D(dest, frustCorners[6], frustCorners[2]);
//				pushVBLine2D(dest, frustCorners[2], frustCorners[1]);
//				//
//				pushVBLine2D(dest, frustCorners[5], frustCorners[4]);
//				pushVBLine2D(dest, frustCorners[4], frustCorners[7]);
//				pushVBLine2D(dest, frustCorners[7], frustCorners[6]);
//				//
//				pushVBLine2D(dest, frustCorners[4], frustCorners[0]);
//				pushVBLine2D(dest, frustCorners[0], frustCorners[3]);
//				pushVBLine2D(dest, frustCorners[3], frustCorners[7]);
//				//
//				pushVBLine2D(dest, frustCorners[0], frustCorners[1]);
//				pushVBLine2D(dest, frustCorners[3], frustCorners[2]);
//				//
//				nlassert(12 <= TRI_BATCH_SIZE);
//				vba.unlock();
//				material.setColor(CRGBA::Red);
//				driver->renderRawTriangles(material, 0, 12);
//			}			
//		}
//		
		
//		// draw underlying geometry
//		material.setColor(CRGBA::White);
//		{
//			CVertexBufferReadWrite vba;
//			vb.lock(vba);
//			CVector *dest = vba.getVertexCoordPointer(0);
//			const CVector *endDest = dest + TRI_BATCH_SIZE * 3;
//			// compute cam pos in tri grid
//			currPosX = (sint) ((camMat.getPos().x - cornerMin.x) / CELL_SIZE);
//			currPosY = (sint) ((camMat.getPos().y - cornerMin.y) / CELL_SIZE);
//			for(sint y = currPosY - GRID_EXTENT; y < currPosY + (sint) GRID_EXTENT; ++y)
//			{
//				for(sint x = currPosX - GRID_EXTENT; x < currPosX + (sint) GRID_EXTENT; ++x)
//				{					
//					if (y < 0) continue;
//					if (y >= triGridHeight) break;
//					if (x < 0) continue;
//					if (x >= triGridWidth) break;					
//					sint triRefIndex = triGrid(x, y);
//					while (triRefIndex != -1)
//					{
//						CTriangle tri = tris[triRefs[triRefIndex].TriIndex];
//						tri.V0.x += cornerMin.x;
//						tri.V0.y += cornerMin.y;
//						tri.V1.x += cornerMin.x;
//						tri.V1.y += cornerMin.y;
//						tri.V2.x += cornerMin.x;
//						tri.V2.y += cornerMin.y;
//
//						triRefIndex = triRefs[triRefIndex].NextTriRef;
//						pushVBTri2D(dest, tri);						
//						if (dest == endDest)
//						{
//							// flush batch
//							vba.unlock();
//							driver->renderRawTriangles(material, 0, TRI_BATCH_SIZE);
//							// reclaim a new batch
//							vb.lock(vba);
//							dest = vba.getVertexCoordPointer(0);
//							endDest = dest + TRI_BATCH_SIZE * 3;
//						}
//					}					
//				}
//			}
//			vba.unlock();
//			uint numRemainingTris = TRI_BATCH_SIZE - ((endDest - dest) / 3);
//			if (numRemainingTris)
//			{
//				driver->renderRawTriangles(material, 0, numRemainingTris);
//			}
//		}
		
		uint NUM_RAYS = 10000;
		if (CNELU::AsyncListener.isKeyPushed(KeyT))
		{
			printf("Starting raytracing test 1");			
			uint numHits = 0;
			NLMISC::TTime startTime = CTime::getLocalTime();
			for(uint k = 0; k < NUM_RAYS; ++k)
			{
				sint zone = rand() % packedZonesArray.size();
				CPackedZoneBase *zonePtr = packedZonesArray[zone];
				CVector cornerMin = zonePtr->Box.getMin();
				CVector cornerMax = zonePtr->Box.getMax();
				CVector start(frand(1.f) * (cornerMax.x - cornerMin.x) + cornerMin.x,
					          frand(1.f) * (cornerMax.y - cornerMin.y) + cornerMin.y,
							  frand(1.f) * (cornerMax.z - cornerMin.z) + cornerMin.z);
				CVector end(frand(1.f) * (cornerMax.x - cornerMin.x) + cornerMin.x,
					          frand(1.f) * (cornerMax.y - cornerMin.y) + cornerMin.y,
							  frand(1.f) * (cornerMax.z - cornerMin.z) + cornerMin.z);
				CVector inter;
				bool result = pw.raytrace(start, end, inter);
				if (result)
				{
					++numHits;
				}
			}
			NLMISC::TTime endTime = CTime::getLocalTime();

			float dt = (float) (endTime - startTime) / 1000.f;			
			printf("Total time = %.2f s\n", dt);
			printf("Num rays = %d\n", (int) NUM_RAYS);
			printf("Num hits = %d\n", (int) numHits);
			printf("Num test per seconds = %.2f\n", (float) NUM_RAYS / dt);
		}		
		//
		if (CNELU::AsyncListener.isKeyPushed(KeyU))
		{
			printf("Starting raytracing test 2");			
			uint numHits = 0;
			NLMISC::TTime startTime = CTime::getLocalTime();
			for(uint k = 0; k < NUM_RAYS; ++k)
			{				
				CVector start = camMat.getPos();					          
				CVector end(start.x + (frand(200.f) - 100.f),
					        start.y + (frand(200.f) - 100.f),
							start.z + (frand(5.f) - 2.5f));					          
				CVector inter;
				bool result = pw.raytrace(start, end, inter);
				if (result)
				{
					++numHits;
				}
			}
			NLMISC::TTime endTime = CTime::getLocalTime();

			float dt = (float) (endTime - startTime) / 1000.f;
			printf("Total time = %.2f s\n", dt);
			printf("Num rays = %d\n", (int) NUM_RAYS);
			printf("Num hits = %d\n", (int) numHits);
			printf("Num test per seconds = %.2f\n", (float) NUM_RAYS / dt);			
		}
		//
		driver->setPolygonMode(IDriver::Filled);
		//
		driver->swapBuffers();
	}
	while(!CNELU::AsyncListener.isKeyPushed(KeyESCAPE));


	
	return 0;
}

*/


