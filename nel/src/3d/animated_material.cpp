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

#include "std3d.h"

#include "nel/3d/animated_material.h"
#include "nel/misc/common.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void	CMaterialBase::CAnimatedTexture::serial(NLMISC::IStream &f)
{
	ITexture	*text= NULL;
	if(f.isReading())
	{
		f.serialPolyPtr(text);
		Texture= text;
	}
	else
	{
		text= Texture;
		f.serialPolyPtr(text);
	}
}



// ***************************************************************************
CMaterialBase::CMaterialBase()
{
	DefaultAmbient.setDefaultValue(CRGBA(64,64,64));
	DefaultDiffuse.setDefaultValue(CRGBA(128,128,128));
	DefaultSpecular.setDefaultValue(CRGBA(0,0,0));
	DefaultShininess.setDefaultValue(10);
	DefaultEmissive.setDefaultValue(CRGBA(128,128,128));
	DefaultOpacity.setDefaultValue(1);
	DefaultTexture.setDefaultValue(0x7FFFFFFF);
	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		DefaultTexAnimTracks[k].setDefaultValue();
	}
}


// ***************************************************************************
void	CMaterialBase::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(1);

	f.serial(Name);
	f.serial(DefaultAmbient, DefaultDiffuse, DefaultSpecular);
	f.serial(DefaultShininess, DefaultEmissive, DefaultOpacity, DefaultTexture);
	f.serialCont(_AnimatedTextures);
	if (ver > 0)
	{
		for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
		{
			f.serial(DefaultTexAnimTracks[k]);
		}
	}
}


// ***************************************************************************
void	CMaterialBase::copyFromMaterial(CMaterial *pMat)
{
	DefaultAmbient.setDefaultValue(pMat->getAmbient());
	DefaultDiffuse.setDefaultValue(pMat->getDiffuse());
	DefaultSpecular.setDefaultValue(pMat->getSpecular());
	DefaultShininess.setDefaultValue(pMat->getShininess());
	DefaultEmissive.setDefaultValue(pMat->getEmissive());
	DefaultOpacity.setDefaultValue(pMat->getDiffuse().A/255.f);

	/// get uv value from material
	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		if (pMat->isUserTexMatEnabled(k))
		{
			float uTrans, vTrans, uScale, vScale, wRot;
			pMat->decompUserTexMat(k, uTrans, vTrans, wRot, uScale, vScale);
			DefaultTexAnimTracks[k].DefaultUTrans.setDefaultValue(uTrans);
			DefaultTexAnimTracks[k].DefaultVTrans.setDefaultValue(vTrans);
			DefaultTexAnimTracks[k].DefaultUScale.setDefaultValue(uScale);
			DefaultTexAnimTracks[k].DefaultVScale.setDefaultValue(vScale);
			DefaultTexAnimTracks[k].DefaultWRot.setDefaultValue(wRot);
		}
	}
}


// ***************************************************************************
void			CMaterialBase::setAnimatedTexture(uint32 id, CSmartPtr<ITexture>  pText)
{
	// add or replace the texture.
	_AnimatedTextures[id].Texture= pText;
}
// ***************************************************************************
bool			CMaterialBase::validAnimatedTexture(uint32 id)
{
	TAnimatedTextureMap::iterator	it;
	it= _AnimatedTextures.find(id);
	return it!=_AnimatedTextures.end();
}
// ***************************************************************************
ITexture*		CMaterialBase::getAnimatedTexture(uint32 id)
{
	TAnimatedTextureMap::iterator	it;
	it= _AnimatedTextures.find(id);
	if( it!=_AnimatedTextures.end() )
		return it->second.Texture;
	else
		return NULL;
}




// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CAnimatedMaterial::CAnimatedMaterial(CMaterialBase *baseMat)
{
	nlassert(baseMat);
	_MaterialBase= baseMat;

	// IAnimatable.
	IAnimatable::resize(AnimValueLast);

	_Ambient.Value= _MaterialBase->DefaultAmbient.getDefaultValue();
	_Diffuse.Value= _MaterialBase->DefaultDiffuse.getDefaultValue();
	_Specular.Value= _MaterialBase->DefaultSpecular.getDefaultValue();
	_Shininess.Value= _MaterialBase->DefaultShininess.getDefaultValue();
	_Emissive.Value= _MaterialBase->DefaultEmissive.getDefaultValue();
	_Opacity.Value= _MaterialBase->DefaultOpacity.getDefaultValue();

	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		_TexAnimatedMatValues[k].affect(baseMat->DefaultTexAnimTracks[k]);
	}
}


// ***************************************************************************
void	CAnimatedMaterial::setMaterial(CMaterial *pMat)
{
	_Material= pMat;
}


// ***************************************************************************
std::string		CAnimatedMaterial::getMaterialName() const
{
	nlassert(_MaterialBase);
	return _MaterialBase->Name;
}


// ***************************************************************************
void	CAnimatedMaterial::update()
{
	if(isTouched(OwnerBit) && _Material!=NULL /*&& _Material->isLighted()*/)
	{

		// well, just update all...  :)

		// diffuse part.
		CRGBA	diff= _Diffuse.Value;
		sint c= (sint)(_Opacity.Value*255);
		clamp(c, 0, 255);
		diff.A= c;

		// setup material.
		if (_Material->isLighted())
		{
			_Material->setLighting(true, _Emissive.Value, _Ambient.Value, diff, _Specular.Value, _Shininess.Value);
		}
		else
		{
			_Material->setColor(diff);
		}

		// clear flags.
		clearFlag(AmbientValue);
		clearFlag(DiffuseValue);
		clearFlag(SpecularValue);
		clearFlag(ShininessValue);
		clearFlag(EmissiveValue);
		clearFlag(OpacityValue);


		// Texture Anim.
		if(isTouched(TextureValue))
		{
			nlassert(_MaterialBase);

			uint32	id= _Texture.Value;
			if(_MaterialBase->validAnimatedTexture(id))
			{
				_Material->setTexture(0, _MaterialBase->getAnimatedTexture(id) );
			}
			clearFlag(TextureValue);
		}

		// Get texture matrix from animated value to setup the material
		for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
		{
			if (_Material->isUserTexMatEnabled(k))
			{
				const CTexAnimatedMatValues &texMatAV = _TexAnimatedMatValues[k];

				CMatrix convMat; // exported v are already inverted (todo : optim this if needed, this matrix shouldn't be necessary)
				convMat.setRot(CVector::I, -CVector::J, CVector::K);
				convMat.setPos(CVector::J);
				float fCos = cosf(texMatAV._WRot.Value);
				float fSin = sinf(texMatAV._WRot.Value);
				CMatrix SR;
				SR.setRot(CVector(texMatAV._UScale.Value * fCos, texMatAV._VScale.Value * fSin, 0.f),
						  CVector(- texMatAV._UScale.Value * fSin, texMatAV._VScale.Value * fCos, 0.f),
						  CVector::K);
				CVector half(0.5f, 0.5f, 0.f);
				SR.setPos(SR.mulVector(- half - CVector(texMatAV._UTrans.Value, texMatAV._VTrans.Value, 0.f)) + half);
				_Material->setUserTexMat(k, convMat * SR * convMat);
			}
		}

		// We are OK!
		IAnimatable::clearFlag(OwnerBit);
	}
}


// ***************************************************************************
IAnimatedValue* CAnimatedMaterial::getValue (uint valueId)
{
	switch(valueId)
	{
	case AmbientValue: return &_Ambient;
	case DiffuseValue: return &_Diffuse;
	case SpecularValue: return &_Specular;
	case ShininessValue: return &_Shininess;
	case EmissiveValue: return &_Emissive;
	case OpacityValue: return &_Opacity;
	case TextureValue: return &_Texture;
	default: // this may be a texture animated value...
		if (valueId >= TextureMatValues && valueId < AnimValueLast)
		{
			const uint baseId = valueId - TextureMatValues;
			const uint texNum =   baseId / NumTexAnimatedValues; // stage index
			const uint argID  =   baseId % NumTexAnimatedValues; // value for this stage
			switch(argID)
			{
				case 0:	return &_TexAnimatedMatValues[texNum]._UTrans;
				case 1:	return &_TexAnimatedMatValues[texNum]._VTrans;
				case 2:	return &_TexAnimatedMatValues[texNum]._UScale;
				case 3:	return &_TexAnimatedMatValues[texNum]._VScale;
				case 4:	return &_TexAnimatedMatValues[texNum]._WRot;
			}
		}
	break;
	};

	// shoudl not be here!!
	nlstop;
	return NULL;
}
// ***************************************************************************
const char *CAnimatedMaterial::getValueName (uint valueId) const
{
	switch(valueId)
	{
	case AmbientValue: return getAmbientValueName();
	case DiffuseValue: return getDiffuseValueName();
	case SpecularValue: return getSpecularValueName();
	case ShininessValue: return getShininessValueName();
	case EmissiveValue: return getEmissiveValueName();
	case OpacityValue: return getOpacityValueName();
	case TextureValue: return getTextureValueName();
	default: // this may be a texture animated value...
		if (valueId >= TextureMatValues && valueId < AnimValueLast)
		{
			const uint baseId = valueId - TextureMatValues;
			const uint texNum =   baseId / NumTexAnimatedValues;
			const uint argID  =   baseId % NumTexAnimatedValues;
			switch(argID)
			{
				case 0:	return getTexMatUTransName      (texNum);
				case 1:	return getTexMatVTransName(texNum);
				case 2:	return getTexMatUScaleName(texNum);
				case 3:	return getTexMatVScaleName(texNum);
				case 4:	return getTexMatWRotName(texNum);
			}
		}
	break;
	};

	// shoudl not be here!!
	nlstop;
	return "";
}
// ***************************************************************************
ITrack*	CAnimatedMaterial::getDefaultTrack (uint valueId)
{
	nlassert(_MaterialBase);

	switch(valueId)
	{
	case AmbientValue: return	&_MaterialBase->DefaultAmbient;
	case DiffuseValue: return	&_MaterialBase->DefaultDiffuse;
	case SpecularValue: return	&_MaterialBase->DefaultSpecular;
	case ShininessValue: return	&_MaterialBase->DefaultShininess;
	case EmissiveValue: return	&_MaterialBase->DefaultEmissive;
	case OpacityValue: return	&_MaterialBase->DefaultOpacity;
	case TextureValue: return	&_MaterialBase->DefaultTexture;
	default: // this may be a texture animated value...
		if (valueId >= TextureMatValues && valueId < AnimValueLast)
		{
			const uint baseId = valueId - TextureMatValues;
			const uint texNum =   baseId / NumTexAnimatedValues;
			const uint argID  =   baseId % NumTexAnimatedValues;
			switch(argID)
			{
				case 0:	return 	&_MaterialBase->DefaultTexAnimTracks[texNum].DefaultUTrans;
				case 1:	return 	&_MaterialBase->DefaultTexAnimTracks[texNum].DefaultVTrans;
				case 2:	return 	&_MaterialBase->DefaultTexAnimTracks[texNum].DefaultUTrans;
				case 3:	return 	&_MaterialBase->DefaultTexAnimTracks[texNum].DefaultVTrans;
				case 4:	return 	&_MaterialBase->DefaultTexAnimTracks[texNum].DefaultWRot;
			}
		}
	break;
	};

	// shoudl not be here!!
	nlstop;
	return NULL;
}
// ***************************************************************************
void	CAnimatedMaterial::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	// For CAnimatedMaterial, channels are detailled (material rendered after clip)!
	addValue(chanMixer, AmbientValue, OwnerBit, prefix, true);
	addValue(chanMixer, DiffuseValue, OwnerBit, prefix, true);
	addValue(chanMixer, SpecularValue, OwnerBit, prefix, true);
	addValue(chanMixer, ShininessValue, OwnerBit, prefix, true);
	addValue(chanMixer, EmissiveValue, OwnerBit, prefix, true);
	addValue(chanMixer, OpacityValue, OwnerBit, prefix, true);
	addValue(chanMixer, TextureValue, OwnerBit, prefix, true);
	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		for (uint l = 0; l < NumTexAnimatedValues; ++l)
		{
			addValue(chanMixer, TextureMatValues + l + k * NumTexAnimatedValues, OwnerBit, prefix, true);
		}
	}
}

// ***************************************************************************
const char *CAnimatedMaterial::getTexMatUTransName(uint stage)
{
	static char names[IDRV_MAT_MAXTEXTURES][16];
	static bool init = false;
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	if (!init) // where name initialized ?
	{
		for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
		{
			sprintf(&names[k][0], "UTrans%d", k);
		}
		init = true;
	}
	return names[stage];
}

// ***************************************************************************
const char *CAnimatedMaterial::getTexMatVTransName(uint stage)
{
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	static char names[IDRV_MAT_MAXTEXTURES][16];
	static bool init = false;
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	if (!init) // where name initialized ?
	{
		for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
		{
			sprintf(&names[k][0], "VTrans%d", k);
		}
		init = true;
	}
	return names[stage];
}


// ***************************************************************************
const char *CAnimatedMaterial::getTexMatUScaleName(uint stage)
{
	static char names[IDRV_MAT_MAXTEXTURES][16];
	static bool init = false;
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	if (!init) // where name initialized ?
	{
		for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
		{
			sprintf(&names[k][0], "UScale%d", k);
		}
		init = true;
	}
	return names[stage];
}

// ***************************************************************************
const char *CAnimatedMaterial::getTexMatVScaleName(uint stage)
{
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	static char names[IDRV_MAT_MAXTEXTURES][16];
	static bool init = false;
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	if (!init) // where name initialized ?
	{
		for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
		{
			sprintf(&names[k][0], "VScale%d", k);
		}
		init = true;
	}
	return names[stage];
}


// ***************************************************************************
const char *CAnimatedMaterial::getTexMatWRotName(uint stage)
{
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	static char names[IDRV_MAT_MAXTEXTURES][16];
	static bool init = false;
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	if (!init) // where name initialized ?
	{
		for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
		{
			sprintf(&names[k][0], "WRot%d", k);
		}
		init = true;
	}
	return names[stage];
}



} // NL3D
