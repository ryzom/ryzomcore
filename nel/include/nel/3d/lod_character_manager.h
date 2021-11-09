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

#ifndef NL_LOD_CHARACTER_MANAGER_H
#define NL_LOD_CHARACTER_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
#include "nel/misc/object_vector.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/texture_blank.h"
#include "nel/3d/vertex_stream_manager.h"


namespace NLMISC
{
class	CMatrix;
class	CBitmap;
}


namespace NL3D
{


using NLMISC::CRGBA;
using NLMISC::CMatrix;
using NLMISC::CUV;

class	IDriver;
class	CLodCharacterShapeBank;
class	CLodCharacterShape;
class	CLodCharacterInstance;
class	CLodCharacterTexture;


// ***************************************************************************
/**
 * This bitmap is builded in the Instance texturing build process of CLodCharacterManager
 */
class CLodCharacterTmpBitmap
{
public:
	CLodCharacterTmpBitmap();

	// free memory. 1*1 pixel with black stored
	void			reset();
	/** build from a bitmap (NB: converted internally). Should be not so big (eg:64*64).
	 *	Width and height must be <=256
	 */
	void			build(const NLMISC::CBitmap &bmp);
	// build from a single color (for untextured materials)
	void			build(CRGBA col);

	// get a pixel from this bitmap.
	CRGBA			getPixel(uint8 U, uint8 V)
	{
		U>>= _UShift;
		V>>= _VShift;
		return _Bitmap[(V<<_WidthPower) + U];
	}

// **************
private:
	// The pixels.
	NLMISC::CObjectVector<CRGBA>	_Bitmap;
	// The powerOf2 of the width
	uint							_WidthPower;
	// The shift to apply from uint8 to fit in widht/height texture.
	uint							_UShift, _VShift;

};


// ***************************************************************************
/**
 * A Manager used to display CLodCharacter instances.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterManager
{
public:

	/// Constructor
	CLodCharacterManager();
	~CLodCharacterManager();


	/// \name build process
	// @{

	/// reset the manager.
	void			reset();

	/** create a Shape Bank. NB: a vector of ShapeBank is maintained internally, hence, not so many shapeBank should be
	 *	created at same Time.
	 *	\return	id of the shape Bank.
	 */
	uint32			createShapeBank();

	/// get a shape Bank. Useful for serialisation for example. return NULL if not found
	const CLodCharacterShapeBank	*getShapeBank(uint32 bankId) const;

	/// get a shape Bank. Useful for serialisation for example. return NULL if not found
	CLodCharacterShapeBank	*getShapeBank(uint32 bankId);

	/// delete a Shape Bank. No-op if bad id.
	void			deleteShapeBank(uint32 bankId);


	/** Get a shapeId by its name. -1 if not found.
	 *	Call valid only if compile() has been correctly called
	 */
	sint32			getShapeIdByName(const std::string &name) const;

	/// Get a const ref on a shape. Ptr not valid if shape Banks are modfied. NULL if not found
	const CLodCharacterShape	*getShape(uint32 shapeId) const;

	/** re-compile the shape map. This must be called after changing shape bank list.
	 *	It return false if same names have been found, but it is still correctly builded.
	 */
	bool			compile();

	// @}


	/// \name render process
	// @{

	/** set the max number of vertices the manager can render in one time. Default is 3000 vertices.
	 *	nlassert if isRendering()
	 */
	void			setMaxVertex(uint32 maxVertex);

	/// see setMaxVertex()
	uint32			getMaxVertex() const {return _MaxNumVertices;}

	/** set the number of vbhard to allocate for the vertexStream. The more, the better (no lock stall).
	 *	Default is 8.
	 *	With MaxVertices==3000 and numVBHard==8, this led us with 576 Ko in AGP. And this is sufficient cause it can
	 *	handle 300 entities of approx 80 vertices each frame with no lock at all.
	 */
	void			setVertexStreamNumVBHard(uint32 numVBHard);

	/// see setVertexStreamNumVBHard
	uint32			getVertexStreamNumVBHard() const {return _NumVBHard;}

	/** Start the rendering process, freeing VBuffer.
	 *	nlassert if isRendering()
	 *	NB: VBhard is locked here, so you must call endRender to unlock him (even if 0 meshes are rendered)
	 *
	 *	\param managerPos is to help ZBuffer Precision (see IDriver::setupViewMatrixEx). This vector is removed from
	 *	all instance worldMatrixes, and a IDriver::setupModelMatrix() will be done with this position.
	 *	Hence, whatever value you give, the result will be the same. But if you give a value near the camera position,
	 *	ZBuffer precision will be enhanced.
	 */
	void			beginRender(IDriver *driver, const CVector &managerPos);

	/** Add an instance to the render list.
	 *	nlassert if not isRendering()
	 *	initInstance() must have been called before (nlassert)
	 *	\param instance the lod instance information (with precomputed color/Uvs)
	 *	\param worldMatrix is the world matrix, used to display the mesh
	 *	\param ambient is the ambient used to simulate the lighting on the lod.
	 *	\param diffuse is the diffuse used to simulate the lighting on the lod.
	 *	\param lightDir is the diffuse used to simulate the lighting on the lod (should be the bigger light influence)
	 *		Don't need to be normalized (must do it internally)
	 *	\return false if the key can't be added to this pass BECAUSE OF TOO MANY VERTICES reason. If the shapeId or animId
	 *	are bad id, it return true!! You may call endRender(), then restart a block. Or you may just stop the process
	 *	if you want.
	 */
	bool			addRenderCharacterKey(CLodCharacterInstance &instance, const CMatrix &worldMatrix,
		CRGBA ambient, CRGBA diffuse, const CVector &lightDir);

	/**	compile the rendering process, effectively rendering into driver the lods.
	 *	nlassert if not isRendering().
	 *	The VBHard is unlocked here.
	 */
	void			endRender();

	/// tells if we are beetween a beginRender() and a endRender()
	bool			isRendering() const {return _Rendering;}

	/** Setup a correction matrix for Lighting. Normals are multiplied with this matrix before lighting.
	 *	This is important in Ryzom because models (and so Lods) are building with eye looking in Y<0.
	 *	But they are animated with eye looking in X>0.
	 *	The default setup is hence a matrix which do a RotZ+=90.
	 *	\see addRenderCharacterKey
	 */
	void			setupNormalCorrectionMatrix(const CMatrix &normalMatrix);

	// @}


	/// \name Instance texturing.
	// @{

	/// Init the instance texturing with this manager. A texture space is reserved (if possible), and UVs are generated.
	void			initInstance(CLodCharacterInstance &instance);
	/// Release a lod instance. Free texture space.
	void			releaseInstance(CLodCharacterInstance &instance);

	/// reset the textureSpace. Instance must have been inited (nlassert). return false if no more texture space available
	bool					startTextureCompute(CLodCharacterInstance &instance);
	/// get a tmp bitmap for a special slot. caller can fill RGBA texture for the associated material Id in it.
	CLodCharacterTmpBitmap	&getTmpBitmap(uint8 id) {return _TmpBitmaps[id];}
	/// add a texture from an instance. Texture Lookup are made in _TmpBitmaps
	void					addTextureCompute(CLodCharacterInstance &instance, const CLodCharacterTexture &lodTexture);
	/// end and compile. reset/free memory of _TmpBitmaps up to numBmpToReset.
	void					endTextureCompute(CLodCharacterInstance &instance, uint numBmpToReset);

	// @}


	/// \name Misc
	// @{
	/** test if the CLod intersect the ray (0, K) (after mul vertices by toRaySpace)
	 *	\return false if not supported/no triangles, else true if can do the test (even if don't intersect!)
	 *	if intersect, dist2D=0, and distZ= Depth Distance
	 *	if don't intersect, dist2D="nearest distance to the ray", and distZ=0
	 *	\param computeDist2D if false and don't intersect, then return dist2D=FLT_MAX, and distZ=0
	 */
	bool	fastIntersect(const CLodCharacterInstance &instance, const NLMISC::CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D);
	// @}

// ******************************
private:
	/// Map name To Id.
	typedef	std::map<std::string, uint32>	TStrIdMap;
	typedef	TStrIdMap::iterator				ItStrIdMap;
	typedef	TStrIdMap::const_iterator		CstItStrIdMap;


private:

	/// Array of shapeBank
	std::vector<CLodCharacterShapeBank*>	_ShapeBankArray;

	/// Map of shape id
	TStrIdMap						_ShapeMap;


	/// \name render process
	// @{

	CVector							_ManagerMatrixPos;

	// The material.
	CMaterial						_Material;

	uint							_CurrentVertexId;
	uint							_MaxNumVertices;
	uint							_NumVBHard;
	CVertexStreamManager			_VertexStream;
	uint8							*_VertexData;
	uint							_VertexSize;
	bool							_Rendering;
	bool							_LockDone;

	// list of triangles
	uint							_CurrentTriId;
	CIndexBuffer					_Triangles;

	// The inverse of the normal correction matrix.
	CMatrix							_LightCorrectionMatrix;

	// @}


	/// \name Instance texturing.
	// @{

	// The Lod texture block. Can have 256 Lods in it.
	CSmartPtr<CTextureBlank>		_BigTexture;
	// Free texture space Ids;
	std::vector<uint>				_FreeIds;

	// The TMP Textures for build.
	CLodCharacterTmpBitmap			_TmpBitmaps[256];

	// return NULL if can't.
	CRGBA			*getTextureInstance(CLodCharacterInstance &instance);

	// @}

};


} // NL3D


#endif // NL_LOD_CHARACTER_MANAGER_H

/* End of lod_character_manager.h */
