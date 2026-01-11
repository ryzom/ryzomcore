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

#ifndef NL_BONE_H
#define NL_BONE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/transformable.h"
#include "nel/3d/channel_mixer.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/bsphere.h"


namespace NL3D
{


// ***************************************************************************
class	IAnimCtrl;
class	CSkeletonModel;


// ***************************************************************************
/**
 * This is a bone default value. This object is stored in the SkeletonShape, and is serialised.
 * For hierarchy of bones, it contains the id of his father.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CBoneBase : public NLMISC::CRefCount
{
public:
	/// Name of this bone, for Animation access.
	std::string				Name;

	/// The Inverse of bindpos for this bone.
	CMatrix					InvBindPos;

	/// The Father of this bone. -1 means no one.
	sint32					FatherId;

	/// true if unheritScale from father (default==true).
	bool					UnheritScale;

	/// Default tracks.
	CTrackDefaultVector		DefaultPos;
	CTrackDefaultVector		DefaultRotEuler;
	CTrackDefaultQuat		DefaultRotQuat;
	CTrackDefaultVector		DefaultScale;
	CTrackDefaultVector		DefaultPivot;

	/// The distance at which the bone is disabled in the skeleton. If 0, never disable.
	float					LodDisableDistance;

	/** Additionally to the standard scale, you can multiply the effect on the skin with a special SkinScale
	 *	This scale is applied only on the skin (even son bones positions won't be affected)
	 *	Default to (1,1,1)
	 */
	CVector					SkinScale;

public:

	/// ctor, with default pos as NULL (but scale as 1,1,1).
	CBoneBase();

	/// save/load.
	void			serial(NLMISC::IStream &f);

};


// ***************************************************************************
/**
 * This is a bone, for skeleton animation, with information for result WorldMatrix.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CBone : public ITransformable
{
public:

	/** Constructor. build a bone from a CBoneBase*.
	 * By default, a bone is in RotQuat transform mode.
	 * This ctor:
	 *	- store a RefPtr on the bonebase (for getDefaultTracks() method). the refptr is just for nlassert.
	 *	- copy the bonebase default track value into Animated Values Pos/Rot etc....
	 */
	CBone(CBoneBase *boneBase);


	/// retrieve the boneName from BoneBase.
	const std::string	&getBoneName() const {nlassert(_BoneBase); return _BoneBase->Name;}
	/// retrieve the fatherId from BoneBase.
	sint32				getFatherId() const {nlassert(_BoneBase); return _BoneBase->FatherId;}
	/// retrieve the boneBase
	CBoneBase			&getBoneBase() const {nlassert(_BoneBase); return *_BoneBase;}


	/// \name Herited from ITransformable
	// @{
	/// retrieve the default track from skeleton shape.
	virtual ITrack* getDefaultTrack (uint valueId);

	/// register the ITransformable channels as detailled channels.
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);
	// @}


	/** Compute the LocalSkeletonMatrix, the WorldMatrix, and the BoneSkinMatrix (for skinning).
	 * NB: the result localSkeletonMatrix depends on BoneBase::UnheritScale. \n
	 * NB: the result worldMatrix depends on BoneBase::UnheritScale. \n
	 * NB: the result boneSkinMatrix depends on BoneBase::InvBindPos. \n
	 * \param parent the parent of this bone (maybe NULL if root). His WorldMatrix is used, so it should be computed before.
	 * \param rootMatrix is used as father worldmatrix if parent==NULL. Useful for computing WorldMatrix.
	 * \param skeletonForAnimCtrl if NULL, no AnimCtrl is performed, else skeletonForAnimCtrl->getWorldMAtrix() should be == to rootMatrix
	 */
	void			compute(CBone *parent, const CMatrix &rootMatrix, CSkeletonModel *skeletonForAnimCtrl);

	/** Interpolate the current result of _BoneSkinMatrix with otherMatrix.
	 *	when interp==0.f, _BoneSkinMatrix= otherMatrix.
	 *	NB: the interpolation is made on per-vector basis => bad matrix interpolation.
	 */
	void			interpolateBoneSkinMatrix(const CMatrix &otherMatrix, float interp);

	/// retrieve the matrix local to the skeleton, computed in compute().
	const CMatrix	&getLocalSkeletonMatrix() const {return _LocalSkeletonMatrix;}

	/// retrieve the WorldMatrix computed in compute().
	const CMatrix	&getWorldMatrix() const {return _WorldMatrix;}

	/// retrieve the BoneSkinMatrix computed in compute().
	const CMatrix	&getBoneSkinMatrix() const {return _BoneSkinMatrix;}


	/// enable the channels (lodEnable) associated to this bone in the channelMixer.
	void			lodEnableChannels(CChannelMixer *chanMixer, bool enable);

	/** Force to eval the animation of that bone
	  * Useful when a bone position is needed, and if the father skeleton has been clipped (and thus not detail-animated)
	  * \param chanMixer the channel mixer to which that bone has been registered
	  */
	inline void			forceAnimate(CChannelMixer &chanMixer);


	/** Additionally to the standard scale, you can multiply the effect on the skin with a special SkinScale
	 *	This scale is applied only on the skin (even son bones positions won't be affected)
	 *	Default to (1,1,1)
	 */
	void				setSkinScale(CVector &skinScale);
	const CVector		&getSkinScale() const {return _SkinScale;}

// *************************
public:
	// Private to SkeletonModel. You should not set this ptr directly. see CSkeletonModel::setBoneAnimCtrl()
	// The extra controller (IK...) on this bone
	IAnimCtrl					*_AnimCtrl;

	// Private to SkeletonModel. This represent the max sphere for all skins around this bone
	NLMISC::CBSphere			_MaxSphere;

private:
	// the boneBase of the skeletonShape which create this bone..
	NLMISC::CRefPtr<CBoneBase>	_BoneBase;

	// The result Matrix, local to the skeleton.
	CMatrix						_LocalSkeletonMatrix;
	// The result WorldMatrix.
	CMatrix						_WorldMatrix;
	// The result Disaplcement _LocalSkeletonMatrix, local to the skeleton.
	CMatrix						_BoneSkinMatrix;

	// The bkuped channelIds for each channel of the bone. -1 if not registered (or no tracks in animationSet).
	sint						_PosChannelId;
	sint						_RotEulerChannelId;
	sint						_RotQuatChannelId;
	sint						_ScaleChannelId;
	sint						_PivotChannelId;

	// see setSkinScale()
	CVector						_SkinScale;
};


/////////////
// INLINES //
/////////////
inline void	CBone::forceAnimate(CChannelMixer &chanMixer)
{
	sint ids[] =
	{
		_PosChannelId,
		_RotEulerChannelId,
		_RotQuatChannelId,
		_ScaleChannelId,
		_PivotChannelId
	};
	chanMixer.evalChannels(ids, sizeof(ids) / sizeof(ids[0]));
}

} // NL3D


#endif // NL_BONE_H

/* End of bone.h */
