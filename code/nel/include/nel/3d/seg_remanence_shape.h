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


#ifndef NL_SEG_REMANENCE_SHAPE_H
#define NL_SEG_REMANENCE_SHAPE_H


#include "nel/misc/smart_ptr.h"
#include "nel/misc/vector.h"
#include "nel/misc/aabbox.h"


#include "nel/3d/texture.h"
#include "nel/3d/shape.h"
#include "nel/3d/material.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/animated_material.h"




// for debug only
//#define DEBUG_SEG_REMANENCE_DISPLAY


namespace NL3D
{

const NLMISC::CClassId SegRemanenceShapeId(0x4ef83d8d, 0x14d018f7);


class CVertexBuffer;

/** Shape for the remanence of a segment (sword stroke ..)
  *
  * NB this doesn't support material with animated texture matrix (texture matrix is used internally)
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CSegRemanenceShape : public IShape
{
public:
	//\name Object
	//@{
		/** default ctor : a seg from (0, 0, 0) to (0, 0, 1)
		  * A default bbox is set (from (-3, -3, -3) to (3, 3, 3))
		  */
		CSegRemanenceShape();
		//
		~CSegRemanenceShape();
		// copy ctor
		CSegRemanenceShape(const CSegRemanenceShape &other);
		// assignement
		CSegRemanenceShape &operator = (const CSegRemanenceShape &other);

		virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
		NLMISC_DECLARE_CLASS(CSegRemanenceShape);
	//@}

	/** Set material.
	  * A double sided material is forced
	  */
	void				setMaterial(const CMaterial &mat);
	/// Get material
	const CMaterial    &getMaterial() const { return _Mat; }

	/** Set the number of slices used to draw the remanence
	  * Should be >= 2
	  */
	void				setNumSlices(uint32 numSlices);
	// Get number of slices
	uint32				getNumSlices() const { return _NumSlices; }
	// Set the period of sampling of slices (in seconds)
	inline float		getSliceTime() const { return _SliceTime; }
	// Set the period of sampling of slices. This must be > 0
	void				setSliceTime(float sliceTime);
	/** Set the number of corners. By default it is 2 so you get a segment
	  * NB this invalidate previous calls to setCorner
	  * NB numCorners must be >= 2
	  */
	void				setNumCorners(uint numCorners);
	// get the number of corners
	uint32				getNumCorners() const { return (uint32)_Corners.size(); }
	// Set a corner
	void				setCorner(uint corner, const NLMISC::CVector &value);
	// Get a corner
	NLMISC::CVector		getCorner(uint corner) const;
	/** Set this shape bounding box
	  * NB: ideally the bbox should be computed per model, but practically, a fixed bbox suffice.
	  */
	void				setBBox(const NLMISC::CAABBox &bbox);

	/** Enable texture shifting on the u coordinates at the start of the animation, so that the texture will 'unroll' until it is completly visible
	  * NB this apply on all texture stages, and clamps the U coordinates.
	  * NB the default is on
	  */
	void				setTextureShifting(bool on = true);
	/// Test whether there is texture shifting at the start of the animation
	bool				getTextureShifting() const { return _TextureShifting; }

	/// from IShape
	virtual	void		getAABBox(NLMISC::CAABBox &bbox) const { bbox = _BBox.getAABBox(); }

	/// Force animated material. This can be called only once
	void				setAnimatedMaterial(const std::string &name);
	/// Get animated material or NULL if none
	CMaterialBase	   *getAnimatedMaterial() const { return _AnimatedMat; }
	/** When stop() is called, the fx doesn't disappear, it must rollup (a texture shifting is performed).
	  * This set the speed of rollup.
	  * 1 means it takes   sliceTime * numSlice to unroll
	  * 0.5 means it takes  2 * sliceTime * numSlice to unroll
	  * and so on
	  * NB ratio must be > 0
	  */
	void				setRollupRatio(float ratio);
	float				getRollupRatio() const { return _RollUpRatio; }

	// from IShape
	virtual bool		clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix);

	/// \name access default tracks.
	// @{
		CTrackDefaultVector*	getDefaultPos()		{return &_DefaultPos;}
		CTrackDefaultQuat*		getDefaultRotQuat()	{return &_DefaultRotQuat;}
		CTrackDefaultVector*	getDefaultScale()		{return &_DefaultScale;}
	// @}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

protected:
	///\name from IShape
	//@{
		virtual void				render(IDriver *drv, CTransformShape *trans, bool opaquePass);
		virtual void				flushTextures (IDriver &driver, uint selectedTexture);
		virtual	CTransformShape		*createInstance(CScene &scene);
		virtual float				getNumTriangles (float distance);
	//@}
private:
	typedef std::vector<CVector>	TCornerVect;
private:
	bool						_GeomTouched;
	bool						_MatTouched;
	bool						_TextureShifting;
	//
	uint32						_NumSlices;
	float						_SliceTime;
	float						_RollUpRatio;
	TCornerVect					_Corners;	// start of segment
	//
	CMaterial					_Mat;
	/// For clipping.
	NLMISC::CAABBoxExt			_BBox;
	CMaterialBase				*_AnimatedMat;
	// Default tracks
	CTrackDefaultVector			_DefaultPos;
	CTrackDefaultQuat			_DefaultRotQuat;
	CTrackDefaultVector			_DefaultScale;
private:
	void    setupMaterial();
	void	copyFromOther(const CSegRemanenceShape &other);
};





}

#endif





