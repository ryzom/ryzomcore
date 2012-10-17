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

#ifndef NL_WATER_MODEL_H
#define NL_WATER_MODEL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/polygon.h"
#include "nel/3d/u_water.h"
//
#include "nel/3d/transform_shape.h"
#include "nel/3d/material.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/texture_emboss.h"
#include "nel/3d/driver.h"


namespace MISC
{
	class CVector;
}

namespace NL3D {


class CWaterPoolManager;
class CWaterShape;
class IDriver;
class CVertexBufferReadWrite;

/**
 * A water surface
 *
 * In order to get precise reflections, we tesselate the shape by projecting it on screen, and by subdividing it by a fixed size grid.
 * For each grid cell :
 * - if it is entirely inside the projected shape, it is displayed as it
 * - if is outside,  no-op
 * - when it intersect the projected shape border, we generate some triangles by clipping the grid cell against the projected shape
 *
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CWaterModel : public CTransformShape
{
public:
	/// ctor
	CWaterModel();

	// dtor
	~CWaterModel();

	// to call the first time after the shape & the matrix  has been set
	void init()
	{
		updateDiffuseMapMatrix(true);
	}

	// register this model
	static void registerBasic();
	static CTransform *creator() { return new CWaterModel; }

	// get default tracks
	virtual ITrack* getDefaultTrack (uint valueId);

	/// inherited from UWaterInstance
	virtual uint32	getWaterHeightMapID() const;

	/// inherited from UWaterInstance
	virtual float	getHeightFactor() const;

	/// inherited from UWaterInstance
	virtual float   getHeight(const NLMISC::CVector2f &pos);

	/// inherited from UWaterInstance
	virtual float   getAttenuatedHeight(const NLMISC::CVector2f &pos, const NLMISC::CVector &viewer);

	/// \name CTransform traverse specialisation
	// @{
	virtual void	traverseRender();
	virtual	bool	clip();
	// @}

	// get num wanted vertices for current frame (& precache clipped triangles)
	uint getNumWantedVertices();

	// fill vertex buffer with this shape datas, and returns pointer to next free location
	uint fillVB(void *dataStart, uint startTri, IDriver &drv);

	// setup vertex buffer before render
	static void setupVertexBuffer(CVertexBuffer &vb, uint numWantedVertices, IDriver *drv);

	// For Debug purpose
	void	debugDumpMem(void* &clippedPolyBegin, void* &clippedPolyEnd);
	void	debugClearClippedPoly();

protected:
	friend class CWaterShape;
	void setupMaterialNVertexShader(IDriver *drv, CWaterShape *shape, const NLMISC::CVector &obsPos, bool above, float zHeight);
	//
	void setupSimpleRender(CWaterShape *shape, const NLMISC::CVector &obsPos, bool above);
	// compute the clipped poly for cards that have vertex shaders
	void computeClippedPoly();
	// simple rendering version
	//void doSimpleRender(IDriver *drv);
private:
	static NLMISC::CRefPtr<IDriver> _CurrDrv;
	CSmartPtr<CTextureEmboss> _EmbossTexture;
	// Matrix to compute uv of diffuse map
	NLMISC::CVector2f		  _ColorMapMatColumn0, _ColorMapMatColumn1, _ColorMapMatPos;
	uint64					  _MatrixUpdateDate;
	// vertex buffer for simple rendering
	static CMaterial		  _WaterMat;
	static CMaterial		  _SimpleWaterMat;
	// grid cells that are exactly inside the poly
	NLMISC::CPolygon2D::TRasterVect	 _Inside;
	sint							 _MinYInside;
	// water surface clipped by frustum
	NLMISC::CPolygon		   _ClippedPoly;
	// link into list of water model to display
public:
	CWaterModel **_Prev;
	CWaterModel *_Next;
private:
	// clipped tris of the shape after it has been projected on grid, all packed in a single vector
	std::vector<NLMISC::CVector2f> _ClippedTris;
	// for each clipped tri, gives it number of vertices
	std::vector<uint>		   _ClippedTriNumVerts;
	// vertex range into global vb for current render
	uint32					   _StartTri;
	uint32                     _NumTris;
public:
	// for use by CScene
	void unlink();
	void link();
private:
	void updateDiffuseMapMatrix(bool force = false);
	uint fillVBHard(void *dataStart, uint startIndex);
	uint fillVBSoft(void *dataStart, uint startIndex);
};

//=====================================================================================================================

/// This model can create wave where it is located. It has no display...
class CWaveMakerModel : public CTransformShape
{
	public:

	CWaveMakerModel();

	// register this model
	static void		registerBasic();

	static CTransform *creator() { return new CWaveMakerModel; }

	// get default tracks
	virtual ITrack* getDefaultTrack (uint valueId);

	/// \name CTransform traverse specialisation
	// @{
	/** this do :
	 *  - call CTransformShape::traverseAnimDetail()
	 *  - perform perturbation
	 */
	virtual void	traverseAnimDetail();
	// @}

protected:

	friend class	CWaveMakerShape;
	TAnimationTime  _Time;
};

// tmp for debug
extern uint8 *waterVBEnd;

} // NL3D


#endif // NL_WATER_MODEL_H

/* End of water_model.h */
