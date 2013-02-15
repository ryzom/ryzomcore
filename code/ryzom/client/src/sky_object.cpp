// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
//
#include "nel/3d/u_instance_material.h"
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_instance.h"
//
#include "nel/misc/path.h"
#include "nel/misc/file.h"
//
#include "sky_object.h"
#include "sky.h"

using namespace NLMISC;
using namespace NL3D;

////////////////////////////
// CSkyObject::CColorInfo //
////////////////////////////

// *************************************************************************************************
void CSkyObject::CColorInfo::init(const CSkyObjectSheet::CColorInfoSheet &ci,
								  std::map<std::string, CBitmap *> &bitmapByName,
								  std::vector<CBitmap *> &builtBitmaps)
{
	Mode = ci.Mode;
	bool alreadyBuilt;
	Map = buildSharedBitmap(ci.MapName, bitmapByName, builtBitmaps, alreadyBuilt);
}

// *************************************************************************************************
CRGBA CSkyObject::CColorInfo::computeColor(float dayPart, float weatherLevel, CRGBA fogColor)
{
	switch(Mode)
	{
		case Unused:		return CRGBA(0, 0, 0, 0);
		case FogColor:		return fogColor;
		case BitmapColor:
			clamp(dayPart, 0.f, 1.f);
			clamp(weatherLevel, 0.f, 1.f);
			if (!Map) return CRGBA(0, 0, 0, 0);
			return Map->getColor(dayPart, weatherLevel, true, false);
		case BitmapColorModulatedByFogColor:
		{
			clamp(dayPart, 0.f, 1.f);
			clamp(weatherLevel, 0.f, 1.f);
			if (!Map) return CRGBA(0, 0, 0, 0);
			CRGBA result = Map->getColor(dayPart, weatherLevel, true, false);
			result.modulateFromColor(result, fogColor);
			return result;
		}
		break;
		default:
			nlassert(0); // unknwon type
		break;
	}
	return CRGBA(0, 0, 0, 0);
}


// *************************************************************************************************
////////////////////////////////////
// CSkyObject::CColorGradientInfo //
////////////////////////////////////

void CSkyObject::CColorGradientInfo::init(const CSkyObjectSheet::CColorGradientInfoSheet &cgis,
							  std::map<std::string,CBitmap *> &bitmapByName,
							  std::vector<CBitmap *> &builtBitmaps)
{
	TargetTextureStage = cgis.TargetTextureStage;
	WeatherToGradient.resize(cgis.WeatherToGradient.size());
	CBitmap *lastSeenBitmap = NULL;
	for(uint k = 0; k < cgis.WeatherToGradient.size(); ++k)
	{
		bool alreadyBuilt;
		WeatherToGradient[k] = buildSharedBitmap(cgis.WeatherToGradient[k], bitmapByName, builtBitmaps, alreadyBuilt);
		if (WeatherToGradient[k])
		{
			if (!WeatherToGradient[k]->convertToType(CBitmap::RGBA))
			{
				// can't use bitmap..
				WeatherToGradient[k] = NULL; // don't do a delete here because it'is 'builtBitmaps' that has ownership
			}
			else
			{
				if (!alreadyBuilt)
				{
					// rotate the bitmap because it is faster to blit a row than a column
					WeatherToGradient[k]->rot90CCW();
					WeatherToGradient[k]->flipV();
				}
				if (lastSeenBitmap)
				{
					if (WeatherToGradient[k]->getWidth() != lastSeenBitmap->getWidth() ||
						WeatherToGradient[k]->getHeight() != lastSeenBitmap->getHeight()
					   )
					{
						nlwarning("All bitmaps must have the same size in the gradient");
					}
				}
				lastSeenBitmap = WeatherToGradient[k];
			}
		}
	}
}

/*
static void dumpGrad(CBitmap &bm)
{
	CRGBA *pixs =  (CRGBA *) &bm.getPixels(0)[0];
	for(uint k = 0; k < bm.getWidth(); ++k)
	{
		nlinfo("(r, g, b, a) = (%d, %d, %d, %d)", pixs[k].R, pixs[k].G, pixs[k].B, pixs[k].A);
	}
}
*/


// *************************************************************************************************
void CSkyObject::CColorGradientInfo::setup(NL3D::UInstance instance, float dayPart, float weatherLevel, CBitmap &gradientCache, CBitmap &gradientCacheBlurred)
{
	if (instance.empty()) return;
	if (WeatherToGradient.empty()) return;
	clamp(dayPart, 0.f, 1.f);
	clamp(weatherLevel, 0.f, 1.f);
	// takes 2 closest bitmaps to do the blend
	uint bm0Index = std::min(uint(weatherLevel * WeatherToGradient.size()), uint(WeatherToGradient.size() - 1));
	uint bm1Index = std::min(uint(weatherLevel * WeatherToGradient.size() + 1), uint(WeatherToGradient.size() - 1));
	CBitmap *bm0 = WeatherToGradient[bm0Index];
	CBitmap *bm1 = WeatherToGradient[bm1Index];
	if (!bm1 && !bm0) return;
	if (!bm1) bm1 = bm0;
	if (!bm0) bm0 = bm1;
	// make sure that both bitmap have the same size
	if (bm0->getWidth() != bm1->getWidth() || bm0->getHeight() != bm1->getHeight()) return;
	// extract the 2 slices before to blend
	uint slice0 = (uint) (dayPart * bm0->getHeight());
	uint slice1 = (uint) (dayPart * bm0->getHeight() + 1) % bm0->getHeight();
	//nlinfo("slice0 = %d", slice0);
	Slice0[0].resize(bm0->getWidth(), 1);
	Slice0[1].resize(bm0->getWidth(), 1);
	Slice0[0].blit(*bm0, 0, slice0, bm0->getWidth(), 1, 0, 0);
	Slice0[1].blit(*bm0, 0, slice1, bm0->getWidth(), 1, 0, 0);
	Slice0[0].blend(Slice0[0], Slice0[1], (uint) (256 * fmodf(dayPart * bm0->getHeight(), 1.f)), true);
	Slice1[0].resize(bm0->getWidth(), 1);
	Slice1[1].resize(bm0->getWidth(), 1);
	Slice1[0].blit(*bm1, 0, slice0, bm1->getWidth(), 1, 0, 0);
	Slice1[1].blit(*bm1, 0, slice1, bm1->getWidth(), 1, 0, 0);
	Slice1[0].blend(Slice1[0], Slice1[1], (uint) (256 * fmodf(dayPart * bm0->getHeight(), 1.f)), true);
	Slice0[0].blend(Slice0[0], Slice1[0], (uint) (256 * fmodf(weatherLevel * WeatherToGradient.size(), 1.f)), true);
	// see if blended result differs from cache, if so, update texture of the instance
	const uint32 *newGrad = (uint32 *) &Slice0[0].getPixels()[0];
	if (gradientCache.getPixels(0).empty())
	{
		gradientCache.resize(bm1->getWidth() , 1, CBitmap::RGBA);
	}
	const uint32 *oldGrad = (uint32 *) &gradientCache.getPixels()[0];
	nlassert(gradientCache.getWidth() == Slice0[0].getWidth());
	if (!std::equal(newGrad, newGrad + bm0->getWidth(), oldGrad))
	{
		// update the cache
		gradientCache.swap(Slice0[0]);
		// build a blurred version of the gradient cache (improve quality of gradient)
		if (gradientCacheBlurred.getWidth() != gradientCache.getWidth() ||
			gradientCacheBlurred.getHeight() != gradientCache.getHeight())
		{
			gradientCacheBlurred.resize(gradientCache.getWidth(), 1);
		}
		nlassert(gradientCacheBlurred.PixelFormat == gradientCache.PixelFormat);
		CRGBA *destPix = (CRGBA *) &gradientCacheBlurred.getPixels()[0];
		const CRGBA *srcPix = (const CRGBA *)  &gradientCache.getPixels(0)[0];
		*destPix++ = *srcPix ++;
		const CRGBA *lastSrcPix = srcPix + Slice0[0].getWidth() - 2;
		while (srcPix != lastSrcPix)
		{
			destPix->R = (uint8) (((uint16) srcPix[- 1].R +	(uint16) srcPix->R + (uint16) srcPix[1].R) * (256 / 3) >> 8);
			destPix->G = (uint8) (((uint16) srcPix[- 1].G +	(uint16) srcPix->G + (uint16) srcPix[1].G) * (256 / 3) >> 8);
			destPix->B = (uint8) (((uint16) srcPix[- 1].B +	(uint16) srcPix->B + (uint16) srcPix[1].B) * (256 / 3) >> 8);
			destPix->A = (uint8) (((uint16) srcPix[- 1].A +	(uint16) srcPix->A + (uint16) srcPix[1].A) * (256 / 3) >> 8);
			++ destPix;
			++ srcPix;
		}
		*destPix++ = *srcPix ++;
		// set the new texture
		uint numMaterials = instance.getNumMaterials();
		for(uint k = 0; k < numMaterials; ++k)
		{
			// do a free ccw rotate by swapping height & width (because height is 1)
			instance.getMaterial(k).setTextureMem(TargetTextureStage,
												   &gradientCacheBlurred.getPixels()[0],
												   gradientCacheBlurred.getWidth() * gradientCacheBlurred.getHeight() * sizeof(uint32),
												   false,
												   false,
												   gradientCacheBlurred.getHeight(),
												   gradientCacheBlurred.getWidth());
			// clamp on v coordinate
			instance.getMaterial(k).setWrapT(TargetTextureStage, NL3D::UInstanceMaterial::Clamp);
		}
	}
}

// *************************************************************************************************
////////////////
// CSkyObject //
////////////////

void CSkyObject::init(const CSkyObjectSheet::CVersionSheet &sheet,
					  NL3D::UInstance instance,
					  std::map<std::string,CBitmap *> &bitmapByName,
					  std::vector<CBitmap *> &builtBitmaps,
					  bool	visibleInMainScene,
					  bool	visibleInEnvMap
					 )
{
	if (instance.empty()) return;
	Instance = instance;
	// set display priority
	instance.setTransparencyPriority(sheet.TransparencyPriority);
	PS.cast(Instance);
	//
	DiffuseColor.init(sheet.DiffuseColor, bitmapByName, builtBitmaps);
	ParticleEmitters.init(sheet.ParticleEmitters, bitmapByName, builtBitmaps);
	for(uint k = 0; k < SKY_MAX_NUM_STAGE; ++k)
	{
		ConstantColor[k].init(sheet.ConstantColor[k], bitmapByName, builtBitmaps);
		bool alreadyBuilt;
		if (!sheet.OffsetUBitmap[k].empty())
		{
			OffsetUBitmap[k] = buildSharedBitmap(sheet.OffsetUBitmap[k], bitmapByName, builtBitmaps, alreadyBuilt);
		}
		if (!sheet.OffsetVBitmap[k].empty())
		{
			OffsetVBitmap[k] = buildSharedBitmap(sheet.OffsetVBitmap[k], bitmapByName, builtBitmaps, alreadyBuilt);
		}
	}
	ColorGradient.init(sheet.ColorGradient, bitmapByName, builtBitmaps);
	RefColor = sheet.RefColor;
	std::copy(sheet.TexPanner, sheet.TexPanner + SKY_MAX_NUM_STAGE, TexPanner);
	std::copy(sheet.OffsetFactor, sheet.OffsetFactor + SKY_MAX_NUM_STAGE, OffsetFactor);
	for(uint k = 0; k < SKY_MAX_NUM_FX_USER_PARAMS; ++k)
	{
		if (!sheet.FXUserParamBitmap[k].empty())
		{
			bool alreadyBuilt;
			FXUserParams[k] = buildSharedBitmap(sheet.FXUserParamBitmap[k], bitmapByName, builtBitmaps, alreadyBuilt);
		}
	}
	Name = sheet.ShapeName;
	VisibleInMainScene = visibleInMainScene;
	VisibleInEnvMap = visibleInEnvMap;
}


// *************************************************************************************************
bool CSkyObject::setup(const CClientDate &date, const CClientDate &animationDate, float numHoursInDay, float weatherLevel, CRGBA fogColor, bool envMapScene)
{
	if (Instance.empty()) return false;
	Active = true;
	nlassert(numHoursInDay > 0.f);
	float dayPart = date.Hour / numHoursInDay;
	clamp(dayPart, 0.f, 1.f);
	clamp(weatherLevel, 0.f, 1.f);
	if (DiffuseColor.Mode != Unused)
	{
		CRGBA newDiffuseColor = DiffuseColor.computeColor(dayPart, weatherLevel, fogColor);
		if (newDiffuseColor != LastDiffuseColor)
		{
			// is it a particle system
			if (!PS.empty())
			{
				PS.setUserColor(newDiffuseColor);
				// PS are hiden / shown, so must unfreeze hrc. (adding an isntance group causes hrc to be frozen)
				PS.unfreezeHRC();
			}
			else
			{
				// set diffuse color for each material with normal shader
				uint numMaterials = Instance.getNumMaterials();
				for(uint k = 0; k < numMaterials; ++k)
				{
					UInstanceMaterial im = Instance.getMaterial(k);
					if (im.isLighted())
					{
						Instance.getMaterial(k).setDiffuse(newDiffuseColor);
					}
					else
					{
						Instance.getMaterial(k).setColor(newDiffuseColor);
					}
				}
				// set mean color for other objects (lens flares...)
				Instance.setMeanColor(newDiffuseColor);
			}
			LastDiffuseColor = newDiffuseColor;
		}
		if (RefColor == DiffuseColorRef) // if this is the ref color, then the object is not visible if alpha is 0
		{
			if (newDiffuseColor.A == 0)
			{
				Active = false;
			}
		}
	}
	// is it a particle system
	if (ParticleEmitters.Mode != Unused)
	{
		CRGBA newParticleEmittersColor = ParticleEmitters.computeColor(dayPart, weatherLevel, fogColor);
		if (newParticleEmittersColor != LastParticleEmittersColor)
		{
			if (!PS.empty())
			{
				// emitters are on is any of the components is not 0
				PS.activateEmitters(newParticleEmittersColor != CRGBA::Black);
			}
			LastParticleEmittersColor = newParticleEmittersColor;
		}
		if (RefColor == ParticleEmittersColorRef) // if this is the ref color, then the object is not visible if alpha is 0
		{
			if (LastParticleEmittersColor == CRGBA::Black)
			{
				if (!PS.hasParticles()) // can deactivate PS only when all particles are off
				{
					Active = false;
				}
			}
		}
	}
	uint numMaterials = Instance.getNumMaterials();
	for(uint k = 0; k < SKY_MAX_NUM_STAGE; ++k)
	{
		if (ConstantColor[k].Mode != Unused)
		{
			CRGBA newConstantColor = ConstantColor[k].computeColor(dayPart, weatherLevel, fogColor);
			if (newConstantColor != LastConstantColor[k])
			{
				for(uint l = 0; l < numMaterials; ++l)
				{
					Instance.getMaterial(l).setConstantColor(k, newConstantColor);
				}
				LastConstantColor[k] = newConstantColor;
			}
			if (RefColor == (TSkyRefColor) (ConstantColor0Ref + k))
			{
				if (newConstantColor.A == 0)
				{
					Active = false;
				}
			}
		}
	}
	bool draw = Active;
	if (envMapScene && !VisibleInEnvMap)
	{
		draw = false;
	}
	else if (!envMapScene && !VisibleInMainScene)
	{
		draw = false;
	}
	if (draw)
	{
		Instance.show();
		Instance.unfreezeHRC();
	}
	else
	{
		Instance.hide();
		Instance.freezeHRC();
	}
	double animTime = animationDate.Hour  + (double) animationDate.Day * (double) numHoursInDay;
	if (PS.empty())
	{
		////////////////////
		// gradient setup //
		////////////////////
		ColorGradient.setup(Instance, dayPart, weatherLevel, GradientCache, GradientCacheBlurred);

		///////////////////////
		// tex panning setup //
		///////////////////////
		for(uint k = 0; k < SKY_MAX_NUM_STAGE; ++k)
		{
			if (TexPanner[k].U != 0.f || TexPanner[k].V != 0.f ||
				OffsetUBitmap != NULL || OffsetVBitmap != NULL )
			{
				//nlinfo("global date = %f", animTime);
				// there's tex panning for that stage
				double u = TexPanner[k].U * animTime;
				u = fmod(u, 1);
				double v = TexPanner[k].V * animTime;
				v = fmod(v, 1);
				CVector offset((float) u, (float) v, 0.f);
				// apply scaling if needed
				if (OffsetUBitmap[k])
				{
					offset.x += OffsetFactor[k].U * OffsetUBitmap[k]->getColor(dayPart, weatherLevel, true, false).R;
				}
				if (OffsetVBitmap[k])
				{
					offset.y += OffsetFactor[k].V * OffsetVBitmap[k]->getColor(dayPart, weatherLevel, true, false).R;
				}
				CMatrix mat;
				mat.setPos(offset);
				for(uint l = 0; l < numMaterials; ++l)
				{
					Instance.getMaterial(l).enableUserTexMat(k);
					Instance.getMaterial(l).setUserTexMat(k, mat);
				}
			}
		}
	}
	else
	{
		// user params setup
		for(uint k = 0; k < SKY_MAX_NUM_FX_USER_PARAMS; ++k)
		{
			if (FXUserParams[k])
			{
				CRGBA color = FXUserParams[k]->getColor(dayPart, weatherLevel, true, false);
				PS.setUserParam(k, color.R / 255.f);
			}
		}
	}
	return Active;
}

// *************************************************************************************************
CSkyObject::~CSkyObject()
{
}

