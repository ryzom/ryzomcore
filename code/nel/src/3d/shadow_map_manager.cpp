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
#include "nel/3d/shadow_map_manager.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/driver.h"
#include "nel/3d/scene.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/dru.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/visual_collision_manager.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/fast_floor.h"


using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {


// ***************************************************************************
// easineasout
static inline float	easeInEaseOut(float x)
{
	float	y;
	// cubic such that f(0)=0, f'(0)=0, f(1)=1, f'(1)=0.
	float	x2=x*x;
	float	x3=x2*x;
	y= -2*x3 + 3*x2;
	return y;
}


// ***************************************************************************
CShadowMapManager::CShadowMapManager()
{
	uint	i;

	// For Texture profiling
	_TextureCategory= new ITexture::CTextureCategory("SHADOW MANAGER");

	setQuadGridSize(NL3D_SMM_QUADGRID_SIZE, NL3D_SMM_QUADCELL_SIZE);
	_ShadowCasters.reserve(256);
	_GenerateShadowCasters.reserve(256);
	_PolySmooth= true;

	// **** Setup Fill
	_FillQuads.setVertexFormat(CVertexBuffer::PositionFlag);
	_FillMaterial.initUnlit();
	_FillMaterial.setColor(CRGBA(0,0,0,0));
	_FillMaterial.setZWrite(false);
	_FillMaterial.setZFunc(CMaterial::always);
	_FillMaterial.setDoubleSided(true);
	_FillQuads.setPreferredMemory(CVertexBuffer::RAMVolatile, true);

	// **** Setup Blur
	_BlurQuads.setVertexFormat(CVertexBuffer::PositionFlag |
		CVertexBuffer::TexCoord0Flag |
		CVertexBuffer::TexCoord1Flag |
		CVertexBuffer::TexCoord2Flag |
		CVertexBuffer::TexCoord3Flag);
	_BlurQuads.setPreferredMemory(CVertexBuffer::RAMVolatile, true);

	// Only 2 quads are used to blur
	_BlurQuads.setNumVertices(8);
	for (i=0;i<2;i++)
	{
		_BlurMaterial[i].initUnlit();
		_BlurMaterial[i].setColor(CRGBA::White);
		_BlurMaterial[i].setZWrite(false);
		_BlurMaterial[i].setZFunc(CMaterial::always);
		_BlurMaterial[i].setDoubleSided(true);
		// Setup The Blur. NB: it will take advantage of Max 4 texture driver support, but will still
		// work with 2 or 3 (less beautifull).
		uint j;
		for(j=1;j<4;j++)
		{
			_BlurMaterial[i].texEnvOpRGB(j, CMaterial::InterpolateConstant);
			_BlurMaterial[i].texEnvArg0RGB(j, CMaterial::Texture, CMaterial::SrcColor);
			_BlurMaterial[i].texEnvArg1RGB(j, CMaterial::Previous, CMaterial::SrcColor);
			_BlurMaterial[i].texEnvOpAlpha(j, CMaterial::InterpolateConstant);
			_BlurMaterial[i].texEnvArg0Alpha(j, CMaterial::Texture, CMaterial::SrcAlpha);
			_BlurMaterial[i].texEnvArg1Alpha(j, CMaterial::Previous, CMaterial::SrcAlpha);
		}
		// Factor for Stage so the sum is 1.
		_BlurMaterial[i].texConstantColor(1, CRGBA(128,128,128,128));		// factor= 1/2
		_BlurMaterial[i].texConstantColor(2, CRGBA(85,85,85,85));			// factor= 1/3
		_BlurMaterial[i].texConstantColor(3, CRGBA(64,64,64,64));			// factor= 1/4
	}

	_BlurTextureW= 0;
	_BlurTextureH= 0;

	// *** Setup copy
	_CopyQuads.setVertexFormat (CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
	_CopyQuads.setNumVertices(4);
	_CopyQuads.setPreferredMemory(CVertexBuffer::RAMVolatile, true);
	CVertexBufferReadWrite vba;
	_CopyQuads.lock (vba);
	vba.setVertexCoord (0, CVector (0, 0, 0));
	vba.setVertexCoord (1, CVector (1, 0, 0));
	vba.setVertexCoord (2, CVector (1, 0, 1));
	vba.setVertexCoord (3, CVector (0, 0, 1));

	// Copy material
	_CopyMaterial.initUnlit();
	_CopyMaterial.setColor(CRGBA::White);
	_CopyMaterial.setZWrite(false);
	_CopyMaterial.setZFunc(CMaterial::always);
	_CopyMaterial.setDoubleSided(true);
	_CopyMaterial.setBlend (false);
	_CopyMaterial.setAlphaTest (false);
	_CopyMaterial.setBlendFunc (CMaterial::one, CMaterial::zero);

	// **** Setup Receiving

	// Setup the clamp texture.
	const	uint	clampTextSize= 512;
	const	uint	clampNearFadeSize= 32;
	const	uint	clampFarFadeSize= 128;
	uint			textMemSize= 4*clampTextSize*1;
	// Fill mem
	uint8	*tmpMem= new uint8[textMemSize];
	memset(tmpMem, 255, textMemSize);
	for(i=0;i<clampNearFadeSize;++i)
	{
		float	f= (float)i/clampNearFadeSize;
		f= easeInEaseOut(f);
		tmpMem[4*i+3]= uint8(255.f*f);
	}
	for(i=0;i<clampFarFadeSize;++i)
	{
		float	f= (float)i/clampFarFadeSize;
		f= easeInEaseOut(f);
		tmpMem[4*(clampTextSize-i-1)+3]= uint8(255.f*f);
	}
	// build the texture
	_ClampTexture = new CTextureMem (tmpMem, 4*clampTextSize*1, true, false, clampTextSize, 1);
	_ClampTexture->setWrapS (ITexture::Clamp);
	_ClampTexture->setWrapT (ITexture::Clamp);
	_ClampTexture->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	_ClampTexture->generate();
	_ClampTexture->setReleasable (false);
	// For Texture Profiling
	_ClampTexture->setTextureCategory(_TextureCategory);

	// init material
	_ReceiveShadowMaterial.initUnlit();
	_ReceiveShadowMaterial.setBlend(true);
	_ReceiveShadowMaterial.setBlendFunc(CMaterial::zero, CMaterial::invsrccolor);
	_ReceiveShadowMaterial.setZWrite(false);
	// FillRate Optim
	_ReceiveShadowMaterial.setAlphaTest(true);
	_ReceiveShadowMaterial.setAlphaTestThreshold(0.01f);

	// ---- Stage 0. Project the ShadowMap. Blend the color between ShadowColor and White.
	// setup texture coord gen
	_ReceiveShadowMaterial.enableUserTexMat(0, true);
	_ReceiveShadowMaterial.setTexCoordGen(0, true);
	_ReceiveShadowMaterial.setTexCoordGenMode(0, CMaterial::TexCoordGenObjectSpace);
	// Setup the stage so we interpolate ShadowColor and White (according to shadowmap alpha)
	// nico : with D3D driver, limitation of the number of per stage constant (Only 1 if diffuse is used), so do a blend between inv diffuse & black (instead of diffuse & white), which resolve to a modulate between
	// source alpha & inverse diffuse.  then invert result at subsequent stage
	_ReceiveShadowMaterial.texEnvOpRGB(0, CMaterial::Modulate);
	_ReceiveShadowMaterial.texEnvArg0RGB(0, CMaterial::Diffuse, CMaterial::InvSrcColor);
	_ReceiveShadowMaterial.texEnvArg1RGB(0, CMaterial::Texture, CMaterial::SrcAlpha);
	// Take Alpha for AlphaTest only.
	_ReceiveShadowMaterial.texEnvOpAlpha(0, CMaterial::Replace);
	_ReceiveShadowMaterial.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);

	// ---- Stage 1. "Modulate" by Clamp Texture. Blend the color between stage0 color and White.
	// setup texture coord gen
	_ReceiveShadowMaterial.enableUserTexMat(1, true);
	_ReceiveShadowMaterial.setTexCoordGen(1, true);
	_ReceiveShadowMaterial.setTexCoordGenMode(1, CMaterial::TexCoordGenObjectSpace);
	_ReceiveShadowMaterial.setTexture(1, _ClampTexture);
	// Setup the stage so we interpolate Shadow and White (according to clamp alpha)
	_ReceiveShadowMaterial.texEnvOpRGB(1, CMaterial::Modulate);
	_ReceiveShadowMaterial.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor); // Color is inverted before the blend
	_ReceiveShadowMaterial.texEnvArg1RGB(1, CMaterial::Texture, CMaterial::SrcAlpha);
	// Take Alpha for AlphaTest only. (take 1st texture alpha...)
	_ReceiveShadowMaterial.texEnvOpAlpha(0, CMaterial::Replace);
	_ReceiveShadowMaterial.texEnvArg0Alpha(0, CMaterial::Previous, CMaterial::SrcAlpha);

	// **** Setup Casting
	_CasterShadowMaterial.initUnlit();
	_CasterShadowMaterial.setColor(CRGBA::White);
	_CasterShadowMaterial.setZWrite(false);
	_CasterShadowMaterial.setZFunc(CMaterial::always);
	_CasterShadowMaterial.setDoubleSided(true);
	// Alpha Polygon coverage accumulate, for polygon smoothing
	_CasterShadowMaterial.setBlend(true);
	_CasterShadowMaterial.setBlendFunc(CMaterial::one, CMaterial::one);

	_BlurQuads.setName("CShadowMapManager::_BlurQuads");
	_FillQuads.setName("CShadowMapManager::_FillQuads");
	_CopyQuads.setName("CShadowMapManager::_CopyQuads");
}

// ***************************************************************************
CShadowMapManager::~CShadowMapManager()
{
	clearAllShadowCasters();
}

// ***************************************************************************
void			CShadowMapManager::setQuadGridSize(uint size, float cellSize)
{
	_ShadowReceiverGrid.create(size, cellSize);
}

// ***************************************************************************
void			CShadowMapManager::addShadowCaster(CTransform *model)
{
	_ShadowCasters.push_back(model);
}

// ***************************************************************************
void			CShadowMapManager::addShadowReceiver(CTransform *model)
{
	CAABBox	bb;
	model->getReceiverBBox(bb);

	_ShadowReceiverGrid.insert(bb.getMin(), bb.getMax(), model);
}

// ***************************************************************************
void			CShadowMapManager::renderGenerate(CScene *scene)
{
	H_AUTO( NL3D_ShadowManager_Generate );

	// Each frame, do a small garbage collector for unused free textures.
	garbageShadowTextures(scene);

	IDriver *driverForShadowGeneration= scene->getRenderTrav().getAuxDriver();
	CSmartPtr<NL3D::ITexture> previousRenderTarget = driverForShadowGeneration->getRenderTarget();

	// Init
	// ********
	uint32	wndW= _BlurTextureW, wndH= _BlurTextureH;
	// get some text/screen size.
	if(driverForShadowGeneration)
		driverForShadowGeneration->getWindowSize(wndW, wndH);
	uint	baseTextureSize= scene->getShadowMapTextureSize();
	// Minimize the Dest Texture size, so the blurTexture don't get too heavy in VRAM.
	uint32	textDestW= min(wndW, (uint32)NL3D_SMM_MAX_TEXTDEST_SIZE);
	uint32	textDestH= min(wndH, (uint32)NL3D_SMM_MAX_TEXTDEST_SIZE);

	// if not needed or if not possible, exit. test for wndSize is also important when window is minimized
	if( _ShadowCasters.empty() ||
		textDestW<baseTextureSize || textDestH<baseTextureSize)
	{
		clearAllShadowCasters();
		return;
	}

	// If Needed to project some ShadowCaster, but none to compute this frame, quit.
	if( _GenerateShadowCasters.empty() )
	{
		// But here don't reset since the renderProject() will do job
		return;
	}

	// get the number of shadowMap compute we can do in one screen.
	uint	numTextW= textDestW/baseTextureSize;
	uint	numTextH= textDestH/baseTextureSize;
	if(!isPowerOf2(numTextW))
		numTextW= raiseToNextPowerOf2(numTextW)/2;
	if(!isPowerOf2(numTextH))
		numTextH= raiseToNextPowerOf2(numTextH)/2;
	// the max shadow casters we can do in 1 screen pass.
	uint	maxSCPerPass= numTextW * numTextH;

	// compute vp float size.
	float	vpWidth= (float)baseTextureSize / (float)(numTextW*baseTextureSize);
	float	vpHeight= (float)baseTextureSize / (float)(numTextH*baseTextureSize);


	// Create / Update the Blur Texture
	updateBlurTexture(*driverForShadowGeneration, numTextW * baseTextureSize, numTextH * baseTextureSize);


	// Do NPass if a screen is not sufficient to render all shadow maps...
	// ********

	// bkup driver state
	CViewport	bkupViewport;
	driverForShadowGeneration->getViewport(bkupViewport);
	bool		bkupFog= driverForShadowGeneration->fogEnabled();

	// setup some state
	driverForShadowGeneration->enableFog(false);
	// Allow Writing on alpha only. => don't write on RGB objects!
	driverForShadowGeneration->setColorMask(false, false, false, true);

	uint	numSC= (uint)_GenerateShadowCasters.size();
	uint	baseSC= 0;
	while(numSC>0)
	{
		uint	numPassSC= min(maxSCPerPass, numSC);
		// number of line including the last line if not empty
		uint	numTotalLine= (numPassSC+numTextW-1)/numTextW;
		// number of column.
		uint	numTotalCol= (numPassSC<numTextW)?numPassSC:numTextW;

		// Render to the blur texture
		driverForShadowGeneration->setRenderTarget (_BlurTexture[0], 0, 0, numTotalCol*baseTextureSize, numTotalLine*baseTextureSize);

		uint	textX, textY;
		uint	i;

		// Render All Shadow Map
		// ********

		// Render the polygons with Smooth Anti-Alias. Less jittering for little performance overcost
		if(_PolySmooth)
			driverForShadowGeneration->enablePolygonSmoothing(true);

		textX=0;
		textY=0;
		for(i=0;i<numPassSC;i++)
		{
			// get the transform to compute shadow map.
			CTransform	*sc= _GenerateShadowCasters[baseSC+i];

			// select the shadow direction
			CVector		lightDir;
			computeShadowDirection(scene, sc, lightDir);

			// setup viewport to render to
			CViewport	vp;
			vp.init(textX*baseTextureSize/(float)_BlurTextureW, textY*baseTextureSize/(float)_BlurTextureH, vpWidth, vpHeight);
			driverForShadowGeneration->setupViewport(vp);

			// TODO_SHADOW: optim: one big erase per pass, but just bbox needed (according to number of SC to render)
			// do a siccor or prefer do a polygon clear?
			CScissor	sic;
			sic.init(textX*baseTextureSize/(float)_BlurTextureW, textY*baseTextureSize/(float)_BlurTextureH, vpWidth, vpHeight);
			driverForShadowGeneration->setupScissor(sic);
			driverForShadowGeneration->clear2D(CRGBA(0,0,0,0));

			// render to screen
			sc->generateShadowMap(lightDir);

			// next text
			textX++;
			if(textX==numTextW)
			{
				textX= 0;
				textY++;
			}
		}

		// Restore
		if(_PolySmooth)
			driverForShadowGeneration->enablePolygonSmoothing(false);

		// For Subsequent operations, setup a full viewport and a "Screen Frustum"
		CScissor	sic;
		sic.initFullScreen();
		// TODO_SHADOW: optim: need scissor?
		driverForShadowGeneration->setupScissor(sic);
		CViewport	vp;
		vp.initFullScreen();
		driverForShadowGeneration->setupViewport(vp);
		driverForShadowGeneration->setFrustum(0, (float)_BlurTextureW, 0, (float)_BlurTextureH, -1,1,false);
		driverForShadowGeneration->setupViewMatrix(CMatrix::Identity);
		driverForShadowGeneration->setupModelMatrix(CMatrix::Identity);

		// Ensure the One pixel black security on texture border
		fillBlackBorder(driverForShadowGeneration, numPassSC, numTextW, numTextH, baseTextureSize);

		// Blur.
		// ********
		uint	numBlur= scene->getShadowMapBlurSize();
		clamp(numBlur, 0U, 3U);
		uint blurTarget = 0;
		for(i=0;i<numBlur;i++)
		{
			// Set the blur texture target
			blurTarget = (i+1)&1;
			const uint blurSource = i&1;
			driverForShadowGeneration->setRenderTarget (_BlurTexture[blurTarget], 0, 0, numTotalCol*baseTextureSize, numTotalLine*baseTextureSize);

			// blur
			applyFakeGaussianBlur(driverForShadowGeneration, numPassSC, numTextW, numTextH, baseTextureSize, blurSource);

			// Ensure the One pixel black security on texture border
			fillBlackBorder(driverForShadowGeneration, numPassSC, numTextW, numTextH, baseTextureSize);
		}

		// Copy the last blur texture
		_CopyMaterial.setTexture(0, _BlurTexture[blurTarget]);

		// Store Screen in ShadowMaps
		// ********
		textX=0;
		textY=0;
		for(i=0;i<numPassSC;i++)
		{
			// get the transform to compute shadow map.
			CTransform	*sc= _GenerateShadowCasters[baseSC+i];
			CShadowMap	*sm= sc->getShadowMap();
			if(sm)
			{
				ITexture	*text= sm->getTexture();
				if(text)
				{
					uint	bts= baseTextureSize;

					// todo hulud : Try the temporary buffer trick (openGL)
					//if (!driverForShadowGeneration->copyTargetToTexture (text, 0, 0, textX*bts, textY*bts, bts, bts))
					{
						// Copy the texture
						driverForShadowGeneration->setRenderTarget (text, 0, 0, bts, bts);
						driverForShadowGeneration->clear2D (CRGBA(0,0,0,0));

						// Viewport is already fullscreen, set the frustum
						driverForShadowGeneration->setFrustum(0, 1, 0, 1, -1,1,false);

						// Set the vertex buffer UV
						{
							CVertexBufferReadWrite vba;
							_CopyQuads.lock (vba);
							const float u= (float)(textX*bts)*_BlurTextureOOW;
							const float v= (float)(textY*bts)*_BlurTextureOOH;
							const float width= (float)bts*_BlurTextureOOW;
							const float height= (float)bts*_BlurTextureOOH;
							vba.setTexCoord	(0, 0, u, v);
							vba.setTexCoord	(1, 0, u+width, v);
							vba.setTexCoord	(2, 0, u+width, v+height);
							vba.setTexCoord	(3, 0, u, v+height);
						}

						// Vertex buffer
						driverForShadowGeneration->activeVertexBuffer (_CopyQuads);

						CScissor	sic;
						sic.initFullScreen();
						// TODO_SHADOW: optim: need scissor?
						driverForShadowGeneration->setupScissor(sic);

						driverForShadowGeneration->setupViewMatrix(CMatrix::Identity);
						driverForShadowGeneration->setupModelMatrix(CMatrix::Identity);

						// Render the shadow in the final shadow texture
						vp.init (0, 0, 1, 1);
						driverForShadowGeneration->setupViewport(vp);
						driverForShadowGeneration->renderRawQuads (_CopyMaterial, 0, 1);

						// Set default render target
						driverForShadowGeneration->setRenderTarget (NULL);
					}

					// Indicate to the ShadowMap that we have updated his Texture
					sm->LastGenerationFrame= scene->getNumRender();
				}
			}

			// next text
			textX++;
			if(textX==numTextW)
			{
				textX= 0;
				textY++;
			}
		}


		// next screen pass.
		baseSC+= numPassSC;
		numSC-= numPassSC;
	}

	// Set default render target
	driverForShadowGeneration->setRenderTarget (previousRenderTarget);

	// Allow Writing on all.
	driverForShadowGeneration->setColorMask(true, true, true, true);
	// Restore driver state. (do it here because driverForShadowGeneration may be the main screen).
	driverForShadowGeneration->setupViewport(bkupViewport);
	driverForShadowGeneration->enableFog(bkupFog);
	// TODO_SHADOW: optim need scissor?
	CScissor	sic;
	sic.initFullScreen();
	driverForShadowGeneration->setupScissor(sic);

	// ensure the Scene Driver has correct matrix setup (in common case where AuxDriver == Std Driver)
	scene->getRenderTrav().setupDriverCamera();


	// Clear ShadowCaster Generation
	clearGenerateShadowCasters();
}

// ***************************************************************************
void			CShadowMapManager::renderProject(CScene *scene)
{
	// if not needed exit. NB renderGenerate() must have been called before.
	if( _ShadowCasters.empty() )
	{
		return;
	}


	// Project ShadowMap on receivers.
	// ********

	H_AUTO( NL3D_ShadowManager_Project );


	/* Fog Case: Since we do a modulate, we don't want to modulate the fog color with himself.
		Instead, if the shadowed pixel is in full fog, we have to modulate him with Blac (modulate with INVERSE-source color, actually ...)
		=> replace fog color with black temporarily
	*/
	IDriver	*driver= scene->getRenderTrav().getDriver();
	CRGBA	bkupFogColor= driver->getFogColor();


	driver->setupFog(driver->getFogStart(), driver->getFogEnd(), CRGBA::Black);

	/* Light case: CVisualCollisionManager use a fakeLight to avoid ShadowMapping on backFaces of meshs
		Hence must clean all lights, and enable only the Light0 in driver
	*/
	// Use CRenderTrav::resetLightSetup() to do so, to reset its Cache information
	scene->getRenderTrav().resetLightSetup();
	driver->enableLight(0, true);


	// For each ShadowMap
	for(uint i=0;i<_ShadowCasters.size();i++)
	{
		CTransform	*caster= _ShadowCasters[i];
		CShadowMap	*sm= caster->getShadowMap();
		nlassert(sm);
		// NB: the ShadowCaster may not have a texture yet, for example because of Generate selection...
		// If the final fade is 1, don't render!
		if( sm->getTexture() && sm->getFinalFade()<1 )
		{
			CVector		casterPos= caster->getWorldMatrix().getPos();

			// Compute the World bbox (for quadGrid intersection)
			CAABBox		worldBB= sm->LocalBoundingBox;
			worldBB.setCenter(worldBB.getCenter() + casterPos);

			// compute the world matrix of the projection.
			CMatrix		worldProjMat= sm->LocalProjectionMatrix;
			worldProjMat.setPos(worldProjMat.getPos()+casterPos);

			// Now compute the textureMatrix, from WorldSpace to UV.
			CMatrix		wsTextMat;
			wsTextMat= worldProjMat;
			wsTextMat.invert();

			// setup the Material.
			_ReceiveShadowMaterial.setTexture(0, sm->getTexture());
			/// Get The Mean Ambiant and Diffuse the caster receive (and cast, by approximation)
			CRGBA	ambient, diffuse;
			computeShadowColors(scene, caster, ambient, diffuse);
			// In some case, the ambient may be black, which cause problems because the shadow pop while diffuse fade.
			// ThereFore, supose always a minimum of ambiant 10.
			ambient.R= max(uint8(10), ambient.R);
			ambient.G= max(uint8(10), ambient.G);
			ambient.B= max(uint8(10), ambient.B);
			// copute the shadowColor so that modulating a Medium diffuse terrain will  get the correct result.
			uint	R= ambient.R + (diffuse.R>>1);
			uint	G= ambient.G + (diffuse.G>>1);
			uint	B= ambient.B + (diffuse.B>>1);
			clamp(R, 1U, 256U);
			clamp(G, 1U, 256U);
			clamp(B, 1U, 256U);
			/* screen= text*(a+d*0.5) (mean value). if we do shadowColor= a/(a+d*0.5f),
				then we'll have "in theory"  screen= text*a
			*/
			R= (uint)(256 * ambient.R / (float)R);
			G= (uint)(256 * ambient.G / (float)G);
			B= (uint)(256 * ambient.B / (float)B);
			clamp(R,0U,255U);
			clamp(G,0U,255U);
			clamp(B,0U,255U);
			/// Finally "modulate" with FinalFade.
			if(sm->getFinalFade()>0)
			{
				sint	factor= OptFastFloor( 256 * sm->getFinalFade() );
				clamp(factor, 0, 256);
				R= 255*factor + R*(256-factor); R>>=8;
				G= 255*factor + G*(256-factor); G>>=8;
				B= 255*factor + B*(256-factor); B>>=8;
			}
			_ReceiveShadowMaterial.setColor(CRGBA(uint8(R),uint8(G),uint8(B),255));

			// init the _ShadowMapProjector
			_ShadowMapProjector.setWorldSpaceTextMat(wsTextMat);

			// select receivers.
			_ShadowReceiverGrid.select(worldBB.getMin(), worldBB.getMax());
			// For all receivers
			TShadowReceiverGrid::CIterator	it;
			for(it= _ShadowReceiverGrid.begin();it!=_ShadowReceiverGrid.end();it++)
			{
				CTransform	*receiver= *it;
				// Avoid Auto-Casting.
				if(receiver==caster)
					continue;

				// update the material texture projection
				// see getReceiverRenderWorldMatrix() Doc for why using this instead of getWorldMatrix()
				_ShadowMapProjector.applyToMaterial(receiver->getReceiverRenderWorldMatrix(), _ReceiveShadowMaterial);

				// cast the shadow on them
				receiver->receiveShadowMap(sm, casterPos, _ReceiveShadowMaterial);
			}

			// Additionaly, the VisualCollisionManager may manage some shadow receiving
			CVisualCollisionManager		*shadowVcm= scene->getVisualCollisionManagerForShadow();
			if(shadowVcm)
			{
				shadowVcm->receiveShadowMap(driver, sm, casterPos, _ReceiveShadowMaterial, _ShadowMapProjector);
			}
		}
	}

	// Restore fog color
	driver->setupFog(driver->getFogStart(), driver->getFogEnd(), bkupFogColor);

	// Leave Light Setup in a clean State
	scene->getRenderTrav().resetLightSetup();


	// TestYoyo. Display Projection BBox.
	/*{
		for(uint i=0;i<_ShadowCasters.size();i++)
		{
			// get the transform to compute shadow map.
			CTransform	*sc= _ShadowCasters[i];

			CShadowMap	*sm= sc->getShadowMap();
			if(sm)
			{
				CVector		p0= sm->LocalProjectionMatrix.getPos() + sc->getWorldMatrix().getPos();
				IDriver		&drv= *driver;

				drv.setupModelMatrix(CMatrix::Identity);

				CDRU::drawWiredBox(p0, sm->LocalProjectionMatrix.getI(), sm->LocalProjectionMatrix.getJ(),
					sm->LocalProjectionMatrix.getK(), CRGBA::White, drv);
			}
		}
	}*/

	/* // hulud test
	CScissor	sic;
	sic.initFullScreen();
	// TODO_SHADOW: optim: need scissor?
	driver->setupScissor(sic);
	driver->setupViewMatrix(CMatrix::Identity);
	driver->setupModelMatrix(CMatrix::Identity);
	driver->setFrustum (0,1,0,1,-1,1,false);
	// Render the shadow in the final shadow texture
	CViewport vp;
	vp.init (0, 0, 0.5f, 0.5f);
	driver->setupViewport(vp);

	static CVertexBuffer CopyQuads;
	static CMaterial CopyMaterial;
	CopyMaterial.initUnlit();
	CopyMaterial.setColor(CRGBA::White);
	CopyMaterial.setZWrite(false);
	CopyMaterial.setZFunc(CMaterial::always);
	CopyMaterial.setDoubleSided(true);
	CopyMaterial.setBlend (false);
	CopyMaterial.setAlphaTest (false);
	CopyMaterial.setBlendFunc (CMaterial::one, CMaterial::zero);
	CopyMaterial.texEnvOpRGB(0, CMaterial::Replace);
	CopyMaterial.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcAlpha);
	CopyQuads.setVertexFormat (CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
	CopyQuads.setNumVertices(4);
	{
		CVertexBufferReadWrite vba;
		CopyQuads.lock (vba);
		vba.setVertexCoord (0, CVector (0, 0, 0));
		vba.setVertexCoord (1, CVector (1, 0, 0));
		vba.setVertexCoord (2, CVector (1, 0, 1));
		vba.setVertexCoord (3, CVector (0, 0, 1));
		vba.setTexCoord	(0, 0, 0, 0);
		vba.setTexCoord	(1, 0, 1, 0);
		vba.setTexCoord	(2, 0, 1, 1);
		vba.setTexCoord	(3, 0, 0, 1);
	}
	driver->activeVertexBuffer (CopyQuads);

	if (!_ShadowCasters.empty())
	{
		// get the transform to compute shadow map.
		CTransform	*sc= _ShadowCasters[0];

		CShadowMap	*sm= sc->getShadowMap();
		if(sm)
		{
			CopyMaterial.setTexture (0, sm->getTexture());
			driver->renderRawQuads (CopyMaterial, 0, 1);
		}
	}*/


	// Release pass.
	// ********
	clearAllShadowCasters();
}


// ***************************************************************************
void			CShadowMapManager::computeShadowDirection(CScene *scene, CTransform *sc, CVector &lightDir)
{
	// merge the sunLight and pointLights into a single directional
	lightDir= scene->getSunDirection();
	const	CLightContribution	&lc= sc->getLightContribution();
	// For Better result, weight with the light color too.
	CRGBA	color= scene->getSunDiffuse();
	lightDir*= (float)lc.SunContribution * (color.R + color.G + color.B);

	// merge pointLights
	const CVector		&modelPos= sc->getWorldMatrix().getPos();
	for(uint i=0;i<NL3D_MAX_LIGHT_CONTRIBUTION;i++)
	{
		CPointLight		*pl= lc.PointLight[i];
		// End of List?
		if(!pl)
			break;

		CVector	plDir= modelPos - pl->getPosition();
		plDir.normalize();
		// Sum with this light, weighted by AttFactor, and light color
		color= pl->getDiffuse();
		lightDir+= plDir * (float)lc.AttFactor[i] * (float)(color.R + color.G + color.B);
	}

	// normalize merged dir
	lightDir.normalize();

	// clamp the light direction in z, according to Caster restriction
	float	zThre= sc->getShadowMapDirectionZThreshold();
	if(lightDir.z>zThre)
	{
		/* normalize the x/y component so z=zthre
			we want this: sqrt(x2+y2+z2)==1, which solve for x2+y2= 1-z2
			the scale to apply to x and y is therefore deduced from:
					sqr(scale)=(1-z2)/(x2+y2)
		*/
		float	scale= 0.f;
		if(lightDir.x!=0.f || lightDir.y!=0.f)
			scale= sqrtf( (1-sqr(zThre)) / (sqr(lightDir.x)+sqr(lightDir.y)) );
		lightDir.x*= scale;
		lightDir.y*= scale;
		// force z component to be at least zthre
		lightDir.z= zThre;

		// re-normalize in case of precision problems
		lightDir.normalize();
	}
}


// ***************************************************************************
void			CShadowMapManager::computeShadowColors(CScene *scene, CTransform *sc, CRGBA &ambient, CRGBA &diffuse)
{
	const	CLightContribution	&lc= sc->getLightContribution();

	// Get the current ambiant
	ambient= lc.computeCurrentAmbient(scene->getSunAmbient());

	// Compute the current diffuse as a sum (not a mean)
	uint	r, g, b;
	CRGBA	color= scene->getSunDiffuse();
	r= color.R * lc.SunContribution;
	g= color.G * lc.SunContribution;
	b= color.B * lc.SunContribution;

	// Add PointLights contribution
	for(uint i=0;i<NL3D_MAX_LIGHT_CONTRIBUTION;i++)
	{
		CPointLight		*pl= lc.PointLight[i];
		// End of List?
		if(!pl)
			break;

		// Sum with this light, weighted by AttFactor
		color= pl->getDiffuse();
		r+= color.R * lc.AttFactor[i];
		g+= color.G * lc.AttFactor[i];
		b+= color.B * lc.AttFactor[i];
	}

	// normalize
	r>>=8;
	g>>=8;
	b>>=8;

	// Don't take the MergedPointLight into consideration (should add to the diffuse part here, but rare case)

	diffuse.R= uint8(min(r, 255U));
	diffuse.G= uint8(min(g, 255U));
	diffuse.B= uint8(min(b, 255U));
}


// ***************************************************************************
void			CShadowMapManager::fillBlackBorder(IDriver *drv, uint numPassText, uint numTextW, uint numTextH, uint baseTextureSize)
{
	if(numPassText==0)
		return;

	// the number of lines that have all their column disp.
	uint	numFullLine= numPassText/numTextW;
	// for the last line not full, the number of column setuped
	uint	lastLineNumCol=  numPassText - (numFullLine*numTextW);
	// number of line including the last line if not empty
	uint	numTotalLine= numFullLine + (lastLineNumCol?1:0);

	// Compute how many quads to render
	uint	numHQuads= numTotalLine * 2;
	uint	numTotalCol;
	uint	numVQuads;
	if(numFullLine)
		numTotalCol= numTextW;
	else
		numTotalCol= lastLineNumCol;
	numVQuads= numTotalCol * 2;

	_FillQuads.setNumVertices((numVQuads + numHQuads)*4);

	// Fill HQuads.
	uint	i;
	for(i=0;i<numTotalLine;i++)
	{
		uint	w;
		if(i<numFullLine)
			w= numTextW*baseTextureSize;
		else
			w= lastLineNumCol*baseTextureSize;
		// bottom of text
		setBlackQuad(i*2+0, 0, i*baseTextureSize, w, 1);
		// top of text
		setBlackQuad(i*2+1, 0, (i+1)*baseTextureSize-1, w, 1);
	}

	// Fill VQuads;
	uint	baseId= numTotalLine*2;
	for(i=0;i<numTotalCol;i++)
	{
		uint	h;
		if(i<lastLineNumCol)
			h= numTotalLine*baseTextureSize;
		else
			h= numFullLine*baseTextureSize;
		// left of text
		setBlackQuad(baseId + i*2+0, i*baseTextureSize, 0, 1, h);
		// right of text
		setBlackQuad(baseId + i*2+1, (i+1)*baseTextureSize-1, 0, 1, h);
	}

	// Render Quads
	_FillMaterial.setColor(CRGBA(0,0,0,0));
	drv->activeVertexBuffer(_FillQuads);
	drv->renderRawQuads(_FillMaterial, 0, numHQuads+numVQuads);
}


// ***************************************************************************
void			CShadowMapManager::setBlackQuad(uint index, sint x, sint y, sint w, sint h)
{
	float	x0= (float)x;
	float	y0= (float)y;
	float	x1= (float)x+(float)w;
	float	y1= (float)y+(float)h;
	index*= 4;
	CVertexBufferReadWrite vba;
	_FillQuads.lock(vba);
	vba.setVertexCoord (index+0, CVector (x0, 0, y0));
	vba.setVertexCoord (index+1, CVector (x1, 0, y0));
	vba.setVertexCoord (index+2, CVector (x1, 0, y1));
	vba.setVertexCoord (index+3, CVector (x0, 0, y1));
}

// ***************************************************************************
void			CShadowMapManager::updateBlurTexture(IDriver &drv, uint w, uint h)
{
	w= max(w, 2U);
	h= max(h, 2U);
	// if same size than setup, quit
	if(_BlurTextureW==w && _BlurTextureH==h)
		return;

	// release old SmartPtr
	uint i, j;
	for (i=0; i<2; i++)
	{
		_BlurMaterial[i].setTexture(0, NULL);
		_BlurMaterial[i].setTexture(1, NULL);
		_BlurMaterial[i].setTexture(2, NULL);
		_BlurMaterial[i].setTexture(3, NULL);
	}
	_BlurTexture[0]= NULL;
	_BlurTexture[1]= NULL;
	_BlurTextureW= w;
	_BlurTextureH= h;
	// NB: the format must be RGBA; else slow copyFrameBufferToTexture()
	for (i=0; i<2; i++)
	{
		uint8	*tmpMem= new uint8[4*_BlurTextureW*_BlurTextureH];
		_BlurTexture[i] = new CTextureMem (tmpMem, 4*_BlurTextureW*_BlurTextureH, true, false, _BlurTextureW, _BlurTextureH);
		_BlurTexture[i]->setWrapS (ITexture::Clamp);
		_BlurTexture[i]->setWrapT (ITexture::Clamp);
		_BlurTexture[i]->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
		_BlurTexture[i]->generate();
		_BlurTexture[i]->setReleasable (false);
		_BlurTexture[i]->setRenderTarget (true);
		// For Texture Profiling
		_BlurTexture[i]->setTextureCategory(_TextureCategory);
	}

	uint maxNumCstLighted;
	uint maxNumCstUnlighted;
	drv.getNumPerStageConstant(maxNumCstLighted, maxNumCstUnlighted);

	// set to the material
	for (i=0; i<2; i++)
	{
		for (j=0; j<maxNumCstUnlighted; j++)
		{
			_BlurMaterial[i].setTexture(j, _BlurTexture[i]);
		}
	}

	// compute values for texturing
	_BlurTextureOOW= 1.f / _BlurTextureW;
	_BlurTextureOOH= 1.f / _BlurTextureH;
	// The Delta HalfPixel
	_BlurTextureD05W= 0.5f*_BlurTextureOOW;
	_BlurTextureD05H= 0.5f*_BlurTextureOOH;
}


// ***************************************************************************
void			CShadowMapManager::copyScreenToBlurTexture(IDriver *drv, uint numPassText, uint numTextW, uint numTextH, uint baseTextureSize)
{
	if(numPassText==0)
		return;

	// TODO_SHADOW: optim: split into 2 copy for less pixel draw on the last line? No because of OverHead?

	// number of line including the last line if not empty
//	uint	numTotalLine= (numPassText+numTextW-1)/numTextW;
	// number of column.
//	uint	numTotalCol= (numPassText<numTextW)?numPassText:numTextW;

	/* todo hulud shadows
	drv->copyFrameBufferToTexture(_BlurTexture, 0, 0, 0, 0, 0, numTotalCol*baseTextureSize, numTotalLine*baseTextureSize); */
}

// ***************************************************************************
void			CShadowMapManager::applyFakeGaussianBlur(IDriver *drv, uint numPassText, uint numTextW, uint numTextH, uint baseTextureSize, uint blurSource)
{
	if(numPassText==0)
		return;

	// the number of lines that have all their column disp.
	uint	numFullLine= numPassText/numTextW;
	// for the last line not full, the number of column setuped
	uint	lastLineNumCol=  numPassText - (numFullLine*numTextW);

	// Split into 2 quads. one for the first full lines, and one for the last not full line.
	uint	index= 0;
	if(numFullLine)
		setBlurQuadFakeGaussian(index++, 0, 0, numTextW*baseTextureSize, numFullLine*baseTextureSize);
	if(lastLineNumCol)
		setBlurQuadFakeGaussian(index++, 0, numFullLine*baseTextureSize, lastLineNumCol*baseTextureSize, baseTextureSize);

	// render
	drv->activeVertexBuffer(_BlurQuads);
	drv->renderRawQuads(_BlurMaterial[blurSource], 0, index);
}


// ***************************************************************************
void			CShadowMapManager::setBlurQuadFakeGaussian(uint index, sint x, sint y, sint w, sint h)
{
	float	x0= (float)x;
	float	y0= (float)y;
	float	x1= (float)x+(float)w;
	float	y1= (float)y+(float)h;
	float	u0= x0*_BlurTextureOOW;
	float	v0= y0*_BlurTextureOOH;
	float	u1= x1*_BlurTextureOOW;
	float	v1= y1*_BlurTextureOOH;
	index*= 4;

	// NB: the order of the Delta (--,++,-+,+-) is made so it works well with 2,3 or 4 texture support.

	// vertex 0
	CVertexBufferReadWrite vba;
	_BlurQuads.lock(vba);
	vba.setVertexCoord (index+0, CVector (x0, 0, y0));
	vba.setTexCoord(index+0, 0, u0-_BlurTextureD05W, v0-_BlurTextureD05H);
	vba.setTexCoord(index+0, 1, u0+_BlurTextureD05W, v0+_BlurTextureD05H);
	vba.setTexCoord(index+0, 2, u0-_BlurTextureD05W, v0+_BlurTextureD05H);
	vba.setTexCoord(index+0, 3, u0+_BlurTextureD05W, v0-_BlurTextureD05H);
	// vertex 1
	vba.setVertexCoord (index+1, CVector (x1, 0, y0));
	vba.setTexCoord(index+1, 0, u1-_BlurTextureD05W, v0-_BlurTextureD05H);
	vba.setTexCoord(index+1, 1, u1+_BlurTextureD05W, v0+_BlurTextureD05H);
	vba.setTexCoord(index+1, 2, u1-_BlurTextureD05W, v0+_BlurTextureD05H);
	vba.setTexCoord(index+1, 3, u1+_BlurTextureD05W, v0-_BlurTextureD05H);
	// vertex 2
	vba.setVertexCoord (index+2, CVector (x1, 0, y1));
	vba.setTexCoord(index+2, 0, u1-_BlurTextureD05W, v1-_BlurTextureD05H);
	vba.setTexCoord(index+2, 1, u1+_BlurTextureD05W, v1+_BlurTextureD05H);
	vba.setTexCoord(index+2, 2, u1-_BlurTextureD05W, v1+_BlurTextureD05H);
	vba.setTexCoord(index+2, 3, u1+_BlurTextureD05W, v1-_BlurTextureD05H);
	// vertex 3
	vba.setVertexCoord (index+3, CVector (x0, 0, y1));
	vba.setTexCoord(index+3, 0, u0-_BlurTextureD05W, v1-_BlurTextureD05H);
	vba.setTexCoord(index+3, 1, u0+_BlurTextureD05W, v1+_BlurTextureD05H);
	vba.setTexCoord(index+3, 2, u0-_BlurTextureD05W, v1+_BlurTextureD05H);
	vba.setTexCoord(index+3, 3, u0+_BlurTextureD05W, v1-_BlurTextureD05H);
}


// ***************************************************************************
struct	CShadowMapSort
{
	CTransform		*Caster;
	float			Weight;

	bool		operator<(const CShadowMapSort &o) const
	{
		return Weight<o.Weight;
	}
};

// ***************************************************************************
void			CShadowMapManager::selectShadowMapsToGenerate(CScene *scene)
{
	// TODO: Scene option.
	const uint		maxPerFrame= 8;
	const float		minCamDist= 10;
	const CVector	&camPos= scene->getRenderTrav().CamPos;
	uint			i;

	// **** Clear first
	clearGenerateShadowCasters();

	// If the scene filter skeleton render, suppose no generation at all. Ugly.
	if(! (scene->getFilterRenderFlags() & UScene::FilterSkeleton) )
		return;

	// **** Select
	// For all ShadowCaster inserted
	static vector<CShadowMapSort>		sortList;
	sortList.clear();
	sortList.reserve(_ShadowCasters.size());
	for(i=0;i<_ShadowCasters.size();i++)
	{
		CTransform	*caster= _ShadowCasters[i];
		/* If the shadowMap exist, and if not totaly faded
			NB: take FinalFade here because if 1, it won't be rendered in renderProject()
			so don't really need to update (useful for update reason, but LastGenerationFrame do the job)
		*/
		if(caster->getShadowMap() && caster->getShadowMap()->getFinalFade()<1 )
		{
			CShadowMapSort		sms;
			sms.Caster= caster;
			// The Weight is the positive delta of frame
			sms.Weight= (float)(scene->getNumRender() - caster->getShadowMap()->LastGenerationFrame);
			// Modulated by Caster Distance from Camera.
			float	distToCam= (caster->getWorldMatrix().getPos() - camPos).norm();
			distToCam= max(distToCam, minCamDist);
			// The farthest, the less important
			sms.Weight/= distToCam;

			// Append
			sortList.push_back(sms);
		}
	}

	// Sort increasing
	sort(sortList.begin(), sortList.end());

	// Select the best
	uint	numSel= min((uint)sortList.size(), maxPerFrame);
	_GenerateShadowCasters.resize(numSel);
	for(i= 0;i<numSel;i++)
	{
		_GenerateShadowCasters[i]= sortList[sortList.size()-1-i].Caster;
	}

	// **** Flag selecteds
	// For All selected models, indicate that they will generate shadowMap for this Frame.
	for(i=0;i<_GenerateShadowCasters.size();i++)
	{
		_GenerateShadowCasters[i]->setGeneratingShadowMap(true);
	}
}


// ***************************************************************************
void			CShadowMapManager::clearAllShadowCasters()
{
	_ShadowReceiverGrid.clear();
	_ShadowCasters.clear();
	clearGenerateShadowCasters();
}


// ***************************************************************************
void			CShadowMapManager::clearGenerateShadowCasters()
{
	// Reset first each flag of all models
	for(uint i=0;i<_GenerateShadowCasters.size();i++)
	{
		_GenerateShadowCasters[i]->setGeneratingShadowMap(false);
	}

	_GenerateShadowCasters.clear();
}

// ***************************************************************************
ITexture		*CShadowMapManager::allocateTexture(uint textSize)
{
	nlassert( isPowerOf2(textSize) );

	// **** First, find a free texture already allocated.
	if(!_FreeShadowTextures.empty())
	{
		ITexture	*freeText= _FreeShadowTextures.back()->second;
		// If Ok for the size.
		if(freeText->getWidth() == textSize)
		{
			// take this texture => no more free.
			_FreeShadowTextures.pop_back();
			return freeText;
		}
		// else, suppose that we still take this slot.
		else
		{
			// but since bad size, delete this slot from the map and create a new one (below)
			_ShadowTextureMap.erase(_FreeShadowTextures.back());
			// no more valid it
			_FreeShadowTextures.pop_back();
		}
	}

	// **** Else Allocate new one.
	// NB: the format must be RGBA; else slow copyFrameBufferToTexture()
	uint8	*tmpMem= new uint8[4*textSize*textSize];
	ITexture	*text;
	text = new CTextureMem (tmpMem, 4*textSize*textSize, true, false, textSize, textSize);
	text->setWrapS (ITexture::Clamp);
	text->setWrapT (ITexture::Clamp);
	text->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	text->generate();
	text->setReleasable (false);
	text->setRenderTarget (true);
	// For Texture Profiling
	text->setTextureCategory(_TextureCategory);

	// Setup in the map.
	_ShadowTextureMap[text]= text;

	return text;
}

// ***************************************************************************
void			CShadowMapManager::releaseTexture(ITexture *text)
{
	if(!text)
		return;

	ItTextureMap	it= _ShadowTextureMap.find(text);
	nlassert(it!=_ShadowTextureMap.end());

	// Don't release it, but insert in Free Space
	_FreeShadowTextures.push_back(it);
}

// ***************************************************************************
void			CShadowMapManager::garbageShadowTextures(CScene *scene)
{
	uint	defSize= scene->getShadowMapTextureSize();

	// For all Free Textures only, release the one that are no more of the wanted default ShadowMap Size.
	std::vector<ItTextureMap>::iterator		itVec= _FreeShadowTextures.begin();
	for(;itVec!=_FreeShadowTextures.end();)
	{
		if((*itVec)->second->getWidth() != defSize)
		{
			// release the map texture iterator
			_ShadowTextureMap.erase(*itVec);
			// release the Vector Free iterator.
			itVec= _FreeShadowTextures.erase(itVec);
		}
		else
		{
			itVec++;
		}
	}

	// For memory optimisation, allow only a small extra of Texture allocated.
	if(_FreeShadowTextures.size()>NL3D_SMM_MAX_FREETEXT)
	{
		// Release the extra texture (Hysteresis: divide by 2 the max wanted free to leave)
		uint	startToFree= NL3D_SMM_MAX_FREETEXT/2;
		for(uint i=startToFree;i<_FreeShadowTextures.size();i++)
		{
			// Free the texture entry.
			_ShadowTextureMap.erase(_FreeShadowTextures[i]);
		}
		// resize vector
		_FreeShadowTextures.resize(startToFree);
	}
}


} // NL3D

