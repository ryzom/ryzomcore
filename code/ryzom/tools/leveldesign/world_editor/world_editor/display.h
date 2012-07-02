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

#ifndef DISPLAY_H
#define DISPLAY_H

#include "nel/misc/smart_ptr.h"
#include "nel/misc/matrix.h"
#include "nel/misc/geom_ext.h"
#include <nel/3d/texture_file.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/material.h>
#include <nel/3d/text_context.h>
#include "world_editor_doc.h"


namespace NL3D
{
	class CFontManager;
	class CFontGenerator;
}


class CMainFrame;


// a texture to be used with CDisplay
// NB : virtual are there for plugin access (no lib included)
class CPrimTexture : public NLMISC::CRefCount
{
public:
	CPrimTexture() : _Width(0), _Height(0) {}
	virtual ~CPrimTexture() {}
	// Build texture from a file
	virtual void	buildFromFile(const std::string &filename);
	// Build from a CBitmap object (content is copied)
	virtual void    buildFromNLBitmap(const NLMISC::CBitmap &bm);
	// get width of texture. 'setFile' should have been called
	uint	getWidth() const { return _Width; }
	// get height of texture. 'setFile' should have been called
	uint	getHeight() const { return _Height; }
private:
	friend class CDisplay;	
	NLMISC::CSmartPtr<NL3D::ITexture>	  _Texture;
	uint								  _Width, _Height;
};

class CDisplay : public CView
{
	// number of primitives layers
	enum { NUM_PRIM_LAYER = 4 };


	DECLARE_DYNCREATE(CDisplay)

	CMainFrame *_MainFrame;

public:

	bool	Interactif;

	bool	_MouseLeftDown;
	bool	_MouseRightDown;
	bool	_MouseMidDown;
	bool	_MouseMoved;
	bool	_MouseMoving;
	CPoint	_LastMousePos;
	CPoint	_SelectionMin, _SelectionMax;
	bool	_ClickedSelected;
	
	// World infos
	float	_Factor;
	NLMISC::CVector _Offset;
	NLMISC::CVector	_InitViewMin, _InitViewMax;
	float	_CellSize;
	bool	_DisplayGrid;
	bool	_DisplayZone;
	// This is the inworld coordinate of the bounding square
	NLMISC::CVector	_CurViewMin, _CurViewMax;
	NLMISC::CVector	_CurPos;
	NLMISC::CVector	_OrigineZoom;
	sint32	_LastX, _LastY;

	bool _CtrlKeyDown;
	NLMISC::CRGBA _BackgroundColor;

	uint32	getWidth ()
	{
		return _WindowWidth;
	}
	uint32	getHeight ()
	{
		return _WindowHeight;
	}

	// Cursor
	void invalidateCursor ()
	{
		_ValidMouse = false;
	}
	void updateCursor ();

	BOOL PreTranslateMessage(MSG *pmsg);

private:
	bool	_ValidMouse;

	// pointers to store 2*2 collision textures
	NLMISC::CSmartPtr<NL3D::CTextureFile>	_collisionTexture[2*2];
	sint32	_lastXCollisionTextureMove;
	sint32	_lastYCollisionTextureMove;
	sint32	_lastXCollisionTexture[2*2];
	sint32	_lastYCollisionTexture[2*2];
	// pointers to store some collision texture in cache (really speed up small movement on map)
	struct TCollisionTextureCache
	{
		NLMISC::CSmartPtr<NL3D::CTextureFile>	Texture;
		sint32		PosX;
		sint32		PosY;
	};
	enum
	{
		// Each texture is 256*256 in 16 bits, 128Kb
		COLLISION_TEXTURE_CACHE_SIZE = 32		// 4Mo texture cache
	};
	std::list<TCollisionTextureCache>		_collisionTextureCache;
	void DrawCollisionTexture(sint32 count, float x1, float y1);
		
	// Pick variable
private:
	uint32	_WindowWidth;
	uint32	_WindowHeight;

	// Last id of the picked primitive
	uint	_LastPickedPrimitiveId;

	// Last primitives picked
	std::vector<CDatabaseLocatorPointer>	_LastPickedPrimitive;

	// Draw the pick rect
	bool	_DrawSelection;
	void	drawSelection ();

	// Select the primtives
	void	selectPrimitives (const std::vector<class CDatabaseLocatorPointer> &result, bool addSelection);

	// \name Transform

	// Current transform action
	class IActionInteractive	*_TranformAction;

	// Mouse zoom ?
	enum TMouseMoveMode
	{
		Nothing,
		DrawTiles,			// In mode 0 : left click in primitive in tile mode
		RemoveTiles,		// In mode 0 : right click in primitive in tile mode
		TransformPrimitive,	// In mode 1 : left click in primitive mode
		MenuPrimitive,		// In mode 1 : right click in primitive mode
		DrawTransition,		// In mode 2 : left click in transition mode
		PluginMove,			// In mode 3 : left / right click in plugin mode
		ZoomView,			// All mode  : shift + left click
		DragView,			// All mode  : middle click
	};
	TMouseMoveMode	_MouseMode;

public:
	// Get the average position of selected items
	bool getSelectedAverage (NLMISC::CVector &average);
	
	// Pick methods with a point
	static bool	pickPoint (const NLMISC::CVector &pickPos, const NLMISC::CVector &v0, const NLMISC::CVector &v1, float bias);
	static bool	pickPoint (const NLMISC::CVector &pickPos, const NLMISC::CVector &point, float bias);

	// Pick some primitives with a point
	bool pickPoint (const NLMISC::CVector &pickPos, std::vector<class CDatabaseLocatorPointer> &result, bool pickOnlySelected);

	// Pick a primitive edge from selected primitive. Return the locator on the first edge vertex.
	bool pickEdgePoint (const NLMISC::CVector &pickPos, std::vector<CDatabaseLocatorPointer> &result);

	// Pick methods with a rect
	static bool	pickRect (const NLMISC::CVector &pickPosMin, const NLMISC::CVector &pickPosMax, const NLMISC::CVector &v0, const NLMISC::CVector &v1);
	static bool	pickRect (const NLMISC::CVector &pickPosMin, const NLMISC::CVector &pickPosMax, const NLMISC::CVector &point);

	// Pick some primitives  with a rect
	bool pickRect (const NLMISC::CVector &pickPosMin, const NLMISC::CVector &pickPosMax, std::vector<class CDatabaseLocatorPointer> &result, bool pickOnlyShown);

	// Set the display region
	void setDisplayRegion (const NLMISC::CVector &min, const NLMISC::CVector &max);

	// center the display on a position (keep the same scale)
	void setDisplayCenter (const NLMISC::CVector &center, bool zoom);

private:

	// Proxy methods
	void	setupWorldMatrixAndFrustum ();
	void	initRenderProxy ();
	void	flushRenderProxy ();
public:

	// HOTSPOTS
	enum THotSpot 
	{
		BottomLeft=0,
		MiddleLeft, 
		TopLeft,
		MiddleBottom, 
		MiddleMiddle, 
		MiddleTop, 
		BottomRight, 
		MiddleRight,
		TopRight,
		HotSpotCount
	};


	// CONVERSIONS
	virtual void	pixelToWorld (NLMISC::CVector &pixels);
	virtual void	pixelVectorToWorld (NLMISC::CVector &pixels);
	// convert world unit to pixel units (integer precision)
	virtual void	worldToPixel (NLMISC::CVector &pixels);	
	// convert world unit to pixel units (float precision)
	virtual void	worldToFloatPixel(NLMISC::CVector &pixels);	
	// convert world unit to pixel units for a vector (integer precision)
	virtual void	worldVectorToPixelVector(NLMISC::CVector &pixels);
	// convert world unit to pixel units for a vector (float precision)
	virtual void	worldVectorToFloatPixelVector(NLMISC::CVector &pixels);
	virtual bool	isClipped (const NLLIGO::CPrimVector *pVec, uint32 nNbVec);

	// UNTEXTURED PRIMITIVES

	virtual void	lineRenderProxy (NLMISC::CRGBA color, const NLMISC::CVector& pos0, const NLMISC::CVector& pos1, uint primitiveLayer);
	virtual void	triRenderProxy (NLMISC::CRGBA color, const NLMISC::CVector& pos0, const NLMISC::CVector& pos1, const NLMISC::CVector& pos2, uint primitiveLayer);
	virtual void	pointRenderProxy (NLMISC::CRGBA color, const NLMISC::CVector& pos0);
	virtual void    primitiveRenderProxy(const NLLIGO::IPrimitive &primitive);

	// TEXT		
	// NB : rendering is done instantly, it is not buffered & delayed to the next flush as with the xxxxRenderProxy functions
	// NB : coordinates are expressed in pixels
	virtual void    print(const ucstring &text, float x, float y, uint32 fontSize, NLMISC::CRGBA color, THotSpot hotspot);
	virtual void    print(const std::string &text, float x, float y, uint32 fontSize, NLMISC::CRGBA color, THotSpot hotspot);
	
	
	
	// TEXTURED PRIMITIVES

	// TODO nico : for now, using several textures is not convenient, because one has to call 'flush' before changing the texture, which is slow,
	//             This is not a good idea if a custom primitive displayer is used (because there's potentially one flush per primitive)
	//             intended usage is at post render time (in the 'postRender' method of plugins), in that way :
	//             call 'flush'
	//             set the wanted texture
	//             draw all quads with that texture
	//             and repeat for each new texture
	//             ideally, all quad texture should be packed in another bigger texture to avoid too much texture switching & flushs
	//           
	//             NB : no need to call 'flush' in the end, because the framework does it.
	//             NB : because of primitive layers, up to NUM_PRIM_LAYER textures can be set at once

	

	// Assign a texture to a primitive layer. Only the last set texture is used at render time, and affect all primitives of that layer
	virtual void	setLayerTexture(uint primitiveLayer, CPrimTexture *tex);
	// Draw a textured quad.
	virtual void    texQuadRenderProxy(const NLMISC::CQuadColorUV &quad, uint primitiveLayer);
	// flush render. Use it before changing texture for a layer
	virtual void    flush();
private:

	// Draw a primitive
	void	drawPrimitive (const NLLIGO::IPrimitive *primitive, bool forceDefaultDisplay = false);
	float	intToWorld (uint value) const;	
	void	drawLink (const NLLIGO::IPrimitive *first, const NLLIGO::IPrimitive *second);



	// Display meter grid points
	void	displayPoints ();
	static bool	isInTriangleOrEdge(	double x, double y, 
								double xt1, double yt1, 
								double xt2, double yt2, 
								double xt3, double yt3 );

	// Proxy primitive render
	NLMISC::CMatrix					_View2World;
	NLMISC::CMatrix					_World2View;
	NL3D::CVertexBuffer				_ProxyVB[NUM_PRIM_LAYER];
	NL3D::CVertexBufferReadWrite	_ProxyVBAccess[NUM_PRIM_LAYER];
	NL3D::CVertexBuffer				_ProxyVBPoints;
	NL3D::CVertexBufferReadWrite	_ProxyVBPointsAccess;
	NL3D::CVertexBuffer				_ProxyVBTexQuads[NUM_PRIM_LAYER];
	NL3D::CVertexBufferReadWrite	_ProxyVBTexQuadsAccess[NUM_PRIM_LAYER];
	NL3D::CIndexBuffer				_ProxyPBTri[NUM_PRIM_LAYER];
	NL3D::CIndexBufferReadWrite		_ProxyPBTriAccess[NUM_PRIM_LAYER];
	NL3D::CIndexBuffer				_ProxyPBLine[NUM_PRIM_LAYER];
	NL3D::CIndexBufferReadWrite		_ProxyPBLineAccess[NUM_PRIM_LAYER];
	NL3D::CMaterial					_ProxyMaterial;
	NLMISC::CRefPtr<CPrimTexture>   _ProxyTexture[NUM_PRIM_LAYER];

	
	NL3D::CFontManager				*_FontManager;
	NL3D::CTextContext				*_TextContext;

public:

	CDisplay();
	~CDisplay();
	
	void init (CMainFrame *pMF); // pMF For update of the statusbar
	void setCellSize (float size);

	void setBackgroundColor (NLMISC::CRGBA &col);
	NLMISC::CRGBA getBackgroundColor ();

	void setDisplayGrid (bool bDisp);
	bool getDisplayGrid ();
	void calcCurView();
	void setFactor (float newFactor, bool follow, const NLMISC::CVector &position);

	NLMISC::CVector convertToWorld(CPoint &p);

	void renderSelection ();
	void renderPluginPosition();

	virtual void OnDraw (CDC* pDC);

	// Get interactif texts
	bool getActionText (std::string &text);
	bool getActionHelp (std::string &text);

private:

	// Mouse handlers
	enum TButton { Left, Middle, Right };

	void buttonDown (TButton button, UINT nFlags, CPoint point);
	void updateMouseMode (UINT nFlags);


	afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy ();

	afx_msg BOOL OnMouseWheel	(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnMButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk (UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove	(UINT nFlags, CPoint point);
	afx_msg void OnChar (UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp (UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd (CDC *pC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()

private:

	//void displayGrid();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisplay)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
};

CDisplay *getDisplay ();

#endif // DISPLAY_H
/////////////////////////////////////////////////////////////////////////////
