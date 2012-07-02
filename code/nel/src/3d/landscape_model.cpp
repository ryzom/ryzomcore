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

#include <vector>
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/landscape.h"
#include "nel/3d/cluster.h"
#include "nel/3d/scene.h"
#include "nel/3d/light_trav.h"
using namespace std;
using namespace NLMISC;

namespace NL3D
{



// ***************************************************************************
void	CLandscapeModel::registerBasic()
{
	CScene::registerModel(LandscapeModelId, TransformId, CLandscapeModel::creator);
}


// ***************************************************************************
CLandscapeModel::CLandscapeModel()
{
	Landscape.OwnerModel = this;
	Landscape.init();
	_ActiveAdditive=false;
	_Additive=1.f;

	// The model is renderable
	CTransform::setIsRenderable(true);

	// RenderFilter: We are a Landscape
	_RenderFilterType= UScene::FilterLandscape;

	// Mesh support shadow map receiving only
	CTransform::setIsShadowMapReceiver(true);

	_RefineCenterUser= CVector::Null;
	_RefineCenterAuto= true;
}


// ***************************************************************************
void	CLandscapeModel::initModel()
{
	CTransform::initModel();

	// Enable the landscape to be clipped by the Cluster System.
	getOwnerScene()->getRoot()->clipDelChild(this);
	getOwnerScene()->getRootCluster()->clipAddChild(this);

	// After creating the landscape (and so the VegetableManager in the ctor).
	// we must init correclty the VegetableManager.
	Landscape.createVegetableBlendLayersModels(getOwnerScene());
}


// ***************************************************************************
void	CLandscapeModel::traverseHrc()
{
	// Call std method.
	CTransform::traverseHrc();

	/* Compile Animate PointLight here.
		NB: Animated PointLight are important for Dynamic Object lighted by landscape lights.
		The landscape and vegetable are not influenced by thoses animation (as if the light were not animated)
		NB: important to do before the LightTrav. Also even if landscape is hidden, still must animate those light?
	*/
	Landscape.setPointLightFactor(*getOwnerScene());
}


// ***************************************************************************
void	CLandscapeModel::traverseClip()
{
	CClipTrav		&clipTrav= getOwnerScene()->getClipTrav();

	/// if already clipped in this frame, This means that we are visible from 2 CCluster.
	if (_ClipDate == clipTrav.CurrentDate)
	{
		// We may set the WorldFrustumPyramid. But do this only if not already set.
		if(!ClusteredPyramidIsFrustum)
		{
			/* If the current pyramid is different from the last stored pyramid, this mean per exemple
			 that the player is in a room and look the landscape from 2 windows through portals.
			 The other case is that the camera is in 2 clusters, so the clusterSystem start from this 2, and so we arrive
			 here 2 times (but through the same portal => same pyramid obviously...)
			 */
			if( ClusteredPyramid!=clipTrav.WorldPyramid )
			{
				// in this rare case take the Whole frustum pyramid (a more correct code is to make an Union of the 2 pyramids...)
				ClusteredPyramid= clipTrav.WorldFrustumPyramid;
				ClusteredPyramidIsFrustum= true;
			}
		}
	}
	else
	{
		// If the camera is in Rootcluster or visible through a protal from one Room Cluster only, use the clustered pyramid.
		ClusteredPyramid= clipTrav.WorldPyramid;
		// We are not sure we are the FrustumPyramid
		ClusteredPyramidIsFrustum= false;
	}

	// Call normal CTransform traverseClip to insert this model in render list etc...
	// NB: CTransform::traverseClip() update _ClipDate...
	CTransform::traverseClip();
}


// ***************************************************************************
void	CLandscapeModel::clipAndRenderLandscape()
{
	/*
		NB: For best Optimisation, it is important that the Clip act at same time as the Render.
		Why? For complex GPU/CPU synchronisation.

		Since the landscape is rendered at end of the opaque pass, the next clip of the next frame will arise
		too early (if we assume no transparent or out-scene rendering), RESULTING IN A LOCK => A CPU STALL!!

		In a "landscape only" program, this is not easily avoidable. But in common programs with other
		opaque meshs, skinning etc... we'll have lot of CPU/GPU Work between this clip and the Landscape Render.
		And since the clip() lock the VB, and stall, all those tasks won't be parralelized.

		I have already seen such a stall (3 ms).

		Therefore, all Landscape VB Lock arise at the same time=> The GPU will render the landscape while the CPU
		will prepare render and do skinning of the next opaque frame for instance.
	*/

	// Clip
	// ********

	// The real Landscape clip is done here, after std clip
	H_AUTO( NL3D_Landscape_Clip );

	// Should be unlocked
	nlassert (!Landscape.isLocked());

	CClipTrav		&clipTrav= getOwnerScene()->getClipTrav();
	CRenderTrav		&renderTrav= getOwnerScene()->getRenderTrav();

	// Yes, this is ugly, but the clip pass is finished in render(), for clipping TessBlocks.
	// This saves another Landscape patch traversal, so this is faster...
	// Order them in order which clip faster (first horizontal, then vertical).
	// NB: TessBlock are ALWAYS clipped with the frustum pyramid, not the clustered one (faster clip for most common cases).
	CurrentPyramid[0]= clipTrav.WorldFrustumPyramid[NL3D_CLIP_PLANE_LEFT];
	CurrentPyramid[1]= clipTrav.WorldFrustumPyramid[NL3D_CLIP_PLANE_RIGHT];
	CurrentPyramid[2]= clipTrav.WorldFrustumPyramid[NL3D_CLIP_PLANE_TOP];
	CurrentPyramid[3]= clipTrav.WorldFrustumPyramid[NL3D_CLIP_PLANE_BOTTOM];
	nlassert(NL3D_TESSBLOCK_NUM_CLIP_PLANE==4);

	// Before Landscape clip, must setup Driver, for good VB allocation.
	Landscape.setDriver(getOwnerScene()->getRenderTrav().getDriver());

	// get the refineCenter.
	CVector		refineCenter;
	if(_RefineCenterAuto)
		refineCenter= clipTrav.CamPos;
	else
		refineCenter= _RefineCenterUser;

	Landscape.lockBuffers ();

	// Use the Clustered pyramid for Patch, but Frustum pyramid for TessBlocks.
	// We are sure that pyramid has normalized plane normals.
	Landscape.clip(refineCenter, ClusteredPyramid);


	// Render
	// ********

	// Change the landscape center. All Geomorphed pos (in VertexBuffer only or during VertexProgram)
	// subtract this position.
	Landscape.setPZBModelPosition(renderTrav.CamPos);

	/* setup the model matrix
		ZBuffer Precision: set the modelMatrix to the current landscape PZBModelPosition.
		NB: don't use renderTrav.CamPos directly because setPZBModelPosition() may modify the position
	*/
	_RenderWorldMatrix.identity();
	_RenderWorldMatrix.setPos(Landscape.getPZBModelPosition());
	renderTrav.getDriver()->setupModelMatrix(_RenderWorldMatrix);


	// Scene Time/Lighting Mgt.
	CScene		*scene= getOwnerScene();
	nlassert(scene);
	{
		// For vegetable, set the animation Time.
		Landscape.setVegetableTime(scene->getCurrentTime());

		// For vegetable updateLighting, set the system Time.
		Landscape.setVegetableUpdateLightingTime(scene->getCurrentSystemTime());

		// updateLighting
		H_BEFORE( NL3D_Landscape_UpdateLighting );
		Landscape.updateLighting(scene->getCurrentSystemTime());
		H_AFTER( NL3D_Landscape_UpdateLighting );

		// if SceneLighting enabled
		if(scene->isLightingSystemEnabled())
		{
			H_AUTO( NL3D_Landscape_DynamicLighting );

			// For vegetable, set the lighting
			Landscape.setupVegetableLighting(scene->getSunAmbient(), scene->getSunDiffuse(),
				scene->getSunDirection());

			// current visible Dynamic light list are registered in LightTrav::LightingManager
			CLightTrav		&lightTrav= scene->getLightTrav();
			// Get all dynamic light list, and light landscape with it.
			Landscape.computeDynamicLighting(lightTrav.LightingManager.getAllDynamicLightList());
		}
	}

	// First, refine.
	H_BEFORE( NL3D_Landscape_Refine );
	Landscape.refine(refineCenter);
	H_AFTER( NL3D_Landscape_Refine );

	// then render.
	H_BEFORE( NL3D_Landscape_Render );
	Landscape.render(refineCenter, renderTrav.CamLook, CurrentPyramid, isAdditive ());
	H_AFTER( NL3D_Landscape_Render );

	// Should be unlocked by render
	nlassert (!Landscape.isLocked());
}

// ***************************************************************************
void	CLandscapeModel::traverseRender()
{
	CRenderTrav		&renderTRav= getOwnerScene()->getRenderTrav();

	// No-Op. But delay the clip and render to the end of Opaque Rendering. For VBLock optim
	renderTRav.addRenderLandscape(this);

	// If the landscape receive shadow, then add it.
	if(canReceiveShadowMap())
		renderTRav.getShadowMapManager().addShadowReceiver(this);
}

// ***************************************************************************
void	CLandscapeModel::profileRender()
{
	Landscape.profileRender();
}


// ***************************************************************************
// ***************************************************************************
// Dynamic ShadowMaping
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CLandscapeModel::getReceiverBBox(CAABBox &bbox)
{
	// Build a dummy bbox around the TileNear distance.
	bbox.setCenter(Landscape.getOldRefineCenter());
	float	hs= Landscape.getTileNear() * 1.5f; // Enlarge a little because may have some triangles outside this dist.
	bbox.setHalfSize(CVector(hs,hs,hs));
}

// ***************************************************************************
void		CLandscapeModel::receiveShadowMap(CShadowMap *shadowMap, const CVector &casterPos, const CMaterial &shadowMat)
{
	IDriver	*drv= getOwnerScene()->getRenderTrav().getDriver();

	// Setup the matrix. The same than one used for render (ie PZB included)
	drv->setupModelMatrix(_RenderWorldMatrix);

	// render.
	Landscape.receiveShadowMap(drv, shadowMap, casterPos, shadowMat, Landscape.getPZBModelPosition());
}



} // NL3D
