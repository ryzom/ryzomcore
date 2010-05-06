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

#ifndef NL_BLOOM_EFFECT_H
#define NL_BLOOM_EFFECT_H

// Misc
#include "nel/misc/singleton.h"
#include "nel/misc/vector_2f.h"

// 3D
#include "nel/3d/texture.h"

//-----------------------------------------------------------------------------------------------------------
//---------------------------------------- CBloomEffect -----------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
// CBloomEffect class apply a bloom effect on all scene appart from interfaces. All scene is rendered in a
// render target (a Frame Buffer Object on OpengL, the normal back buffer in Direct3D) which is stretched
// in a 256*256 another render target.
// We apply a horizontal blur on this 256*256 render target, then a vertical blur on the result of this first pass.
// The final blured render target is blend with the initial render target of scene, with a dest + src - dest*src
// blend operation.
//-----------------------------------------------------------------------------------------------------------
class CBloomEffect : public NLMISC::CSingleton<CBloomEffect>
{
public:

	/// Constructor
	CBloomEffect();

	// Destructor
	~CBloomEffect();

	// Called after the Driver initialization to indicate if OpenGL or Direct3D is used.
	// They are some differences in bloom algorithm in function of this API choice.
	// If bloom effect is activated and supported, private method init() is called to initialize
	// textures and materials.
	void init(bool initBloomEffect);

	// Called at the beginning of renderAll method in the main loop, if window has been resized,
	// reinitialize bloom textures according to new window size.
	// The bloom texture (_InitText attribute) which is used as render target during scene render
	// is reinitialized with window dimensions.
	// If window size exceeds 256*256 the textures used to apply blur are reinitialized with
	// 256*256 size. If a dimension is less than 256, the texture is initialized with the nearer
	// power of 2, lower than this window dimension.
	void initBloom();

	// Called at the end of renderAll method in the main loop, recover stretched texture, apply
	// the both blur passes, and the blending operation between initial render target and the blured one.
	void endBloom();

	// In OpenGL, the use of FBO implies that Z buffer values of scene render have been stored in
	// a depth render target. Then, to display 3D interfaces, we must display them in the same FBO,
	// to keep correct Z tests.
	// This method is called at the end of interfaces display in the main loop, to display final render target
	// (with added interfaces) in the color frame buffer.
	// NB : In Direct3D, the final render target is displayed at the end of endBloom call.
	void endInterfacesDisplayBloom();

private:

	// Initialize textures and materials.
	void init();

	// Initialize a bloom texture with new dimensions.
	void initTexture(NLMISC::CSmartPtr<NL3D::ITexture> & tex, bool isMode2D, uint32 width, uint32 height);

	// Called in endBloom method to build a blured texture. Two passes (then two calls)
	// are necessary : horizontal and vertical.
	// For the first pass, blur is applied horizontally to stretched texture _BlurFinalTex and recover in
	// _BlurHorizontalTex render target texure.
	// For the second pass, blur is applied vertically to precedent _BlurHorizontalTex texture and recover
	// in _BlurFinalTex render target texture.
	// For each pass, thanks to a vertex program, first texture is displayed with several little decals
	// in order to obtain in the render target texture a mix of color of a texel and its neighbored texels
	// on the axe of the pass.
	void doBlur(bool horizontalBlur);

	// Called in endBloom method after the both doBlur passes. Apply blend operation between intial render target
	// texture _InitText of scene and the blured texture _BlurFinalTex.
	void applyBlur();


	// render target textures
	// used to display scene
	NLMISC::CSmartPtr<NL3D::ITexture>  _InitText;
	// used as stretched texture from _InitText, as displayed texture in first blur pass,
	// and as render target in second blur pass.
	NLMISC::CSmartPtr<NL3D::ITexture>  _BlurFinalTex;
	// used as rander target in first blur pass, and as displayed texture on second blur pass.
	NLMISC::CSmartPtr<NL3D::ITexture>  _BlurHorizontalTex;


	// materials
	// used to display first texture in doBlur passes.
	NL3D::UMaterial		_BlurMat;
	// used to display final render target texture in endInterfacesDisplayBloom call (OpenGL).
	NL3D::UMaterial		_DisplayInitMat;
	// used to blend intial scene render target texture and blur texture according to a
	// dest+src - dest*src blend operation.
	NL3D::UMaterial		_DisplayBlurMat;
	// used to blend intial scene render target texture and blur texture according to a
	// dest+src - dest*src blend operation, with a square stage operation.
	NL3D::UMaterial		_DisplaySquareBlurMat;

	// quads
	NLMISC::CQuadUV		_BlurQuad;
	NLMISC::CQuadUV		_DisplayQuad;

	// openGL or Direct3D?
	bool				_InitBloomEffect;

	// textures and materials already initialized?
	bool				_Init;

	// current window dimensions
	uint32				_WndWidth;
	uint32				_WndHeight;

	// current blur texture dimensions
	uint32				_BlurWidth;
	uint32				_BlurHeight;
};

#endif
