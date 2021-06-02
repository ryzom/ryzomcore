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

#ifndef NL_U_INSTANCE_H
#define NL_U_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "u_transform.h"
#include "u_instance_material.h"
#include "u_shape.h"
#include "nel/misc/aabbox.h"


namespace NL3D
{


class	UInstanceMaterial;


// ***************************************************************************
/**
 * Game interface for manipulating Objects, animations etc...
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UInstance : public UTransform
{
public:


	/** Get the untransformed AABBox of the mesh. NULL (gtSize()==0) if no mesh.
	 */
	void				getShapeAABBox(NLMISC::CAABBox &bbox) const;

	/**
	 * Set the blend shape factor for this instance
	 * blendShapeName is the name of the blendshape we want to set
	 * factor the blendshape percentage from -100.0 to 100.0
	 * dynamic tells the optimizer if the blendshape have to change in real time
	 */
	void				setBlendShapeFactor (const std::string &blendShapeName, float factor, bool dynamic);

	/// \name Material access.
	// @{
	/// return number of materials this mesh instance use.
	uint				getNumMaterials() const;
	/// return a local access on a material, to change its values. (NB: overwrite, if animated).
	UInstanceMaterial	getMaterial(uint materialId);
	/** Select textures of material among several sets (if available)
	 *	NB: if success and if getAsyncTextureMode()==true, then setAsyncTextureDirty(true) is called
	 */
	void selectTextureSet(uint id);
	// @}

	/** Change MRM Distance setup. Only for mesh which support MRM. NB MeshMultiLod apply it only on Lod0
	 *	(if Lod0 is a MRM).
	 *	NB: This apply to the shape directly!! ie All instances using same shape will be affected
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);


	/** Change Max Display distance. After this distance the shape won't be displayed.
	 *	setting <0 means -1 and so means DistMax = infinite (default in meshs but multilod meshes).
	 *	NB: This apply to the shape directly!! ie All instances using same shape will be affected
	 *
	 *	Note: If the instance is skinned/sticked to a skeleton, this setup is not taken into account. But you can
	 *	call USkeleton::setShapeDistMax() to have same effect.
	 *
	 *	Note (complex): All instances of the same shape which are freezeHRC()-ed and are linked to the
	 *	QuadGridClipManager (ie not linked to a cluster) may not be updated correctly.
	 *	In other words, you should setup this value only at beginning of program, just after creating your
	 *	instance (more exactly before UScene::render()), and all instances of the same shape should be setuped
	 *	with same value (or don't call setShapeDistMax() for subsequent instances).
	 *	If you don't do this, QuadGridClipManager may clip such instances nearer than they should
	 */
	void		setShapeDistMax(float distMax);

	/// see setShapeDistMax()
	float		getShapeDistMax() const;

	/// Test if there is a start/stop caps in the objects (some fxs such as remanence)
	bool		canStartStop();
	// For instance that have a start/stop caps
	void		start();
	// For instance that have a start/stop caps
	void		stop() ;
	// For instance that have a start/stop caps
	bool		isStarted() const;

	// Get the model distmax.
	float		getDistMax() const;
	// Set the model distmax.
	void		setDistMax(float distMax);
	// If the model has a coarse mesh, it set its dist. Set to -1 to keep default
	void		setCoarseMeshDist(float dist);
	// If the model has a coarse mesh, it returns its distance if it has been set, or -1 if default is used (or if no coarse mesh present)
	float		getCoarseMeshDist() const;


	/// \name Async Texture Loading
	/** All those methods no-op or return 0/false if the instance is not a CMeshBaseInstance.
	 *	isAsyncTextyreReady() return true if the instance is not a CMeshBaseInstance.
	 */
	// @{
	/** if true, the instance is said in "AsyncTextureMode". Ie user must fill AsyncTextures field with name of the
	 *	textures to load. At each startAsyncTextureLoading(), the system start to load async them.
	 *	Then, isAsyncTextureReady() should be test each frame, to know if loading has completed.
	 *	By default, AsyncTextureMode=false.
	 *	When it swap from false to true, each texture file in Materials are replaced with
	 *	"blank.tga", and true fileNames are copied into AsyncTextures.
	 *	When it swap from true to false, the inverse is applied.
	 *	NB: calling enableAsyncTextureMode(true) calls setAsyncTextureDirty(true)
	 */
	void		enableAsyncTextureMode(bool enable);
	bool		getAsyncTextureMode() const;
	/** Start to load all textures in AsyncTextures array (if needed)
	 *	NB: old setup is kept in Material => instance is still rendered with "coherent" textures, until new textures
	 *	are ready
	 *	no op if not in async texture mode.
	 */
	void		startAsyncTextureLoading();
	/**	return true if all the async textures of the instances are uploaded.
	 *	if was not ready before, this swap the upload textures into the rendered ones so they are rendered
	 *	return always true if not in async texture mode, or if startAsyncTextureLoading() has not been called
	 *	since last enableAsyncTextureMode(true)
	 */
	bool		isAsyncTextureReady();

	/** For Lod of texture, and load balancing, set the approximate distance of the instance to the camera.
	 */
	void		setAsyncTextureDistance(float dist);
	/** \see setAsyncTextureDistance()
	 */
	float		getAsyncTextureDistance() const;

	/** User is free to flag this state, to know if startAsyncTextureLoading() should be called.
	 *	Internal system don't use this flag.
	 *	Default is false
	 */
	void		setAsyncTextureDirty(bool flag);
	/// see dirtAsyncTextureState()
	bool		isAsyncTextureDirty() const;

	// @}

	/** Trails specific. Set the slice time (period used to sample the trail pos)
	  * If the object is not a trail, this has no effect
	  */
	// @{
	void		setSliceTime(float duration);
	float		getSliceTime() const;
	// @}

	/** Test if driver support rendering of all material of that shape.
	  * \param  forceBaseCaps When true, the driver is considered to have the most basic required caps (2 stages hardwares, no pixelShader), so that any fancy material will fail the test.
	  */
	bool		supportMaterialRendering(UDriver &drv, bool forceBaseCaps);

	/// get the shape. NULL if no instance bound
	UShape		getShape() const;

	// get the shape name. empty if no instance bound
	const std::string &getShapeName() const;

	// dynamic cast from a transform. empty if cast fail
	void		cast(UTransform object);

	/// \name access default position. Valid only for CMeshBaseInstance
	/// NB: return false if the instance is not a CMeshBaseInstance (value not modified)
	// @{
	bool					getDefaultPos (CVector &) const;
	bool					getDefaultRotQuat (CQuat &) const;
	bool					getDefaultScale (CVector &) const;
	/** Set a scale relative to the default exported matrix
	 *	NB: really useful for instance if you want to scale an instance relatively to the scale
	 *	exported from the artist (if he had not set a "reset XForm")
	 *	NB: no op if the object is not a CMeshBaseInstance
	 */
	void					setRelativeScale (const CVector &rs);
	// @}

	/// Proxy interface

	/// Constructors
	UInstance() { _Object = NULL; };
	UInstance(class CTransformShape *object) { _Object = (ITransformable*)object; };
	/// Attach an object to this proxy
	void			attach(class CTransformShape *object) { _Object = (ITransformable*)object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CTransformShape	*getObjectPtr() const {return (CTransformShape*)_Object;}
};

} // NL3D

#endif // NL_U_INSTANCE_H

/* End of u_instance.h */
