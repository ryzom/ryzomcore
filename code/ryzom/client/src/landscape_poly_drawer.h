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

#ifndef CL_LANDSCAPE_POLY_DRAWER_H
#define CL_LANDSCAPE_POLY_DRAWER_H

// Misc
#include "nel/misc/aabbox.h"
#include "nel/misc/polygon.h"
#include "nel/misc/singleton.h"

// 3D
#include "nel/3d/index_buffer.h"
#include "nel/3d/vertex_buffer.h"


//-----------------------------------------------------------------------------------------------------------
//---------------------------------------- CInitStencil -----------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
// CInitStencil inherits from the abstract NEL class ILandscapePolyDrawingCallback, attribut of UScene.
// An instance of CInitStencil is initialized at the time of CLandscapePolyDrawer initialization
// and passed to current scene. Thus, when stencil operation must be modified in a NEL class,
// this instance is used with its callbacks without any NEL class knows CLandscapePolyDrawer class.
//-----------------------------------------------------------------------------------------------------------
class CInitStencil : public NL3D::ILandscapePolyDrawingCallback
{

public:

	// This instance is passed to current scene as its ILandscapePolyDrawingCallback instance.
	// (UScene::setLandscapePolyDrawingCallback is called)
	void init();

private:
	// from NL3D::ILandscapePolyDrawingCallback
	// This method is called before landscape render to modify stencil operation.
	// The eighth bit will be written with a 1 during next render. Thus we will differentiate stencil buffer parts
	// which will support Shadow Volume algorithm (landscape) and the other parts (veget...)
	virtual void beginPolyDrawing();

	// from NL3D::ILandscapePolyDrawingCallback
	// This method is called after landscape render and before veget render to modify back stencil operation.
	// The eighth bit will be again written with a 0 during next render.
	virtual void endPolyDrawing();
};


//-----------------------------------------------------------------------------------------------------------
//---------------------------------- CLandscapePolyDrawer ---------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
// CLandscapePolyDrawer class manages display of a polygonal zones group, thanks to a Robust Shadow Volume algorithm.
// To avoid intersection between a shadow volume and the frustum, the "Far" plane of this one is pushed back
// to infinity during shadow volume display.
// To a best robustness and to join depth range [depthMin, depthMax] used in display of main scene, an epsilon
// factor is calculated to transpose Z buffer values from [depthMin, 1] to [depthMin, depthMax]
// while in [depthMin, 1] depth range is used during shadow volume display.
//-----------------------------------------------------------------------------------------------------------
class CLandscapePolyDrawer : public NLMISC::CSingleton<CLandscapePolyDrawer>
{

public:

	// Constructor
	CLandscapePolyDrawer();

	// Destructor
	~CLandscapePolyDrawer();

	// At every frame, polygons list is intialized and filled with addPloy calls.
	void addPoly(const NLMISC::CPolygon2D &poly, const NLMISC::CRGBA & color, const NLMISC::CAABBox & bBox);

	// At every frame end, polygons list is released. This allows an easy management to dynamic destructions,
	// addition or modifications of polygons.
	void deletePolygons();

	// Compute bbox of enclosed patch of a landscape from a delimiting 2D polygon.
	static void computeBBoxFromPolygon(const NLMISC::CPolygon2D &poly2D, NLMISC::CAABBox &destBBox);

	// Called to initialize ILandscapePolyDrawingCallback instance of current scene with _InitStencil attribut.
	void  initLandscapePolyDrawingCallback();


private:

	// renderScene is called in main loop. It can called beginRenderLandscapePolyPart and renderLandscapePolyPart
	// methods.
	friend void renderScene();

	// Enable stencil test and initialize function and operation of stencil at the beginning of renderScene method,
	// before opaque render of canopy and main scene parts.
	// The eighth bit will be written with a 0 during next render to mark stencil buffer parts which will
	// support Shadow Volume algorithm.
	void beginRenderLandscapePolyPart();

	// Render polygons after opaque render of canopy and main scene parts and before transparency parts render.
	// ZFail algorithm is only applied to stencil buffer parts whose eighth bit is equal to 1
	// (avoid to color vegetation)
	void renderLandscapePolyPart();

	// Build vertex and index buffer of the associated polygon when addPloy is called.
	// Vertices coordinates aren't calculated immediately but in drawShadowVolume method,
	// because they are calculated in camera location.
	void buildShadowVolume(uint poly);

	// Render shadow volume of a polygon (color and depth buffers updates are disable).
	// This method is called twice  durong the both passes of ZFail algorithm.
	// Coordinates of shadow volume vertex buffer are calculated in first pass.
	void drawShadowVolume(uint poly, bool firstPass);

	// Render polygon by displaying a quad which covers all zone delimited by scissor test.
	// This quad is applied only to stencil buffer parts whose 7 last bits aren't equal to 0,
	// thus the polygon projection on lanscape is delimited.
	void drawPolygon(uint poly);

	// Setup an infinite frustum and a [0, 1] depth range  in a way Z Buffer values coincide
	// with those of main scene with the depth range of main scene.
	void infiniteFrustum();

	// Setup initial projection matrix and depth range
	void finiteFrustum();

	// Setup scissor delimitation for polygon
	void setScissor(uint polyId);


	// This callback allows to setup stencil operation before and after vegetables and shadow render,
	// in order to separate them of landscape stencil buffer values.
	CInitStencil *						_InitStencil;

	// Vectors of polygons and their respective barycenters, colors, bounding boxes,
	// vertex and index buffers.
	std::vector<NLMISC::CPolygon2D>		_Polygons;
	std::vector<NLMISC::CVector2f>		_Barycenters;
	std::vector<NLMISC::CRGBA>			_PolyColor;
	std::vector<NLMISC::CAABBox>		_BBoxes;

	std::vector<NL3D::CVertexBuffer>	_PolyVB;
	std::vector<NL3D::CIndexBuffer>		_PolyIB;

	// Quad used to display polygons on marked parts of stencil buffer.
	NLMISC::CQuad						_Shadow;

	// Initial frustum used before shadow volumes display and stored to reset old values at the end of this display.
	NL3D::CFrustum						_FiniteFrustum;

	// Max depth range used before shadow volumes display and stored to reset old values back at the end of this display.
	float								_MaxDepthRange;

	// View matrix used before shadow volumes display and stored to reset old values at the end of this display.
	NLMISC::CMatrix						_OldViewMatrix;
};

#endif
