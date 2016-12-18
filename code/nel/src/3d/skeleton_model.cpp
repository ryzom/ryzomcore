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

#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/anim_detail_trav.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/3d/scene.h"
#include "nel/3d/lod_character_manager.h"
#include "nel/3d/lod_character_shape.h"
#include "nel/misc/rgba.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/vertex_stream_manager.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/async_texture_manager.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
void		CSkeletonModel::registerBasic()
{
	CScene::registerModel(SkeletonModelId, TransformShapeId, CSkeletonModel::creator);
}


// ***************************************************************************
CTrackDefaultString		CSkeletonModel::_DefaultSpawnScript;


// ***************************************************************************
IAnimatedValue	*CSkeletonModel::getValue (uint valueId)
{
	// what value ?
	switch (valueId)
	{
	case SpawnScriptValue: return &_SpawnScript;
	}

	return CTransformShape::getValue(valueId);
}

// ***************************************************************************
const char	*CSkeletonModel::getValueName (uint valueId) const
{
	// what value ?
	switch (valueId)
	{
	case SpawnScriptValue: return getSpawnScriptValueName();
	}

	return CTransformShape::getValueName(valueId);
}

// ***************************************************************************
ITrack		*CSkeletonModel::getDefaultTrack (uint valueId)
{
	// what value ?
	switch (valueId)
	{
	case SpawnScriptValue: return &_DefaultSpawnScript;
	}

	return CTransformShape::getDefaultTrack(valueId);
}

// ***************************************************************************
void		CSkeletonModel::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	/* add the Spawn Script value. The animation is evaluated at detail time, as the script evaluation.
		This seems dangerous (create and delete models at evalDetail time) but works because:
		- deletedModels() in a current CScene::render() are delayed to end of render()
			and are "temp removed" from the render trav
		- createdModels() in CSkeletonSpawnScript are delayed to the end of CScene::render()
		- if a skeleton is not visible, or in LOD form, then its sticked SpawnedModels are not visible too,
			whether or not they are too old regarding the animation time
	*/
	_SpawnScriptChannelId= addValue(chanMixer, SpawnScriptValue, OwnerBit, prefix, true);

	// add standard
	CTransformShape::registerToChannelMixer(chanMixer, prefix);

	// Add any bones.
	for(uint i=0;i<Bones.size();i++)
	{
		// append  bonename.
		Bones[i].registerToChannelMixer(chanMixer, prefix + Bones[i].getBoneName() + ".");
	}

}

// ***************************************************************************
CSkeletonModel::CSkeletonModel()
{
	IAnimatable::resize(AnimValueLast);
	_DisplayedAsLodCharacter= false;
	_LodCharacterDistance= 0;
	_OOLodCharacterDistance= 0;
	_IsEnableLOD=true;

	_DefaultMRMSetup= true;

	_SkinToRenderDirty= false;

	_CLodVertexAlphaDirty= true;

	_LodEmit= CRGBA::Black;

	// Inform the transform that I am a skeleton
	CTransform::setIsSkeleton(true);

	// By default, no skins, hence, impossible to have transparent pass. But opaque pass is always possible
	// because of CLod rendering
	setOpacity(true);
	setTransparency(false);

	// AnimDetail behavior: Must be traversed in AnimDetail, even if no channel mixer registered
	CTransform::setIsForceAnimDetail(true);

	// LoadBalancing behavior. true because directly act on skins to draw all their MRM level
	CTransform::setIsLoadbalancable(true);

	// Lighting behavior. Lightable because skins/stickedObjects may surely need its LightContribution
	CTransform::setIsLightable(true);

	// Render behavior. Always renderable, because either render the skeleton as a CLod, or render skins
	CTransform::setIsRenderable(true);

	// build a bug-free level detail
	buildDefaultLevelDetail();

	// RenderFilter: We are a skeleton
	_RenderFilterType= UScene::FilterSkeleton;

	_AnimCtrlUsage= 0;

	// ShadowMap
	CTransform::setIsShadowMapCaster(true);
	_ShadowMap= NULL;

	// SpawnScript
	_SSSWOPos= CVector::Null;
	_SSSWODir= CVector::J;
	_SpawnScriptChannelId= -1;
}


// ***************************************************************************
CSkeletonModel::~CSkeletonModel()
{
	// if initModel() called
	if(getOwnerScene())
	{
		// release the _SpawnScriptEvaluator (delete any instance sticked)
		_SpawnScriptEvaluator.release(getOwnerScene());

		// remove from scene
		getOwnerScene()->eraseSkeletonModelToList(_ItSkeletonInScene);
	}


	// detach skeleton sons from skins.
	while(_Skins.begin()!=_Skins.end())
	{
		detachSkeletonSon(*_Skins.begin());
	}

	// detach skeleton sons from sticked objects.
	while(_StickedObjects.begin()!=_StickedObjects.end())
	{
		detachSkeletonSon(*_StickedObjects.begin());
	}

	// Free Lod instance
	setLodCharacterShape(-1);

	// delete the shadowMap
	deleteShadowMap();
}


// ***************************************************************************
void	CSkeletonModel::initModel()
{
	// Link this skeleton to the CScene.
	_ItSkeletonInScene= getOwnerScene()->appendSkeletonModelToList(this);

	// Call base class
	CTransformShape::initModel();
}


// ***************************************************************************
void		CSkeletonModel::initBoneUsages()
{
	// reset all to 0.
	_BoneUsage.resize(Bones.size());
	for(uint i=0; i<_BoneUsage.size(); i++)
	{
		_BoneUsage[i].Usage= 0;
		_BoneUsage[i].ForcedUsage= 0;
		_BoneUsage[i].CLodForcedUsage= 0;
		_BoneUsage[i].MustCompute= 0;
		_BoneUsage[i].ValidBoneSkinMatrix= 0;
	}
	// reserve space for bone to compute
	_BoneToCompute.reserve(Bones.size());

	_BoneToComputeDirty= false;
	_CurLod= 0;
	_CurLodInterp= 1.f;
	// Default is 0.5 meters.
	_LodInterpMultiplier= 1.f / 0.5f;
}


// ***************************************************************************
void		CSkeletonModel::incBoneUsage(uint i, TBoneUsageType boneUsageType)
{
	nlassert(i<_BoneUsage.size());

	// Get ptr on according refCount
	uint16	*usagePtr;
	if(boneUsageType == UsageNormal)
		usagePtr= &_BoneUsage[i].Usage;
	else if(boneUsageType == UsageForced)
		usagePtr= &_BoneUsage[i].ForcedUsage;
	else
		usagePtr= &_BoneUsage[i].CLodForcedUsage;

	// If the bone was not used before, must update MustCompute.
	if(*usagePtr==0)
		_BoneToComputeDirty= true;

	// Inc the refCount of the bone.
	nlassert(*usagePtr<65535);
	(*usagePtr)++;
}


// ***************************************************************************
void		CSkeletonModel::decBoneUsage(uint i, TBoneUsageType boneUsageType)
{
	nlassert(i<_BoneUsage.size());

	// Get ptr on according refCount
	uint16	*usagePtr;
	if(boneUsageType == UsageNormal)
		usagePtr= &_BoneUsage[i].Usage;
	else if(boneUsageType == UsageForced)
		usagePtr= &_BoneUsage[i].ForcedUsage;
	else
		usagePtr= &_BoneUsage[i].CLodForcedUsage;

	// If the bone was used before (and now won't be more), must update MustCompute.
	if(*usagePtr==1)
		_BoneToComputeDirty= true;

	// Inc the refCount of the bone.
	nlassert(*usagePtr>0);
	(*usagePtr)--;
}


// ***************************************************************************
void		CSkeletonModel::flagBoneAndParents(uint32 boneId, std::vector<bool>	&boneUsage) const
{
	nlassert( boneUsage.size()==Bones.size() );
	nlassert( boneId<Bones.size() );

	// Flag this bone.
	boneUsage[boneId]= true;

	// if has father, flag it (recurs).
	sint	fatherId= Bones[boneId].getFatherId();
	if(fatherId>=0)
		flagBoneAndParents(fatherId, boneUsage);
}


// ***************************************************************************
void		CSkeletonModel::incForcedBoneUsageAndParents(uint i, bool forceCLod)
{
	// inc forced.
	incBoneUsage(i, forceCLod?UsageCLodForced:UsageForced );

	// recurs to father
	sint	fatherId= Bones[i].getFatherId();
	// if not a root bone...
	if(fatherId>=0)
		incForcedBoneUsageAndParents(fatherId, forceCLod);
}

// ***************************************************************************
void		CSkeletonModel::decForcedBoneUsageAndParents(uint i, bool forceCLod)
{
	// dec forced
	decBoneUsage(i, forceCLod?UsageCLodForced:UsageForced);

	// recurs to father
	sint	fatherId= Bones[i].getFatherId();
	// if not a root bone...
	if(fatherId>=0)
		decForcedBoneUsageAndParents(fatherId, forceCLod);
}


// ***************************************************************************
void		CSkeletonModel::updateBoneToCompute()
{
	// If already computed, skip
	if(!_BoneToComputeDirty)
		return;

	// get the channelMixer owned by CTransform.
	CChannelMixer	*chanMixer= getChannelMixer();

	// Get Lod infos from skeletonShape
	CSkeletonShape		*skeShape= (CSkeletonShape*)(IShape*)Shape;
	const CSkeletonShape::CLod	&lod= skeShape->getLod(_CurLod);

	// reset _BoneToCompute
	_BoneToCompute.clear();

	// For all bones
	for(uint i=0; i<_BoneUsage.size(); i++)
	{
		// If we are in CLod mode
		if(isDisplayedAsLodCharacter())
			// don't compute the bone
			_BoneUsage[i].MustCompute= 0;
		else
		{
			// set MustCompute to non 0 if (Usage && Lod) || ForcedUsage;
			_BoneUsage[i].MustCompute= (_BoneUsage[i].Usage & lod.ActiveBones[i]) | _BoneUsage[i].ForcedUsage;
		}
		// if CLodForcedUsage for the bone, it must be computed, whatever _DisplayedAsLodCharacter state
		_BoneUsage[i].MustCompute|= _BoneUsage[i].CLodForcedUsage;

		// If the bone must be computed (if !0)
		if(_BoneUsage[i].MustCompute)
		{
			// lodEnable the channels of this bone
			if(chanMixer)
				Bones[i].lodEnableChannels(chanMixer, true);

			// This bone is computed => take his valid boneSkinMatrix.
			_BoneUsage[i].ValidBoneSkinMatrix= i;

			// Append to the list to compute.
			//-------
			CBoneCompute	bc;
			bc.Bone= &Bones[i];
			sint	fatherId= Bones[i].getFatherId();
			// if a root bone...
			if(fatherId==-1)
				bc.Father= NULL;
			else
				bc.Father= &Bones[fatherId];
			// MustInterpolate??
			bc.MustInterpolate= false;
			const CSkeletonShape::CLod	*lodNext= NULL;
			// if a lod exist after current lod, and if lod interpolation enabled
			if( _CurLod < skeShape->getNumLods()-1 && _LodInterpMultiplier>0 )
			{
				// get next lod.
				lodNext= &skeShape->getLod(_CurLod+1);
				// Lod interpolation on this bone ?? only if at next lod, the bone is disabled.
				// And only if it is not enabed because of a "Forced reason"
				// Must also have a father, esle can't interpolate.
				if(lodNext->ActiveBones[i]==0 && _BoneUsage[i].ForcedUsage==0 && _BoneUsage[i].CLodForcedUsage==0
					&& bc.Father)
					bc.MustInterpolate= true;
			}
			// append
			_BoneToCompute.push_back(bc);
		}
		else
		{
			// lodDisable the channels of this bone
			if(chanMixer)
				Bones[i].lodEnableChannels(chanMixer, false);

			// This bone is not computed => take the valid boneSkinMatrix of his father
			sint	fatherId= Bones[i].getFatherId();
			if(fatherId<0)
				// just take me, even if not computed.
				_BoneUsage[i].ValidBoneSkinMatrix= i;
			else
				// NB: father ValidBoneSkinMatrix already computed because of the hierarchy order of Bones array.
				_BoneUsage[i].ValidBoneSkinMatrix= _BoneUsage[fatherId].ValidBoneSkinMatrix;
		}
	}

	// Enable SpawnScript animation only if we are not in CLod
	if(_SpawnScriptChannelId>=0 && chanMixer)
		chanMixer->lodEnableChannel(_SpawnScriptChannelId, !isDisplayedAsLodCharacter());

	// computed
	_BoneToComputeDirty= false;
}


// ***************************************************************************
bool		CSkeletonModel::isBoneComputed(uint boneId) const
{
	if(boneId>=_BoneUsage.size())
		return false;
	else
		return _BoneUsage[boneId].MustCompute!=0 && isClipVisible();
}


// struct used by CSkeletonModel::forceComputeBone only
struct CForceComputeBoneInfo
{
	CTransform *Transform;
	uint		StickBoneID; // if the transform is a skeleton, gives the bone to which child of interest is sticked
};

// ***************************************************************************
bool CSkeletonModel::forceComputeBone(uint boneId)
{
	if(boneId >= _BoneUsage.size()) return false;
	// build list of ancestor, then must build
	std::vector<CForceComputeBoneInfo> ancestors;
	// count the number of ancestors (to avoid unwanted alloc with vector)
	uint numAncestors = 1;
	CTransform *currTransform = this;
	for(;;)
	{
		currTransform = currTransform->_HrcParent ? currTransform->_HrcParent : currTransform->_FatherSkeletonModel; // find father transform (maybe a skeleton or a std transform)
		if (!currTransform) break; // root reached ?
		++ numAncestors;
	}
	ancestors.reserve(numAncestors);
	// build list of ancestor
	currTransform = this;
	uint currStickBone = boneId;
	for(;;)
	{
		// if curr transform is a skeleton, animate all bone from stick bone to the root bone
		if (currTransform->isSkeleton())
		{
			if (_ChannelMixer)
			{
				CSkeletonModel *skel = static_cast<CSkeletonModel *>(currTransform);
				nlassert(boneId < skel->_BoneUsage.size());
				nlassert(currStickBone < skel->Bones.size());
				sint currBoneIndex = currStickBone;
				// force channel mixer to eval for that bone
				while (currBoneIndex != -1)
				{
					nlassert((uint) currBoneIndex < skel->Bones.size());
					skel->Bones[currBoneIndex].forceAnimate(*_ChannelMixer);
					currBoneIndex = skel->Bones[currBoneIndex].getFatherId();
				}
			}
		}
		else
		{
			// update stickBone ID (if father is a skeleton)
			currStickBone = _FatherBoneId;
		}
		CForceComputeBoneInfo fcbi;
		fcbi.StickBoneID = currStickBone;
		fcbi.Transform   = currTransform;
		ancestors.push_back(fcbi);
		currTransform = currTransform->_HrcParent ? currTransform->_HrcParent : currTransform->_FatherSkeletonModel; // find father transform (maybe a skeleton or a std transform)
		if (!currTransform) break; // root reached ?
	}
	// bones must be recomputed from father bone to sons, so must traverse bones until root is reached to retrieve correct ordering
	CBone *OrderedBone[MaxNumBones];
	//
	const CMatrix *parentWorldMatrix = &CMatrix::Identity;
	for(std::vector<CForceComputeBoneInfo>::reverse_iterator it = ancestors.rbegin(); it != ancestors.rend(); ++it)
	{
		// update world matrix (NB : the call to getmatrix will update the local matrix)
		it->Transform->setWorldMatrix(*parentWorldMatrix * it->Transform->getMatrix());
		if (it->Transform->isSkeleton())
		{
			CSkeletonModel *skel = static_cast<CSkeletonModel *>(it->Transform);
			// reorder bones
			uint numBones = 0;
			nlassert(it->StickBoneID < skel->Bones.size());
			sint currBoneIndex = it->StickBoneID;
			nlassert(currBoneIndex != -1);
			do
			{
				nlassert(numBones < MaxNumBones);
				nlassert((uint) currBoneIndex < skel->Bones.size());
				OrderedBone[numBones] = &skel->Bones[currBoneIndex];
				currBoneIndex = OrderedBone[numBones]->getFatherId();
				++ numBones;
			}
			while (currBoneIndex != -1);
			const CMatrix &modelWorldMatrix = it->Transform->getWorldMatrix();
			// recompute bones
			CBone *fatherBone = NULL;
			while (numBones--)
			{
				OrderedBone[numBones]->compute(fatherBone, modelWorldMatrix, NULL);
				fatherBone = OrderedBone[numBones];
			}
			parentWorldMatrix = &(OrderedBone[0]->getWorldMatrix());
		}
		else
		{
			parentWorldMatrix = &it->Transform->getWorldMatrix();
		}
	}
	return true;
}



// ***************************************************************************
const NLMISC::CMatrix	&CSkeletonModel::getActiveBoneSkinMatrix(uint boneId) const
{
	// Get me or first father with MustCompute==true.
	uint validBoneId= _BoneUsage[boneId].ValidBoneSkinMatrix;
	// return his WorldMatrix.
	return Bones[validBoneId].getBoneSkinMatrix();
}


// ***************************************************************************
bool		CSkeletonModel::bindSkin(CTransform *mi)
{
	nlassert(mi);
	if( !mi->isSkinnable() )
		return false;

	// try to detach this object from any skeleton first (possibly me).
	if(mi->_FatherSkeletonModel)
		mi->_FatherSkeletonModel->detachSkeletonSon(mi);

	// Then Add me.
	_Skins.insert(mi);

	// advert skin transform it is skinned.
	mi->_FatherSkeletonModel= this;
	// setApplySkin() use _FatherSkeletonModel to computeBonesId() and to update current skeleton bone usage.
	mi->setApplySkin(true);


	// Unlink the Skin from Hrc and clip, because SkeletonModel now does the job for him.
	// First ensure that the transform is not frozen (unlink from some quadGrids etc...)
	mi->unfreezeHRC();
	// then never re-parse in validateList/Hrc/Clip
	mi->unlinkFromUpdateList();
	mi->hrcUnlink();
	// ClipTrav is a graph, so must unlink from ALL olds models.
	mi->clipUnlinkFromAll();
	// Ensure flag is correct
	mi->_ClipLinkedInSonsOfAncestorSkeletonModelGroup= false;


	// must recompute lod vertex alpha when LodCharacter used
	dirtLodVertexAlpha();
	// must recompute list of skins.
	dirtSkinRenderLists();

	// Ok, skinned
	return true;
}
// ***************************************************************************
void		CSkeletonModel::stickObject(CTransform *mi, uint boneId)
{
	// by default don't force display of "mi" if the skeleton become in CLod state
	stickObjectEx(mi, boneId, false);
}
// ***************************************************************************
void		CSkeletonModel::stickObjectEx(CTransform *mi, uint boneId, bool forceCLod)
{
	nlassert(mi);

	// if "mi" is a skeleton, forceCLod must be true, for correct animation purpose
	if(dynamic_cast<CSkeletonModel*>(mi))
		forceCLod= true;

	// try to detach this object from any skeleton first (possibly me).
	if(mi->_FatherSkeletonModel)
		mi->_FatherSkeletonModel->detachSkeletonSon(mi);

	// Then Add me.
	_StickedObjects.insert(mi);
	// increment the refCount usage of the bone
	incForcedBoneUsageAndParents(boneId, forceCLod);

	// advert transform of its sticked state.
	mi->_FatherSkeletonModel= this;
	mi->_FatherBoneId= boneId;
	// advert him if it is "ForceCLod" sticked
	mi->_ForceCLodSticked= forceCLod;

	// link correctly Hrc only. ClipTrav grah updated in Hrc traversal.
	hrcLinkSon( mi );

	// must recompute lod vertex alpha when LodCharacter used
	dirtLodVertexAlpha();
}
// ***************************************************************************
void		CSkeletonModel::detachSkeletonSon(CTransform *tr)
{
	nlassert(tr);

	// If the instance is not binded/sticked to the skeleton, exit.
	if(tr->_FatherSkeletonModel!=this)
		return;

	// try to erase from StickObject.
	_StickedObjects.erase(tr);
	// try to erase from Skins.
	_Skins.erase(tr);

	// If the instance is not skinned, then it is sticked
	bool	wasSkinned= tr->isSkinned()!=0;
	if( !wasSkinned )
	{
		// Then decrement Bone Usage RefCount. Decrement from CLodForcedUsage if was sticked with forceCLod==true
		decForcedBoneUsageAndParents(tr->_FatherBoneId, tr->_ForceCLodSticked);
	}
	else
	{
		// it is skinned, advert the skinning is no more OK.
		// setApplySkin() use _FatherSkeletonModel to update current skeleton bone usage.
		tr->setApplySkin(false);
	}

	// advert transform it is no more sticked/skinned.
	tr->_FatherSkeletonModel= NULL;
	tr->_ForceCLodSticked= false;

	// link correctly Hrc / Clip / UpdateList...
	getOwnerScene()->getRoot()->hrcLinkSon( tr );
	if( !wasSkinned )
	{
		//  No-op. ClipTrav graph/UpdateList updated in Hrc traversal.
	}
	else
	{
		// Skin case: must do the Job here.
		// Update ClipTrav here.
		getOwnerScene()->getRoot()->clipAddChild(tr);
		// Must re-add to the update list.
		tr->linkToUpdateList();
	}


	// must recompute lod vertex alpha when LodCharacter used
	dirtLodVertexAlpha();
	// must recompute list of skins if was skinned
	if( wasSkinned )
		dirtSkinRenderLists();
}


// ***************************************************************************
sint32		CSkeletonModel::getBoneIdByName(const std::string &name) const
{
	CSkeletonShape		*shp= safe_cast<CSkeletonShape*>((IShape*)Shape);
	return shp->getBoneIdByName(name);
}


// ***************************************************************************
void		CSkeletonModel::setInterpolationDistance(float dist)
{
	dist= std::max(0.f, dist);
	// disable interpolation?
	if(dist==0)
		_LodInterpMultiplier= 0.f;
	else
		_LodInterpMultiplier= 1.f / dist;
}

// ***************************************************************************
float		CSkeletonModel::getInterpolationDistance() const
{
	if(_LodInterpMultiplier==0)
		return 0.f;
	else
		return 1.f / _LodInterpMultiplier;
}


// ***************************************************************************
void	CSkeletonModel::traverseAnimDetail()
{
	CSkeletonShape	*skeShape= ((CSkeletonShape*)(IShape*)Shape);

	/* NB: If "this" skeleton has an AncestorSkeletonModel displayed but "this" skeleton is clipped,
		it will be still animDetailed.
		So its possible sticked sons will be displayed with correct WorldMatrix (if not themselves clipped).
	*/

	/*	If the Root Skeleton Model (ie me or my AncestorSM) is asked to render a ShadowMap, BUT I am
		in CLod Form (and visible in HRC else won't be rendered in shadowMap...), then temporarly
		Avoid CLod!! To really compute the bones for this frame only.
	*/
	bool	tempAvoidCLod= false;
	bool	genShadow;
	if(_AncestorSkeletonModel)
		genShadow= _AncestorSkeletonModel->isGeneratingShadowMap();
	else
		genShadow= isGeneratingShadowMap();
	// do the test.
	if(genShadow && isDisplayedAsLodCharacter() && isHrcVisible() )
	{
		tempAvoidCLod= true;
		// Disable it just the time of this AnimDetail
		setDisplayLodCharacterFlag(false);
	}


	// Update Lod, and animate.
	//===============

	/*
		CTransformShape::traverseAnimDetail() is torn in 2 here because
		channels may be enabled/disabled by updateBoneToCompute()
	*/

	// First update Skeleton WorldMatrix (case where the skeleton is sticked).
	CTransform::updateWorldMatrixFromFather();
	// get dist from camera.
	float	dist= (getWorldMatrix().getPos() - getOwnerScene()->getClipTrav().CamPos).norm();
	// Use dist to get current lod to use for this skeleton
	uint	newLod= skeShape->getLodForDistance( dist );
	if(!_IsEnableLOD) newLod = 0;
	if(newLod != _CurLod)
	{
		// set new lod to use.
		_CurLod= newLod;
		// dirt the skeleton.
		_BoneToComputeDirty= true;
	}

	// If needed, let's know which bone has to be computed, and enable / disable (lod) channels in channelMixer.
	updateBoneToCompute();

	// Animate skeleton.
	CTransformShape::traverseAnimDetailWithoutUpdateWorldMatrix();

	// If in normal mode, must update the SpawnScript
	if(!isDisplayedAsLodCharacter())
	{
		_SpawnScriptEvaluator.evaluate(this);
	}

	// Prepare Lod Bone interpolation.
	//===============

	float	lodBoneInterp;
	const CSkeletonShape::CLod	*lodNext= NULL;
	// if a lod exist after current lod, and if lod interpolation enabled
	if( _CurLod < skeShape->getNumLods()-1 && _LodInterpMultiplier>0 && _IsEnableLOD)
	{
		// get next lod.
		lodNext= &skeShape->getLod(_CurLod+1);
		// get interp value to next.
		lodBoneInterp= (lodNext->Distance - dist) * _LodInterpMultiplier;
		NLMISC::clamp(lodBoneInterp, 0.f, 1.f);
		// if still 1, keep cur matrix => disable interpolation
		if(lodBoneInterp==1.f)
			lodNext=NULL;
	}
	// else, no interpolation
	else
	{
		lodBoneInterp=1.f;
	}
	_CurLodInterp= lodBoneInterp;


	// Compute bones
	//===============


	// test if bones must be updated. either if animation change or if BoneUsage change.
	// Retrieve the WorldMatrix of the current CTransformShape.
	const CMatrix		&modelWorldMatrix= getWorldMatrix();

	// must test / update the hierarchy of Bones.
	// Since they are orderd in depth-first order, we are sure that parent are computed before sons.
	uint							numBoneToCompute= (uint)_BoneToCompute.size();
	CSkeletonModel::CBoneCompute	*pBoneCompute= numBoneToCompute? &_BoneToCompute[0] : NULL;
	// traverse only bones which need to be computed
	for(;numBoneToCompute>0;numBoneToCompute--, pBoneCompute++)
	{
		// compute the bone with his father, if any
		pBoneCompute->Bone->compute( pBoneCompute->Father, modelWorldMatrix, this);

		// Lod interpolation on this bone .. only if interp is enabled now, and if bone wants it
		if(lodNext && pBoneCompute->MustInterpolate)
		{
			// interpolate with my father matrix.
			const CMatrix		&fatherMatrix= pBoneCompute->Father->getBoneSkinMatrix();
			pBoneCompute->Bone->interpolateBoneSkinMatrix(fatherMatrix, lodBoneInterp);
		}
	}

	IAnimatable::clearFlag(CSkeletonModel::OwnerBit);

	// Sticked Objects:
	// they will update their WorldMatrix after, because of the AnimDetail traverse scheme:
	// traverse visible Clip models, and if skeleton, traverse Hrc sons.


	// Restore the Initial CLod flag if needed (see above)
	if(tempAvoidCLod)
	{
		setDisplayLodCharacterFlag(true);
	}


	// Update Animated Skins.
	//===============
	for(uint i=0;i<_AnimDetailSkins.size();i++)
	{
		// traverse it. NB: updateWorldMatrixFromFather() is called but no-op because isSkinned()
		_AnimDetailSkins[i]->traverseAnimDetail();
	}
}


// ***************************************************************************
void		CSkeletonModel::computeAllBones(const CMatrix &modelWorldMatrix)
{
	// must test / update the hierarchy of Bones.
	// Since they are orderd in depth-first order, we are sure that parent are computed before sons.
	for(uint i=0;i<Bones.size();i++)
	{
		sint	fatherId= Bones[i].getFatherId();
		// if a root bone...
		if(fatherId==-1)
			// Compute root bone worldMatrix. Do not allow special AnimCtrl
			Bones[i].compute( NULL, modelWorldMatrix, NULL);
		else
			// Compute bone worldMatrix. Do not allow special AnimCtrl
			Bones[i].compute( &Bones[fatherId], modelWorldMatrix, NULL);
	}

}


// ***************************************************************************
void		CSkeletonModel::setLodCharacterDistance(float dist)
{
	_LodCharacterDistance= max(dist, 0.f);
	if(_LodCharacterDistance>0)
		_OOLodCharacterDistance= 1.0f/_LodCharacterDistance;
	else
		_OOLodCharacterDistance= 0;
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterShape(sint shapeId)
{
	// get a ptr on the scene which owns us, and so on the lodManager.
	CScene					*scene= getOwnerScene();
	CLodCharacterManager	*mngr= scene->getLodCharacterManager();

	// if mngr not setuped, noop (lod not possible).
	if(!mngr)
		return;

	// If a shape was setup, free the instance
	if(_CLodInstance.ShapeId>=0)
	{
		mngr->releaseInstance(_CLodInstance);
		_CLodInstance.ShapeId= -1;
	}

	// assign
	_CLodInstance.ShapeId= shapeId;

	// if a real shape is setuped, alloc an instance
	if(_CLodInstance.ShapeId>=0)
	{
		mngr->initInstance(_CLodInstance);
	}
}


// ***************************************************************************
void		CSkeletonModel::computeLodTexture()
{
	// is lod setuped
	if(_CLodInstance.ShapeId<0)
		return;

	// get a ptr on the scene which owns us, and so on the lodManager.
	CScene					*scene= getOwnerScene();
	CLodCharacterManager	*mngr= scene->getLodCharacterManager();
	// mngr must be setuped since shape Id is >-1
	nlassert(mngr);
	/* Get the asyncTextureManager. This is a Hack. We use the AsyncTextureManager to store very low version of Textures
		(kept in DXTC1 format for minimum memory overhead).
		HENCE Lod Texture can work only with Async Textured instances!!
	*/
	CAsyncTextureManager	*asyncMngr= scene->getAsyncTextureManager();
	// if not setuped, cancel
	if(!asyncMngr)
		return;


	// **** start process. If cannot (TextureId==no more texture space), just quit.
	if(!mngr->startTextureCompute(_CLodInstance))
		return;
	uint maxNumBmpToReset= 0;

	// **** For all skins which have a LodTexture setuped
	ItTransformSet	it= _Skins.begin();
	for(;it!=_Skins.end();it++)
	{
		// the skin should be a meshBaseInstance setuped to asyncTexturing
		CMeshBaseInstance	*mbi= dynamic_cast<CMeshBaseInstance*>(*it);
		if(mbi && mbi->getAsyncTextureMode() && mbi->Shape)
		{
			CMeshBase	*mb= (CMeshBase*)(IShape*)(mbi->Shape);
			// get the LodTexture info of this shape.
			const CLodCharacterTexture	*lodText= mb->getLodCharacterTexture();
			// if setuped
			if(lodText)
			{
				// Ok, compute influence of this instance on the Lod.

				// ---- Build all bmps of the instance with help of the asyncTextureManager
				uint	numMats= (uint)mbi->Materials.size();
				// 256 materials possibles for the lod Manager
				numMats= min(numMats, 256U);
				// for endTexturecompute
				maxNumBmpToReset= max(maxNumBmpToReset, numMats);
				// process each materials
				for(uint i=0;i<numMats;i++)
				{
					// get the manager bitmap to write to
					CLodCharacterTmpBitmap	&dstBmp= mngr->getTmpBitmap(uint8(i));

					// if the material stage 0 is not textured, or has not a valid async id, build the bitmap with a color.
					sint			asyncTextId= mbi->getAsyncTextureId(i,0);
					const CBitmap	*coarseBitmap= NULL;
					if(asyncTextId!=-1)
					{
						// get it from async manager
						coarseBitmap= asyncMngr->getCoarseBitmap(asyncTextId);
					}

					// So if we have no bmp here, build with material color, else build a texture
					if(!coarseBitmap)
					{
						dstBmp.build(mbi->Materials[i].getDiffuse());
					}
					else
					{
						dstBmp.build(*coarseBitmap);
					}
				}

				// ---- add the lodTextureInfo to the current texture computed
				mngr->addTextureCompute(_CLodInstance, *lodText);
			}
		}
	}

	// **** compile the process
	mngr->endTextureCompute(_CLodInstance, maxNumBmpToReset);

}


// ***************************************************************************
void		CSkeletonModel::setLodCharacterAnimId(uint animId)
{
	_CLodInstance.AnimId= animId;
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterAnimTime(TGlobalAnimationTime time)
{
	_CLodInstance.AnimTime= time;
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterWrapMode(bool wrapMode)
{
	_CLodInstance.WrapMode= wrapMode;
}


// ***************************************************************************
float		CSkeletonModel::computeDisplayLodCharacterPriority() const
{
	// if enabled
	if(_LodCharacterDistance>0 && _CLodInstance.ShapeId>=0)
	{
		CVector		globalPos;

		// Get object position, test visibility;
		// If has a skeleton ancestor, take his world position instead, because ours is invalid.
		if( _AncestorSkeletonModel != NULL)
		{
			// if the ancestore is clipped, quit
			if( !_AncestorSkeletonModel->isClipVisible() )
				return 0;
			// take ancestor world position
			globalPos= _AncestorSkeletonModel->getWorldMatrix().getPos();
		}
		else
		{
			// if the skeleton is clipped, quit
			if( !isClipVisible() )
				return 0;
			// take our world position
			globalPos= getWorldMatrix().getPos();
		}

		// compute distance from camera.
		float	dist= (getOwnerScene()->getClipTrav().CamPos - globalPos).norm();

		// compute priority
		return dist*_OOLodCharacterDistance;
	}
	else
		return 0;
}


// ***************************************************************************
void		CSkeletonModel::setDisplayLodCharacterFlag(bool displayCLod)
{
	// if enabled
	if(_LodCharacterDistance>0 && _CLodInstance.ShapeId>=0)
	{
		// If the flag has changed since last frame, must recompute bone Usage.
		if(_DisplayedAsLodCharacter != displayCLod)
			_BoneToComputeDirty= true;

		// set new state
		_DisplayedAsLodCharacter= displayCLod;
	}
}


// ***************************************************************************
void		CSkeletonModel::traverseRender()
{
	H_AUTO( NL3D_Skeleton_Render );

	// render as CLod, or render Skins.
	if(isDisplayedAsLodCharacter())
		renderCLod();
	else
		renderSkins();
}


// ***************************************************************************
void			CSkeletonModel::computeCLodVertexAlpha(CLodCharacterManager *mngr)
{
	// if shape id set.
	if(_CLodInstance.ShapeId<0)
		return;
	// get the lod shape,a nd check exist in the manager
	const CLodCharacterShape	*lodShape= mngr->getShape(_CLodInstance.ShapeId);
	if(lodShape)
	{
		// start process.
		//-----------------
		lodShape->startBoneAlpha(_CLodInstance.VertexAlphas);

		// build an Id map, from Skeleton Ids to the lodShapes ids. (because may be differents)
		static vector<sint>	boneMap;
		// reset to -1 (ie not found)
		boneMap.clear();
		boneMap.resize(Bones.size(), -1);
		uint i;
		// for all skeletons bones.
		for(i=0; i<boneMap.size(); i++)
		{
			boneMap[i]= lodShape->getBoneIdByName(Bones[i].getBoneName());;
		}

		// Parse all skins
		//-----------------
		ItTransformSet	it;
		for(it= _Skins.begin(); it!=_Skins.end(); it++)
		{
			CTransform	*skin= *it;

			// get array of bone used for this skin.
			const vector<sint32>	*skinUsage= skin->getSkinBoneUsage();
			// check correct skin
			if(skinUsage)
			{
				// For all bones used
				for(uint i=0; i<skinUsage->size(); i++)
				{
					// the id in the vector point to a bone in the skeleton. Hence use the boneMap to translate it
					// in the lodShape ids.
					sint	idInLod= boneMap[(*skinUsage)[i]];
					// only if id found in the lod shape
					if(idInLod>=0)
						// add color to this bone.
						lodShape->addBoneAlpha(idInLod, _CLodInstance.VertexAlphas);
				}

			}
		}

		// Parse all sticked objects
		//-----------------
		for(it= _StickedObjects.begin(); it!=_StickedObjects.end(); it++)
		{
			CTransform	*object= *it;

			// get on which bone this object is linked.
			// use the boneMap to translate id to lodShape id.
			sint	idInLod= boneMap[object->_FatherBoneId];

			// only if id found in the lod shape
			if(idInLod>=0)
				// add color to this bone.
				lodShape->addBoneAlpha(idInLod, _CLodInstance.VertexAlphas);
		}

	}

}


// ***************************************************************************
void			CSkeletonModel::updateSkinRenderLists()
{
	// If need to update array of skins to compute
	if(_SkinToRenderDirty)
	{
		uint	i;

		_SkinToRenderDirty= false;

		// Reset the LevelDetail.
		_LevelDetail.MinFaceUsed= 0;
		_LevelDetail.MaxFaceUsed= 0;
		// If must follow default MRM setup from skins.
		if(_DefaultMRMSetup)
		{
			_LevelDetail.DistanceCoarsest= 0;
			_LevelDetail.DistanceMiddle= 0;
			_LevelDetail.DistanceFinest= 0;
		}

		// reset Bone Sphere of skeleton.
		static	std::vector<bool>	sphereEmpty;
		sphereEmpty.clear();
		sphereEmpty.resize(Bones.size(), true);
		for(i=0;i<Bones.size();i++)
		{
			// Default sphere is centered on the bone pos, and has 0 radius.
			Bones[i]._MaxSphere.Center= CVector::Null;
			Bones[i]._MaxSphere.Radius= 0;
		}

		// Parse to count new size of the arrays, and to build MRM info, and bone Max sphere
		uint	opaqueSize= 0;
		uint	transparentSize= 0;
		uint	animDetailSize= 0;
		ItTransformSet		it;
		// also test if can support fast intersection
		_SupportFastIntersect= !_Skins.empty();
		for(it= _Skins.begin();it!=_Skins.end();it++)
		{
			CTransform	*skin= *it;

			// If the skin is hidden, don't add it to any list!
			if(skin->getVisibility()==CHrcTrav::Hide)
				continue;

			// if transparent, then must fill in transparent list.
			if(skin->isTransparent())
				transparentSize++;
			// else may fill in opaquelist. NB: for optimisation, don't add in opaqueList
			// if added to the transperent list (all materials are rendered)
			else if(skin->isOpaque())
				opaqueSize++;

			// if animDetailable, then must fill list
			if(skin->isAnimDetailable())
				animDetailSize++;

			// if the skin support MRM, then must update levelDetal number of faces
			CTransformShape		*trShape= dynamic_cast<CTransformShape*>(skin);
			if(trShape)
			{
				const	CMRMLevelDetail		*skinLevelDetail= trShape->getMRMLevelDetail();
				if(skinLevelDetail)
				{
					// Add Faces to the Skeleton level detail
					_LevelDetail.MinFaceUsed+= skinLevelDetail->MinFaceUsed;
					_LevelDetail.MaxFaceUsed+= skinLevelDetail->MaxFaceUsed;
					// MRM Max skin setup.
					if(_DefaultMRMSetup)
					{
						// Get the maximum distance setup (ie the one which degrades the less)
						_LevelDetail.DistanceCoarsest= max(_LevelDetail.DistanceCoarsest, skinLevelDetail->DistanceCoarsest);
						_LevelDetail.DistanceMiddle= max(_LevelDetail.DistanceMiddle, skinLevelDetail->DistanceMiddle);
						_LevelDetail.DistanceFinest= max(_LevelDetail.DistanceFinest, skinLevelDetail->DistanceFinest);
					}
				}
			}

			// Enlarge Bone BBox
			const std::vector<sint32>			*boneUsage= skin->getSkinBoneUsage();
			const std::vector<NLMISC::CBSphere>	*boneSphere= skin->getSkinBoneSphere();
			if(boneUsage && boneSphere)
			{
				nlassert(boneUsage->size()==boneSphere->size());
				for(i=0;i<boneUsage->size();i++)
				{
					const CBSphere	&sphere= (*boneSphere)[i];
					sint			boneId= (*boneUsage)[i];
					nlassert(boneId<(sint)Bones.size());
					// if valid boneId, and sphere not empty (ie not -1 radius)
					if(boneId>-1 && sphere.Radius>=0)
					{
						if(sphereEmpty[boneId])
						{
							sphereEmpty[boneId]= false;
							Bones[boneId]._MaxSphere= sphere;
						}
						else
						{
							Bones[boneId]._MaxSphere.setUnion(Bones[boneId]._MaxSphere, sphere);
						}
					}
				}
			}

			// Whole skeleton model Support Fast intersection only if all
			// displayed skins support skin intersection
			_SupportFastIntersect= _SupportFastIntersect && skin->supportIntersectSkin();
		}

		// MRM Max skin setup.
		if(_DefaultMRMSetup)
		{
			// compile LevelDetail.
			if(_LevelDetail.MaxFaceUsed==0)
				// build a bug-free level detail
				buildDefaultLevelDetail();
			else
				_LevelDetail.compileDistanceSetup();
		}

		// alloc array.
		_OpaqueSkins.clear();
		_TransparentSkins.clear();
		_AnimDetailSkins.clear();
		_OpaqueSkins.resize(opaqueSize);
		_TransparentSkins.resize(transparentSize);
		_AnimDetailSkins.resize(animDetailSize);

		// ReParse, to fill array.
		uint	opaqueId= 0;
		uint	transparentId= 0;
		uint	animDetailId= 0;
		for(it= _Skins.begin();it!=_Skins.end();it++)
		{
			CTransform	*skin= *it;

			// If the skin is hidden, don't add it to any list!
			if(skin->getVisibility()==CHrcTrav::Hide)
				continue;

			// if transparent, then must fill in transparent list.
			if(skin->isTransparent())
			{
				nlassert(transparentId<transparentSize);
				_TransparentSkins[transparentId++]= skin;
			}
			// else may fill in opaquelist. NB: for optimisation, don't add in opaqueList
			// if added to the transperent list (all materials are rendered)
			else if(skin->isOpaque())
			{
				nlassert(opaqueId<opaqueSize);
				_OpaqueSkins[opaqueId++]= skin;
			}

			// if animDetailable, then must fill list
			if(skin->isAnimDetailable())
			{
				nlassert(animDetailId<animDetailSize);
				_AnimDetailSkins[animDetailId++]= skin;
			}
		}

		// set the Transparency to the skeleton only if has at least one transparent skin
		setTransparency( transparentSize>0 );
	}
}


// ***************************************************************************
void			CSkeletonModel::buildDefaultLevelDetail()
{
	// Avoid divide by zero.
	_LevelDetail.MinFaceUsed= 0;
	_LevelDetail.MaxFaceUsed= 0;
	_LevelDetail.DistanceFinest= 1;
	_LevelDetail.DistanceMiddle= 2;
	_LevelDetail.DistanceCoarsest= 3;
	_LevelDetail.compileDistanceSetup();
}


// ***************************************************************************
void			CSkeletonModel::renderCLod()
{
	CRenderTrav			&renderTrav= getOwnerScene()->getRenderTrav();
	IDriver				*drv= renderTrav.getDriver();
	CScene				*scene= getOwnerScene();

	// Transparent pass? quit
	if(!renderTrav.isCurrentPassOpaque())
		return;

	// the lod manager. no op if not here
	CLodCharacterManager	*mngr= scene->getLodCharacterManager();
	if(!mngr)
		return;

	// Get global lighting on the instance. Suppose SunAmbient only.
	//=================
	const CLightContribution	*lightContrib;

	// the std case is to take my model lightContribution
	if(_AncestorSkeletonModel==NULL)
		lightContrib= &getSkeletonLightContribution();
	// but if skinned/sticked (directly or not) to a skeleton, take its.
	else
		lightContrib= &_AncestorSkeletonModel->getSkeletonLightContribution();

	// compute his main light contribution result. Try first with sun
	CRGBA	mainAmbient= scene->getSunAmbient();
	CRGBA	mainDiffuse= scene->getSunDiffuse();
	// modulate sun contribution
	mainDiffuse.modulateFromuiRGBOnly(mainDiffuse, lightContrib->SunContribution );
	CVector	mainLightDir= scene->getSunDirection();
	// Add ambient with Lod Emit
	mainAmbient.addRGBOnly(mainAmbient, _LodEmit);


	/* During night, and in the buildings, it may be better to use one of the other Points lights
		Test only with the first pointLight, for faster compute, even if It may fail in some cases.
	*/
	CPointLight	*mainPL= lightContrib->PointLight[0];
	if(mainPL)
	{
		CRGBA	plDiffuse;
		// get the diffuse of the pointLight, attenuated from distance and importance.
		plDiffuse.modulateFromuiRGBOnly(mainPL->getDiffuse(), lightContrib->AttFactor[0]);
		plDiffuse.A = 255;
		// compare the 2 diffuse
		uint	d0= mainDiffuse.R + mainDiffuse.G + mainDiffuse.B;
		uint	d1= plDiffuse.R + plDiffuse.G + plDiffuse.B;
		// if the pointLight is lighter, take it.
		if(d1>d0)
		{
			// leave ambient, but take diffuse and pointLight fake Direction
			mainDiffuse= plDiffuse;
			mainLightDir= getWorldMatrix().getPos() - mainPL->getPosition();
			mainLightDir.normalize();
		}
	}


	// compute colors of the lods.
	//=================
	// NB: even if texturing is sufficient, still important for AlphaTest.

	// If must recompute alpha because of change of skin added/deleted
	if(_CLodVertexAlphaDirty)
	{
		// recompute vertex alpha
		computeCLodVertexAlpha(mngr);
		// set _CLodVertexAlphaDirty to false.
		_CLodVertexAlphaDirty= false;
	}

	// render the Lod in the LodManager.
	//=================
	// render must have been intialized
	nlassert(mngr->isRendering());


	// add the instance to the manager.
	if(!mngr->addRenderCharacterKey(_CLodInstance, getWorldMatrix(),
		mainAmbient, mainDiffuse, mainLightDir) )
	{
		// If failed to add it because no more vertex space in the manager, retry.

		// close vertexBlock, compile render
		mngr->endRender();
		// and restart.
		mngr->beginRender(drv, renderTrav.CamPos);

		// retry. but no-op if refail.
		mngr->addRenderCharacterKey(_CLodInstance, getWorldMatrix(),
			mainAmbient, mainDiffuse, mainLightDir);
	}
}


// ***************************************************************************
void			CSkeletonModel::renderSkins()
{
	// Render skins according to the pass.
	CRenderTrav			&rdrTrav= getOwnerScene()->getRenderTrav();
	// get a ptr on the driver
	IDriver				*drv= rdrTrav.getDriver();
	nlassert(drv);


	// Compute the levelOfDetail
	float	alphaMRM= _LevelDetail.getLevelDetailFromPolyCount(getNumTrianglesAfterLoadBalancing());

	// force normalisation of normals..
	bool	bkupNorm= drv->isForceNormalize();
	drv->forceNormalize(true);


	// rdr good pass
	if(rdrTrav.isCurrentPassOpaque())
	{
		// Compute in Pass Opaque only the light contribution.
		// Easier for skeleton: suppose lightable, no local attenuation

		// the std case is to take my model lightContribution
		if(_AncestorSkeletonModel==NULL)
			setupCurrentLightContribution(&_LightContribution, false);
		// but if sticked (directly or not) to a skeleton, take its.
		else
			setupCurrentLightContribution(&_AncestorSkeletonModel->_LightContribution, false);


		// Activate Driver setup: light and modelMatrix
		changeLightSetup( &rdrTrav );
		rdrTrav.getDriver()->setupModelMatrix(getWorldMatrix());


		// Render all totaly opaque skins.
		renderSkinList(_OpaqueSkins, alphaMRM);
	}
	else
	{
		// NB: must have some transparent skins, since thee skeletonModel is traversed in the transparent pass.

		// Activate Driver setup: light and modelMatrix
		changeLightSetup( &rdrTrav );
		rdrTrav.getDriver()->setupModelMatrix(getWorldMatrix());


		// render all opaque/transparent skins
		renderSkinList(_TransparentSkins, alphaMRM);
	}


	// bkup force normalisation.
	drv->forceNormalize(bkupNorm);
}


// ***************************************************************************
void			CSkeletonModel::renderSkinList(NLMISC::CObjectVector<CTransform*, false> &skinList, float alphaMRM)
{
	CRenderTrav			&rdrTrav= getOwnerScene()->getRenderTrav();

	// if the SkinManager is not possible at all, just rendered the std way.
	if( !rdrTrav.getMeshSkinManager() )
	{
		for(uint i=0;i<skinList.size();i++)
		{
			skinList[i]->renderSkin(alphaMRM);
		}
	}
	else
	{
		// get the meshSkinManager
		CVertexStreamManager	&meshSkinManager= *rdrTrav.getMeshSkinManager();

		// array (rarely allocated) of skins with grouping support
		static	std::vector<CTransform*>	skinsToGroup;
		static	std::vector<uint>			baseVertices;
		skinsToGroup.clear();
		baseVertices.clear();

		// get the maxVertices the manager support
		uint	maxVertices= meshSkinManager.getMaxVertices();
		uint	vertexSize= meshSkinManager.getVertexSize();

		// render any skins which do not support SkinGrouping, and fill array of skins to group
		for(uint i=0;i<skinList.size();i++)
		{
			// If don't support, or if too big to fit in the manager, just renderSkin()
			if(!skinList[i]->supportSkinGrouping())
			{
				H_AUTO( NL3D_Skin_NotGrouped );
				skinList[i]->renderSkin(alphaMRM);
			}
			else
			{
				skinsToGroup.push_back(skinList[i]);
			}
		}

		H_AUTO( NL3D_Skin_Grouped );

		// For each skin, have an index which gives the decal of the vertices in the buffer
		baseVertices.resize(skinsToGroup.size());

		// while there is skin to render in group
		uint	skinId= 0;
		while(skinId<skinsToGroup.size())
		{
			// space left in the manager
			uint	remainingVertices= maxVertices;
			uint	currentBaseVertex= 0;


			// First pass, fill The VB.
			//------------
			// lock buffer
			uint8	*vbDest= meshSkinManager.lock();

			// For all skins until the buffer is full
			uint	startSkinId= skinId;
			while(skinId<skinsToGroup.size())
			{
				// if success to fill the AGP
				sint	numVerticesAdded= skinsToGroup[skinId]->renderSkinGroupGeom(alphaMRM, remainingVertices,
					vbDest + vertexSize*currentBaseVertex );
				// -1 means that this skin can't render because no space left for her. Then stop for this block
				if(numVerticesAdded==-1)
					break;
				// Else ok, get the currentBaseVertex for this skin
				baseVertices[skinId]= currentBaseVertex;
				// and jump to the next place
				currentBaseVertex+= numVerticesAdded;
				remainingVertices-= numVerticesAdded;

				// go to the next skin
				skinId++;
			}

			// release buffer. ATI: release only vertices used.
			meshSkinManager.unlock(currentBaseVertex);

			// Second pass, render the primitives.
			//------------
			meshSkinManager.activate();

			/* Render any primitives that are not specular. Group specular ones into specularRdrPasses.
				NB: this speed a lot (specular setup is heavy)!
			*/
			static std::vector<CSkinSpecularRdrPass>	specularRdrPasses;
			specularRdrPasses.clear();
			for(uint i=startSkinId;i<skinId;i++)
			{
				// render the skin in the current buffer
				skinsToGroup[i]->renderSkinGroupPrimitives(baseVertices[i], specularRdrPasses, i);
			}

			// If any skin Specular rdrPass to render
			if(!specularRdrPasses.empty())
			{
				// Sort by Specular Map. HTimerInfo: take 0.0% time
				sort(specularRdrPasses.begin(), specularRdrPasses.end());

				// Batch Specular! HTimerInfo: take 0.2%
				rdrTrav.getDriver()->startSpecularBatch();

				// Render all of them
				for(uint i=0;i<specularRdrPasses.size();i++)
				{
					CSkinSpecularRdrPass	&specRdrPass= specularRdrPasses[i];
					// render the associated skin in the current buffer
					skinsToGroup[specRdrPass.SkinIndex]->renderSkinGroupSpecularRdrPass(specRdrPass.RdrPassIndex);
				}

				// End Batch Specular! HTimerInfo: take 0.0%
				rdrTrav.getDriver()->endSpecularBatch();
			}


			// End of this block, swap to the next buffer. HTimerInfo: take 0.0%
			meshSkinManager.swapVBHard();
		}
	}
}


// ***************************************************************************
float			CSkeletonModel::getNumTriangles (float distance)
{
	// If the skeleton is displayed as a CLod suppose 0 triangles.
	if( isDisplayedAsLodCharacter() )
		return 0;
	else
		// NB: this is an approximation, but this is continious.
		return _LevelDetail.getNumTriangles(distance);
}

// ***************************************************************************
void			CSkeletonModel::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	// check input.
	if(distanceFinest<0)	return;
	if(distanceMiddle<=distanceFinest)	return;
	if(distanceCoarsest<=distanceMiddle)	return;

	// Change.
	_LevelDetail.DistanceFinest= distanceFinest;
	_LevelDetail.DistanceMiddle= distanceMiddle;
	_LevelDetail.DistanceCoarsest= distanceCoarsest;

	// compile
	_LevelDetail.compileDistanceSetup();

	// Never more use MAX skin setup.
	_DefaultMRMSetup= false;
}


// ***************************************************************************
void			CSkeletonModel::resetDefaultMRMDistanceSetup()
{
	_DefaultMRMSetup= true;

	// Must use Skins linked to know the MRM setup.
	dirtSkinRenderLists();
}


// ***************************************************************************
bool			CSkeletonModel::computeRenderedBBox(NLMISC::CAABBox &bbox, bool computeInWorld)
{
	// reset bbox
	CAABBox		tmpBBox;
	tmpBBox.setCenter(CVector::Null);
	tmpBBox.setHalfSize(CVector::Null);
	bool	empty= true;

	// Not visible => empty bbox
	if(!isClipVisible())
		return false;

	// For all bones
	uint	i;
	for(i=0;i<Bones.size();i++)
	{
		if(isBoneComputed(i))
		{
			CVector	pos;
			if(computeInWorld)
				pos= Bones[i].getWorldMatrix().getPos();
			else
				pos= Bones[i].getLocalSkeletonMatrix().getPos();
			if(empty)
			{
				empty= false;
				tmpBBox.setCenter(pos);
			}
			else
				tmpBBox.extend(pos);
		}
	}

	// End!
	if(!empty)
	{
		bbox= tmpBBox;
		return true;
	}
	else
		return false;
}

// ***************************************************************************
void CSkeletonModel::getWorldMaxBoneSpheres(std::vector<NLMISC::CBSphere> &dest) const
{
	dest.clear();
	// Not visible => empty bbox
	if(!isClipVisible())
		return;
	dest.resize(_BoneToCompute.size());
	for(uint i=0;i<_BoneToCompute.size();i++)
	{
		CBone			*bone= _BoneToCompute[i].Bone;
		bone->_MaxSphere.applyTransform(bone->getWorldMatrix(), dest[i]);
	}
}

// ***************************************************************************
bool		CSkeletonModel::computeRenderedBBoxWithBoneSphere(NLMISC::CAABBox &bbox, bool computeInWorld)
{
	// Not visible => empty bbox
	if(!isClipVisible())
		return false;

	if(_BoneToCompute.empty())
		return false;

	if (_Skins.empty())
		return false;

	updateSkinRenderLists();

	// **** Compute The BBox with Bones of the skeleton
	CVector		minBB(0.f, 0.f, 0.f), maxBB(0.f, 0.f, 0.f);
	for(uint i=0;i<_BoneToCompute.size();i++)
	{
		CBone			*bone= _BoneToCompute[i].Bone;
		// compute the world / local sphere
		const	CMatrix	&boneMat = computeInWorld ? bone->getWorldMatrix() : bone->getLocalSkeletonMatrix();
		CBSphere		sphere;
		bone->_MaxSphere.applyTransform(boneMat, sphere);
		// compute bone min max bounding cube.
		CVector		minBone, maxBone;
		minBone= maxBone= sphere.Center;
		float	r= sphere.Radius;


		minBone.x-= r;
		minBone.y-= r;
		minBone.z-= r;
		maxBone.x+= r;
		maxBone.y+= r;
		maxBone.z+= r;


		// set or extend
		if(i==0)
		{
			minBB= minBone;
			maxBB= maxBone;
		}
		else
		{
			minBB.minof(minBB, minBone);
			maxBB.maxof(maxBB, maxBone);
		}
	}

	// build the bbox
	bbox.setMinMax(minBB, maxBB);
	return true;
}


// ***************************************************************************
bool			CSkeletonModel::computeCurrentBBox(NLMISC::CAABBox &bbox, bool forceCompute /* = false*/, bool computeInWorld)
{
	// animate all bones channels (detail only channels). don't bother cur lod state.
	CChannelMixer	*chanmix= getChannelMixer();
	if (chanmix)
	{
		// Force detail evaluation.
		chanmix->resetEvalDetailDate();
		chanmix->eval(true, 0);
		chanmix->resetEvalDetailDate();
	}
	// compute all skeleton bones
	computeAllBones(CMatrix::Identity);

	// reset bbox
	CAABBox		tmpBBox;
	tmpBBox.setCenter(CVector::Null);
	tmpBBox.setHalfSize(CVector::Null);
	bool	empty= true;

	// For all bones
	uint	i;
	for(i=0;i<Bones.size();i++)
	{
		// Is the bone used ?? (whatever bone lod, or CLod state)
		uint16	mustCompute = forceCompute ? 1 : _BoneUsage[i].Usage | _BoneUsage[i].ForcedUsage | _BoneUsage[i].CLodForcedUsage;

		// If the bone is used.
		if(mustCompute)
		{
			CVector	pos;
			if(computeInWorld)
				pos= Bones[i].getWorldMatrix().getPos();
			else
				pos= Bones[i].getLocalSkeletonMatrix().getPos();
			if(empty)
			{
				empty= false;
				tmpBBox.setCenter(pos);
			}
			else
				tmpBBox.extend(pos);
		}
	}

	// End!
	if(!empty)
	{
		bbox= tmpBBox;
		return true;
	}
	else
		return false;
}


// ***************************************************************************
void		CSkeletonModel::getLightHotSpotInWorld(CVector &modelPos, float &modelRadius) const
{
	// If the bone 0 is computed (pelvis), then return its worldMatrix
	if(isBoneComputed(0))
	{
		modelPos= Bones[0].getWorldMatrix().getPos();
	}
	else
	{
		/* Else return the skeleton pos. NB: this seems useless since bone 0 not computed means no Skins.
			But lighting computation is still done and may use a VisualCollisionEntity.
			This system cache some infos according to position. This is why we must return a position
			near the skeleton (else cache crash each frame => slowdown...)
		*/
		modelPos= _WorldMatrix.getPos();
	}

	// Consider Skeletons as not big lightable
	modelRadius= 0;
}


// ***************************************************************************
void		CSkeletonModel::setBoneAnimCtrl(uint boneId, IAnimCtrl *ctrl)
{
	if(boneId>=Bones.size())
		return;

	CBone	&bone= Bones[boneId];

	// Update refCount
	if(ctrl && !bone._AnimCtrl)
		_AnimCtrlUsage++;
	else if(!ctrl && bone._AnimCtrl)
		_AnimCtrlUsage--;

	// set
	bone._AnimCtrl= ctrl;
}


// ***************************************************************************
IAnimCtrl	*CSkeletonModel::getBoneAnimCtrl(uint boneId) const
{
	if(boneId>=Bones.size())
		return NULL;

	return Bones[boneId]._AnimCtrl;
}


// ***************************************************************************
bool		CSkeletonModel::fastIntersect(const NLMISC::CVector &p0, const NLMISC::CVector &dir, float &dist2D, float &distZ, bool computeDist2D)
{
	if(!_SupportFastIntersect)
		return false;

	// no intersection by default
	dist2D= FLT_MAX;
	distZ= FLT_MAX;

	// The skinning must be done in final RaySpace.
	CMatrix		toRaySpace;
	// compute the ray matrix
	CVector	dirn= dir;
	if(dirn.isNull())
		dirn= CVector::K;
	dirn.normalize();
	toRaySpace.setArbitraryRotK(dirn);
	toRaySpace.setPos(p0);
	// The skinning must be done in ray space: (RayMat-1)*skelWorldMatrix;
	toRaySpace.invert();
	toRaySpace*= getWorldMatrix();

	// displayed as a CLod?
	if(isDisplayedAsLodCharacter())
	{
		// must do the test with the CLod, because Bones are not animated at all (hence skinning would be false)
		CLodCharacterManager	*mngr= getOwnerScene()->getLodCharacterManager();
		if(!mngr)
			return false;

		// test the instance
		if(!mngr->fastIntersect(_CLodInstance, toRaySpace, dist2D, distZ, computeDist2D))
			return false;
	}
	else
	{
		// For all skins
		ItTransformSet		it;
		for(it= _Skins.begin();it!=_Skins.end();it++)
		{
			CTransform	*skin= *it;

			// If the skin is hidden, don't test intersection!
			if(skin->getVisibility()==CHrcTrav::Hide)
				continue;

			if(!skin->supportIntersectSkin())
				continue;

			// compute intersection with this skin
			float	skinDist2D, skinDistZ;
			if(skin->intersectSkin(toRaySpace, skinDist2D, skinDistZ, computeDist2D))
			{
				// true intersection found?
				if(skinDist2D==0)
				{
					dist2D= 0;
					distZ= min(distZ, skinDistZ);
				}
				// else lower the distance to the skins?
				else if(dist2D>0)
				{
					dist2D= min(dist2D, skinDist2D);
				}
			}
		}
	}

	// no intersection found? set Z to 0 (to be clean)
	if(dist2D>0)
		distZ= 0;

	return true;
}

// ***************************************************************************
void		CSkeletonModel::remapSkinBones(const std::vector<string> &bonesName, std::vector<sint32> &bonesId, std::vector<uint> &remap)
{
	// Resize boneId to the good size.
	bonesId.resize(bonesName.size());
	remap.resize(bonesName.size());

	// **** For each bones, compute remap
	for (uint bone=0; bone<remap.size(); bone++)
	{
		// Look for the bone
		sint32 boneId = getBoneIdByName (bonesName[bone]);

		// Setup the _BoneId.
		bonesId[bone]= boneId;

		// Bones found ?
		if (boneId != -1)
		{
			// Set the bone id
			remap[bone] = (uint32)boneId;
		}
		else
		{
			// Put id 0
			remap[bone] = 0;

			// Error
			nlwarning ("Bone %s not found in the skeleton.", bonesName[bone].c_str());
		}
	}
}

// ***************************************************************************
// ***************************************************************************
// ShadowMap
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CSkeletonModel::generateShadowMap(const CVector &lightDir)
{
	H_AUTO( NL3D_Skeleton_GenerateShadow );

	// get the driver for Texture Render
	CScene			*scene= getOwnerScene();
	CRenderTrav		&renderTrav= scene->getRenderTrav();
	IDriver			*driver= renderTrav.getAuxDriver();

	if(!Shape)
		return;

	// update ShadowMap data if needed.
	// ****
	updateShadowMap(driver);

	// compute the ProjectionMatrix.
	// ****

	// Compute the BBox in World, with bounding Box of Bones, and with BoundingBox of sticked Objects
	CAABBox		bbWorld;
	computeWorldBBoxForShadow(bbWorld);


	// Here the bbox is defined in world, hence must remove the World Pos.
	CMatrix		localPosMatrix;
	localPosMatrix.setPos(-getWorldMatrix().getPos());

	// setup cameraMatrix with BBox and Enlarge For 1 pixel
	CMatrix		cameraMatrix;
	_ShadowMap->buildCasterCameraMatrix(lightDir, localPosMatrix, bbWorld, cameraMatrix);


	// Render.
	// ****
	// setup the orhtogonal frustum and viewMatrix to include all the object.
	driver->setFrustum(0,1,0,1,0,1,false);
	driver->setupViewMatrix(cameraMatrix.inverted());

	// render the Skinned meshs, and also the Sticked Objects/Skeletons
	CMaterial	&castMat= renderTrav.getShadowMapManager().getCasterShadowMaterial();
	renderIntoSkeletonShadowMap(this, castMat);

	// Infos.
	// ****

	// Compute the BackPoint: the first point to be shadowed.
	CVector		backPoint= bbWorld.getCenter();
	// get the 3/4 bottom of the shape
	backPoint.z-= bbWorld.getHalfSize().z/2;
	// Use the 3/4 bottom of the BBox minus the light direction in XY.
	CVector	ldir= lightDir;
	ldir.z= 0;
	ldir.normalize();
	// NB: This way seems to works quite well, even if the worldBBox is changing every frame.
	float	lenXY= (CVector(bbWorld.getHalfSize().x, bbWorld.getHalfSize().y, 0)).norm();
	backPoint-= ldir*lenXY;
	// localPos.
	backPoint-= getWorldMatrix().getPos();

	// Compute LocalProjectionMatrix and other infos from cameraMatrix and backPoint?
	_ShadowMap->buildProjectionInfos(cameraMatrix, backPoint, getShadowMapMaxDepth());
}

// ***************************************************************************
CShadowMap	*CSkeletonModel::getShadowMap()
{
	return _ShadowMap;
}

// ***************************************************************************
void			CSkeletonModel::createShadowMap()
{
	// create the shadowMap
	if(!_ShadowMap)
	{
		_ShadowMap= new CShadowMap(&getOwnerScene()->getRenderTrav().getShadowMapManager());
		getOwnerScene()->registerShadowCasterToList(this);
	}
}

// ***************************************************************************
void			CSkeletonModel::deleteShadowMap()
{
	if(_ShadowMap)
	{
		delete _ShadowMap;
		_ShadowMap= NULL;
		getOwnerScene()->unregisterShadowCasterToList(this);
	}
}

// ***************************************************************************
void		CSkeletonModel::updateShadowMap(IDriver * /* driver */)
{
	nlassert(_ShadowMap);

	// create/update texture
	if(_ShadowMap->getTextureSize()!=getOwnerScene()->getShadowMapTextureSize())
	{
		_ShadowMap->initTexture(getOwnerScene()->getShadowMapTextureSize());
	}
}


// ***************************************************************************
void		CSkeletonModel::renderShadowSkins(CMaterial &castMat)
{
	H_AUTO( NL3D_Skin_RenderShadow );

	CRenderTrav			&rdrTrav= getOwnerScene()->getRenderTrav();
	// Render Shadow in auxiliary driver.
	IDriver				*driver= rdrTrav.getAuxDriver();

	// if the SkinManager is not possible at all, just rendered the std way
	if( !rdrTrav.getShadowMeshSkinManager() )
	{
		// can occurs?????
		// ABORT!! ...  avoid Mesh Shadowing (free shadowMap)? Replace with a dummy Shadow?
		// For now, no-op...
	}
	else
	{
		uint	i;

		// get the meshSkinManager
		CVertexStreamManager	&meshSkinManager= *rdrTrav.getShadowMeshSkinManager();

		// array (rarely allocated) of skins with grouping support
		static	std::vector<CTransform*>	skinsToGroup;
		static	std::vector<uint>			baseVertices;
		skinsToGroup.clear();
		baseVertices.clear();

		// get the maxVertices the manager support
		uint	maxVertices= meshSkinManager.getMaxVertices();
		uint	vertexSize= meshSkinManager.getVertexSize();

		// fill array of skins to group (suppose all support else won't be rendered)
		for(i=0;i<_OpaqueSkins.size();i++)
		{
			if(_OpaqueSkins[i]->supportShadowSkinGrouping())
				skinsToGroup.push_back(_OpaqueSkins[i]);
		}
		for(i=0;i<_TransparentSkins.size();i++)
		{
			if(_TransparentSkins[i]->supportShadowSkinGrouping())
				skinsToGroup.push_back(_TransparentSkins[i]);
		}

		// For each skin, have an index which gives the decal of the vertices in the buffer
		baseVertices.resize(skinsToGroup.size());

		// while there is skin to render in group
		uint	skinId= 0;
		while(skinId<skinsToGroup.size())
		{
			// space left in the manager
			uint	remainingVertices= maxVertices;
			uint	currentBaseVertex= 0;

			// First pass, fill The VB.
			//------------
			// lock buffer
			uint8	*vbDest= meshSkinManager.lock();

			// For all skins until the buffer is full
			uint	startSkinId= skinId;
			while(skinId<skinsToGroup.size())
			{
				// if success to fill the AGP
				sint	numVerticesAdded= skinsToGroup[skinId]->renderShadowSkinGeom(remainingVertices,
					vbDest + vertexSize*currentBaseVertex );
				// -1 means that this skin can't render because no space left for her. Then stop for this block
				if(numVerticesAdded==-1)
					break;
				// Else ok, get the currentBaseVertex for this skin
				baseVertices[skinId]= currentBaseVertex;
				// and jump to the next place
				currentBaseVertex+= numVerticesAdded;
				remainingVertices-= numVerticesAdded;

				// go to the next skin
				skinId++;
			}

			// release buffer. ATI: release only vertices used.
			meshSkinManager.unlock(currentBaseVertex);

			// Second pass, render the primitives.
			//------------
			meshSkinManager.activate();

			// Render any primitives
			for(uint i=startSkinId;i<skinId;i++)
			{
				// render the skin in the current buffer
				skinsToGroup[i]->renderShadowSkinPrimitives(castMat, driver, baseVertices[i]);
			}

			// End of this block, swap to the next buffer
			meshSkinManager.swapVBHard();
		}
	}
}


// ***************************************************************************
bool		CSkeletonModel::computeWorldBBoxForShadow(NLMISC::CAABBox &worldBB)
{
	uint	i;

	// If even not visible, no-op
	if(!isHrcVisible() || !Shape)
		return false;

	// **** Compute The BBox with Bones of the skeleton
	CVector		minBB(0.f, 0.f, 0.f), maxBB(0.f, 0.f, 0.f);
	for(i=0;i<_BoneToCompute.size();i++)
	{
		CBone			*bone= _BoneToCompute[i].Bone;
		// compute the world sphere
		const	CMatrix	&worldMat= bone->getWorldMatrix();
		CBSphere		worldSphere;
		bone->_MaxSphere.applyTransform(worldMat, worldSphere);
		// compute bone min max bounding cube.
		CVector		minBone, maxBone;
		minBone= maxBone= worldSphere.Center;
		float	r= worldSphere.Radius;
		minBone.x-= r;
		minBone.y-= r;
		minBone.z-= r;
		maxBone.x+= r;
		maxBone.y+= r;
		maxBone.z+= r;
		// set or extend
		if(i==0)
		{
			minBB= minBone;
			maxBB= maxBone;
		}
		else
		{
			minBB.minof(minBB, minBone);
			maxBB.maxof(maxBB, maxBone);
		}
	}
	// build the bbox
	worldBB.setMinMax(minBB, maxBB);
	/*
	// Fake Version. Faster (-0.2 ms for 8 compute each frame) but false.
	for(i=0;i<_BoneToCompute.size();i++)
	{
		CBone			*bone= _BoneToCompute[i].Bone;
		const	CMatrix	&worldMat= bone->getWorldMatrix();
		if(i==0)
			worldBB.setCenter(worldMat.getPos());
		else
			worldBB.extend(worldMat.getPos());
	}
	worldBB.setHalfSize(worldBB.getHalfSize() *1.5f);
	*/


	// **** Add to this bbox the ones of the Sticked objects.
	ItTransformSet	it;
	for(it= _StickedObjects.begin();it!=_StickedObjects.end();it++)
	{
		CTransform	*stickModel= *it;
		// Do the same for this son (NB: recurs, may be a skeleton too!!)
		CAABBox		stickBB;
		if(stickModel->computeWorldBBoxForShadow(stickBB))
		{
			// Make union of the 2
			worldBB= CAABBox::computeAABBoxUnion(worldBB, stickBB);
		}
	}

	// Done!
	return true;
}

// ***************************************************************************
void		CSkeletonModel::renderIntoSkeletonShadowMap(CSkeletonModel *rootSkeleton, CMaterial	&castMat)
{
	// If even not visible, no-op
	if(!isHrcVisible() || !Shape)
		return;

	// render into aux Driver
	IDriver			*driver= getOwnerScene()->getRenderTrav().getAuxDriver();

	// **** Render the Skeleton Skins
	// The model Matrix is special here. It must be the Skeleton World Matrix, minus The Root Skeleton pos.
	CMatrix		localPosMatrix;
	localPosMatrix.setRot( getWorldMatrix() );
	// NB: if this==rootSkeleton, then the final pos will be CVector::Null
	localPosMatrix.setPos( getWorldMatrix().getPos() - rootSkeleton->getWorldMatrix().getPos() );
	driver->setupModelMatrix(localPosMatrix);

	// render the skins.
	renderShadowSkins(castMat);

	// **** Render The Sticked Objects.
	ItTransformSet	it;
	for(it= _StickedObjects.begin();it!=_StickedObjects.end();it++)
	{
		CTransform	*stickModel= *it;
		stickModel->renderIntoSkeletonShadowMap(rootSkeleton, castMat);
	}
}



} // NL3D





















