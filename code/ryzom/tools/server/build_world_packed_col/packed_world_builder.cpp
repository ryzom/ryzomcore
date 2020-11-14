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
#include "zone_util.h"
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
#include "nel/3d/landscape.h"
#include "nel/3d/zone.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/event_mouse_listener.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/nelu.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/shape_info.h"
#include "nel/3d/packed_zone.h"
#include "nel/3d/packed_world.h"
#include "nel/3d/texture_file.h"
//
using namespace NL3D;
using namespace NLMISC;


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


//********************************************************************************************************************
void CPackedWorldBuilder::build(const std::vector<std::string> &zoneNames, const std::string &cachePath, bool addLandscapeIG, CPackedWorld &dest, float refineTheshold)
{	
	std::vector<CSmartPtr<CZoneRefCount> >   zones;
	zones.reserve(zoneNames.size());	
	sint zoneMinX = 0;
	sint zoneMaxX = 0;
	sint zoneMinY = 0;
	sint zoneMaxY = 0;
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
		nlwarning("No zones loaded");
		return;
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

	/*
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
	*/

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
				/*
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
				}*/
								
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
							if (f.open(cacheFilename))
							{
								CPackedZoneBase *pb = NULL;
								f.serialPolyPtr(pb);	
								packedZoneGrid(x, y) = pb;
								mustRebuild = false;
								nlinfo("Retrieving zone %d / %d from cache\n", (int) currZoneIndex + 1, (int) zones.size());
							}
						}
						catch(const EStream &)
						{
						}
					}
				}

				if (mustRebuild)
				{
					nlinfo("Rebuilding zone %d / %d \n", (int) currZoneIndex + 1, (int) zones.size());
					CLandscape *landscape = new CLandscape;
					landscape->init();
					landscape->setThreshold (refineTheshold / (1000.f * 1000.f));
					//landscape->setThreshold(0);
					landscape->setTileMaxSubdivision (0);
					// add wanted zones & all zones around for continuity
					for (sint ly = y - 1; ly <= y + 1; ++ly)
					{
						if (ly < 0) continue;
						if (ly >= (sint) gridHeight) break;
						for (sint lx = x - 1; lx <= x + 1; ++lx)
						{
							if (lx < 0) continue;
							if (lx >= (sint) gridWidth)  break;						
							if (zoneGrid(lx, ly))
							{
								if (!zoneGrid(lx, ly)->Loaded)
								{
									
									try
									{										
										CIFile stream(CPath::lookup(zoneGrid(lx, ly)->Path));
										zoneGrid(lx, ly)->Zone.serial(stream);										
										zoneGrid(lx, ly)->Loaded = true;
									}
									catch(const Exception &)
									{
										nlwarning("Error while loading zone %s : zone not loaded \n", zoneGrid(lx, ly)->Path.c_str());
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
					/*if (addVillages)
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
					}*/					
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
	dest.build(packedZonesArray);	
}


//********************************************************************************************************************
void CPackedWorldBuilder::fly(std::vector<CIslandInfo>  &islands, float camSpeed)
{	
	nlassert(!islands.empty());	
	// fly into scene	
	try
	{	
		CNELU::init(1024, 768, CViewport(), 32, true, EmptyWindow, false, true);
	}
	catch(const Exception &e)
	{		
		nlwarning(e.what());		
		return;	
	}
	//
	CFrustum frust;
	frust.init(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 1500.f, 1);
	//
	
	CMatrix camMat;		
	//
	IDriver *driver = CNELU::Driver;
	CEvent3dMouseListener mouseListener;	
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
	CMaterial wiredMaterial;
	wiredMaterial.initUnlit();
	wiredMaterial.setDoubleSided(true);
	wiredMaterial.setZFunc(CMaterial::lessequal);
	wiredMaterial.setColor(CRGBA(255, 255, 255, 250));
	wiredMaterial.texEnvOpAlpha(0, CMaterial::Replace);
	wiredMaterial.texEnvArg0Alpha(0, CMaterial::Diffuse, CMaterial::SrcAlpha);
	wiredMaterial.setBlend(true);
	wiredMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
	CMaterial texturedMaterial;
	texturedMaterial.initUnlit();
	texturedMaterial.setDoubleSided(true);
	texturedMaterial.setZFunc(CMaterial::lessequal);
	//
	uint currWorldIndex = std::numeric_limits<uint>::max();
	bool newPosWanted = true;
	//
	std::vector<TPackedZoneBaseSPtr> zones;	
	//
	CMaterial *zoneMat = NULL;
	do
	{		
		if (newPosWanted)
		{
			currWorldIndex = (currWorldIndex + 1) % islands.size();
			const CIslandInfo &island = islands[currWorldIndex];
			camMat.identity();
			camMat.setPos(island.StartPosition);
			mouseListener.setMatrix(camMat);
			nlassert(island.PW);
			islands[currWorldIndex].PW->getZones(zones);			
			nlwarning("zone = %s \n", posToZoneName(camMat.getPos().x, camMat.getPos().y).c_str());			
			newPosWanted = false;			
			// setup material with projected texture if there's one
			if (island.TexName.empty())
			{
				zoneMat = &material;				
			}
			else
			{
				zoneMat = &texturedMaterial;
				// just add a projected texture
				CTextureFile *newTex = new CTextureFile(island.TexName);
				newTex->setWrapS(ITexture::Clamp);
				newTex->setWrapT(ITexture::Clamp);
				texturedMaterial.setTexture(0, newTex);
				texturedMaterial.texEnvOpRGB(0, CMaterial::Replace);
				texturedMaterial.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
				texturedMaterial.setTexCoordGen(0, true);
				texturedMaterial.setTexCoordGenMode(0, CMaterial::TexCoordGenObjectSpace);
				CMatrix mat;
				CVector scale = island.CornerMax - island.CornerMin;
				scale.x = 1.f / favoid0(scale.x);
				scale.y = 1.f / favoid0(scale.y);
				scale.z = 0.f;
				mat.setScale(scale);
				mat.setPos(CVector(- island.CornerMin.x * scale.x, - island.CornerMin.y * scale.y, 0.f));
				//
				CMatrix uvScaleMat;
				//
				uvScaleMat.setScale(CVector(island.UScale, - island.VScale, 0.f));
				uvScaleMat.setPos(CVector(0.f, island.VScale, 0.f));
				//
				texturedMaterial.enableUserTexMat(0, true);
				texturedMaterial.setUserTexMat(0, uvScaleMat * mat);
			}	
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
		/*
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
		*/

		const uint TRI_BATCH_SIZE = 10000; // batch size for rendering
		
								
		for(uint k = 0; k < zones.size(); ++k)
		{
			zones[k]->render(vb, *driver, *zoneMat, wiredMaterial, camMat, TRI_BATCH_SIZE, localFrustCorners);
		}				
		
		driver->setPolygonMode(IDriver::Filled);
		material.setColor(CRGBA::Green);
		// compute intersection with landscape & display a dot at that position
		CVector lookAtPos = camMat.getPos() + 1000.f * camMat.getJ();
		CVector inter;
		static std::vector<CTriangle> triList;		
		triList.clear();
		bool interFound = islands[currWorldIndex].PW->raytrace(camMat.getPos(), lookAtPos, inter, &triList);
		if (!triList.empty())
		{
			vb.setNumVertices(3 * (uint32)triList.size());
			CVertexBufferReadWrite vba;
			vb.lock(vba);
			CVector *dest = vba.getVertexCoordPointer(0);
			memcpy(dest, &triList[0], sizeof(CTriangle) * triList.size());
			vba.unlock();
			driver->activeVertexBuffer(vb);
			driver->renderRawTriangles(material, 0, (uint32)triList.size());
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
		/*
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
		*/
		//
		/*
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
		*/
		if (CNELU::AsyncListener.isKeyPushed(KeyN))
		{
			newPosWanted = true;
		}
		//
		driver->setPolygonMode(IDriver::Filled);
		//
		driver->swapBuffers();
	}
	while(!CNELU::AsyncListener.isKeyPushed(KeyESCAPE));
	//	
}
