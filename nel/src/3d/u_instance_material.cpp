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

#include "nel/3d/u_instance_material.h"
#include "nel/3d/async_texture_block.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/driver.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/texture_file.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
bool				UInstanceMaterial::isTextureFile(uint stage) const
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::isTextureFile : invalid stage");
		return false;
	}
	return dynamic_cast<CTextureFile *>(_Object->getTexture(stage)) != NULL;
}

// ***************************************************************************
std::string			UInstanceMaterial::getTextureFileName(uint stage) const
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::getTextureFileName : invalid stage");
		return "";
	}

	// If Async mode
	if(_MBI->getAsyncTextureMode())
	{
		nlassert(_AsyncTextureBlock->isTextureFile(stage));
		// return name of the async one.
		return _AsyncTextureBlock->TextureNames[stage];
	}
	else
	{
		// return the name in the material
		return NLMISC::safe_cast<CTextureFile *>(_Object->getTexture(stage))->getFileName();
	}
}

// ***************************************************************************
void				UInstanceMaterial::setTextureFileName(const std::string &fileName, uint stage)
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::setTextureFileName : invalid stage");
		return;
	}

	// If Async mode
	if(_MBI->getAsyncTextureMode())
	{
		if(!_AsyncTextureBlock->isTextureFile(stage))
		{
			nlwarning("UInstanceMaterialUser::setTextureFileName : the texture is not a texture file");
			return;
		}
		// replace the fileName
		_AsyncTextureBlock->TextureNames[stage]= fileName;
		// Flag the instance.
		_MBI->setAsyncTextureDirty(true);
	}
	else
	{
		CTextureFile *otherTex = dynamic_cast<CTextureFile *>(_Object->getTexture(stage));
		if (!otherTex)
		{
			nlwarning("UInstanceMaterialUser::setTextureFileName : the texture is not a texture file");
			return;
		}
		CTextureFile *tf = new CTextureFile(*otherTex);
		tf->setFileName(fileName);
		NLMISC::CSmartPtr<ITexture> old = _Object->getTexture(stage);
		_Object->setTexture(stage, tf);
	}
}

// ***************************************************************************
void UInstanceMaterial::emptyTexture(uint stage /*=0*/)
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::emptyTexture : invalid stage");
		return;
	}
	_Object->setTexture(stage, NULL);
}

// ***************************************************************************
/*bool UInstanceMaterial::isSupportedByDriver(UDriver &drv, bool forceBaseCaps)
{
	IDriver *idrv = NLMISC::safe_cast<CDriverUser *>(&drv)->getDriver();
	return _Object->isSupportedByDriver(*idrv, forceBaseCaps);
}*/

// ***************************************************************************
void UInstanceMaterial::setTextureMem(uint stage, uint8 *data, uint32 length, bool _delete, bool isFile /*=true*/, uint width /*=0*/, uint height /*=0*/, CBitmap::TType texType /*=CBitmap::RGBA*/)
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::emptyTexture : invalid stage");
		return;
	}
	_Object->setTexture((uint8) stage, new CTextureMem(data, length, _delete, isFile, width, height, texType));
}

// ***************************************************************************
bool				UInstanceMaterial::isLighted() const
{
	return _Object->isLighted();
}

// ***************************************************************************
void UInstanceMaterial::setLighting(bool active,
									CRGBA emissive /*=CRGBA(0,0,0)*/,
									CRGBA ambient /*=CRGBA(0,0,0)*/,
									CRGBA diffuse /*=CRGBA(0,0,0)*/,
									CRGBA specular /*=CRGBA(0,0,0)*/,
									float shininess /*=10*/)
{
	_Object->setLighting(active, emissive, ambient, diffuse, specular, shininess);
}

// ***************************************************************************

bool				UInstanceMaterial::isUserColor() const
{
	return _Object->getShader()==CMaterial::UserColor;
}

// ***************************************************************************

void				UInstanceMaterial::setEmissive( CRGBA emissive )
{
	_Object->setEmissive(emissive);
}

// ***************************************************************************

void				UInstanceMaterial::setAmbient( CRGBA ambient )
{
	_Object->setAmbient( ambient);
}

// ***************************************************************************

void				UInstanceMaterial::setDiffuse( CRGBA diffuse )
{
	_Object->setDiffuse( diffuse);
}

// ***************************************************************************

void				UInstanceMaterial::setOpacity( uint8	opa )
{
	_Object->setOpacity( opa );
}

// ***************************************************************************

void				UInstanceMaterial::setSpecular( CRGBA specular )
{
	_Object->setSpecular( specular);
}

// ***************************************************************************

void				UInstanceMaterial::setShininess( float shininess )
{
	_Object->setShininess( shininess );
}

// ***************************************************************************

CRGBA				UInstanceMaterial::getEmissive() const
{
	return _Object->getEmissive();
}

// ***************************************************************************

CRGBA				UInstanceMaterial::getAmbient() const
{
	return _Object->getAmbient();
}

// ***************************************************************************

CRGBA				UInstanceMaterial::getDiffuse() const
{
	return _Object->getDiffuse();
}

// ***************************************************************************

uint8				UInstanceMaterial::getOpacity() const
{
	return _Object->getOpacity();
}

// ***************************************************************************

CRGBA				UInstanceMaterial::getSpecular() const
{
	return _Object->getSpecular();
}

// ***************************************************************************

float				UInstanceMaterial::getShininess() const
{
	return _Object->getShininess();
}

// ***************************************************************************

void				UInstanceMaterial::setColor(CRGBA rgba)
{
	_Object->setColor(rgba) ;
}

// ***************************************************************************

CRGBA				UInstanceMaterial::getColor(void) const
{
	return _Object->getColor();
}

// ***************************************************************************

void				UInstanceMaterial::setUserColor(CRGBA userColor)
{
	if(isUserColor())
		_Object->setUserColor(userColor);
}

// ***************************************************************************

CRGBA				UInstanceMaterial::getUserColor() const
{
	if(isUserColor())
		return _Object->getUserColor();
	else
		return CRGBA(0,0,0,0);
}

// ***************************************************************************

void				UInstanceMaterial::setConstantColor(uint stage, NLMISC::CRGBA color)
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::setConstantColor : invalid stage");
		return;
	}
	_Object->texConstantColor(stage, color);
}

// ***************************************************************************

NLMISC::CRGBA		UInstanceMaterial::getConstantColor(uint stage) const
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterialUser::getConstantColor : invalid stage");
		return NLMISC::CRGBA::Black;
	}
	return _Object->getTexConstantColor(stage);

}

// ***************************************************************************

sint				UInstanceMaterial::getLastTextureStage() const
{
	sint lastStage = -1;
	for(uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		if (_Object->getTexture(k) != NULL)
		{
			lastStage = k;
		}
	}
	return lastStage;
}

// ***************************************************************************

void			UInstanceMaterial::setBlend(bool active)
{
	_Object->setBlend(active);
}

// ***************************************************************************

void			UInstanceMaterial::setBlendFunc(TBlend src, TBlend dst)
{
	_Object->setBlendFunc((CMaterial::TBlend)(uint32)src, (CMaterial::TBlend)(uint32)dst);
}

// ***************************************************************************

void			UInstanceMaterial::setSrcBlend(TBlend val)
{
	_Object->setSrcBlend((CMaterial::TBlend)(uint32)val);
}

// ***************************************************************************

void			UInstanceMaterial::setDstBlend(TBlend val)
{
	_Object->setDstBlend((CMaterial::TBlend)(uint32)val);
}

// ***************************************************************************

void			UInstanceMaterial::setAlphaTestThreshold(float at)
{
	_Object->setAlphaTestThreshold(at);
}

// ***************************************************************************
float UInstanceMaterial::getAlphaTestThreshold() const
{
	return _Object->getAlphaTestThreshold();
}

// ***************************************************************************
void UInstanceMaterial::setAlphaTest(bool active)
{
	_Object->setAlphaTest(active);
}

// ***************************************************************************

void			UInstanceMaterial::setZWrite(bool active)
{
	_Object->setZWrite(active);
}

// ***************************************************************************
void UInstanceMaterial::setZFunc(ZFunc val)
{
		_Object->setZFunc((CMaterial::ZFunc) val);
}

// ***************************************************************************

bool			UInstanceMaterial::getBlend() const
{
	return _Object->getBlend();
}

// ***************************************************************************

UInstanceMaterial::TBlend			UInstanceMaterial::getSrcBlend(void)  const
{
	return (UInstanceMaterial::TBlend)(uint32)_Object->getSrcBlend();
}

// ***************************************************************************

UInstanceMaterial::TBlend			UInstanceMaterial::getDstBlend(void)  const
{
	return (UInstanceMaterial::TBlend)(uint32)_Object->getDstBlend();
}

// ***************************************************************************

void                    UInstanceMaterial::enableUserTexMat(uint stage, bool enabled)
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterial::enableUserTexMat : stage %d is invalid", stage);
		return;
	}
	_Object->enableUserTexMat(stage, enabled);
}

// ***************************************************************************

bool                    UInstanceMaterial::isUserTexMatEnabled(uint stage) const
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterial::enableUserTexMat : stage %d is invalid", stage);
		return false;
	}
	return _Object->isUserTexMatEnabled(stage);
}

// ***************************************************************************

void					UInstanceMaterial::setUserTexMat(uint stage, const NLMISC::CMatrix &m)
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterial::enableUserTexMat : stage %d is invalid", stage);
		return;
	}
	if (!_Object->isUserTexMatEnabled(stage))
	{
		nlwarning("UInstanceMaterial::setUserTexMat : texture stage %d has no user matrix.", stage);
	}
	_Object->setUserTexMat(stage, m);
}

// ***************************************************************************

const NLMISC::CMatrix  &UInstanceMaterial::getUserTexMat(uint stage) const
{
	if (stage >= IDRV_MAT_MAXTEXTURES)
	{
		nlwarning("UInstanceMaterial::enableUserTexMat : stage %d is invalid", stage);
		return CMatrix::Identity;
	}
	if (!_Object->isUserTexMatEnabled(stage))
	{
		nlwarning("UInstanceMaterial::setUserTexMat : texture stage %d has no user matrix.", stage);
		return CMatrix::Identity;
	}
	return _Object->getUserTexMat(stage);
}

// ***************************************************************************

void				UInstanceMaterial::setWrapS(uint stage, TWrapMode mode)
{
	if (stage >= IDRV_MAT_MAXTEXTURES || _Object->getTexture(stage) == NULL)
	{
		nlwarning("UInstanceMaterial::setWrapS : stage %d is invalid or there's no texture", stage);
		return;
	}
	_Object->getTexture(stage)->setWrapS((ITexture::TWrapMode) mode);
}

// ***************************************************************************

void				UInstanceMaterial::setWrapT(uint stage, TWrapMode mode)
{
	if (stage >= IDRV_MAT_MAXTEXTURES || _Object->getTexture(stage) == NULL)
	{
		nlwarning("UInstanceMaterial::setWrapT : stage %d is invalid or there's no texture", stage);
		return;
	}
	_Object->getTexture(stage)->setWrapT((ITexture::TWrapMode) mode);
}

// ***************************************************************************

UInstanceMaterial::TWrapMode			UInstanceMaterial::getWrapS(uint stage) const
{
	if (stage >= IDRV_MAT_MAXTEXTURES || _Object->getTexture(uint8(stage)) == NULL)
	{
		nlwarning("UInstanceMaterial::getWrapS : stage %d is invalid or there's no texture", stage);
		return Repeat;
	}
	return (TWrapMode) _Object->getTexture(uint8(stage))->getWrapS();
}

// ***************************************************************************

UInstanceMaterial::TWrapMode			UInstanceMaterial::getWrapT(uint stage) const
{
	if (stage >= IDRV_MAT_MAXTEXTURES || _Object->getTexture(uint8(stage)) == NULL)
	{
		nlwarning("UInstanceMaterial::getWrapT : stage %d is invalid or there's no texture", stage);
		return Repeat;
	}
	return (TWrapMode) _Object->getTexture(uint8(stage))->getWrapT();
}

// ***************************************************************************

} // NL3D
