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

#ifndef NL_MESH_BASE_INSTANCE_H
#define NL_MESH_BASE_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/material.h"
#include "nel/3d/animated_material.h"
#include "nel/3d/animated_lightmap.h"
#include "nel/3d/animated_morph.h"
#include "nel/3d/async_texture_block.h"


namespace NL3D
{


class CMeshBase;
class CMesh;
class CMeshMRM;
class CAnimatedLightmap;
class CAsyncTextureManager;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshBaseInstanceId=NLMISC::CClassId(0xef44331, 0x739f6bcf);


// ***************************************************************************
/**
 * An base class for instance of CMesh and CMeshMRM  (which derive from CMeshBase).
 * NB: this class is a model but is not designed to be instanciated in CMOT.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshBaseInstance : public CTransformShape
{
public:
	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();

public:


	/** The list of materials, copied from the mesh.
	 * Each CMeshBaseInstance has those materials, so they can be animated or modified for each instance.
	 * By default, they are copied from the Mesh.
	 */
	std::vector<CMaterial>			Materials;

	/** For Aynsc Texture Loading. This has the same size as Materials.
	 *	User can fill here the name of the texture he want to async load.
	 *	WARNING: once AsyncTextureMode is set, Material's Texture fields should not be modified, else
	 *	undefined results
	 */
	std::vector<CAsyncTextureBlock>	AsyncTextures;


	/// \name IAnimatable Interface (registering only IAnimatable sons).
	// @{
	enum	TAnimValues
	{
		OwnerBit= CTransformShape::AnimValueLast,

		AnimValueLast,
	};


	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);

	// @}

	/// \name Derived from CTransformShape.
	// @{
	virtual uint		getNumMaterial () const;
	virtual const CMaterial	*getMaterial (uint materialId) const;
	virtual CMaterial	*getMaterial (uint materialId);
	// @}

	/// \name Derived from ITransformable.
	// @{
	/// Default Track Values.
	virtual ITrack* getDefaultTrack (uint valueId);
	// @}

	/// \name LightMap properties
	// @{
	uint32 getNbLightMap();
	void getLightMapName( uint32 nLightMapNb, std::string &LightMapName );
	// @}

	// To init lightmap information
	void initAnimatedLightIndex (const CScene &scene);

	/// \name BlendShape properties
	// @{
	// Interface
	uint32 getNbBlendShape();
	void getBlendShapeName (uint32 nBlendShapeNb, std::string &BlendShapeName );
	void setBlendShapeFactor (const std::string &BlendShapeName, float rFactor);

	// Internal
	std::vector<CAnimatedMorph>* getBlendShapeFactors()
	{
		return &_AnimatedMorphFactor;
	}
	// @}

	/** Change MRM Distance setup. Only for mesh which support MRM. NB MeshMultiLod apply it only on Lod0.
	 *	NB: This apply to the shape direclty!! ie All instances using same shape will be affected
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	virtual void		changeMRMDistanceSetup(float /* distanceFinest */, float /* distanceMiddle */, float /* distanceCoarsest */) {}

	/** If there are selectable texture in this mesh shape, this replace the matching material instances with the right texture
	 *	If getAsyncTextureMode()==true, then this replace the AsyncTexture fileNames, instead of the Materials file Names.
	 */
	void selectTextureSet(uint id);


	/// \name Async Texture Loading
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
	void			enableAsyncTextureMode(bool enable);
	bool			getAsyncTextureMode() const {return _AsyncTextureMode;}
	/** Start to load all textures in AsyncTextures array (if needed)
	 *	NB: old setup is kept in Material => instance is still rendered with "coherent" textures, until new textures
	 *	are ready
	 *	no op if not in async texture mode.
	 */
	void			startAsyncTextureLoading(const NLMISC::CVector &position);
	/**	return true if all the async textures of the instances are uploaded.
	 *	if was not ready before, this swap the
	 *	return always true if not in async texture mode, or if startAsyncTextureLoading() has not been called
	 *	since last enableAsyncTextureMode(true)
	 */
	bool			isAsyncTextureReady();

	/** For Lod of texture, and load balancing, set the approximate distance of the instance to the camera.
	 */
	void			setAsyncTextureDistance(float dist) {_AsyncTextureDistance= dist;}
	float			getAsyncTextureDistance() const {return _AsyncTextureDistance;}

	/** User is free to flag this state, to know if startAsyncTextureLoading() should be called.
	 *	Internal system don't use this flag.
	 *	Default is false
	 */
	void			setAsyncTextureDirty(bool flag) {_AsyncTextureDirty= flag;}
	/// see dirtAsyncTextureState()
	bool			isAsyncTextureDirty() const {return _AsyncTextureDirty;}

	/** Get an AynscTextureId. ret -1 if not found, or not a textureFile.
	 *	NB: the id returned is the one in _CurrentAsyncTexture it the valid ones (thoses loaded or being loaded)
	 *	Can be used for some (non deleting) request to the AsyncTextureManager
	 */
	sint			getAsyncTextureId(uint matId, uint stage) const;

	// @}

	/// \name CTransform traverse specialisation
	// @{
	/** this do :
	 *	- animate channel mixer for pos, rot, scale etc..
	 *	- call standard CTransform::traverseHrc()
	 */
	virtual void	traverseHrc();
	/** this do :
	 *  - call CTransformShape::traverseAnimDetail()
	 *  - update animated materials.
	 */
	virtual void	traverseAnimDetail();
	// @}


	/// \name Misc
	// @{
	/// see CTransform::fastIntersect()
	virtual bool		fastIntersect(const NLMISC::CVector &p0, const NLMISC::CVector &dir, float &dist2D, float &distZ, bool computeDist2D);
	// @}

protected:
	/// Constructor
	CMeshBaseInstance();
	/// Destructor
	virtual ~CMeshBaseInstance();


private:
	static CTransform	*creator() {return new CMeshBaseInstance;}
	friend	class CMeshBase;


	/** The list of animated materials, instanciated from the mesh.
	 */
	std::vector<CAnimatedMaterial>	_AnimatedMaterials;

	// Index of the Animated lightmap in the scene
	std::vector<sint>				_AnimatedLightmap;

	std::vector<CAnimatedMorph>		_AnimatedMorphFactor;

	/// \name Async Texture Loading
	// @{
	/// 0 if all the texture are async loaded. Setup by the CAsyncTextureManager
	friend	class	CAsyncTextureManager;
	sint							_AsyncTextureToLoadRefCount;
	bool							_AsyncTextureDirty;
	bool							_AsyncTextureMode;
	bool							_AsyncTextureReady;
	// A copy of AsyncTextures done at each startAsyncTextureLoading().
	std::vector<CAsyncTextureBlock>	_CurrentAsyncTextures;
	// distance for texture load balancing
	float							_AsyncTextureDistance;

	void			releaseCurrentAsyncTextures();

	// @}

/// public only for IMeshVertexProgram classes.
public:

	/// CMeshVPWindTree instance specific part.
	float		_VPWindTreePhase;		// Phase time of the wind animation. 0-1
	bool		_VPWindTreeFixed;       // Enable lighting for mesh vp wind tree material when no vp are available (else vertex color is used to tint the object ...)

};



} // NL3D


#endif // NL_MESH_BASE_INSTANCE_H

/* End of mesh_base_instance.h */
