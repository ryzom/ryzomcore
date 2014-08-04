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

// NEL3D
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_camera.h"

//3D
#include "nel/3d/driver_user.h"
#include "nel/3d/texture_bloom.h"
#include "nel/3d/texture_user.h"

#include "nel/3d/bloom_effect.h"


using namespace NLMISC;
using namespace NL3D;
using namespace std;

namespace NL3D
{


// vertex program used to blur texture
static const char *TextureOffset =
"!!VP1.0																	\n\
	MOV o[COL0].x, c[8].x;	          										\n\
	MOV o[COL0].y, c[8].y;	          										\n\
	MOV o[COL0].z, c[8].z;	          										\n\
	MOV o[COL0].w, c[8].w;	          										\n\
	MOV o[HPOS].x, v[OPOS].x;												\n\
	MOV o[HPOS].y, v[OPOS].y;												\n\
	MOV o[HPOS].z, v[OPOS].z;												\n\
	MOV o[HPOS].w, c[9].w;													\n\
	ADD o[TEX0], v[TEX0], c[10];											\n\
	ADD o[TEX1], v[TEX0], c[11];											\n\
	ADD o[TEX2], v[TEX0], c[12];											\n\
	ADD o[TEX3], v[TEX0], c[13];											\n\
	END \n";


static NLMISC::CSmartPtr<CVertexProgram> TextureOffsetVertexProgram;


//-----------------------------------------------------------------------------------------------------------

CBloomEffect::CBloomEffect()
{
	if (!TextureOffsetVertexProgram)
	{
		TextureOffsetVertexProgram = new CVertexProgram(TextureOffset);
	}

	_Driver = NULL;
	_Scene = NULL;
	_SquareBloom = true;
	_DensityBloom = 128;
	_Init = false;

	_BlurFinalTex = NULL;
	_BlurHorizontalTex = NULL;
}

//-----------------------------------------------------------------------------------------------------------

CBloomEffect::~CBloomEffect()
{
	if (_Init)
	{
		if (!_DisplayBlurMat.empty())
		{
			if (_Driver) _Driver->deleteMaterial(_DisplayBlurMat);
		}

		if (!_DisplaySquareBlurMat.empty())
		{
			if (_Driver) _Driver->deleteMaterial(_DisplaySquareBlurMat);
		}

		if (!_BlurMat.empty())
		{
			if (_Driver) _Driver->deleteMaterial(_BlurMat);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::init()
{
	if (!((CDriverUser *)_Driver)->getDriver()->supportBloomEffect())
		return;

	CDriverUser *dru = static_cast<CDriverUser *>(_Driver);
	IDriver *drv = dru->getDriver();

	_BlurWidth = 256;
	_BlurHeight = 256;

	// initialize blur material
	_BlurMat = _Driver->createMaterial();
	CMaterial * matObject = _BlurMat.getObjectPtr();
	_BlurMat.initUnlit();
	_BlurMat.setColor(CRGBA::White);
	_BlurMat.setBlend (false);
	_BlurMat.setAlphaTest (false);
	matObject->setBlendFunc (CMaterial::one, CMaterial::zero);
	matObject->setZWrite(false);
	matObject->setZFunc(CMaterial::always);
	matObject->setDoubleSided(true);

	// initialize stages of fixed pipeline
	CRGBA constantCol1(85, 85, 85, 85);
	CRGBA constantCol2(43, 43, 43, 43);
	// stage 0
	matObject->texConstantColor(0, constantCol1);
	matObject->texEnvOpRGB(0, CMaterial::Modulate);
	matObject->texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
	matObject->texEnvArg1RGB(0, CMaterial::Constant, CMaterial::SrcColor);

	// stage 1
	matObject->texConstantColor(1, constantCol2);
	matObject->texEnvOpRGB(1, CMaterial::Mad);
	matObject->texEnvArg0RGB(1, CMaterial::Texture, CMaterial::SrcColor);
	matObject->texEnvArg1RGB(1, CMaterial::Constant, CMaterial::SrcColor);
	matObject->texEnvArg2RGB(1, CMaterial::Previous, CMaterial::SrcColor);

	// stage 2
	matObject->texConstantColor(2, constantCol1);
	matObject->texEnvOpRGB(2, CMaterial::Mad);
	matObject->texEnvArg0RGB(2, CMaterial::Texture, CMaterial::SrcColor);
	matObject->texEnvArg1RGB(2, CMaterial::Constant, CMaterial::SrcColor);
	matObject->texEnvArg2RGB(2, CMaterial::Previous, CMaterial::SrcColor);

	// stage 3
	matObject->texConstantColor(3, constantCol2);
	matObject->texEnvOpRGB(3, CMaterial::Mad);
	matObject->texEnvArg0RGB(3, CMaterial::Texture, CMaterial::SrcColor);
	matObject->texEnvArg1RGB(3, CMaterial::Constant, CMaterial::SrcColor);
	matObject->texEnvArg2RGB(3, CMaterial::Previous, CMaterial::SrcColor);

	// initialize linear blur material
	_DisplayBlurMat = _Driver->createMaterial();
	CMaterial * matObjectFinal = _DisplayBlurMat.getObjectPtr();
	_DisplayBlurMat.initUnlit();
	_DisplayBlurMat.setColor(CRGBA::White);
	matObjectFinal->setBlend(true);
	matObjectFinal->setBlendFunc(CMaterial::one, CMaterial::invsrccolor);
	matObjectFinal->setZWrite(false);
	matObjectFinal->setZFunc(CMaterial::always);
	matObjectFinal->setDoubleSided(true);

	// matObjectFinal->setTexture(0, _BlurFinalTex);
	matObjectFinal->texEnvOpRGB(0, CMaterial::Modulate);
	matObjectFinal->texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
	matObjectFinal->texEnvArg1RGB(0, CMaterial::Constant, CMaterial::SrcColor);

	// initialize square blur material
	_DisplaySquareBlurMat = _Driver->createMaterial();
	matObjectFinal = _DisplaySquareBlurMat.getObjectPtr();
	_DisplaySquareBlurMat.initUnlit();
	_DisplaySquareBlurMat.setColor(CRGBA::White);
	matObjectFinal->setBlend(true);
	matObjectFinal->setBlendFunc(CMaterial::one, CMaterial::invsrccolor);
	matObjectFinal->setZWrite(false);
	matObjectFinal->setZFunc(CMaterial::always);
	matObjectFinal->setDoubleSided(true);

	matObjectFinal->texEnvOpRGB(0, CMaterial::Modulate);
	matObjectFinal->texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
	matObjectFinal->texEnvArg1RGB(0, CMaterial::Constant, CMaterial::SrcColor);

	matObjectFinal->texEnvOpRGB(1, CMaterial::Modulate);
	matObjectFinal->texEnvArg0RGB(1, CMaterial::Texture, CMaterial::SrcColor);
	matObjectFinal->texEnvArg1RGB(1, CMaterial::Previous, CMaterial::SrcColor);

	// initialize quads
	_BlurQuad.V0 = CVector(-1.f, -1.f,	0.5f);
	_BlurQuad.V1 = CVector(1.f,	 -1.f,	0.5f);
	_BlurQuad.V2 = CVector(1.f,	 1.f,	0.5f);
	_BlurQuad.V3 = CVector(-1.f, 1.f,	0.5f);
	if (drv->textureCoordinateAlternativeMode())
	{
		_BlurQuad.Uv0 = CUV(0.f, 1.f);
		_BlurQuad.Uv1 = CUV(1.f, 1.f);
		_BlurQuad.Uv2 = CUV(1.f, 0.f);
		_BlurQuad.Uv3 = CUV(0.f, 0.f);
	}
	else
	{
		_BlurQuad.Uv0 = CUV(0.f, 0.f);
		_BlurQuad.Uv1 = CUV(1.f, 0.f);
		_BlurQuad.Uv2 = CUV(1.f, 1.f);
		_BlurQuad.Uv3 = CUV(0.f, 1.f);
	}

	_Init = true;
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::applyBloom()
{
	if (!((CDriverUser *)_Driver)->getDriver()->supportBloomEffect())
		return;

	// don't activate bloom when PolygonMode is different from Filled
	if (_Driver->getPolygonMode() != UDriver::Filled) return;

	if (_Driver->getWindowWidth()==0 || _Driver->getWindowHeight()==0)
		return;

	if (!_Init)
		init();

	CDriverUser *dru = static_cast<CDriverUser *>(_Driver);
	IDriver *drv = dru->getDriver();

	NL3D::ITexture *renderTarget = drv->getRenderTarget();
	nlassert(renderTarget);
	nlassert(renderTarget->isBloomTexture());

	uint width = renderTarget->getWidth();
	uint height = renderTarget->getHeight();
	bool mode2D = static_cast<CTextureBloom *>(renderTarget)->isMode2D();
	nlassert(renderTarget->getUploadFormat() == ITexture::Auto);

	if (width >= 256) _BlurWidth = 256;
	else _BlurWidth = raiseToNextPowerOf2(width) / 2;
	if (height >= 256) _BlurHeight = 256;
	else _BlurHeight = raiseToNextPowerOf2(height) / 2;

	nlassert(!_BlurFinalTex);
	_BlurFinalTex = _Driver->getRenderTargetManager().getRenderTarget(_BlurWidth, _BlurHeight, true);
	nlassert(!_BlurHorizontalTex);
	_BlurHorizontalTex = _Driver->getRenderTargetManager().getRenderTarget(_BlurWidth, _BlurHeight, true);

	_DisplayBlurMat.getObjectPtr()->setTexture(0, _BlurFinalTex->getITexture());
	_DisplaySquareBlurMat.getObjectPtr()->setTexture(0, _BlurFinalTex->getITexture());
	_DisplaySquareBlurMat.getObjectPtr()->setTexture(1, _BlurFinalTex->getITexture());

	CTextureUser texNull;
	dru->setRenderTarget(texNull);

	// Stretch original render target into blur texture
	CTextureUser txt1(renderTarget);
	CTextureUser txt2(_BlurFinalTex->getITexture());
	CRect rect1(0, 0, width, height);
	CRect rect2(0, 0, _BlurWidth, _BlurHeight);
	dru->stretchRect(_Scene, txt1, rect1, txt2, rect2);
	_Driver->setMatrixMode2D11();

	// horizontal blur pass
	doBlur(true);

	// vertical blur pass
	doBlur(false);

	// apply blur with a blend operation
	drv->setRenderTarget(renderTarget);
	applyBlur();

	// cleanup material texture references
	_DisplayBlurMat.getObjectPtr()->setTexture(0, NULL);
	_DisplaySquareBlurMat.getObjectPtr()->setTexture(0, NULL);
	_DisplaySquareBlurMat.getObjectPtr()->setTexture(1, NULL);
	_BlurMat.getObjectPtr()->setTexture(0, NULL);
	_BlurMat.getObjectPtr()->setTexture(1, NULL);
	_BlurMat.getObjectPtr()->setTexture(2, NULL);
	_BlurMat.getObjectPtr()->setTexture(3, NULL);

	// recycle render targets
	_Driver->getRenderTargetManager().recycleRenderTarget(_BlurFinalTex);
	_BlurFinalTex = NULL;
	_Driver->getRenderTargetManager().recycleRenderTarget(_BlurHorizontalTex);
	_BlurHorizontalTex = NULL;
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::applyBlur()
{
	NL3D::IDriver *drvInternal = ((CDriverUser *) _Driver)->getDriver();

	// initialize vertex program
	drvInternal->activeVertexProgram(TextureOffsetVertexProgram);
	drvInternal->setUniform4f(IDriver::VertexProgram, 8, 255.f, 255.f, 255.f, 255.f);
	drvInternal->setUniform4f(IDriver::VertexProgram, 9, 0.0f, 0.f, 0.f, 1.f);

	// initialize blur material
	UMaterial displayBlurMat;
	if(_SquareBloom)
	{
		displayBlurMat = _DisplaySquareBlurMat;
	}
	else
	{
		displayBlurMat = _DisplayBlurMat;
	}
	CMaterial * matObjectFinal = displayBlurMat.getObjectPtr();

	uint8 d = _DensityBloom;
	CRGBA constCoeff(d, d, d, d);
	matObjectFinal->texConstantColor(0, constCoeff);

	// display quad
	_Driver->drawQuad(_BlurQuad, displayBlurMat);

	// disable vertex program
	drvInternal->activeVertexProgram(NULL);
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::doBlur(bool horizontalBlur)
{
	CVector2f blurVec;
	ITexture * startTexture;
	ITexture * endTexture;

	// set displayed texture and render target texture of the pass
	if (horizontalBlur)
	{
		blurVec = CVector2f(1.f, 0.f);
		startTexture = _BlurFinalTex->getITexture();
		endTexture = _BlurHorizontalTex->getITexture();
	}
	else
	{
		blurVec = CVector2f(0.f, 1.f);
		startTexture = _BlurHorizontalTex->getITexture();
		endTexture = _BlurFinalTex->getITexture();
	}

	NL3D::IDriver *drvInternal = ((CDriverUser *) _Driver)->getDriver();
	CTextureUser txt(endTexture);
	// initialize render target
	if(!((CDriverUser *) _Driver)->setRenderTarget(txt, 0, 0, _BlurWidth, _BlurHeight))
	{
		nlwarning("setRenderTarget return false with blur texture for bloom effect\n");
		return;
	}

	// initialize vertex program
	drvInternal->activeVertexProgram(TextureOffsetVertexProgram);
	drvInternal->setUniform4f(IDriver::VertexProgram, 8, 255.f, 255.f, 255.f, 255.f);
	drvInternal->setUniform4f(IDriver::VertexProgram, 9, 0.0f, 0.f, 0.f, 1.f);

	// set several decal constants in order to obtain in the render target texture a mix of color
	// of a texel and its neighbored texels on the axe of the pass.
	float decalL, decal2L, decalR, decal2R;
	if (drvInternal->textureCoordinateAlternativeMode())
	{
		if (horizontalBlur)
		{
			decalL = 0.5f;
			decal2L = -0.5f;
			decalR = 1.5f;
			decal2R = 2.5f;
		}
		else
		{
			decalL = 0.0f;
			decal2L = -1.0f;
			decalR = 1.0f;
			decal2R = 2.0f;
		}
	}
	else
	{
		decalL = -0.5f;
		decal2L = -1.5f;
		decalR = 0.5f;
		decal2R = 1.5f;
	}
	drvInternal->setUniform2f(IDriver::VertexProgram, 10, (decalR/(float)_BlurWidth)*blurVec.x,		(decalR/(float)_BlurHeight)*blurVec.y);
	drvInternal->setUniform2f(IDriver::VertexProgram, 11, (decal2R/(float)_BlurWidth)*blurVec.x,		(decal2R/(float)_BlurHeight)*blurVec.y);
	drvInternal->setUniform2f(IDriver::VertexProgram, 12, (decalL/(float)_BlurWidth)*blurVec.x,		(decalL/(float)_BlurHeight)*blurVec.y);
	drvInternal->setUniform2f(IDriver::VertexProgram, 13, (decal2L/(float)_BlurWidth)*blurVec.x,		(decal2L/(float)_BlurHeight)*blurVec.y);

	// initialize material textures
	CMaterial * matObject = _BlurMat.getObjectPtr();
	matObject->setTexture(0, startTexture);
	matObject->setTexture(1, startTexture);
	matObject->setTexture(2, startTexture);
	matObject->setTexture(3, startTexture);

	// display
	_Driver->drawQuad(_BlurQuad, _BlurMat);

	// disable render target and vertex program
	drvInternal->activeVertexProgram(NULL);
	CTextureUser cu;
	((CDriverUser *)_Driver)->setRenderTarget(cu, 0, 0, 0, 0);
}

}; // NL3D
