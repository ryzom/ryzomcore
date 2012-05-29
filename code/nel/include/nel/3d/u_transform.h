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

#ifndef NL_U_TRANSFORM_H
#define NL_U_TRANSFORM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "u_instance_group.h"
#include "u_transformable.h"


namespace NL3D
{

using NLMISC::CVector;
using NLMISC::CMatrix;
using NLMISC::CQuat;

class	ILogicInfo;
class	CCluster;

// ***************************************************************************
/**
 * Base interface for manipulating Movable Objects in the scene: camera, lights, instances etc...
 * see UTransformable. A UTransform can be set in a hierachy, and can be hidden.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UTransform : public UTransformable
{
public:
	// Enum should be the same than in CHrcTrav.

	/// The visibility flag. In the root case, Herit means Show.
	enum	TVisibility
	{
		Show=0,		// The model is shown in the hierarchy
		Hide,		// The model is hidden in the hierarchy
		Herit,		// The model herit the visibilty from his father

		VisibilityCount
	};


public:


	/// \name Hierarchy manipulation
	// @{
	/** Hierarchy edit. unlink this from oldparent, and make this be a son of newFather.
	 * if this was already a son of newFather, no-op.
	 * \param newFather the new Father. If NULL, the transform will be linked to the root of the hierarchy (Default!).
	 */
	void			parent(UTransform newFather);


	void			setClusterSystem (UInstanceGroup *pIG);
	UInstanceGroup	*getClusterSystem () const;
// @}


	/// \name visibility
	// @{
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
	TVisibility		getVisibility();
	// @}


	/// \name freezeHRC
	// @{
	/** Optimisation: freeze the HRC so the WorldMatrix computed at next render() will be kept for long, and
	 *	the model won't either be tested in HRC.
	 *
	 *	NB: the model won't be tested in HRC only if this model is a "root"
	 *	For maximum optimisation, you should freezeHRC() all the models of a hierarchy, from base root to leaves.
	 *
	 *	NB: if the hierarchy of this object must change, or if the object must moves, you must call unfreezeHRC() first,
	 *	and you should do this for all the parents of this model.
	 */
	void			freezeHRC();


	/**	see freezeHRC().
	 */
	void			unfreezeHRC();
	// @}


	/// name Lighting Behavior.
	// @{
	/** Set the UserLightable flag. if false, isLightable() will always return false.
	 *	Doing this, user can disable lighting on a model which may be interesting for speed.
	 *	NB: most of models don't need it. For example models with LightMaps are by default Lighing-disabled.
	 *	Default behavior is UserLightable==true.
	 */
	void			setUserLightable(bool enable);

	/** Get the UserLightable flag.
	 */
	bool			getUserLightable() const ;
	// @}


	/** Set the current ordering layer for this transform.
	  * Typically, this is used to sort transparent objects. Isn't used with solid objects.
	  * For now :
	  * Layer 0 is for underwater
	  * Layer 1 is for water surfaces
	  * Layer 2 is for object above water
	  */
	void			setOrderingLayer(uint layer);

	/// Get the ordering layer
	uint			getOrderingLayer() const;


	/** Set the LogicInfo for this transfrom, eg to retrieve statc light information, see ILogicInfo.
	 *	Ptr is kept in UTransfrom, so should call setLogicInfo(NULL) before to clean up.
	 */
	void			setLogicInfo(ILogicInfo *logicInfo);


	/// Return true if the object was determined as Visible in Hrc during the last Scene->rendere(). NB: interesting only if Herit. else can use getVisibility()
	bool			getLastWorldVisState() const;

	/// Return true if the object was rendered during the last Scene->rendere(). return false else (ie clipped)
	bool			getLastClippedState() const;

	/// Fill a list of cluster that contain this tranform. This is valid after the clip traversal
	void			getLastParentClusters(std::vector<CCluster*> &clusters) const;

	/** get the last world matrix computed in last render().
	 *	NB: this WM is computed in last render() only if the object was not clipped. So use it wisely.
	 *	use getLastClippedState() to konw if the object was visible in last render().
	 */
	const CMatrix	&getLastWorldMatrixComputed() const ;


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
	void			setLoadBalancingGroup(const std::string &group);

	/** Get the load Balancing group of a model. see setLoadBalancingGroup().
	 */
	const std::string	&getLoadBalancingGroup() const ;

	// @}


	/// name Misc
	// @{

	/** set the Mean color of the transform. The mean color can be used for many purpose, such as drawing
	 *	objects if the textures are not loaded. It is used also for Lod Character.
	 *	Default color is (255,255,255)
	 */
	void			setMeanColor(NLMISC::CRGBA color);

	/// see setMeanColor()
	NLMISC::CRGBA	getMeanColor() const ;

	/** true if the transform support fast intersection (fastIntersect() works)
	 *	For now, only supports skeleton with ALL skins that are shadowSkined and MRM
	 */
	bool		supportFastIntersect() const;
	/** test if the transform intersect the ray (p0, dir).
	 *	\return false if not supported/no triangles, else true if can do the test (even if don't intersect!)
	 *	if intersect, dist2D=0, and distZ= Depth Distance
	 *	if don't intersect, dist2D="nearest distance to the ray", and distZ=0
	 *	\param computeDist2D if false and don't intersect, then return dist2D=FLT_MAX, and distZ=0
	 */
	bool		fastIntersect(const NLMISC::CVector &p0, const NLMISC::CVector &dir, float &dist2D, float &distZ, bool computeDist2D);

	// @}

	/// name Accessors for opacity/transparency
	// @{
	void			setTransparency(bool v);
	void			setOpacity(bool v);
	// return a non-zero value if true
	uint32			isOpaque();
	uint32			isTransparent();
	/** If the object is a multi-lod, preserve the last opacity / transparency set by the user.
      * False is the default, and causes the current LOD transparency / opacity to overwrite
	  * the corresponding flags for this transform (thus, setTransparency / setOpacity have no effect on multi-lod)
	  * unless this is set)
	  */
	void			setBypassLODOpacityFlag(bool bypass);

	/** Set priority for transparency ordering
	  * Transparent objects are sorted by priority, and then by distances
	  * The priority is clamped by the priority range of the scene. By default this range is [0, 0] so there's no priority sorting
	  * This range can be changed by calling UScene::setupTransparencySorting
	  */
	void			setTransparencyPriority(uint8 priority);
	// @}

	/// \name ShadowMapping
	// @{
	/** By default, map shadow casting is disabled. This enabled shadow for this model.
	 *	Fails if the model don't support dynamic Map Shadow Casting (eg landscape)
	 */
	void			enableCastShadowMap(bool state);
	/// true if the instance cast shadow. By default false
	bool			canCastShadowMap() const;

	/** By default, map shadow receiving is disabled. This enabled shadow for this model.
	 *	Fails if the model don't support dynamic Map Shadow Receiving (eg Particle system)
	 */
	void			enableReceiveShadowMap(bool state);
	/// true if the instance receive shadow. By default false
	bool			canReceiveShadowMap() const ;

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
	float				getShadowMapDirectionZThreshold() const;

	/** To limit some problems with interior, a limit of shadow depth by caster can be given.
	 *	This is the length in the lightDir direction where the shadow can touch receivers.
	 *	Can be used also to have some big objects that cast shadows further.
	 *	Default to 8.0.
	 */
	void				setShadowMapMaxDepth(float depth);
	float				getShadowMapMaxDepth() const;

	// @}

	/** Force the transform to always be attached to the root
	  * As a consequence, it can't be inserted into a cluster system (even the root cluster)
	  * and is thus always visible when in the frustum (not clusterized)
	  * NB : any call to setClusterSystem will be ignored (must remain unclesterized)
	  * NB : any call to parent will be ignored (must remain linked to the root)
      */
	void			setForceClipRoot(bool forceClipRoot);
	bool			getForceClipRoot() const;

	/// Proxy interface

	/// Constructors
	UTransform() { _Object = NULL; }
	UTransform(class CTransform *object) { _Object = (ITransformable*)object; };
	/// Attach an object to this proxy
	void			attach(class CTransform *object) { _Object = (ITransformable*)object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CTransform	*getObjectPtr() const {return (CTransform*)(_Object);}
};


} // NL3D


#endif // NL_U_TRANSFORM_H

/* End of u_transform.h */
