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

#include "nel/misc/types_nl.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/anim_detail_trav.h"
#include "nel/3d/load_balancing_trav.h"
#include "nel/3d/cluster.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/camera.h"
#include "nel/3d/quad_grid_clip_cluster.h"
#include "nel/3d/quad_grid_clip_manager.h"
#include "nel/3d/root_model.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/scene.h"
#include "nel/3d/skeleton_model.h"
#include "nel/misc/fast_floor.h"

using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
CClipTrav::CClipTrav() : ViewPyramid(6), WorldPyramid(6)
{
	_VisibleList.resize(1024);
	_CurrentNumVisibleModels= 0;
	CurrentDate = 0;
	Accel.create (64, 16.0f);

	ForceNoFrustumClip= false;
	_QuadGridClipManager= NULL;
	_TrackClusterVisibility= false;
}

// ***************************************************************************
CClipTrav::~CClipTrav()
{
}

// ***************************************************************************
bool CClipTrav::fullSearch (vector<CCluster*>& vCluster, const CVector& pos)
{
	CQuadGrid<CCluster*>::CIterator itAcc;

	// search with help of Accel
	bool bInWorld = true;
	Accel.select (pos, pos);
	itAcc = Accel.begin();
	while (itAcc != Accel.end())
	{
		CCluster *pCluster = *itAcc;
		if (pCluster->isIn (pos))
		{
			vCluster.push_back (pCluster);
			bInWorld = false;
		}
		++itAcc;
	}

	// if not found at all
	if (bInWorld)
	{
		vCluster.push_back (RootCluster);
	}
	// else must filter to take only the ones that are the lower in the hierarchy (same as in old code)
	else
	{
		uint	i;
		std::set<CCluster*>		parentExclude;

		// for each cluster, avoid its parent
		for(i=0;i<vCluster.size();i++)
		{
			CCluster	*cluster= vCluster[i];
			while(cluster->Father)
			{
				cluster= cluster->Father;
				parentExclude.insert(cluster);
			}
		}

		// reparse to remove clusters presents in the exclude set
		std::vector<CCluster*>::iterator	it;
		for(it=vCluster.begin(); it!=vCluster.end();)
		{
			if(parentExclude.find(*it)!=parentExclude.end())
				it= vCluster.erase(it);
			else
				it++;
		}
	}

	/*
	// Old slow code (up to 5 ms on a 2.4 Ghz, in a big city).
	uint32 i, j;
	for (i = 0; i < pIG->_ClusterInstances.size(); ++i)
	{
		for (j = 0; j < pIG->_ClusterInstances[i]->Children.size(); ++j)
		{
			if (fullSearch (vCluster, pIG->_ClusterInstances[i]->Children[j]->Group, pos))
				return true;
		}
	}

	for (i = 0; i < pIG->_ClusterInstances.size(); ++i)
	{
		if (pIG->_ClusterInstances[i]->isIn(pos))
			vCluster.push_back (pIG->_ClusterInstances[i]);
	}
	if (vCluster.size() > 0)
		return true;*/

	return true;
}

/// Set cluster tracking on/off (ie storage of thje visible cluster during clip traversal)
void CClipTrav::setClusterVisibilityTracking(bool track)
{
	_TrackClusterVisibility = track;
}
/// Check the activation of cluster visibility tracking.
bool CClipTrav::getClusterVisibilityTracking()
{
	return _TrackClusterVisibility;
}
/// Add a visible cluster to the list
void CClipTrav::addVisibleCluster(CCluster *cluster)
{
	_VisibleClusters.push_back(cluster);
}
/** Return the list of cluster visible after the clip traversal
 *	You must activate the cluster tracking to obtain a result.
*/
const std::vector<CCluster*> &CClipTrav::getVisibleClusters()
{
	return _VisibleClusters;
}


// ***************************************************************************
void CClipTrav::traverse()
{
	H_AUTO( NL3D_TravClip );

	// The root must exist
	CTransform	*sceneRoot= Scene->getRoot();

	// Increment the current date of the traversal
	++CurrentDate;
	// Update Clip infos.
	CTravCameraScene::update();

	// Compute pyramid in view basis.
	CVector		pfoc(0,0,0);
	CVector		lb(Left,  Near, Bottom );
	CVector		lt(Left,  Near, Top    );
	CVector		rb(Right, Near, Bottom );
	CVector		rt(Right, Near, Top    );

	CVector		lbFar(Left,  Far, Bottom);
	CVector		ltFar(Left,  Far, Top   );
	CVector		rbFar(Right, Far, Bottom);
	CVector		rtFar(Right, Far, Top   );

	uint32 i, j;

	ViewPyramid[NL3D_CLIP_PLANE_NEAR].make(lt, lb, rt);
	ViewPyramid[NL3D_CLIP_PLANE_FAR].make(lbFar, ltFar, rtFar);

	if(Perspective)
	{
		ViewPyramid[NL3D_CLIP_PLANE_LEFT].make(pfoc, lt, lb);
		ViewPyramid[NL3D_CLIP_PLANE_TOP].make(pfoc, rt, lt);
		ViewPyramid[NL3D_CLIP_PLANE_RIGHT].make(pfoc, rb, rt);
		ViewPyramid[NL3D_CLIP_PLANE_BOTTOM].make(pfoc, lb, rb);
	}
	else
	{
		ViewPyramid[NL3D_CLIP_PLANE_LEFT].make(lt, ltFar, lbFar);
		ViewPyramid[NL3D_CLIP_PLANE_TOP].make(lt, rtFar, ltFar);
		ViewPyramid[NL3D_CLIP_PLANE_RIGHT].make(rt, rbFar, rtFar);
		ViewPyramid[NL3D_CLIP_PLANE_BOTTOM].make(lb, lbFar, rbFar);
	}

	// Compute pyramid in World basis.
	// The vector transformation M of a plane p is computed as p*M-1.
	// Here, ViewMatrix== CamMatrix-1. Hence the following formula.
	for (i = 0; i < 6; i++)
	{
		WorldPyramid[i]= ViewPyramid[i]*ViewMatrix;
	}

	// bkup this pyramid (because this one may be modified by the cluster system).
	WorldFrustumPyramid= WorldPyramid;


	// update the QuadGridClipManager.
	if(_QuadGridClipManager)
	{
		_QuadGridClipManager->updateClustersFromCamera(CamPos);
	}

	H_BEFORE( NL3D_TravClip_ClearLists );

	// Clear the traversals list.
	Scene->getAnimDetailTrav().clearVisibleList();
	Scene->getLoadBalancingTrav().clearVisibleList();
	Scene->getLightTrav().clearLightedList();
	Scene->getRenderTrav().clearRenderList();

	H_AFTER( NL3D_TravClip_ClearLists );


	H_BEFORE( NL3D_TravClip_ResetVisible );

	/* For all objects marked visible in preceding render, reset _Visible state here.
		NB: must reset _Visible State to false because sometimes traverseClip() are even not executed
		(Cluster clip, QuadGridClipManager clip...).
		And somes models read this _Visible state. eg: Skins/StickedObjects test the Visible state of
		their _AncestorSkeletonModel.
	*/
	for (i=0;i<_CurrentNumVisibleModels;i++)
	{
		// if the model still exists (see ~CTransform())
		if( _VisibleList[i] )
		{
			// disable his visibility.
			_VisibleList[i]->_Visible= false;
			// let him know that it is no more in the list.
			_VisibleList[i]->_IndexInVisibleList= -1;
		}
	}
	// Clear The visible List.
	_CurrentNumVisibleModels= 0;
	// Clear the visible cluster list.
	_VisibleClusters.clear();

	H_AFTER( NL3D_TravClip_ResetVisible );

	// Found where is the camera
	//========================

	H_BEFORE( NL3D_TravClip_FindCameraCluster);

	// Found the cluster where the camera is
	static vector<CCluster*> vCluster;

	vCluster.clear();
	sceneRoot->clipDelChild(RootCluster);

	// In which cluster is the camera ?
	CQuadGrid<CCluster*>::CIterator itAcc;
	if (Camera->getClusterSystem() == (CInstanceGroup*)-1)
	{
		fullSearch(vCluster, CamPos);
		for (i = 0; i < vCluster.size(); ++i)
			sceneRoot->clipAddChild(vCluster[i]);
	}
	else
	{
		bool bInWorld = true;
		Accel.select (CamPos, CamPos);
		itAcc = Accel.begin();
		while (itAcc != Accel.end())
		{
			CCluster *pCluster = *itAcc;
			if (pCluster->Group == Camera->getClusterSystem())
			if (pCluster->isIn (CamPos))
			{
				sceneRoot->clipAddChild(pCluster);
				vCluster.push_back (pCluster);
				bInWorld = false;
			}
			++itAcc;
		}

		if (bInWorld)
		{
			sceneRoot->clipAddChild(RootCluster);
			vCluster.push_back (RootCluster);
		}
	}

	/// Flag all cluster to know if they are in camera or not.
	for(i=0;i<vCluster.size();i++)
	{
		vCluster[i]->setCameraIn(true);
	}


	H_AFTER( NL3D_TravClip_FindCameraCluster);

	// Manage Moving Objects
	//=====================

	H_BEFORE( NL3D_TravClip_MovingObjects);

	// Unlink the moving objects from their clusters
	CHrcTrav	&hrcTrav= Scene->getHrcTrav();
	for (i = 0; i < hrcTrav._MovingObjects.size(); ++i)
	{
		CTransformShape *pTfmShp = hrcTrav._MovingObjects[i];

		static vector<CTransform*> vModels;
		vModels.clear();
		uint	numClipParents= pTfmShp->clipGetNumParents();
		for(j=0;j<numClipParents;j++)
		{
			CTransform *pFather = pTfmShp->clipGetParent(j);

			// Does the father is a cluster ??
			if ( pFather->isCluster() )
			{
				vModels.push_back (pFather);
			}
		}
		// Remove me from all clusters
		for (j = 0; j < vModels.size(); ++j)
		{
			vModels[j]->clipDelChild(pTfmShp);
		}
		// Remove me from Root Too
		sceneRoot->clipDelChild(pTfmShp);

		// NB: only the SonsOfAncestorSkeletonModelGroup may still be here.
	}

	// Affect the moving objects to their clusters
	for (i = 0; i < hrcTrav._MovingObjects.size(); ++i)
	{
		CTransformShape *pTfmShp = hrcTrav._MovingObjects[i];

		bool bInWorld = true;
		CAABBox box;
		pTfmShp->getAABBox (box);
		// Transform the box in the world
		CVector c = box.getCenter();
		CVector p = box.getCenter()+box.getHalfSize();
		const CMatrix &wm = pTfmShp->getWorldMatrix();
		c = wm * c;
		p = wm * p;
		float s = (p - c).norm();

		Accel.select (c+CVector(s,s,s), c+CVector(-s,-s,-s));
		itAcc = Accel.begin();
		while (itAcc != Accel.end())
		{
			CCluster *pCluster = *itAcc;
			if (pCluster->Group == pTfmShp->getClusterSystem())
			if (pCluster->isIn (c,s))
			{
				pCluster->clipAddChild(pTfmShp);
				bInWorld = false;
			}
			++itAcc;
		}

		// Moving object in the world -> link to root or to the CQuadGridClipManager.
		if (bInWorld)
		{
			if( _QuadGridClipManager && pTfmShp->isQuadGridClipEnabled() )
			{
				// try to insert in the best cluster of the _QuadGridClipManager.
				if(!_QuadGridClipManager->linkModel(pTfmShp))
					// if fails, link to "root".
					RootCluster->clipAddChild(pTfmShp);
			}
			else
			{
				RootCluster->clipAddChild(pTfmShp);
			}
		}
	}

	H_AFTER( NL3D_TravClip_MovingObjects);

	// Clip the graph.
	//=====================

	H_BEFORE( NL3D_TravClip_Traverse);


	// Traverse the graph.
	sceneRoot->traverseClip();


	// Unlink the cluster where we are
	for (i = 0; i < vCluster.size(); ++i)
	{
		// reset Camera In Flag.
		vCluster[i]->setCameraIn(false);

		// remove from Clip Root
		sceneRoot->clipDelChild(vCluster[i]);
	}

	H_AFTER( NL3D_TravClip_Traverse);

	// Load Balance the Skeleton CLod state here.
	// =========================
	/* Can't do it in LoadBalancingTrav because sons with _AncestorSkeletonModel!=NULL may be hiden if a skeleton
		is displayed in CLod mode.
		So must do it here, then clip all sons of AncestoreSkeletonModelGroup.
	*/
	H_BEFORE( NL3D_TravClip_LoadBalanceCLod);
	loadBalanceSkeletonCLod();
	H_AFTER( NL3D_TravClip_LoadBalanceCLod);

	H_BEFORE( NL3D_TravClip_SkeletonClip);

	// At the end of the clip traverse, must update clip for Objects which have a skeleton ancestor
	// =========================
	// those are linked to the SonsOfAncestorSkeletonModelGroup, so traverse it now.
	if (Scene->SonsOfAncestorSkeletonModelGroup)
		Scene->SonsOfAncestorSkeletonModelGroup->traverseClip();

	// For All Shadow Casters (skeletons + others), clip their ShadowMap possible projection against the frustum only.
	// =========================
	/*
		Done here, because can't do in clip() in case of a Model in a cluster
		(We insert in cluster with the Model BBox, not the Model + Shadow BBox).
	*/
	clipShadowCasters();

	// Update Here the Skin render Lists of All visible Skeletons
	// =========================
	/*
		Done here, because AnimDetail and Render need correct lists. NB: important to do it
		before Render Traversal, because updateSkinRenderLists() may change the transparency flag!!
		NB: can't do it in any traverse() because must be sure that it is done
		(traverseHRC not called if SonOfAncestorSkeletonModel, and traverseClip not called if in a cluster).
		NB: must do even if clipped because:
			1/ maybe used for generateShadow() (through the ancestorSkeletonModel)
			2/ the cost of method is 0 all the time (but when true changes)
	*/
	CScene::ItSkeletonModelList		itSkel;
	for(itSkel= Scene->getSkeletonModelListBegin(); itSkel!=Scene->getSkeletonModelListEnd(); itSkel++)
	{
		CSkeletonModel	*sm= *itSkel;
		sm->updateSkinRenderLists();
	}

	H_AFTER( NL3D_TravClip_SkeletonClip);
}


// ***************************************************************************
void CClipTrav::setQuadGridClipManager(CQuadGridClipManager *mgr)
{
	_QuadGridClipManager= mgr;
}

// ***************************************************************************
void CClipTrav::registerCluster (CCluster* pCluster)
{
	pCluster->AccelIt = Accel.insert (pCluster->getBBox().getMin(), pCluster->getBBox().getMax(), pCluster);
}

// ***************************************************************************
void CClipTrav::unregisterCluster (CCluster* pCluster)
{
	if(!pCluster)
		return;

	Accel.erase(pCluster->AccelIt);

	// just ensure it point to NULL
	pCluster->AccelIt= CQuadGrid<CCluster*>::CIterator();
}


// ***************************************************************************
void CClipTrav::loadBalanceSkeletonCLod()
{
	CScene::ItSkeletonModelList		itSkel;
	_TmpSortSkeletons.clear();

	// **** compute CLod priority of each skeleton,
	for(itSkel= Scene->getSkeletonModelListBegin(); itSkel!=Scene->getSkeletonModelListEnd(); itSkel++)
	{
		CSkeletonModel	*sm= *itSkel;
		float	pr= sm->computeDisplayLodCharacterPriority();
		// If valid priority (CLOd enabled, and skeleton visible)
		if(pr>0)
		{
			sm->setDisplayLodCharacterFlag(false);
			CSkeletonKey key;
			// don't bother OptFastFloor precision.
			key.Priority= OptFastFloor(pr*0xFFFFFF00);
			key.SkeletonModel= sm;
			_TmpSortSkeletons.push_back(key);
		}
	}

	// **** sort by priority in ascending order
	uint	nMaxSkelsInNotCLodForm= Scene->getMaxSkeletonsInNotCLodForm();
	// Optim: need it only if too many skels
	if(_TmpSortSkeletons.size()>nMaxSkelsInNotCLodForm)
	{
		sort(_TmpSortSkeletons.begin(), _TmpSortSkeletons.end());
	}

	// **** set CLod flag for skeletons > setting
	uint	n = min(nMaxSkelsInNotCLodForm, (uint)_TmpSortSkeletons.size());
	for (uint i = n; i < _TmpSortSkeletons.size(); i++)
	{
		_TmpSortSkeletons[i].SkeletonModel->setDisplayLodCharacterFlag(true);
	}

}


// ***************************************************************************
void CClipTrav::reserveVisibleList(uint numModels)
{
	// enlarge only.
	if(numModels>_VisibleList.size())
		_VisibleList.resize(numModels);
}


// ***************************************************************************
struct	CFadeShadowMapSort
{
	CTransform		*Model;
	float			Dist;

	bool	operator<(const CFadeShadowMapSort &o) const
	{
		return Dist<o.Dist;
	}
};

// ***************************************************************************
void	CClipTrav::clipShadowCasters()
{
	H_AUTO( NL3D_TravClip_ShadowCasters );

	CScene::ItShadowCasterList		itShadowCaster;

	float	dFade= NL3D_SMM_FADE_SPEED * Scene->getEllapsedTime();
	float	distFadeStart= Scene->getShadowMapDistFadeStart();
	float	distFadeEnd= Scene->getShadowMapDistFadeEnd();
	float	OODeltaDistFade;
	if(distFadeEnd-distFadeStart>0)
		OODeltaDistFade= 1.0f/(distFadeEnd-distFadeStart);
	else
		OODeltaDistFade= 1000000;
	uint	maxCastInScreen= Scene->getShadowMapMaxCasterInScreen();
	uint	maxCastAround= Scene->getShadowMapMaxCasterAround();


	// **** First select ShadowCasters that are visible or that may have their shadow visible

	// Lod Models that will cast ShadowMaps.
	static std::vector<CFadeShadowMapSort>		aroundCasters;
	static std::vector<CFadeShadowMapSort>		screenCasters;
	aroundCasters.clear();
	screenCasters.clear();

	// For all casters
	for(itShadowCaster= Scene->getShadowCasterListBegin(); itShadowCaster!=Scene->getShadowCasterListEnd(); itShadowCaster++)
	{
		CTransform		*sc= *itShadowCaster;
		nlassert(sc->canCastShadowMap());

		CShadowMap		*shadowMap= sc->getShadowMap();
		nlassert( shadowMap );

		// Binded to an Ancestor skeleton?? If so, don't render since the Ancestor Skeleton render all of his sons
		// Additionally, no-op if this caster is hidden in HRC!!
		if( sc->_AncestorSkeletonModel==NULL && sc->isHrcVisible() )
		{
			bool	visible= false;
			// if we are already visible, then its ok, we either don't need to test.
			if(sc->isClipVisible())
				visible= true;
			// else do a bigger test
			else
			{
				// TODO_SHADOW: Select a better "Caster Sphere".
				// If the model "sc" is a CSkeletonModel, It depends also on Sticked objects/Skeletons.
				// Build the sphere around the caster that can receive shadow.
				CBSphere	sphere;
				// Suppose an Object sphere of 3 meter radius, centered on caster Pos.
				const	float	objectRadius= 3.f;
				sphere.Center= sc->getWorldMatrix().getPos();
				// Add to this sphere the max Depth extent.
				// NB: works because suppose that the Shadow BBox include the model Center.
				sphere.Radius= objectRadius + sc->getShadowMapMaxDepth();

				// Clip This sphere against the Frustum.
				visible= true;
				for(uint i=0;i<WorldFrustumPyramid.size();i++)
				{
					// if SpherMax OUT return false.
					float	d= WorldFrustumPyramid[i]*sphere.Center;
					if(d>sphere.Radius)
					{
						visible= false;
						break;
					}
				}
			}

			// If the ShadowMap is visible, add it to the List Of ShadowMap to Render.
			if(visible)
				Scene->getRenderTrav().getShadowMapManager().addShadowCaster(sc);

			// update Fading.
			CFadeShadowMapSort	fsms;
			fsms.Model= sc;
			fsms.Dist= (sc->getWorldMatrix().getPos() - CamPos).norm();
			// lod In and Out.
			aroundCasters.push_back(fsms);
			if(visible)
				screenCasters.push_back(fsms);
			// Compute normal Distance fading.
			shadowMap->DistanceFade= (fsms.Dist-distFadeStart)*OODeltaDistFade;
			// Increment the TemporalFadeOut by default  (see below)
			shadowMap->TemporalOutScreenFade+= dFade;
			// if !visible, increment the InScreenFadeAccum
			if(!visible)
				shadowMap->InScreenFadeAccum+= dFade;
		}
		// If the model is not visible, or if temporary bound to a AncestorSkeletonModel
		else
		{
			// Increment the TemporalFadeOut. Because since will be hidden (or sticked)
			// for a long time, allow the process to free texture.
			shadowMap->TemporalOutScreenFade+= dFade;
			// Since not visible, increment the InScreenFadeAccum
			shadowMap->InScreenFadeAccum+= dFade;
		}
	}

	// **** Load Balance Models that cast ShadowMaps around,

	sort(aroundCasters.begin(), aroundCasters.end());
	sort(screenCasters.begin(), screenCasters.end());

	// For All Around nearest Casters, decrement their fadeOutScreen
	uint	numNearCast= min((uint)aroundCasters.size(), maxCastAround);
	uint	i;
	for(i=0;i<numNearCast;i++)
	{
		CTransform	*model= aroundCasters[i].Model;
		CShadowMap	*shadowMap= model->getShadowMap();
		// Decrement the fade. *2 because default incremented above.
		shadowMap->TemporalOutScreenFade-= 2*dFade;
	}

	// For InScreen Casters, increment or decrement their fadeInScreen. Also resolve InScreenFadeAccum.
	numNearCast= min((uint)screenCasters.size(), maxCastInScreen);
	// nearest: decrement fadeInScreen, and remove accum
	for(i=0;i<numNearCast;i++)
	{
		CShadowMap	*shadowMap= screenCasters[i].Model->getShadowMap();
		// Decrement the fade.
		shadowMap->TemporalInScreenFade-= dFade + shadowMap->InScreenFadeAccum;
		// Since visible and resolved, reset accum
		shadowMap->InScreenFadeAccum= 0;
	}
	// farthest: increment fadeInScreen, and append accum
	for(i=numNearCast;i<screenCasters.size();i++)
	{
		CShadowMap	*shadowMap= screenCasters[i].Model->getShadowMap();
		// Increment the fade.
		shadowMap->TemporalInScreenFade+= dFade + shadowMap->InScreenFadeAccum;
		// Since visible and resolved, reset accum
		shadowMap->InScreenFadeAccum= 0;
	}


	// clamp values, and release texture where appropriated
	for(itShadowCaster= Scene->getShadowCasterListBegin(); itShadowCaster!=Scene->getShadowCasterListEnd(); itShadowCaster++)
	{
		CTransform	*sc= *itShadowCaster;
		nlassert(sc->canCastShadowMap());

		CShadowMap		*shadowMap= sc->getShadowMap();
		shadowMap->processFades();
	}

	// **** Use the rendered Skeletons ShadowMap to select the Ones that will be generated this Frame.
	Scene->getRenderTrav().getShadowMapManager().selectShadowMapsToGenerate(Scene);


	/* **** Then for All ShadowCasters not visibles but that will generate their shadowMap,
		- we must compute the LightTraversal(for ShadowLight direction)
		- for skeleton models only, we must compute the AnimDetailTraversal (for bone animation)
		We MUST NOT flag the skeleton as Visible, and we MUST NOT insert in LoadBalancing
		(since won't be rendered)
		NB: Do nothing for Sons of the Ancestor Skeleton because:
			1/ light do nothing with them (see std clip)
			2/ animDetail will be called recursively (see std clip and CSkeletonModel::traverseAnimDetail())
	*/
	for(itShadowCaster= Scene->getShadowCasterListBegin(); itShadowCaster!=Scene->getShadowCasterListEnd(); itShadowCaster++)
	{
		CTransform	*sc= *itShadowCaster;
		// compute its shadowMap this frame? and not visible?
		if(sc->isGeneratingShadowMap() && !sc->isClipVisible() )
		{
			nlassert(sc->_AncestorSkeletonModel==NULL);
			// Add it only to the lightTrav
			if( sc->isLightable() )
				Scene->getLightTrav().addLightedModel(sc);
			// If it is a skeleton, add it also to the anim detail
			if( sc->isSkeleton() && sc->isAnimDetailable() )
				Scene->getAnimDetailTrav().addVisibleModel(sc);
		}
	}
}


}
