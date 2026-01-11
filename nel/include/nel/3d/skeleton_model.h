// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_SKELETON_MODEL_H
#define NL_SKELETON_MODEL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/object_vector.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/bone.h"
#include "nel/3d/mrm_level_detail.h"
#include "nel/3d/lod_character_instance.h"
#include "nel/3d/skeleton_spawn_script.h"

namespace NLMISC
{
	class	CAABBox;
}

namespace NL3D
{

class CSkeletonShape;
class CLodCharacterManager;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		SkeletonModelId=NLMISC::CClassId(0x7d4703b4, 0x43ad6ab1);


// ***************************************************************************
/** Struct used when SkinGroup is used.
 *	Used to sort specular render pass of all Skins (sorted by specular map).
 */
class	CSkinSpecularRdrPass
{
public:
	// The skin index in the grouped rendering.
	uint16		SkinIndex;
	// The index of the skin rdrPass
	uint16		RdrPassIndex;
	// The texture id of the specular texture. This is the sort Key.
	uintptr_t	SpecId;

	bool	operator<(const CSkinSpecularRdrPass &o) const
	{
		return SpecId<o.SpecId;
	}
};


// ***************************************************************************
/**
 * A Skeleton model, instance of CSkeletonShape.
 *
 * Skeletons sons are added with bindSkin(), stickObject(). They are removed auto at dtor.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSkeletonModel : public CTransformShape
{
public:
	/// max number of bones supported in skeleton
	enum	{ MaxNumBones = 256 };

	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();

public:

	/** The list of CBone, created from the CSkeletonShape.
	 * They are odered in depth-first order.
	 */
	std::vector<CBone>			Bones;

public:
	/// \name IAnimatable Interface (registering only IAnimatable sons (bones)).
	// @{
	enum	TAnimValues
	{
		OwnerBit= CTransformShape::AnimValueLast,
		SpawnScriptValue,
		AnimValueLast,
	};

	/// From IAnimatable
	virtual IAnimatedValue* getValue (uint valueId);
	/// From IAnimatable
	virtual const char *getValueName (uint valueId) const;
	/// Default Track Values for SpawnScriptValue is empty string
	virtual ITrack* getDefaultTrack (uint valueId);
	/// Register bones into chanMixer.
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);

	/// Return the name of the spwan script track
	static const char *getSpawnScriptValueName() {return "spawn_script";}

	// @}


	/// \name Skin operation.
	// @{
	/** bind a skin to the skeleton. NB: ~CTransform() calls detachSkeletonSon().
	 * NB: nlassert() if there is too many skins/sticked objects on this skeleton (more than 255).
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 * NB: when a skin is binded, the command hide(), show(), ... have no effect on it, until it is detachSkeletonSon()-ed
	 * NB: For Skins, all Hrc/Clip/UpdateList link is done here
	 * \return false if mi is not skinnable, true otherwise
	 */
	bool		bindSkin(CTransform *mi);
	/** parent a CTransform to a bone of the skeleton. NB: ~CTransform() calls detachSkeletonSon().
	 * NB: nlassert() if there is too many skins/sticked objects on this skeleton (more than 255).
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 * NB: mi is made son of skeleton model in Traversals Hrc, and change are made at render() for ClipTrav.
	 */
	void		stickObject(CTransform *mi, uint boneId);

	/** same method as stickObject(), but if you set forceCLod as true, then this object will be visible
	 *	even if the skeleton father is in CLod state (ie displayed with a CLodCharacterShape)
	 *	NB: if "mi" is a skeleton model, forceCLod is considerer true, whatever the value passed in.
	 */
	void		stickObjectEx(CTransform *mi, uint boneId, bool forceCLod);

	/** unparent a CTransform from a bone of the skeleton, or unbind a skin. No-op if not a son of this skeleton
	 * NB: mi is made son of Root in Traversals Hrc, and change are made at render() for ClipTrav.
	 * NB: For Skins, all Hrc/Clip/UpdateList link is done here
	 */
	void		detachSkeletonSon(CTransform *mi);

	/** Force the skeletonModel to recompute at next render which skins to render, at which pass.
	 *	If you call setOpacity()/setTransparency() on one of the skins binded to the skeleton, you should call this
	 *	method, else strange result may occurs.
	 *	NB: this is automatically called by bindSkin()/detachSkeletonSon()
	 */
	void		dirtSkinRenderLists() {_SkinToRenderDirty= true;}

	/** return the set of skins added with bindSkin()
	 */
	const std::set<CTransform*>		&getSkins() const {return _Skins;};

	/** return the set of stickedObjects added with stickObject()
	 */
	const std::set<CTransform*>		&getStickedObjects() const {return _StickedObjects;};

	// @}


	/// \name Skin/BoneUsage Accessor. Called by CMeshInstance only.
	// @{

	typedef enum {UsageNormal, UsageForced, UsageCLodForced} TBoneUsageType;

	/** increment the refCount of the ith bone.
	 *	set boneUsageType to UsageNormal if enable Skeleton Bone degradation (skins)
	 *	Forced usage are for Sticked objects
	 */
	void		incBoneUsage(uint i, TBoneUsageType boneUsageType);
	/// decrement the refCount of the ith bone. set forced to the same param passed when incBoneUsage()
	void		decBoneUsage(uint i, TBoneUsageType boneUsageType);

	/** This method update boneUsage (must be of size of Bones).
	 *	It's flag boneUsage[boneId] to true, and all parents of boneId.
	 */
	void		flagBoneAndParents(uint32 boneId, std::vector<bool>	&boneUsage) const;

	// @}


	/// \name Misc.
	// @{
	/// return, from skeleton shape, the BoneIdByName. -1 if not here.
	sint32		getBoneIdByName(const std::string &name) const;

	/// Tell if a bone has been computed in the last frame or not. false if boneId is invalid
	bool		isBoneComputed(uint boneId) const;

	/** Force to compute a bone right now (useful if the skeleton is not visible and the bone position is needed)
      * no-op is bone not present or already computed
	  * \return true if good indes & bone recomputed
	  */
	bool		forceComputeBone(uint boneId);

	/// return the number of bones currently animated/computed (because of bindSkin()/stickObject() / Lod system).
	uint		getNumBoneComputed() const {return (uint)_BoneToCompute.size();}

	/** change the Lod Bone interpolation distance (in meters). If 0, interpolation is disabled.
	 *	The smaller this value is, the more Lod skeleton system will "pop". Default is 0.5 meters.
	 */
	void		setInterpolationDistance(float dist);

	/// see setInterpolationDistance()
	float		getInterpolationDistance() const;

	/** if Bones[boneId] is "Computed" (usage/lod), return Bones[boneId].getBoneSkinMatrix()
	 *	else return parent ones (recurs, but precomputed)
	 */
	const NLMISC::CMatrix	&getActiveBoneSkinMatrix(uint boneId) const;


	/** Tool function, especially for animation bake. It updates all bones (independent of bone usage,
	 *	and lod interpolation), and take a user skeleton worldMatrix as input.
	 *	NB: no detail animation is performed here, just the compute of bone hierarchy.
	 *	NB: also, no special AnimCtrl (IK etc....) is performed here
	 */
	void		computeAllBones(const CMatrix &modelWorldMatrix);

	/** Retrieve the current approx BBox around the skeleton, computed in the last CSene::render().
	 *	for all computed bones, extend the bbox with their pos
	 *	\param bbox return the bbox of the skinned skeleton, local to the skeleton. If the skeleton was clipped, the bbox
	 *	is not modified.
	 *	\param computeInWorld true if want to get the bbox in world.
	 *	\return true if the bbox is computed, false otherwise.
	 */
	bool		computeRenderedBBox(NLMISC::CAABBox &bbox, bool computeInWorld= false);

	/** same as computeRenderedBBox(), always in world, but use the bone max sphere to enlarge the bbox
	 *	NB: sticked objects don't influence the result
	 */
	bool		computeRenderedBBoxWithBoneSphere(NLMISC::CAABBox &bbox, bool computeInWorld= true);

	/** same as computeRenderedBBox() but force animation and compute of all bones => don't need render(), but slower.
	 *	for all used bones, extend the bbox with their pos
	 *	NB: AnimCtrl are not evaluated by this method (since computed with 0 pos).
	 *	\param bbox return the bbox of the skinned skeleton, local to the skeleton. If the skeleton is not skinned/sticked
	 *	at all, bbox is not modified.
	 *  \param forceCompute force evalution even if not skinned
	 *	\param computeInWorld true if want to get the bbox in world.
	 *	\return true if the bbox is computed, false otherwise.
	 */
	bool		computeCurrentBBox(NLMISC::CAABBox &bbox, bool forceCompute = false, bool computeInWorld= false);

	/// return the current Script of Spwan.
	const std::string		&getSpawnScript() const {return _SpawnScript.Value;}
	/// set the current Spawn Script (for debug, should only be used by animation)
	void					setSpawnScript(const std::string &s) {_SpawnScript.Value= s;}

	/// SkeletonSpawnScript special: World Spawned objects are still relative to this position (default: Null)
	void			setSSSWOPos(const CVector &pos) {_SSSWOPos= pos;}
	const CVector	&getSSSWOPos() const {return _SSSWOPos;}
	/// SkeletonSpawnScript special: World Spawned objects are still relative to this direction (default: J)
	void			setSSSWODir(const CVector &dir) {_SSSWODir= dir;}
	const CVector	&getSSSWODir() const {return _SSSWODir;}
	/// don't use
	CSkeletonSpawnScript	&getSSSScript() {return _SpawnScriptEvaluator;}

	/// see CTransform::fastIntersect()
	virtual bool	fastIntersect(const NLMISC::CVector &p0, const NLMISC::CVector &dir, float &dist2D, float &distZ, bool computeDist2D);

	/** Internal. Tool method used by Skins, to remap their bones id to skeleton ones
	 *	\param bonesName input list of bones name
	 *	\param boneId output list of bone remapped (size==bonesName.size()). -1 if bone not found
	 *	\param remap == boneId, but with 0 where boneId[i]==-1
	*/
	void		remapSkinBones(const std::vector<std::string> &bonesName, std::vector<sint32> &bonesId, std::vector<uint> &remap);

	// @}


	/// \name CLod / Character Lod
	// @{

	/// enable/disable LOD
	void enableLOD(bool isEnable) { _IsEnableLOD=isEnable; }

	/// Change the Character Lod shape Id. set -1 if want to disable the feature (default)
	void			setLodCharacterShape(sint shapeId);
	/// see setLodCharacterShape
	sint			getLodCharacterShape() const {return _CLodInstance.ShapeId;}

	/// Change/get the Character Lod anim setup.
	void			setLodCharacterAnimId(uint animId);
	uint			getLodCharacterAnimId() const {return _CLodInstance.AnimId;}
	void			setLodCharacterAnimTime(TGlobalAnimationTime time);
	TGlobalAnimationTime	getLodCharacterAnimTime() const {return _CLodInstance.AnimTime;}

	/// tells if the animation must loop or clamp.
	void			setLodCharacterWrapMode(bool wrapMode);
	bool			getLodCharacterWrapMode() const {return _CLodInstance.WrapMode;}


	/** True if the skeleton model and his skins are to be displayed with a CLodCharacterShape, instead of the std way
	 *	This state is modified early during the HRC Traversal. Because Clip traversal need this result.
	 */
	bool			isDisplayedAsLodCharacter() const {return _DisplayedAsLodCharacter;}

	/** This is the distance at which the skeleton use a CLodCharacterShape to display himself
	 *	if 0, never display the skeleton as a CLodCharacterShape
	 */
	void			setLodCharacterDistance(float dist);

	/// see setLodCharacterDistance. 0 if disabled
	float			getLodCharacterDistance() const {return _LodCharacterDistance;}

	/** Called in ClipTrav pass. Used to update the flag _DisplayedAsLodCharacter.
	 *	return 0 if CLod not enabled. Else return a priority>0 computed from curDistance/LodCharacterDistance
	 *	return 0 too if the skeleton is not visible or if his ancestorSkeletonModel is not visible.
	 *	If priority is >1 then the skeleton must be displayed in CLod form
	 */
	float			computeDisplayLodCharacterPriority() const;

	/** Called in ClipTrav pass. setup the flag.
	 */
	void			setDisplayLodCharacterFlag(bool displayCLod);

	/** Call it when you want the system to recompute the Lod texture
	 *	NB: Lod texturing is possible only in conjunction with AsyncTextureManager. Hence, instances skinned
	 *	to the skeleton should be in AsyncTextureMode.
	 *	For best result, you should wait that each of these instances are isAsyncTextureReady() (texture loaded)
	 */
	void			computeLodTexture();

	/// Set the emissive of the skeleton model, when it is rendered in CLod form. Default to Black
	void			setLodEmit(NLMISC::CRGBA emit) {_LodEmit= emit;}
	NLMISC::CRGBA	getLodEmit() const {return _LodEmit;}

	// @}

	/// \name Load balancing methods
	// @{

	/// Special version for skins
	virtual float	getNumTriangles (float distance);

	/** Special version for skins. NB: skins never follow their original MRM distance setup, but follow
	 *	this skeleton MRM setup. Default is to follow the MAX of all skins binded (ie the finer).
	 *	NB: Unlike CMeshBaseInstance::changeMRMDistanceSetup(), this setup applies to the SkeletonModel, not the shape.
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	void			changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);

	/// Reset the Default MRM setup: follow MAX skin setup (ie the finer)
	void			resetDefaultMRMDistanceSetup();

	// @}


	/// \name CTransform traverse specialisation
	// @{
	/** this do :
	 *  - call CTransformShape::traverseAnimDetail()
	 *  - update animated bones.
	 */
	virtual void	traverseAnimDetail();
	/**	If displayed as a CLod, render it, else render the skins binded to this skeleton
	 */
	virtual	void	traverseRender();
	// @}

	// Lighting: get the Root world position!
	virtual	void		getLightHotSpotInWorld(CVector &modelPos, float &modelRadius) const;

	/// \name AnimCtrl (IK...)
	// @{
	/** Set a special ctrl on a bone. see IAnimCtrl. NB: once an animCtrl is set to a bone in a skeleton,
	 *	his bones are always computed each frame.
	 *	set to NULL if you want to reset this bone AnimCtrl.
	 */
	void			setBoneAnimCtrl(uint boneId, IAnimCtrl *ctrl);
	IAnimCtrl		*getBoneAnimCtrl(uint boneId) const;
	// @}


	/// \name ShadowMap CTransform Implementation
	// @{
	virtual	void		generateShadowMap(const CVector &lightDir);
	virtual	CShadowMap	*getShadowMap();
	virtual bool		computeWorldBBoxForShadow(NLMISC::CAABBox &worldBB);
	virtual void		renderIntoSkeletonShadowMap(CSkeletonModel *rootSkeleton, CMaterial	&castMat);
	// @}

	// TMP nico debug function : retrieve all max bone spheres in world coordinates
	void				getWorldMaxBoneSpheres(std::vector<NLMISC::CBSphere> &dest) const;

// ***********************
protected:
	/// Constructor
	CSkeletonModel();
	/// Destructor
	virtual ~CSkeletonModel();

	/// Build link to traversals.
	virtual	void	initModel();

private:
	static CTransform	*creator() {return new CSkeletonModel;}
	friend	class CSkeletonShape;


public:
	// update if needed the renderList
	void						updateSkinRenderLists();

private:

	// The iterator of the skeleton inserted in the scene
	std::list<CSkeletonModel*>::iterator	_ItSkeletonInScene;

	/// skins/sticked objects
	typedef	std::set<CTransform*>		TTransformSet;
	typedef	TTransformSet::iterator		ItTransformSet;
	/// The skins.
	TTransformSet				_Skins;
	/// The StickedObjects.
	TTransformSet				_StickedObjects;

	// see dirtSkinRenderLists
	bool						_SkinToRenderDirty;
	// see _LevelDetail
	bool						_DefaultMRMSetup;
	// Raw lists of Skins. Both for transparent and opaque pass
	typedef NLMISC::CObjectVector<CTransform*, false>	TTransformArray;
	TTransformArray				_OpaqueSkins;
	TTransformArray				_TransparentSkins;
	// Skins which need to be animated (very rare)
	TTransformArray				_AnimDetailSkins;

	// The level detail used to drive MRM skins
	CMRMLevelDetail				_LevelDetail;
	// build a bug-free level detail
	void						buildDefaultLevelDetail();

	/// \name Bone Usage.
	// @{

	struct CBoneUsage
	{
		/// The bone Usage (refCount).
		uint16			Usage;
		/// Same as Usage, but must be animated/computed, even if Skeleton Lods say not (stickedObjects).
		uint16			ForcedUsage;
		/** Same as ForcedUsage, but must be animated/computed, even if the skeleton is in CLod state
		 *	ie displayed with a CLodCharacterShape. This is important for skeletons which have skeletons
		 *	sons sticked on them
		 */
		uint16			CLodForcedUsage;
		/** The current state: which bones need to be computed. ie:
		 *	(CLodForcedUsage) | ( ((Usage & currentLodUsage) | ForcedUsage) & skeleton not in CLod state )
		 */
		uint16			MustCompute;
		/// Myself if MustCompute==true, or the first parent with MustCompute==true.
		uint			ValidBoneSkinMatrix;
	};

	// A bone to compute information
	struct CBoneCompute
	{
		// The bone
		CBone			*Bone;
		// Father of the bone. May be NULL
		CBone			*Father;
		// true if must interpolate this bone with next lod
		bool			MustInterpolate;
	};

	/// The list of BoneUsage. Updated by Meshes, stickObject(), and lod changes.
	std::vector<CBoneUsage>		_BoneUsage;
	/// List of bones to compute.
	std::vector<CBoneCompute>	_BoneToCompute;
	/// Flag set if the MRSkeleton lod change, or if a new bone Usage change (only if was 0 or become 0).
	bool						_BoneToComputeDirty;
	/// The current lod activated for this skeleton
	uint						_CurLod;
	/// The current lod Interpolation Value for this skeleton
	float						_CurLodInterp;
	/// For lod interpolation. Inverse of distance. If 0, disable interpolation.
	float						_LodInterpMultiplier;


	/// called by CSkeletonShape::createInstance(). init the vector.
	void		initBoneUsages();

	/// increment the refCount of the ith bone and its parents. for stickObjects.
	void		incForcedBoneUsageAndParents(uint i, bool forceCLod);
	/// increment the refCount of the ith bone and its parents. for stickObjects.
	void		decForcedBoneUsageAndParents(uint i, bool forceCLod);

	/// According to Usage, ForedUsage and current skeleton Lod, update MustCompute and ValidBoneSkinMatrix
	void		updateBoneToCompute();

	// @}


	/// \name CLod / Character Lod
	// @{

	// return the contribution of lights (for Character Lod render).
	const CLightContribution	&getSkeletonLightContribution() {return _LightContribution;}

	/** True if the skeleton model and his skins have to be displayed with a CLodCharacterShape, instead of the std way
	 *	This state is modified early during the HRC Traversal. Because Clip traversal need this result.
	 */
	bool					_DisplayedAsLodCharacter;

	/// is enable LOD
	bool _IsEnableLOD;

	/// see setLodCharacterDistance
	float					_LodCharacterDistance;
	float					_OOLodCharacterDistance;

	/// The Lod instance. -1 by default
	CLodCharacterInstance	_CLodInstance;

	/** dirt when a bindSkin/stickObject/detachSkeletonSon is called
	 */
	bool					_CLodVertexAlphaDirty;

	/// see setLodEmit()
	NLMISC::CRGBA			_LodEmit;


	void				dirtLodVertexAlpha() {_CLodVertexAlphaDirty= true;}

	/// recompute _CLodVertexAlpha, ignoring _CLodVertexAlphaDirty
	void				computeCLodVertexAlpha(CLodCharacterManager *mngr);

	// @}


	/// \name Rendering
	// @{
	/** render the skeleton as a CLod.
	 *  - update instance Lighting
	 *  - render the lod.
	 *	- no op if passTransparent
	 */
	void			renderCLod();

	/** render the skins of the skeleton
	 *  - update instance Lighting, and setup Driver lighting
	 *	- activate skeleton Matrix
	 *  - render all the skins (according if passOpaque or not)
	 */
	void			renderSkins();

	/** render a list of skin, no lighting setup etc..., but use where possible a CVertexStreamManager
	 */
	void			renderSkinList(NLMISC::CObjectVector<CTransform*, false>	&skinList, float alphaMRM);
	// @}

	/// \name AnimCtrl (IK...)
	// @{
	// If >0, then user may change any if this bone each frame...
	sint			_AnimCtrlUsage;
	// @}


	// SkeletonModel can generate Shadow Map
	CShadowMap			*_ShadowMap;
	void			updateShadowMap(IDriver *driver);
	void			renderShadowSkins(CMaterial	&castMat);


	/// \name Spawn Script Animation
	// @{

	// The animated Spawn script
	CAnimatedValueString			_SpawnScript;
	// The manager which evaluates the script and appropriate process
	CSkeletonSpawnScript			_SpawnScriptEvaluator;
	// Special for World Objects
	CVector							_SSSWOPos;
	CVector							_SSSWODir;
	sint							_SpawnScriptChannelId;

	// Default track
	static CTrackDefaultString		_DefaultSpawnScript;		// ""
	// @}

protected:
	virtual void			createShadowMap();
	virtual void			deleteShadowMap();

};


// ***************************************************************************
/*
 *	Function used by CMeshMRM and CMeshMRMSkinned, to compute CMatrix3x4 of used skeleton bones
 */
template <class TMatrixArray>
inline void	computeBoneMatrixes3x4(TMatrixArray &boneMat3x4, const std::vector<uint32> &matInfs, const CSkeletonModel *skeleton)
{
	// For all matrix this lod use.
	for(uint i= 0; i<matInfs.size(); i++)
	{
		// Get Matrix info.
		uint	matId= matInfs[i];
		const CMatrix		&boneMat= skeleton->getActiveBoneSkinMatrix(matId);

		// compute "fast" matrix 3x4.
		// resize Matrix3x4.
		if(matId>=boneMat3x4.size())
		{
			boneMat3x4.resize(matId+1);
		}
		boneMat3x4[matId].set(boneMat);
	}
}


/// Same as computeBoneMatrixes3x4, but premul by a matrix.
template <class TMatrixArray>
inline void	computeBoneMatrixes3x4PreMul(TMatrixArray &boneMat3x4, const CMatrix &preMulMat, const std::vector<uint32> &matInfs, const CSkeletonModel *skeleton)
{
	// For all matrix this lod use.
	for(uint i= 0; i<matInfs.size(); i++)
	{
		// Get Matrix info.
		uint	matId= matInfs[i];
		const CMatrix		&boneMat= skeleton->getActiveBoneSkinMatrix(matId);
		CMatrix		boneMatMul;
		boneMatMul.setMulMatrixNoProj(preMulMat, boneMat);

		// compute "fast" matrix 3x4.
		// resize Matrix3x4.
		if(matId>=boneMat3x4.size())
		{
			boneMat3x4.resize(matId+1);
		}
		boneMat3x4[matId].set(boneMatMul);
	}
}





} // NL3D


#endif // NL_SKELETON_MODEL_H

/* End of skeleton_model.h */
