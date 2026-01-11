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

#ifndef NL_U_SKELETON_H
#define NL_U_SKELETON_H

#include "nel/misc/types_nl.h"
#include "nel/misc/aabbox.h"
#include "u_transform.h"
#include "animation_time.h"


namespace NL3D
{

class	UInstance;
class	UBone;
class	UPlayList;
class	IAnimCtrl;

// ***************************************************************************
/**
 * Game interface for manipulating Skeleton.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class USkeleton : public UTransform
{
public:


	/// \name Skin operation.
	// @{
	/** bind a MeshInstance skin to the skeleton. NB: ~UTransform() and ~USkeleton() call detachSkeletonSon().
	 * NB: nlerror() if mi is not a UInstance. (ie a mesh instance).
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 * NB: when a skin is binded, the command hide(), show(), ... have no effect on it, until it is detachSkeletonSon()-ed
	 * \return false if mi is NULL or not skinnable, true otherwise
	 */
	bool		bindSkin(UInstance mi);
	/** parent a CTransform to a bone of the skeleton. NB: ~CTransform() calls detachSkeletonSon().
	 * This object will be visible only when the Skeleton is not clipped.
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 */
	void		stickObject(UTransform mi, uint boneId);
	/** same method as stickObject(), but if you set forceCLod as true, then this object will be visible
	 *	even if the skeleton father is in CLod state (ie displayed with a CLodCharacterShape)
	 *	NB: if "mi" is a skeleton model, forceCLod is considerer true, whatever the value passed in.
	 */
	void		stickObjectEx(UTransform mi, uint boneId, bool forceCLod);
	/** unparent a CTransform from a bone of the skeleton, or unbind a skin. No-op if not here.
	 * NB: mi is placed at root of hierarchy.
	 */
	void		detachSkeletonSon(UTransform mi);
	/** Get the array of sticked objects
	 */
	void		getStickedObjects(std::vector<UTransform> &sticks);
	// @}


	/// \name Bone access.
	// @{
	/// retrieve the number of bones.
	uint		getNumBones() const;
	/// retrieve the bone. nlerror if not here. (>=getNumBones())
	UBone		getBone(uint boneId) const;
	/// retrieve the bone Id, by his name. -1 if not found.
	sint		getBoneIdByName(const std::string &boneName) const;
	/// Tell if a bone has been computed in the last frame or not. false if boneId is invalid
	bool		isBoneComputed(uint boneId) const;
	/// Force to compute a bone, even if object clipped. false if boneId is invalid
	bool		forceComputeBone(uint boneId);
	// @}


	/// \name Bone Lod interaction / MRM
	// @{

	/// return the number of bones currently animated/computed (because of bindSkin()/stickObject() / Lod system).
	uint		getNumBoneComputed() const;

	/** change the Lod Bone interpolation distance (in meters). If 0, interpolation is disabled.
	 *	The smaller this value is, the more Lod skeleton system will "pop". Default is 0.5 meters.
	 */
	void		setInterpolationDistance(float dist);
	/// see setInterpolationDistance()
	float		getInterpolationDistance() const;


	/** Change Max Display Skeleton distance. After this distance the shape won't be displayed.
	 *	setting <0 means -1 and so means DistMax = infinite (default in meshs but multilod meshes).
	 *	NB: This apply to the shape direclty!! ie All instances using same shape will be affected
	 *
	 *	Note: If the skeleton himself is sticked to another skeleton, this setup is not taken into account.
	 *	ie the skeleton clip follow the ancestor skeleton clip result (ie the first skeleton in hierarchy
	 *	which is not sticked).
	 *
	 *	Note (complex): same remark for QuadGridClipManager interaction with this function as in
	 *	UInstance::setShapeDistMax()
	 */
	void		setShapeDistMax(float distMax);

	/// see setShapeDistMax()
	float		getShapeDistMax() const;

	/** Special version for skins. NB: skins never follow their original MRM distance setup, but follow
	 *	this skeleton MRM setup. Default is 3-10-50.
	 *	NB: Unlike UInstance::changeMRMDistanceSetup(), this setup applies to the SkeletonModel, not the shape.
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);

	// @}


	/// \name CLod / Character Lod
	/**	At a certain distance, the whole skeleton and all its skins may be replaced with a small Lod, animated
	 *	with a CLodCharacterManager.
	 */
	// @{

	/** Change the Character Lod shape Id. set -1 if want to disable the feature (default)
	 */
	void		setLodCharacterShape(sint shapeId);
	/// see setLodCharacterShape
	sint		getLodCharacterShape() const;

	/// enable/disable LOD
	void		enableLOD(bool isEnable);

	/// Change/get the Character Lod anim setup.
	void		setLodCharacterAnimId(uint animId);
	uint		getLodCharacterAnimId() const;
	void		setLodCharacterAnimTime(TGlobalAnimationTime time);
	TGlobalAnimationTime	getLodCharacterAnimTime() const;

	/// tells if the animation must loop or clamp.
	void		setLodCharacterWrapMode(bool wrapMode);
	bool		getLodCharacterWrapMode() const;

	/** True if the skeleton model and his skins have been displayed with a CLodCharacterShape at last scene render
	 */
	bool		isDisplayedAsLodCharacter() const;

	/** This is the distance at which the skeleton use a CLodCharacterShape to display himself
	 *	if 0, never display the skeleton as a CLodCharacterShape
	 */
	void		setLodCharacterDistance(float dist);

	/// see setLodCharacterDistance. 0 if disabled
	float		getLodCharacterDistance() const;

	/** Call it when you want the system to recompute the Lod texture
	 *	NB: Lod texturing is possible only in conjunction with AsyncTextureManager. Hence, instances skinned
	 *	to the skeleton should be in AsyncTextureMode.
	 *	For best result, you should wait that each of these instances are isAsyncTextureReady() (texture loaded)
	 */
	void		computeLodTexture();

	/// Set the emissive of the skeleton model, when it is rendered in CLod form. Default to Black
	void			setLodEmit(NLMISC::CRGBA emit);
	NLMISC::CRGBA	getLodEmit() const;

	// @}


	/// \name Misc.
	// @{
	/** Retrieve the current approx BBox around the skeleton, computed in the last USene::render().
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
	 *	\param bbox return the bbox of the skinned skeleton, local to the skeleton. If the skeleton is not skinned/sticked
	 *	at all, bbox is not modified.
	 *	\param playList set NULL if no one, else this playList will be played at the time playTime
	 *  \param forceCompute force evaluation of bbox even if not skinned
	 *	\param computeInWorld true if want to get the bbox in world.
	 *	\return true if the bbox is computed, false otherwise.
	 */
	bool		computeCurrentBBox(NLMISC::CAABBox &bbox, UPlayList *playList, double playTime=0, bool forceCompute = false, bool computeInWorld= false);

	/// SkeletonSpawnScript (SSS) special: World Spawned objects are still relative to this position (default: Null)
	void					setSSSWOPos(const NLMISC::CVector &pos);
	const NLMISC::CVector	&getSSSWOPos() const;
	/// SkeletonSpawnScript special: World Spawned objects are still relative to this direction (default: J)
	void					setSSSWODir(const NLMISC::CVector &dir);
	const NLMISC::CVector	&getSSSWODir() const;

	// get the shape name. empty if no instance bound
	const std::string		&getShapeName() const;

	// @}


	/// \name AnimCtrl (IK...)
	// @{
	/** Set a special ctrl on a bone. see IAnimCtrl.
	 *	set to NULL if you want to reset this bone AnimCtrl.
	 *	No-op if Bad BoneId.
	 */
	void		setBoneAnimCtrl(uint boneId, IAnimCtrl *ctrl);
	/// return NULL if bad BoneId
	IAnimCtrl	*getBoneAnimCtrl(uint boneId) const;
	// @}

	// dynamic cast from a transform. empty if cast fail
	void		cast(UTransform object);

	/// Proxy interface

	/// Constructors
	USkeleton() { _Object = NULL; }
	USkeleton(class CSkeletonModel *object) { _Object = (ITransformable*)object; };
	/// Attach an object to this proxy
	void			attach(class CSkeletonModel *object) { _Object = (ITransformable*)object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CSkeletonModel	*getObjectPtr() const {return (CSkeletonModel*)_Object;}
};


} // NL3D


#endif // NL_U_SKELETON_H

/* End of u_skeleton.h */
