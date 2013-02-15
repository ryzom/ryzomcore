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

#ifndef NL_ANIMATED_MATERIAL_H
#define NL_ANIMATED_MATERIAL_H


#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/animatable.h"
#include "nel/3d/material.h"
#include "nel/3d/track.h"
#include <map>


namespace NL3D
{



// ***************************************************************************
/**
 * An material Reference for Animated reference. This object is stored in the mesh, and is serialised.
 * NB: formated for 3ds Max :). Emissive anim is a float, with a constant RGB factor.
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CMaterialBase : public NLMISC::CRefCount
{
public:

	CMaterialBase();

	/** setup the default tracks from a material
	 * This method:
	 *	- copy the material contents into the Defaults tracks values.
	 *
	 * NB: for emissive part, emissive defaut track value is set to 1, and emissive factor is set to the
	 * RGB emissive value stored into pMat.
	 */
	void			copyFromMaterial(CMaterial *pMat);


	// Name of this material, for Animation access.
	std::string				Name;


	// Default tracks.
	CTrackDefaultRGBA		DefaultAmbient;
	CTrackDefaultRGBA		DefaultDiffuse;
	CTrackDefaultRGBA		DefaultSpecular;
	CTrackDefaultFloat		DefaultShininess;
	CTrackDefaultRGBA		DefaultEmissive;
	CTrackDefaultFloat		DefaultOpacity;
	CTrackDefaultInt		DefaultTexture;

	// Texture animation
	struct CTexAnimTracks
	{

		CTrackDefaultFloat DefaultUTrans; // u translation
		CTrackDefaultFloat DefaultVTrans; // v translation
		CTrackDefaultFloat DefaultUScale; // u scale
		CTrackDefaultFloat DefaultVScale; // u scale
		CTrackDefaultFloat DefaultWRot; // v scale


		// number of default tracks
		enum { NumTexAnimatedValues = 5 };


		void setDefaultValue()
		{
			DefaultUTrans.setDefaultValue(0);
			DefaultVTrans.setDefaultValue(0);
			DefaultUScale.setDefaultValue(1);
			DefaultVScale.setDefaultValue(1);
			DefaultWRot.setDefaultValue(0);
		}

		void			serial(NLMISC::IStream &f)
		{
			(void)f.serialVersion(0);
			f.serial(DefaultUTrans, DefaultVTrans, DefaultUScale, DefaultVScale);
		}
	};
	CTexAnimTracks			DefaultTexAnimTracks[IDRV_MAT_MAXTEXTURES];

	/// save/load.
	void			serial(NLMISC::IStream &f);


	/// \name Texture Animation mgt.
	/** Animated materials support Texture animation. This is the place where you define your list of texture.
	 * This list of animated texture is serialised. AnimatedMaterial animate texture with sint32 Tracks. If the id is not
	 * found in CMaterialBase, then the CMaterial texture is left as before.
	 *
	 * NB: id 0x7FFFFFFF is a reserved id, used as default to indicate no valid Animated texture.
	 */
	// @{
	/// assign a specific texture for an id (a uint32). It is valid to give a NULL ptr (=> untextured). Sorted as a SmartPtr.
	void			setAnimatedTexture(uint32 id, CSmartPtr<ITexture>  pText);
	/// is this Id valid?
	bool			validAnimatedTexture(uint32 id);
	/// return the texture for this Id. return NULL either if NULL texture for this id or if(!validAnimatedTexture()).
	ITexture*		getAnimatedTexture(uint32 id);

	// @}

// *********************
private:

	struct	CAnimatedTexture
	{
		CSmartPtr<ITexture>			Texture;

		// serial.
		void	serial(NLMISC::IStream &f);
	};

	typedef	std::map<uint32, CAnimatedTexture>	TAnimatedTextureMap;
	TAnimatedTextureMap			_AnimatedTextures;

};



// ***************************************************************************
/**
 * An animated material Instance of CMaterialBase
 * NB: formated for 3ds Max :). Emissive anim is a float, with a constant RGB factor.
 * Texture animation: see update().
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimatedMaterial : public IAnimatable
{
public:

	/// \name ctor / setup.
	// @{
	/** Constructor.
	 * This ctor:
	 *	- store a RefPtr on the BaseMaterial (for getDefaultTracks() method).
	 *	- copy the material default track value into Animated Values.
	 */
	CAnimatedMaterial(CMaterialBase *baseMat);


	/** setup the material context for this animated material.
	 * This method:
	 *	- store a RefPtr on the material, for future anim update.
	 */
	void			setMaterial(CMaterial *pMat);

	/** Return the name of this material (stored in CMaterialBase).
	 */
	std::string		getMaterialName() const;
	// @}


	/** Check if the animated material is touched, and if necessary update the stored material (if any).
	 * Texture animation: for now, texture animation is possible only on stage 0. If TextureValue flag is touched (ie
	 * a texture anim track is linked to the animated value), update() look into the CMaterialBase what texture to set.
	 * If the track gives a bad Id for the texture, no-op.
	 */
	void			update();


	/// \name Get some track name
	// @{
	static const char *getAmbientValueName() {return "ambient";}
	static const char *getDiffuseValueName() {return "diffuse";}
	static const char *getSpecularValueName() {return "specular";}
	static const char *getShininessValueName() {return "shininess";}
	static const char *getEmissiveValueName() {return "emissive";}
	static const char *getOpacityValueName() {return "opacity";}
	static const char *getTextureValueName() {return "texture";}
	//
	static const char *getTexMatUTransName(uint stage);
	static const char *getTexMatVTransName(uint stage);
	static const char *getTexMatUScaleName(uint stage);
	static const char *getTexMatVScaleName(uint stage);
	static const char *getTexMatWRotName(uint stage);

	// @}

	/// number of animated values for each animated texture, taken from CMaterialBase
	enum { NumTexAnimatedValues = CMaterialBase::CTexAnimTracks::NumTexAnimatedValues   };

	/// \name Herited from IAnimatable
	// @{
	/// Added values.
	enum	TAnimValues
	{
		OwnerBit= IAnimatable::AnimValueLast,
		AmbientValue,
		DiffuseValue,
		SpecularValue,
		ShininessValue,
		EmissiveValue,
		OpacityValue,
		TextureValue,
		TextureMatValues,
		AnimValueLast = TextureMatValues + NumTexAnimatedValues * IDRV_MAT_MAXTEXTURES /* texture matrix anim */
	};

	/// From IAnimatable
	virtual IAnimatedValue* getValue (uint valueId);

	/// From IAnimatable
	virtual const char *getValueName (uint valueId) const;

	/// From IAnimatable.
	virtual ITrack* getDefaultTrack (uint valueId);

	/// From IAnimatable.
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);

	// @}


// ********************
private:

	// The material instantiator.
	CRefPtr<CMaterialBase>	_MaterialBase;
	// The material.
	CRefPtr<CMaterial>		_Material;

	// AnimValues.
	CAnimatedValueRGBA		_Ambient;
	CAnimatedValueRGBA		_Diffuse;
	CAnimatedValueRGBA		_Specular;
	CAnimatedValueFloat		_Shininess;
	CAnimatedValueRGBA		_Emissive;
	CAnimatedValueFloat		_Opacity;
	CAnimatedValueInt		_Texture;

	struct CTexAnimatedMatValues
	{
		CAnimatedValueFloat _UTrans;
		CAnimatedValueFloat _VTrans;
		CAnimatedValueFloat _UScale;
		CAnimatedValueFloat _VScale;
		CAnimatedValueFloat _WRot;

		void affect(CMaterialBase::CTexAnimTracks &at)
		{
			_UTrans.Value= at.DefaultUTrans.getDefaultValue();
			_VTrans.Value= at.DefaultVTrans.getDefaultValue();
			_UScale.Value= at.DefaultUScale.getDefaultValue();
			_VScale.Value= at.DefaultVScale.getDefaultValue();
			_WRot.Value= at.DefaultWRot.getDefaultValue();
		}
	};
	CTexAnimatedMatValues		_TexAnimatedMatValues[IDRV_MAT_MAXTEXTURES];
};


} // NL3D


#endif // NL_ANIMATED_MATERIAL_H

/* End of animated_material.h */
