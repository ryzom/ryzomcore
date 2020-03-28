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

#ifndef NL_TRANSFORM_H
#define NL_TRANSFORM_H

#include "nel/3d/hrc_trav.h"
#include "nel/3d/track.h"
#include "nel/3d/transformable.h"
#include "nel/3d/animated_value.h"
#include "nel/3d/channel_mixer.h"
#include "nel/misc/matrix.h"
#include "nel/misc/matrix.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/light_contribution.h"
#include "nel/3d/lighting_manager.h"
#include "nel/misc/class_id.h"
#include "nel/3d/fast_ptr_list.h"


namespace	NLMISC
{
	class	CAABBox;
};


namespace	NL3D
{

using NLMISC::CRGBA;


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;
using NLMISC::CAABBox;


class	CSkeletonModel;
class	CInstanceGroup;
class	ILogicInfo;
class	CLoadBalancingGroup;
class	CSkinSpecularRdrPass;
class	CShadowMap;
class	CMaterial;
class	IDriver;
class CInstanceUser;

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		TransformId=NLMISC::CClassId(0x174750cb, 0xf952024);


// ***************************************************************************
/**
 * A basic node which provide an animatable matrix (ITransformable).
 * May be derived for each node who want to support such a scheme (CCamera, CLight, CInstance ... )
 *
 * CTransform ALWAYS herit scale from fathers! (joints skeleton may not...) (nbyoyo: else, this breaks the update() system).
 *
 * CTransform Default tracks are identity (derived class may change this).
 *
 * CTransform by default IS NOT RENDERABLE. ie never inserted in renderList.
 *	Deriver should call setIsRenderable(true) to make the model renderable
 *
 * CTransform by default IS NOT ANIMDETAIL-able. ie never inserted in anim detail list.
 *	It is automatically inserted in anim detail list if registerToChannelMixer() is called.
 *	Deriver should call setIsForceAnimdetail(true) to make the model always anim-detail-ed
 *
 * CTransform by default IS NOT LOADBALANCE-able. ie never inserted into list for LoadBlancing.
 *	Deriver should call setIsLoadbalancable(true) to support this traversal.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTransform : public ITransformable
{
public:
	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();

	/// get the scene which has created us
	CScene			*getOwnerScene() const {return _OwnerScene;}

public:

	/// \name Model updating/traversing features
	// @{

	/** This function update the model (called by CScene::updateModels())
	 * Deriver Must :
	 *	- call BaseClass::update() (eg: CTransform::update()).
	 *	- test if something is different (eg: animation modification). Then update Model information (eg compute new Matrix).
	 *
	 * The default behavior is to update transform Matrix etc...
	 */
	virtual void	update();

	/**
	 *	Extra init for a model. this method is called by the framework at the very end of CScene::createModel()
	 *	Warning! if the model is a CTransformShape, then when initModel() is called, Shape and other related member/setup
	 *	of IShape::createInstance() are not yet done (because createModel() is called at the beginning in createInstance()).
	 *
	 *	Because initModel() is called at the very end, deriver could implement anything like creating other models,
	 *	but not deleting this model...
	 *
	 *	Default behavior is to do nothing.
	 */
	virtual void	initModel();

	/// Each method is called in its associated traversal
	virtual void	traverseHrc();
	/** The base traverseClip method.
	 * The behavior is to:
	 *	- test if _WorldVis is visible.
	 *	- test if is clipped with clip() OR IF SKELETON MODEL, USE SKELETON MODEL clip!!
	 *	- if visible and not clipped, set \c _Visible=true (else false). and
	 *		- add the CTransform* to the ClipTrav list
	 *	- if _Visible==true, and renderable, add it to the RenderTraversal: \c RenderTrav->addRenderModel(model);
	 *	- always traverseSons(), to clip the sons.
	 */
	virtual void	traverseClip();
	/// call updateWorldMatrixFromFather(), then traverseAnimDetailWithoutUpdateWorldMatrix()
	virtual void	traverseAnimDetail();
	/// no-op by default
	virtual void	traverseLoadBalancing();
	/// traverse the lightedModel per default: recompute LightContribution is isLightable()
	virtual void	traverseLight();
	/// no-op by default
	virtual void	traverseRender();

	/// clip method called by traverseClip(). deafult is always visible
	virtual	bool	clip()
	{
		return true;
	}

	/// Called at RenderTrav to profile current render. no-op per default
	virtual	void	profileRender();


	// @}

	// build matching user interface object (a class derived from CInstanceUser)
	virtual class UTransform *buildMatchingUserInterfaceObject ();

public:

	/** Accessors for opacity/transparency
	 */
	void			setTransparency(bool v);
	void			setOpacity(bool v);
	// no op for non multi-lod object, else, force the opacity /
	void			setBypassLODOpacityFlag(bool bypass);
	bool			getBypassLODOpacityFlag() const{ return getStateFlag(BypassLODOpacity) != 0;  }
	// return a non-zero value if true
	uint32			isOpaque() { return getStateFlag(IsOpaque); }
	uint32			isTransparent() { return getStateFlag(IsTransparent); }


	/** Set the current layer for this transform.
	  * Typically, this is used to sort transparent objects. Isn't used with solid objects.
	  * For now :
	  * Layer 0 is for underwater
	  * Layer 1 is for water surfaces
	  * Layer 2 is for object above water
	  */
	void			setOrderingLayer(uint layer) { _OrderingLayer = uint8(layer); }

	/// Get the ordering layer
	uint			getOrderingLayer() const { return _OrderingLayer; }

	/// Set priority for transparent ordering. When sorting before drawing, priority is taken in account before distance, so
	//  an object with distance 10 and priority 1 will be displayed before any object with distance 1 and priority 0.
	// IMPORTANT : priority is clamped by the number of priorities defined in the scene. By default there's only one priority of 0
	// that is possible, so priority ordering doesn't actually occurs.
	void			setTransparencyPriority(uint8 priority) { _TransparencyPriority = priority; }
	uint8			getTransparencyPriority() const { return _TransparencyPriority; }

	/// Hide the object and his sons.
	void			hide();
	/// Show the objet and his sons.
	void			show();
	/*
	 *	Enable / disable user clipping. If enable, the transform is not clipped into the engine.
	 *  The user has to use show / hide to clip or not the transform.
	 */
	void			setUserClipping(bool enable);
	/// Return the user clipping state
	bool			getUserClipping() const;
	/// herit the visibility from his father. (default behavior).
	void			heritVisibility();
	/// Get the local visibility state.
	CHrcTrav::TVisibility	getVisibility() {return Visibility;}
	/// Get the skeleton model. Returnr NULL in normal mode.
	CSkeletonModel*			getSkeletonModel () const {return _FatherSkeletonModel;}
	// Get the ancestor skeleton, (father skeleton that has the least depth in the model tree), or NULL if none
	CSkeletonModel			*getAncestorSkeletonModel() const { return _AncestorSkeletonModel; }

	/// \name Hierarchy linking
	// @{
	/** link son to this in Hierarchy traversal
	  * NB: link does nothing if the son node is HRC frozen */
	void			hrcLinkSon(CTransform *son);
	/** unlink this from any Father in Hrc. No-op if no parent
	  * NB: unlink does nothing if the node is HRC frozen */
	void			hrcUnlink();
	// get the Hrc parent if any (else NULL)
	CTransform		*hrcGetParent() const {return _HrcParent;}
	// get Sons links. NB: indices are no more valid after an hrcUnlink()
	uint			hrcGetNumChildren() const {return _HrcSons.size();}
	CTransform		*hrcGetChild(uint index) const;
	// @}


	/// \name Clip linking
	// @{
	// Add a link from me to son for Clip Graph (no-op if son==this)
	void			clipAddChild(CTransform *son);
	// Remove a link from me to son for Clip Graph (no-op if not found)
	void			clipDelChild(CTransform *son);
	// unlink from all parent clip
	void			clipUnlinkFromAll();
	// get Parents links. NB: indices are no more valid after a clipDelChild()
	uint			clipGetNumParents() const {return (uint)_ClipParents.size();}
	CTransform		*clipGetParent(uint index) const;
	// get Sons links. NB: indices are no more valid after a clipDelChild()
	uint			clipGetNumChildren() const {return _ClipSons.size();}
	CTransform		*clipGetChild(uint index) const;
	// @}


	/// \name Derived from ITransformable.
	// @{
	/// Default Track Values are identity (pos,pivot= 0, scale= 1, rots=0).
	virtual ITrack* getDefaultTrack (uint valueId);
	/** register transform channels (in global anim mode).
	  * \see	setChannelMixerOwnerShip
	  */
	virtual void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);
	// @}

	/** This force gives this object ownership of the channel mixer it is registered to, so it will delete it when the dtor is called.
	  * It should be called AFTER this object has been registered to a channel mixer, because a new registration will broke the ownership.
	  * This is useful for automatic animations, when there's no owner of the channel mixer that could delete it.
	  */
	void			setChannelMixerOwnerShip(bool enable = true)	{ setStateFlag(IsDeleteChannelMixer, enable); }
	bool			getChannelMixerOwnerShip() const { return getStateFlag(IsDeleteChannelMixer)!=0; }


	/** freeze the preceding position of the model. Do not use, special code for cluster.
	 *	This inform the scene that preceding position setuped by user is "frozen". ie at next render(), this
	 *	object won't be added to the "object moving list" (useful for cluster mgt).
	 *	The "frozen" state is disabled (at render() time) if:
	 *	- change in position (directly or indireclty, such as animation) is performed after the freeze().
	 *	- the "frozen" state of a father is not enabled (or disabled by a change in position of him :) ).
	 */
	void			freeze();
	void			setDontUnfreezeChildren(bool val);


	/** freeze the HRC so the WorldMatrix computed at next render() will be kept for long, and the model won't
	 *	either be tested in HRC (which is still expensive, even if the worldmatrix doesn't need to be recomputed).
	 *	The model won't either be validated. It is suposed to not change at all. Also, if it is not a son of a CCluster,
	 *	it may be accelerated during Cliping (with CQuadGridClipManager).
	 *
	 *	NB: the model won't be tested in HRC anymore.
	 *	calling freezeHRC() on a model in a hierarchy without calling it to the root of the hierarchy
	 *	will result in that the model won't be validated nor be HRC traversed.
	 *	To be simplier, you should freezeHRC() all the models of a hierarchy, from base root to leaves.
	 *
	 *	NB: if the hierarchy of this object must change, or if the object must moves, you must call unfreezeHRC() first,
	 *	and you should do this for all the parents of this model.
	 */
	void			freezeHRC();


	/**	see freezeHRC().
	 */
	void			unfreezeHRC();


	/** special feature for CQuadGridClipManager. return a non-zero value if true
	 */
	uint32			isQuadGridClipEnabled() const {return getStateFlag(QuadGridClipEnabled);}

	/**
	 * Get the worldMatrix that is computed at last Hrc pass
	 */
	const CMatrix&	getWorldMatrix() const {return _WorldMatrix;}

	// force the world matrix (could be modified next frame after hrc traversal
	void			setWorldMatrix(const CMatrix &mat) { _WorldMatrix = mat;}

	/** tells if the transform has been determined as visible in the hrc traversal
	 */
	bool	isHrcVisible() const
	{
		return _WorldVis;
	}

	/** tells if the transform has been clipped in the clip traversal.
	 */
	bool	isClipVisible() const
	{
		return _Visible;
	}


	// A RefPtr is kept on the clusterSystem, so getClusterSystem() will return NULL if the instance group has been deleted
	void				setClusterSystem (CInstanceGroup *pCS);
	CInstanceGroup*		getClusterSystem ();


	/// name Lighting Behavior.
	// @{
	/** reset lights which influence this models. NB: the model is removed from all lights's list (except
	 *	FrozenStaticLightSetup). Called by light rendering.
	 *
	 *	NB: the model is NOT removed from LightingManager (with eraseStaticLightedModel()).
	 */
	void				resetLighting();

	/** true if the model can be lighted (such as CMeshBaseInstance)
	 *	Default behavior is false.
	 *	Deriver must use setIsLightable(true) method if the instance can be lighted.
	 *	\return 0 if getUserLightable() is false, or if the model can't be lighted at all. else return a non-zero value
	 */
	uint32				isLightable() const {return getStateFlag(IsFinalLightable);}

	/** Set the UserLightable flag. if false, isLightable() will always return false.
	 *	Doing this, user can disable lighting on a model which may be interesting for speed.
	 *	Default behavior is UserLightable==true.
	 */
	void				setUserLightable(bool enable);

	/** Get the UserLightable flag.
	 */
	bool				getUserLightable() const {return getStateFlag(IsUserLightable)!=0;}

	/** Freeze and set the Static Light Setup. Called by CInstanceGroup::addToScene()
	 *	NB: it calls resetLighting() first.
	 *	NB: nlassert(numPointLights<=NL3D_MAX_LIGHT_CONTRIBUTION)
	 */
	void				freezeStaticLightSetup(CPointLight *pointLight[NL3D_MAX_LIGHT_CONTRIBUTION],
		uint numPointLights, uint8 sunContribution, CPointLight *frozenAmbientlight);

	/** unFreeze the Static Light Setup. Must be called if static pointLights are deleted.
	 *	NB: it calls resetLighting() first.
	 *	NB: do not need to call it if pointLights and this transform are deleted at same time.
	 */
	void				unfreezeStaticLightSetup();

	/** non-zero if the lighting Manager must take into account the bbox of the transform.
	 *	Default behavior is false. Deriver must call setIsBigLightable() at initialisation to change it.
	 */
	uint32				isBigLightable() const {return getStateFlag(IsBigLightable);}

	/** return true if the current light contribution of this model use a MergedPointLight
	 */
	bool				useMergedPointLight() const {return _LightContribution.UseMergedPointLight;}

	/** Return the current light contribution of this model
	  */
	const CLightContribution &getLightContribution() const { return _LightContribution; }


	/** get the HotSpot of the model for Light computation. For models with global attenuation, this is
	 *	the point taken for attenuation computes. NB: should return the current world position.
	 *	NB: return also the modelRadius (only for bigLightable)
	 *	Default to NULL.
	 */
	virtual	void		getLightHotSpotInWorld(CVector &modelPos, float &modelRadius) const {modelPos= CVector::Null; modelRadius=0;}

	// @}


	/** Set the LogicInfo for this transfrom, eg to retrieve statc light information, see ILogicInfo.
	 *	Ptr is kept in CTransfrom, so should call setLogicInfo(NULL) before to clean up.
	 */
	void				setLogicInfo(ILogicInfo *logicInfo) {_LogicInfo= logicInfo;}


	/** Get the untransformed AABBox of the transform. NULL (gtCenter()= 0, gtSize()==0) by default.
	 */
	virtual void		getAABBox(NLMISC::CAABBox &bbox) const;


	/// name Load Balancing Behavior.
	// @{

	/** Change the load Balancing group of a model. Every models are in a special LoadBalancingGroup.
	 *	NB: the group is  created if did not exist.
	 *	NB: if models are skinned, it is their Skeleton which drive the group
	 *
	 *	By default, models lies in the "Default" group, but Skeletons for skinning and ParticlesSystems which
	 *	are in "Skin" and "Fx" group respectively.
	 *	The "Default" group is special because it is not balanced (ie models are only degraded from
	 *	their distance to camera)
	 */
	void				setLoadBalancingGroup(const std::string &group);

	/** Get the load Balancing group of a model. see setLoadBalancingGroup().
	 */
	const std::string	&getLoadBalancingGroup() const;

	// @}


	/// \name Skinning Behavior.
	// @{
	/// return non-zero if I am a skeleton. if yes, static_cast<CSkeletonModel*> may be used
	uint32				isSkeleton() const {return getStateFlag(IsSkeleton);}
	/// non-zero if the model is skinned onto a skeleton.
	uint32				isSkinned() const {return getStateFlag(IsSkinned);}
	/** Called for edition purpose (slow call O(NVertex))
	 *	It return the BBox (local to the Bone at its bind pos) of all of the vertices
	 *	of this mesh that are bound to this bone
	 *	NB: the instance must be skinned to the skeleton instance, BUT the result is not modified
	 *	by current bone world matrix (eg: no current scale influence)
	 *	\param boneId: the id of the skeleton's bone (which the instance is skinned to)
	 *	\param bbox: the bbox (in bone basis) filled if return is true
	 *	\return false if no vertices are bound to
	 */
	virtual bool		getSkinBoneBBox(NLMISC::CAABBox &/* bbox */, uint /* boneId */) {return false;}
	// @}


	/// name Misc
	// @{

	/** set the Mean color of the transform. The mean color can be used for many purpose, such as drawing
	 *	objects if the textures are not loaded. It is used also for Lod Character.
	 *	Default color is (255,255,255)
	 */
	void				setMeanColor(CRGBA color);

	/// see setMeanColor()
	CRGBA				getMeanColor() const {return _MeanColor;}

	/// non-zero if the model is animDetailable (ie added to the animDetail list if visible)
	uint32				isAnimDetailable() const {return getStateFlag(IsAnimDetailable);}
	/// non-zero if the model is loadBalancable (ie added to the loadBalancing list if visible)
	uint32				isLoadBalancable() const {return getStateFlag(IsLoadBalancable);}
	/// non-zero if the model is renderable (ie something may appear on screen)
	uint32				isRenderable() const {return getStateFlag(IsRenderable);}

	/// non-zero if the CTransform can be casted to a CMeshBaseInstance
	uint32				isMeshBaseInstance() const {return getStateFlag(IsMeshBaseInstance);}
	/// non-zero if the CTransform can be casted to a CTransformShape
	uint32				isTransformShape() const {return getStateFlag(IsTransformShape);}
	/// non-zero if the CTransform can be casted to a CCluster
	uint32				isCluster() const {return getStateFlag(IsCluster);}

	/** true if the transform support fast intersection (fastIntersect() works)
	 *	For now, supports
	 *		- skeleton with ALL skins that are shadowSkined (w or wo MRM)
	 *		- mesh that are not skinned (Standard/MRM/MultiLod), and that have been flagged through
	 *			CShapeBank::buildSystemGeometryForshape()
	 */
	bool				supportFastIntersect() const {return _SupportFastIntersect;}
	/** test if the transform intersect the ray (p0, dir).
	 *	\return false if not supported/no triangles, else true if can do the test (even if don't intersect!)
	 *	if intersect, dist2D=0, and distZ= Depth Distance
	 *	if don't intersect, dist2D="nearest distance to the ray", and distZ=0
	 *	\param computeDist2D if false and don't intersect, then return dist2D=FLT_MAX, and distZ=0
	 */
	virtual bool		fastIntersect(const NLMISC::CVector &/* p0 */, const NLMISC::CVector &/* dir */, float &/* dist2D */, float &/* distZ */, bool /* computeDist2D */) {return false;}

	/// internal only: used by CMeshBase
	void				enableFastIntersectSupport(bool enable) {_SupportFastIntersect= enable;}

	// @}


	/** \name ShadowMapping
	 *	NB: It is the deriver work to call CRenderTrav::getShadowMapManager().addShadowReceiver() or
	 *	CRenderTrav::getShadowMapManager().addShadowCaster(), typically in traverseHRC() or in traverseRender()
	 *	NB: when _AncestorSkeletonModel!=NULL, the shadowMapCaster should not be Added.
	 *	NB: Deriver must also implement createShadowMap() and deleteShadowMap() protected callBacks,
	 *	if it supports CastShadowMaping
	 */
	// @{
	/** By default, map shadow casting is disabled. This enabled shadow for this model.
	 *	Fails if the model don't support dynamic Map Shadow Casting (eg landscape)
	 *	Dervier note: createShadowMap() and deleteShadowMap() is called here.
	 */
	void				enableCastShadowMap(bool state);
	/// true if the instance cast shadow. By default false
	bool				canCastShadowMap() const {return getStateFlag(IsFinalShadowMapCaster)!=0;}

	/** By default, map shadow receiving is disabled. This enabled shadow for this model.
	 *	Fails if the model don't support dynamic Map Shadow Receiving (eg Particle system)
	 */
	void				enableReceiveShadowMap(bool state) {if(modelCanReceiveShadowMap()) setStateFlag(IsFinalShadowMapReceiver, state);}
	/// true if the instance receive shadow. By default false
	bool				canReceiveShadowMap() const {return getStateFlag(IsFinalShadowMapReceiver)!=0;}

	/// true if the model provide a method to support shadowMap generation
	uint32				modelCanCastShadowMap() const {return getStateFlag(IsShadowMapCaster);}
	/// true if the model provide a method to support shadowMap receiving
	uint32				modelCanReceiveShadowMap() const {return getStateFlag(IsShadowMapReceiver);}

	/** For Casters. Display the Shadow to the "Auxiliary Driver".
	 *	This method should only write to AlphaBuffer (since RGB may be the current rendered scene!),
	 *	with Alpha==1 when pixel is shadowed.
	 *	The ShadowMapManager has already cleared the AlphaBuffer to black, and has already enabled alpha write only.
	 *	The ShadowMapManager has already setuped Viewport/Scissor as its convenience.
	 *	The extra blurring is a work of the ShadowMapManager (which blurs multiple shadows in a same pass)
	 *	NB: you can overwrite the current driver frustum/ViewMatrix/modelMatrix without backuping it (ShadowMapManager work)
	 */
	virtual	void		generateShadowMap(const CVector &/* lightDir */) { }
	/** get The shadow Map result for receveing. If NULL, nothing is displayed.
	 */
	virtual	CShadowMap	*getShadowMap() {return NULL;}

	/** For receivers. get the World Instance bbox that includes the receiver.
	 */
	virtual void		getReceiverBBox(CAABBox &bbox);

	/** For receivers. Modulate the Object with a ShadowMap. The model shoud render in the scene driver a version
	 *	of its geometry simplified, and modulate the background with shadowColor.
	 *	\param casterPos the world position of the caster model.
	 *	\param shadowMat a correclty setuped material with good ShadowColor, ready to be rendered.
	 */
	virtual void		receiveShadowMap(CShadowMap * /* shadowMap */, const CVector &/* casterPos */, const CMaterial &/* shadowMat */) { }

	/** For receivers. Retrieve the WorldMatrix of the model used for IDriver::render(). By default it returns getWorldMatrix().
	 *	The exception is the Landscape and his "ZBuffer Problem" management.
	 */
	virtual const CMatrix	&getReceiverRenderWorldMatrix() const {return getWorldMatrix();}

	/// For ShadowMapManager. true if the model is rendering its ShadowMap this frame.
	void				setGeneratingShadowMap(bool state) {if(canCastShadowMap()) setStateFlag(IsGeneratingShadowMap, state);}
	bool				isGeneratingShadowMap() const {return getStateFlag(IsGeneratingShadowMap)!=0;}

	/** Special For Skeleton Caster. When Skeletons cast shadows, they first compute the WorldBBox.
	 *	The model should compute its bbox in World (best fit).
	 *	\return false if the model don't support it (default), or if hidden in HRC!!
	 */
	virtual bool		computeWorldBBoxForShadow(NLMISC::CAABBox &/* worldBB */) {return false;}
	/** Special For Skeleton Caster. Render into the AuxDriver the mesh, within the current
	 *	setuped Frustum/ViewMatrix.
	 *	no-op by default, or if hidden in HRC!!
	 *	\param rootSkeleton the skeleton which is currently rendering its shadowMap
	 */
	virtual void		renderIntoSkeletonShadowMap(CSkeletonModel * /* rootSkeleton */, CMaterial	&/* castMat */) { }

	/** To limit some problems when the light direction is too on the XY axis.
	 *	This method set an "angle" threshold for the shadow direction
	 *	Actually, you give the minimum negative Z (not ang angle) the normalized shadow direction must have
	 *	\param zthre possible values are in [-1,1].
	 *		-1 force the direction to be (0,0,-1) in all case
	 *		0 means the z may be 0 (the direction is totaly XY), but at least the direction must go downward
	 *		1 means there is no restriction, the shadow direction can either be upward
	 *	default is -0.5 (this implies a minimum angle of 30 degrees with the XY plane)
	 */
	void				setShadowMapDirectionZThreshold(float zthre);
	float				getShadowMapDirectionZThreshold() const {return _ShadowMapDirectionZThreshold;}

	/** To limit some problems with interior, a limit of shadow depth by caster can be given.
	 *	This is the length in the lightDir direction where the shadow can touch receivers.
	 *	Can be used also to have some big objects that cast shadows further.
	 *	Default to 8.0.
	 */
	void				setShadowMapMaxDepth(float depth);
	float				getShadowMapMaxDepth() const {return _ShadowMapMaxDepth;}

	// @}

	/** Force the transform to always be attached to the root
	  * As a consequence, it can't be inserted into a cluster system (even the root cluster)
	  * and is thus always visible when in the frustum (not clusterized)
	  * NB : any call to setClusterSystem will cause an assertion when the flag is set
	  * NB : any call to hrcUnlink will cause an assertion when the flag is set (must remain linked to the root)
      */
	void				setForceClipRoot(bool forceClipRoot);
	bool				getForceClipRoot() const { return getStateFlag(ForceClipRoot) != 0; }


	// test if the model is a flare
	virtual	bool isFlare() const { return false; }


	/// Don't use, used by CSkeletonSpawnScript to indicate that the WorldMatrix of this object is special
	void		setSSSWO(bool state) {setStateFlag(SSSWO, state);}
	bool		getSSSWO() const {return getStateFlag(SSSWO)!=0;}


// ********
private:
	CHrcTrav::TVisibility	Visibility;

	static	CTrackDefaultVector		DefaultPos;
	static	CTrackDefaultVector		DefaultPivot;
	static	CTrackDefaultVector		DefaultRotEuler;
	static	CTrackDefaultQuat		DefaultRotQuat;
	static	CTrackDefaultVector		DefaultScale;

protected:
	/** Constructor
	 * The deriver \b should do a \c TouchObs.resize(Last), to ensure he resize the BitSet correctly.
	 * The dervier \b should keep/declare ctor and dtor protected, to avoid user error (new and delete).
	 */
	CTransform();
	/// Destructor
	virtual ~CTransform();

	/// special feature for CQuadGridClipManager. called at unfreezeHRC(). Used by CTransformShape.
	virtual	void	unlinkFromQuadCluster() {}

	/// \name Skinning Behavior.
	// @{

	/// Deriver must change this method if the model can be skinned. called rarely
	virtual	bool			isSkinnable() const {return false;}
	/** Deriver must change this method if isSkinnable(). called by CSkeletonModel::bindSkin()
	 *	NB: _FatherSkeletonModel is valid when setApplySkin() is called
	 *	The default behavior must be called: it sets the flag so isSkinned() return the good thing
	 */
	virtual	void			setApplySkin(bool state);
	/** Deriver must change this method if isSkinnable(). It return the list of bone (correct skeleton index)
	 *	used by the skins (NB: without the parents of the bone).
	 *	NB: if an index is -1, it means that the skin bone has not been found in the skeleton (skip it)
	 *	default is to return NULL.
	 */
	virtual const std::vector<sint32>			*getSkinBoneUsage() const {return NULL;}
	/** Deriver must change this method if isSkinnable(). It return a list of sphere relative to each bone
	 *	of the father skeleton. Use with getSkinBoneUsage() to know to which bone this sphere apply
	 *	NB: if a sphere radius is -1, it means that the bone is not used (for any reason...)
	 *	default is to return NULL.
	 */
	virtual const std::vector<NLMISC::CBSphere>	*getSkinBoneSphere() const {return NULL;}
	/** Deriver must change this method if isSkinnable(). It renders the skin with current ctx of the skeletonModel
	 *	SkeletonModel has already setuped the Light and the modelMatrix in the driver.
	 *	If the skin is a MRM, it is the skeleton which drives the MRM level with alphaMRM: [0,1]
	 *	default is nop
	 */
	virtual void			renderSkin(float /* alphaMRM */) { }


	/** Deriver may support SkinGrouping if isSkinnable().
	 *	It renders the skin with current ctx of the skeletonModel, but torn in 2 pass: fillVB,a nd renderPrimitives
	 *	Deriver may check NL3D_MESH_SKIN_MANAGER_VERTEXFORMAT and NL3D_MESH_SKIN_MANAGER_MAXVERTICES
	 */
	virtual	bool			supportSkinGrouping() const {return false;}
	/** if supportSkinGrouping(), called to transform the VBuffer, and store it into dest.
	 *	\return number of vertices added to the VBuffer, or -1 if > reaminingVertices
	 */
	virtual	sint			renderSkinGroupGeom(float /* alphaMRM */, uint /* remainingVertices */, uint8 * /* dest */) {return 0;}
	/** if supportSkinGrouping(), called to render the primitives of the already skinned vertices (VB activated in the driver)
	 *	Optionally, fill specRdrPasses with specular rdrPass to sort (used for specular grouping).
	 *	\param baseVertex value to add to each PBlock index.
	 */
	virtual	void			renderSkinGroupPrimitives(uint /* baseVertex */, std::vector<CSkinSpecularRdrPass> &/* specularRdrPasses */, uint /* skinIndex */) { }
	/// Render a specific specular renderPass returned by renderSkinGroupPrimitives
	virtual	void			renderSkinGroupSpecularRdrPass(uint /* rdrPass */) { }

	/// Special Skinning For ShadowMapping
	virtual	bool			supportShadowSkinGrouping() const {return false;}
	virtual	sint			renderShadowSkinGeom(uint /* remainingVertices */, uint8 * /* vbDest */) {return 0;}
	virtual	void			renderShadowSkinPrimitives(CMaterial &/* castMat */, IDriver * /* drv */, uint /* baseVertex */) { }

	/** Special use of skinning to compute intersection of a ray with it.
	 *	\return false if not supported/no triangles, else true if can do the test (even if don't intersect!)
 	 *	if intersect, dist2D=0, and distZ= Depth Distance
 	 *	if don't intersect, dist2D="nearest distance to the ray", and distZ=0
	 *	\param computeDist2D if false and don't intersect, then return dist2D=FLT_MAX, and distZ=0
	 */
	virtual	bool			supportIntersectSkin() const {return false;}
	virtual	bool			intersectSkin(const CMatrix &/* toRaySpace */, float &/* dist2D */, float &/* distZ */, bool /* computeDist2D */) {return false;}

	// The SkeletonModel, root of us (skinning or sticked object). NULL , if normal mode.
	CSkeletonModel	*_FatherSkeletonModel;
	// If sticked object, id of the bone in the _FatherSkeletonModel.
	uint			_FatherBoneId;

	// @}

	/// name Lighting Behavior.
	// @{

	/// The contribution of all lights. This enlarge the struct only of approx 15%
	CLightContribution		_LightContribution;

	/// non-zero if the object needs to updatelighting.
	uint32					isNeedUpdateLighting() const {return getStateFlag(IsNeedUpdateLighting);}
	/// non-zero if the object has a FrozenStaticLightSetup not correclty updated.
	uint32					isNeedUpdateFrozenStaticLightSetup() const {return getStateFlag(IsNeedUpdateFrozenStaticLightSetup);}

	/// each transform may be in a quadGird of lighted models (see CLightingManager)
	CLightingManager::CQGItLightedModel		_LightedModelIt;

	// @}


	/** get the channelMixer owned by the transform. return result of a refPtr => may be NULL.
	 */
	CChannelMixer			*getChannelMixer() const {return _ChannelMixer;}


	/// \name Model Feature initialisation.
	// @{

	/// Deriver must use this method with true to indicate the model support lighting.
	void				setIsLightable(bool val);
	/** Deriver must use this method with true to indicate the model can be rendered.
	 *	"can be rendered" means if object has to be inserted in RenderTrav list.
	 *	eg: a mesh must be inserted in a render list, but not a light, or a NULL transform.
	 *	The default is false.
	 */
	void				setIsRenderable(bool val);
	/// Deriver must use this method with true to indicate the model is a big lightable.
	void				setIsBigLightable(bool val);
	/// For CSkeletonModel only.
	void				setIsSkeleton(bool val);
	/** Deriver must use this method with true if the model must be AnimDetail-ed whatever
	 *	registerToChannelMixer() has been called or not
	 */
	void				setIsForceAnimDetail(bool val);
	/// Deriver must use this method with true to indicate the model support loadBalancing.
	void				setIsLoadbalancable(bool val);

	/// For CMeshBaseInstance only
	void				setIsMeshBaseInstance(bool val) {setStateFlag(IsMeshBaseInstance, val);}
	/// For CTransformShape only.
	void				setIsTransformShape(bool val) {setStateFlag(IsTransformShape, val);}
	/// For CCluster only.
	void				setIsCluster(bool val) {setStateFlag(IsCluster, val);}

	/// ShadowMap
	void				setIsShadowMapCaster(bool val) {setStateFlag(IsShadowMapCaster, val);}
	void				setIsShadowMapReceiver(bool val) {setStateFlag(IsShadowMapReceiver, val);}

	// @}

	/// Test if obj must be displayed when sticked to an object displayed as a LOD (example: sword in hand of a character displayed as a LOD state)
	bool				getShowWhenLODSticked() const { return _ForceCLodSticked; }

	// force to compute that transform matrix (useful if matrix needed but clipped because sticked to a clipped skeleton for example)
	void				forceCompute();

private:
	static CTransform	*creator() {return new CTransform;}
	friend class	CSkeletonModel;
	friend class	CScene;
	friend class	CClipTrav;
	friend class	CAnimDetailTrav;
	friend class	CRenderTrav;

	// The Scene which owns us
	CScene			*_OwnerScene;

	/// \name Hrc / Clip hierarchy.
	// @{
	// Hrc hierarchy. One parent and possible multiple sons
	CFastPtrListNode			_HrcNode;
	CFastPtrList<CTransform>	_HrcSons;
	CTransform					*_HrcParent;
	NLMISC::CRefPtr<CTransform>	_HrcParentUnfreeze;

	/* Clip Graph. DAG (Direct Acyclic Graph)
	 *	NB: implementation optmized for Low number of parent. clipAddChild() and clipDelChild() is in O(numParents).
	 */
	struct	CClipNode
	{
		CFastPtrListNode			ClipNode;
		CTransform					*Parent;
	};
	CFastPtrList<CTransform>	_ClipSons;
	std::vector<CClipNode*>		_ClipParents;
	bool			clipHasParent(CTransform *parent);
	void			clipDelFromParent(CTransform *parent);

	// @}

	/// \name Model updating/traversing features
	// @{
	// linked list of models to update.
	CTransform		*_PrecModelToUpdate;
	CTransform		*_NextModelToUpdate;

	// for CScene::createModel() and for CTransform::freezeHRC() only.
	void			linkToUpdateList();
	void			unlinkFromUpdateList();

	// @}


	// For anim detail.
	NLMISC::CRefPtr<CChannelMixer>		_ChannelMixer;

	// Last date of ITransformable matrix.
	uint64			_LastTransformableMatrixDate;

	NLMISC::CRefPtr<CInstanceGroup>		_ClusterSystem;


	enum	TFreezeHRCState	{ FreezeHRCStateDisabled=0, FreezeHRCStateRequest, FreezeHRCStateReady, FreezeHRCStateEnabled};
	TFreezeHRCState			_FreezeHRCState;

	uint8				_OrderingLayer;
	uint8				_TransparencyPriority;

	// For stickObjectEx(). with forceCLod==true
	bool				_ForceCLodSticked : 1;

	/// true if need to compute transform
	bool				_TransformDirty   : 1;

	// see supportFastIntersect. Filled by CSkeletonModel
	bool				_SupportFastIntersect : 1;

	/// See ILogicInfo. Used for lighting.	default is NULL.
	ILogicInfo			*_LogicInfo;


	/// see setMeanColor()
	CRGBA				_MeanColor;

	/// For Shadow Caster registration to list
	std::list<CTransform*>::iterator		_ItShadowCasterInScene;

	/// For Shadow Casters
	float						_ShadowMapDirectionZThreshold;
	float						_ShadowMapMaxDepth;

	/// \name State Flag mgt (boolean compression)
	// @{

	/// State Flags.
	enum	TState	{
		// Post-clipping Traversal flags. If set, then the object is inserted into traversal list.
		IsAnimDetailable=		0x0001,
		IsLoadBalancable=		0x0002,
		IsLightable=			0x0004,
		IsRenderable=			0x0008,
		// Transparency Flags.
		IsTransparent=			0x0010,
		IsOpaque=				0x0020,
		// For fast clip.
		QuadGridClipEnabled=	0x0040,
		// Lighting.
		IsUserLightable=		0x0080,
		IsFinalLightable=		0x0100,		// IsLightable && IsUserLightable
		IsBigLightable=			0x0200,
		IsNeedUpdateLighting=	0x0400,
		IsNeedUpdateFrozenStaticLightSetup=
								0x0800,
		// Skinning
		IsSkeleton=				0x1000,		// set if the model is a skeleton (faster than dynamic_cast)
		IsSkinned=				0x2000,		// true if the model is isSkinnable() and if currently skinned
		// Misc
		IsDeleteChannelMixer=	0x4000,
		IsForceAnimDetail=		0x8000,
		IsMeshBaseInstance=		0x10000,
		IsTransformShape=		0x20000,	// set if the model is a transform_shape (faster than dynamic_cast)
		IsCluster=				0x40000,	// set if the model is a cluster (faster than dynamic_cast)
		UserClipping=			0x80000,	// set if the user provide a clip method, don't call clip() in ClipTrav

		// ShadowMap
		IsShadowMapCaster=		0x100000,	// set if the model can cast ShadowMap
		IsFinalShadowMapCaster=	0x200000,	// set if the model can cast ShadowMap AND the user want it
		IsShadowMapReceiver=	0x400000,	// set if the model can receive ShadowMap
		IsFinalShadowMapReceiver= 0x800000,	// set if the model can receive ShadowMap AND the user want it
		IsGeneratingShadowMap=	0x1000000,	// temp set if the model is asked to render its shadowMap this frame.

		ForceClipRoot			= 0x2000000,// Force the object to always be attached to the root
		                                    // As a consequence, it can't be inserted into a cluster system (even the root cluster)
											// and is thus always visible when in the frustum
		ClusterSystemAuto		= 0x4000000,

		SSSWO					= 0x8000000,	// Special for SkeletonSpawnScript. if set, the WorldMatrix is special

		BypassLODOpacity		= 0x10000000	// for mesh multi lod : do not use the LOD opacity / transparency, but the parent one (overwritten at traversal, else ...)

		// NB: may continue on >=0x20000000
	};

	/// Flags for the General State of the Transform. They are both static or dynamic flags.
	uint32				_StateFlags;

	/// This is used to set Static or dynamic flags. val must take 0 or 1.
	void				setStateFlag(uint32 mask, bool val)
	{
		// reset the state.
		_StateFlags&= ~mask;
		// set the state
		_StateFlags|= ( 0- ((uint32)val) ) & mask;
	}

	/// return a non zero-value if state is set.
	uint32				getStateFlag(uint32 mask) const
	{
		return _StateFlags&mask;
	}

	// @}

protected:

	/** State for renderFiltering. Default is 0xFFFFFFFF (always displayed)
	 *	Deriver work to change this value
	 */
	uint32						_RenderFilterType;


protected:

	/// \name Hrc Traversal
	// @{

	/// Hrc IN variables.
	CMatrix		_LocalMatrix;
	CHrcTrav::TVisibility	_LocalVis;	// The visibility state of the node.
	sint64		_LocalDate;				// The update date of the LocalMatrix.
	/// Hrc OUT variables.
	CMatrix		_WorldMatrix;
	sint64		_WorldDate;			// The update date of the WorldMatrix.
	bool		_WorldVis;			// Is the node visible? (enabled?)
	// Transform Specicic Hrc
	bool		_Frozen;
	bool		_DontUnfreezeChildren; // Useful when cluster system move to not test instance again
	bool		_ClipLinkedInSonsOfAncestorSkeletonModelGroup;
	// !NULL if any skeleton is an ancestor in hierarchy. Updated at each Hrc traversal!!
	CSkeletonModel	*_AncestorSkeletonModel;

	/// Update the world state according to the parent world state and the local states.
	void		updateWorld();
	// according to _AncestorSkeletonModel, link clipTrav.
	void		updateClipTravForAncestorSkeleton();

	// @}


	/// \name Clip Traversal
	// @{

	/// date of last traverseClip()
	sint64		_ClipDate;
	// The index of the Observer in the _VisibleList; -1 (default) means not in
	sint		_IndexInVisibleList;
	/// set to true is the object is visible (not clipped).
	bool		_Visible;

	// @}

	/// \name Render Traversal
	// @{
	// Used by CRenderTrav. see CRenderTrav::removeRenderModel() implementation
	uint8		_IndexLSBInRenderList;
	// @}

	/// \name AnimDetail Traversal
	// @{

	/** For Skeleton Object Stick.
	 *	update the wolrd matrix. no-op if skinned. no-op if no AcnestorSkeletonModel.
	 *	use standard father WorldMatrix if !_FatherSkeletonModel else get the correct boneId
	 *	WorldMatrix from _FatherSkeletonModel
	 */
	void			updateWorldMatrixFromFather();

	/** traverse without updatin WorldMatrixFromFather:
	 *	- animdetail if the model channelmixer is not NULL, and if model not clipped
	 */
	void			traverseAnimDetailWithoutUpdateWorldMatrix();

	// @}

	/// \name LoadBalancing Traversal
	// @{
	// Which group owns this model
	CLoadBalancingGroup		*_LoadBalancingGroup;
	// @}

	/// \name ShadowMap
	// @{
	/** To implement for ShadowCaster support. typically allocate a CShadowMap and store
	 *	NB: the texture doesn't have to be inited at this time. Update it each frame in generateShadowMap()
	 *	MUST call registerShadowCasterToList()
     */
	virtual void			createShadowMap() {}
	/// To implement for ShadowCaster support. typically free the shadowMap + MUST call unregisterShadowCasterToList()
	virtual void			deleteShadowMap() {}
	// @}

};


} // namespace NL3D


#endif // NL_TRANSFORM_H

/* End of transform.h */
