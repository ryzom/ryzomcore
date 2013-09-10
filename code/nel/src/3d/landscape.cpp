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

#include "std3d.h"


#include "nel/3d/landscape.h"
#include "nel/3d/landscape_model.h"
#include "nel/misc/bsphere.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/texture_far.h"
#include "nel/3d/landscape_profile.h"
#include "nel/3d/height_map.h"
#include "nel/3d/tile_noise_map.h"
#include "nel/3d/vegetable_manager.h"
#include "nel/3d/vegetable.h"
#include "nel/3d/landscape_vegetable_block.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/tile_vegetable_desc.h"
#include "nel/3d/texture_dlm.h"
#include "nel/3d/patchdlm_context.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/scene.h"


#include "nel/3d/vertex_program.h"

using namespace NLMISC;
using namespace std;



namespace NL3D
{


// ***************************************************************************
/*
	Target is 20K faces  in frustum.
	So 80K faces at same time
	So 160K elements (bin tree).
	A good BlockSize (in my opinion) is EstimatedMaxSize / 10, to have less memory leak as possible,
	and to make not so many system allocation.

	NL3D_TESSRDR_ALLOC_BLOCKSIZE is 2 times less, because elements are in Far zone or in Near zone only
	(approx same size...)
*/
#define	NL3D_TESS_ALLOC_BLOCKSIZE		16000
#define	NL3D_TESSRDR_ALLOC_BLOCKSIZE	8000


// ***************************************************************************
// This value is important for the precision of the priority list
#define	NL3D_REFINE_PLIST_DIST_STEP		0.0625
/* This value is important, because faces will be inserted at maximum at this entry in the priority list.
	If not so big (eg 50 meters), a big bunch of faces may be inserted in this entry, which may cause slow down
	sometimes, when all this bunch comes to 0 in the priority list.
	To avoid such a thing, see CTessFacePriorityList::init(), and use of NL3D_REFINE_PLIST_DIST_MAX_MOD.
	Here, distMax= 2048*0.0625= 128
*/
#define	NL3D_REFINE_PLIST_NUM_ENTRIES			2048
#define	NL3D_REFINE_PLIST_DIST_MAX_MOD			0.7f
// For the Split priority list only, numbers of quadrants. MergeList has 0 quadrants.
#define	NL3D_REFINE_PLIST_SPLIT_NUMQUADRANT		16


/*
	OverHead size of one RollingTable of priority list is 8 * (NL3D_REFINE_PLIST_NUM_ENTRIES)
	So here, it is "only" 16K.

	Since we have 2 Priority list and 16 quadrants for the split one, the total overhead is 18*12.8= 288K
*/


// ***************************************************************************
// Size (in cases) of the quadgrid. must be e power of 2.
const uint			CLandscape::_PatchQuadGridSize= 128;
// Size of a case of the quadgrid.
const float			CLandscape::_PatchQuadGridEltSize= 16;


// ***************************************************************************

// Bitmap Cross

class CTextureCross : public ITexture
{
public:
	/**
	 * Generate the texture
	 * \author Stephane Coutelas
	 * \date 2000
	 */
	virtual void doGenerate(bool /* async */)
	{
		// Resize
		resize (16, 16);

		// Cross
		static const uint32 cross[16*16]=
		{
			//  0			1			2			3			4			5			6			7			8			9			10			11			12			13			14			15
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		};

		// Null
		memcpy (&_Data[0][0], cross, 16*16*4);
	}

	// Dummy serial...
	virtual void	serial(NLMISC::IStream &/* f */)  throw(NLMISC::EStream) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureCross);
};


// ***************************************************************************
const char	*EBadBind::what() const throw()
{
	sint			numErr= 0;
	const	sint	NErrByLines= 4;

	_Output= "Landscape Bind Error in (3DSMax indices!! (+1) ): ";

	std::list<CBindError>::const_iterator		it;
	for(it= BindErrors.begin();it!=BindErrors.end(); it++, numErr++)
	{
		char	tmp[256];
		sint	x= it->ZoneId & 255;
		sint	y= it->ZoneId >> 8;
		sprintf(tmp, "zone%3d_%c%c.patch%3d;   ", y+1, (char)('A'+(x/26)), (char)('A'+(x%26)), it->PatchId+1);
		if( (numErr%NErrByLines) == 0)
			_Output+= "\n";
		_Output+= tmp;
	}
	return _Output.c_str();
}


// ***************************************************************************
// Init BlockAllcoator with standard BlockMemory.
CLandscape::CLandscape() :
	TessFaceAllocator(NL3D_TESS_ALLOC_BLOCKSIZE),
	TessVertexAllocator(NL3D_TESS_ALLOC_BLOCKSIZE),
	TessNearVertexAllocator(NL3D_TESSRDR_ALLOC_BLOCKSIZE),
	TessFarVertexAllocator(NL3D_TESSRDR_ALLOC_BLOCKSIZE),
	TileMaterialAllocator(NL3D_TESSRDR_ALLOC_BLOCKSIZE),
	TileFaceAllocator(NL3D_TESSRDR_ALLOC_BLOCKSIZE),
	_Far0VB(CLandscapeVBAllocator::Far0, "LandscapeFar0VB"),
	_Far1VB(CLandscapeVBAllocator::Far1, "LandscapeFar1VB"),
	_TileVB(CLandscapeVBAllocator::Tile, "LandscapeTileVB")
{
	OwnerModel = NULL;

	// Init the Tile Infos with Max TileId
	TileInfos.resize(NL3D::NbTilesMax, NULL);

	// Far texture not initialized till initTileBanks is not called
	_FarInitialized=false;

	// Init far lighting with White/black
	setupStaticLight (CRGBA(255,255,255), CRGBA(0,0,0), 1.f);
	// Default material for pointLights
	_PointLightDiffuseMaterial= CRGBA::White;

	_FarTransition= 10;		// 10 meters.
	_TileDistNear=100.f;
	_Threshold= 0.001f;
	_RefineMode=true;

	_TileMaxSubdivision= 0;

	_NFreeLightMaps= 0;

	// By default Automatic light comes from up.
	_AutomaticLighting = false;
	_AutomaticLightDir= -CVector::K;

	// By default, noise is enabled.
	_NoiseEnabled= true;

	// By default, we compute Geomorph and Alpha in software.
	_VertexShaderOk= false;
	_VPThresholdChange= false;

	_RenderMustRefillVB= false;

	// priority list.
	_MustRefineAllAtNextRefine= true;
	_SplitPriorityList.init(NL3D_REFINE_PLIST_DIST_STEP, NL3D_REFINE_PLIST_NUM_ENTRIES, NL3D_REFINE_PLIST_DIST_MAX_MOD, NL3D_REFINE_PLIST_SPLIT_NUMQUADRANT);
	// See updateRefine* Doc in tesselation.cpp for why the merge list do not need quadrants.
	_MergePriorityList.init(NL3D_REFINE_PLIST_DIST_STEP, NL3D_REFINE_PLIST_NUM_ENTRIES, NL3D_REFINE_PLIST_DIST_MAX_MOD, 0);
	// just for getTesselatedPos to work properly.
	_OldRefineCenter= CVector::Null;

	// create / Init the vegetable manager.
	_VegetableManager= new CVegetableManager(NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_UNLIT, NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_LIGHTED);

	// Init vegetable  setup.
	_VegetableManagerEnabled= false;
	_DriverOkForVegetable= false;
	// default global vegetable color, used for dynamic lighting only (arbitrary).
	_DLMGlobalVegetableColor.set(180, 180, 180);

	_PZBModelPosition= CVector::Null;


	// Default: no updateLighting.
	_ULFrequency= 0;
	_ULPrecTimeInit= false;
	// Default: no textureFar created.
	_ULTotalFarPixels= 0;
	_ULFarPixelsToUpdate= 0;
	_ULRootTextureFar= NULL;
	// Default: no patch created
	_ULTotalNearPixels= 0;
	_ULNearPixelsToUpdate= 0;
	_ULRootNearPatch= NULL;
	_ULNearCurrentTessBlockId= 0;


	// Dynamic Lighting.
	_TextureDLM= new CTextureDLM(NL3D_LANDSCAPE_DLM_WIDTH, NL3D_LANDSCAPE_DLM_HEIGHT);
	_PatchDLMContextList= new CPatchDLMContextList;
	_DLMMaxAttEnd= 30.f;

	CLandscapeGlobals::PassTriArray.setFormat(NL_LANDSCAPE_INDEX_FORMAT);

	// Alloc some global space for tri rendering.
	if( CLandscapeGlobals::PassTriArray.getNumIndexes() < 1000 )
		CLandscapeGlobals::PassTriArray.setNumIndexes( 1000 );

	// set volatile index buffer to avoid stalls
	CLandscapeGlobals::PassTriArray.setPreferredMemory(CIndexBuffer::RAMVolatile, false);

	_LockCount = 0;

	_TextureTileCategory= new ITexture::CTextureCategory("LANDSCAPE TILES");
	_TextureFarCategory= new ITexture::CTextureCategory("LANDSCAPE FAR");
	_TextureNearCategory= new ITexture::CTextureCategory("LANDSCAPE LIGHTMAP NEAR");
}

// ***************************************************************************
CLandscape::~CLandscape()
{
	clear();

	// release the VegetableManager.
	delete _VegetableManager;
	_VegetableManager= NULL;

	// Dynamic Lighting.
	// smartPtr delete
	_TextureDLM= NULL;
	delete _PatchDLMContextList;
	_PatchDLMContextList= NULL;
}


// ***************************************************************************
void			CLandscape::init()
{
	// Fill Far mat.
	// Must init his BlendFunction here!!! becaus it switch between blend on/off during rendering.
	FarMaterial.initUnlit();
	FarMaterial.setSrcBlend(CMaterial::srcalpha);
	FarMaterial.setDstBlend(CMaterial::invsrcalpha);

	// FarMaterial: pass trhough Alpha from diffuse.
	FarMaterial.texEnvOpAlpha(0, CMaterial::Replace);
	FarMaterial.texEnvArg0Alpha(0, CMaterial::Diffuse, CMaterial::SrcAlpha);
	FarMaterial.texEnvOpAlpha(1, CMaterial::Replace);
	FarMaterial.texEnvArg0Alpha(1, CMaterial::Diffuse, CMaterial::SrcAlpha);
	// FarMaterial: Add RGB from static lightmap and dynamic lightmap
	FarMaterial.texEnvOpRGB(0, CMaterial::Replace);
	FarMaterial.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
	FarMaterial.texEnvOpRGB(1, CMaterial::Add);
	FarMaterial.texEnvArg0RGB(1, CMaterial::Texture, CMaterial::SrcColor);
	FarMaterial.texEnvArg1RGB(1, CMaterial::Previous, CMaterial::SrcColor);


	// Init material for tile.
	TileMaterial.initUnlit();

	// init quadGrid.
	_PatchQuadGrid.create(_PatchQuadGridSize, _PatchQuadGridEltSize);
}


// ***************************************************************************
void			CLandscape::setThreshold (float thre)
{
	thre= max(thre, 0.f);
	if(thre != _Threshold)
	{
		_Threshold= thre;
		_VPThresholdChange= true;
		// force refine all at next refine
		_MustRefineAllAtNextRefine= true;
	}
}


// ***************************************************************************
void			CLandscape::setTileNear (float tileNear)
{
	tileNear= max(tileNear, _FarTransition);

	if(tileNear!=_TileDistNear)
	{
		_TileDistNear= tileNear;
		resetRenderFarAndDeleteVBFV();
		// force refine all at next refine
		_MustRefineAllAtNextRefine= true;
	}

}


// ***************************************************************************
void			CLandscape::setTileMaxSubdivision (uint tileDiv)
{
	nlassert(tileDiv<=4);

	if(tileDiv!=_TileMaxSubdivision)
	{
		_TileMaxSubdivision= tileDiv;
		// Force at Tile==0. Nex refine will split correctly.
		forceMergeAtTileLevel();
	}
}
// ***************************************************************************
uint 			CLandscape::getTileMaxSubdivision ()
{
	return _TileMaxSubdivision;
}


// ***************************************************************************
void			CLandscape::resetRenderFarAndDeleteVBFV()
{
	// For all patch of all zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		((*it).second)->resetRenderFarAndDeleteVBFV();
	}
}


// ***************************************************************************
void			CLandscape::forceMergeAtTileLevel()
{
	// For all patch of all zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		((*it).second)->forceMergeAtTileLevel();
	}
}


// ***************************************************************************
bool			CLandscape::addZone(const CZone	&newZone)
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (CVector::Null);
	// NB: adding a zone may add vertices in VB in visible patchs (because of binds)=> buffers are locked.

	uint16	zoneId= newZone.getZoneId();

	if(Zones.find(zoneId)!=Zones.end())
	{
		unlockBuffers();
		return false;
	}
	CZone	*zone= new CZone;

	// copy zone.
	zone->build(newZone);

	// Affect the current lighting of pointLight to the zone.
	if (OwnerModel)
	{
		CScene *scene = OwnerModel->getOwnerScene();
		zone->_PointLightArray.initAnimatedLightIndex(*scene);
		zone->_PointLightArray.setPointLightFactor(*scene);
	}

	// apply the landscape heightField, modifying BBoxes.
	zone->applyHeightField(*this);

	// compile the zone for this landscape.
	zone->compile(this, Zones);

	// For test of _PatchQuadGrid erase.
	CAABBox	zoneBBForErase=	zone->getZoneBB().getAABBox();
	// Avoid precision problems by enlarging a little bbox size of zone for erase
	zoneBBForErase.setHalfSize( zoneBBForErase.getHalfSize() * 1.1f);

	// add patchs of this zone to the quadgrid.
	for(sint i= 0; i<zone->getNumPatchs(); i++)
	{
		const CPatch *pa= ((const CZone*)zone)->getPatch(i);
		CPatchIdentEx	paId;
		paId.ZoneId= zoneId;
		paId.PatchId= uint16(i);
		paId.Patch= pa;
		CAABBox		bb= pa->buildBBox();
		_PatchQuadGrid.insert(bb.getMin(), bb.getMax(), paId);
		// NB: the bbox of zone is used to remove patch. Hence it is VERY important that zoneBBox includes
		// all patchs bbox (else some patchs entries may not be deleted in removeZone()).
		nlassert(zoneBBForErase.include(bb));
	}

	// Must realase VB Buffers
	unlockBuffers();

	// Because bind may add faces in other (visible) zones because of enforced split, we must check
	// and update any FaceVector.
	updateTessBlocksFaceVector();

	return true;
}
// ***************************************************************************
bool			CLandscape::removeZone(uint16 zoneId)
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (CVector::Null);
	// NB: remove a zone may change vertices in VB in visible patchs => buffers are locked.

	// find the zone.
	if(Zones.find(zoneId)==Zones.end())
	{
		unlockBuffers();
		return false;
	}
	CZone	*zone= Zones[zoneId];


	// delete patchs from this zone to the quadgrid.
	// use the quadgrid itself to find where patch are. do this using bbox of zone.
	CAABBox	zoneBBForErase=	zone->getZoneBB().getAABBox();
	// Avoid precision problems by enlarging a little bbox size of zone for erase
	zoneBBForErase.setHalfSize( zoneBBForErase.getHalfSize() * 1.1f);
	// select iterators in the area of this zone.
	_PatchQuadGrid.clearSelection();
	_PatchQuadGrid.select(zoneBBForErase.getMin(), zoneBBForErase.getMax());
	// for each patch, remove it if from deleted zone.
	CQuadGrid<CPatchIdentEx>::CIterator	it;
	sint	nPatchRemoved= 0;
	for(it= _PatchQuadGrid.begin(); it!= _PatchQuadGrid.end();)
	{
		// if the patch belong to the zone to remove
		if( (*it).ZoneId== zone->getZoneId() )
		{
			// remove from the quadgrid.
			it= _PatchQuadGrid.erase(it);
			nPatchRemoved++;
		}
		else
			it++;
	}
	// verify we have removed all patch in the quadGrid for this zone
	nlassert(nPatchRemoved==zone->getNumPatchs());


	// remove the zone.
	zone->release(Zones);
	delete zone;

	// Must realase VB Buffers
	unlockBuffers();

	// because of forceMerge() at unbind, removeZone() can cause change in faces in other (visible) zones.
	updateTessBlocksFaceVector();

	return true;
}
// ***************************************************************************
void			CLandscape::getZoneList(std::vector<uint16>	&zoneIds) const
{
	zoneIds.clear();
	zoneIds.reserve(Zones.size());
	std::map<uint16, CZone*>::const_iterator	it;
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		zoneIds.push_back((*it).first);
	}
}
// ***************************************************************************
void			CLandscape::buildZoneName(sint zoneId, std::string &zoneName)
{
	char	tmp[256];
	sint	x= zoneId & 255;
	sint	y= zoneId >> 8;
	sprintf(tmp, "%d_%c%c", y+1, (char)('A'+(x/26)), (char)('A'+(x%26)));
	zoneName= tmp;
}
// ***************************************************************************
void			CLandscape::clear()
{
	// Build the list of zoneId.
	vector<uint16>	zoneIds;
	getZoneList(zoneIds);

	// Remove each zone one by one.
	sint i;
	for(i=0;i<(sint)zoneIds.size();i++)
	{
		nlverify(removeZone(zoneIds[i]));
	}

	// ensure the quadgrid is empty.
	_PatchQuadGrid.clear();

	releaseAllTiles();

	// If not done, delete all VBhards allocated.
	_Far0VB.clear();
	_Far1VB.clear();
	_TileVB.clear();


	// Reset All Far Texture and unlink _ULRootTextureFar ciruclarList.
	ItSPRenderPassVector	itFar= _TextureFars.begin();
	// unitl set is empty
	while( itFar != _TextureFars.end() )
	{
		// erase with link update.
		clearFarRenderPass(*itFar);
		itFar++;
	}
	// delete all
	_TextureFars.clear();


	// reset driver.
	_Driver= NULL;
}

// ***************************************************************************
void			CLandscape::setDriver(IDriver *drv)
{
	nlassert(drv);
	if(_Driver != drv)
	{
		_Driver= drv;

		// Does the driver support VertexShader???
		// only if VP supported by GPU.
		_VertexShaderOk= (_Driver->supportVertexProgram() && !_Driver->isVertexProgramEmulated());


		// Does the driver has sufficient requirements for Vegetable???
		// only if VP supported by GPU, and Only if max vertices allowed.
		_DriverOkForVegetable= _VertexShaderOk && (_Driver->getMaxVerticesByVertexBufferHard()>=(uint)NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_MAX);

	}
}

// ***************************************************************************
void			CLandscape::clip(const CVector &refineCenter, const std::vector<CPlane>	&pyramid)
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (refineCenter);
	// NB: clip may add/remove vertices in VB in visible patchs => buffers are locked.


	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->clip(pyramid);
	}

	// Must realase VB Buffers
	unlockBuffers();

	// clip() should not cause change in faces in visible patchs.
	// It should not happens, but check for security.
	nlassert(_TessBlockModificationRoot.getNextToModify()==NULL);
	updateTessBlocksFaceVector();

}
// ***************************************************************************
void			CLandscape::refine(const CVector &refineCenter)
{
	NL3D_PROFILE_LAND_SET(ProfNRefineFaces, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineComputeFaces, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineLeaves, 0);
	NL3D_PROFILE_LAND_SET(ProfNSplits, 0);
	NL3D_PROFILE_LAND_SET(ProfNMerges, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineInTileTransition, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineWithLowDistance, 0);
	NL3D_PROFILE_LAND_SET(ProfNSplitsPass, 0);

	if(!_RefineMode)
		return;

	// Update the priority list.
	// ==========================
	CTessFacePListNode		rootSplitTessFaceToUpdate;
	CTessFacePListNode		rootMergeTessFaceToUpdate;
	if( _MustRefineAllAtNextRefine )
	{
		// ok, first pass done, setup OldRefineCetner
		_MustRefineAllAtNextRefine= false;
		_OldRefineCenter= refineCenter;

		// then shift all faces
		_SplitPriorityList.shiftAll(rootSplitTessFaceToUpdate);
		_MergePriorityList.shiftAll(rootMergeTessFaceToUpdate);
	}
	else
	{
		// else, compute delta between positions
		CVector		diff= refineCenter - _OldRefineCenter;
		_OldRefineCenter= refineCenter;

		// and shift according to distance of deplacement.
		_SplitPriorityList.shift(diff, rootSplitTessFaceToUpdate);
		_MergePriorityList.shift(diff, rootMergeTessFaceToUpdate);
	}


	// Refine Faces which may need it.
	// ==========================
	// Update globals
	updateGlobalsAndLockBuffers (refineCenter);
	// NB: refine may change vertices in VB in visible patchs => buffers are locked.

	// Increment the update date.
	CLandscapeGlobals::CurrentDate++;

	// Because CTessFacePriorityList::insert use it.
	NLMISC::OptFastFloorBegin();

	/* While there is still face in list, update them
		NB: updateRefine() always insert the face in _***PriorityList, so face is removed from
		root***TessFaceToUpdate list.
		NB: it is possible ( with enforced merge() ) that faces dissapears from root***TessFaceToUpdate list
		before they are traversed here. It is why we must use a Circular list system, and not an array of elements.
		Basically. TessFaces are ALWAYS in a list, either in one of the entry list in _***PriorityList, or in
		root***TessFaceToUpdate list.

		It is newTessFace() and deleteTessFace() which insert/remove the nodes in the list.
	*/
	// Update the Merge priority list.
	while( rootMergeTessFaceToUpdate.nextInPList() != &rootMergeTessFaceToUpdate )
	{
		// Get the face.
		CTessFace	*face= static_cast<CTessFace*>(rootMergeTessFaceToUpdate.nextInPList());

		// update the refine of this face. This may lead in deletion (merge) of other faces which are still in
		// root***TessFaceToUpdate, but it's work.
		face->updateRefineMerge();
	}


	// Update the Split priority list.
	do
	{
		NL3D_PROFILE_LAND_ADD(ProfNSplitsPass, 1);

		// Append the new leaves, to the list of triangles to update
		rootSplitTessFaceToUpdate.appendPList(_RootNewLeaves);

		// While triangle to test for split exists
		while( rootSplitTessFaceToUpdate.nextInPList() != &rootSplitTessFaceToUpdate )
		{
			// Get the face.
			CTessFace	*face= static_cast<CTessFace*>(rootSplitTessFaceToUpdate.nextInPList());

			// update the refine of this face.
			face->updateRefineSplit();
		}

	}
	// do it until we are sure no more split is needed, ie no more faces are created
	while( _RootNewLeaves.nextInPList() != &_RootNewLeaves );

	// Because CTessFacePriorityList::insert use it.
	NLMISC::OptFastFloorEnd();


	// Before unlockBuffers, test for vegetable IG creation.
	{
		H_AUTO( NL3D_Vegetable_Update );

		// Because CLandscapeVegetableBlock::update() use OptFastFloor..
		NLMISC::OptFastFloorBegin();

		// For each vegetableBlock, test IG creation
		CLandscapeVegetableBlock	*vegetBlock= _VegetableBlockList.begin();
		for(;vegetBlock!=NULL; vegetBlock= (CLandscapeVegetableBlock*)vegetBlock->Next)
		{
			vegetBlock->update(refineCenter, _VegetableManager);
		}

		// update lighting for vegetables
		_VegetableManager->updateLighting();

		// Stop fastFloor optim.
		NLMISC::OptFastFloorEnd();
	}


	// Must realase VB Buffers
	unlockBuffers();

	// refine() may cause change in faces in visible patchs.
	updateTessBlocksFaceVector();

}


// ***************************************************************************
void			CLandscape::refineAll(const CVector &refineCenter)
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (refineCenter);
	// NB: refineAll may change vertices in VB in visible patchs => buffers are locked.

	// Increment the update date.
	CLandscapeGlobals::CurrentDate++;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->refineAll();
	}

	// Must realase VB Buffers
	unlockBuffers();

	// refineAll() may cause change in faces in visible patchs.
	updateTessBlocksFaceVector();
}


// ***************************************************************************
void			CLandscape::excludePatchFromRefineAll(sint zoneId, uint patch, bool exclude)
{
	ItZoneMap it= Zones.find(uint16(zoneId));
	if(it!=Zones.end())
	{
		it->second->excludePatchFromRefineAll(patch, exclude);
	}

}


// ***************************************************************************
void			CLandscape::averageTesselationVertices()
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (CVector::Null);
	// NB: averageTesselationVertices may change vertices in VB in visible patchs => buffers are locked.

	// Increment the update date.
	CLandscapeGlobals::CurrentDate++;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->averageTesselationVertices();
	}

	// Must realase VB Buffers
	unlockBuffers();

	// averageTesselationVertices() should not cause change in faces in any patchs.
	// It should not happens, but check for security.
	nlassert(_TessBlockModificationRoot.getNextToModify()==NULL);
	updateTessBlocksFaceVector();

}



// ***************************************************************************
void			CLandscape::updateGlobalsAndLockBuffers (const CVector &refineCenter)
{
	// Setup CLandscapeGlobals static members...

	// Far limits.
	CLandscapeGlobals::FarTransition= _FarTransition;

	// Tile subdivsion part.
	CLandscapeGlobals::TileMaxSubdivision= _TileMaxSubdivision;
	CLandscapeGlobals::TileDistNear = _TileDistNear;
	CLandscapeGlobals::TileDistFar = CLandscapeGlobals::TileDistNear+20;
	CLandscapeGlobals::TileDistNearSqr = sqr(CLandscapeGlobals::TileDistNear);
	CLandscapeGlobals::TileDistFarSqr = sqr(CLandscapeGlobals::TileDistFar);
	CLandscapeGlobals::OOTileDistDeltaSqr = 1.0f / (CLandscapeGlobals::TileDistFarSqr - CLandscapeGlobals::TileDistNearSqr);

	// Tile Pixel size part.
	// \todo yoyo: choose according to wanted tile pixel size.
	CLandscapeGlobals::TilePixelSize= 128.0f;
	CLandscapeGlobals::TilePixelBias128= 0.5f/CLandscapeGlobals::TilePixelSize;
	CLandscapeGlobals::TilePixelScale128= 1-1/CLandscapeGlobals::TilePixelSize;
	CLandscapeGlobals::TilePixelBias256= 0.5f/(CLandscapeGlobals::TilePixelSize*2);
	CLandscapeGlobals::TilePixelScale256= 1-1/(CLandscapeGlobals::TilePixelSize*2);

	// RefineThreshold.
	CLandscapeGlobals::RefineThreshold= _Threshold;

	if (_Threshold == 0.0f)
		CLandscapeGlobals::OORefineThreshold = FLT_MAX;
	else
		CLandscapeGlobals::OORefineThreshold = 1.0f / CLandscapeGlobals::RefineThreshold;

	// Refine Center*.
	CLandscapeGlobals::RefineCenter= refineCenter;
	CLandscapeGlobals::TileFarSphere.Center= CLandscapeGlobals::RefineCenter;
	CLandscapeGlobals::TileFarSphere.Radius= CLandscapeGlobals::TileDistFar;
	CLandscapeGlobals::TileNearSphere.Center= CLandscapeGlobals::RefineCenter;
	CLandscapeGlobals::TileNearSphere.Radius= CLandscapeGlobals::TileDistNear;

	// PZBModelPosition
	CLandscapeGlobals::PZBModelPosition= _PZBModelPosition;

	// VB Allocators.
	CLandscapeGlobals::CurrentFar0VBAllocator= &_Far0VB;
	CLandscapeGlobals::CurrentFar1VBAllocator= &_Far1VB;
	CLandscapeGlobals::CurrentTileVBAllocator= &_TileVB;

	if(_Driver)
		lockBuffers ();
}


// ***************************************************************************
void			CLandscape::lockBuffers ()
{
	// Already locked
	if ((_LockCount++) == 0)
	{
		// Must check driver, and create VB infos,locking buffers.
		if(_Driver)
		{
			_Far0VB.updateDriver(_Driver);
			_Far1VB.updateDriver(_Driver);
			_TileVB.updateDriver(_Driver);

			// must do the same for _VegetableManager.
			if(_DriverOkForVegetable)
				_VegetableManager->updateDriver(_Driver);
		}

		_Far0VB.lockBuffer(CLandscapeGlobals::CurrentFar0VBInfo);
		_Far1VB.lockBuffer(CLandscapeGlobals::CurrentFar1VBInfo);
		_TileVB.lockBuffer(CLandscapeGlobals::CurrentTileVBInfo);

		// lock buffer of the vegetable manager.
		_VegetableManager->lockBuffers();

		// VertexProgrma mode???
		CLandscapeGlobals::VertexProgramEnabled= _VertexShaderOk;
	}
}


// ***************************************************************************
void			CLandscape::unlockBuffers(bool force)
{
	// Already locked
	if ((_LockCount == 1) || force)
	{
		_Far0VB.unlockBuffer();
		_Far1VB.unlockBuffer();
		_TileVB.unlockBuffer();

		// unlock buffer of the vegetable manager.
		_VegetableManager->unlockBuffers();
		_LockCount = 0;
	}

	if (_LockCount > 0)
		_LockCount--;
}

// ***************************************************************************
void			CLandscape::updateTessBlocksFaceVector()
{
	// while some tessBlock to update remains.
	CTessBlock	*tb;
	while( (tb=_TessBlockModificationRoot.getNextToModify()) !=NULL )
	{
		// Get the patch which owns this TessBlock.
		CPatch	*patch= tb->getPatch();

		// If this patch is visible, recreate faceVector for his tessBlock.
		patch->recreateTessBlockFaceVector(*tb);

		// remove from list.
		tb->removeFromModifyList();
	}
}




// ***************************************************************************
static inline void	initPassTriArray(CPatchRdrPass &/* pass */, uint32 numIndex)
{

	//uint	numIndices= pass.getMaxRenderedFaces()*3;
	// realloc if necessary
	// We use
	/*if( CLandscapeGlobals::PassTriArray.getNumIndexes() < numIndices )
		CLandscapeGlobals::PassTriArray.setNumIndexes( numIndices );*/
	CLandscapeGlobals::PassTriArray.setFormat(NL_LANDSCAPE_INDEX_FORMAT);
	CLandscapeGlobals::PassTriArray.setNumIndexes(numIndex);
	// reset ptr.
	nlassert (!CLandscapeGlobals::PassTriArray.isLocked());
	CLandscapeGlobals::PassTriArray.lock (CLandscapeGlobals::PassTriArrayIBA);
	NL3D_LandscapeGlobals_PassTriCurPtr= CLandscapeGlobals::PassTriArrayIBA.getPtr();
	NL3D_LandscapeGlobals_PassTriFormat = CLandscapeGlobals::PassTriArrayIBA.getFormat();

}


// ***************************************************************************
static
#ifndef NL_DEBUG
	inline
#endif
void	drawPassTriArray(CMaterial &mat)
{
	nlassert (CLandscapeGlobals::PassTriArray.isLocked());
	CLandscapeGlobals::PassTriArrayIBA.unlock();
	if(NL3D_LandscapeGlobals_PassNTri>0)
	{
		//mat.setZFunc(CMaterial::ZFunc::greaterequal);
		CLandscapeGlobals::PatchCurrentDriver->setupMaterial(mat);
		CLandscapeGlobals::PatchCurrentDriver->activeIndexBuffer(CLandscapeGlobals::PassTriArray);
		CLandscapeGlobals::PatchCurrentDriver->renderSimpleTriangles(0, NL3D_LandscapeGlobals_PassNTri);
		NL3D_LandscapeGlobals_PassNTri= 0;
	}
}


// ***************************************************************************
static inline uint32 countNumWantedIndex(CRdrTileId	*tileToRdr, uint rdrPass)
{
	uint32		numIndex = 0;
	while(tileToRdr)
	{
		if(tileToRdr->TileMaterial->Pass[rdrPass].PatchRdrPass)
		{
			// renderSimpleTriangles() with the material setuped.
			numIndex += *(tileToRdr->TileMaterial->TileFaceVectors[rdrPass]);
		}
		tileToRdr= (CRdrTileId*)tileToRdr->getNext();
	}
	return 3 * numIndex;
}

// ***************************************************************************
static inline uint32 countNumWantedIndexFar0(CPatch	*patch)
{
	uint32		numTri = 0;
	while(patch)
	{
		// renderSimpleTriangles() with the material setuped.
		numTri += patch->countNumTriFar0();
		patch = patch->getNextFar0ToRdr();
	}
	return 3 * numTri;
}

// ***************************************************************************
static inline uint32 countNumWantedIndexFar1(CPatch	*patch)
{
	uint32		numTri = 0;
	while(patch)
	{
		numTri += patch->countNumTriFar1();
		patch = patch ->getNextFar1ToRdr();
	}
	return 3 * numTri;
}



// ***************************************************************************
void			CLandscape::render(const CVector &refineCenter, const CVector &frontVector, const CPlane	pyramid[NL3D_TESSBLOCK_NUM_CLIP_PLANE], bool doTileAddPass)
{
	IDriver *driver= _Driver;
	nlassert(driver);

	// values in Stencil Buffer which match with landscape are replace by 128
	// rest of Stencil is replace by 0.
	CScene *scene = NULL;
	if (OwnerModel)
	{
		scene = OwnerModel->getOwnerScene();
		if(scene->getLandscapePolyDrawingCallback())
		{
			scene->getLandscapePolyDrawingCallback()->beginPolyDrawing();
		}
	}

	// Increment the update date for preRender.
	CLandscapeGlobals::CurrentRenderDate++;


	ItZoneMap	it;
	sint		i;
	ItTileRdrPassSet		itTile;
	ItSPRenderPassVector	itFar;

	// Yoyo: profile.
	NL3D_PROFILE_LAND_SET(ProfNRdrFar0, 0);
	NL3D_PROFILE_LAND_SET(ProfNRdrFar1, 0);
	NL3D_PROFILE_LAND_SET(ProfNPatchRdrFar0, 0);
	NL3D_PROFILE_LAND_SET(ProfNPatchRdrFar1, 0);
	for(i=0;i<NL3D_MAX_TILE_PASS;i++)
	{
		NL3D_PROFILE_LAND_SET(ProfNRdrTile[i], 0);
	}


	// -2. Update globals
	//====================
	updateGlobalsAndLockBuffers (refineCenter);
	// NB: render may change vertices in VB in visible patchs => buffers are locked.


	// -1. clear all PatchRenderPass renderList
	//===================

	H_BEFORE( NL3D_Landscape_Render_Clear );

	// Fars.
	for(itFar= _TextureFars.begin(); itFar!= _TextureFars.end(); itFar++)
	{
		CPatchRdrPass	&pass= **itFar;
		// clear list.
		pass.clearAllRenderList();
	}

	// Tiles.
	for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
	{
		CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);
		// clear list.
		pass.clearAllRenderList();
	}

	// Lightmaps.
	for(sint lightRdrPass=0; lightRdrPass<(sint)_TextureNears.size(); lightRdrPass++)
	{
		CPatchRdrPass	&pass= *_TextureNears[lightRdrPass];
		// clear list.
		pass.clearAllRenderList();
	}

	H_AFTER( NL3D_Landscape_Render_Clear );

	// 0. preRender pass.
	//===================

	H_BEFORE( NL3D_Landscape_Render_PreRender );

	// change Far0 / Far1.
	// Clip TessBlocks against pyramid and Far Limit.
	for(i=0; i<NL3D_TESSBLOCK_NUM_CLIP_PLANE; i++)
	{
		CTessBlock::CurrentPyramid[i]= pyramid[i];
	}
	// Update VB with change of Far0 / Far1.
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->preRender();
	}

	H_AFTER( NL3D_Landscape_Render_PreRender );
	H_BEFORE( NL3D_Landscape_Render_Refill );

	// Check if the vertex buffers must be refilled
	_Far0VB.checkVertexBuffersResident();
	_Far1VB.checkVertexBuffersResident();
	_TileVB.checkVertexBuffersResident();

	// Reallocation Mgt. If any of the VB is reallocated, we must refill it entirely.
	// NB: all VBs are refilled entirely. It is not optimal (maybe 3* too slow), but reallocation are supposed
	// to be very rare.
	if( _Far0VB.reallocationOccurs() || _Far1VB.reallocationOccurs() || _TileVB.reallocationOccurs() )
		_RenderMustRefillVB= true;

	// VertexProgram dependency on RefineThreshold Management. If VertexShader, and if the refineThreshold has
	// changed since the last time, we must refill All the VB, because data are out of date.
	if( _VertexShaderOk && _VPThresholdChange )
	{
		_VPThresholdChange= false;
		_RenderMustRefillVB= true;
	}

	// If we must refill the VB (for any reason).
	if(_RenderMustRefillVB )
	{
		// Ok, ok, we refill All the VB with good data.
		_RenderMustRefillVB= false;

		// First reset the flag, so fillVB() will effectively fill the VB.
		_Far0VB.resetReallocation();
		_Far1VB.resetReallocation();
		_TileVB.resetReallocation();

		// Then recompute good VBInfo (those in CurrentVBInfo are false!!).
		// Do it by unlocking then re-locking Buffers.
		unlockBuffers(true);
		lockBuffers();

		// Finally, fill the VB for all patchs visible.
		for(it= Zones.begin();it!=Zones.end();it++)
		{
			if((*it).second->ClipResult==CZone::ClipOut)
				continue;
			for(sint i=0;i<(*it).second->getNumPatchs(); i++)
			{
				CPatch	*patch= (*it).second->getPatch(i);
				patch->fillVBIfVisible();
			}
		}
	}

	H_AFTER( NL3D_Landscape_Render_Refill );
	H_BEFORE( NL3D_Landscape_Render_SoftGeomorph );

	// If software GeoMorph / Alpha Transition (no VertexShader), do it now.
	if(!_VertexShaderOk)
	{
		// For all patch visible, compute geomoprh and alpha in software.
		for(it= Zones.begin();it!=Zones.end();it++)
		{
			if((*it).second->ClipResult==CZone::ClipOut)
				continue;
			for(sint i=0;i<(*it).second->getNumPatchs(); i++)
			{
				CPatch	*patch= (*it).second->getPatch(i);
				// If visible, compute Geomorph And Alpha
				patch->computeSoftwareGeomorphAndAlpha();
			}
		}

		/*
			Optim note: here, lot of vertices are
				1/ geomorphed twice (vertices on edges of patchs)
				2/ vertices are geomorphed, but not used (because o the Tessblock clip),
					because lot of vertices used by faces in small TessBlocks are still in MasterBlock.

			Some tries have been made to solve this, but result are even worse (2 times or more), because:
				1/
					- does not really matter edges of patchs (and corner) because the majority is in interior of patch.
					- in this case, we need to reset all the flags which is very costly (reparse all zones...) .
				2/ Except for the old CTessBlockEdge management which not solve all the thing, the other solution is
					to test all faces not clipped (on a per TessBlock basis), to compute only vertices needed.
					But in this cases, result are worse, maybe because there is 6 times more tests, and with bad BTB cache.
		*/
	}

	H_AFTER( NL3D_Landscape_Render_SoftGeomorph );

	// Must realase VB Buffers Now!! The VBuffers are now OK!
	// NB: no parallelism is made between 3dCard and Fill of vertices.
	// We Suppose Fill of vertices is rare, and so do not need to be parallelized.
	unlockBuffers(true);


	// If VertexShader enabled, setup VertexProgram Constants.
	if (_VertexShaderOk)
	{
		bool uprogstate = driver->isUniformProgramState();
		uint nbvp = uprogstate ? CLandscapeVBAllocator::MaxVertexProgram : 1;
		for (uint i = 0; i < nbvp; ++i)
		{
			// activate the program to set the uniforms in the program state for all programs
			// note: when uniforms are driver state, the indices must be the same across programs
			if (uprogstate) _TileVB.activateVP(i);
			CVertexProgramLandscape *program = _TileVB.getVP(i);
			// c[0..3] take the ModelViewProjection Matrix.
			driver->setUniformMatrix(IDriver::VertexProgram, program->getUniformIndex(CGPUProgramIndex::ModelViewProjection), IDriver::ModelViewProjection, IDriver::Identity);
			// c[4] take useful constants.
			driver->setUniform4f(IDriver::VertexProgram, program->idx().ProgramConstants0, 0, 1, 0.5f, 0);
			// c[5] take RefineCenter
			driver->setuniform3f(IDriver::VertexProgram, program->idx().RefineCenter, refineCenter);
			// c[6] take info for Geomorph trnasition to TileNear.
			driver->setUniform2f(IDriver::VertexProgram, program->idx().TileDist, CLandscapeGlobals::TileDistFarSqr, CLandscapeGlobals::OOTileDistDeltaSqr);
			// c[10] take the fog vector.
			driver->setUniformFog(IDriver::VertexProgram, program->getUniformIndex(CGPUProgramIndex::Fog));
			// c[12] take the current landscape Center / delta Pos to apply
			driver->setUniform3f(IDriver::VertexProgram, program->idx().PZBModelPosition, _PZBModelPosition);
		}
	}


	// 1. TileRender pass.
	//====================

	// Yoyo: profile
	NL3D_PROFILE_LAND_SET(ProfNTileSetupMaterial, driver->profileSetupedMaterials() );
	H_BEFORE( NL3D_Landscape_Render_Tile );

	// First, update Dynamic Lighting for Near, ie multiply Dynamic Lightmap with UserColor, and upload to texture.
	// ==================
	CPatchDLMContext	*dlmCtxPtr= _PatchDLMContextList->begin();
	while(dlmCtxPtr!=NULL)
	{
		// do it only if the patch has some Near stuff to render, and if it is visible
		if(dlmCtxPtr->getPatch()->getFar0() == 0
			 && !dlmCtxPtr->getPatch()->isRenderClipped() )
		{
			// upload lightmap into textureDLM, modulating before with patch TileColor.
			// NB: no-op if both src and dst are already full black.
			dlmCtxPtr->compileLighting(CPatchDLMContext::ModulateTileColor);
		}

		// next
		dlmCtxPtr= (CPatchDLMContext*)dlmCtxPtr->Next;
	}


	// Active VB.
	// ==================

	// Active the good VB, and maybe activate the VertexProgram Nb 0.
	_TileVB.activate(0);


	// Render.
	// ==================
	// Before any render call. Set the global driver used to render.
	CLandscapeGlobals::PatchCurrentDriver= driver;

	// bkup the original fog color
	CRGBA	fogColor= driver->getFogColor();

	// Render Order. Must "invert", since initial order is NOT the render order. This is done because the lightmap pass
	// DO NOT have to do any renderTile(), since it is computed in RGB0 pass.
	nlassert(NL3D_MAX_TILE_PASS==5);
	static	sint	RenderOrder[NL3D_MAX_TILE_PASS]= {NL3D_TILE_PASS_RGB0, NL3D_TILE_PASS_RGB1, NL3D_TILE_PASS_RGB2,
		NL3D_TILE_PASS_LIGHTMAP, NL3D_TILE_PASS_ADD};
	// For ALL pass..
	for(i=0; i<NL3D_MAX_TILE_PASS; i++)
	{
		sint	passOrder= RenderOrder[i];


		// If VertexShader enabled, and if lightmap or post Add pass, must setup good VertexProgram
		if(_VertexShaderOk)
		{
			if(passOrder == NL3D_TILE_PASS_LIGHTMAP)
			{
				// Must activate the vertexProgram to take TexCoord2 to stage0
				_TileVB.activate(1);
			}
			else if(passOrder == NL3D_TILE_PASS_ADD)
			{
				// Must re-activate the standard VertexProgram
				_TileVB.activate(0);
			}
		}


		// Do add pass???
		if((passOrder==NL3D_TILE_PASS_ADD) && !doTileAddPass)
			continue;


		// Setup common material for this pass.
		//=============================
		// Default: Replace envmode.
		TileMaterial.texEnvOpRGB(0, CMaterial::Replace);
		TileMaterial.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		TileMaterial.texEnvOpAlpha(0, CMaterial::Replace);
		TileMaterial.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);
		// NB: important to set Replace and not modulate, because in case of VerexProgram enabled,
		// Diffuse o[COL0] is undefined.

		// Copy from stage 0 to stage 1.
		TileMaterial.setTexEnvMode(1, TileMaterial.getTexEnvMode(0));

		// setup multitex / blending.
		if(passOrder==NL3D_TILE_PASS_RGB0)
		{
			// first pass, no blend.
			TileMaterial.setBlend(false);
		}
		else
		{
			TileMaterial.setBlend(true);
			switch(passOrder)
			{
				case NL3D_TILE_PASS_RGB1:
				case NL3D_TILE_PASS_RGB2:
					// alpha blending.
					TileMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);

					// Must use a special envmode for stage1: "separateAlpha"!!.
					// keep the color from previous stage.
					TileMaterial.texEnvOpRGB(1, CMaterial::Replace);
					TileMaterial.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor);
					// take the alpha from current stage.
					TileMaterial.texEnvOpAlpha(1, CMaterial::Replace);
					TileMaterial.texEnvArg0Alpha(1, CMaterial::Texture, CMaterial::SrcAlpha);
					break;
				case NL3D_TILE_PASS_LIGHTMAP:
					// modulate alpha blending.
					TileMaterial.setBlendFunc(CMaterial::zero, CMaterial::srccolor);

					// Setup the material envCombine so DynamicLightmap (stage 0) is added to static lightmap.
					TileMaterial.texEnvOpRGB(1, CMaterial::Add);
					TileMaterial.texEnvArg0RGB(1, CMaterial::Texture, CMaterial::SrcColor);
					TileMaterial.texEnvArg1RGB(1, CMaterial::Previous, CMaterial::SrcColor);

					break;
				case NL3D_TILE_PASS_ADD:
					// Use srcalpha for src (and not ONE), since additive are blended with alpha Cte/AlphaTexture
					TileMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::one);

					// for MAYBE LATER smooth night transition, setup Alpha cte of stage0, and setup alpha stage.
					TileMaterial.texEnvOpAlpha(0, CMaterial::Replace);
					TileMaterial.texEnvArg0Alpha(0, CMaterial::Constant, CMaterial::SrcAlpha);
					// Temp, just setup alpha to 1.
					TileMaterial.texConstantColor(0, CRGBA(255, 255, 255, 255));

					// Must use a special envmode for stage1: "separateAlpha"!!.
					// NB: it still works if The RdrPass has no texture.
					// keep the color from previous stage.
					TileMaterial.texEnvOpRGB(1, CMaterial::Replace);
					TileMaterial.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor);
					// modulate the alpha of current stage with previous
					TileMaterial.texEnvOpAlpha(1, CMaterial::Modulate);
					TileMaterial.texEnvArg0Alpha(1, CMaterial::Texture, CMaterial::SrcAlpha);
					TileMaterial.texEnvArg1Alpha(1, CMaterial::Previous, CMaterial::SrcAlpha);

					break;
				default:
					nlstop;
			};
		}
		// Reset the textures (so there is none in Addtive pass or in Lightmap).
		TileMaterial.setTexture(0, NULL);
		TileMaterial.setTexture(1, NULL);
		TileMaterial.setTexture(2, NULL);


		// Render All material RdrPass.
		//=============================
		// Special code for Lightmap and RGB0, for faster render.
		if(passOrder==NL3D_TILE_PASS_RGB0)
		{
			// for RGB0 pass, setup the normal fogColor
			driver->setupFog(driver->getFogStart(), driver->getFogEnd(), fogColor);

			// RGB0 pass.
			ItTileRdrPassSet	itTile;
			for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
			{
				// Get a ref on the render pass. Const cast work because we only modify attribut from CPatchRdrPass
				// that don't affect the operator< of this class
				CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);
				// Enlarge PassTriArray as needed
				CRdrTileId	*tileToRdr= pass.getRdrTileRoot(passOrder);
				uint32 numWantedIndex =  countNumWantedIndex(tileToRdr, NL3D_TILE_PASS_RGB0);
				if (numWantedIndex)
				{
					initPassTriArray(pass, numWantedIndex);
					// Setup Diffuse texture of the tile.
					TileMaterial.setTexture(0, pass.TextureDiffuse);
					// Add triangles to array
					while(tileToRdr)
					{
						// renderSimpleTriangles() with the material setuped.
						tileToRdr->TileMaterial->renderTilePassRGB0();
						tileToRdr= (CRdrTileId*)tileToRdr->getNext();
					}

					// Render triangles.
					drawPassTriArray(TileMaterial);
				}
			}
		}
		else if(passOrder==NL3D_TILE_PASS_LIGHTMAP)
		{
			// for Lightmap pass (modulate blend), setup a White fogColor. This is not
			// mathematically correct but works fine
			driver->setupFog(driver->getFogStart(), driver->getFogEnd(), CRGBA::White);

			// Lightmap Pass.
			/* \todo yoyo: TODO_CLOUD: setup stage2, and setup texcoord generation. COMPLEX because of interaction
			 with Dynamic LightMap
			*/

			// Setup the Dynamic Lightmap into stage 0.
			TileMaterial.setTexture(0, _TextureDLM);

			// if vertex shader not used.
			if(!_VertexShaderOk)
			{
				// special setup  such that stage0 takes Uv2.
				driver->mapTextureStageToUV(0, 2);
			}
			// else VertexProgram map itself the good vertex Attribute to UV0.


			// Render All the lightmaps.
			for(sint lightRdrPass=0; lightRdrPass<(sint)_TextureNears.size(); lightRdrPass++)
			{
				CPatchRdrPass	&pass= *_TextureNears[lightRdrPass];

				// Enlarge PassTriArray as needed
				CRdrTileId		*tileToRdr= pass.getRdrTileRoot(passOrder);
				uint32 numWantedIndex = countNumWantedIndex(tileToRdr, NL3D_TILE_PASS_RGB0);
				if (numWantedIndex)
				{
					initPassTriArray(pass, numWantedIndex);
					// Setup Lightmap into stage1. Because we share UV with pass RGB0. So we use UV1.
					// Also, now stage0 is used for DynamicLightmap
					TileMaterial.setTexture(1, pass.TextureDiffuse);
					// Add triangles to array
					while(tileToRdr)
					{
						// renderSimpleTriangles() with the material setuped.
						tileToRdr->TileMaterial->renderTilePassLightmap();
						tileToRdr= (CRdrTileId*)tileToRdr->getNext();
					}
					// Render triangles.
					drawPassTriArray(TileMaterial);
				}
			}

			// if vertex shader not used.
			if(!_VertexShaderOk)
			{
				// Reset special stage/UV setup to normal behavior
				driver->mapTextureStageToUV(0, 0);
			}
		}
		else
		{
			// RGB1, RGB2, and ADD pass.
			// for ADD pass (additive blend), setup a Black fogColor
			if(passOrder==NL3D_TILE_PASS_ADD)
				driver->setupFog(driver->getFogStart(), driver->getFogEnd(), CRGBA::Black);
			// else setup the standard color (std blend)
			else
				driver->setupFog(driver->getFogStart(), driver->getFogEnd(), fogColor);

			// Render Base, Transitions or Additives.
			ItTileRdrPassSet	itTile;
			for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
			{
				// Get a ref on the render pass. Const cast work because we only modify attribut from CPatchRdrPass
				// that don't affect the operator< of this class
				CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);

				// Enlarge PassTriArray as needed
				CRdrTileId		*tileToRdr= pass.getRdrTileRoot(passOrder);
				uint32 numWantedIndex = countNumWantedIndex(tileToRdr, passOrder);
				if (numWantedIndex)
				{
					initPassTriArray(pass, numWantedIndex);

					// Add triangles to array
					while(tileToRdr)
					{
						// renderSimpleTriangles() with the material setuped.
						tileToRdr->TileMaterial->renderTile(passOrder);
						tileToRdr= (CRdrTileId*)tileToRdr->getNext();
					}

					// Pass not empty ?
					if(NL3D_LandscapeGlobals_PassNTri>0)
					{
						// Setup material.
						// Setup Diffuse texture of the tile.
						TileMaterial.setTexture(0, pass.TextureDiffuse);

						// If transition tile, must enable the alpha for this pass.
						// NB: Additive pass may have pass.TextureAlpha==NULL
						TileMaterial.setTexture(1, pass.TextureAlpha);
					}

					// Render triangles.
					drawPassTriArray(TileMaterial);
				}
			}
		}
	}

	// restore old fog color
	driver->setupFog(driver->getFogStart(), driver->getFogEnd(), fogColor);

	// Yoyo: profile
	NL3D_PROFILE_LAND_SET(ProfNTileSetupMaterial, driver->profileSetupedMaterials()-ProfNTileSetupMaterial );
	H_AFTER( NL3D_Landscape_Render_Tile );

	// 2. Far0Render pass.
	//====================

	// Yoyo: profile
	NL3D_PROFILE_LAND_SET(ProfNFar0SetupMaterial, driver->profileSetupedMaterials() );
	H_BEFORE( NL3D_Landscape_Render_DLM );

	// First, update Dynamic Lighting for Far, ie multiply Dynamic Lightmap with TextureFar, and upload to texture.
	// ==================
	dlmCtxPtr= _PatchDLMContextList->begin();
	while(dlmCtxPtr!=NULL)
	{
		// do it only if the patch has some Far stuff to render, and if it is visible
		if( (dlmCtxPtr->getPatch()->getFar0()>0 || dlmCtxPtr->getPatch()->getFar1()>0)
			 && !dlmCtxPtr->getPatch()->isRenderClipped() )
		{
			// upload lightmap into textureDLM, modulating before with patch TextureFar.
			// NB: no-op if both src and dst are already full black.
			dlmCtxPtr->compileLighting(CPatchDLMContext::ModulateTextureFar);
		}

		// next
		dlmCtxPtr= (CPatchDLMContext*)dlmCtxPtr->Next;
	}

	H_AFTER( NL3D_Landscape_Render_DLM );
	H_BEFORE( NL3D_Landscape_Render_Far0 );

	// Active VB.
	// ==================

	// Active the good VB, and maybe activate the std VertexProgram.
	_Far0VB.activate(0);


	// Render.
	// ==================

	// Setup common material.
	FarMaterial.setBlend(false);
	// set the DLM texture.
	FarMaterial.setTexture(1, _TextureDLM);

	// Render All material RdrPass0.
	itFar=_TextureFars.begin();
	while (itFar!=_TextureFars.end())
	{
		CPatchRdrPass	&pass= **itFar;

		// Enlarge PassTriArray as needed
		CPatch		*patchToRdr= pass.getRdrPatchFar0();
		if (patchToRdr)
		{
			uint32 numWantedIndex = countNumWantedIndexFar0(patchToRdr);
			if (numWantedIndex)
			{
				initPassTriArray(pass, numWantedIndex);
				// Setup the material.
				FarMaterial.setTexture(0, pass.TextureDiffuse);
				// If the texture need to be updated, do it now.
				if(pass.TextureDiffuse && pass.TextureDiffuse->touched())
					driver->setupTexture(*pass.TextureDiffuse);
				// Add triangles to array
				while(patchToRdr)
				{
					// renderSimpleTriangles() with the material setuped.
					patchToRdr->renderFar0();
					patchToRdr= patchToRdr->getNextFar0ToRdr();
				}
				// Render triangles.
				drawPassTriArray(FarMaterial);
			}
		}
		// Next render pass
		itFar++;
	}

	// Yoyo: profile
	NL3D_PROFILE_LAND_SET(ProfNFar0SetupMaterial, driver->profileSetupedMaterials()-ProfNFar0SetupMaterial );
	H_AFTER( NL3D_Landscape_Render_Far0 );


	// 3. Far1Render pass.
	//====================

	// Yoyo: profile
	NL3D_PROFILE_LAND_SET(ProfNFar1SetupMaterial, driver->profileSetupedMaterials() );
	H_BEFORE( NL3D_Landscape_Render_Far1 );

	// Active VB.
	// ==================

	// Active the good VB, and maybe activate the std VertexProgram.
	_Far1VB.activate(0);


	// Render
	// ==================

	// Setup common material.
	FarMaterial.setBlend(true);
	// set the DLM texture.
	FarMaterial.setTexture(1, _TextureDLM);


	// Render All material RdrPass1.
	itFar=_TextureFars.begin();
	while (itFar!=_TextureFars.end())
	{
		CPatchRdrPass	&pass= **itFar;

		// Enlarge PassTriArray as needed
		CPatch		*patchToRdr= pass.getRdrPatchFar1();
		if (patchToRdr)
		{
			//uint32 numWantedIndex = countNumWantedIndexFar1(patchToRdr);
			uint32 numWantedIndex = countNumWantedIndexFar1(patchToRdr);
			if (numWantedIndex)
			{
				initPassTriArray(pass, numWantedIndex);

				// Setup the material.
				FarMaterial.setTexture(0, pass.TextureDiffuse);
				// If the texture need to be updated, do it now.
				if(pass.TextureDiffuse && pass.TextureDiffuse->touched())
					driver->setupTexture(*pass.TextureDiffuse);

				// Add triangles to array
				while(patchToRdr)
				{
					// renderSimpleTriangles() with the material setuped.
					patchToRdr->renderFar1();
					patchToRdr= patchToRdr->getNextFar1ToRdr();
				}

				// Render triangles.
				drawPassTriArray(FarMaterial);
			}
		}
		// Next render pass
		itFar++;
	}

	// Yoyo: profile
	NL3D_PROFILE_LAND_SET(ProfNFar1SetupMaterial, driver->profileSetupedMaterials()-ProfNFar1SetupMaterial );
	H_AFTER( NL3D_Landscape_Render_Far1 );


	// 4. "Release" texture materials.
	//================================
	FarMaterial.setTexture(0, NULL);
	FarMaterial.setTexture(1, NULL);
	FarMaterial.setTexture(2, NULL);
	FarMaterial.setTexture(3, NULL);
	TileMaterial.setTexture(0, NULL);
	TileMaterial.setTexture(1, NULL);
	TileMaterial.setTexture(2, NULL);
	TileMaterial.setTexture(3, NULL);

	// To ensure no use but in render()..
	CLandscapeGlobals::PatchCurrentDriver= NULL;

	// Desactive the vertex program (if anyone)
	if(_VertexShaderOk)
		driver->activeVertexProgram (NULL);


	// 5. Vegetable Management.
	//================================
	if(scene && scene->getLandscapePolyDrawingCallback())
	{
		scene->getLandscapePolyDrawingCallback()->endPolyDrawing();
	}

	// First, update Dynamic Lighting for Vegetable, ie just copy.
	// ==================
	if(isVegetableActive())
	{
		/* Actually we modulate the DLM with an arbitrary constant for this reason:
			Color of vegetable (ie their material) are NOT modulated with DLM.
			Doing this without using PixelShader / additional UVs seems to be impossible.
			And add new UVs (+700K in AGP) just for this is not worth the effort.

			We prefer using a constant to simulate the "global vegetable color", which is a big trick.

			Additionally, the vegetable take the diffuse lighting of landscape, which is
			false because it replaces the diffuse lighting it should have (ie with his own Normal and
			his own "global vegetable color")

			We can't do anything for "correct normal vegetable", but it is possible to replace landscape
			material with vegetable material, by dividing _DLMGlobalVegetableColor by LandscapeDiffuseMaterial.
			This is a very approximate result because of CRGBA clamp, but it is acceptable.
		*/
		CRGBA	vegetDLMCte;
		// the constant is _DLMGlobalVegetableColor / PointLightDiffuseMaterial
		uint	v;
		v= (_DLMGlobalVegetableColor.R*256) / (_PointLightDiffuseMaterial.R+1);
		vegetDLMCte.R= (uint8)min(v, 255U);
		v= (_DLMGlobalVegetableColor.G*256) / (_PointLightDiffuseMaterial.G+1);
		vegetDLMCte.G= (uint8)min(v, 255U);
		v= (_DLMGlobalVegetableColor.B*256) / (_PointLightDiffuseMaterial.B+1);
		vegetDLMCte.B= (uint8)min(v, 255U);

		// Parse all patch which have some vegetables
		dlmCtxPtr= _PatchDLMContextList->begin();
		while(dlmCtxPtr!=NULL)
		{
			// do it only if the patch has some vegetable stuff to render, and if it is visible
			// NB: we may have some vegetable stuff to render if the patch has some TileMaterial created.
			if(dlmCtxPtr->getPatch()->getTileMaterialRefCount()>0
				 && !dlmCtxPtr->getPatch()->isRenderClipped() )
			{
				// NB: no-op if both src and dst are already full black.
				dlmCtxPtr->compileLighting(CPatchDLMContext::ModulateConstant, vegetDLMCte);
			}

			// next
			dlmCtxPtr= (CPatchDLMContext*)dlmCtxPtr->Next;
		}
	}


	// profile.
	_VegetableManager->resetNumVegetableFaceRendered();

	// render all vegetables, only if driver support VertexProgram.
	// ==================
	if(isVegetableActive())
	{
		// Use same plane as TessBlock for faster clipping.
		vector<CPlane>		vegetablePyramid;
		vegetablePyramid.resize(NL3D_TESSBLOCK_NUM_CLIP_PLANE);
		for(i=0;i<NL3D_TESSBLOCK_NUM_CLIP_PLANE;i++)
		{
			vegetablePyramid[i]= pyramid[i];
		}
		_VegetableManager->render(refineCenter, frontVector, vegetablePyramid, _TextureDLM, driver);
	}
}


// ***************************************************************************
// ***************************************************************************
// Tile mgt.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
ITexture		*CLandscape::findTileTexture(const std::string &textName, bool clamp)
{
	ITexture	*text;
	text= TileTextureMap[textName];
	// If just inserted, RefPtr is NULL!!  :)
	// This test too if the RefPtr is NULL... (tile released)
	// The object is not owned by this map. It will be own by the multiple RdrPass via CSmartPtr.
	// They will destroy it when no more points to them.
	if(!text)
	{
		TileTextureMap[textName]= text= new CTextureFile(textName);
		text->setWrapS(clamp?ITexture::Clamp:ITexture::Repeat);
		text->setWrapT(clamp?ITexture::Clamp:ITexture::Repeat);
		text->setUploadFormat(ITexture::DXTC5);
		text->setTextureCategory(_TextureTileCategory);
	}
	return text;
}


// ***************************************************************************
CPatchRdrPass	*CLandscape::findTileRdrPass(const CPatchRdrPass &pass)
{
	ItTileRdrPassSet	it;
	// If already here, find it, else insert.
	it= (TileRdrPassSet.insert(pass)).first;

	return const_cast<CPatchRdrPass*>(&(*it));
}


// ***************************************************************************
void			CLandscape::loadTile(uint16 tileId)
{
	CTile		*tile;
	CTileInfo	*tileInfo;
	string		textName;

	// Retrieve or create texture.
	// ===========================
	// Tile Must exist.
	// nlassert(tileId==0xFFFF || tileId<TileBank.getTileCount());
	if(tileId<TileBank.getTileCount())
		tile= TileBank.getTile(tileId);
	else
		tile= NULL;
	// TileInfo must not exist.
	nlassert(TileInfos[tileId]==NULL);
	TileInfos[tileId]= tileInfo= new CTileInfo;

	// Fill additive part.
	// ===================
	if(tile)
		textName= tile->getRelativeFileName(CTile::additive);
	else
		textName= "";
	// If no additive for this tile, rdrpass is NULL.
	if(textName=="")
		tileInfo->AdditiveRdrPass= NULL;
	else
	{
		// Fill rdrpass.
		CPatchRdrPass	pass;
		// Avoid using Clamp for diffuse, because of recent NVidia GL drivers Bugs in 77.72
		pass.TextureDiffuse= findTileTexture(TileBank.getAbsPath()+textName, false);

		// We may have an alpha part for additive.
		textName= tile->getRelativeFileName (CTile::alpha);
		if(textName!="")
			// Must Use clamp for alpha (although NVidia drivers are buggy), because the texture doesn't tile at all
			pass.TextureAlpha= findTileTexture(TileBank.getAbsPath()+textName, true);

		// Fill tileInfo.
		tileInfo->AdditiveRdrPass= findTileRdrPass(pass);
		// Fill UV Info.
		// NB: for now, One Tile== One Texture, so UVScaleBias is simple.
		tileInfo->AdditiveUvScaleBias.x= 0;
		tileInfo->AdditiveUvScaleBias.y= 0;
		tileInfo->AdditiveUvScaleBias.z= 1;
	}


	// Fill diffuse part.
	// =======================
	// Fill rdrpass.
	CPatchRdrPass	pass;
	// The diffuse part for a tile is inevitable.
	if(tile)
	{
		textName= tile->getRelativeFileName(CTile::diffuse);
		if(textName!="")
			// Avoid using Clamp for diffuse, because of recent NVidia GL drivers Bugs in 77.72
			pass.TextureDiffuse= findTileTexture(TileBank.getAbsPath()+textName, false);
		else
		{
			pass.TextureDiffuse= new CTextureCross;
			nldebug("Missing Tile diffuse texname: %d", tileId);
		}
	}
	else
		pass.TextureDiffuse= new CTextureCross;
	if(tile)
	{
		textName= tile->getRelativeFileName (CTile::alpha);
		if(textName!="")
			// Must Use clamp for alpha (although NVidia drivers are buggy), because the texture doesn't tile at all
			pass.TextureAlpha= findTileTexture(TileBank.getAbsPath()+textName, true);
	}


	// Fill tileInfo.
	tileInfo->DiffuseRdrPass= findTileRdrPass(pass);
	// Fill UV Info.
	// NB: for now, One Tile== One Texture, so UVScaleBias is simple.
	tileInfo->DiffuseUvScaleBias.x= 0;
	tileInfo->DiffuseUvScaleBias.y= 0;
	tileInfo->DiffuseUvScaleBias.z= 1;
	tileInfo->AlphaUvScaleBias.x= 0;
	tileInfo->AlphaUvScaleBias.y= 0;
	tileInfo->AlphaUvScaleBias.z= 1;
	// Retrieve the good rot alpha decal.
	if(tile)
		tileInfo->RotAlpha= tile->getRotAlpha();
	else
		tileInfo->RotAlpha= 0;


	// Increment RefCount of RenderPart.
	// =================================
	if(tileInfo->AdditiveRdrPass)
		tileInfo->AdditiveRdrPass->RefCount++;
	if(tileInfo->DiffuseRdrPass)
		tileInfo->DiffuseRdrPass->RefCount++;

}


// ***************************************************************************
void			CLandscape::releaseTile(uint16 tileId)
{
	CTileInfo	*tileInfo;
	tileInfo= TileInfos[tileId];
	nlassert(tileInfo!=NULL);

	// "Release" the rdr pass.
	if(tileInfo->AdditiveRdrPass)
		tileInfo->AdditiveRdrPass->RefCount--;
	if(tileInfo->DiffuseRdrPass)
		tileInfo->DiffuseRdrPass->RefCount--;

	delete tileInfo;
	TileInfos[tileId]= NULL;
}


// ***************************************************************************
CPatchRdrPass	*CLandscape::getTileRenderPass(uint16 tileId, bool additiveRdrPass)
{
	CTileInfo	*tile= TileInfos[tileId];

	// If not here, create it.
	//========================
	if(tile==NULL)
	{
		// Force loading of tile.
		loadTile(tileId);

		tile= TileInfos[tileId];
		nlassert(tile!=NULL);
	}

	// Retrieve.
	//========================
	if(additiveRdrPass)
	{
		// NB: additive pass is not lighted by the lightmap, so there is no lighted version of this rednerpass.
		return tile->AdditiveRdrPass;
	}
	else
	{
		return tile->DiffuseRdrPass;
	}
}


// ***************************************************************************
void			CLandscape::getTileUvScaleBiasRot(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias, uint8 &rotAlpha)
{
	CTileInfo	*tile= TileInfos[tileId];
	// tile should not be NULL.
	// Because load of tiles are always done in getTileRenderPass(), and this insertion always succeed.
	nlassert(tile);

	rotAlpha= 0;
	switch(bitmapType)
	{
		case CTile::diffuse:
			uvScaleBias= tile->DiffuseUvScaleBias; break;
		case CTile::additive:
			uvScaleBias= tile->AdditiveUvScaleBias; break;
		case CTile::alpha:
			uvScaleBias= tile->AlphaUvScaleBias;
			rotAlpha= tile->RotAlpha;
			break;
		default: break;
	}
}


// ***************************************************************************
NLMISC::CSmartPtr<ITexture>		CLandscape::getTileTexture(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias)
{
	CPatchRdrPass	*pass;
	if(bitmapType== CTile::additive)
		pass= getTileRenderPass(tileId, true);
	else
		pass= getTileRenderPass(tileId, false);
	if(!pass)
		return NULL;
	uint8	dummy;
	getTileUvScaleBiasRot(tileId, bitmapType, uvScaleBias, dummy);

	// return the wanted texture.
	if(bitmapType==CTile::diffuse || bitmapType==CTile::additive)
		return pass->TextureDiffuse;
	else
		return pass->TextureAlpha;
}


// ***************************************************************************
CTileElement *CLandscape::getTileElement(const CPatchIdent &patchId, const CUV &uv)
{
	// \todo yoyo: TODO_ZONEID: change ZoneId in 32 bits...
	std::map<uint16, CZone*>::const_iterator	it= Zones.find((uint16)patchId.ZoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId.PatchId<N);
		CPatch	*pa= const_cast<CZone*>((*it).second)->getPatch(patchId.PatchId);
		return pa->getTileElement (uv);
	}
	else
		// Return not found
		return NULL;
}


// ***************************************************************************
void			CLandscape::flushTiles(IDriver *drv, uint32 tileStart, uint32 nbTiles)
{
	nlassert(nbTiles<=65536);
	nlassert(tileStart+nbTiles<=65536);

	// Load tile rdrpass, force setup the texture.
	for(sint tileId= tileStart; tileId<(sint)(tileStart+nbTiles); tileId++)
	{
		CTileInfo	*tile= TileInfos[tileId];
		if(tile==NULL)
		{
			loadTile(uint16(tileId));
			CTileInfo	*tile= TileInfos[tileId];
			nlassert(tile);
			if(tile->DiffuseRdrPass)
			{
				const CPatchRdrPass	&pass= *tile->DiffuseRdrPass;
				// If present and not already setuped...
				if(pass.TextureDiffuse && !pass.TextureDiffuse->setupedIntoDriver())
					drv->setupTexture(*pass.TextureDiffuse);
				// If present and not already setuped...
				if(pass.TextureAlpha && !pass.TextureAlpha->setupedIntoDriver())
					drv->setupTexture(*pass.TextureAlpha);
			}
			if(tile->AdditiveRdrPass)
			{
				const CPatchRdrPass	&pass= *tile->AdditiveRdrPass;
				// If present and not already setuped...
				if(pass.TextureDiffuse && !pass.TextureDiffuse->setupedIntoDriver())
					drv->setupTexture(*pass.TextureDiffuse);
				// If present and not already setuped...
				if(pass.TextureAlpha && !pass.TextureAlpha->setupedIntoDriver())
					drv->setupTexture(*pass.TextureAlpha);
			}
		}
	}
}


// ***************************************************************************
void			CLandscape::releaseTiles(uint32 tileStart, uint32 nbTiles)
{
	nlassert(nbTiles<=65536);
	nlassert(tileStart+nbTiles<=65536);

	// release tiles.
	for(sint tileId= tileStart; tileId<(sint)(tileStart+nbTiles); tileId++)
	{
		CTileInfo	*tile= TileInfos[tileId];
		if(tile!=NULL)
		{
			releaseTile(uint16(tileId));
		}
	}

	// For all rdrpass, release one that are no more referenced.
	ItTileRdrPassSet	it;
	for(it= TileRdrPassSet.begin(); it!=TileRdrPassSet.end();)
	{
		// If no more tile access the rdrpass, delete it.
		if((*it).RefCount==0)
		{
			ItTileRdrPassSet itDel=it++;
			TileRdrPassSet.erase(itDel);
		}
		else
			it++;
	}

	// Textures are automaticly deleted by smartptr, but not their entry int the map (TileTextureMap).
	// => doesn't matter since findTileTexture() manages this case.
	// And the memory overhead is not a problem (we talk about pointers).
}


// ***************************************************************************
uint		CLandscape::getTileLightMap(CRGBA  map[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE], CPatchRdrPass *&lightmapRdrPass)
{
	sint	textNum;
	uint	lightMapId;
	/*
		NB: TextureNear are a grow only Array... TextureNear are never deleted. Why? :
		2/ Unused near texture may be uncahced by opengl (and maybe by windows, to disk).

	  (old reason, no longer valid, since lightmaps are unlinked from tiles.
		1/ There is an important issue with releasing texture nears: tiles may acces them (see getTileRenderPass())
	  )
	*/
	// 0. Alloc Near Texture if necessary.
	//====================================
	if(_NFreeLightMaps==0)
	{
		CTextureNear	*text= new CTextureNear(TextureNearSize);
		TSPRenderPass	newPass= new CPatchRdrPass;
		text->setTextureCategory(_TextureNearCategory);

		newPass->TextureDiffuse= text;

		_TextureNears.push_back(newPass);
		_NFreeLightMaps+= text->getNbAvailableTiles();
	}

	// 1. Search the first texture which has a free tile.
	//==================================================
	CTextureNear	*nearText= NULL;
	CPatchRdrPass	*nearRdrPass= NULL;
	for(textNum=0;textNum<(sint)_TextureNears.size();textNum++)
	{
		nearRdrPass= _TextureNears[textNum];
		nearText= (CTextureNear*)(ITexture*)nearRdrPass->TextureDiffuse;
		if(nearText->getNbAvailableTiles()!=0)
			break;
	}
	nlassert(textNum<(sint)_TextureNears.size());
	// A empty space has been found.
	_NFreeLightMaps--;

	// 2. Fill the texture with the data, and updaterect.
	//===================================================
	lightMapId= nearText->getTileAndFillRect(map);
	// Compute the Id.
	lightMapId= textNum*NbTileLightMapByTexture + lightMapId;


	// 3. updateLighting
	//===================================================
	// Increment number of pixels to update for near.
	_ULTotalNearPixels+= NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE;


	// Result:
	lightmapRdrPass= nearRdrPass;
	return lightMapId;
}
// ***************************************************************************
void		CLandscape::getTileLightMapUvInfo(uint tileLightMapId, CVector &uvScaleBias)
{
	uint	id, s,t;

	// Scale.
	static const float	scale10= (float)NL_TILE_LIGHTMAP_SIZE/TextureNearSize;
	static const float	scale4= 4.f/TextureNearSize;
	static const float	scale1= 1.f/TextureNearSize;
	// The size of a minilightmap, mapped onto the polygon, is still 4 pixels.
	uvScaleBias.z= scale4;

	// Get the id local in the texture.
	id= tileLightMapId%NbTileLightMapByTexture;

	// Commpute UVBias.
	// Get the coordinate of the tile, in tile number.
	s= id%NbTileLightMapByLine;
	t= id/NbTileLightMapByLine;
	// But the real size of a minilightmap is 10 pixels, and we must reach the pixel 1,1.
	uvScaleBias.x= s*scale10 + scale1;
	uvScaleBias.y= t*scale10 + scale1;
}
// ***************************************************************************
void		CLandscape::releaseTileLightMap(uint tileLightMapId)
{
	uint	id, textNum;

	// Get the id local in the texture.
	textNum= tileLightMapId / NbTileLightMapByTexture;
	id= tileLightMapId % NbTileLightMapByTexture;
	nlassert(textNum<_TextureNears.size());

	// Release the tile in this texture.
	CPatchRdrPass	*nearRdrPass= _TextureNears[textNum];
	CTextureNear	*nearText= (CTextureNear*)(ITexture*)nearRdrPass->TextureDiffuse;
	nearText->releaseTile(id);
	_NFreeLightMaps++;

	// updateLighting
	// Decrement number of pixels to update for near.
	_ULTotalNearPixels-= NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE;
}


// ***************************************************************************
void		CLandscape::refillTileLightMap(uint tileLightMapId, CRGBA  map[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE])
{
	uint	id, textNum;

	// Get the id local in the texture.
	textNum= tileLightMapId / NbTileLightMapByTexture;
	id= tileLightMapId % NbTileLightMapByTexture;
	nlassert(textNum<_TextureNears.size());

	// get a ptr on the texture.
	CPatchRdrPass	*nearRdrPass= _TextureNears[textNum];
	CTextureNear	*nearText= (CTextureNear*)(ITexture*)nearRdrPass->TextureDiffuse;

	// refill this tile
	nearText->refillRect(id, map);
}



// ***************************************************************************
// ***************************************************************************
// Far.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CPatchRdrPass*	CLandscape::getFarRenderPass(CPatch* pPatch, uint farIndex, float& farUScale, float& farVScale, float& farUBias, float& farVBias, bool& bRot)
{
	// Check args
	nlassert (farIndex>0);

	// Get size of the far texture
	uint width=(pPatch->getOrderS ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);
	uint height=(pPatch->getOrderT ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);

	// For updateLighting: increment total of pixels to update.
	_ULTotalFarPixels+= width*height;

	// Try to allocate in every textures
	uint	i;
	sint	bestRdrPass= -1;
	sint	bestSplit= INT_MAX;
	for(i=0;i<_TextureFars.size();i++)
	{
		CTextureFar *pTextureFar=(CTextureFar*)(&*(_TextureFars[i]->TextureDiffuse));

		sint	splitRes= pTextureFar->tryAllocatePatch(pPatch, farIndex);
		// if found with no split, ok, stop!
		if(splitRes==0)
		{
			bestRdrPass= i;
			break;
		}
		// else if found, but with split
		else if(splitRes > 0)
		{
			// Then must take the small split possible along all the texture fars.
			if (splitRes<bestSplit)
			{
				bestRdrPass= i;
				bestSplit= splitRes;
			}
		}
	}

	// If no one found, must allocate a new render pass.
	if(bestRdrPass==-1)
	{
		bestRdrPass= (sint)_TextureFars.size();

		// add a new render pass
		CPatchRdrPass	*pass=new CPatchRdrPass;

		// Fill the render pass
		CTextureFar *pTextureFar=new CTextureFar;
		pTextureFar->setTextureCategory(_TextureFarCategory);

		// Append this textureFar to the list of TextureFar to updateLighting.
		if(_ULRootTextureFar==NULL)
			_ULRootTextureFar= pTextureFar;
		else
			pTextureFar->linkBeforeUL(_ULRootTextureFar);

		// Set the bank
		pTextureFar->_Bank=&TileFarBank;

		// Set as diffuse texture for this renderpass
		pass->TextureDiffuse=pTextureFar;

		// Add the render pass to the list
		_TextureFars.push_back(pass);
	}


	// Ok, add the patch to the best render pass in the _TextureFars
	TSPRenderPass pass= _TextureFars[bestRdrPass];

	// Get a pointer on the diffuse far texture
	CTextureFar *pTextureFar=(CTextureFar*)(&*(pass->TextureDiffuse));

	// Allocate really in it (must success since tryAllocate() has succeed)
	pTextureFar->allocatePatch(pPatch, farIndex, farUScale, farVScale, farUBias, farVBias, bRot);

	// Return the renderpass
	return pass;
}


// ***************************************************************************
void		CLandscape::freeFarRenderPass (CPatch* pPatch, CPatchRdrPass* pass, uint farIndex)
{
	// Get size of the far texture
	uint width=(pPatch->getOrderS ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);
	uint height=(pPatch->getOrderT ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);

	// For updateLighting: decrement total of pixels to update.
	_ULTotalFarPixels-= width*height;
	nlassert(_ULTotalFarPixels>=0);

	// Get a pointer on the diffuse far texture
	CTextureFar *pTextureFar=(CTextureFar*)(&*(pass->TextureDiffuse));

	// Remove from the patch from the texture if empty
	pTextureFar->removePatch (pPatch, farIndex);
}


// ***************************************************************************
void		CLandscape::clearFarRenderPass (CPatchRdrPass* pass)
{
	// Before deleting, must remove TextureFar from UpdateLighting list.

	// Get a pointer on the diffuse far texture
	CTextureFar *pTextureFar=(CTextureFar*)(&*(pass->TextureDiffuse));

	// If I delete the textureFar which is the current root
	if(_ULRootTextureFar==pTextureFar)
	{
		// switch to next
		_ULRootTextureFar= pTextureFar->getNextUL();
		// if still the same, it means that the circular list is now empty
		if(_ULRootTextureFar==pTextureFar)
			_ULRootTextureFar= NULL;
	}

	// unlink the texture from list
	pTextureFar->unlinkUL();
}


// ***************************************************************************
// ***************************************************************************
// Misc.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CZone*			CLandscape::getZone (sint zoneId)
{
	TZoneMap::iterator	it;
	it= Zones.find(uint16(zoneId));
	if (it!=Zones.end())
		return (*it).second;
	else
		return NULL;
}


// ***************************************************************************
const CZone*	CLandscape::getZone (sint zoneId) const
{
	TZoneMap::const_iterator	it;

	it= Zones.find(uint16(zoneId));
	if (it!=Zones.end())
		return (*it).second;
	else
		return NULL;
}



// ***************************************************************************
void			CLandscape::checkZoneBinds(CZone &curZone, EBadBind &bindError)
{
	for(sint i=0;i<curZone.getNumPatchs();i++)
	{
		const CZone::CPatchConnect	&pa= *curZone.getPatchConnect(i);

		// Check the bindInfos.
		for(sint j=0;j<4;j++)
		{
			const CPatchInfo::CBindInfo	&bd=pa.BindEdges[j];
			// Just 1/1 for now.
			if(bd.NPatchs==1)
			{
				CZone	*oZone= getZone(bd.ZoneId);
				// If loaded zone.
				if(oZone)
				{
					const CZone::CPatchConnect	&po= *(oZone->getPatchConnect(bd.Next[0]));
					const CPatchInfo::CBindInfo	&bo= po.BindEdges[bd.Edge[0]];
					if(bo.NPatchs!=1 || bo.Next[0]!=i || bo.Edge[0]!=j)
						bindError.BindErrors.push_back( EBadBind::CBindError(curZone.getZoneId(), i));
				}
			}
		}
	}
}


// ***************************************************************************
void			CLandscape::checkBinds() throw(EBadBind)
{
	EBadBind	bindError;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		CZone	&curZone= *(*it).second;
		checkZoneBinds(curZone, bindError);
	}

	if(!bindError.BindErrors.empty())
		throw bindError;
}


// ***************************************************************************
void			CLandscape::checkBinds(uint16 zoneId) throw(EBadBind)
{
	EBadBind	bindError;

	ItZoneMap it= Zones.find(zoneId);
	if(it!= Zones.end())
	{
		CZone	&curZone= *(*it).second;
		checkZoneBinds(curZone, bindError);
		if(!bindError.BindErrors.empty())
			throw bindError;
	}
}



// ***************************************************************************
void			CLandscape::addTrianglesInBBox(const CPatchIdentEx &paIdEx, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel) const
{
	// No clear here, just add triangles to the array.
	const CPatch	*pa= paIdEx.Patch;

	CPatchIdent		paId;
	paId.ZoneId= paIdEx.ZoneId;
	paId.PatchId= paIdEx.PatchId;
	pa->addTrianglesInBBox(paId, bbox, triangles, tileTessLevel);
}


// ***************************************************************************
void			CLandscape::buildTrianglesInBBox(const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel)
{
	// clear selection.
	triangles.clear();

	// search path of interest.
	_PatchQuadGrid.clearSelection();
	_PatchQuadGrid.select(bbox.getMin(), bbox.getMax());
	CQuadGrid<CPatchIdentEx>::CIterator	it;

	// for each patch, add triangles to the array.
	for(it= _PatchQuadGrid.begin(); it!= _PatchQuadGrid.end(); it++)
	{
		addTrianglesInBBox((*it), bbox, triangles, tileTessLevel);
	}
}



// ***************************************************************************
void			CLandscape::addPatchBlocksInBBox(const CPatchIdentEx &paIdEx, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds)
{
	// No clear here, just add blocks to the array.
	const CPatch	*pa= paIdEx.Patch;

	CPatchIdent		paId;
	paId.ZoneId= paIdEx.ZoneId;
	paId.PatchId= paIdEx.PatchId;
	pa->addPatchBlocksInBBox(paId, bbox, paBlockIds);
}


// ***************************************************************************
void			CLandscape::buildPatchBlocksInBBox(const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds)
{
	// clear selection.
	paBlockIds.clear();

	// search path of interest.
	_PatchQuadGrid.clearSelection();
	_PatchQuadGrid.select(bbox.getMin(), bbox.getMax());
	CQuadGrid<CPatchIdentEx>::CIterator	it;

	// for each patch, add blocks to the array.
	for(it= _PatchQuadGrid.begin(); it!= _PatchQuadGrid.end(); it++)
	{
		addPatchBlocksInBBox((*it), bbox, paBlockIds);
	}
}


// ***************************************************************************
void			CLandscape::fillPatchQuadBlock(CPatchQuadBlock &quadBlock) const
{
	sint zoneId=  quadBlock.PatchBlockId.PatchId.ZoneId;
	sint patchId= quadBlock.PatchBlockId.PatchId.PatchId;
	std::map<uint16, CZone*>::const_iterator	it= Zones.find(uint16(zoneId));
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId>=0);
		nlassert(patchId<N);

		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId);
		pa->fillPatchQuadBlock(quadBlock);
	}
}




// ***************************************************************************
void			CLandscape::buildCollideFaces(const CAABBoxExt &bbox, vector<CTriangle>	&faces, bool faceClip)
{
	CBSphere	bsWanted(bbox.getCenter(), bbox.getRadius());

	faces.clear();
	// For all zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		const CAABBoxExt	&bb= (*it).second->getZoneBB();
		CBSphere	bs(bb.getCenter(), bb.getRadius());
		// If zone intersect the wanted area.
		//===================================
		if(bs.intersect(bsWanted))
		{
			// Then trace all patch.
			sint	N= (*it).second->getNumPatchs();
			for(sint i=0;i<N;i++)
			{
				const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(i);

				// If patch in wanted area....
				//============================
				if(bsWanted.intersect((*it).second->getPatchBSphere(i)))
				{
					// 0. Build the faces.
					//====================
					sint	ordS= pa->getOrderS();
					sint	ordT= pa->getOrderT();
					sint	x,y,j;
					vector<CTriangle>	tmpFaces;
					tmpFaces.reserve(ordS*ordT);
					float	OOS= 1.0f/ordS;
					float	OOT= 1.0f/ordT;
					for(y=0;y<ordT;y++)
					{
						for(x=0;x<ordS;x++)
						{
							CTriangle	f;
							f.V0= pa->computeVertex(x*OOS, y*OOT);
							f.V1= pa->computeVertex(x*OOS, (y+1)*OOT);
							f.V2= pa->computeVertex((x+1)*OOS, (y+1)*OOT);
							tmpFaces.push_back(f);
							f.V0= pa->computeVertex(x*OOS, y*OOT);
							f.V1= pa->computeVertex((x+1)*OOS, (y+1)*OOT);
							f.V2= pa->computeVertex((x+1)*OOS, y*OOT);
							tmpFaces.push_back(f);
						}
					}

					// 1. Clip the faces.
					//===================
					if(faceClip)
					{
						// Insert only faces which are In the area.
						for(j=0;j<(sint)tmpFaces.size();j++)
						{
							CTriangle	&f= tmpFaces[j];
							if(bbox.intersect(f.V0, f.V1, f.V2))
							{
								faces.push_back(f);
							}
						}
					}
					else
					{
						// Else insert ALL.
						faces.insert(faces.end(), tmpFaces.begin(), tmpFaces.end());
					}
				}
			}
		}
	}
}


// ***************************************************************************
void			CLandscape::buildCollideFaces(sint zoneId, sint patch, std::vector<CTriangle> &faces)
{
	faces.clear();

	ItZoneMap it= Zones.find(uint16(zoneId));
	if(it!=Zones.end())
	{
		// Then trace all patch.
		sint	N= (*it).second->getNumPatchs();
		nlassert(patch>=0);
		nlassert(patch<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patch);

		// Build the faces.
		//=================
		sint	ordS= pa->getOrderS();
		sint	ordT= pa->getOrderT();
		sint	x,y;
		float	OOS= 1.0f/ordS;
		float	OOT= 1.0f/ordT;
		for(y=0;y<ordT;y++)
		{
			for(x=0;x<ordS;x++)
			{
				CTriangle	f;
				f.V0= pa->computeVertex(x*OOS, y*OOT);
				f.V1= pa->computeVertex(x*OOS, (y+1)*OOT);
				f.V2= pa->computeVertex((x+1)*OOS, (y+1)*OOT);
				faces.push_back(f);
				f.V0= pa->computeVertex(x*OOS, y*OOT);
				f.V1= pa->computeVertex((x+1)*OOS, (y+1)*OOT);
				f.V2= pa->computeVertex((x+1)*OOS, y*OOT);
				faces.push_back(f);
			}
		}
	}
}


// ***************************************************************************
CVector			CLandscape::getTesselatedPos(const CPatchIdent &patchId, const CUV &uv) const
{
	// First, must update globals, for CTessFace::computeTesselatedPos() to work properly.

	// VertexProgrma mode???
	CLandscapeGlobals::VertexProgramEnabled= _VertexShaderOk;

	// If VertexProgram enabled
	if( CLandscapeGlobals::VertexProgramEnabled )
	{
		/* because VertexProgram enabled, CTessVertex::Pos (geomorphed Pos) are not computed each frame
		   Hence, CTessFace::computeTesselatedPos() will call CTessVertex::computeGeomPos() to have correct
		   CTessVertex::Pos. ThereFore we must setup globals so CTessVertex::computeGeomPos() works properly.
		*/

		// see copy in updateGlobalsAndLockBuffers(). NB: Just copy what needed here!!!!

		// Tile subdivsion part.
		CLandscapeGlobals::TileDistNear = _TileDistNear;
		CLandscapeGlobals::TileDistFar = CLandscapeGlobals::TileDistNear+20;
		CLandscapeGlobals::TileDistNearSqr = sqr(CLandscapeGlobals::TileDistNear);
		CLandscapeGlobals::TileDistFarSqr = sqr(CLandscapeGlobals::TileDistFar);
		CLandscapeGlobals::OOTileDistDeltaSqr = 1.0f / (CLandscapeGlobals::TileDistFarSqr - CLandscapeGlobals::TileDistNearSqr);

		// RefineThreshold.
		CLandscapeGlobals::RefineThreshold= _Threshold;
		CLandscapeGlobals::OORefineThreshold= 1.0f / CLandscapeGlobals::RefineThreshold;

		// Refine Center*.
		// NB: setup the last setuped refineCenter.
		CLandscapeGlobals::RefineCenter= _OldRefineCenter;
	}


	// \todo yoyo: TODO_ZONEID: change ZoneId in 32 bits...
	std::map<uint16, CZone*>::const_iterator	it= Zones.find((uint16)patchId.ZoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId.PatchId<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId.PatchId);

		return pa->getTesselatedPos(uv);
	}
	else
		return CVector::Null;
}


#define NL_TILE_FAR_SIZE_ORDER0 (NL_NUM_PIXELS_ON_FAR_TILE_EDGE*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)
#define NL_TILE_FAR_SIZE_ORDER1 ((NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>1)*(NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>1))
#define NL_TILE_FAR_SIZE_ORDER2 ((NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>2)*(NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>2))

// ***************************************************************************
// internal use
bool			CLandscape::eraseTileFarIfNotGood (uint tileNumber, uint sizeOrder0, uint sizeOrder1, uint sizeOrder2)
{
	// The same tiles ?
	bool bSame=true;

	// It is the same tile ?
	if (TileFarBank.getTile (tileNumber)->isFill (CTileFarBank::diffuse))
	{
		// Good diffuse size ?
		if (
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::diffuse, CTileFarBank::order0) != sizeOrder0) ||
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::diffuse, CTileFarBank::order1) != sizeOrder1) ||
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::diffuse, CTileFarBank::order2) != sizeOrder2)
			)
		{
			TileFarBank.getTile (tileNumber)->erasePixels (CTileFarBank::diffuse);
			bSame=false;
		}
	}

	// It is the same tile ?
	if (TileFarBank.getTile (tileNumber)->isFill (CTileFarBank::additive))
	{
		// Good additive size ?
		if (
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::additive, CTileFarBank::order0) != sizeOrder0) ||
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::additive, CTileFarBank::order1) != sizeOrder1) ||
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::additive, CTileFarBank::order2) != sizeOrder2)
			)
		{
			TileFarBank.getTile (tileNumber)->erasePixels (CTileFarBank::additive);
			bSame=false;
		}
	}

	// Return true if the tiles seem to be the sames
	return bSame;
}

// ***************************************************************************
bool			CLandscape::initTileBanks ()
{
	// *** Check the two banks are OK
	_FarInitialized=false;

	// Compatibility check
	bool bCompatibility=true;

	// Same number of tiles
	if (TileBank.getTileCount()==TileFarBank.getNumTile())
	{
		// Same tileSet
		for (int tileSet=0; tileSet<TileBank.getTileSetCount(); tileSet++)
		{
			// Same tile128
			int tile;
			for (tile=0; tile<TileBank.getTileSet(tileSet)->getNumTile128(); tile++)
			{
				// tile number
				uint tileNumber=TileBank.getTileSet(tileSet)->getTile128(tile);

				// erase the tiles if not good
				bCompatibility&=eraseTileFarIfNotGood (tileNumber, NL_TILE_FAR_SIZE_ORDER0, NL_TILE_FAR_SIZE_ORDER1, NL_TILE_FAR_SIZE_ORDER2);
			}

			// Same tile256
			for (tile=0; tile<TileBank.getTileSet(tileSet)->getNumTile256(); tile++)
			{
				// tile number
				uint tileNumber=TileBank.getTileSet(tileSet)->getTile256(tile);

				// erase the tiles if not good
				bCompatibility&=eraseTileFarIfNotGood (tileNumber, NL_TILE_FAR_SIZE_ORDER0<<2, NL_TILE_FAR_SIZE_ORDER1<<2, NL_TILE_FAR_SIZE_ORDER2<<2);
			}

			// Same transition
			for (tile=0; tile<CTileSet::count; tile++)
			{
				// tile number
				uint tileNumber=TileBank.getTileSet(tileSet)->getTransition(tile)->getTile();

				// erase the tiles if not good
				bCompatibility&=eraseTileFarIfNotGood (tileNumber, NL_TILE_FAR_SIZE_ORDER0, NL_TILE_FAR_SIZE_ORDER1, NL_TILE_FAR_SIZE_ORDER2);
			}
		}

		// Far actived!
		_FarInitialized=true;
	}

	// Register / Load the vegetables.
	TileBank.initTileVegetableDescs(_VegetableManager);

	return bCompatibility;
}


// ***************************************************************************
void			CLandscape::setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply)
{
	sint nMultiply=(sint)(256.f*multiply);
	for (int i=0; i<256; i++)
	{
		sint max=0;
		sint r=(((nMultiply*diffuse.R*i)>>8)+ambiant.R*(256-i))>>8;
		if (r>max)
			max=r;
		sint g=(((nMultiply*diffuse.G*i)>>8)+ambiant.G*(256-i))>>8;
		if (g>max)
			max=g;
		sint b=(((nMultiply*diffuse.B*i)>>8)+ambiant.B*(256-i))>>8;
		if (b>max)
			max=b;
		// Not << 8 but << 7 because the _LightValue color table represent a ramp from 0 to 512
		r <<= 7;
		g <<= 7;
		b <<= 7;
		max=std::max(max, 256);
		r/=max;
		g/=max;
		b/=max;
		clamp (r, 0, 255);
		clamp (g, 0, 255);
		clamp (b, 0, 255);
		_LightValue[i].R=uint8(r);
		_LightValue[i].G=uint8(g);
		_LightValue[i].B=uint8(b);
		_LightValue[i].A=255;
	}
}

// ***************************************************************************
void			CLandscape::enableAutomaticLighting(bool enable)
{
	_AutomaticLighting= enable;
}

// ***************************************************************************
void			CLandscape::setupAutomaticLightDir(const CVector &lightDir)
{
	_AutomaticLightDir= lightDir;
	_AutomaticLightDir.normalize();
}


// ***************************************************************************
CVector		CLandscape::getHeightFieldDeltaZ(float x, float y) const
{
	if(_HeightField.ZPatchs.size()==0)
		return CVector::Null;

	// map to _HeightField coordinates.
	x-= _HeightField.OriginX;
	y-= _HeightField.OriginY;
	x*= _HeightField.OOSizeX;
	y*= _HeightField.OOSizeY;
	// get patch on the grid.
	sint	ix, iy;
	ix= (sint)floor(x);
	iy= (sint)floor(y);
	// out of the grid?
	if( ix<0 || ix>=(sint)_HeightField.Width || iy<0 || iy>=(sint)_HeightField.Height)
		return CVector::Null;

	// get patch.
	const CBezierPatchZ	&paz= _HeightField.ZPatchs[iy*_HeightField.Width + ix];

	// compute result.
	CVector	ret=CVector::Null;
	ret.x= 0;
	ret.y= 0;
	ret.z= paz.eval(x-ix, y-iy);

	return ret;
}



// ***************************************************************************
void		CLandscape::CBezierPatchZ::makeInteriors()
{
	float		&a = Vertices[0];
	float		&b = Vertices[1];
	float		&c = Vertices[2];
	float		&d = Vertices[3];
	Interiors[0] = Tangents[7] + Tangents[0] - a;
	Interiors[1] = Tangents[1] + Tangents[2] - b;
	Interiors[2] = Tangents[3] + Tangents[4] - c;
	Interiors[3] = Tangents[5] + Tangents[6] - d;
}
// ***************************************************************************
float		CLandscape::CBezierPatchZ::eval(float ps, float pt) const
{
	float	p;

	float ps2 = ps * ps;
	float ps1 = 1.0f - ps;
	float ps12 = ps1 * ps1;
	float s0 = ps12 * ps1;
	float s1 = 3.0f * ps * ps12;
	float s2 = 3.0f * ps2 * ps1;
	float s3 = ps2 * ps;
	float pt2 = pt * pt;
	float pt1 = 1.0f - pt;
	float pt12 = pt1 * pt1;
	float t0 = pt12 * pt1;
	float t1 = 3.0f * pt * pt12;
	float t2 = 3.0f * pt2 * pt1;
	float t3 = pt2 * pt;

	p = Vertices[0]	* s0 * t0	+
		Tangents[7] * s1 * t0	+
		Tangents[6] * s2 * t0	+
		Vertices[3] * s3 * t0;
	p+= Tangents[0] * s0 * t1	+
		Interiors[0]* s1 * t1	+
		Interiors[3]* s2 * t1	+
		Tangents[5] * s3 * t1;
	p+=	Tangents[1] * s0 * t2	+
		Interiors[1]* s1 * t2	+
		Interiors[2]* s2 * t2	+
		Tangents[4] * s3 * t2;
	p+=	Vertices[1] * s0 * t3	+
		Tangents[2] * s1 * t3	+
		Tangents[3] * s2 * t3	+
		Vertices[2] * s3 * t3;

	return p;
}


// ***************************************************************************
void		CLandscape::setHeightField(const CHeightMap &hf)
{
	if(hf.getWidth()<2)
		return;
	if(hf.getHeight()<2)
		return;

	// Fill basics.
	_HeightField.OriginX= hf.OriginX;
	_HeightField.OriginY= hf.OriginY;
	_HeightField.SizeX= hf.SizeX;
	_HeightField.SizeY= hf.SizeY;
	_HeightField.OOSizeX= 1/hf.SizeX;
	_HeightField.OOSizeY= 1/hf.SizeY;
	uint	w= hf.getWidth()-1;
	uint	h= hf.getHeight()-1;
	_HeightField.Width= w;
	_HeightField.Height= h;
	_HeightField.ZPatchs.resize(w * h);

	// compute  patchs
	sint	x,y;

	// compute vertices / tangents on each patch
	for(y=0;y<(sint)h;y++)
	{
		for(x=0;x<(sint)w;x++)
		{
			CBezierPatchZ	&paz= _HeightField.ZPatchs[y*w+x];
			// vertices.
			paz.Vertices[0]= hf.getZ(x, y);
			paz.Vertices[1]= hf.getZ(x, y+1);
			paz.Vertices[2]= hf.getZ(x+1, y+1);
			paz.Vertices[3]= hf.getZ(x+1, y);
		}
	}

	// compute tangents
	for(y=0;y<(sint)h;y++)
	{
		for(x=0;x<(sint)w;x++)
		{
			CBezierPatchZ	&paz= _HeightField.ZPatchs[y*w+x];
			sint	tg;
			// For each tangent, what vertex (relative to x,y) we must take.
			struct	CDeltaPos
			{
				sint	ox,oy;
				sint	dx1,dy1;
				sint	dx2,dy2;
			};
			static CDeltaPos	deltas[8]= {
				{0,0, 0,1, 0,-1} ,
				{0,1, 0,0, 0,2} ,
				{0,1, 1,1, -1,1} ,
				{1,1, 0,1, 2,1} ,
				{1,1, 1,0, 1,2} ,
				{1,0, 1,1, 1,-1} ,
				{1,0, 0,0, 2,0} ,
				{0,0, 1,0, -1,0} ,
				};

			// compute each tangent of this patch.
			for(tg=0; tg<8;tg++)
			{
				sint	x0,y0;
				sint	x1,y1;
				sint	x2,y2;
				x0= x+deltas[tg].ox; y0= y+deltas[tg].oy;
				x1= x+deltas[tg].dx1; y1= y+deltas[tg].dy1;
				x2= x+deltas[tg].dx2; y2= y+deltas[tg].dy2;

				// borders case.
				if(x2<0 || x2>=(sint)hf.getWidth() || y2<0 || y2>=(sint)hf.getHeight())
				{
					float		v,dv;
					// base of tangents.
					v= hf.getZ(x0,y0);
					// target tangents.
					dv= hf.getZ(x1,y1) - v;
					// result of tangent.
					paz.Tangents[tg]= v+dv/3;
				}
				// middle case.
				else
				{
					float		v,dv;
					// base of tangents.
					v= hf.getZ(x0,y0);
					// target tangents.
					dv= hf.getZ(x1,y1) - v;
					// add mirror target tangent.
					dv+= -(hf.getZ(x2,y2) - v);
					dv/=2;
					// result of tangent.
					paz.Tangents[tg]= v+dv/3;
				}
			}
		}
	}

	// compute interiors.
	for(y=0;y<(sint)h;y++)
	{
		for(x=0;x<(sint)w;x++)
		{
			CBezierPatchZ	&paz= _HeightField.ZPatchs[y*w+x];
			paz.makeInteriors();
		}
	}

}


// ***************************************************************************
void		CLandscape::getTessellationLeaves(std::vector<const CTessFace*>  &leaves) const
{
	leaves.clear();

	std::map<uint16, CZone*>::const_iterator	it;
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		// Then trace all patch.
		sint	N= (*it).second->getNumPatchs();
		for(sint i=0;i<N;i++)
		{
			const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(i);

			pa->appendTessellationLeaves(leaves);
		}
	}

}


// ***************************************************************************
void		CLandscape::setPZBModelPosition(const CVector &pos)
{
	_PZBModelPosition= pos;
}


// ***************************************************************************
class	CTextureFarLevelInfo
{
public:
	uint	NumUsedPatchs;
	uint	NumTextures;
	CTextureFarLevelInfo()
	{
		NumUsedPatchs= 0;
		NumTextures= 0;
	}
};

void		CLandscape::profileRender()
{
	// TODO yoyo: new Far mgt profile
	/*
	std::map<CVector2f, CTextureFarLevelInfo >	levelFarMap;

	nlinfo("***** Landscape TextureFar Profile. NumTextureFar= %d", _TextureFars.size());
	// Profile Texture Allocate
	ItSPRenderPassVector	itFar;
	uint	totalMemUsage= 0;
	for(itFar= _TextureFars.begin(); itFar!= _TextureFars.end(); itFar++)
	{
		CPatchRdrPass	&pass= **itFar;
		CTextureFar *textureFar= safe_cast<CTextureFar*>((ITexture*)pass.TextureDiffuse);

		// Info
		uint	memUsage= textureFar->getPatchWidth()*textureFar->getPatchHeight()*NL_NUM_FAR_PATCHES_BY_TEXTURE*2;
		nlinfo("  * Patch Texture Size: (%d,%d) => :%d bytes for %d patchs",
			textureFar->getPatchWidth(), textureFar->getPatchHeight(),
			memUsage, NL_NUM_FAR_PATCHES_BY_TEXTURE);
		totalMemUsage+= memUsage;

		// Profile Texture Far Allocated
		nlinfo("  * NumberOf Patch in the texture:%d", textureFar->getPatchCount());

		// Profile currently used Patchs
		uint	numRdrPatch= 0;
		CPatch	*pa= pass.getRdrPatchFar0();
		while(pa)
		{
			numRdrPatch++;
			pa= pa->getNextFar0ToRdr();
		}
		pa= pass.getRdrPatchFar1();
		while(pa)
		{
			numRdrPatch++;
			pa= pa->getNextFar1ToRdr();
		}
		nlinfo("  * NumberOf Patch in frustum for this texture (Far0+Far1):%d", numRdrPatch);

		// regroup by level
		CVector2f	sizeLevel;
		sizeLevel.x= (float)textureFar->getPatchWidth();
		sizeLevel.y= (float)textureFar->getPatchHeight();
		levelFarMap[sizeLevel].NumUsedPatchs+= textureFar->getPatchCount();
		levelFarMap[sizeLevel].NumTextures++;
	}

	nlinfo("***** Landscape TextureFar Level Profile. TotalVideoMemory= %d", totalMemUsage);
	std::map<CVector2f, CTextureFarLevelInfo >::iterator	itLevelFar= levelFarMap.begin();
	while(itLevelFar!=levelFarMap.end())
	{
		nlinfo("  * Level PatchSize: (%d, %d). Total NumberOf Patch: %d. Use Percentage: %d %%",
			(uint)itLevelFar->first.x, (uint)itLevelFar->first.y, itLevelFar->second.NumUsedPatchs,
			100*itLevelFar->second.NumUsedPatchs/(itLevelFar->second.NumTextures*NL_NUM_FAR_PATCHES_BY_TEXTURE) );

		itLevelFar++;
	}
	*/
}


// ***************************************************************************
// ***************************************************************************
// Allocators.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTessFace			*CLandscape::newTessFace()
{
	// allcoate the face.
	CTessFace		*face= TessFaceAllocator.allocate();

	// for refine() mgt, append the face to the list of newLeaves, so they will be tested in refine()
	face->linkInPList(_RootNewLeaves);

	return face;
}

// ***************************************************************************
CTessVertex			*CLandscape::newTessVertex()
{
	return TessVertexAllocator.allocate();
}

// ***************************************************************************
CTessNearVertex		*CLandscape::newTessNearVertex()
{
	return TessNearVertexAllocator.allocate();
}

// ***************************************************************************
CTessFarVertex		*CLandscape::newTessFarVertex()
{
	return TessFarVertexAllocator.allocate();
}

// ***************************************************************************
CTileMaterial		*CLandscape::newTileMaterial()
{
	return TileMaterialAllocator.allocate();
}

// ***************************************************************************
CTileFace			*CLandscape::newTileFace()
{
	return TileFaceAllocator.allocate();
}

// ***************************************************************************
void				CLandscape::deleteTessFace(CTessFace *f)
{
	// for refine() mgt, must remove from refine priority list, or from the temp rootTessFaceToUpdate list.
	f->unlinkInPList();

	TessFaceAllocator.free(f);
}

// ***************************************************************************
void				CLandscape::deleteTessVertex(CTessVertex *v)
{
	TessVertexAllocator.free(v);
}

// ***************************************************************************
void				CLandscape::deleteTessNearVertex(CTessNearVertex *v)
{
	TessNearVertexAllocator.free(v);
}

// ***************************************************************************
void				CLandscape::deleteTessFarVertex(CTessFarVertex *v)
{
	TessFarVertexAllocator.free(v);
}

// ***************************************************************************
void				CLandscape::deleteTileMaterial(CTileMaterial *tm)
{
	TileMaterialAllocator.free(tm);
}

// ***************************************************************************
void				CLandscape::deleteTileFace(CTileFace *tf)
{
	TileFaceAllocator.free(tf);
}



// ***************************************************************************
// ***************************************************************************
// Noise
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLandscape::setNoiseMode(bool enable)
{
	_NoiseEnabled= enable;
}

// ***************************************************************************
bool			CLandscape::getNoiseMode() const
{
	return _NoiseEnabled;
}


// ***************************************************************************
// ***************************************************************************
// Micro vegetation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CLandscape::enableVegetable(bool enable)
{
	_VegetableManagerEnabled= enable;

	// if false, delete all Vegetable IGs.
	if(!_VegetableManagerEnabled)
	{
		// Landscape always create ClipBlokcs, but IGs/addInstances() are created only if isVegetableActive().
		// For all zones.
		for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
		{
			// for all patch.
			sint	N= (*it).second->getNumPatchs();
			for(sint i=0;i<N;i++)
			{
				// delete vegetable Igs of this patch
				CPatch	*pa= ((*it).second)->getPatch(i);
				pa->deleteAllVegetableIgs();
			}

		}
	}
	// if true
	else
	{
		//  reload all Shapes (actually load only new shapes)
		TileBank.initTileVegetableDescs(_VegetableManager);

		// And recreate vegetable igs.
		// Landscape always create ClipBlokcs, but IGs/addInstances() are created only if isVegetableActive().
		// For all zones.
		for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
		{
			// for all patch.
			sint	N= (*it).second->getNumPatchs();
			for(sint i=0;i<N;i++)
			{
				// recreate vegetable Igs of this patch
				CPatch	*pa= ((*it).second)->getPatch(i);
				pa->recreateAllVegetableIgs();
			}

		}
	}

}

// ***************************************************************************
bool		CLandscape::isVegetableActive() const
{
	return _VegetableManagerEnabled && _DriverOkForVegetable;
}

// ***************************************************************************
void		CLandscape::loadVegetableTexture(const string &textureFileName)
{
	// load the texture in the manager
	_VegetableManager->loadTexture(textureFileName);
}

// ***************************************************************************
void		CLandscape::setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight)
{
	// set the directional light to the manager
	_VegetableManager->setDirectionalLight(ambient, diffuse, directionalLight);
}

// ***************************************************************************
void		CLandscape::setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin)
{
	// setup vegetable manager
	_VegetableManager->setWind(windDir, windFreq, windPower, windBendMin);
}


// ***************************************************************************
void		CLandscape::setVegetableTime(double time)
{
	// setup vegetable manager
	_VegetableManager->setTime(time);
}

// ***************************************************************************
void		CLandscape::setVegetableUpdateLightingTime(double time)
{
	// setup vegetable manager
	_VegetableManager->setUpdateLightingTime(time);
}


// ***************************************************************************
uint		CLandscape::getNumVegetableFaceRendered() const
{
	return _VegetableManager->getNumVegetableFaceRendered();
}


// ***************************************************************************
const CTileVegetableDesc	&CLandscape::getTileVegetableDesc(uint16 tileId)
{
	return TileBank.getTileVegetableDesc(tileId);
}


// ***************************************************************************
void		CLandscape::createVegetableBlendLayersModels(CScene *scene)
{
	_VegetableManager->createVegetableBlendLayersModels(scene);
}


// ***************************************************************************
void		CLandscape::setVegetableUpdateLightingFrequency(float freq)
{
	_VegetableManager->setUpdateLightingFrequency(freq);
}

// ***************************************************************************
void		CLandscape::setupColorsFromTileFlags(const NLMISC::CRGBA colors[4])
{
	for (TZoneMap::iterator it = Zones.begin(); it != Zones.end(); ++it)
	{
		it->second->setupColorsFromTileFlags(colors);
	}
}

// ***************************************************************************
void		CLandscape::setVegetableDensity(float density)
{
	// if the density is really different from what actually setuped
	if(density!=_VegetableManager->getGlobalDensity())
	{
		_VegetableManager->setGlobalDensity(density);

		// must recreate all vegetables IGs
		for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
		{
			// for all patch.
			sint	N= (*it).second->getNumPatchs();
			for(sint i=0;i<N;i++)
			{
				// delete vegetable Igs of this patch
				CPatch	*pa= ((*it).second)->getPatch(i);
				pa->deleteAllVegetableIgs();
				// then recreate vegetable Igs of this patch
				pa->recreateAllVegetableIgs();
			}

		}
	}

}

// ***************************************************************************
float		CLandscape::getVegetableDensity() const
{
	return _VegetableManager->getGlobalDensity();
}

// ***************************************************************************
// ***************************************************************************
// Lightmap Get interface.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
uint8		CLandscape::getLumel(const CPatchIdent &patchId, const CUV &uv) const
{
	// \todo yoyo: TODO_ZONEID: change ZoneId in 32 bits...
	std::map<uint16, CZone*>::const_iterator	it= Zones.find((uint16)patchId.ZoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId.PatchId<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId.PatchId);

		return pa->getLumel(uv);
	}
	else
		// Return full sun contribution as default
		return 255;
}

// ***************************************************************************
void		CLandscape::appendTileLightInfluences(const CPatchIdent &patchId, const CUV &uv,
	std::vector<CPointLightInfluence> &pointLightList) const
{
	// \todo yoyo: TODO_ZONEID: change ZoneId in 32 bits...
	std::map<uint16, CZone*>::const_iterator	it= Zones.find((uint16)patchId.ZoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId.PatchId<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId.PatchId);

		pa->appendTileLightInfluences(uv, pointLightList);
	}
}


// ***************************************************************************
// ***************************************************************************
// Lighting
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLandscape::removeAllPointLights()
{
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		// for all patch.
		sint	N= (*it).second->getNumPatchs();
		for(sint i=0;i<N;i++)
		{
			// Clear TileLightInfluences
			CPatch	*pa= ((*it).second)->getPatch(i);
			pa->resetTileLightInfluences();
		}

		// Remove all PointLights.
		(*it).second->_PointLightArray.clear();
	}

}


// ***************************************************************************
void			CLandscape::setPointLightFactor(const CScene &scene)
{
	// Affect currently added zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->_PointLightArray.setPointLightFactor(scene);
	}
}


// ***************************************************************************
void			CLandscape::updateLighting(double time)
{
	_ULTime= time;

	// first time in this method??
	if(!_ULPrecTimeInit)
	{
		_ULPrecTimeInit= true;
		_ULPrecTime= _ULTime;
	}
	// compute delta time from last update.
	float dt= float(_ULTime - _ULPrecTime);
	_ULPrecTime= _ULTime;


	// If not disabled
	if(_ULFrequency)
	{
		// Do it for near and far in 2 distinct ways.
		updateLightingTextureFar(dt * _ULFrequency);
		updateLightingTextureNear(dt * _ULFrequency);
	}

}


// ***************************************************************************
void			CLandscape::updateLightingAll()
{
	// Do it for near and far in 2 distinct ways.
	//================
	updateLightingTextureFar(1);
	updateLightingTextureNear(1);


	// update lighting for vegetables
	//================

	// Must lock buffers for update Lighting of vegetables.
	updateGlobalsAndLockBuffers (CVector::Null);

	// Because updateLighting() may use OptFastFloor..
	NLMISC::OptFastFloorBegin();

	// update ALL lighting for vegetables
	_VegetableManager->updateLightingAll();

	// Stop fastFloor optim.
	NLMISC::OptFastFloorEnd();

	// Must realase VB Buffers
	unlockBuffers();
}


// ***************************************************************************
void			CLandscape::setUpdateLightingFrequency(float freq)
{
	freq= max(freq, 0.f);
	_ULFrequency= freq;
}


// ***************************************************************************
void			CLandscape::linkPatchToNearUL(CPatch *patch)
{
	// Append this patch to the list of patch to updateLighting.
	if(_ULRootNearPatch==NULL)
		_ULRootNearPatch= patch;
	else
		patch->linkBeforeNearUL(_ULRootNearPatch);
}

// ***************************************************************************
void			CLandscape::unlinkPatchFromNearUL(CPatch *patch)
{
	// If I unlink the patch which is the current root
	if(_ULRootNearPatch==patch)
	{
		// switch to next
		_ULRootNearPatch= patch->getNextNearUL();
		// if still the same, it means that the circular list is now empty
		if(_ULRootNearPatch==patch)
			_ULRootNearPatch= NULL;
		// reset tessBlock counter.
		_ULNearCurrentTessBlockId= 0;
	}

	// unlink the patch from list
	patch->unlinkNearUL();
}


// ***************************************************************************
void			CLandscape::updateLightingTextureFar(float ratio)
{
	// compute number of pixels to update.
	_ULFarPixelsToUpdate+= ratio * _ULTotalFarPixels;
	// maximize, so at max, it computes all patchs, just one time.
	_ULFarPixelsToUpdate= min(_ULFarPixelsToUpdate, (float)_ULTotalFarPixels);

	// Test Profile Yoyo
	/*extern bool YOYO_LandULTest;
	if(YOYO_LandULTest)
	{
		nlinfo("YOYO_UL Far: %dK, %dK", (sint)_ULFarPixelsToUpdate/1024, (sint)_ULTotalFarPixels/1024);
	}*/

	// while there is still some pixels to update.
	while(_ULFarPixelsToUpdate > 0 && _ULRootTextureFar)
	{
		// update patch (if not null) in the textureFar.
		_ULFarPixelsToUpdate-= _ULRootTextureFar->touchPatchULAndNext();

		// last patch in the texture??
		if( _ULRootTextureFar->endPatchULTouch() )
		{
			// yes, go to next texture.
			_ULRootTextureFar= _ULRootTextureFar->getNextUL();
			// reset to 0th patch.
			_ULRootTextureFar->startPatchULTouch();
		}
	}

	// Now, _ULFarPixelsToUpdate should be <=0. (most of the time < 0)
}


// ***************************************************************************
void			CLandscape::updateLightingTextureNear(float ratio)
{
	// compute number of pixels to update.
	_ULNearPixelsToUpdate+= ratio * _ULTotalNearPixels;
	// maximize, so at max, it computes all patchs, just one time.
	_ULNearPixelsToUpdate= min(_ULNearPixelsToUpdate, (float)_ULTotalNearPixels);


	// while there is still some pixels to update.
	while(_ULNearPixelsToUpdate > 0 && _ULRootNearPatch)
	{
		// update tessBlock (if lightmap exist for this tessBlock) in the patch.
		_ULNearPixelsToUpdate-= _ULRootNearPatch->updateTessBlockLighting(_ULNearCurrentTessBlockId);
		// Next tessBlock to process.
		_ULNearCurrentTessBlockId++;

		// last tessBlock in the patch??
		if(_ULNearCurrentTessBlockId>=_ULRootNearPatch->getNumNearTessBlocks())
		{
			// yes, go to next patch.
			_ULRootNearPatch= _ULRootNearPatch->getNextNearUL();
			// reset to 0th tessBlock.
			_ULNearCurrentTessBlockId=0;
		}
	}

}


// ***************************************************************************
void			CLandscape::computeDynamicLighting(const std::vector<CPointLight*>	&pls)
{
	uint	i;

	// Update globals, and lock buffers
	//====================
	updateGlobalsAndLockBuffers (CVector::Null);
	// NB: averageTesselationVertices may change vertices in VB in visible patchs => buffers are locked.


	// Run all DLM Context create, to init Lighting process.
	//===============
	CPatchDLMContext	*ctxPtr= _PatchDLMContextList->begin();
	while(ctxPtr!=NULL)
	{
		// init lighting process, do differential from last computeDynamicLighting()
		ctxPtr->getPatch()->beginDLMLighting();

		// next
		ctxPtr= (CPatchDLMContext*)ctxPtr->Next;
	}


	// compile all pointLights
	//===============
	static vector<CPatchDLMPointLight>	dlmPls;
	dlmPls.resize(pls.size());
	for(i=0;i<dlmPls.size();i++)
	{
		// compile the pl.
		dlmPls[i].compile(*pls[i], _PointLightDiffuseMaterial, _DLMMaxAttEnd);
	}


	// For all pointLight, intersect patch.
	//===============
	for(i=0;i<dlmPls.size();i++)
	{
		CPatchDLMPointLight	&pl= dlmPls[i];

		// search patchs of interest: those which interssect the pointLight
		_PatchQuadGrid.clearSelection();
		_PatchQuadGrid.select(pl.BBox.getMin(), pl.BBox.getMax());
		CQuadGrid<CPatchIdentEx>::CIterator	it;

		// for each patch, light it with the light.
		for(it= _PatchQuadGrid.begin(); it!= _PatchQuadGrid.end(); it++)
		{
			// get the patch
			const CPatch	*pa= (*it).Patch;

			// More precise clipping:
			if( pa->getZone()->getPatchBSphere(pa->getPatchId()).intersect( pl.BSphere ) )
			{
				// Ok, light the patch with this spotLight
				const_cast<CPatch*>(pa)->processDLMLight(pl);
			}
		}

	}


	// Run all DLM Context create, to end Lighting process.
	//===============
	ctxPtr= _PatchDLMContextList->begin();
	while(ctxPtr!=NULL)
	{
		// get enxt now, because the DLM itself may be deleted in endDLMLighting()
		CPatchDLMContext	*next= (CPatchDLMContext*)ctxPtr->Next;

		// delete the DLM if no more needed (near don't use nor pointLights)
		ctxPtr->getPatch()->endDLMLighting();

		// next
		ctxPtr= next;
	}


	// Must realase VB Buffers
	//====================
	unlockBuffers();

}


// ***************************************************************************
void			CLandscape::setDynamicLightingMaxAttEnd(float maxAttEnd)
{
	maxAttEnd= max(maxAttEnd, 1.f);
	_DLMMaxAttEnd= maxAttEnd;
}


// ***************************************************************************
uint			CLandscape::getDynamicLightingMemoryLoad() const
{
	uint	mem= 0;
	// First, set size of global texture overhead.
	mem= NL3D_LANDSCAPE_DLM_WIDTH * NL3D_LANDSCAPE_DLM_HEIGHT * sizeof(CRGBA);

	// Then, for each patchContext created
	CPatchDLMContext	*ctxPtr= _PatchDLMContextList->begin();
	while(ctxPtr!=NULL)
	{
		// add its memory load.
		mem+= ctxPtr->getMemorySize();

		// next
		ctxPtr= (CPatchDLMContext*)ctxPtr->Next;
	}

	return mem;
}


// ***************************************************************************
void			CLandscape::setDLMGlobalVegetableColor(CRGBA gvc)
{
	_DLMGlobalVegetableColor= gvc;
}


// ***************************************************************************
void			CLandscape::setPointLightDiffuseMaterial(CRGBA diffuse)
{
	_PointLightDiffuseMaterial= diffuse;
}


// ***************************************************************************
void			CLandscape::initAnimatedLightIndex(const CScene &scene)
{
	// Affect currently added zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->_PointLightArray.initAnimatedLightIndex(scene);
	}
}

// ***************************************************************************
void			CLandscape::releaseAllTiles()
{
	nlassert(Zones.empty());
	releaseTiles (0, (uint32)TileInfos.size());
}


// ***************************************************************************
// ***************************************************************************
// Dynamic ShadowMaping
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLandscape::appendToShadowPolyReceiver(CTessFace *face)
{
	CTriangle	tri;
	tri.V0= face->VBase->EndPos;
	tri.V1= face->VLeft->EndPos;
	tri.V2= face->VRight->EndPos;
	// Add and store id for remove
	face->ShadowMapTriId= _ShadowPolyReceiver.addTriangle(tri);
}

// ***************************************************************************
void			CLandscape::removeFromShadowPolyReceiver(CTessFace *face)
{
	if(face->ShadowMapTriId!=-1)
	{
		_ShadowPolyReceiver.removeTriangle(face->ShadowMapTriId);
		// set NULL Id.
		face->ShadowMapTriId=-1;
	}
}


// ***************************************************************************
void			CLandscape::receiveShadowMap(IDriver *drv, CShadowMap *shadowMap, const CVector &casterPos, const CMaterial &shadowMat, const CVector &pzb)
{
	/*  substract the PZB from all coordinates.

		Must add a small zbias because
		The rendered Triangles may be computed with VertexProgram, but _ShadowPolyReceiver
		does not. => there is a small float difference at the end
		Even if same vertex is produced in theory, VertexProgram may cause 2 precision problems:
		1/ On NVidia, even with a simple matrix mul VP, the precision result is not the same
		2/ Our Landscape VP is not a simple matrix mul. Lot of vertex mul/add are done fpr geomorphs
	*/
	CMaterial	&sm= const_cast<CMaterial&>(shadowMat);
	float	oldZBias= sm.getZBias();
	sm.setZBias(-0.02f);
	_ShadowPolyReceiver.render(drv, sm, shadowMap, casterPos, -pzb);
	sm.setZBias(oldZBias);
}

// ***************************************************************************
void			CLandscape::setZFunc(CMaterial::ZFunc val)
{
	TileMaterial.setZFunc(val);
	FarMaterial.setZFunc(val);
}

// ***************************************************************************
void CLandscape::invalidateAllTiles()
{

	updateGlobalsAndLockBuffers(CVector::Null);
	for(TZoneMap::iterator it = Zones.begin(); it != Zones.end(); ++it)
	{
		if (it->second->Compiled)
		{
			for(uint k = 0; k < it->second->Patchs.size(); ++k)
			{
				it->second->Patchs[k].deleteTileUvs();
				it->second->Patchs[k].recreateTileUvs();
			}
		}
	}
	unlockBuffers();
	updateTessBlocksFaceVector();
}

// ***************************************************************************
float CLandscape::getCameraCollision(const CVector &start, const CVector &end, float radius, bool cone)
{
	return _ShadowPolyReceiver.getCameraCollision(start, end,
		cone?CShadowPolyReceiver::CameraColCone:CShadowPolyReceiver::CameraColCylinder, radius);
}

// ***************************************************************************
float CLandscape::getRayCollision(const CVector &start, const CVector &end)
{
	return _ShadowPolyReceiver.getCameraCollision(start, end,
		CShadowPolyReceiver::CameraColSimpleRay, 0.f);
}

// ***************************************************************************
bool CLandscape::isTileCallback(ULandscapeTileCallback *cb) const
{
	return std::find(_TileCallbacks.begin(), _TileCallbacks.end(), cb) != _TileCallbacks.end();
}

// ***************************************************************************
void CLandscape::addTileCallback(ULandscapeTileCallback *cb)
{
	nlassert(cb);
	nlassert(!isTileCallback(cb)); // callback added twice
	_TileCallbacks.push_back(cb);
}

// ***************************************************************************
void CLandscape::removeTileCallback(ULandscapeTileCallback *cb)
{
	nlassert(cb);
	std::vector<ULandscapeTileCallback *>::iterator it = std::find(_TileCallbacks.begin(), _TileCallbacks.end(), cb);
	nlassert(it != _TileCallbacks.end());
	_TileCallbacks.erase(it);
}



} // NL3D





















