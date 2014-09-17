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

#include "nel/3d/scene.h"
#include "nel/3d/trav_scene.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/light_trav.h"
#include "nel/3d/anim_detail_trav.h"
#include "nel/3d/load_balancing_trav.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/transform.h"
#include "nel/3d/camera.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/driver.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/mesh_base.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/mesh_mrm_instance.h"
#include "nel/3d/mesh_mrm_skinned_instance.h"
#include "nel/3d/mesh_multi_lod_instance.h"
#include "nel/3d/shape_bank.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/particle_system_model.h"
#include "nel/3d/coarse_mesh_manager.h"
#include "nel/3d/cluster.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/flare_model.h"
#include "nel/3d/water_model.h"
#include "nel/3d/vegetable_blend_layer_model.h"
#include "nel/3d/root_model.h"
#include "nel/3d/point_light_model.h"
#include "nel/3d/animation.h"
#include "nel/3d/lod_character_manager.h"
#include "nel/3d/seg_remanence.h"
#include "nel/3d/async_texture_manager.h"
#include "nel/3d/water_env_map.h"
#include "nel/3d/skeleton_spawn_script.h"


#include <memory>

#include "nel/misc/time_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

//
#include "nel/misc/system_info.h"

using namespace std;
using namespace NLMISC;

#define NL3D_SCENE_COARSE_MANAGER_TEXTURE	"nel_coarse_texture.tga"


// The manager is limited to a square of 3000m*3000m around the camera. Beyond, models are clipped individually (bad!!).
const	float	NL3D_QuadGridClipManagerRadiusMax= 1500;
const	float	NL3D_QuadGridClipClusterSize= 400;
const	uint	NL3D_QuadGridClipNumDist= 10;
const	float	NL3D_QuadGridClipMaxDist= 1000;


#define	NL3D_SCENE_DEFAULT_SHADOW_MAP_SIZE		64
#define	NL3D_SCENE_DEFAULT_SHADOW_MAP_BLUR_SIZE	2
#define	NL3D_SCENE_DEFAULT_SHADOW_MAP_DIST_FADE_START			40
#define	NL3D_SCENE_DEFAULT_SHADOW_MAP_DIST_FADE_END				50
#define	NL3D_SCENE_DEFAULT_SHADOW_MAP_MAX_CASTER_IN_SCREEN		16
#define	NL3D_SCENE_DEFAULT_SHADOW_MAP_MAX_CASTER_AROUND			64


namespace NL3D
{

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


void	CScene::registerBasics()
{
	CTransform::registerBasic();
	CCamera::registerBasic();
	CMeshBaseInstance::registerBasic();
	CMeshInstance::registerBasic();
	CMeshMRMInstance::registerBasic();
	CMeshMRMSkinnedInstance::registerBasic();
	CLandscapeModel::registerBasic();
	CTransformShape::registerBasic();
	CSkeletonModel::registerBasic();
	CParticleSystemModel::registerBasic() ;
	CMeshMultiLodInstance::registerBasic();
	CCluster::registerBasic();
	CFlareModel::registerBasic();
	CWaterModel::registerBasic();
	CWaveMakerModel::registerBasic();
	CVegetableBlendLayerModel::registerBasic();
	CRootModel::registerBasic();
	CPointLightModel::registerBasic();
	CSegRemanence::registerBasic();
	CQuadGridClipManager::registerBasic();
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CScene::CScene(bool bSmallScene) : LightTrav(bSmallScene)
{
	HrcTrav.Scene= this;
	ClipTrav.Scene= this;
	LightTrav.Scene= this;
	AnimDetailTrav.Scene= this;
	LoadBalancingTrav.Scene= this;
	RenderTrav.Scene= this;

	_ShapeBank = NULL;

	Root= NULL;
	RootCluster= NULL;
	SonsOfAncestorSkeletonModelGroup= NULL;
	_QuadGridClipManager= NULL;

	_CurrentTime = 0 ;
	_EllapsedTime = 0 ;
	_RealTime = 0 ;
	_FirstAnimateCall = true ;

	_LightingSystemEnabled= false;
	_CoarseMeshLightingUpdate= 50;

	_GlobalWindDirection.set(1,0,0);
	// Default as Sithikt wants.
	_GlobalWindPower= 0.2f;

	// global manager (created in CDriverUser)
	_LodCharacterManager= NULL;
	_AsyncTextureManager= NULL;

	_NumRender = 0;

	_MaxSkeletonsInNotCLodForm= 20;

	_FilterRenderFlags= std::numeric_limits<uint32>::max();

	_NextRenderProfile= false;

	// Init default _CoarseMeshManager
	_CoarseMeshManager= new CCoarseMeshManager;
	_CoarseMeshManager->setTextureFile (NL3D_SCENE_COARSE_MANAGER_TEXTURE);

	// Update model list to NULL
	_UpdateModelList= NULL;

	_FlareContext = 0;

	_ShadowMapTextureSize= NL3D_SCENE_DEFAULT_SHADOW_MAP_SIZE;
	_ShadowMapBlurSize= NL3D_SCENE_DEFAULT_SHADOW_MAP_BLUR_SIZE;
	_ShadowMapDistFadeStart= NL3D_SCENE_DEFAULT_SHADOW_MAP_DIST_FADE_START;
	_ShadowMapDistFadeEnd= NL3D_SCENE_DEFAULT_SHADOW_MAP_DIST_FADE_END;
	_ShadowMapMaxCasterInScreen= NL3D_SCENE_DEFAULT_SHADOW_MAP_MAX_CASTER_IN_SCREEN;
	_ShadowMapMaxCasterAround= NL3D_SCENE_DEFAULT_SHADOW_MAP_MAX_CASTER_AROUND;
	_VisualCollisionManagerForShadow= NULL;

	_WaterCallback = NULL;
	_PolyDrawingCallback = NULL;
	_IsRendering = false;

	_FirstFlare = NULL;
	_RenderedPart = UScene::RenderNothing;
	//_WaterEnvMapRdr = NULL;
	//_WaterEnvMap = new CTextureCube;

	_WaterEnvMap = NULL;

	_GlobalSystemTime= 0.0;

	_RequestParticlesAnimate = false;
}
// ***************************************************************************
void	CScene::release()
{
	// reset the _QuadGridClipManager, => unlink models, and delete clusters.
	if( _QuadGridClipManager )
		_QuadGridClipManager->reset();

	// First, delete models.
	set<CTransform*>::iterator	it;
	it= _Models.begin();
	while( it!=_Models.end())
	{
		CTransform	*tr= *it;
		// Don't delete The Roots, because used, for instance in ~CSkeletonModel()
		if(tr!=Root && tr!=RootCluster && tr!=SonsOfAncestorSkeletonModelGroup)
			deleteModel(tr);
		// temp erase from the list
		else
			_Models.erase(it);
		// NB: important to take begin(), and not it++, cause ~CSkeletonModel() may delete ScriptSpawned models
		it= _Models.begin();
	}

	// Then delete the roots
	// reinsert
	if(Root)									_Models.insert(Root);
	if(RootCluster)								_Models.insert(RootCluster);
	if(SonsOfAncestorSkeletonModelGroup)		_Models.insert(SonsOfAncestorSkeletonModelGroup);
	// delete in the reverse order of initDefaultRoots()
	if(SonsOfAncestorSkeletonModelGroup)
	{
		deleteModel(SonsOfAncestorSkeletonModelGroup);
		SonsOfAncestorSkeletonModelGroup= NULL;
	}
	if(RootCluster)
	{
		deleteModel(RootCluster);
		RootCluster= NULL;
	}
	if(Root)
	{
		deleteModel(Root);
		Root= NULL;
	}

	// No models at all.
	_UpdateModelList= NULL;

	// reset ptrs
	_ShapeBank = NULL;
	Root= NULL;
	RootCluster= NULL;
	SonsOfAncestorSkeletonModelGroup= NULL;
	CurrentCamera= NULL;
	_QuadGridClipManager= NULL;
	ClipTrav.setQuadGridClipManager(NULL);

	// DON'T reset the _LodCharacterManager, because it can be shared across scenes
	/*
	if(_LodCharacterManager)
		_LodCharacterManager->reset();
	*/
	_LodCharacterManager= NULL;

	// delete the coarseMeshManager
	if(_CoarseMeshManager)
	{
		delete _CoarseMeshManager;
		_CoarseMeshManager= NULL;
	}

	if(_GlobalInstanceGroup)
	{
		delete _GlobalInstanceGroup;
		_GlobalInstanceGroup = NULL;
	}

	// delete the play list
	_LMAnimsAuto.deleteAll();
}
// ***************************************************************************
CScene::~CScene()
{
	release();
}
// ***************************************************************************
void	CScene::initDefaultRoots()
{
	// Create and set root the default models.
	Root= static_cast<CTransform*>(createModel(TransformId));

	// The root is always freezed (never move).
	Root->freeze();

	// Init the instance group that represent the world
	_GlobalInstanceGroup = new CInstanceGroup;
	RootCluster= (CCluster*)createModel (ClusterId);
	// unlink from hrc.
	RootCluster->hrcUnlink();
	RootCluster->Name = "ClusterRoot";
	RootCluster->Group = _GlobalInstanceGroup;
	_GlobalInstanceGroup->addCluster (RootCluster);

	// init the ClipTrav.RootCluster.
	ClipTrav.RootCluster = RootCluster;

	// Create a SonsOfAncestorSkeletonModelGroup, for models which have a skeleton ancestor
	SonsOfAncestorSkeletonModelGroup= static_cast<CRootModel*>(createModel(RootModelId));
	// must unlink it from all traversals, because special, only used in CClipTrav::traverse()
	SonsOfAncestorSkeletonModelGroup->hrcUnlink();
	Root->clipDelChild(SonsOfAncestorSkeletonModelGroup);
}

// ***************************************************************************
void	CScene::initQuadGridClipManager ()
{
	// Init clip features.
	if( !_QuadGridClipManager )
	{
		// create the model
		_QuadGridClipManager= static_cast<CQuadGridClipManager*>(createModel(QuadGridClipManagerId));
		// unlink it from hrc, and link it only to RootCluster.
		// NB: hence the quadGridClipManager may be clipped by the cluster system
		_QuadGridClipManager->hrcUnlink();
		_QuadGridClipManager->clipUnlinkFromAll();
		RootCluster->clipAddChild(_QuadGridClipManager);

		// init _QuadGridClipManager.
		_QuadGridClipManager->init(NL3D_QuadGridClipClusterSize,
			NL3D_QuadGridClipNumDist,
			NL3D_QuadGridClipMaxDist,
			NL3D_QuadGridClipManagerRadiusMax);
	}
}


// ***************************************************************************
void	CScene::render(bool	doHrcPass)
{
	beginPartRender();
	renderPart(UScene::RenderAll, doHrcPass);
	endPartRender();
}


// ***************************************************************************
void	CScene::beginPartRender()
{
	nlassert(!_IsRendering);

	// Do not delete model during the rendering
	// Also do not create model with CSkeletonSpawnScript model animation
	_IsRendering = true;
	_RenderedPart= UScene::RenderNothing;

}


// ***************************************************************************
void	CScene::endPartRender(bool keepTrav)
{
	nlassert(_IsRendering);
	_IsRendering = false;

	if (!keepTrav)
	{
		// Delete model deleted during the rendering
		uint i;
		for (i=0; i<_ToDelete.size(); i++)
			deleteModel (_ToDelete[i]);
		_ToDelete.clear ();

		// Special for SkeletonSpawnScript animation. create models spawned now
		flushSSSModelRequests();

		// Particle system handling (remove the resources of those which are too far, as their clusters may not have been parsed).
		// Note that only a few of them are tested at each call
		_ParticleSystemManager.refreshModels(ClipTrav.WorldFrustumPyramid, ClipTrav.CamPos);

		// Waiting Instance handling
		double deltaT = _DeltaSystemTimeBetweenRender;
		clamp (deltaT, 0.01, 0.1);
		updateWaitingInstances(deltaT);
	}

	// Reset profiling
	_NextRenderProfile= false;

	IDriver *drv = getDriver();
	drv->activeVertexProgram(NULL);
	drv->activePixelProgram(NULL);
	drv->activeGeometryProgram(NULL);

	// Ensure nothing animates on subsequent renders
	_EllapsedTime = 0.f;

	/*
	uint64 total = PSStatsRegisterPSModelObserver +
				  PSStatsRemovePSModelObserver +
				  PSStatsUpdateOpacityInfos +
				  PSStatsUpdateLightingInfos +
				  PSStatsGetAABBox +
				  PSStatsReallocRsc +
				  PSStatsReleasePSPointer +
				  PSStatsRefreshRscDeletion +
				  PSStatsReleaseRsc +
				  PSStatsReleaseRscAndInvalidate +
				  PSStatsGetNumTriangles +
				  PSStatsCheckAgainstPyramid +
				  PSStatsTraverseAnimDetail +
				  PSStatsDoAnimate +
				  PSStatsTraverseRender +
				  PSStatsTraverseClip +
				  PSStatsCheckDestroyCondition +
				  PSStatsForceInstanciate +
				  PSStatsDoAnimatePart1 +
				  PSStatsDoAnimatePart2 +
				  PSStatsDoAnimatePart3 +
				  PSStatsTraverseAnimDetailPart1 +
				  PSStatsTraverseAnimDetailPart2 +
				  PSStatsTraverseAnimDetailPart3 +
				  PSStatsTraverseAnimDetailPart4 +
				  PSAnim1 +
				  PSAnim2+
				  PSAnim3+
				  PSAnim4+
				  PSAnim5+
				  PSAnim6+
				  PSAnim7+
				  PSAnim8+
				  PSAnim9+
				  PSAnim10+
				  PSAnim11;


	 if (((double) total / (double) NLMISC::CSystemInfo::getProcessorFrequency()) > 0.01)
	 {
		  nlinfo("***** PS STATS ****");
		  #define PS_STATS(var) \
		  nlinfo("time for " #var " = %.2f", (float) (1000 * ((double) var / (double) CSystemInfo::getProcessorFrequency())));

		  PS_STATS(PSStatsRegisterPSModelObserver)
		  PS_STATS(PSStatsRemovePSModelObserver)
		  PS_STATS(PSStatsUpdateOpacityInfos)
		  PS_STATS(PSStatsUpdateLightingInfos)
		  PS_STATS(PSStatsGetAABBox)
		  PS_STATS(PSStatsReallocRsc)
		  PS_STATS(PSStatsReleasePSPointer)
		  PS_STATS(PSStatsRefreshRscDeletion)
		  PS_STATS(PSStatsReleaseRsc)
		  PS_STATS(PSStatsReleaseRscAndInvalidate)
		  PS_STATS(PSStatsGetNumTriangles)
		  PS_STATS(PSStatsCheckAgainstPyramid)
		  PS_STATS(PSStatsTraverseAnimDetail)
		  PS_STATS(PSStatsDoAnimate)
		  PS_STATS(PSStatsTraverseRender)
		  PS_STATS(PSStatsTraverseClip)
		  PS_STATS(PSStatsClipSystemInstanciated);
		  PS_STATS(PSStatsClipSystemNotInstanciated);
		  PS_STATS(PSStatsClipSystemCheckAgainstPyramid);
		  PS_STATS(PSStatsInsertInVisibleList);
		  PS_STATS(PSStatsCheckDestroyCondition)
		  PS_STATS(PSStatsForceInstanciate)
		  PS_STATS(PSStatsDoAnimatePart1)
		  PS_STATS(PSStatsDoAnimatePart2)
		  PS_STATS(PSStatsDoAnimatePart3)
		  PS_STATS(PSStatsTraverseAnimDetailPart1)
		  PS_STATS(PSStatsTraverseAnimDetailPart2)
		  PS_STATS(PSStatsTraverseAnimDetailPart3)
		  PS_STATS(PSStatsTraverseAnimDetailPart4)
		  PS_STATS(PSAnim1)
		  PS_STATS(PSAnim2)
		  PS_STATS(PSAnim3)
		  PS_STATS(PSAnim4)
		  PS_STATS(PSAnim5)
		  PS_STATS(PSAnim6)
		  PS_STATS(PSAnim7)
		  PS_STATS(PSAnim8)
		  PS_STATS(PSAnim9)
		  PS_STATS(PSAnim10)
		  PS_STATS(PSAnim11)
		  PS_STATS(PSStatsZonePlane)
		  PS_STATS(PSStatsZoneSphere)
		  PS_STATS(PSStatsZoneDisc)
		  PS_STATS(PSStatsZoneRectangle)
		  PS_STATS(PSStatsZoneCylinder)
		  PS_STATS(PSMotion1)
		  PS_STATS(PSMotion2)
		  PS_STATS(PSMotion3)
		  PS_STATS(PSMotion4)
		  PS_STATS(PSStatCollision)
		  PS_STATS(PSStatEmit)
		  PS_STATS(PSStatRender)


		nlinfo("num do animate = %d", (int) PSStatsNumDoAnimateCalls);

		nlinfo("Max et = %.2f", PSMaxET);
		nlinfo("Max ps nb pass = %d", (int) PSMaxNBPass);

		PS_STATS(total)

	 }

	 PSStatsRegisterPSModelObserver = 0;
	 PSStatsRemovePSModelObserver = 0;
	 PSStatsUpdateOpacityInfos = 0;
	 PSStatsUpdateLightingInfos = 0;
	 PSStatsGetAABBox = 0;
	 PSStatsReallocRsc = 0;
	 PSStatsReleasePSPointer = 0;
	 PSStatsRefreshRscDeletion = 0;
	 PSStatsReleaseRsc = 0;
	 PSStatsReleaseRscAndInvalidate = 0;
	 PSStatsGetNumTriangles = 0;
	 PSStatsCheckAgainstPyramid = 0;
	 PSStatsTraverseAnimDetail = 0;
	 PSStatsDoAnimate = 0;
	 PSStatsTraverseRender = 0;
	 PSStatsTraverseClip = 0;
	 PSStatsCheckDestroyCondition = 0;
	 PSStatsForceInstanciate = 0;
	 PSStatsClipSystemInstanciated = 0;
	 PSStatsClipSystemNotInstanciated = 0;
	 PSStatsClipSystemCheckAgainstPyramid = 0;
	 PSStatsInsertInVisibleList = 0;
	 PSStatsDoAnimatePart1 = 0;
	 PSStatsDoAnimatePart2 = 0;
	 PSStatsDoAnimatePart3 = 0;
	 PSStatsTraverseAnimDetailPart1 = 0;
	 PSStatsTraverseAnimDetailPart2 = 0;
	 PSStatsTraverseAnimDetailPart3 = 0;
	 PSStatsTraverseAnimDetailPart4 = 0;
	 PSStatsNumDoAnimateCalls = 0;
	 PSAnim1 = 0;
	 PSAnim2 = 0;
	 PSAnim3 = 0;
	 PSAnim4 = 0;
	 PSAnim5 = 0;
	 PSAnim6 = 0;
	 PSAnim7 = 0;
	 PSAnim8 = 0;
	 PSAnim9 = 0;
	 PSAnim10 = 0;
	 PSAnim11 = 0;
	 PSMaxET = 0.f;
	 PSMaxNBPass = 0;
	 PSStatsZonePlane = 0;
	 PSStatsZoneSphere = 0;
	 PSStatsZoneDisc = 0;
	 PSStatsZoneRectangle = 0;
	 PSStatsZoneCylinder = 0;
	 PSMotion1 = 0;
	 PSMotion2 = 0;
	 PSMotion3 = 0;
	 PSMotion4 = 0;
	 PSStatCollision = 0;
	 PSStatEmit = 0;
	 PSStatRender = 0;
	 */
}


// ***************************************************************************
void	CScene::renderPart(UScene::TRenderPart rp, bool	doHrcPass, bool doTrav, bool keepTrav)
{
	nlassert(_IsRendering);

	// if nothing (????), abort
	if(rp==UScene::RenderNothing)
		return;

	// If part asked already rendered, abort
	nlassert((rp & _RenderedPart) == 0); // cannot render the same part twice during a render

	// if first part to be rendered, do the start stuff
	if (_RenderedPart == UScene::RenderNothing)
	{
		RenderTrav.clearWaterModelList();

		if (doTrav)
		{
			// update water envmap
			//updateWaterEnvmap();
			_FirstFlare = NULL;

			double fNewGlobalSystemTime = NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime());
			if(_GlobalSystemTime==0)
				_DeltaSystemTimeBetweenRender= 0.020;
			else
				_DeltaSystemTimeBetweenRender= fNewGlobalSystemTime - _GlobalSystemTime;
			_GlobalSystemTime = fNewGlobalSystemTime;
		}
		//
		++ _NumRender;
		//
		nlassert(CurrentCamera);


		// update models.
		updateModels();


		// Use the camera to setup Clip / Render pass.
		float left, right, bottom, top, znear, zfar;
		CurrentCamera->getFrustum(left, right, bottom, top, znear, zfar);

		// setup basic camera.
		ClipTrav.setFrustum(left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());

		RenderTrav.setFrustum (left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());
		RenderTrav.setViewport (_Viewport);

		LoadBalancingTrav.setFrustum (left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());


		// Set Infos for cliptrav.
		ClipTrav.Camera = CurrentCamera;
		ClipTrav.setQuadGridClipManager (_QuadGridClipManager);


		// **** For all render traversals, traverse them (except the Hrc one), in ascending order.
		if( doHrcPass )
			HrcTrav.traverse();
		else
			HrcTrav._MovingObjects.clear();

		// Set Cam World Matrix for all trav that need it
		ClipTrav.setCamMatrix(CurrentCamera->getWorldMatrix());
		RenderTrav.setCamMatrix (CurrentCamera->getWorldMatrix());
		LoadBalancingTrav.setCamMatrix (CurrentCamera->getWorldMatrix());

		// clip
		ClipTrav.traverse();

		// animDetail
		AnimDetailTrav.traverse();

		// loadBalance
		LoadBalancingTrav.traverse();

		if (doTrav)
		{
			//
			if (_RequestParticlesAnimate)
			{
				_ParticleSystemManager.processAnimate(_EllapsedTime); // deals with permanently animated particle systems
				_RequestParticlesAnimate = false;
			}
		}

		// Light
		LightTrav.traverse();
	}

	// render
	RenderTrav.traverse(rp, (_RenderedPart == UScene::RenderNothing), doTrav);
	if (!keepTrav)
	{
		// Always must clear shadow caster (if render did not work because of IDriver::isLost())
		RenderTrav.getShadowMapManager().clearAllShadowCasters();
	}

	// render flare
	if (rp & UScene::RenderFlare)
	{
		if (doTrav)
		{
			if (_FirstFlare)
			{
				IDriver *drv = getDriver();
				CFlareModel::updateOcclusionQueryBegin(drv);
				CFlareModel	*currFlare = _FirstFlare;
				do
				{
					currFlare->updateOcclusionQuery(drv);
					currFlare = currFlare->Next;
				}
				while(currFlare);
				CFlareModel::updateOcclusionQueryEnd(drv);
			}
		}
		else
		{
			_FirstFlare = NULL;
		}
	}
	_RenderedPart = (UScene::TRenderPart) (_RenderedPart | rp);
}

// ***************************************************************************
void CScene::updateWaitingInstances(double systemTimeEllapsed)
{
	// First set up max AGP upload
	double fMaxBytesToUp = 100 * 256 * 256 * systemTimeEllapsed;
	_ShapeBank->setMaxBytesToUpload ((uint32)fMaxBytesToUp);
	// Parse all the waiting instance
	_ShapeBank->processWaitingShapes ();	// Process waiting shapes load shape, texture, and lightmaps
											// and upload all maps to VRAM pieces by pieces
	TWaitingInstancesMMap::iterator wimmIt = _WaitingInstances.begin();
	while( wimmIt != _WaitingInstances.end() )
	{
		CShapeBank::TShapeState st = _ShapeBank->getPresentState (wimmIt->first);
		if (st == CShapeBank::AsyncLoad_Error)
		{
			// Delete the waiting instance - Nobody can be informed of that...
			TWaitingInstancesMMap::iterator	itDel= wimmIt;
			++wimmIt;
			_WaitingInstances.erase(itDel);
		}
		else if (st == CShapeBank::Present)
		{
			// Then create a reference to the shape
			*(wimmIt->second) = _ShapeBank->addRef(wimmIt->first)->createInstance (*this);
			// Delete the waiting instance
			TWaitingInstancesMMap::iterator	itDel= wimmIt;
			++wimmIt;
			_WaitingInstances.erase(itDel);
		}
		else // st == CShapeBank::NotPresent or loading
		{
			++wimmIt;
		}
	}
}

// ***************************************************************************
void	CScene::setDriver(IDriver *drv)
{
	RenderTrav.setDriver(drv);
}

// ***************************************************************************
IDriver	*CScene::getDriver() const
{
	return (const_cast<CScene*>(this))->RenderTrav.getDriver();
}


// ***************************************************************************
// ***************************************************************************
// Shape mgt.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************

void CScene::setShapeBank(CShapeBank*pShapeBank)
{
	_ShapeBank = pShapeBank;
}

// ***************************************************************************

CTransformShape	*CScene::createInstance(const string &shapeName)
{
	// We must attach a bank to the scene (a ShapeBank handle the shape caches and
	// the creation/deletion of the instances)
	nlassert( _ShapeBank != NULL );

	// If the shape is not present in the bank
	if (_ShapeBank->getPresentState( shapeName ) != CShapeBank::Present)
	{
		// Load it from file
		_ShapeBank->load( shapeName );
		if (_ShapeBank->getPresentState( shapeName ) != CShapeBank::Present)
		{
			return NULL;
		}
	}
	// Then create a reference to the shape
	CTransformShape *pTShp = _ShapeBank->addRef( shapeName )->createInstance(*this);
	if (pTShp) pTShp->setDistMax(pTShp->Shape->getDistMax());

	// Look if this instance get lightmap information
#if defined(__GNUC__) && __GNUC__ < 3
	CMeshBase *pMB = (CMeshBase*)((IShape*)(pTShp->Shape));
#else // not GNUC
	CMeshBase *pMB = dynamic_cast<CMeshBase*>((IShape*)(pTShp->Shape));
#endif // not GNUC
	CMeshBaseInstance *pMBI = dynamic_cast<CMeshBaseInstance*>( pTShp );
	if( ( pMB != NULL ) && ( pMBI != NULL ) )
	{
		// Init lightmap information
		pMBI->initAnimatedLightIndex (*this);

		// Auto animations
		//==========================

		if (_AutomaticAnimationSet)
		{
			if (pMB->getAutoAnim())
			{

				std::string animName = toLower(CFile::getFilenameWithoutExtension(shapeName));
				uint animID = _AutomaticAnimationSet->getAnimationIdByName(animName);
				if (animID != CAnimationSet::NotFound)
				{
					CChannelMixer *chanMix = new CChannelMixer;
					chanMix->setAnimationSet((CAnimationSet *) _AutomaticAnimationSet);
					chanMix->setSlotAnimation(0, animID);

					pMBI->registerToChannelMixer(chanMix, "");
					// Gives this object ownership of the channel mixer so we don't need to keep track of it
					pMBI->setChannelMixerOwnerShip(true);
				}
			}
		}
	}

	CLandscapeModel *pLM = dynamic_cast<CLandscapeModel*>( pTShp );
	if( pLM != NULL )
	{
		// Init lightmap information
		pLM->Landscape.initAnimatedLightIndex (*this);
	}

	return pTShp;
}

// ***************************************************************************

void CScene::createInstanceAsync(const string &shapeName, CTransformShape **pInstance, const NLMISC::CVector &position, uint selectedTexture)
{
	// We must attach a bank to the scene (a ShapeBank handle the shape caches and
	// the creation/deletion of the instances)
	nlassert( _ShapeBank != NULL );
	*pInstance = NULL;
	// Add the instance request
	_WaitingInstances.insert(TWaitingInstancesMMap::value_type(shapeName,pInstance));
	// If the shape is not present in the bank
	if (_ShapeBank->getPresentState( shapeName ) != CShapeBank::Present)
	{
		// Load it from file asynchronously
		_ShapeBank->loadAsync( toLower(shapeName), getDriver(), position, NULL, selectedTexture);
	}
}

// ***************************************************************************
void CScene::deleteInstance(CTransformShape *pTrfmShp)
{
	IShape *pShp = NULL;
	if( pTrfmShp == NULL )
		return;

	pShp = pTrfmShp->Shape;

	deleteModel( pTrfmShp );

	if (pShp)
	{
		// Even if model already deleted by smarptr the release function works
		_ShapeBank->release( pShp );
	}

}

// ***************************************************************************
void CScene::animate( TGlobalAnimationTime atTime )
{
	// todo hulud remove
	if (_FirstAnimateCall)
	{
		_InitTime = atTime;
		_RealTime = atTime;
		// dummy value for first frame
		_EllapsedTime = 0.01f ;
		_FirstAnimateCall = false ;
	}
	else
	{
		_EllapsedTime = (float) (atTime - _RealTime);
		//nlassert(_EllapsedTime >= 0);
		if (_EllapsedTime < 0.0f)	// NT WorkStation PATCH (Yes you are not dreaming
			_EllapsedTime = 0.01f;	// deltaTime can be less than zero!!)
		_EllapsedTime = fabsf(_EllapsedTime);
		_RealTime = atTime ;
		_CurrentTime += _EllapsedTime;
	}

	_LMAnimsAuto.animate( atTime );

	// Change PointLightFactors of all pointLights in registered Igs.
	//----------------

	// First list all current AnimatedLightmaps (for faster vector iteration per ig)
	const uint count = (uint)_AnimatedLightPtr.size ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Blend final colors
		_AnimatedLightPtr[i]->updateGroupColors (*this);
	}

	// For all registered igs.
	ItAnimatedIgSet		itAnIgSet;
	for(itAnIgSet= _AnimatedIgSet.begin(); itAnIgSet!=_AnimatedIgSet.end(); itAnIgSet++)
	{
		CInstanceGroup	*ig= *itAnIgSet;

		// Set the light factor
		ig->setPointLightFactor(*this);
	}

	// Rendered part are invalidate
	_RenderedPart = UScene::RenderNothing;

	// Particles are animated later due to dependencies
	_RequestParticlesAnimate = true;
}


// ***************************************************************************
float	CScene::getNbFaceAsked () const
{
	return LoadBalancingTrav.getNbFaceAsked ();
}


// ***************************************************************************
void	CScene::setGroupLoadMaxPolygon(const std::string &group, uint nFaces)
{
	nFaces= max(nFaces, (uint)1);
	LoadBalancingTrav.setGroupNbFaceWanted(group, nFaces);
}
// ***************************************************************************
uint	CScene::getGroupLoadMaxPolygon(const std::string &group)
{
	return LoadBalancingTrav.getGroupNbFaceWanted(group);
}
// ***************************************************************************
float	CScene::getGroupNbFaceAsked (const std::string &group) const
{
	return LoadBalancingTrav.getGroupNbFaceAsked(group);
}



// ***************************************************************************
void	CScene::setPolygonBalancingMode(TPolygonBalancingMode polBalMode)
{
	LoadBalancingTrav.PolygonBalancingMode= (CLoadBalancingGroup::TPolygonBalancingMode)(uint)polBalMode;
}


// ***************************************************************************
CScene::TPolygonBalancingMode	CScene::getPolygonBalancingMode() const
{
	return (CScene::TPolygonBalancingMode)(uint)LoadBalancingTrav.PolygonBalancingMode;
}

// ***************************************************************************
void  CScene::setLayersRenderingOrder(bool directOrder /*= true*/)
{
	RenderTrav.setLayersRenderingOrder(directOrder);
}

// ***************************************************************************
bool  CScene::getLayersRenderingOrder() const
{
	return 	RenderTrav.getLayersRenderingOrder();
}

// ***************************************************************************
CParticleSystemManager &CScene::getParticleSystemManager()
{
	return _ParticleSystemManager;
}

// ***************************************************************************
void	CScene::enableElementRender(UScene::TRenderFilter elt, bool state)
{
	if(state)
		_FilterRenderFlags|= (uint32)elt;
	else
		_FilterRenderFlags&= ~(uint32)elt;
}


// ***************************************************************************
// ***************************************************************************
// Lighting Mgt.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CScene::enableLightingSystem(bool enable)
{
	_LightingSystemEnabled= enable;

	// Set to RenderTrav and LightTrav
	RenderTrav.LightingSystemEnabled= _LightingSystemEnabled;
	LightTrav.LightingSystemEnabled= _LightingSystemEnabled;
}


// ***************************************************************************
void			CScene::setAmbientGlobal(NLMISC::CRGBA ambient)
{
	RenderTrav.AmbientGlobal= ambient;
}
void			CScene::setSunAmbient(NLMISC::CRGBA ambient)
{
	RenderTrav.SunAmbient= ambient;
}
void			CScene::setSunDiffuse(NLMISC::CRGBA diffuse)
{
	RenderTrav.SunDiffuse= diffuse;
}
void			CScene::setSunSpecular(NLMISC::CRGBA specular)
{
	RenderTrav.SunSpecular= specular;
}
void			CScene::setSunDirection(const NLMISC::CVector &direction)
{
	RenderTrav.setSunDirection(direction);
}


// ***************************************************************************
NLMISC::CRGBA	CScene::getAmbientGlobal() const
{
	return RenderTrav.AmbientGlobal;
}
NLMISC::CRGBA	CScene::getSunAmbient() const
{
	return RenderTrav.SunAmbient;
}
NLMISC::CRGBA	CScene::getSunDiffuse() const
{
	return RenderTrav.SunDiffuse;
}
NLMISC::CRGBA	CScene::getSunSpecular() const
{
	return RenderTrav.SunSpecular;
}
NLMISC::CVector	CScene::getSunDirection() const
{
	return RenderTrav.getSunDirection();
}


// ***************************************************************************
void		CScene::setMaxLightContribution(uint nlights)
{
	LightTrav.LightingManager.setMaxLightContribution(nlights);
}
uint		CScene::getMaxLightContribution() const
{
	return LightTrav.LightingManager.getMaxLightContribution();
}

void		CScene::setLightTransitionThreshold(float lightTransitionThreshold)
{
	LightTrav.LightingManager.setLightTransitionThreshold(lightTransitionThreshold);
}
float		CScene::getLightTransitionThreshold() const
{
	return LightTrav.LightingManager.getLightTransitionThreshold();
}


// ***************************************************************************
void		CScene::addInstanceGroupForLightAnimation(CInstanceGroup *ig)
{
	nlassert( ig );
	nlassert( _AnimatedIgSet.find(ig) == _AnimatedIgSet.end() );
	_AnimatedIgSet.insert(ig);
}

// ***************************************************************************
void		CScene::removeInstanceGroupForLightAnimation(CInstanceGroup *ig)
{
	nlassert( ig );
	ItAnimatedIgSet		itIg= _AnimatedIgSet.find(ig);
	if ( itIg != _AnimatedIgSet.end() )
		_AnimatedIgSet.erase(itIg);
}


// ***************************************************************************
void		CScene::setCoarseMeshLightingUpdate(uint8 period)
{
	_CoarseMeshLightingUpdate= max((uint8)1, period);
}


// ***************************************************************************
// ***************************************************************************
/// Weather mgt
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void		CScene::setGlobalWindPower(float gwp)
{
	_GlobalWindPower= gwp;
}
// ***************************************************************************
void		CScene::setGlobalWindDirection(const CVector &gwd)
{
	_GlobalWindDirection= gwd;
	_GlobalWindDirection.z= 0;
	_GlobalWindDirection.normalize();
}


// ***************************************************************************
// ***************************************************************************
/// Private
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CScene::ItSkeletonModelList	CScene::appendSkeletonModelToList(CSkeletonModel *skel)
{
	_SkeletonModelList.push_front(skel);
	return _SkeletonModelList.begin();
}

// ***************************************************************************
void					CScene::eraseSkeletonModelToList(CScene::ItSkeletonModelList	it)
{
	_SkeletonModelList.erase(it);
}

// ***************************************************************************
void					CScene::registerShadowCasterToList(CTransform *sc)
{
	nlassert(sc);
	_ShadowCasterList.push_front(sc);
	sc->_ItShadowCasterInScene= _ShadowCasterList.begin();
}

// ***************************************************************************
void					CScene::unregisterShadowCasterToList(CTransform *sc)
{
	nlassert(sc);
	_ShadowCasterList.erase(sc->_ItShadowCasterInScene);
}


// ***************************************************************************
// ***************************************************************************
// Old CMOT integrated methods
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
set<CScene::CModelEntry>	CScene::_RegModels;


// ***************************************************************************
void	CScene::registerModel(const CClassId &idModel, const CClassId &idModelBase, CTransform* (*creator)())
{
	nlassert(idModel!=CClassId::Null);
	nlassert(creator);
	// idModelBase may be Null...

	CModelEntry		e;
	e.BaseModelId= idModelBase;
	e.ModelId= idModel;
	e.Creator= creator;

	// Insert/replace e.
	_RegModels.erase(e);
	_RegModels.insert(e);
}


// ***************************************************************************
CTransform	*CScene::createModel(const CClassId &idModel)
{
	nlassert(idModel!=CClassId::Null);

	CModelEntry	e;
	e.ModelId= idModel;
	set<CModelEntry>::iterator	itModel;
	itModel= _RegModels.find(e);

	if(itModel==_RegModels.end())
	{
		nlstop;			// Warning, CScene::registerBasics () has not been called !
		return NULL;
	}
	else
	{
		CTransform	*m= (*itModel).Creator();
		if(!m)	return NULL;

		// Set the owner for the model.
		m->_OwnerScene= this;

		// link model to Root in HRC and in clip. NB: if exist!! (case for the Root and RootCluster :) )
		if(Root)
		{
			Root->hrcLinkSon(m);
			Root->clipAddChild(m);
		}

		// Insert the model into the set.
		_Models.insert(m);

		// By default the model is update() in CScene::updateModels().
		m->linkToUpdateList();

		// Once the model is correclty created, finish init him.
		m->initModel();

		// Ensure all the Traversals has enough space for visible list.
		ClipTrav.reserveVisibleList((uint)_Models.size());
		AnimDetailTrav.reserveVisibleList((uint)_Models.size());
		LoadBalancingTrav.reserveVisibleList((uint)_Models.size());
		LightTrav.reserveLightedList((uint)_Models.size());
		RenderTrav.reserveRenderList((uint)_Models.size());

		return m;
	}
}
// ***************************************************************************
void	CScene::deleteModel(CTransform *model)
{
	if(model==NULL)
		return;

	// No model delete during the render
	if (_IsRendering)
	{
		// add ot list of object to delete
		_ToDelete.push_back (model);
		// remove this object from the RenderTrav, hence it won't be displayed
		// still animDetail/Light/LoadBalance it. This is useless, but very rare
		// and I prefer doing like this than to add a NULL ptr Test in each Traversal (which I then must do in CRenderTrav)
		RenderTrav.removeRenderModel(model);
	}
	// standard delete
	else
	{
		set<CTransform*>::iterator	it= _Models.find(model);
		if(it!=_Models.end())
		{
			delete *it;
			_Models.erase(it);
		}
	}
}


// ***************************************************************************
void	CScene::updateModels()
{
	// check all the models which must be checked.
	CTransform	*model= _UpdateModelList;
	CTransform	*next;
	while( model )
	{
		// next to update. get next now, because model->update() may remove model from the list.
		next= model->_NextModelToUpdate;

		// update the model.
		model->update();

		// next.
		model= next;
	}
}


// ***************************************************************************
void	CScene::setLightGroupColor(uint lightmapGroup, NLMISC::CRGBA color)
{
	// If too small, resize with white
	if (lightmapGroup >= _LightGroupColor.size ())
	{
		_LightGroupColor.resize (lightmapGroup+1, CRGBA::White);
	}

	// Set the color
	_LightGroupColor[lightmapGroup] = color;
}


// ***************************************************************************
sint CScene::getAnimatedLightNameToIndex (const std::string &name) const
{
	std::map<std::string, uint>::const_iterator ite = _AnimatedLightNameToIndex.find (name);
	if (ite != _AnimatedLightNameToIndex.end ())
		return (sint)ite->second;
	else
		return -1;
}


// ***************************************************************************
void CScene::setAutomaticAnimationSet(CAnimationSet *as)
{
	// Backup the animation set
	_AutomaticAnimationSet = as;

	// Delete all auto lightmap animations
	_LMAnimsAuto.deleteAll();
	_AnimatedLightNameToIndex.clear();
	_AnimatedLight.clear();
	_AnimatedLightPtr.clear();
	_LightGroupColor.clear();

	// Register each animation as lightmap
	const uint count = _AutomaticAnimationSet->getNumAnimation();
	uint i;
	for (i=0; i<count; i++)
	{
		// Pointer on the animation
		CAnimation *pAnim = _AutomaticAnimationSet->getAnimation(i);

/*		uint nAnimNb;
		// Reset the automatic animation if no animation wanted
		if( pAnim == NULL )
		{
			_AnimatedLight.clear();
			_AnimatedLightPtr.clear();
			_AnimatedLightNameToIndex.clear();
			nAnimNb = _LightmapAnimations.getAnimationIdByName("Automatic");
			if( nAnimNb != CAnimationSet::NotFound )
			{
				CAnimation *anim = _LightmapAnimations.getAnimation( nAnimNb );
				delete anim;
			}
			_LightmapAnimations.reset();
			_LMAnimsAuto.deleteAll();
			return;
		}
		*/

		set<string> setTrackNames;
		pAnim->getTrackNames( setTrackNames );

		// nAnimNb = _LightmapAnimations.addAnimation( "Automatic", pAnim );
		// _LightmapAnimations.build();

		set<string>::iterator itSel = setTrackNames.begin();
		while ( itSel != setTrackNames.end() )
		{
			string ate = *itSel;
			if( strncmp( itSel->c_str(), "LightmapController.", 19 ) == 0 )
			{
				// The light name
				const char *lightName = strrchr ((*itSel).c_str (), '.')+1;

				// Light animation doesn't exist ?
				if (_AnimatedLightNameToIndex.find (lightName) == _AnimatedLightNameToIndex.end())
				{
					// Channel mixer for light anim
					CChannelMixer *cm = new CChannelMixer();
					cm->setAnimationSet( _AutomaticAnimationSet );

					// Add an automatic animation
					_AnimatedLight.push_back ( CAnimatedLightmap ((uint)_LightGroupColor.size ()) );
					_AnimatedLightPtr.push_back ( &_AnimatedLight.back () );
					_AnimatedLightNameToIndex.insert ( std::map<std::string, uint>::value_type (lightName, (uint32)_AnimatedLightPtr.size ()-1 ) );
					CAnimatedLightmap &animLM = _AnimatedLight.back ();
					animLM.setName( *itSel );

					cm->addChannel( animLM.getName(), &animLM, animLM.getValue(CAnimatedLightmap::FactorValue),
						animLM.getDefaultTrack(CAnimatedLightmap::FactorValue), CAnimatedLightmap::FactorValue,
						CAnimatedLightmap::OwnerBit, false);

					// Animated lightmap playlist
					CAnimationPlaylist *pl = new CAnimationPlaylist();
					pl->setAnimation( 0, i );
					pl->setWrapMode( 0, CAnimationPlaylist::Repeat );
					_LMAnimsAuto.addPlaylist(pl,cm);
				}
			}
			++itSel;
		}
	}
}


// ***************************************************************************
// ***************************************************************************
/// Misc
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void					CScene::profileNextRender()
{
	_NextRenderProfile= true;

	// Reset All Stats.
	BenchRes.reset();
}


// ***************************************************************************
void			CScene::setShadowMapTextureSize(uint size)
{
	size= max(size, 2U);
	size= raiseToNextPowerOf2(size);
	_ShadowMapTextureSize= size;
}

// ***************************************************************************
void			CScene::setShadowMapBlurSize(uint bs)
{
	_ShadowMapBlurSize= bs;
}

// ***************************************************************************
void			CScene::enableShadowPolySmooth(bool enable)
{
	RenderTrav.getShadowMapManager().enableShadowPolySmooth(enable);
}

// ***************************************************************************
bool			CScene::getEnableShadowPolySmooth() const
{
	return RenderTrav.getShadowMapManager().getEnableShadowPolySmooth();
}


// ***************************************************************************
void			CScene::setShadowMapDistFadeStart(float dist)
{
	_ShadowMapDistFadeStart= max(0.f, dist);
}
// ***************************************************************************
void			CScene::setShadowMapDistFadeEnd(float dist)
{
	_ShadowMapDistFadeEnd= max(0.f, dist);
}
// ***************************************************************************
void			CScene::setShadowMapMaxCasterInScreen(uint num)
{
	_ShadowMapMaxCasterInScreen= num;
}
// ***************************************************************************
void			CScene::setShadowMapMaxCasterAround(uint num)
{
	_ShadowMapMaxCasterAround= num;
}

// ***************************************************************************
CInstanceGroup *CScene::findCameraClusterSystemFromRay(CInstanceGroup *startClusterSystem,
											   const NLMISC::CVector &startPos, NLMISC::CVector &endPos)
{
	CInstanceGroup	*resultCS= NULL;

	CClipTrav	&clipTrav= getClipTrav();

	// **** Search all cluster where the startPos is in
	static vector<CCluster*> vCluster;
	vCluster.clear();

	bool bInWorld = true;
	clipTrav.Accel.select (startPos, startPos);
	CQuadGrid<CCluster*>::CIterator itAcc = clipTrav.Accel.begin();
	while (itAcc != clipTrav.Accel.end())
	{
		CCluster *pCluster = *itAcc;
		if( pCluster->Group == startClusterSystem &&
			pCluster->isIn (startPos) )
		{
			vCluster.push_back (pCluster);
			bInWorld = false;
		}
		++itAcc;
	}
	if (bInWorld)
	{
		vCluster.push_back (RootCluster);
	}

	// **** Do a traverse starting from each start cluser, clipping the ray instead of the camera pyramid
	uint	i;
	static vector<CCluster*> vClusterVisited;
	vClusterVisited.clear();
	for(i=0;i<vCluster.size();i++)
	{
		vCluster[i]->cameraRayClip(startPos, endPos, vClusterVisited);
	}

	// **** From each cluster, select possible clusterSystem
	static vector<CInstanceGroup*> possibleClusterSystem;
	possibleClusterSystem.clear();
	for(i=0;i<vClusterVisited.size();i++)
	{
		// select only cluster where the EndPos lies in. Important else in landscape, we'll always say
		// that we are in a Son Cluster.
		if(vClusterVisited[i]->isIn(endPos))
		{
			CInstanceGroup	*cs= vClusterVisited[i]->Group;
			// insert if not exist (NB: 1,2 possible clusterSystem so O(N2) is OK)
			uint	j;
			for(j=0;j<possibleClusterSystem.size();j++)
			{
				if(possibleClusterSystem[j]==cs)
					break;
			}
			if(j==possibleClusterSystem.size())
				possibleClusterSystem.push_back(cs);
		}
	}

	// If no cluster found, then we may be in a "Data Error case".
	// In this case, ensure the Camera position in a cluster
	if(possibleClusterSystem.empty())
	{
		CCluster	*bestCluster= NULL;
		float		shortDist= FLT_MAX;

		for(i=0;i<vClusterVisited.size();i++)
		{
			// if the ray is at least partially in this cluster
			CVector		a= startPos;
			CVector		b= endPos;
			if(vClusterVisited[i]->clipSegment(a, b))
			{
				float	dist= (endPos - b).norm();
				if(dist<shortDist)
				{
					bestCluster= vClusterVisited[i];
					shortDist= dist;
				}
			}
		}

		// if found
		if(bestCluster)
		{
			// append the best one to the possible Cluster System
			possibleClusterSystem.push_back(bestCluster->Group);

			// and modify endPos, so the camera will really lies into this cluster
			const float	threshold= 0.05f;
			shortDist+= threshold;
			// must not goes more than startPos!
			float	rayDist= (startPos - endPos).norm();
			shortDist= min(shortDist, rayDist);
			endPos+= (startPos - endPos).normed() * shortDist;
		}
	}

	// NB: still possible that the possibleClusterSystem is empty, if not in any cluster for instance :)


	// **** From each possible clusterSystem, select the one that is the lower in hierarchy
	// common case
	if(possibleClusterSystem.empty())
		resultCS= NULL;
	else if(possibleClusterSystem.size()==1)
	{
		// if it is the rootCluster set NULL (should have the same behavior but do like standard case)
		if(possibleClusterSystem[0]==RootCluster->getClusterSystem())
			resultCS= NULL;
		// set this cluster system
		else
			resultCS= possibleClusterSystem[0];
	}
	// conflict case
	else
	{
		// compute the hierarchy level of each cluster system, take the highest
		CInstanceGroup	*highest= NULL;
		uint			highestLevel= 0;
		for(i=0;i<possibleClusterSystem.size();i++)
		{
			uint	level= 0;
			CInstanceGroup	*ig= possibleClusterSystem[i];
			while(ig)
			{
				ig= ig->getParentClusterSystem();
				level++;
			}
			if(level>=highestLevel)
			{
				highestLevel= level;
				highest= possibleClusterSystem[i];
			}
		}

		// set the highest cluster system
		resultCS= highest;
	}

	return resultCS;
}

// ***************************************************************************
void CScene::renderOcclusionTestMeshsWithCurrMaterial()
{
	for(std::set<CTransform*>::iterator it = _Models.begin(); it != _Models.end(); ++it)
	{
		if ((*it)->isFlare())
		{
			CFlareModel *flare = NLMISC::safe_cast<CFlareModel *>(*it);
			flare->renderOcclusionTestMesh(*RenderTrav.getDriver());
		}
	}
}

// ***************************************************************************
void CScene::renderOcclusionTestMeshs()
{
	nlassert(RenderTrav.getDriver());
	RenderTrav.getDriver()->setupViewport(RenderTrav.getViewport());
	RenderTrav.getDriver()->activeVertexProgram(NULL);
	RenderTrav.getDriver()->activePixelProgram(NULL);
	RenderTrav.getDriver()->activeGeometryProgram(NULL);
	IDriver::TPolygonMode oldPolygonMode = RenderTrav.getDriver()->getPolygonMode();
	CMaterial m;
	m.initUnlit();
	m.setColor(CRGBA(255, 255, 255, 127));
	m.setBlend(true);
	m.setDstBlend(CMaterial::invsrcalpha);
	m.setSrcBlend(CMaterial::srcalpha);
	m.setZWrite(false);
	RenderTrav.getDriver()->setupMaterial(m);
	getDriver()->setPolygonMode(IDriver::Filled);
	renderOcclusionTestMeshsWithCurrMaterial();
	m.setColor(CRGBA::Black);
	RenderTrav.getDriver()->setupMaterial(m);
	getDriver()->setPolygonMode(IDriver::Line);
	renderOcclusionTestMeshsWithCurrMaterial();
	getDriver()->setPolygonMode(oldPolygonMode);
}


// ***************************************************************************
void CScene::updateWaterEnvMaps(TGlobalAnimationTime time)
{
	IDriver *drv = getDriver();
	nlassert(drv);
	if (_WaterEnvMap)
	{
		_WaterEnvMap->update(time, *drv);
	}
}


// ***************************************************************************
void CScene::addSSSModelRequest(const class CSSSModelRequest &req)
{
	_SSSModelRequests.push_back(req);
}

// ***************************************************************************
void CScene::flushSSSModelRequests()
{
	for(uint i=0;i<_SSSModelRequests.size();i++)
	{
		_SSSModelRequests[i].execute();
	}
	_SSSModelRequests.clear();
}


} // NL3D





















