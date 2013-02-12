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

#include "nel/3d/transform.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/scene.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/root_model.h"
#include "nel/3d/u_transform.h"
#include "nel/misc/fast_floor.h"
#include "nel/misc/hierarchical_timer.h"


using namespace NLMISC;
using namespace std;

namespace	NL3D
{


// ***************************************************************************
#define	NL3D_TRANSFORM_DEFAULT_SHADOW_MAP_DEPTH		8.f


// ***************************************************************************
void	CTransform::registerBasic()
{
	CScene::registerModel( TransformId, 0, CTransform::creator);
}


// ***************************************************************************
CTransform::CTransform()
{
	// important to reset for destructor to know if linked or not (CCluster !!)
	_OwnerScene= NULL;

	// Hrc/Graph hierarchy
	_HrcParent= NULL;
	_HrcParentUnfreeze= NULL;

	_PrecModelToUpdate= NULL;
	_NextModelToUpdate= NULL;

	_TransformDirty= true;

	Visibility= CHrcTrav::Herit;

	_LastTransformableMatrixDate= 0;

	_FatherSkeletonModel= NULL;

	_ClusterSystem = NULL;

	_FreezeHRCState= FreezeHRCStateDisabled;

	_OrderingLayer = 2;

	_TransparencyPriority = 0;


	// No logicInfo by default
	_LogicInfo= NULL;

	_ForceCLodSticked= false;

	// default MeanColor value
	_MeanColor.set(255,255,255,255);

	// Default ShadowMap direction
	_ShadowMapDirectionZThreshold= -0.5f;
	_ShadowMapMaxDepth= NL3D_TRANSFORM_DEFAULT_SHADOW_MAP_DEPTH;

	// Setup some state.

	/*
		Default are:
			IsAnimDetailable= 0
			IsLoadBalancable= 0
			IsLightable= 0
			IsRenderable= 0
			IsTransparent= 0
			IsOpaque= 1
			QuadGridClipEnabled= 0.

			IsUserLightable= 1			// default, the model may be lighted.
			IsFinalLightable= 0
			IsNeedUpdateLighting= 0
			ISNeedUpdateFrozenStaticLightSetup= 0

			IsSkeleton= 0
			IsTransformShape=0
			IsCluster= 0
			IsMeshBaseInstance= 0

			IsDeleteChannelMixer = 0;
	*/
	_StateFlags= IsOpaque | IsUserLightable;

	// By default, always allow rendering of Transform Models.
	_RenderFilterType= std::numeric_limits<uint32>::max();

	// By default, don't suport fast intersection detection
	_SupportFastIntersect= false;


	// **** HRC Init Traversal Computed Data.
	_LocalVis= CHrcTrav::Herit; _LocalMatrix.identity(); _LocalDate=0;
	_WorldVis= true; _WorldMatrix.identity();
	// Init the _WorldDate to -1 so at first pass, _LocalDate>_WorldDate, and so
	// the model will be processed and so it'll may be inserted in LightingManager (for example)
	_WorldDate=-1;
	_Frozen = false;
	_DontUnfreezeChildren = false;
	_AncestorSkeletonModel= NULL;
	_ClipLinkedInSonsOfAncestorSkeletonModelGroup= false;

	// **** Clip Init Traversal Computed Data.
	_ClipDate= 0;
	_Visible=false;
	_IndexInVisibleList= -1;

	// **** AnimDetail Init Traversal Computed Data.
	// none

	// **** LoadBalancing Init Traversal Computed Data.
	_LoadBalancingGroup= NULL;
}


// ***************************************************************************
CTransform::~CTransform()
{
	// If still binded to a father skeleton
	if( _FatherSkeletonModel )
	{
		/* If skinned, cannot detach me from skeleton here because detachSkeletonSon()
			use some virtual calls of transform: setApplySkin().
			Hence, It is the deriver job to detach himself from the skeleton.

			NB: test isSkinned(), not isSkinnable(), since isSkinned() is not virtual ....
			This means that if a Mesh isSkinnable(), but never skinned, it is not asserted here.
		*/
		if( isSkinned() )
		{
			nlstop;
		}
		else
			// Can detach Me. Important for UTransform sticked
			_FatherSkeletonModel->detachSkeletonSon(this);
	}

	// resetLighting, removing me from PointLight Transform list.
	// NB: not done for FrozenStaticLightSetup, because those lights don't owns me.
	resetLighting();

	// Must also remove me from the lightingManager.
	// must test getOwnerScene() because of CCluster usage out of CScene (thanks to mat!! :) )
	if(getOwnerScene())
	{
		CLightTrav	&lightTrav= getOwnerScene()->getLightTrav();
		_LightedModelIt= lightTrav.LightingManager.eraseStaticLightedModel(_LightedModelIt);
	}

	if (getChannelMixerOwnerShip()) delete (CChannelMixer *) _ChannelMixer;

	// ensure the model is no more linked to the UpdateList.
	unlinkFromUpdateList();

	// I must remove me from _VisibleList.
	if(_IndexInVisibleList>=0)
	{
		CClipTrav	&clipTrav= getOwnerScene()->getClipTrav();
		nlassert(_IndexInVisibleList < (sint)clipTrav._CurrentNumVisibleModels );
		// Mark NULL. NB: faster than a CRefPtr.
		clipTrav._VisibleList[_IndexInVisibleList]= NULL;
		_IndexInVisibleList= -1;
	}

	// remove me from parents in Hrc and Clip
	setStateFlag(ForceClipRoot, false); // ensure that not 'glued' to the root so that the following call will succeed
	hrcUnlink();
	clipUnlinkFromAll();

	// remove mys sons.
	while(hrcGetNumChildren())
	{
		hrcGetChild(0)->hrcUnlink();
	}
	while(clipGetNumChildren())
	{
		clipDelChild(clipGetChild(0));
	}

	nlassert(_HrcSons.empty());
	nlassert(_HrcParent==NULL);
	nlassert(_ClipSons.empty());
	nlassert(_ClipParents.empty());
}


// ***************************************************************************
void	CTransform::initModel()
{
	// assign me to the default group
	_LoadBalancingGroup= getOwnerScene()->getLoadBalancingTrav().getDefaultGroup();
}



// ***************************************************************************
void		CTransform::hide()
{
	// Optim: do nothing if already set
	if(Visibility!= CHrcTrav::Hide)
	{
		_TransformDirty= true;
		Visibility= CHrcTrav::Hide;
		// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
		if(isSkinned())
		{
			nlassert(_FatherSkeletonModel);
			_FatherSkeletonModel->dirtSkinRenderLists();
		}
	}
}

// ***************************************************************************
void		CTransform::setTransparency(bool v)
{
	bool bTmp = getStateFlag(IsTransparent) == 0 ? false : true;
	if (bTmp != v)
	{
		setStateFlag(IsTransparent, v);
		if(isSkinned())
		{
			nlassert(_FatherSkeletonModel);
			_FatherSkeletonModel->dirtSkinRenderLists();
		}
	}
}

// ***************************************************************************
void CTransform::setBypassLODOpacityFlag(bool bypass)
{
	setStateFlag(BypassLODOpacity, bypass);
}

// ***************************************************************************
void		CTransform::setOpacity(bool v)
{
	bool bTmp = getStateFlag(IsOpaque) == 0 ? false : true;
	if (bTmp != v)
	{
		setStateFlag(IsOpaque, v);
		if(isSkinned())
		{
			nlassert(_FatherSkeletonModel);
			_FatherSkeletonModel->dirtSkinRenderLists();
		}
	}
}


// ***************************************************************************
void		CTransform::show()
{
	// Optim: do nothing if already set
	if(Visibility!= CHrcTrav::Show)
	{
		_TransformDirty= true;
		Visibility= CHrcTrav::Show;
		// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
		if(isSkinned())
		{
			nlassert(_FatherSkeletonModel);
			_FatherSkeletonModel->dirtSkinRenderLists();
		}
	}
}
// ***************************************************************************
void		CTransform::heritVisibility()
{
	// Optim: do nothing if already set
	if(Visibility!= CHrcTrav::Herit)
	{
		_TransformDirty= true;
		Visibility= CHrcTrav::Herit;
		// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
		if(isSkinned())
		{
			nlassert(_FatherSkeletonModel);
			_FatherSkeletonModel->dirtSkinRenderLists();
		}
	}
}


// ***************************************************************************
CTrackDefaultVector		CTransform::DefaultPos( CVector::Null );
CTrackDefaultVector		CTransform::DefaultRotEuler( CVector::Null );
CTrackDefaultQuat		CTransform::DefaultRotQuat( NLMISC::CQuat::Identity );
CTrackDefaultVector		CTransform::DefaultScale( CVector(1,1,1) );
CTrackDefaultVector		CTransform::DefaultPivot( CVector::Null );

ITrack* CTransform::getDefaultTrack (uint valueId)
{
	// Cyril: prefer do it here in CTransform, because of CCamera, CLight etc... (which may not need a default value too!!)

	// what value ?
	switch (valueId)
	{
	case PosValue:			return &DefaultPos;
	case RotEulerValue:		return &DefaultRotEuler;
	case RotQuatValue:		return &DefaultRotQuat;
	case ScaleValue:		return &DefaultScale;
	case PivotValue:		return &DefaultPivot;
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getDefaultTrack(valueId);

	return NULL;

}

// ***************************************************************************
void	CTransform::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	if (getChannelMixerOwnerShip() && chanMixer != _ChannelMixer)
	{
		delete _ChannelMixer;
		setChannelMixerOwnerShip(false);
	}

	// Hey!! we are animated!!
	_ChannelMixer= chanMixer;

	// Update flag, if we must be inserted in AnimDetail
	setStateFlag(IsAnimDetailable, _ChannelMixer || getStateFlag(IsForceAnimDetail) );

	// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
	if(isSkinned())
	{
		nlassert(_FatherSkeletonModel);
		_FatherSkeletonModel->dirtSkinRenderLists();
	}

	// For CTransfom, channels are not detailled.
	addValue(chanMixer, PosValue, OwnerBit, prefix, false);
	addValue(chanMixer, RotEulerValue, OwnerBit, prefix, false);
	addValue(chanMixer, RotQuatValue, OwnerBit, prefix, false);
	addValue(chanMixer, ScaleValue, OwnerBit, prefix, false);
	addValue(chanMixer, PivotValue, OwnerBit, prefix, false);

	// Deriver note: if necessary, call	BaseClass::registerToChannelMixer(chanMixer, prefix);
}


// ***************************************************************************
void			CTransform::freeze()
{
	// First, update the model
	// _Frozen state is disabled here (in CTransform::update()).
	update();

	// Then flag the frozen state.
	_Frozen= true;
}

// ***************************************************************************
void			CTransform::setDontUnfreezeChildren(bool val)
{
	_DontUnfreezeChildren = val;
}


// ***************************************************************************
void		CTransform::freezeHRC()
{
	// if disabled, say we are ready to validate our worldMatrix for long.
	if(_FreezeHRCState==FreezeHRCStateDisabled)
	{
		_FreezeHRCState= FreezeHRCStateRequest;
		setStateFlag(QuadGridClipEnabled, true);

		/* If the transform is not frozen (ie staticaly inserted in a cluster),
			We must be sure it will be tested against QuadGridClipManager at next ClipTrav pass.
			=> must make this object a "moving object" at next render=> dirt _LocalMatrixDate.
		*/
		if(!_Frozen)
		{
			_TransformDirty= true;
		}
	}
}


// ***************************************************************************
void		CTransform::unfreezeHRC()
{
	// if this model is no HRC frozen disabled
	if(_FreezeHRCState!=FreezeHRCStateDisabled)
	{
		// if model correctly frozen.
		if(_FreezeHRCState == CTransform::FreezeHRCStateEnabled )
		{
			// Should not be linked : can't link after a freezeHRC
			nlassert (_HrcParent == NULL);

			// Set as unfreeze else, hrcLinkSon doesn't work
			_FreezeHRCState= FreezeHRCStateDisabled;

			// Link this model to the previous HRC parent.
			if (_HrcParentUnfreeze)
				_HrcParentUnfreeze->hrcLinkSon( this );
			else
				getOwnerScene()->getRoot()->hrcLinkSon( this );

			// Link this object to the validateList.
			linkToUpdateList();

			// if lightable()
			if( isLightable() )
			{
				CLightTrav	&lightTrav= getOwnerScene()->getLightTrav();
				// Lighting: must remove the object from the quadGrid.
				// NB: works if _LightedModelIt==NULL. result is that _LightedModelIt= NULL.
				_LightedModelIt= lightTrav.LightingManager.eraseStaticLightedModel(_LightedModelIt);
			}

		}
		else
			_FreezeHRCState= FreezeHRCStateDisabled;

		// unlink me from any QuadCluster, and disable QuadCluster
		unlinkFromQuadCluster();
		setStateFlag(QuadGridClipEnabled, false);
	}
}


// ***************************************************************************
void		CTransform::update()
{
	// test if the matrix has been changed in ITransformable.
 	if(ITransformable::compareMatrixDate(_LastTransformableMatrixDate))
	{
		_LastTransformableMatrixDate= ITransformable::getMatrixDate();
		_TransformDirty= true;
	}

	// update the freezeHRC state.
	if(_FreezeHRCState != CTransform::FreezeHRCStateDisabled)
	{
		// if the model request to be frozen in HRC
		if(_FreezeHRCState == CTransform::FreezeHRCStateRequest )
		{
			// Wait for next Hrc traversal to compute good _WorldMatrix for this model and his sons.
			// Also, next Hrc traversal will insert the model in the LightingManager quadGrid (if lightable)
			_FreezeHRCState = CTransform::FreezeHRCStateReady;
		}
		// if the model is ready to be frozen in HRC, then do it!!
		else if( _FreezeHRCState == CTransform::FreezeHRCStateReady )
		{
			// Unlink this model.
			hrcUnlink();

			// unLink this object from the validateList. NB: the list will still be correclty parsed.
			unlinkFromUpdateList();

			// if lightable, the model is inserted in a quadgrid to update his lighting only when
			// dynamicLights touch him (since himself is static).
			if( isLightable() )
			{
				CLightTrav	&lightTrav= getOwnerScene()->getLightTrav();
				// Lighting: must reinsert the object from the quadGrid.
				// NB: works if _LightedModelIt==NULL. result is that _LightedModelIt= NULL.
				_LightedModelIt= lightTrav.LightingManager.eraseStaticLightedModel(_LightedModelIt);
				// insert in the quadgrid.
				_LightedModelIt= lightTrav.LightingManager.insertStaticLightedModel(this);
			}

			// Now this model won't be tested for validation nor for worldMatrix update. End!!
			_FreezeHRCState = CTransform::FreezeHRCStateEnabled;
		}
	}

	// update _LocalMatrix
	if(_TransformDirty)
	{
		// update the local matrix.
		_LocalMatrix= getMatrix();
		_LocalVis= Visibility;
		// update the date of the local matrix.
		_LocalDate= getOwnerScene()->getHrcTrav().CurrentDate;

		// The transform has been modified. Hence, it is no more frozen.
		_Frozen= false;

		// ok!
		_TransformDirty= false;
	}
}


// ***************************************************************************
void	CTransform::getAABBox(NLMISC::CAABBox &bbox) const
{
	bbox.setCenter(CVector::Null);
	bbox.setHalfSize(CVector::Null);
}


// ***************************************************************************
void	CTransform::setLoadBalancingGroup(const std::string &group)
{
	// Get the traversal.
	CLoadBalancingTrav	&trav= getOwnerScene()->getLoadBalancingTrav();
	// get the group from trav (create if needed), and set it.
	_LoadBalancingGroup= trav.getOrCreateGroup(group);
}


// ***************************************************************************
const std::string &CTransform::getLoadBalancingGroup() const
{
	// get the group name
	return _LoadBalancingGroup->Name;
}


// ***************************************************************************
void		CTransform::setMeanColor(CRGBA color)
{
	// if the color is different from prec
	if(color!=_MeanColor)
	{
		// change it.
		_MeanColor= color;
	}
}


// ***************************************************************************
void		CTransform::setIsLightable(bool val)
{
	setStateFlag(IsLightable, val);
	// update IsFinalLightable
	setStateFlag(IsFinalLightable, (getStateFlag(IsLightable) && getStateFlag(IsUserLightable)) );
}
// ***************************************************************************
void		CTransform::setUserLightable(bool enable)
{
	setStateFlag(IsUserLightable, enable);
	// update IsFinalLightable
	setStateFlag(IsFinalLightable, (getStateFlag(IsLightable) && getStateFlag(IsUserLightable)) );
}


// ***************************************************************************
void		CTransform::setIsRenderable(bool val)
{
	setStateFlag(IsRenderable, val);
}

// ***************************************************************************
void		CTransform::setIsBigLightable(bool val)
{
	setStateFlag(IsBigLightable, val);
}
// ***************************************************************************
void		CTransform::setIsSkeleton(bool val)
{
	setStateFlag(IsSkeleton, val);
}
// ***************************************************************************
void		CTransform::setApplySkin(bool state)
{
	setStateFlag(IsSkinned, state);
}

// ***************************************************************************
void		CTransform::setIsForceAnimDetail(bool val)
{
	setStateFlag(IsForceAnimDetail, val );

	// Update flag, if we must be inserted in AnimDetail
	setStateFlag(IsAnimDetailable, _ChannelMixer || getStateFlag(IsForceAnimDetail) );

	// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
	if(isSkinned())
	{
		nlassert(_FatherSkeletonModel);
		_FatherSkeletonModel->dirtSkinRenderLists();
	}
}
// ***************************************************************************
void		CTransform::setIsLoadbalancable(bool val)
{
	setStateFlag(IsLoadBalancable, val );
}


// ***************************************************************************
void		CTransform::linkToUpdateList()
{
	if(!_OwnerScene)
		return;

	// If the model is not already inserted.
	if( ! (_PrecModelToUpdate!=NULL  ||  _OwnerScene->_UpdateModelList==this) )
	{
		// insert it.
		_NextModelToUpdate= _OwnerScene->_UpdateModelList;
		_PrecModelToUpdate= NULL;
		if(_NextModelToUpdate)
			_NextModelToUpdate->_PrecModelToUpdate= this;
		_OwnerScene->_UpdateModelList= this;
	}
}

// ***************************************************************************
void		CTransform::unlinkFromUpdateList()
{
	if(!_OwnerScene)
		return;

	// If the model is inserted.
	if( _PrecModelToUpdate!=NULL  ||  _OwnerScene->_UpdateModelList==this )
	{
		// update prec.
		if(_PrecModelToUpdate)
			_PrecModelToUpdate->_NextModelToUpdate= _NextModelToUpdate;
		else
			_OwnerScene->_UpdateModelList= _NextModelToUpdate;

		// update next.
		if(_NextModelToUpdate)
			_NextModelToUpdate->_PrecModelToUpdate= _PrecModelToUpdate;

		// End.
		_PrecModelToUpdate= NULL;
		_NextModelToUpdate= NULL;
	}
}


// ***************************************************************************
// ***************************************************************************
// Hrc Trav
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CTransform::updateWorld()
{
	const	CMatrix		*pFatherWM;
	bool				visFather;

	// If not root case, link to Fahter.
	if(_HrcParent)
	{
		pFatherWM= &(_HrcParent->_WorldMatrix);
		visFather= _HrcParent->_WorldVis;

		// if _HrcParent is not frozen (for any reason), disable us!

		if (!_HrcParent->_Frozen && !_HrcParent->_DontUnfreezeChildren)
			_Frozen= false;

		// herit _AncestorSkeletonModel
		if (_HrcParent->_AncestorSkeletonModel)
			// If my father has an _AncestorSkeletonModel, get it.
			_AncestorSkeletonModel= _HrcParent->_AncestorSkeletonModel;
		else
			// else I have an ancestor skel model if I am sticked/binded directly to a skeleton model.
			_AncestorSkeletonModel= _FatherSkeletonModel;
	}
	// else, default!!
	else
	{
		pFatherWM= &(CMatrix::Identity);
		visFather= true;

		// at the root of the hierarchy, we have no parent, hence no FatherSkeletonModel nor _AncestorSkeletonModel.
		_AncestorSkeletonModel= NULL;

		// NB: Root is Frozen by essence :), so don't modify the frozen state here.
	}

	// Combine matrix
	if(_LocalDate>_WorldDate || (_HrcParent && _HrcParent->_WorldDate>_WorldDate) )
	{
		// Must recompute the world matrix.  ONLY IF I AM NOT SKINNED/STICKED TO A SKELETON in the hierarchy!
		if( _AncestorSkeletonModel==NULL )
		{
			_WorldMatrix=  *pFatherWM * _LocalMatrix;
			_WorldDate= getOwnerScene()->getHrcTrav().CurrentDate;

			// Add the model to the moving object list, only if I am a transform shape
			if (!_Frozen && isTransformShape() && !getStateFlag(ForceClipRoot))
				getOwnerScene()->getHrcTrav()._MovingObjects.push_back (static_cast<CTransformShape*>(this));
		}
	}

	// Update dynamic lighting.
	/*
		If the model is not frozen in StaticLight, then must update lighting each frame.
		Even if the object doesn't move, a new dynamic light may enter in its aera. Hence we must test
		it in the light quadrid. StaticLight-ed Objects don't need it because they are inserted in a special quadgrid,
		where dynamics lights touch all StaticLight-ed object to force their computing

		NB: not done if _AncestorSkeletonModel!=NULL. no need because  in this case,
		result is driven by the _LightContribution of the _AncestorSkeletonModel.
	*/
	if( !_LightContribution.FrozenStaticLightSetup && _AncestorSkeletonModel==NULL )
	{
		// if the model is lightable reset lighting
		if( isLightable() )
			resetLighting();
	}

	// Combine visibility.
	switch(_LocalVis)
	{
		case CHrcTrav::Herit: _WorldVis= visFather; break;
		case CHrcTrav::Hide: _WorldVis= false; break;
		case CHrcTrav::Show: _WorldVis= true; break;
		default: break;
	}


	// If I have an ancestor Skeleton Model, I must be binded in ClipTrav to the SonsOfAncestorSkeletonModelGroup
	updateClipTravForAncestorSkeleton();

}


// ***************************************************************************
void	CTransform::updateClipTravForAncestorSkeleton()
{
	// If I have an ancestor Skeleton Model, I must be binded in ClipTrav to the SonsOfAncestorSkeletonModelGroup
	if(_AncestorSkeletonModel && !_ClipLinkedInSonsOfAncestorSkeletonModelGroup)
	{
		// must unlink from ALL olds models.
		clipUnlinkFromAll();

		// And link to SonsOfAncestorSkeletonModelGroup.
		getOwnerScene()->SonsOfAncestorSkeletonModelGroup->clipAddChild(this);

		// update the flag.
		_ClipLinkedInSonsOfAncestorSkeletonModelGroup= true;
	}


	// else I must be binded to the standard Root.
	if(!_AncestorSkeletonModel && _ClipLinkedInSonsOfAncestorSkeletonModelGroup)
	{
		// verify first I am really still linked to the SonsOfAncestorSkeletonModelGroup.
		// This test is important, because link may have changed for any reason (portals, clipManager....).
		if( clipGetNumParents() == 1 && clipGetParent(0)==getOwnerScene()->SonsOfAncestorSkeletonModelGroup )
		{
			// must unlink from ALL olds models.
			clipUnlinkFromAll();
			// and now, link to std root.
			getOwnerScene()->getRoot()->clipAddChild(this);
		}

		// update the flag
		_ClipLinkedInSonsOfAncestorSkeletonModelGroup= false;
	}
}


// ***************************************************************************
void	CTransform::traverseHrc()
{
	// Recompute the matrix, according to _HrcParent matrix mode, and local matrix.
	updateWorld();

	// Traverse the Hrc sons.
	uint	num= hrcGetNumChildren();
	for(uint i=0;i<num;i++)
		hrcGetChild(i)->traverseHrc();
}


// ***************************************************************************
// ***************************************************************************
// Clip Trav
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CTransform::setClusterSystem(CInstanceGroup *pCS)
{
	if (pCS != NULL)
	{
		nlassert(!getStateFlag(ForceClipRoot)); // the transform must be linked to the root, and have not cluster system when this flag is set
	}
	// Special case for the "AutoClusterSystem" when pCS==-1
	if(pCS==(CInstanceGroup*)-1)
	{
		_ClusterSystem = NULL;
		setStateFlag(ClusterSystemAuto, true);
	}
	else
	{
		_ClusterSystem = pCS;
		setStateFlag(ClusterSystemAuto, false);
	}
}

// ***************************************************************************
CInstanceGroup*		CTransform::getClusterSystem ()
{
	if(getStateFlag(ClusterSystemAuto))
		return (CInstanceGroup*)-1;
	else
		return _ClusterSystem;
}

// ***************************************************************************
void	CTransform::traverseClip()
{
	// disable H_AUTO, because slowdown when lot of models (eg 1000-2000 tested in forest)
	//H_AUTO( NL3D_TransformClip );

	CScene			*scene= getOwnerScene();
	CClipTrav		&clipTrav= scene->getClipTrav();

	if ((_ClipDate == clipTrav.CurrentDate) && _Visible)
		return;
	_ClipDate = clipTrav.CurrentDate;

	// clip: update Visible flag.
	_Visible= false;
	// if at least visible.
	if(_WorldVis)
	{
		// If linked to a SkeletonModel anywhere in the hierarchy, don't clip, and use skeleton model clip result.
		// This works because we are sons of a special node which is not in the clip traversal, and
		// which is traversed at end of the traversal.
		if( _AncestorSkeletonModel!=NULL )
		{
			_Visible= _AncestorSkeletonModel->isClipVisible();
			// Special test: if we are sticked to a skeletonModel, and if we are still visible, maybe we don't have to
			if(_Visible && _FatherSkeletonModel)
			{
				// if our skeletonModel father is displayed with a Lod, maybe we are not to be displayed
				if(_FatherSkeletonModel->isDisplayedAsLodCharacter())
				{
					// We are visible only if we where sticked to the skeleton with forceCLod==true.
					// This is also true if we are actually a skeletonModel
					if(!_ForceCLodSticked)
						// otherWise we are not visible. eg: this is the case of skins and some sticked object
						_Visible= false;
				}
			}
		}
		// else, clip.
		else
		{
			// If the instance is not filtered
			if(scene->getFilterRenderFlags() & _RenderFilterType)
			{
				// User cliping enabled ?
				if (_StateFlags & UserClipping)
					_Visible= true;
				else
					_Visible= clip();
			}
		}
	}

	// if visible, add to list.
	if(_Visible)
	{
		// add this model to the visibility list.
		clipTrav.addVisibleModel(this);

		// Has not an ancestor skeleton model?
		if( _AncestorSkeletonModel==NULL )
		{
			// If needed, insert the model in the lighted list.
			// don't insert if has an ancestorSkeletonModel, because in this case, result is driven by
			// the _LightContribution of the _AncestorSkeletonModel.
			if( isLightable() )
				scene->getLightTrav().addLightedModel(this);

			// If needed, insert the model in the animDetail list.
			// don't insert if has an ancestoreSkeletonModel, because in this case, this ancestore will
			// animDetail through the hierarchy...
			if( isAnimDetailable() )
				scene->getAnimDetailTrav().addVisibleModel(this);
		}

		// If needed, Add it to the loadBalancing trav
		if( isLoadBalancable() )
			scene->getLoadBalancingTrav().addVisibleModel(this);

		// If needed, insert the model in the render list.
		if( isRenderable() )
			scene->getRenderTrav().addRenderModel(this);
	}

	// Traverse the Clip sons.
	uint	num= clipGetNumChildren();
	for(uint i=0;i<num;i++)
		clipGetChild(i)->traverseClip();
}



// ***************************************************************************
// ***************************************************************************
// AnimDetail Trav
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CTransform::updateWorldMatrixFromFather()
{
	// If I am not skinned, and If I have a skeleton ancestor
	if(!isSkinned() && _AncestorSkeletonModel )
	{
		// Compute the HRC _WorldMatrix.
		// if I am not sticked.
		if(!_FatherSkeletonModel)
		{
			// get the normal father worldMatrix in Hrc.
			CTransform	*fatherTransform= hrcGetParent();
			// if exist
			if(fatherTransform)
			{
				const CMatrix &parentWM= fatherTransform->_WorldMatrix;
				// combine worldMatrix
				_WorldMatrix= parentWM * _LocalMatrix;
			}
			else
				_WorldMatrix= _LocalMatrix;
		}
		else
		{
			// get the worldMatrix of the bone if I am sticked (standard stick)
			if(!getStateFlag(SSSWO))
			{
				const CMatrix &parentWM= _FatherSkeletonModel->Bones[_FatherBoneId].getWorldMatrix();
				// combine worldMatrix
				_WorldMatrix= parentWM * _LocalMatrix;
			}
			// Special SkeletonSpawnScript stick
			else
			{
				// The parent matrix must be computed from a special matrix given to the skeleton model
				CMatrix		parentWM;
				parentWM.setRot(CVector::I, _FatherSkeletonModel->getSSSWODir(), CVector::K);
				parentWM.normalize(CMatrix::YZX);
				parentWM.setPos(_FatherSkeletonModel->getSSSWOPos());
				// combine worldMatrix
				_WorldMatrix= parentWM * _LocalMatrix;
			}
		}
	}
}


// ***************************************************************************
void			CTransform::traverseAnimDetailWithoutUpdateWorldMatrix()
{
	// AnimDetail behavior: animate only if not clipped.
	// NB: no need to test because of VisibilityList use.

	// test if the refptr is NULL or not (RefPtr).
	CChannelMixer	*chanmix= _ChannelMixer;
	if(chanmix)
	{
		// eval detail!!
		chanmix->eval(true, getOwnerScene()->getAnimDetailTrav().CurrentDate);
	}
}

// ***************************************************************************
void			CTransform::traverseAnimDetail()
{
	// First, test if I must update my worldMatrix because of the ancestorSkeleton scheme
	updateWorldMatrixFromFather();

	// eval channelMixer.
	traverseAnimDetailWithoutUpdateWorldMatrix();

	// NB: if want to add something, do it in traverseAnimDetailWithoutUpdateWorldMatrix(), because
	// CSkeletonModel doesn't call CTransform::traverseAnimDetail()
}


// ***************************************************************************
// ***************************************************************************
// LoadBalancing
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CTransform::traverseLoadBalancing()
{
	// noop
}


// ***************************************************************************
// ***************************************************************************
// Lighting.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CTransform::resetLighting()
{
	// if the model is already isNeedUpdateLighting, his light setup is reseted.
	// so no need to reset again

	if(isNeedUpdateLighting())
		return;


	// For all light not in FrozenStaticLightSetup, remove me from their list
	uint	startLight= 0;
	if(_LightContribution.FrozenStaticLightSetup)
	{
		startLight= _LightContribution.NumFrozenStaticLight;
	}

	// for all light in the list, remove me from their list.
	for(uint i=startLight; i<NL3D_MAX_LIGHT_CONTRIBUTION; i++)
	{
		CPointLight		*pl= _LightContribution.PointLight[i];
		// if end of list, break.
		if(!pl)
			break;
		else
		{
			// remove me from this light.
			pl->removeLightedModel(_LightContribution.TransformIterator[i]);
		}
	}
	// empty the list.
	if(startLight<NL3D_MAX_LIGHT_CONTRIBUTION)
		_LightContribution.PointLight[startLight]= NULL;


	// the model needs to update his lighting.
	setStateFlag(IsNeedUpdateLighting, true);

}


// ***************************************************************************
void			CTransform::freezeStaticLightSetup(CPointLight *pointLight[NL3D_MAX_LIGHT_CONTRIBUTION],
		uint numPointLights, uint8 sunContribution, CPointLight *frozenAmbientlight)
{
	nlassert(numPointLights <= NL3D_MAX_LIGHT_CONTRIBUTION);

	// resetLighting() first.
	resetLighting();

	// Enable StaticLightSetup.
	_LightContribution.FrozenStaticLightSetup= true;
	_LightContribution.NumFrozenStaticLight= uint8(numPointLights);
	_LightContribution.SunContribution= sunContribution;
	// setup the FrozenAmbientLight
	_LightContribution.FrozenAmbientLight= frozenAmbientlight;
	// Setup other pointLights
	uint i;
	for(i=0;i<numPointLights;i++)
	{
		// set the light
		_LightContribution.PointLight[i]= pointLight[i];
		// Enable at max.
		_LightContribution.Factor[i]= 255;
		// Compute static AttFactor Later because don't have WorlPosition of the model here!!
		setStateFlag(IsNeedUpdateFrozenStaticLightSetup, true);

		// Do NOT set the iterator, because it is a staticLight.
	}
	// End the list
	if(i<NL3D_MAX_LIGHT_CONTRIBUTION)
		_LightContribution.PointLight[i]= NULL;
}

// ***************************************************************************
void			CTransform::unfreezeStaticLightSetup()
{
	// resetLighting() first.
	resetLighting();

	// Disable StaticLightSetup.
	_LightContribution.FrozenStaticLightSetup= false;
	_LightContribution.NumFrozenStaticLight= 0;
	// End the list
	_LightContribution.PointLight[0]= NULL;
	// No more FrozenAmbientLight
	_LightContribution.FrozenAmbientLight= NULL;

	// Don't need to update StaticLightSetup since no more exist.
	setStateFlag(IsNeedUpdateFrozenStaticLightSetup, false);
}


// ***************************************************************************
void	CTransform::traverseLight()
{
	// if the model do not need to update his lighting, just skip.
	if(!isNeedUpdateLighting())
		return;


	// If a freezeStaticLightSetup() has been called on this model recently.
	if(isNeedUpdateFrozenStaticLightSetup())
	{
		// Now, the correct matrix is computed.
		// get the untransformed bbox from the model.
		CAABBox		bbox;
		getAABBox(bbox);
		// get transformed center pos of bbox
		CVector	worldModelPos= getWorldMatrix() * bbox.getCenter();

		// So we can compute AttFactor for each static light influencing this static object
		uint	numPointLights= _LightContribution.NumFrozenStaticLight;
		for(uint i=0;i<numPointLights;i++)
		{
			const CPointLight	*pl= _LightContribution.PointLight[i];
			// don't worry about the precision of floor, because of *255.
			float	distToModel= (pl->getPosition() - worldModelPos).norm();
			sint	attFactor= NLMISC::OptFastFloor( 255 * pl->computeLinearAttenuation(worldModelPos, distToModel) );
			_LightContribution.AttFactor[i]= (uint8)attFactor;
		}

		// clean.
		setStateFlag(CTransform::IsNeedUpdateFrozenStaticLightSetup, false);
	}


	// see CTransform::clip(), here I am Lightable(), and I have no _AncestorSkeletonModel
	// So I am sure that I really need to recompute my ModelLightContributions.
	CScene	*scene= getOwnerScene();
	scene->getLightTrav().LightingManager.computeModelLightContributions(scene->getSunAmbient(), this,
		_LightContribution, _LogicInfo);

	// done!
	setStateFlag(CTransform::IsNeedUpdateLighting, false);
}


// ***************************************************************************
// ***************************************************************************
// Rendering
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CTransform::traverseRender()
{
	// no-op
}

// ***************************************************************************
void	CTransform::profileRender()
{
	// no-op
}


// ***************************************************************************
// ***************************************************************************
// Hrc Linking
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CTransform::hrcLinkSon(CTransform *son)
{
	if(!son)
		return;

	// If not unfrozen, can't link
	if (son->_FreezeHRCState != CTransform::FreezeHRCStateDisabled)
		return;

	// no-op if already me.
	if(son->_HrcParent==this)
		return;

	// unlink from anyone
	son->hrcUnlink();

	// link son to me
	_HrcSons.insert(son, &son->_HrcNode);

	// link me to son
	son->_HrcParent= this;

	// Backup parent
	son->_HrcParentUnfreeze= this;

	// my son should recompute his worldMatrix!
	son->_WorldDate= -1;
}

// ***************************************************************************
void			CTransform::hrcUnlink()
{
	// no-op if already NULL
	if(_HrcParent==NULL)
		return;

	// if ForceClipRoot flag is set, then the fx can't be linked elsewhere in the hierarchy
	nlassert(!getStateFlag(ForceClipRoot));

	// unlink my parent from me.
	_HrcNode.unlink();

	// unlink me from parent
	_HrcParent= NULL;
	_HrcParentUnfreeze= NULL;

	// I should recompute my worldMatrix (well not useful since not linked, but still do it...)
	_WorldDate= -1;
}

// ***************************************************************************
CTransform		*CTransform::hrcGetChild(uint index) const
{
	nlassert(index < _HrcSons.size());
	return (const_cast<CTransform*>(this))->_HrcSons.begin()[index];
}


// ***************************************************************************
// ***************************************************************************
// Clip Linking
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CTransform::clipAddChild(CTransform *son)
{
	if(!son)
		return;

	// if already linked, no-op.
	if(son->clipHasParent(this))
		return;

	// add a new parent entry for our son.
	CClipNode	*clipNode= new CClipNode;
	son->_ClipParents.push_back(clipNode);

	// link the son to us
	clipNode->Parent= this;

	// link us to the son
	_ClipSons.insert(son, &clipNode->ClipNode);
}

// ***************************************************************************
void			CTransform::clipDelChild(CTransform *son)
{
	if(!son)
		return;

	// try to remove from me from my parent
	son->clipDelFromParent(this);
}

// ***************************************************************************
void			CTransform::clipUnlinkFromAll()
{
	// unlink from all parent clip
	while( clipGetNumParents() )
	{
		clipDelFromParent( clipGetParent(0) );
	}
}

// ***************************************************************************
CTransform		*CTransform::clipGetParent(uint index) const
{
	nlassert(index < _ClipParents.size());
	return _ClipParents[index]->Parent;
}

// ***************************************************************************
CTransform		*CTransform::clipGetChild(uint index) const
{
	nlassert(index < _ClipSons.size());
	return (const_cast<CTransform*>(this))->_ClipSons.begin()[index];
}


// ***************************************************************************
bool			CTransform::clipHasParent(CTransform *parent)
{
	// search O(n) for all parents
	for(uint i=0;i<_ClipParents.size();i++)
	{
		if(_ClipParents[i]->Parent==parent)
			return true;
	}

	return false;
}

// ***************************************************************************
void			CTransform::clipDelFromParent(CTransform *parent)
{
	// search O(n) for all Parents
	uint	numParents= (uint)_ClipParents.size();
	for(uint i=0;i<numParents;i++)
	{
		if(_ClipParents[i]->Parent==parent)
		{
			// found! remove me from my parent list
			_ClipParents[i]->ClipNode.unlink();

			// remove this parent entry. swap with last
			swap(_ClipParents[i], _ClipParents[numParents-1]);

			// and delete last.
			delete _ClipParents[numParents-1];
			_ClipParents.resize(numParents-1);

			break;
		}
	}
}

// ***************************************************************************
void			CTransform::setUserClipping(bool enable)
{
	setStateFlag (UserClipping, enable);
}

// ***************************************************************************
bool			CTransform::getUserClipping() const
{
	return getStateFlag(UserClipping) != 0;
}

// ***************************************************************************
// ***************************************************************************
// ShadowMap
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void			CTransform::getReceiverBBox(CAABBox &bbox)
{
	bbox.setCenter(CVector::Null);
	bbox.setHalfSize(CVector::Null);
}

// ***************************************************************************
void			CTransform::enableCastShadowMap(bool state)
{
	bool	precState= canCastShadowMap();

	if(modelCanCastShadowMap())
		setStateFlag(IsFinalShadowMapCaster, state);
	else
		setStateFlag(IsFinalShadowMapCaster, false);

	// if just enabled, create the shadowMap
	if(canCastShadowMap() && !precState)
	{
		createShadowMap();
		// The user must have created it.
		nlassert(getShadowMap());
	}
	// if just disabled, free ressource
	else if(!canCastShadowMap() && precState)
	{
		deleteShadowMap();
	}
}

// ***************************************************************************
void CTransform::forceCompute()
{
	// if father is a skeleton, force to compute the bone we are sticked to
	if (_FatherSkeletonModel)
	{
		_FatherSkeletonModel->forceComputeBone(_FatherBoneId);
	}
	else
	{
		// force to compute the father
		if (_HrcParent)
		{
			_HrcParent->forceCompute();
		}
	}
	// compute
	update();
	updateWorldMatrixFromFather();
}

// ***************************************************************************
void CTransform::setForceClipRoot(bool forceClipRoot)
{
	if (forceClipRoot == (getStateFlag(ForceClipRoot) != 0)) return;
	if (forceClipRoot)
	{
		// unlink from previous father and link to the root
		hrcUnlink();
		if (_OwnerScene)
		{
			_OwnerScene->getRoot()->hrcLinkSon(this);
		}
		setClusterSystem(NULL);
	}
	setStateFlag(ForceClipRoot, forceClipRoot);
}

// ***************************************************************************
UTransform *CTransform::buildMatchingUserInterfaceObject()
{
	return new UTransform(this);
}

// ***************************************************************************
void CTransform::setShadowMapDirectionZThreshold(float zthre)
{
	clamp(zthre, -1.f, 1.f);
	_ShadowMapDirectionZThreshold= zthre;
}

// ***************************************************************************
void CTransform::setShadowMapMaxDepth(float depth)
{
	depth= max(0.f, depth);
	_ShadowMapMaxDepth= depth;
}


}
