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


static CVertexProgram TextureOffsetVertexProgram(TextureOffset);

// TODO_VP_GLSL


//-----------------------------------------------------------------------------------------------------------

CBloomEffect::CBloomEffect()
{
	_Driver = NULL;
	_Scene = NULL;
	_SquareBloom = true;
	_DensityBloom = 128;
	_Init = false;
	_InitBloomEffect = false;
}

//-----------------------------------------------------------------------------------------------------------

CBloomEffect::~CBloomEffect()
{
	if(_Init)
	{
		if(!_DisplayInitMat.empty())
		{
			_DisplayInitMat.getObjectPtr()->setTexture(0, NULL);
			if (_Driver) _Driver->deleteMaterial(_DisplayInitMat);
		}
		_InitText = NULL;

		if(!_DisplayBlurMat.empty())
		{
			_DisplayBlurMat.getObjectPtr()->setTexture(0, NULL);
			if (_Driver) _Driver->deleteMaterial(_DisplayBlurMat);
		}
		if(!_DisplaySquareBlurMat.empty())
		{
			_DisplaySquareBlurMat.getObjectPtr()->setTexture(0, NULL);
			_DisplaySquareBlurMat.getObjectPtr()->setTexture(1, NULL);
			if (_Driver) _Driver->deleteMaterial(_DisplaySquareBlurMat);
		}

		if(!_BlurMat.empty())
		{
			_BlurMat.getObjectPtr()->setTexture(0, NULL);
			_BlurMat.getObjectPtr()->setTexture(1, NULL);
			_BlurMat.getObjectPtr()->setTexture(2, NULL);
			_BlurMat.getObjectPtr()->setTexture(3, NULL);
			if (_Driver) _Driver->deleteMaterial(_BlurMat);
		}

		_BlurHorizontalTex = NULL;
		_BlurFinalTex = NULL;
	}
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::init(bool initBloomEffect)
{
	_InitBloomEffect = initBloomEffect;

	if(((CDriverUser *)_Driver)->getDriver()->supportBloomEffect())
		init();
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::init()
{
	_WndWidth = _Driver->getWindowWidth();
	_WndHeight = _Driver->getWindowHeight();

	_BlurWidth = 256;
	_BlurHeight = 256;

	// initialize textures
	_InitText = NULL;
	_BlurHorizontalTex = NULL;
	_BlurFinalTex = NULL;
	if(_InitBloomEffect)
	{
		initTexture(_InitText, false, _WndWidth, _WndHeight);
	}
	initTexture(_BlurFinalTex,		true, _BlurWidth, _BlurHeight);
	initTexture(_BlurHorizontalTex, true, _BlurWidth, _BlurHeight);

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

	// initialize display materials
	if(_InitBloomEffect)
	{
		_DisplayInitMat = _Driver->createMaterial();
		CMaterial * matObjectInit = _DisplayInitMat.getObjectPtr();
		_DisplayInitMat.initUnlit();
		_DisplayInitMat.setColor(CRGBA::White);
		_DisplayInitMat.setBlend (false);
		_DisplayInitMat.setAlphaTest (false);
		matObjectInit->setBlendFunc (CMaterial::one, CMaterial::zero);
		matObjectInit->setZWrite(false);
		matObjectInit->setZFunc(CMaterial::always);
		matObjectInit->setDoubleSided(true);
		matObjectInit->setTexture(0, _InitText);
	}

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

	matObjectFinal->setTexture(0, _BlurFinalTex);
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

	matObjectFinal->setTexture(0, _BlurFinalTex);
	matObjectFinal->texEnvOpRGB(0, CMaterial::Modulate);
	matObjectFinal->texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
	matObjectFinal->texEnvArg1RGB(0, CMaterial::Constant, CMaterial::SrcColor);

	matObjectFinal->setTexture(1, _BlurFinalTex);
	matObjectFinal->texEnvOpRGB(1, CMaterial::Modulate);
	matObjectFinal->texEnvArg0RGB(1, CMaterial::Texture, CMaterial::SrcColor);
	matObjectFinal->texEnvArg1RGB(1, CMaterial::Previous, CMaterial::SrcColor);

	// initialize quads
	_DisplayQuad.V0 = CVector(0.f, 0.f, 0.5f);
	_DisplayQuad.V1 = CVector(1.f, 0.f, 0.5f);
	_DisplayQuad.V2 = CVector(1.f, 1.f, 0.5f);
	_DisplayQuad.V3 = CVector(0.f, 1.f, 0.5f);

	_BlurQuad.V0 = CVector(-1.f, -1.f,	0.5f);
	_BlurQuad.V1 = CVector(1.f,	 -1.f,	0.5f);
	_BlurQuad.V2 = CVector(1.f,	 1.f,	0.5f);
	_BlurQuad.V3 = CVector(-1.f, 1.f,	0.5f);

	_Init = true;
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::initTexture(CSmartPtr<ITexture> & tex, bool isMode2D, uint32 width, uint32 height)
{
	NL3D::IDriver *drvInternal = ((CDriverUser *) _Driver)->getDriver();

	tex = new CTextureBloom();
	tex->setReleasable(false);
	tex->resize(width, height);
	tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	tex->setWrapS(ITexture::Clamp);
	tex->setWrapT(ITexture::Clamp);
	((CTextureBloom *)tex.getPtr())->mode2D(isMode2D);
	if(tex->TextureDrvShare==NULL || tex->TextureDrvShare->DrvTexture.getPtr()==NULL)
	{
		tex->setRenderTarget(true);
		drvInternal->setupTexture(*tex);
	}
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::initBloom() // clientcfg
{
	if(!((CDriverUser *)_Driver)->getDriver()->supportBloomEffect())
		return;

	// don't activate bloom when PolygonMode is different from Filled
	if (_Driver->getPolygonMode() != UDriver::Filled) return;

	if(_Driver->getWindowWidth()==0 || _Driver->getWindowHeight()==0)
		return;

	if(!_Init)
		init();

	_OriginalRenderTarget = static_cast<CDriverUser *>(_Driver)->getDriver()->getRenderTarget();

	// if window resize, reinitialize textures
	if(_WndWidth!=_Driver->getWindowWidth() || _WndHeight!=_Driver->getWindowHeight())
	{
		_WndWidth = _Driver->getWindowWidth();
		_WndHeight = _Driver->getWindowHeight();

		if(_InitBloomEffect)
		{
			// release old SmartPtr
			_DisplayInitMat.getObjectPtr()->setTexture(0, NULL);
			_InitText = NULL;

			initTexture(_InitText, false, _WndWidth, _WndHeight);

			_DisplayInitMat.getObjectPtr()->setTexture(0, _InitText);
		}

		bool reinitBlurTextures = false;
		if(_WndWidth<_BlurWidth || _WndHeight<_BlurHeight)
		{
			_BlurWidth = raiseToNextPowerOf2(_WndWidth)/2;
			_BlurHeight = raiseToNextPowerOf2(_WndHeight)/2;

			reinitBlurTextures = true;
		}

		if(_WndWidth>256 && _BlurWidth!=256)
		{
			_BlurWidth = 256;
			reinitBlurTextures = true;
		}

		if(_WndHeight>256 && _BlurHeight!=256)
		{
			_BlurHeight = 256;
			reinitBlurTextures = true;
		}

		if(reinitBlurTextures)
		{
			// release old SmartPtr
			_DisplayBlurMat.getObjectPtr()->setTexture(0, NULL);

			_DisplaySquareBlurMat.getObjectPtr()->setTexture(0, NULL);
			_DisplaySquareBlurMat.getObjectPtr()->setTexture(1, NULL);

			_BlurMat.getObjectPtr()->setTexture(0, NULL);
			_BlurMat.getObjectPtr()->setTexture(1, NULL);
			_BlurMat.getObjectPtr()->setTexture(2, NULL);
			_BlurMat.getObjectPtr()->setTexture(3, NULL);

			_BlurHorizontalTex = NULL;
			_BlurFinalTex = NULL;

			initTexture(_BlurFinalTex,		true, _BlurWidth, _BlurHeight);
			initTexture(_BlurHorizontalTex, true, _BlurWidth, _BlurHeight);

			_DisplayBlurMat.getObjectPtr()->setTexture(0, _BlurFinalTex);

			_DisplaySquareBlurMat.getObjectPtr()->setTexture(0, _BlurFinalTex);
			_DisplaySquareBlurMat.getObjectPtr()->setTexture(1, _BlurFinalTex);
		}
	}

	if (!_OriginalRenderTarget)
	{
		NL3D::CTextureUser txt = (_InitBloomEffect) ? (CTextureUser(_InitText)) : (CTextureUser());
		if(!(static_cast<CDriverUser *>(_Driver)->setRenderTarget(txt, 0, 0, _WndWidth, _WndHeight)))
		{
			nlwarning("setRenderTarget return false with initial texture for bloom effect\n");
			return;
		}
	}
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::endBloom() // clientcfg
{
	if(!_Driver->supportBloomEffect() || !_Init)
		return;

	// don't activate bloom when PolygonMode is different from Filled
	if (_Driver->getPolygonMode() != UDriver::Filled) return;

	if(_Driver->getWindowWidth()==0 || _Driver->getWindowHeight()==0)
		return;

	CTextureUser txt1 = _OriginalRenderTarget ? CTextureUser(_OriginalRenderTarget) : ((_InitBloomEffect) ? (CTextureUser(_InitText)) : (CTextureUser()));
	CTextureUser txt2(_BlurFinalTex);
	CRect rect1(0, 0, _WndWidth, _WndHeight);
	CRect rect2(0, 0, _BlurWidth, _BlurHeight);
	// stretch rect
	((CDriverUser *) _Driver)->stretchRect(_Scene, txt1 , rect1,
		txt2, rect2);

	// horizontal blur pass
	doBlur(true);

	// vertical blur pass
	doBlur(false);

	// apply blur with a blend operation
	applyBlur();
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::applyBlur()
{
	NL3D::IDriver *drvInternal = ((CDriverUser *) _Driver)->getDriver();

	/*if (_OriginalRenderTarget)
	{
		CTextureUser txt(_OriginalRenderTarget);
		if(!(static_cast<CDriverUser *>(_Driver)->setRenderTarget(txt, 0, 0, _WndWidth, _WndHeight)))
		{
			nlwarning("setRenderTarget return false with original render target for bloom effect\n");
			return;
		}
	}
	// in opengl, display in init texture
	else if(_InitBloomEffect)
	{
		CTextureUser txt(_InitText);
		if(!(static_cast<CDriverUser *>(_Driver)->setRenderTarget(txt, 0, 0, _WndWidth, _WndHeight)))
		{
			nlwarning("setRenderTarget return false with initial texture for bloom effect\n");
			return;
		}
	}*/
	CTextureUser txtApply = _OriginalRenderTarget ? CTextureUser(_OriginalRenderTarget) : ((_InitBloomEffect) ? (CTextureUser(_InitText)) : (CTextureUser()));
	if(!(static_cast<CDriverUser *>(_Driver)->setRenderTarget(txtApply, 0, 0, _WndWidth, _WndHeight)))
	{
		nlwarning("setRenderTarget return false with initial texture for bloom effect\n");
		return;
	}

	// display blur texture
	// initialize blur texture coordinates
	if(_InitBloomEffect)
	{
		_BlurQuad.Uv0 = CUV(0.f, 0.f);
		_BlurQuad.Uv1 = CUV(1.f, 0.f);
		_BlurQuad.Uv2 = CUV(1.f, 1.f);
		_BlurQuad.Uv3 = CUV(0.f, 1.f);
	}
	else
	{
		_BlurQuad.Uv0 = CUV(0.f, 1.f);
		_BlurQuad.Uv1 = CUV(1.f, 1.f);
		_BlurQuad.Uv2 = CUV(1.f, 0.f);
		_BlurQuad.Uv3 = CUV(0.f, 0.f);
	}

	// initialize vertex program
	drvInternal->activeVertexProgram(&TextureOffsetVertexProgram);
	drvInternal->setConstant(8, 255.f, 255.f, 255.f, 255.f);
	drvInternal->setConstant(9, 0.0f, 0.f, 0.f, 1.f);

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
	UCamera	pCam = _Scene->getCam();
	_Driver->setMatrixMode2D11();
	_Driver->drawQuad(_BlurQuad, displayBlurMat);
	_Driver->setMatrixMode3D(pCam);

	// disable vertex program
	drvInternal->activeVertexProgram(NULL);
}

//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::endInterfacesDisplayBloom() // clientcfg
{
	// Render from render target to screen if necessary.
	// Don't do this when the blend was done to the screen or when rendering to a user provided rendertarget.
	if ((_OriginalRenderTarget.getPtr() == NULL) && _InitBloomEffect)
	{
		if(!_Driver->supportBloomEffect() || !_Init)
			return;

		// don't activate bloom when PolygonMode is different from Filled
		if (_Driver->getPolygonMode() != UDriver::Filled) return;

		if(_Driver->getWindowWidth()==0 || _Driver->getWindowHeight()==0)
			return;

		NL3D::IDriver *drvInternal = ((CDriverUser *) _Driver)->getDriver();
		CTextureUser txtNull;
		((CDriverUser *)_Driver)->setRenderTarget(txtNull, 0, 0, 0, 0);

		// initialize texture coordinates
		float newU = drvInternal->isTextureRectangle(_InitText) ? (float)_WndWidth : 1.f;
		float newV = drvInternal->isTextureRectangle(_InitText) ? (float)_WndHeight : 1.f;

		_DisplayQuad.Uv0 = CUV(0.f,  0.f);
		_DisplayQuad.Uv1 = CUV(newU, 0.f);
		_DisplayQuad.Uv2 = CUV(newU, newV);
		_DisplayQuad.Uv3 = CUV(0.f,  newV);

		// init material texture
//		CMaterial * matObjectInit = _DisplayInitMat.getObjectPtr();

		// display
		UCamera	pCam = _Scene->getCam();
		_Driver->setMatrixMode2D11();
		_Driver->drawQuad(_DisplayQuad, _DisplayInitMat);
		_Driver->setMatrixMode3D(pCam);
	}

	_OriginalRenderTarget = NULL;
}


//-----------------------------------------------------------------------------------------------------------

void CBloomEffect::doBlur(bool horizontalBlur)
{
	CVector2f blurVec;
	ITexture * startTexture;
	ITexture * endTexture;

	// set displayed texture and render target texture of the pass
	if(horizontalBlur)
	{
		blurVec = CVector2f(1.f, 0.f);
		startTexture = _BlurFinalTex;
		endTexture = _BlurHorizontalTex;
	}
	else
	{
		blurVec = CVector2f(0.f, 1.f);
		startTexture = _BlurHorizontalTex;
		endTexture = _BlurFinalTex;
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
	drvInternal->activeVertexProgram(&TextureOffsetVertexProgram);
	drvInternal->setConstant(8, 255.f, 255.f, 255.f, 255.f);
	drvInternal->setConstant(9, 0.0f, 0.f, 0.f, 1.f);

	// set several decal constants in order to obtain in the render target texture a mix of color
	// of a texel and its neighbored texels on the axe of the pass.
	float decalL, decal2L, decalR, decal2R;
	if(_InitBloomEffect)
	{
		decalL = -0.5f;
		decal2L = -1.5f;
		decalR = 0.5f;
		decal2R = 1.5f;
	}
	else
	{
		decalL = 0.f;
		decal2L = -1.f;
		decalR = 1.f;
		decal2R = 2.f;
	}
	drvInternal->setConstant(10, (decalR/(float)_BlurWidth)*blurVec.x,		(decalR/(float)_BlurHeight)*blurVec.y, 0.f, 0.f);
	drvInternal->setConstant(11, (decal2R/(float)_BlurWidth)*blurVec.x,		(decal2R/(float)_BlurHeight)*blurVec.y, 0.f, 0.f);
	drvInternal->setConstant(12, (decalL/(float)_BlurWidth)*blurVec.x,		(decalL/(float)_BlurHeight)*blurVec.y, 0.f, 0.f);
	drvInternal->setConstant(13, (decal2L/(float)_BlurWidth)*blurVec.x,		(decal2L/(float)_BlurHeight)*blurVec.y, 0.f, 0.f);

	// initialize material textures
	CMaterial * matObject = _BlurMat.getObjectPtr();
	matObject->setTexture(0, startTexture);
	matObject->setTexture(1, startTexture);
	matObject->setTexture(2, startTexture);
	matObject->setTexture(3, startTexture);

	// initialize quad
	_BlurQuad.Uv0 = CUV(0.0f,	0.0f);
	_BlurQuad.Uv1 = CUV(1.f,		0.0f);
	_BlurQuad.Uv2 = CUV(1.f,		1.f);
	_BlurQuad.Uv3 = CUV(0.0f,	1.f);

	// display
	UCamera	pCam = _Scene->getCam();
	_Driver->setMatrixMode2D11();
	_Driver->drawQuad(_BlurQuad, _BlurMat);

	// disable render target and vertex program
	drvInternal->activeVertexProgram(NULL);
	CTextureUser cu;
	((CDriverUser *)_Driver)->setRenderTarget(cu, 0, 0, 0, 0);
	_Driver->setMatrixMode3D(pCam);
}

}; // NL3D
