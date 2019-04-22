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


#include "nel/3d/u_material.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/texture_user.h"
#include "nel/3d/driver_user.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************

bool UMaterial::isSupportedByDriver(UDriver &drv, bool forceBaseCaps)
{
	CMaterial *object = getObjectPtr();
	return object->isSupportedByDriver(*(NLMISC::safe_cast<CDriverUser *>(&drv)->getDriver()), forceBaseCaps);
}

// ***************************************************************************

void UMaterial::setAlphaTest(bool active)
{
	CMaterial *object = getObjectPtr();
	object->setAlphaTest(active);
}

// ***************************************************************************

bool UMaterial::getAlphaTest() const
{
	CMaterial *object = getObjectPtr();
	return object->getAlphaTest();
}

// ***************************************************************************

void UMaterial::setAlphaTestThreshold(float threshold)
{
	CMaterial *object = getObjectPtr();
	object->setAlphaTestThreshold(threshold);
}

// ***************************************************************************

float UMaterial::getAlphaTestThreshold() const
{
	CMaterial *object = getObjectPtr();
	return object->getAlphaTestThreshold();
}

// ***************************************************************************

void UMaterial::setTexture(UTexture* ptex)
{
	setTexture (0, ptex);
}

// ***************************************************************************

void UMaterial::setTexture(uint stage, UTexture* ptex)
{
	CMaterial *object = getObjectPtr();
	CTextureUser	*text= dynamic_cast<CTextureUser*>(ptex);
	if (text != NULL)
	{
		object->setTexture (stage, text->getITexture());
	}
	else
	{
		object->setTexture (stage, NULL);
	}
	// NB: _Material smartpoint to this ITexture. But this is correct because so does CTextureUser.
}

// ***************************************************************************

bool UMaterial::texturePresent()
{
	return texturePresent (0);
}

// ***************************************************************************

bool UMaterial::texturePresent (uint stage)
{
	CMaterial *object = getObjectPtr();
	return object->texturePresent (stage);
}

// ***************************************************************************

void UMaterial::selectTextureSet(uint id)
{
	CMaterial *object = getObjectPtr();
	object->selectTextureSet(id);
}

// ***************************************************************************

void UMaterial::setBlend(bool active)
{
	CMaterial *object = getObjectPtr();
	object->setBlend(active);
}

// ***************************************************************************

void UMaterial::setBlendFunc(TBlend src, TBlend dst)
{
	CMaterial *object = getObjectPtr();
	object->setBlendFunc((CMaterial::TBlend)(uint32)src, (CMaterial::TBlend)(uint32)dst);
}

// ***************************************************************************

void UMaterial::setSrcBlend(TBlend val)
{
	CMaterial *object = getObjectPtr();
	object->setSrcBlend((CMaterial::TBlend)(uint32)val);
}

// ***************************************************************************

void UMaterial::setDstBlend(TBlend val)
{
	CMaterial *object = getObjectPtr();
	object->setDstBlend((CMaterial::TBlend)(uint32)val);
}

// ***************************************************************************

bool UMaterial::getBlend() const
{
	CMaterial *object = getObjectPtr();
	return object->getBlend();
}

// ***************************************************************************

UMaterial::TBlend UMaterial::getSrcBlend(void)  const
{
	CMaterial *object = getObjectPtr();
	return (UMaterial::TBlend)(uint32)object->getSrcBlend();
}

// ***************************************************************************

UMaterial::TBlend UMaterial::getDstBlend(void)  const
{
	CMaterial *object = getObjectPtr();
	return (UMaterial::TBlend)(uint32)object->getDstBlend();
}

// ***************************************************************************

void UMaterial::texEnvOpRGB(uint stage, TTexOperator ope)
{
	CMaterial *object = getObjectPtr();
	object->texEnvOpRGB(stage, (CMaterial::TTexOperator)(uint32)ope);
}

// ***************************************************************************

void UMaterial::texEnvArg0RGB (uint stage, TTexSource src, TTexOperand oper)
{
	CMaterial *object = getObjectPtr();
	object->texEnvArg0RGB (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
}

// ***************************************************************************

void UMaterial::texEnvArg1RGB (uint stage, TTexSource src, TTexOperand oper)
{
	CMaterial *object = getObjectPtr();
	object->texEnvArg1RGB (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
}

// ***************************************************************************

void UMaterial::texEnvArg2RGB (uint stage, TTexSource src, TTexOperand oper)
{
	CMaterial *object = getObjectPtr();
	object->texEnvArg2RGB (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
}

// ***************************************************************************

void UMaterial::texEnvOpAlpha(uint stage, TTexOperator ope)
{
	CMaterial *object = getObjectPtr();
	object->texEnvOpAlpha (stage, (CMaterial::TTexOperator)(uint32)ope);
}

// ***************************************************************************

void UMaterial::texEnvArg0Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	CMaterial *object = getObjectPtr();
	object->texEnvArg0Alpha (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
}

// ***************************************************************************

void UMaterial::texEnvArg1Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	CMaterial *object = getObjectPtr();
	object->texEnvArg1Alpha (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
}

// ***************************************************************************

void UMaterial::texEnvArg2Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	CMaterial *object = getObjectPtr();
	object->texEnvArg2Alpha (stage, (CMaterial::TTexSource)(uint32)src, (CMaterial::TTexOperand)(uint32)oper);
}

// ***************************************************************************

void			UMaterial::setZFunc(ZFunc val)
{
	CMaterial *object = getObjectPtr();
	object->setZFunc((CMaterial::ZFunc)(uint32) val);
}

// ***************************************************************************

void			UMaterial::setZWrite(bool active)
{
	CMaterial *object = getObjectPtr();
	object->setZWrite(active);
}

// ***************************************************************************

void			UMaterial::setZBias(float val)
{
	CMaterial *object = getObjectPtr();
	object->setZBias(val);
}

// ***************************************************************************

UMaterial::ZFunc			UMaterial::getZFunc(void)  const
{
	CMaterial *object = getObjectPtr();
	return (UMaterial::ZFunc)(uint32)object->getZFunc();
}

// ***************************************************************************

bool			UMaterial::getZWrite(void)  const
{
	CMaterial *object = getObjectPtr();
	return object->getZWrite();
}

// ***************************************************************************

float			UMaterial::getZBias(void)  const
{
	CMaterial *object = getObjectPtr();
	return object->getZBias();
}

// ***************************************************************************

void			UMaterial::setColor(CRGBA rgba)
{
	CMaterial *object = getObjectPtr();
	object->setColor(rgba);
}

// ***************************************************************************

CRGBA			UMaterial::getColor(void) const
{
	CMaterial *object = getObjectPtr();
	return object->getColor();
}

// ***************************************************************************

void			UMaterial::setDoubleSided(bool doubleSided)
{
	CMaterial *object = getObjectPtr();
	object->setDoubleSided(doubleSided);
}

// ***************************************************************************

bool			UMaterial::getDoubleSided() const
{
	CMaterial *object = getObjectPtr();
	return object->getDoubleSided();
}

// ***************************************************************************

void			UMaterial::initUnlit()
{
	CMaterial *object = getObjectPtr();
	object->initUnlit();
}


// ***************************************************************************
bool			UMaterial::isLighted() const
{
	CMaterial *object = getObjectPtr();
	return object->isLighted();
}
// ***************************************************************************
CRGBA			UMaterial::getEmissive() const
{
	CMaterial *object = getObjectPtr();
	return object->getEmissive();
}
// ***************************************************************************
CRGBA			UMaterial::getAmbient() const
{
	CMaterial *object = getObjectPtr();
	return object->getAmbient();
}
// ***************************************************************************
CRGBA			UMaterial::getDiffuse() const
{
	CMaterial *object = getObjectPtr();
	return object->getDiffuse();
}
// ***************************************************************************
uint8			UMaterial::getOpacity() const
{
	CMaterial *object = getObjectPtr();
	return object->getOpacity();
}
// ***************************************************************************
CRGBA			UMaterial::getSpecular() const
{
	CMaterial *object = getObjectPtr();
	return object->getSpecular();
}
// ***************************************************************************
float			UMaterial::getShininess() const
{
	CMaterial *object = getObjectPtr();
	return object->getShininess();
}

} // NL3D
