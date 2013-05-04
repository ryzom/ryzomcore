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

#ifndef NL_FLARE_SHAPE_H
#define NL_FLARE_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/shape.h"
#include "nel/3d/texture.h"
#include "nel/3d/track.h"



namespace NL3D {


// class id for flares
const NLMISC::CClassId FlareModelClassId =  NLMISC::CClassId(0x6d674c32, 0x53b961a0);

// max number of flares
const uint MaxFlareNum= 10;

class CMesh;
class CShapeBank;

/**
 * shape for a flare
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CFlareShape : public IShape
{
public:
	NLMISC_DECLARE_CLASS(CFlareShape);

	///\name Object
		//@{
		/// Constructor
		CFlareShape();

		/// serial this shape
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
		//@}


	/// inherited from IShape
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// inherited from IShape
	virtual bool				clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix);


	/// inherited from IShape. Does nothing. A new traverseRender() was set for that
	virtual void				render(IDriver * /* drv */, CTransformShape * /* trans */, bool /* opaquePass */) {}

	/// inherited from IShape
	virtual	void				getAABBox(NLMISC::CAABBox &bbox) const;

	/// inherited from ishape
	virtual float				getNumTriangles (float distance);

	/// inherited from ishape
	virtual void				flushTextures (IDriver &driver, uint selectedTexture);

	/** set a texture for the flare
	  * \param index the index of the flare to set. Vaklue ranges from 0 to MaxFlareNum - 1
	  * \param tex the texture to set. NULL removes the texture
	  */
	void						setTexture(uint index, ITexture *tex)
	{
		nlassert(index < MaxFlareNum);
		_Tex[index] = tex;
		if (tex)
		{
			// clamp borders
			_Tex[index]->setWrapS(ITexture::Clamp);
			_Tex[index]->setWrapT(ITexture::Clamp);
		}
	}

	/** get the nth texture used by the flare.
	  *  \param index the index of the flare to set. Value ranges from 0 to MaxFlareNum - 1
	  */
	ITexture					*getTexture(uint index)
	{
		nlassert(index < MaxFlareNum);
		return _Tex[index];
	}

	/// get the texture used by the flare (const version)
	const ITexture				*getTexture(uint index) const
	{
		nlassert(index < MaxFlareNum);
		return _Tex[index];
	}

	/** set the size of the nth flare flare
	  * \param index the index of the flare to set. Value ranges from 0 to MaxFlareNum - 1
	  */
	void						setSize(uint index, float size)
	{
		nlassert(index < MaxFlareNum);
		_Size[index]  = size;
	}

	/** get the size of the nth flare
	  * \param index the index of the flare to set. Value ranges from 0 to MaxFlareNum - 1
	  */
	float						getSize(uint index) const
	{
		return _Size[index];
	}

	/** set the relative position of the nth flares. The default goes linearly from 0 (which appear at the position of the flare)
	  * to 1 (which appears at the center of the screen when the flare spaving is set to 1
	  * \see setFlareSpacing()
	  */
	void						setRelativePos(uint index, float pos)
	{
		nlassert(index < MaxFlareNum);
		_Pos[index] = pos;
	}

	/// get the relative pos of the nth flare
	float						getRelativePos(uint index) const
	{
		nlassert(index < MaxFlareNum);
		return _Pos[index];
	}

	/// set the color of flares
	void						setColor(NLMISC::CRGBA col)
	{
		_Color = col;
	}

	/// get the color of flares
	NLMISC::CRGBA				getColor(void) const
	{
		return _Color;
	}

	/// set the flares spacing
	void						setFlareSpacing(float spacing)
	{
		_Spacing = spacing;
	}

	/** Get the flares spacing : A spacing of 1.f means thta the last flare will reach the center of the screen
	  * , a spacing of 0.5f means only the half way to the middle of the screen will be reached
	  */
	float						getFlareSpacing(void) const
	{
		return _Spacing;
	}

	/// set the persistence of this shape, in second (the time it takes to fade from white to black)
	void						setPersistence(TAnimationTime persistence)
	{
		_Persistence = persistence;
	}

	/** get the persistence of this shape
	  * \see setPersistence
	  */
	TAnimationTime				getPersistence(void) const
	{
		return _Persistence;
	}


	/// force radial attenuation of the flares
	void						setAttenuable(bool enable = true)	{ _Attenuable = enable; }

	/// check whether radial :attenuation is on
	bool						getAttenuable(void) const			{ return _Attenuable;   }

	/// set the range for attenuation
	void						setAttenuationRange(float range)    { _AttenuationRange = range; }

	/// get the attenuation range
	float						getAttenuationRange(void) const		{ return _AttenuationRange; }


	/// force the first flare to keep its real size (e.g the isze on screen doesn't remains constant)
	void						setFirstFlareKeepSize(bool enable = true) { _FirstFlareKeepSize = enable; }

	/// test whether the first flare keep its real size
	bool						getFirstFlareKeepSize(void) const		  { return _FirstFlareKeepSize; }

	/// enable dazzle when the flare is near the center of the screen
	void						enableDazzle(bool enable = true)		  { _DazzleEnabled = enable; }

	/// check whether dazzle is enabled
	bool						hasDazzle(void) const	{  return _DazzleEnabled; }

	/** set Dazzle color
	  * \see enableDazzle()
	  */
	void						setDazzleColor(NLMISC::CRGBA col) { _DazzleColor = col; }

	/** get Dazzle color
	  * \see enableDazzle()
	  */
	NLMISC::CRGBA				getDazzleColor(void) const { return _DazzleColor; }

	/** Set Dazzle attenuation range. It is the same than with attenuationRange. 1 mean that the dazzle stops when the flare
	  *  is at the border of screen. 0.5, for the half way between center and border etc .
	  *  \see enableDazzle()
	  */
	void						setDazzleAttenuationRange(float range) { _DazzleAttenuationRange = range; }

	/// get the attenuation range of Dazzle
	float						getDazzleAttenuationRange(void) const { return _DazzleAttenuationRange; }

	/** set the maxViewDist for the flares
      * The default is 1000
	  */
	void						setMaxViewDist(float dist) { _MaxViewDist = dist; }


	/// get the max view dist
	float						getMaxViewDist(void) const { return _MaxViewDist; }

	/** set a distance ratio. when dist / maxViewDist is above this ratio, the flares will start to fade
	  * The default is 0.9
	  */
	void						setMaxViewDistRatio(float ratio) { _MaxViewDistRatio = ratio; }

	/// get the max view dist ratio
	float						getMaxViewDistRatio(void) const  { return  _MaxViewDistRatio; }

	/** The flare is considered to be at the infinite. This mean that it is always drawn
	  * And that there's no attenuation with dist. The real flare must be created far from the camera
	  * for this to work. The defualt is false
	  */
	void						setFlareAtInfiniteDist(bool enabled = true) { _InfiniteDist = enabled; }

	/// test whether the flare is at the infinite
	bool						getFlareAtInfiniteDist(void) const			{ return _InfiniteDist; }

	/// Transform default tracks.
	CTrackDefaultVector			_DefaultPos;


	/// \name access default tracks.
	// @{
	CTrackDefaultVector*	getDefaultPos ()		{return &_DefaultPos;}
	// @}

	/** Set the name of the mesh that is used to test the visible surface
	  * If the value is empty, then a simple point is used
	  * The mesh materials are ignored
	  */
	void					setOcclusionTestMeshName(const std::string &shapeName);
	const std::string	   &getOcclusionTestMeshName() const { return _OcclusionTestMeshName; }
	/** Return the mesh that is used to perform the occlusion test.
	  * \return pointer to the mesh, or NULL if not used or not found
	  */
	CMesh				   *getOcclusionTestMesh(CShapeBank &sb);
	// Tell whether the occlusion mesh inherit the rotation/scale part of the model matrix
	void					setOcclusionTestMeshInheritScaleRot(bool on) { _OcclusionTestMeshInheritScaleRot = on; }
	bool					getOcclusionTestMeshInheritScaleRot() const { return _OcclusionTestMeshInheritScaleRot; }

	void    setScaleWhenDisappear(bool enable) { _ScaleWhenDisappear = enable; }
	bool	getScaleWhenDisappear() const { return _ScaleWhenDisappear; }
	void	setSizeDisappear(float size) { _SizeDisappear = size; }
	void	setAngleDisappear(float angle) { _AngleDisappear = angle; }
	float	getSizeDisappear() const { return _SizeDisappear; }
	float	getAngleDisappear() const { return _AngleDisappear; }

	// decide whether first flare is displayed using lookat mode
	void	setLookAtMode(bool on) { _LookAtMode = on; }
	bool	getLookAtMode() const {	return _LookAtMode; }
protected:
	friend class CFlareModel;
	NLMISC::CSmartPtr<ITexture> _Tex[MaxFlareNum];
	NLMISC::CRGBA				_Color;
	NLMISC::CRGBA				_DazzleColor;
	float						_Size[MaxFlareNum];
	float						_SizeDisappear;
	bool						_ScaleWhenDisappear;
	float                       _AngleDisappear;
	float						_Pos[MaxFlareNum];
	TAnimationTime				_Persistence;
	float						_Spacing;
	bool					    _Attenuable;
	float					    _AttenuationRange;
	bool						_FirstFlareKeepSize;
	bool						_DazzleEnabled;
	float						_DazzleAttenuationRange;
	float						_MaxViewDist;
	float						_MaxViewDistRatio;
	bool						_InfiniteDist;
	std::string					_OcclusionTestMeshName;
	NLMISC::CSmartPtr<CMesh>	_OcclusionTestMesh;
	bool						_OcclusionMeshNotFound;
	bool						_OcclusionTestMeshInheritScaleRot;
	bool						_LookAtMode;
};


} // NL3D


#endif // NL_FLARE_SHAPE_H

/* End of flare_shape.h */
