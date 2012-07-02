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

#include "stdafx.h"
#include "display.h"
#include "world_editor_doc.h"
#include "world_editor.h"
#include "editor_primitive.h"
#include "tools_logic.h"
#include <nel/3d/font_manager.h>
#include <nel/3d/computed_string.h>

#include "main_frm.h"
#include "action.h"
#include "pacs.h"

#include "nel/misc/path.h"
#include "nel/misc/bitmap.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;
using namespace NLLIGO;


extern bool	DontUse3D;

// Color definition

#define DEFAULT_ARROW_PRIMITIVE_COLOR (CRGBA (128, 0, 0, 255))
#define DEFAULT_DOT_PRIMITIVE_COLOR (CRGBA (0, 0, 0, 255))
#define SELECTED_PRIMITIVE_COLOR (CRGBA (255, 255, 255, 128))
#define SELECTED_ARROW_PRIMITIVE_COLOR (CRGBA (255, 0, 0, 255))
#define SELECTED_DOT_PRIMITIVE_COLOR (CRGBA (255, 255, 0, 255))
#define SELECTED_DOT_COLOR (CRGBA (255, 0, 0, 255))
#define LINK_LINE_COLOR (CRGBA (255, 255, 255, 128))

// Size definition

#define POINT_ARROW_LINE_SIZE (20.f)
#define POINT_ARROW_HEAD_SIZE (8.f)
#define POINT_DOT_SIZE (3.f)

// Circle 

#define CIRCLE_SEGMENT_SIZE 20
#define CIRCLE_MIN_SEGMENT_COUNT 8

// Pick definition
#define PICK_DOT_SIZE (10)
#define PICK_PATH_SIZE (10)
#define LINK_LINE_SIZE (2)

// Rotate per pixel
#define ROTATE_PER_PIXEL (2.f*(float)Pi/640.f)

// Scale per pixel
#define SCALE_PER_PIXEL (4.f/640.f)

// ***************************************************************************

/*

Primitive mouse state machine
-----------------------------

LClickDown:

	CLICKED_ON_SELECTED = false
	if subobject mode and add point mode
	{
		add a point in selected primitives
		select the new points
		active move transformation mode
	}
	else
	{
		if left/right click on primitives
		{
			if left click + ctrl + click on primitive
			{
				choose a clicked primitive
				toggle its selection state
			}
			else
			{
				if at least one clicked primitive is selected
				{
					CLICKED_ON_SELECTED = true
				}
				else
				{
					unselect all
					choose the first clicked primitive
					select it
				}
			}
		}

		if left click and the mouse is over a selected primitive
		{
			enable selected tranformation mode
		}
	}
	
	if left click and no transformation mode actived
	{
		if in subobject mode and over a selected primitive edge
		{
			add a point on this edge
			select the point
			active move transformation mode
		}
	}

	if left click and no transformation mode actived
	{
		disable tranformation mode
		show rect selection
	}

	if right click
	{
		show menu
	}

LClickUp:

	if the mouse moved
	{
		if tranformation mode is enabled
		{
			make the tranformation
		}
		else
		{
			hide rect selection
			make a rect selection
			if nothing selected in the rect and CTRL not pushed
				unselect all
		}
	}
	else
	{
		if tranformation mode is enabled
		{
			hide rect selection
		}

		if CLICKED_ON_SELECTED == true
		{
			choose a not selected clicked primitive 
			unselect all
			select it
		}
		
		if no primitive under the mouse and CTRL not pushed
		{
			unselect all
		}
	}

*/







BEGIN_MESSAGE_MAP (CDisplay, CView)
	//{{AFX_MSG_MAP(CDisplay)
	ON_WM_CREATE ()
	ON_WM_SIZE ()
	ON_WM_PAINT ()
	ON_WM_MOUSEWHEEL ()
	ON_WM_MBUTTONDOWN ()
	ON_WM_LBUTTONDOWN ()
	// ON_WM_LBUTTONDBLCLK ()
	ON_WM_RBUTTONDOWN ()
	ON_WM_MBUTTONUP ()
	ON_WM_LBUTTONUP ()
	ON_WM_RBUTTONUP ()
	ON_WM_MOUSEMOVE ()
	ON_WM_CHAR ()
	ON_WM_KEYUP ()
	ON_WM_DESTROY ()
	ON_WM_ERASEBKGND ()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
IMPLEMENT_DYNCREATE (CDisplay, CView)

// ***************************************************************************
CDisplay::CDisplay ()
{
	_MouseMode = Nothing;
	_MouseRightDown = false;
	_MouseLeftDown = false;
	_MouseMidDown = false;
	_MouseMoved = false;
	_MouseMoving = false;
	_Factor = 1.0f;
	_Offset = CVector(0.0f, 0.0f, 0.0f);
	_DisplayGrid = true;
	_LastX = _LastY = -10000;
	_CtrlKeyDown = false;
	_MainFrame = NULL;
	_BackgroundColor = CRGBA(192,192,192,255);
	_LastPickedPrimitiveId = 0;
	_TranformAction = NULL;
	Interactif = true;
	invalidateCursor ();
	_DrawSelection = false;
	for (int i=0;i < 4;++i)
	{
		_collisionTexture[i] = NULL;
		_lastXCollisionTexture[i] = 100;
		_lastYCollisionTexture[i] = 100;
	}
	_lastXCollisionTextureMove = 100;
	_lastYCollisionTextureMove = 100;

	_FontManager = new NL3D::CFontManager;
	_FontManager->setMaxMemory(2000000);	
	_TextContext = new NL3D::CTextContext;	
}

CDisplay::~CDisplay()
{	
	delete _TextContext;
	delete _FontManager;
}

// ***************************************************************************
void CDisplay::print(const ucstring &text, float x, float y, uint32 fontSize, NLMISC::CRGBA color, THotSpot hotspot)
{
	_TextContext->setColor(color);
	_TextContext->setFontSize(fontSize);
	_TextContext->setHotSpot((NL3D::CComputedString::THotSpot) hotspot);
	uint32 width, height;
	nlassert(CNELU::Driver);
	CNELU::Driver->getWindowSize(width, height);
	_TextContext->printAt(x / (float) width, y / (float) height, text);
}

// ***************************************************************************
void CDisplay::print(const std::string &text, float x, float y, uint32 fontSize, NLMISC::CRGBA color, THotSpot hotspot)
{
	print(ucstring(text), x, y, fontSize, color, hotspot);
}


// ***************************************************************************
void CDisplay::init (CMainFrame *pMF)
{
	_MainFrame = pMF;

	if (!DontUse3D)
	{
		registerSerial3d ();
		CScene::registerBasics ();
		init3d ();

		try
		{
			//CNELU::init (512, 512, CViewport(), 32, true, this->m_hWnd);
			//NL3D::registerSerial3d();
			CNELU::initDriver (512, 512, 32, true, this->m_hWnd);
			CNELU::initScene (CViewport());

			CNELU::Driver->forceDXTCCompression (true);

			// setMatrixMode2D11
			CFrustum	f(0.0f,1.0f,0.0f,1.0f,-1.0f,1.0f,false);
			CVector		I(1,0,0);
			CVector		J(0,0,1);
			CVector		K(0,-1,0);
			CMatrix		ViewMatrix, ModelMatrix;
			ViewMatrix.identity ();
			ViewMatrix.setRot (I,J,K, true);
			ModelMatrix.identity ();

			CNELU::Driver->setFrustum (f.Left, f.Right, f.Bottom, f.Top, f.Near, f.Far, f.Perspective);
			CNELU::Driver->setupViewMatrix (ViewMatrix);
			CNELU::Driver->setupModelMatrix (ModelMatrix);
			CNELU::Driver->activate ();
			CNELU::clearBuffers ();
			CNELU::swapBuffers ();

			// init the text context			
			_TextContext->init(CNELU::Driver, _FontManager);
			_TextContext->setFontGenerator(NLMISC::CPath::getWindowsDirectory() + "Fonts\\arial.ttf");
			_TextContext->setKeep800x600Ratio(true);
			_TextContext->setShaded(true);
			_TextContext->setShadeColor(NLMISC::CRGBA::Black);
		}
		catch(...)
		{
			DontUse3D = true;
		}
	}

	SetCurrentDirectory (pMF->_ExeDir.c_str());
}

// ***************************************************************************
void CDisplay::setCellSize (float size)
{
	_CellSize = size;
	_InitViewMax = CVector (5.5f*_CellSize, 5.5f*_CellSize, 0.0f);
	_InitViewMin = CVector (-5.5f*_CellSize, -5.5f*_CellSize, 0.0f);
}

// ***************************************************************************
void CDisplay::setBackgroundColor (NLMISC::CRGBA &col)
{
	_BackgroundColor = col;
}

// ***************************************************************************
NLMISC::CRGBA CDisplay::getBackgroundColor ()
{
	return _BackgroundColor;
}

// ***************************************************************************
void CDisplay::setDisplayGrid (bool bDisp)
{
	_DisplayGrid = bDisp;
}

// ***************************************************************************
bool CDisplay::getDisplayGrid ()
{
	return _DisplayGrid;
}

// ***************************************************************************
void CDisplay::calcCurView()
{
	_CurViewMin = _InitViewMin * _Factor + _Offset;
	_CurViewMax = _InitViewMax * _Factor + _Offset;

	if (DontUse3D)
		return;

	// View to world matrix
	CNELU::Driver->getWindowSize (_WindowWidth, _WindowHeight);
	_View2World.identity ();
	if ((_WindowWidth > 0) && (_WindowHeight > 0) )
		_View2World.scale (CVector ((_CurViewMax.x - _CurViewMin.x)/(float)_WindowWidth, (_CurViewMax.y - _CurViewMin.y)/(float)_WindowHeight, 1));
	_View2World.setPos (_CurViewMin);

	// World to view matrix
	_World2View = _View2World;
	_World2View.invert ();

}

// ***************************************************************************
CVector CDisplay::convertToWorld (CPoint&p)
{
	CVector ret;
	::CRect rect;
	
	GetClientRect (rect);
	calcCurView ();
	ret.x = ((float)p.x) / ((float)rect.Width());
	ret.x = ret.x*(_CurViewMax.x-_CurViewMin.x) + _CurViewMin.x;
	ret.y = 1.0f - (((float)p.y) / ((float)rect.Height()));
	ret.y = ret.y*(_CurViewMax.y-_CurViewMin.y) + _CurViewMin.y;
	ret.z = 0.0f;

	return ret;
}

// ***************************************************************************

void CDisplay::renderPluginPosition()
{
	CVector p1, p2, p3, p4;
	float	size = 500;

	p1 = _MainFrame->_PositionControl;
	p1.x = (p1.x-_CurViewMin.x) / (_CurViewMax.x-_CurViewMin.x);
	p1.y = (p1.y-_CurViewMin.y) / (_CurViewMax.y-_CurViewMin.y);

	p2 = p3 = p4 = p1;

	p1.x -= size;
	p1.y -= size;
	p2.x += size;
	p2.y += size;
	p3.x -= size;
	p3.y += size;
	p4.x += size;
	p4.y -= size;

	if (DontUse3D)
		return;

	CDRU::drawLine (p1.x, p1.y, p2.x, p2.y, *CNELU::Driver, CRGBA(0,255,0,255));
	CDRU::drawLine (p3.x, p3.y, p4.x, p4.y, *CNELU::Driver, CRGBA(0,255,0,255));

}

// ***************************************************************************

void CDisplay::initRenderProxy ()
{
	// Init the material (RGBA + Alpha blend)
	_ProxyMaterial.initUnlit ();
	_ProxyMaterial.setSrcBlend(CMaterial::srcalpha);
	_ProxyMaterial.setDstBlend(CMaterial::invsrcalpha);
	_ProxyMaterial.setBlend (true);
	_ProxyMaterial.setColor (CRGBA(255, 255, 255, 192));
	_ProxyMaterial.setZFunc (CMaterial::always);
	_ProxyMaterial.setDoubleSided(true);

	// Init the vertex buffers
	for (uint i=0; i<NUM_PRIM_LAYER; i++)
	{
		_ProxyVB[i].setVertexFormat (CVertexBuffer::PositionFlag|CVertexBuffer::PrimaryColorFlag);
		_ProxyVB[i].setNumVertices (0);
		_ProxyVB[i].reserve (3000);
		_ProxyVB[i].lock (_ProxyVBAccess[i]);
		_ProxyPBLine[i].setNumIndexes (0);
		_ProxyPBTri[i].setNumIndexes (0);
		_ProxyPBLine[i].reserve (2*1000);
		_ProxyPBTri[i].reserve (3*1000);
		_ProxyPBTri[i].lock (_ProxyPBTriAccess[i]);
		_ProxyPBLine[i].lock (_ProxyPBLineAccess[i]);
	}
	_ProxyVBPoints.setVertexFormat (CVertexBuffer::PositionFlag|CVertexBuffer::PrimaryColorFlag);
	_ProxyVBPoints.setNumVertices (0);
	_ProxyVBPoints.reserve (1000);
	_ProxyVBPoints.lock (_ProxyVBPointsAccess);
	//
	for (uint i=0; i<NUM_PRIM_LAYER; i++)
	{
		_ProxyVBTexQuads[i].setVertexFormat (CVertexBuffer::PositionFlag|CVertexBuffer::PrimaryColorFlag|CVertexBuffer::TexCoord0Flag);
		_ProxyVBTexQuads[i].setNumVertices (0);
		_ProxyVBTexQuads[i].reserve (1000);
		_ProxyVBTexQuads[i].lock (_ProxyVBTexQuadsAccess[i]);
	}
}

// ***************************************************************************

void CDisplay::setupWorldMatrixAndFrustum ()
{
	// Flush the Vertex Buffer
	
	if (DontUse3D)
		return;

	CMatrix mtx, mtx2;

	CNELU::Driver->setupViewport (CViewport());

	// View to world matrix
	mtx2.identity ();
	mtx.identity ();
	mtx.scale (CVector (_CurViewMax.x - _CurViewMin.x, 1, _CurViewMax.y - _CurViewMin.y));
	mtx2.rotateX ((float)(-Pi/2.0));
	mtx2 *= mtx;
	mtx2.setPos (CVector (_CurViewMin.x, _CurViewMin.y, 0));

	// World to view matrix
	mtx2.invert ();
	CNELU::Driver->setupViewMatrix (mtx2);

	// Model matrix
	mtx.identity();
	CNELU::Driver->setupModelMatrix (mtx);

	// Frustrum
	CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);
}

// ***************************************************************************
void CDisplay::flushRenderProxy ()
{
	if (DontUse3D)
		return;

	// Setup matrix
	setupWorldMatrixAndFrustum ();	

	uint i;
	for (i=0; i<NUM_PRIM_LAYER; i++)
	{
		// Unlock accesses
		_ProxyVBAccess[i].unlock ();
		_ProxyPBLineAccess[i].unlock();
		_ProxyPBTriAccess[i].unlock();

		// Render primitives 0
		CNELU::Driver->activeVertexBuffer (_ProxyVB[i]);
		CNELU::Driver->activeIndexBuffer (_ProxyPBTri[i]);
		CNELU::Driver->renderTriangles (_ProxyMaterial, 0, _ProxyPBTri[i].getNumIndexes()/3);
		CNELU::Driver->activeIndexBuffer (_ProxyPBLine[i]);
		CNELU::Driver->renderLines (_ProxyMaterial, 0, _ProxyPBLine[i].getNumIndexes()/2);
	}

	// Render points
	_ProxyVBPointsAccess.unlock ();
	if (_ProxyVBPoints.getNumVertices())
	{
		CNELU::Driver->activeVertexBuffer (_ProxyVBPoints);
		CNELU::Driver->renderRawPoints (_ProxyMaterial, 0, _ProxyVBPoints.getNumVertices());
	}
			
	// Render textured quads
	for (i=0; i<NUM_PRIM_LAYER; i++)
	{		
		if (_ProxyTexture[i] && _ProxyTexture[i]->_Texture)
		{
			_ProxyMaterial.setTexture(0, _ProxyTexture[i]->_Texture);			
		}
		else
		{
			_ProxyMaterial.setTexture(0, NULL);
		}
		_ProxyVBTexQuadsAccess[i].unlock();
		CNELU::Driver->activeVertexBuffer(_ProxyVBTexQuads[i]);
		CNELU::Driver->renderRawQuads(_ProxyMaterial, 0, _ProxyVBTexQuads[i].getNumVertices() / 4);
	}
	_ProxyMaterial.setTexture(0, NULL);	
}

// ***************************************************************************

void CDisplay::lineRenderProxy (CRGBA color, const CVector& pos0, const CVector& pos1, uint primitive)
{
	nlassert(primitive < NUM_PRIM_LAYER);
	// Add two vertices
	CVector p0 = pos0;
	p0.z = 0;
	CVector p1 = pos1;
	p1.z = 0;
	uint first = _ProxyVB[primitive].getNumVertices ();
	if ((first+2) > _ProxyVB[primitive].capacity())
	{
		_ProxyVBAccess[primitive].unlock();
		_ProxyVB[primitive].reserve (first*2);
		_ProxyVB[primitive].lock (_ProxyVBAccess[primitive]);
	}
	_ProxyVB[primitive].setNumVertices (first + 2);
	_ProxyVBAccess[primitive].setVertexCoord(first, p0);
	_ProxyVBAccess[primitive].setVertexCoord(first+1, p1);
	_ProxyVBAccess[primitive].setColor(first, color);
	_ProxyVBAccess[primitive].setColor(first+1, color);

	uint firstIndex = _ProxyPBLine[primitive].getNumIndexes ();
	if ((firstIndex+2) > _ProxyPBLine[primitive].capacity())
	{
		_ProxyPBLineAccess[primitive].unlock();
		_ProxyPBLine[primitive].reserve (firstIndex*2);
		_ProxyPBLine[primitive].lock (_ProxyPBLineAccess[primitive]);
	}
	_ProxyPBLine[primitive].setNumIndexes (firstIndex+2);
	_ProxyPBLineAccess[primitive].setLine (firstIndex, first, first + 1);
}

// ***************************************************************************

void CDisplay::triRenderProxy (CRGBA color, const CVector& pos0, const CVector& pos1, const CVector& pos2, uint primitive)
{
	nlassert(primitive < NUM_PRIM_LAYER);
	// Add three vertices
	CVector p0 = pos0;
	p0.z = 0;
	CVector p1 = pos1;
	p1.z = 0;
	CVector p2 = pos2;
	p2.z = 0;
	uint first = _ProxyVB[primitive].getNumVertices ();
	if ((first+3) > _ProxyVB[primitive].capacity())
	{
		_ProxyVBAccess[primitive].unlock();
		_ProxyVB[primitive].reserve (first*2);
		_ProxyVB[primitive].lock (_ProxyVBAccess[primitive]);
	}
	_ProxyVB[primitive].setNumVertices (first + 3);
	_ProxyVBAccess[primitive].setVertexCoord(first, p0);
	_ProxyVBAccess[primitive].setVertexCoord(first+1, p1);
	_ProxyVBAccess[primitive].setVertexCoord(first+2, p2);
	_ProxyVBAccess[primitive].setColor(first, color);
	_ProxyVBAccess[primitive].setColor(first+1, color);
	_ProxyVBAccess[primitive].setColor(first+2, color);


	uint firstIndex = _ProxyPBTri[primitive].getNumIndexes ();
	if ((firstIndex+3) > _ProxyPBTri[primitive].capacity())
	{
		_ProxyPBTriAccess[primitive].unlock();
		_ProxyPBTri[primitive].reserve (firstIndex*2);
		_ProxyPBTri[primitive].lock (_ProxyPBTriAccess[primitive]);
	}
	_ProxyPBTri[primitive].setNumIndexes (firstIndex+3);
	_ProxyPBTriAccess[primitive].setTri (firstIndex, first, first + 1, first + 2);
}

// ***************************************************************************

void CDisplay::texQuadRenderProxy(const NLMISC::CQuadColorUV &quad, uint primLayer)
{	
	nlassert(primLayer < NUM_PRIM_LAYER);	
	NL3D::CVertexBuffer &vb = _ProxyVBTexQuads[primLayer];
	NL3D::CVertexBufferReadWrite &vba = _ProxyVBTexQuadsAccess[primLayer];
	uint first = vb.getNumVertices ();
	if ((first+4) > vb.capacity())
	{
		vba.unlock();
		vb.reserve (first*2);
		vb.lock (vba);
	}
	vb.setNumVertices (first + 4);
	vba.setVertexCoord(first, quad.V0);
	vba.setVertexCoord(first+1, quad.V1);
	vba.setVertexCoord(first+2, quad.V2);
	vba.setVertexCoord(first+3, quad.V3);
	vba.setColor(first, quad.Color0);
	vba.setColor(first+1, quad.Color1);
	vba.setColor(first+2, quad.Color2);
	vba.setColor(first+3, quad.Color3);
	vba.setTexCoord(first, 0, quad.Uv0);
	vba.setTexCoord(first + 1, 0, quad.Uv1);
	vba.setTexCoord(first + 2, 0, quad.Uv2);
	vba.setTexCoord(first + 3, 0, quad.Uv3);
	
}


// ***************************************************************************

void CDisplay::pointRenderProxy (CRGBA color, const CVector& pos0)
{	
	// Add three vertices
	CVector p0 = pos0;
	p0.z = 0;
	uint first = _ProxyVBPoints.getNumVertices ();
	if ((first+1) > _ProxyVBPoints.capacity())
	{
		_ProxyVBPointsAccess.unlock ();
		_ProxyVBPoints.reserve (first*2);
		_ProxyVBPoints.lock (_ProxyVBPointsAccess);
	}
	_ProxyVBPoints.setNumVertices (first + 1);
	_ProxyVBPointsAccess.setVertexCoord(first, p0);
	_ProxyVBPointsAccess.setColor(first, color);
}



// ***************************************************************************
void CDisplay::primitiveRenderProxy(const NLLIGO::IPrimitive &primitive)
{
	drawPrimitive(&primitive, true);
}


// ***************************************************************************
void CDisplay::flush()
{
	flushRenderProxy();
	initRenderProxy();
}


// ***************************************************************************

void CDisplay::pixelToWorld (CVector &pixels)
{
	pixels = _View2World * pixels;
}

void CDisplay::pixelVectorToWorld(NLMISC::CVector &pixels)
{
	pixels = _View2World.mulVector(pixels);
}


// ***************************************************************************

inline float CDisplay::intToWorld (uint value) const
{
	if (_WindowWidth == 0)
		return 0.0f;
	return (float)value * (_CurViewMax.x - _CurViewMin.x) / _WindowWidth;
}

// ***************************************************************************

inline void CDisplay::worldToPixel (CVector &pixels)
{
	pixels = _World2View * pixels;
	pixels.x = (float)(sint32)pixels.x;
	pixels.y = (float)(sint32)pixels.y;
	pixels.z = (float)(sint32)pixels.z;
}

// ***************************************************************************
void CDisplay::worldToFloatPixel(NLMISC::CVector &pixels)
{
	pixels = _World2View * pixels;
}

// ***************************************************************************

void CDisplay::worldVectorToPixelVector(NLMISC::CVector &pixels)
{
	pixels = _World2View.mulVector(pixels);
	pixels.x = (float)(sint32)pixels.x;
	pixels.y = (float)(sint32)pixels.y;
	pixels.z = (float)(sint32)pixels.z;
}

// ***************************************************************************

void CDisplay::worldVectorToFloatPixelVector(NLMISC::CVector &pixels)
{
	pixels = _World2View.mulVector(pixels);
}



// ***************************************************************************

bool CDisplay::isClipped (const CPrimVector *pVec, uint32 nNbVec)
{
	uint32 i;
	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].x > _CurViewMin.x)
			break;
	if (i == nNbVec)
		return true; // Entirely clipped

	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].x < _CurViewMax.x)
			break;
	if (i == nNbVec)
		return true;

	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].y > _CurViewMin.y)
			break;
	if (i == nNbVec)
		return true;

	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].y < _CurViewMax.y)
			break;
	if (i == nNbVec)
		return true;
	return false; // Not entirely clipped
}

// ***************************************************************************

bool CDisplay::isInTriangleOrEdge(	double x, double y, 
												double xt1, double yt1, 
												double xt2, double yt2, 
												double xt3, double yt3 )
{
	// Test vector T1X and T1T2
	double sign1 = ((xt2-xt1)*(y-yt1) - (yt2-yt1)*(x-xt1));
	// Test vector T2X and T2T3
	double sign2 = ((xt3-xt2)*(y-yt2) - (yt3-yt2)*(x-xt2));
	// Test vector T3X and T3T1
	double sign3 = ((xt1-xt3)*(y-yt3) - (yt1-yt3)*(x-xt3));
	if( (sign1 <= 0.0)&&(sign2 <= 0.0)&&(sign3 <= 0.0) )
		return true;
	if( (sign1 >= 0.0)&&(sign2 >= 0.0)&&(sign3 >= 0.0) )
		return true;
	return false;
}

// ***************************************************************************

void CDisplay::drawPrimitive (const NLLIGO::IPrimitive *primitive, bool forceDefaultDisplay)
{	

	// Configurations
	const std::vector<CPrimitiveConfigurations> &configurations = theApp.Config.getPrimitiveConfiguration();

	// Get the hide property
	bool hide = !isPrimitiveVisible (primitive);

	// Not hide ?
	if (!hide)
	{
		// Get the select property
		bool selected = getPrimitiveEditor(primitive)->getSelected();

		// Get a color parameter
		CRGBA mainColor;
		CRGBA arrowColor;
		CRGBA dotColor;
		CRGBA lineColor;
		
		// Is selected ?
		if (selected)
		{
			mainColor = SELECTED_PRIMITIVE_COLOR;
			arrowColor = SELECTED_ARROW_PRIMITIVE_COLOR;
			dotColor = SELECTED_DOT_PRIMITIVE_COLOR;
		}
		else
		{
			// Get the primitive color
			mainColor = theApp.Config.getPrimitiveColor (*primitive);
			arrowColor  = DEFAULT_ARROW_PRIMITIVE_COLOR;
			dotColor = DEFAULT_DOT_PRIMITIVE_COLOR;
			uint8 alpha = mainColor.A;

			// Look for the configuration
			sint search = 0;
			bool colorFound = false;
			while ((search = theApp.getActiveConfiguration (*primitive, search)) != -1)
			{
				// Configuration activated ?
				if (theApp.Configurations[search].Activated)
				{
					colorFound = true;
					mainColor = configurations[search].Color;
					break;
				}
				search++;
			}
			
			// try to get the primitive color ?
			if (!colorFound)
				primitive->getPropertyByName ("Color", mainColor);
			
			// Restore alpha
			mainColor.A = alpha;
		}

		// Line color
		lineColor = mainColor;
		lineColor.A = 255;

		// check if there is a plug in to draw this primitive
		string *className;
		primitive->getPropertyByName("class", className);
//		const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*className);

		std::map<std::string, IPrimitiveDisplayer*>::iterator it;
		if (!forceDefaultDisplay)
		{
			it = theApp.PrimitiveDisplayers.find(*className);
		}
		if (!forceDefaultDisplay && it != theApp.PrimitiveDisplayers.end())
		{
			const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (className->c_str());
			// use a plug in displayer
			IPrimitiveDisplayer::TRenderContext ctx;
			ctx.ShowDetail = _MainFrame->showDetails ();
			ctx.Selected = selected;
			ctx.MainColor = mainColor;
			ctx.ArrowColor = arrowColor;
			ctx.DotColor = dotColor;
			ctx.LineColor = lineColor;
			ctx.PrimitiveClass = primClass;
			ctx.Display = this;

			it->second->drawPrimitive(primitive, ctx);
		}
		else
		{
			// use internal drawing code

			// Point primitive ?
			const CPrimPointEditor *point = dynamic_cast<const CPrimPointEditor *>(primitive);
			if (point)
			{
				// Clip ?
				if (!isClipped (&point->Point, 1))
				{
					// Position in world
					CVector center = point->Point;
					worldToPixel (center);

					// Dot
					CVector dot0, dot1, dot2, dot3;
					dot0 = center;
					dot0.x += POINT_DOT_SIZE;
					dot0.y += POINT_DOT_SIZE;
					dot1 = center;
					dot1.x -= POINT_DOT_SIZE;
					dot1.y += POINT_DOT_SIZE;
					dot2 = center;
					dot2.x -= POINT_DOT_SIZE;
					dot2.y -= POINT_DOT_SIZE;
					dot3 = center;
					dot3.x += POINT_DOT_SIZE;
					dot3.y -= POINT_DOT_SIZE;

					// Transform primitive
					transformVector (dot0, point->Angle, center);
					transformVector (dot1, point->Angle, center);
					transformVector (dot2, point->Angle, center);
					transformVector (dot3, point->Angle, center);

					// In world space
					pixelToWorld (center);
					pixelToWorld (dot0);
					pixelToWorld (dot1);
					pixelToWorld (dot2);
					pixelToWorld (dot3);

					// Draw it
					triRenderProxy (mainColor, dot0, dot1, dot2, selected?2:0);
					triRenderProxy (mainColor, dot2, dot3, dot0, selected?2:0);

					// Got a radius ?
					if (_MainFrame->showDetails ())
					{
						// Prim class available ?
						const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*point);
						if (primClass != NULL)
						{
							// Draw an arraow ?
							if (primClass->ShowArrow)
							{
								// Position in world
								center = point->Point;
								worldToPixel (center);
								CVector arrow = center;
								CVector arrow0 = center;
								arrow.x += POINT_ARROW_LINE_SIZE;
								CVector arrow1 = arrow;
								CVector arrow2 = arrow;
								arrow0.x += POINT_ARROW_LINE_SIZE + POINT_ARROW_HEAD_SIZE;
								arrow1.y += POINT_ARROW_HEAD_SIZE;
								arrow2.y -= POINT_ARROW_HEAD_SIZE;

								// Transform primitive
								transformVector (arrow, point->Angle, center);
								transformVector (arrow0, point->Angle, center);
								transformVector (arrow1, point->Angle, center);
								transformVector (arrow2, point->Angle, center);

								// In world space
								pixelToWorld (center);
								pixelToWorld (arrow);
								pixelToWorld (arrow0);
								pixelToWorld (arrow1);
								pixelToWorld (arrow2);

								// Draw it
								lineRenderProxy (mainColor, center, arrow, selected?2:0);
								triRenderProxy (arrowColor, arrow0, arrow1, arrow2, selected?2:0);
							}
						}

						// Have a radius ?
						string radius;
						if (primitive->getPropertyByName ("radius", radius))
						{
							// Get it
							float fRadius = (float)atof (radius.c_str ());

							// Get the perimeter
							float perimeter = 2.f*(float)Pi*fRadius;

							// Get the perimeter on the screen
							perimeter *= (float)_WindowWidth / (_CurViewMax.x - _CurViewMin.x);
							
							// Get the segement count
							perimeter /= (float)CIRCLE_SEGMENT_SIZE;
							
							// Clamp
							if (perimeter < CIRCLE_MIN_SEGMENT_COUNT)
								perimeter = CIRCLE_MIN_SEGMENT_COUNT;

							// Segment count
							uint segmentCount = (uint)perimeter;

							// Draw a circle
							CVector posInit = center;
							posInit.x += fRadius;
							CVector posPrevious = posInit;
							for (uint i=1; i<segmentCount+1; i++)
							{
								CVector pos = posInit;
								transformVector (pos, (float)i*2.f*(float)Pi/(float)segmentCount, center);
								lineRenderProxy (mainColor, pos, posPrevious, selected?2:0);
								posPrevious = pos;
							}
						}
					}
				}
			}
			else
			{
				// Path primitive ?
				const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *>(primitive);
				const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *>(primitive);
				if (path || zone)
				{
					// Get the pointer on the points
					uint pointsCount = path?path->VPoints.size ():zone->VPoints.size ();
					if (pointsCount)
					{
						const CPrimVector *points = path?&(path->VPoints[0]):&(zone->VPoints[0]);

						// Clipped ?
						if (!isClipped (points, pointsCount))
						{
							// Is a zone ?
							if (zone)
							{
								// 
								vector<sint32> vRef;
								vRef.resize(pointsCount);

								uint32 nStart;

								uint i;
								for(i = 0; i < vRef.size(); ++i)
									vRef[i] = i;

								nStart = 0;
								while (vRef.size() > 2)
								{
									// Is triangle (nStart, nStart+1, nStart+2) back face ?
									sint32 nP1 = vRef[nStart];
									sint32 nP2 = vRef[(nStart+1)%vRef.size()];
									sint32 nP3 = vRef[(nStart+2)%vRef.size()];
									const CVector &pos1 = points[nP1];
									const CVector &pos2 = points[nP2];
									const CVector &pos3 = points[nP3];
									if (((pos2.x-pos1.x) * (pos3.y-pos1.y) - (pos2.y-pos1.y) * (pos3.x-pos1.x)) < 0.0f)
									{
										// Yes -> next triangle
										nStart++;

										if (nStart == vRef.size())
											break;
										continue;
									}

									// Is triangle (nStart, nStart+1, nStart+2) contains the other point ?
									bool bInside = false;
									for (i = 0; i < vRef.size(); ++i)
									{
										if ((vRef[i] != nP1) && (vRef[i] != nP2) && (vRef[i] != nP3))
										{
											if (isInTriangleOrEdge(	points[vRef[i]].x, points[vRef[i]].y, 
																	pos1.x, pos1.y,
																	pos2.x, pos2.y,
																	pos3.x, pos3.y ))
											{
												bInside = true;
												break;
											}
										}
									}

									if (bInside)
									{
										// Yes -> next triangle
										nStart++;

										if (nStart == vRef.size())
											break;
										continue;
									}

									// Draw the triangle
									triRenderProxy (mainColor, pos1, pos2, pos3, selected?2:0);

									// Erase the point in the middle
									for (i = 1+((nStart+1)%vRef.size()); i < vRef.size(); ++i)
										vRef[i-1] = vRef[i];
									vRef.resize (vRef.size()-1);
									nStart = 0;
								}
							}

							// Draw the lines
							uint pt;
							const CPrimVector *previous = path ? &(points[0]) : &(points[pointsCount-1]);
							for (pt=path?1:0; pt<pointsCount; pt++)
							{
								// Current vertex
								const CPrimVector &current = points[pt];

								// Draw the line
								lineRenderProxy (lineColor, current, *previous, selected?3:1);

								// New previous
								previous = &current;
							}

							// If selected, draw dots
							if (selected && _MainFrame->isSelectionLocked ())
							{
								// Draw points
								for (pt=0; pt<pointsCount; pt++)
								{
									// Position in world
									CVector center = points[pt];
									worldToPixel (center);
									
									// Dot
									CVector dot0, dot1, dot2, dot3;
									dot0 = center;
									dot0.x += POINT_DOT_SIZE;
									dot0.y += POINT_DOT_SIZE;
									dot1 = center;
									dot1.x -= POINT_DOT_SIZE;
									dot1.y += POINT_DOT_SIZE;
									dot2 = center;
									dot2.x -= POINT_DOT_SIZE;
									dot2.y -= POINT_DOT_SIZE;
									dot3 = center;
									dot3.x += POINT_DOT_SIZE;
									dot3.y -= POINT_DOT_SIZE;

									// In world space
									pixelToWorld (dot0);
									pixelToWorld (dot1);
									pixelToWorld (dot2);
									pixelToWorld (dot3);

									// Draw it
									triRenderProxy (points[pt].Selected?SELECTED_DOT_COLOR:dotColor, dot0, dot1, dot2, points[pt].Selected?2:0);
									triRenderProxy (points[pt].Selected?SELECTED_DOT_COLOR:dotColor, dot2, dot3, dot0, points[pt].Selected?2:0);
								}
							}

							// Draw a arrow ?
							if (path && _MainFrame->showDetails () && (pointsCount>=2))
							{
								const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*path);
								if (primClass != NULL)
								{
									if (primClass->ShowArrow)
									{
										// Position in world
										CVector center = points[pointsCount-1];
										CVector i = center - points[pointsCount-2];
										i.z = 0;
										i.normalize ();
										CVector j = CVector::K ^ i;
										j.normalize ();
										worldToPixel (center);
										CVector arrow0 = center;
										CVector arrow1 = center;
										CVector arrow2 = center;
										arrow0 += i*POINT_ARROW_HEAD_SIZE;
										arrow1 += j*POINT_ARROW_HEAD_SIZE;
										arrow2 -= j*POINT_ARROW_HEAD_SIZE;

										// In world space
										pixelToWorld (arrow0);
										pixelToWorld (arrow1);
										pixelToWorld (arrow2);

										// Draw it
										triRenderProxy (arrowColor, arrow0, arrow1, arrow2, selected?2:0);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// ***************************************************************************

void CDisplay::drawLink (const NLLIGO::IPrimitive *first, const NLLIGO::IPrimitive *second)
{
	// Is shown ?
	if (_MainFrame->showDetails () && isPrimitiveVisible (first) && isPrimitiveVisible (second))
	{
		// Is a point ?
		const CPrimPointEditor *pointFirst = dynamic_cast<const CPrimPointEditor *>(first);
		const CPrimPointEditor *pointSecond = dynamic_cast<const CPrimPointEditor *>(second);
		if (pointFirst && pointSecond)
		{
			CVector v[4];
			v[0] = pointFirst->Point;
			v[1] = pointFirst->Point;
			v[2] = pointSecond->Point;
			v[3] = pointSecond->Point;
			
			uint i;
			for (i=0; i<4; i++)
				worldToPixel (v[i]);

			// Get the width direction
			CVector dir = v[0] - v[2];
			dir.z = 0;
			dir.normalize ();
			dir *= (float)LINK_LINE_SIZE;
			float temp = dir.x;
			dir.x = dir.y;
			dir.y = -temp;
			v[0] -= dir;
			v[1] += dir;
			v[2] -= dir;
			v[3] += dir;

			for (i=0; i<4; i++)
				pixelToWorld (v[i]);

			// Draw a line
			triRenderProxy (LINK_LINE_COLOR, v[0], v[2], v[3], 0);
			triRenderProxy (LINK_LINE_COLOR, v[3], v[1], v[0], 0);
		}
	}
}

// ***************************************************************************

void CDisplay::OnDraw (CDC* pDC)
{
	if (DontUse3D)
	{
		// just fill the window client ara witha white brush
		RECT client;
		GetClientRect(&client);
		pDC->SelectObject(GetStockObject(WHITE_BRUSH));
		pDC->Rectangle(&client);
		return;
	}

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;

	CNELU::clearBuffers (_BackgroundColor);

	// Display the grid
	calcCurView ();

	// Document
	CWorldEditorDoc *doc = getDocument ();

	if (_MainFrame->showLandscape ())
		_MainFrame->_ZoneBuilder->render (_CurViewMin, _CurViewMax);

	if (_MainFrame->showLayers ())
	{
		initRenderProxy ();
		// Setup matrix
		setupWorldMatrixAndFrustum ();

		// For each database elements
		for (uint i=0; i<doc->getNumDatabaseElement (); i++)
		{
			// Is a landscape ?
			if (doc->isLandscape (i))
			{
				// Get the region
				const NLLIGO::CZoneRegion &region = doc->getZoneRegionAbsolute (i);
				float minx = (float)region.getMinX () * theApp.Config.CellSize;
				float maxx = (float)(region.getMaxX ()+1) * theApp.Config.CellSize;
				float miny = (float)region.getMinY () * theApp.Config.CellSize;
				float maxy = (float)(region.getMaxY ()+1) * theApp.Config.CellSize;

				// Get the primitive
				const NLLIGO::CPrimitives &primitives = doc->getDatabaseElements (i);

				// For each child
				uint numChildren = primitives.RootNode->getNumChildren ();
				for (uint j=0; j<numChildren; j++)
				{
					// Get the children
					const IPrimitive *child;
					nlassert (primitives.RootNode->getChild (child, j));

					// Is it a bitmap ?
					const CPrimBitmap *bitmap = dynamic_cast<const CPrimBitmap *> (child);
					if (bitmap)
					{
						// Is shown ?
						if (isPrimitiveVisible (bitmap))
						{
							// Create a material
							CMaterial material;
							material.initUnlit ();
							material.setTexture (0, bitmap->getTexture ());
							material.setZFunc (CMaterial::always);
							material.setZWrite (false);

							// Blend function
							string blendFunc;
							if (bitmap->getPropertyByName ("blend_type", blendFunc))
							{
								if (blendFunc == "additif")
								{
									material.setBlend (true);
									material.setBlendFunc (CMaterial::one, CMaterial::one);
								}
								else if (blendFunc == "blend")
								{
									material.setBlend (true);
									material.setBlendFunc (CMaterial::srcalpha, CMaterial::invsrcalpha);
								}
								else if (blendFunc == "blend additif")
								{
									material.setBlend (true);
									material.setBlendFunc (CMaterial::srcalpha, CMaterial::one);
								}
								else if (blendFunc == "mix")
								{
									material.setBlend (true);
									material.setBlendFunc (CMaterial::srcalpha, CMaterial::invsrcalpha);
									material.texEnvArg0Alpha (0, CMaterial::Constant, CMaterial::SrcAlpha);
									material.texConstantColor (0, CRGBA(0,0,0, 128));
								}
							}

							// Create the vertices
							std::vector<NLMISC::CTriangleUV> vertices (2);
							vertices[0].V0.x = minx;
							vertices[0].V0.y = maxy;
							vertices[0].V0.z = 0;
							vertices[0].Uv0.U = 0;
							vertices[0].Uv0.V = 0;
							vertices[0].V1.x = minx;
							vertices[0].V1.y = miny;
							vertices[0].V1.z = 0;
							vertices[0].Uv1.U = 0;
							vertices[0].Uv1.V = 1;
							vertices[0].V2.x = maxx;
							vertices[0].V2.y = miny;
							vertices[0].V2.z = 0;
							vertices[0].Uv2.U = 1;
							vertices[0].Uv2.V = 1;
							vertices[1].V0.x = maxx;
							vertices[1].V0.y = miny;
							vertices[1].V0.z = 0;
							vertices[1].Uv0.U = 1;
							vertices[1].Uv0.V = 1;
							vertices[1].V1.x = maxx;
							vertices[1].V1.y = maxy;
							vertices[1].V1.z = 0;
							vertices[1].Uv1.U = 1;
							vertices[1].Uv1.V = 0;
							vertices[1].V2.x = minx;
							vertices[1].V2.y = maxy;
							vertices[1].V2.z = 0;
							vertices[1].Uv2.U = 0;
							vertices[1].Uv2.V = 0;
							

							// Display it
							CDRU::drawTrianglesUnlit (vertices, material, *CNELU::Driver);
						}
					}
				}
			}
		}
		flushRenderProxy ();
	}

	if (_MainFrame->showCollisions())
	{
		sint32 i;
		sint32 dx, dy;
		sint32 fx, fy;
		i = 0;
		double rx = fmod(_CurPos.x, _CellSize);
		double ry = fabs(fmod(_CurPos.y, _CellSize));
		sint32 sy;
		sy = 0;

		if (rx*2 < _CellSize)
		{
			fx = -1;
		}
		else
		{
			fx = 0;
		}

		if (ry*2 < _CellSize)
		{
			fy = 0;
			sy = 1;
		}
		else
		{
			fy = -1;
			sy = 1;
		}
		
		initRenderProxy ();
		for (dy = fy;dy<=fy+sy;++dy)
		{
			for (dx = fx;dx<=fx+1;++dx)
			{
				DrawCollisionTexture(i, _CurPos.x+dx*_CellSize, _CurPos.y+dy*_CellSize);
				++i;
			}
		}
		flushRenderProxy ();
	}

	if (_MainFrame->showPACS ())
	{
		initRenderProxy ();

		// Setup matrix
		setupWorldMatrixAndFrustum ();

		// Draw pacs
		PacsManager.displayPacs(*this);

		// Flush primitives
		flushRenderProxy ();
	}

	if (_MainFrame->showGrid ())
		_MainFrame->_ZoneBuilder->displayGrid (_CurViewMin, _CurViewMax);

	if (_MainFrame->showPrimitives ())
	{
		// Render the primitives

		// Accelerate rendering with vertex buffer
		initRenderProxy ();

		// Select the visible primitive in the quad grid
		PrimitiveQuadGrid.select (_CurViewMin, _CurViewMax);

		// Parse
		TPrimQuadGrid::CIterator ite = PrimitiveQuadGrid.begin();
		while (ite != PrimitiveQuadGrid.end ())
		{
			// Is it a primitive or a link ?
			if ((*ite).Second)
				drawLink ((*ite).First, (*ite).Second);
			else
				drawPrimitive ((*ite).First);
			ite++;
		}

		// Flush primitives
		flushRenderProxy ();
	}

	if (!theApp.Plugins.empty())
	{
		initRenderProxy ();		
		// plugins post render
		for(uint k = 0; k < theApp.Plugins.size(); ++k)
		{
			theApp.Plugins[k]->postRender(*this);
		}
		flushRenderProxy ();
	}


	if (_MainFrame->showPoints ())
		displayPoints ();

	if (_MainFrame->_Mode == 2) // If we are in transition mode
		_MainFrame->_ZoneBuilder->renderTransition (_CurViewMin, _CurViewMax);

	CNELU::Scene->render ();
	
	if (_MainFrame->_Mode == 3) // Disply the plugin position control ?
		renderPluginPosition();

	CNELU::swapBuffers ();

	if (_DrawSelection)
		drawSelection ();
}

// ***************************************************************************
int CDisplay::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate (lpCreateStruct) == -1)
		return -1;
	SetTimer (1, 200, NULL);
	return 0;
}

// ***************************************************************************
void CDisplay::OnDestroy ()
{
	KillTimer (1);
	CView::OnDestroy ();
}

// ***************************************************************************
/*void*/BOOL CDisplay::OnMouseWheel (UINT nFlags, short zDelta, CPoint point)
{
	if (!Interactif)
		return FALSE;

	if (DontUse3D)
		return FALSE;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return FALSE;
	float newFactor = _Factor;
	if (zDelta < 0)
		newFactor *= 2.0f;
	else
		newFactor /= 2.0f;

	setFactor (newFactor, true, _CurPos);

	return TRUE;
}

// ***************************************************************************
void CDisplay::OnMButtonDown (UINT nFlags, CPoint point)
{
	if (!Interactif)
		return;

	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;
	_LastMousePos = point;

	nlassert (!_MouseMidDown);
	_MouseMidDown = true;

	// Mouse capture On !
	if (!_MouseLeftDown && !_MouseRightDown)
		SetCapture ();

	// Update mouse mode
	updateMouseMode (nFlags);

	// Invalidate cursor
	invalidateCursor ();

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************
void CDisplay::OnMButtonUp (UINT nFlags, CPoint point)
{
	if (!Interactif)
		return;

	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;

	// Release capture
	if (_MouseMidDown && !_MouseLeftDown && !_MouseRightDown)
		ReleaseCapture ();

	_MouseMidDown = false;

	// Update mouse mode
	updateMouseMode (nFlags);
	
	// Invalidate cursor
	invalidateCursor ();

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************

inline bool CDisplay::pickPoint (const CVector &pickPos, const CVector &point, float bias)
{
	CVector delta = pickPos;
	delta -= point;
	delta.z = 0;
	return delta.sqrnorm () <= (bias*bias);
}

// ***************************************************************************

inline bool CDisplay::pickRect (const CVector &pickPosMin, const CVector &pickPosMax, const CVector &point)
{
	return ( (point.x >= pickPosMin.x) && (point.y >= pickPosMin.y) && (point.x <= pickPosMax.x) && (point.y <= pickPosMax.y));
}

// ***************************************************************************

inline bool CDisplay::pickPoint (const CVector &pickPos, const CVector &v0, const CVector &v1, float bias)
{

	// Segment vector
	CVector segment = v1;
	segment -= v0;
	float vectorNorm = segment.norm ();

	// Segment NULL ?
	if (vectorNorm == 0)
		return false;

	// Normalize
	segment /= vectorNorm;

	// Project
	CVector tmp = pickPos;
	tmp -= v0;
	float dist = segment * tmp;

	// Clip
	if ( (dist > -bias) && (dist < vectorNorm + bias) )
	{
		// Pos on the seg
		segment *= dist;
		segment += v0;
		segment -= pickPos;
		return segment.sqrnorm () <= (bias*bias);
	}
	return false;
}

// ***************************************************************************

inline bool testValue (float t0, float t1, float v0, float v1, float t, float vMin, float vMax)
{
	if ( ((t0 < t) && (t1 >= t)) || ((t1 < t) && (t0 >= t)))
	{
		float v = v0 + (v1 - v0) * (t - t0) / (t1 - t0);
		return (v >= vMin) && (v <= vMax);
	}
	return false;
}

// ***************************************************************************

inline bool CDisplay::pickRect (const CVector &pickPosMin, const CVector &pickPosMax, const CVector &v0, const CVector &v1)
{
	// Point inside ?
	if (pickRect (pickPosMin, pickPosMax, v0) || pickRect (pickPosMin, pickPosMax, v1))
		return true;

	// Test segment
	return ((testValue (v0.x, v1.x, v0.y, v1.y, pickPosMin.x, pickPosMin.y, pickPosMax.y)) ||
		(testValue (v0.x, v1.x, v0.y, v1.y, pickPosMax.x, pickPosMin.y, pickPosMax.y)) ||
		(testValue (v0.y, v1.y, v0.x, v1.x, pickPosMin.y, pickPosMin.x, pickPosMax.x)) ||
		(testValue (v0.y, v1.y, v0.x, v1.x, pickPosMax.y, pickPosMin.x, pickPosMax.x)));
}

// ***************************************************************************

bool CDisplay::getSelectedAverage (CVector &average)
{
	// Update data
	UpdateSelection ();

	// Reset average
	average = CVector::Null;
	uint averageCount = 0;

	// Locked ?
	bool locked = _MainFrame->isSelectionLocked ();

	// For each selected primitive
	CWorldEditorDoc *doc = getDocument ();
	std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
	while (ite != Selection.end ())
	{
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite);

		// Is there a primitive at this node ?
		if (locator.Primitive)
		{
			// Hidden ?
			if (isPrimitiveVisible (locator.Primitive))
			{
				// Selected ?
				if (getPrimitiveEditor(locator.Primitive)->getSelected())
				{
					// For each point
					uint numVector = locator.Primitive->getNumVector ();
					const CPrimVector *primVector = locator.Primitive->getPrimVector ();
					for (uint vect=0; vect<numVector; vect++)
					{
						// Get it ?
						if (!locked || primVector[vect].Selected || dynamic_cast<const CPrimPointEditor*>(locator.Primitive))
						{
							average += primVector[vect];
							averageCount++;
						}
					}
				}
			}
		}

		ite++;
	}

	if (averageCount)
		average /= (float)averageCount;

	return averageCount != 0;
}

// ***************************************************************************

bool CDisplay::pickPoint (const CVector &pickPos, std::vector<CDatabaseLocatorPointer> &result, bool pickOnlySelected)
{
	// Clear result
	result.clear ();

	float worldPickDotSize = intToWorld (PICK_DOT_SIZE);
	float worldPickPathSize = intToWorld (PICK_PATH_SIZE);

	// The selection coordinate
	const float maxDelta = std::max (worldPickPathSize, worldPickDotSize);
	CVector minPos = pickPos - CVector (maxDelta, maxDelta, maxDelta);
	CVector maxPos = pickPos + CVector (maxDelta, maxDelta, maxDelta);

	// Select the primitives
	PrimitiveQuadGrid.select (minPos, maxPos);

	// Locked ?
	CWorldEditorDoc *doc = getDocument ();
	if (_MainFrame->isSelectionLocked ())
	{
		TPrimQuadGrid::CIterator ite = PrimitiveQuadGrid.begin ();
		while (ite != PrimitiveQuadGrid.end ())
		{
			// Skip links
			if ((*ite).Second == NULL)
			{
				CDatabaseLocatorPointer locator;
				doc->getLocator (locator, (*ite).First);

				// Selected ?
				if (getPrimitiveEditor((*ite).First)->getSelected())
				{
					// Hidden ?
					if (isPrimitiveVisible ((*ite).First))
					{
						// Pick it
						const CPrimPointEditor *point = dynamic_cast<const CPrimPointEditor *> ((*ite).First);
						if (point)
						{
							// Pick it ?
							if (pickPoint (pickPos, point->Point, worldPickDotSize) && (!pickOnlySelected || getPrimitiveEditor((*ite).First)->getSelected()))
							{
								// Add this point
								result.push_back (locator);
								result.back ().XSubPrim = 0;
							}
						}
						else
						{
							// Pick it
							const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> ((*ite).First);
							if (path)
							{
								// For each vertex
								const uint numPt = path->VPoints.size ();
								for (uint i=0; i<numPt; i++)
								{
									// Pick it ?
									if (pickPoint (pickPos, path->VPoints[i], worldPickDotSize) && (!pickOnlySelected || path->VPoints[i].Selected))
									{
										// Add this path
										result.push_back (locator);
										result.back ().XSubPrim = i;
									}
								}
							}
							else
							{
								// Pick it
								const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *> ((*ite).First);
								if (zone)
								{
									// For each vertex
									const uint numPt = zone->VPoints.size ();
									for (uint i=0; i<numPt; i++)
									{
										// Pick it ?
										if (pickPoint (pickPos, zone->VPoints[i], worldPickDotSize) && (!pickOnlySelected || zone->VPoints[i].Selected))
										{
											// Add this zone
											result.push_back (locator);
											result.back ().XSubPrim = i;
										}
									}
								}
							}
						}
					}
				}
			}

			// Next primitive
			ite++;
		}
	}
	else
	{
		TPrimQuadGrid::CIterator ite = PrimitiveQuadGrid.begin ();
		while (ite != PrimitiveQuadGrid.end ())
		{
			// Skip links
			if ((*ite).Second == NULL)
			{
				// Is there a primitive at this node ?
				CDatabaseLocatorPointer locator;
				doc->getLocator (locator, (*ite).First);
				{
					// Hidden ?
					if (isPrimitiveVisible ((*ite).First))
					{
						// Pick it
						const CPrimPointEditor *point = dynamic_cast<const CPrimPointEditor *> ((*ite).First);
						if (point)
						{
							// Pick it ?
							if (pickPoint (pickPos, point->Point, worldPickDotSize) && (!pickOnlySelected || getPrimitiveEditor((*ite).First)->getSelected()))
							{
								// Add this point
								result.push_back (locator);
							}
						}
						else
						{
							// Pick it
							const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> ((*ite).First);
							if (path)
							{
								// For each segment
								const uint numPt = path->VPoints.size ();
								if (numPt>1)
								{
									for (uint i=0; i<numPt-1; i++)
									{
										// Pick it ?
										if (pickPoint (pickPos, path->VPoints[i], path->VPoints[i+1], worldPickPathSize) && (!pickOnlySelected || getPrimitiveEditor((*ite).First)->getSelected()))
										{
											// Add this path
											result.push_back (locator);
										}
									}
								}
							}
							else
							{
								// Pick it
								const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *> ((*ite).First);
								if (zone)
								{
									// Pick it ?
									if (zone->contains (pickPos) && (!pickOnlySelected || getPrimitiveEditor((*ite).First)->getSelected()))
									{
										// Add this path
										result.push_back (locator);
									}
								}
							}
						}
					}
				}
			}

			// Next primitive
			ite++;
		}
	}
	
	return !result.empty();
}

// ***************************************************************************

bool CDisplay::pickEdgePoint (const CVector &pickPos, std::vector<CDatabaseLocatorPointer> &result)
{
	// Clear result
	result.clear ();

	float worldPickPathSize = intToWorld (PICK_PATH_SIZE);

	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// The selection coordinate
	CVector minPos = pickPos - CVector (worldPickPathSize, worldPickPathSize, worldPickPathSize);
	CVector maxPos = pickPos + CVector (worldPickPathSize, worldPickPathSize, worldPickPathSize);

	// Select the primitives
	PrimitiveQuadGrid.select (minPos, maxPos);

	TPrimQuadGrid::CIterator ite = PrimitiveQuadGrid.begin ();
	while (ite != PrimitiveQuadGrid.end ())
	{
		// Skip links
		if ((*ite).Second == NULL)
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, (*ite).First);

			// Hidden ?
			if (isPrimitiveVisible ((*ite).First))
			{
				// Selected ?
				if (getPrimitiveEditor((*ite).First)->getSelected())
				{
					// Pick it
					const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> ((*ite).First);
					if (path)
					{
						// For each vertex
						const uint numPt = path->VPoints.size ();
						if (numPt > 1)
						{
							for (uint i=0; i<numPt-1; i++)
							{
								// Pick it ?
								if (pickPoint (pickPos, path->VPoints[i], path->VPoints[i+1], worldPickPathSize))
								{
									// Add this path
									result.push_back (locator);
									result.back ().XSubPrim = i;
								}
							}
						}
					}
					else
					{
						// Pick it
						const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *> ((*ite).First);
						if (zone)
						{
							// For each vertex
							const uint numPt = zone->VPoints.size ();
							if (numPt > 1)
							{
								uint previous = numPt-1;
								for (uint i=0; i<numPt; i++)
								{
									// Pick it ?
									if (pickPoint (pickPos, zone->VPoints[previous], zone->VPoints[i], worldPickPathSize))
									{
										// Add this zone
										result.push_back (locator);
										result.back ().XSubPrim = previous;
									}

									// Nex previous
									previous = i;
								}
							}
						}
					}
				}
			}
		}

		// Next primitive
		ite++;
	}
	
	return !result.empty();
}

// ***************************************************************************

bool CDisplay::pickRect (const CVector &pickPosMin, const CVector &pickPosMax, std::vector<CDatabaseLocatorPointer> &result, bool pickOnlyShown)
{
	// Clear result
	result.clear ();

	// Select the primitives
	PrimitiveQuadGrid.select (pickPosMin, pickPosMax);

	// Locked ?
	CWorldEditorDoc *doc = getDocument ();
	if (_MainFrame->isSelectionLocked ())
	{
		TPrimQuadGrid::CIterator ite = PrimitiveQuadGrid.begin ();
		while (ite != PrimitiveQuadGrid.end ())
		{
			// Skip links
			if ((*ite).Second == NULL)
			{
				CDatabaseLocatorPointer locator;
				doc->getLocator (locator, (*ite).First);

				// Selected ?
				if (getPrimitiveEditor((*ite).First)->getSelected())
				{
					// Hidden ?
					if ((!pickOnlyShown) || (isPrimitiveVisible ((*ite).First)))
					{
						// Pick it
						const CPrimPointEditor *point = dynamic_cast<const CPrimPointEditor *> ((*ite).First);
						if (point)
						{
							// Pick it ?
							if (pickRect (pickPosMin, pickPosMax, point->Point))
							{
								// Add this point
								result.push_back (locator);
								result.back ().XSubPrim = 0;
							}
						}
						else
						{
							// Pick it
							const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> ((*ite).First);
							if (path)
							{
								// For each vertex
								const uint numPt = path->VPoints.size ();
								for (uint i=0; i<numPt; i++)
								{
									// Pick it ?
									if (pickRect (pickPosMin, pickPosMax, path->VPoints[i]))
									{
										// Add this path
										result.push_back (locator);
										result.back ().XSubPrim = i;
									}
								}
							}
							else
							{
								// Pick it
								const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *> ((*ite).First);
								if (zone)
								{
									// For each vertex
									const uint numPt = zone->VPoints.size ();
									for (uint i=0; i<numPt; i++)
									{
										// Pick it ?
										if (pickRect (pickPosMin, pickPosMax, zone->VPoints[i]))
										{
											// Add this zone
											result.push_back (locator);
											result.back ().XSubPrim = i;
										}
									}
								}
							}
						}
					}
				}
			}
	
			// Next primitive
			ite++;
		}
	}
	else
	{
		TPrimQuadGrid::CIterator ite = PrimitiveQuadGrid.begin ();
		while (ite != PrimitiveQuadGrid.end ())
		{
			// Skip links
			if ((*ite).Second == NULL)
			{
				CDatabaseLocatorPointer locator;
				doc->getLocator (locator, (*ite).First);
				{
					// Hidden ?
					if ((!pickOnlyShown) || (isPrimitiveVisible ((*ite).First)))
					{
						// Pick it
						const CPrimPointEditor *point = dynamic_cast<const CPrimPointEditor *> ((*ite).First);
						if (point)
						{
							// Pick it ?
							if (pickRect (pickPosMin, pickPosMax, point->Point))
							{
								// Add this point
								result.push_back (locator);
							}
						}
						else
						{
							// Pick it
							const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> ((*ite).First);
							if (path)
							{
								// For each segment
								const uint numPt = path->VPoints.size ();
								if (numPt>1)
								{
									for (uint i=0; i<numPt-1; i++)
									{
										// Pick it ?
										if (pickRect (pickPosMin, pickPosMax, path->VPoints[i], path->VPoints[i+1]))
										{
											// Add this path
											result.push_back (locator);
										}
									}
								}
							}
							else
							{
								// Pick it
								const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *> ((*ite).First);
								if (zone)
								{
									// For each segment
									const uint numPt = zone->VPoints.size ();
									if (numPt>1)
									{
										// Last point
										uint lastPt = numPt-1;
										for (uint i=0; i<numPt; i++)
										{
											// Pick it ?
											if (pickRect (pickPosMin, pickPosMax, zone->VPoints[i], zone->VPoints[lastPt]))
											{
												// Add this zone
												result.push_back (locator);
											}

											// Next segment
											lastPt = i;
										}
									}
								}
							}
						}
					}
				}
			}

			// Next primitive
			ite++;
		}
	}
	return !result.empty();
}

// ***************************************************************************

void CDisplay::OnLButtonDown (UINT nFlags, CPoint point)
{
	if (!Interactif)
		return;

	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;

	nlassert (!_MouseLeftDown);
	_MouseLeftDown = true;

	// Mouse capture On !
	if (!_MouseMidDown && !_MouseRightDown)
		SetCapture ();

	// Update mouse mode
	updateMouseMode (nFlags);

	buttonDown (Left, nFlags, point);

	invalidateCursor ();

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************

void CDisplay::OnRButtonDown (UINT nFlags, CPoint point)
{
	if (!Interactif)
		return;

	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;

	nlassert (!_MouseRightDown);
	_MouseRightDown = true;

	// Mouse capture On !
	if (!_MouseMidDown && !_MouseLeftDown)
		SetCapture ();

	// Update mouse mode
	updateMouseMode (nFlags);

	buttonDown (Right, nFlags, point);

	invalidateCursor ();

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************

void CDisplay::buttonDown (TButton button, UINT nFlags, CPoint point)
{
	// this bool is used to fix a nasty bug: when a popup window appears, the message pump is called
	bool EndModification = false;
	_ClickedSelected = false;
	CWorldEditorDoc *doc = getDocument ();
	_CurPos = convertToWorld (point);

	// Begin of an undo stack element
	doc->beginModification ();

	// Mode Zone
	if ((_MouseMode == DrawTiles) || (_MouseMode == RemoveTiles)) 
	{
		if (_MouseMode == DrawTiles)
			_MainFrame->_ZoneBuilder->add (_CurPos);
		if (_MouseMode == RemoveTiles)
			_MainFrame->_ZoneBuilder->del (_CurPos);

		_LastX = (sint32)floor (_CurPos.x / _CellSize);
		_LastY = (sint32)floor (_CurPos.y / _CellSize);
	}

	// Cancel tranform ?
	if (_TranformAction && (_MouseMode != TransformPrimitive))
	{
		_TranformAction->undo ();
		_TranformAction = NULL;
	}
	
	// Mode Logic
	if ((_MouseMode == TransformPrimitive) || (_MouseMode == MenuPrimitive))
	{
		_SelectionMin = point;
		_SelectionMax = point;

		CVector pickPosMin ((float)_SelectionMin.x, (float)(_WindowHeight-_SelectionMin.y-1), 0);
		pixelToWorld (pickPosMin);

		// Left or right only
		if ((button == Left) || (button == Right))
		{
			// Add sub element vertex mode ?
			bool addPointSub = (_MainFrame->getTransformMode () == CMainFrame::AddPoint) && (_MainFrame->isSelectionLocked ());
			if ( !addPointSub || (button == Right) )
			{
				// Pick a primitive that exist
				std::vector<class CDatabaseLocatorPointer> result;
				if (pickPoint (pickPosMin, result, false))
				{
					if ((nFlags & MK_CONTROL) != 0)
					{
						// Next primitives
						_LastPickedPrimitiveId++;
						_LastPickedPrimitiveId %= result.size ();

						// Array
						std::vector<class CDatabaseLocatorPointer> selectPrimitive;
						selectPrimitive.push_back (result[_LastPickedPrimitiveId]);
						selectPrimitives (selectPrimitive, true);
					}
					else
					{
						// Over something selected ?
						uint prim;
						for (prim=0; prim<result.size (); prim++)
						{
							if (_MainFrame->isSelectionLocked ())
							{
								// Is selected ?
								nlassert ((uint)result[prim].XSubPrim < result[prim].Primitive->getNumVector ());
								if (result[prim].Primitive->getPrimVector ()[result[prim].XSubPrim].Selected)
									break;
							}
							else
							{
								// Selected ?
								if (getPrimitiveEditor(result[prim].Primitive)->getSelected())
									break;
							}
						}
						
						// Over something selected ?
						if (prim < result.size ())
						{
							_ClickedSelected = true;
						}
						else
						{
							// Next primitives
							_LastPickedPrimitiveId++;
							_LastPickedPrimitiveId %= result.size ();

							// Select the primitive
							std::vector<class CDatabaseLocatorPointer> selectPrimitive;
							selectPrimitive.push_back (result[_LastPickedPrimitiveId]);
							selectPrimitives (selectPrimitive, false);
						}
					}
				}

				// Update selection
				UpdateSelection ();

				// Over something selected ?
				result.clear ();
				if (pickPoint (pickPosMin, result, true) && (button == Left))
				{
					// Translate or rotate ?
					switch (_MainFrame->getTransformMode ())
					{
					case CMainFrame::Move:
						{
							nlassert (_TranformAction == NULL);

							// Add a new action
							CActionMove *moveAction = new CActionMove (_MainFrame->isSelectionLocked ());
							_TranformAction = moveAction;
							moveAction->setTranslation (CVector::Null);
							moveAction->redo ();
						}
						break;
					case CMainFrame::Rotate:
					case CMainFrame::Turn:
						{
							nlassert (_TranformAction == NULL);

							// Get the pivot
							CVector pivot;
							if (getSelectedAverage (pivot))
							{
								// Add a new action
								CActionRotate *moveAction = new CActionRotate (_MainFrame->isSelectionLocked (), pivot);
								_TranformAction = moveAction;
								moveAction->setAngle (0, false, false);
								moveAction->redo ();
							}
						}
						break;
					case CMainFrame::Scale:
					case CMainFrame::Radius:
						{
							nlassert (_TranformAction == NULL);

							// Get the pivot
							CVector pivot;
							if (getSelectedAverage (pivot))
							{
								// Add a new action
								CActionScale *moveAction = new CActionScale (_MainFrame->isSelectionLocked (), pivot);
								_TranformAction = moveAction;
								moveAction->setScale (1, false, false);
								moveAction->redo ();
							}
						}
						break;
					}
				}
			}
			else
			{
				if (_MainFrame->isSelectionLocked ())
				{
					nlassert (_TranformAction == NULL);

					// Update selection
					UpdateSelection ();

					std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
					while (ite != Selection.end ())
					{
						CDatabaseLocatorPointer locator;
						doc->getLocator (locator, *ite);

						// Primitive ?
						if (locator.Primitive)
						{
							// Hidden ?
							if (isPrimitiveVisible (locator.Primitive))
							{
								// Path or zone ?
								if (dynamic_cast<const CPrimPathEditor*>(locator.Primitive) || dynamic_cast<const CPrimZoneEditor*>(locator.Primitive))
								{
									// The locator
									locator.XSubPrim = locator.Primitive->getNumVector ()-1;

									// Add a new action
									doc->addModification (new CActionAddVertex (locator, pickPosMin));

									// Select it
									locator.XSubPrim++;
									doc->addModification (new CActionUnselectAllSub ());
									doc->addModification (new CActionSelectSub (locator));
								}
							}
						}
						ite++;
					}

					// Set the transform mode
					nlassert (_TranformAction == NULL);

					// Add a new action
					CActionMove *moveAction = new CActionMove (_MainFrame->isSelectionLocked ());
					_TranformAction = moveAction;
					moveAction->setTranslation (CVector::Null);
					moveAction->redo ();
				}
			}
			
			// Left click ?
			if (button == Left)
			{
				// No action actived ?
				if (!_TranformAction && (_MainFrame->getTransformMode () != CMainFrame::Select) )
				{
					// *** Try to add vertices in sub mode

					// Are we in sub mode ?
					if (_MainFrame->isSelectionLocked ())
					{
						// Pick the vertex before the clicked edge
						std::vector<class CDatabaseLocatorPointer> result;
						if (pickEdgePoint (pickPosMin, result))
						{
							// Add a vertex in the first segment
							doc->addModification (new CActionAddVertex (result[0], pickPosMin));

							// Select it
							result[0].XSubPrim++;
							doc->addModification (new CActionUnselectAllSub ());
							doc->addModification (new CActionSelectSub (result[0]));

							// Set the transform mode
							nlassert (_TranformAction == NULL);

							// Add a new action
							CActionMove *moveAction = new CActionMove (_MainFrame->isSelectionLocked ());
							_TranformAction = moveAction;
							moveAction->setTranslation (CVector::Null);
							moveAction->redo ();
						}
					}
				}

				// No action performed ?
				if (!_TranformAction)
				{
					// Remove old selection
					drawSelection ();
					_DrawSelection = true;
				}
			}
			else
			{
				// Update selection
				UpdateSelection ();
				// we stop the modifications before opening the popupmenu (otherwise, it crashes on the plugin !)
				EndModification = true;
				doc->endModification ();
				
				// Right
				CPoint point2 = point;
				ClientToScreen (&point2);
				_MainFrame->createContextMenu (_MainFrame, point2, true);
				
				// No more button
				_MouseRightDown = false;
				_MouseLeftDown = false;
				_MouseMidDown = false;
				updateMouseMode (nFlags);
			}
		}
	}

	// Mode Transition
	if (_MouseMode == DrawTransition)
	{
		_MainFrame->_ZoneBuilder->addTransition (_CurPos);
		_LastX = (sint32)floor (_CurPos.x / _CellSize);
		_LastY = (sint32)floor (_CurPos.y / _CellSize);
	}

	// Plugin position control
	if (_MouseMode == PluginMove) 
	{
		_MainFrame->_PositionControl = _CurPos;
		if (_MainFrame->_CurrentPlugin)
		{
			_MainFrame->_CurrentPlugin->positionMoved(_CurPos);
			// invalide display to update cursor position
			_MainFrame->Invalidate(FALSE);
		}
	}

	// End of an undo stack element
	if (!EndModification)
	{
		doc->endModification ();
	}

	_MouseMoved = false;
	_LastMousePos = point;

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************
void CDisplay::OnLButtonDblClk (UINT nFlags, CPoint point)
{
	if (!Interactif)
		return;

	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;
	if (_MouseRightDown || _MouseMidDown)
		return;

	_CurPos = convertToWorld (point);

	// Begin of an undo stack element
	getDocument ()->beginModification ();

	if (_MainFrame->_Mode == 2) // Mode Transition
	{
		_MainFrame->_ZoneBuilder->addTransition (_CurPos);
		_LastX = (sint32)floor (_CurPos.x / _CellSize);
		_LastY = (sint32)floor (_CurPos.y / _CellSize);
	}

	if (_MainFrame->_Mode == 1) // Mode Logic
	{
	}

	// End of an undo stack element
	getDocument ()->endModification ();

	_LastMousePos = point;

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************

void CDisplay::selectPrimitives (const std::vector<class CDatabaseLocatorPointer> &result, bool addSelection)
{
	// The docmuent
	CWorldEditorDoc *doc = getDocument ();

	// Ctrl ?
	if (!addSelection)
	{
		if (_MainFrame->isSelectionLocked ())
		{
			// Unselect
			doc->addModification (new CActionUnselectAllSub ());
		}
		else
		{
			// Unselect
			doc->addModification (new CActionUnselectAll ());
		}
	}

	// Backup selected flag
	std::vector<bool> selected (result.size ());

	// For each primitive
	uint i;
	for (i=0; i<result.size (); i++)
	{
		// Already selected ?
		if (_MainFrame->isSelectionLocked ())
		{
			// Check the vertex flag
			nlassert ((uint)result[i].XSubPrim < result[i].Primitive->getNumVector ());
			selected[i] = result[i].Primitive->getPrimVector ()[result[i].XSubPrim].Selected;
		}
		else
		{
			// Get selection property
//			IProperty *prop;
			selected[i] = getPrimitiveEditor(result[i].Primitive)->getSelected();
		}
	}

	// For each primitive
	for (i=0; i<result.size (); i++)
	{
		// Perform left click
		if (selected[i])
		{
			// Ctrl ?
			if (addSelection)
			{
				// Unselect all
				if (_MainFrame->isSelectionLocked ())
				{
					// Select it
					doc->addModification (new CActionUnselectSub (result[i]));
				}
				else
				{
					// Select it
					doc->addModification (new CActionUnselect (result[i]));
				}
			}
		}
		else
		{
			// Select it
			if (_MainFrame->isSelectionLocked ())
			{
				doc->addModification (new CActionSelectSub (result[i]));
			}
			else
			{
				doc->addModification (new CActionSelect (result[i]));
			}
		}
	}
}

// ***************************************************************************

void CDisplay::OnLButtonUp (UINT nFlags, CPoint point)
{
	if (!Interactif)
		return;

	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;

	// Release capture
	if 	(!_MouseMidDown && _MouseLeftDown && !_MouseRightDown)
		ReleaseCapture ();

	// Mouse was down ?
	if (_MouseMode == TransformPrimitive)
	{
		// Get the document
		CWorldEditorDoc *doc = getDocument ();

		_CurPos = convertToWorld (point);

		// Begin of an undo stack element
		getDocument ()->beginModification ();

		// Transform ?
		if (_SelectionMin != _SelectionMax)
		{
			if (_TranformAction)
			{
				// Add the action
				_TranformAction->undo ();
				doc->addModification (_TranformAction);

				// Reset action
				_TranformAction = NULL;
			}
			else
			{
				// Erase selection
				drawSelection ();
				_DrawSelection = false;

				// Pick position
				CVector pickPosMin ((float)_SelectionMin.x, (float)(_WindowHeight-_SelectionMin.y-1), 0);
				CVector pickPosMax ((float)_SelectionMax.x, (float)(_WindowHeight-_SelectionMax.y-1), 0);
				if (pickPosMin.x > pickPosMax.x)
				{
					float temp = pickPosMin.x;
					pickPosMin.x = pickPosMax.x;
					pickPosMax.x = temp;
				}
				if (pickPosMin.y > pickPosMax.y)
				{
					float temp = pickPosMin.y;
					pickPosMin.y = pickPosMax.y;
					pickPosMax.y = temp;
				}

				// In world
				pixelToWorld (pickPosMin);
				pixelToWorld (pickPosMax);

				std::vector<class CDatabaseLocatorPointer> result;
				if (pickRect (pickPosMin, pickPosMax, result, true))
				{
					// Select it
					selectPrimitives (result, (nFlags & MK_CONTROL) != 0);
				}
				else
				{
					// No ctrl ?
					if ((nFlags & MK_CONTROL) == 0)
					{
						// Unselect all
						if (_MainFrame->isSelectionLocked ())
						{
							doc->addModification (new CActionUnselectAllSub ());
						}
						else
						{
							doc->addModification (new CActionUnselectAll ());
						}
					}
				}
			}
		}
		else
		{
			if (_TranformAction)
			{
				// Reset action
				delete _TranformAction;
				_TranformAction = NULL;
			}
			else
			{
				// Erase selection
				drawSelection ();
				_DrawSelection = false;
			}

			// Pick position
			CVector pickPosMin ((float)_SelectionMin.x, (float)(_WindowHeight-_SelectionMin.y-1), 0);
			CVector pickPosMax ((float)_SelectionMax.x, (float)(_WindowHeight-_SelectionMax.y-1), 0);
			if (pickPosMin.x > pickPosMax.x)
			{
				float temp = pickPosMin.x;
				pickPosMin.x = pickPosMax.x;
				pickPosMax.x = temp;
			}
			if (pickPosMin.y > pickPosMax.y)
			{
				float temp = pickPosMin.y;
				pickPosMin.y = pickPosMax.y;
				pickPosMax.y = temp;
			}

			// In world
			pixelToWorld (pickPosMin);

			// Pick
			std::vector<class CDatabaseLocatorPointer> result;
			bool picked = pickPoint (pickPosMin, result, false);


			// Pick a primitive that exist
			if (_ClickedSelected)
			{
				// Should e something under the mouse
				nlassert (picked);

				// Look for a non selected primitive
//				IProperty *prop;
				uint prim;
				for (prim=0; prim<result.size (); prim++)
				{
					// Next primitives
					_LastPickedPrimitiveId++;
					_LastPickedPrimitiveId %= result.size ();

					// Selected ?
					if (!getPrimitiveEditor(result[_LastPickedPrimitiveId].Primitive)->getSelected())
						break;
				}

				// Should be over a selected primitive
				nlassert (_LastPickedPrimitiveId != result.size ());

				// Array
				std::vector<class CDatabaseLocatorPointer> selectPrimitive;
				selectPrimitive.push_back (result[_LastPickedPrimitiveId]);
				selectPrimitives (selectPrimitive, false);
			}
			
			// Something under the mouse ?
			if (!picked)
			{
				// No ctrl ?
				if ((nFlags & MK_CONTROL) == 0)
				{
					// Unselect all
					if (_MainFrame->isSelectionLocked ())
					{
						doc->addModification (new CActionUnselectAllSub ());
					}
					else
					{
						doc->addModification (new CActionUnselectAll ());
					}
				}
			}
		}

		// End of an undo stack element
		getDocument ()->endModification ();
	}

	_MouseLeftDown = false;

	// Update mouse mode
	updateMouseMode (nFlags);

	// Invalidate cursor
	invalidateCursor ();

	// Update data
	_MainFrame->updateData ();
	nlassert (_DrawSelection == false);
}

// ***************************************************************************

void CDisplay::OnRButtonUp (UINT nFlags, CPoint point)
{
	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;
	_MouseRightDown = false;

	// Release capture
	if (!_MouseMidDown && !_MouseLeftDown && _MouseRightDown)
		ReleaseCapture ();

	// Update mouse mode
	updateMouseMode (nFlags);

	// Invalidate cursor
	invalidateCursor ();

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************

void CDisplay::drawSelection ()
{
	// Get the DC
	CDC *pDC = GetDC ();

	// Rect
	RECT rect;
	rect.left = std::min (_SelectionMin.x, _SelectionMax.x);
	rect.top = std::min (_SelectionMin.y, _SelectionMax.y);
	rect.right = std::max (_SelectionMin.x, _SelectionMax.x);
	rect.bottom = std::max (_SelectionMin.y, _SelectionMax.y);

	// Draw a xor focus rect
	pDC->DrawFocusRect (&rect);

	// Release the DC
	ReleaseDC (pDC);
}

// ***************************************************************************
void CDisplay::OnMouseMove (UINT nFlags, CPoint point)
{
	if (!Interactif)
		return;

	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;

	if (_MainFrame->GetActiveWindow() == _MainFrame)
		SetFocus();

	if ((_LastMousePos.x == point.x) && (_LastMousePos.y == point.y))
		return;

	_MouseMoving = true;

	// Begin of an undo stack element
	CWorldEditorDoc *doc = getDocument ();
	if (doc)
	{
		doc->beginModification ();
	}
	
	// Get the current position in world
	_CurPos = convertToWorld (point);
	_MouseMoved = true;

	const bool shiftPushed = (nFlags & MK_SHIFT) != 0;
	const bool altPushed = (GetAsyncKeyState (VK_MENU) & 0x8000) != 0;
	const bool ctrlPushed = ( ( GetAsyncKeyState (VK_LCONTROL) | GetAsyncKeyState (VK_RCONTROL) ) & 0x8000 ) != 0;
	switch (_MouseMode)
	{
	// Mode Zone
	case DrawTiles:
		{
			sint32 x = (sint32)floor (_CurPos.x / _CellSize);
			sint32 y = (sint32)floor (_CurPos.y / _CellSize);
			if ((x!=_LastX)||(y!=_LastY))
			{
				_LastX = x;
				_LastY = y;
				_MainFrame->_ZoneBuilder->add (_CurPos);
			}
		}
		break;
	case RemoveTiles:
		{
			sint32 x = (sint32)floor (_CurPos.x / _CellSize);
			sint32 y = (sint32)floor (_CurPos.y / _CellSize);
			if ((x!=_LastX)||(y!=_LastY))
			{
				_LastX = x;
				_LastY = y;
				_MainFrame->_ZoneBuilder->del (_CurPos);
			}
		}
		break;
	case TransformPrimitive:
		{
			// Tranform mode ?
			if (_TranformAction)
			{
				// New position
				_SelectionMax = point;

				switch (_MainFrame->getTransformMode ())
				{
				case CMainFrame::Move:
					{
						// Get the action
						CActionMove *moveAction = dynamic_cast<CActionMove *> (_TranformAction);
						if (moveAction)
						{
							CVector pickPosMin ((float)_SelectionMin.x, (float)(_WindowHeight-_SelectionMin.y-1), 0);
							CVector pickPosMax ((float)_SelectionMax.x, (float)(_WindowHeight-_SelectionMax.y-1), 0);
							pixelToWorld (pickPosMin);
							pixelToWorld (pickPosMax);

							// Set the new translation
							CVector translation = pickPosMax;
							translation -= pickPosMin;
							translation.z = 0;

							// Undo the action
							moveAction->undo ();

							// Set the new translation
							moveAction->setTranslation (translation);

							// Redo the action
							moveAction->redo ();
						}
					}
					break;
				case CMainFrame::Rotate:
				case CMainFrame::Turn:
					{
						CActionRotate *moveAction = dynamic_cast<CActionRotate *> (_TranformAction);
						if (moveAction)
						{
							// Set the new rotation
							float angle = (_SelectionMin.x - _SelectionMax.x + _SelectionMax.y - _SelectionMin.y) * ROTATE_PER_PIXEL;

							// Undo the action
							moveAction->undo ();

							// Set the new translation
							const bool turn = _MainFrame->getTransformMode () == CMainFrame::Turn || ctrlPushed || altPushed || shiftPushed;
							moveAction->setAngle (angle, _MainFrame->getTransformMode ()==CMainFrame::Rotate, turn);

							// Redo the action
							moveAction->redo ();
						}
					}
					break;
				case CMainFrame::Scale:
				case CMainFrame::Radius:
					{
						// Get the action
						CActionScale *moveAction = dynamic_cast<CActionScale *> (_TranformAction);
						if (moveAction)
						{
							// Set the new scale
							float scale = (_SelectionMin.x - _SelectionMax.x + _SelectionMax.y - _SelectionMin.y) * SCALE_PER_PIXEL + 1.f;

							// Undo the action
							moveAction->undo ();

							// Set the new translation
							const bool radius = _MainFrame->getTransformMode () == CMainFrame::Radius || ctrlPushed || altPushed || shiftPushed;
							moveAction->setScale (std::max (0.001f, scale), _MainFrame->getTransformMode ()==CMainFrame::Scale, radius);

							// Redo the action
							moveAction->redo ();
						}
					}
					break;
				}
			}
			else
			{
				// Remove old selection
				nlassert (_DrawSelection);
				drawSelection ();

				// New position
				_SelectionMax = point;

				// Draw new selection
				drawSelection ();
			}
		}
		break;
	// Plugin position control
	case PluginMove:
		{
			_MainFrame->_PositionControl = _CurPos;
			if (_MainFrame->_CurrentPlugin)
			{
				_MainFrame->_CurrentPlugin->positionMoved(_CurPos);
				_MainFrame->Invalidate(FALSE);
			}
		}
		break;
	// View displacement
	case DragView:
		{
			::CRect rect;
			GetClientRect (rect);

			float dx = ((float)(point.x-_LastMousePos.x)) / ((float)rect.Width());
			float dy = -((float)(point.y-_LastMousePos.y)) / ((float)rect.Height());

			_Offset.x -= dx * (_InitViewMax.x-_InitViewMin.x) * _Factor;
			_Offset.y -= dy * (_InitViewMax.y-_InitViewMin.y) * _Factor;

			invalidateLeftView ();
		}
		break;
	case ZoomView:
		{
			::CRect rect;
			GetClientRect (rect);

			float dx = ((float)(point.x-_LastMousePos.x)) / ((float)rect.Width());
			float dy = -((float)(point.y-_LastMousePos.y)) / ((float)rect.Height());
			if (DontUse3D)
				return;
			if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;
			setFactor (_Factor*(float)pow(4.0f, dx+dy), true, _OrigineZoom);
		}
		break;
	}

	// End of an undo stack element
	if (doc)
		doc->endModification ();

	// Nothing pushed ?
	bool displayInfo = false;
	if (_MouseMode == Nothing)
	{
		if (_MainFrame->_Mode == 1) // Mode Logic
		{
			// Pick pos
			CVector pickPos ((float)point.x, (float)(_WindowHeight-point.y-1), 0);
			pixelToWorld (pickPos);

			// Pick something..
			std::vector<class CDatabaseLocatorPointer> result;
			if (pickPoint (pickPos, result, false))
			{
				// Point ?
				uint i;
				for (i=0; i<result.size (); i++)
				{
					// Is it a point
					if (dynamic_cast<const CPrimPointEditor*> (result[i].Primitive) || dynamic_cast<const CPrimPathEditor*> (result[i].Primitive))
						break;
				}

				// Not found ?
				if (i == result.size ())
					i = 0;

				// Set the name
				_MainFrame->displayInfo (result[i].getPathName ().c_str ());
				displayInfo = true;
			}
		}
	}

	// if we have to show the Collisions map, we simply Invalidate the window
	if (_MainFrame->showCollisions())
	{
		sint32 x, y;
		x = (sint32)floor(2*_CurPos.x / _CellSize);
		y = (sint32)floor(2*_CurPos.y / _CellSize);
		
		if (x != _lastXCollisionTextureMove || y != _lastYCollisionTextureMove)
		{
			_lastXCollisionTextureMove = x;
			_lastYCollisionTextureMove = y;
			Invalidate(FALSE);
		}
	}
	
	// No display info ?
	if (!displayInfo)
		_MainFrame->displayInfo ("");

	_LastMousePos = point;

	// Invalidate mouse cursor
	invalidateCursor ();

	// Display the current position in the world in the status bar
	_MainFrame->displayStatusBarInfo ();

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************
void CDisplay::OnChar (UINT nChar, UINT nRepCnt, UINT nFlags)
{
	/* todo remov all this and set normal accelerator */
	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;

	if (nChar == 32) // Space bar == middle mouse button
	{
		_MouseMidDown = true;
		updateMouseMode (nFlags);
	}

	/* if ((nChar == 'g') || (nChar == 'G'))
	{
		_MainFrame->onMenuViewGrid ();
	} */

	if ((nChar == 't') || (nChar == 'T'))
	{
		if (_MainFrame->_Mode == 2)
			_MainFrame->onMenuModeZone ();
		else if (_MainFrame->_Mode == 0)
			_MainFrame->onMenuModeTransition ();
		/* else if (_MainFrame->_Mode == 1)
			_MainFrame->transfert (_MainFrame->_PRegionBuilder.getSelPBName()); */
	}

	// Georges binds
	if ((nChar == 'l') || (nChar == 'L'))
	{
		/* if (_MainFrame->_Mode == 1)
			_MainFrame->lineDown (); */
	}
	if ((nChar == 'o') || (nChar == 'O'))
	{
		/* if (_MainFrame->_Mode == 1)
			_MainFrame->lineUp (); */
	}

	// Escape stop plugin position control.
	if (nChar == 27 && _MainFrame->_Mode == 3)
	{
		 if (_MainFrame->_CurrentPlugin != 0)
		{
			_MainFrame->_CurrentPlugin->lostPositionControl();
			_MainFrame->_CurrentPlugin = 0;
		} 
		_MainFrame->_Mode = _MainFrame->_LastMode;
		_MainFrame->Invalidate(FALSE);
	}
}

// ***************************************************************************
void CDisplay::OnKeyUp (UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (DontUse3D)
		return;

	if ((CNELU::Driver == NULL)||(_MainFrame == NULL)) return;
	if (nChar == 32)
	{
		_MouseMidDown = false;
		updateMouseMode (nFlags);
	}

	// Invalidate cursor
	invalidateCursor ();

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************
void CDisplay::OnSize (UINT nType, int cx, int cy)
{
	if (nType == SIZE_RESTORED)
	{
		if ((cx > 25) && (cy > 25))
		{
			float xRatio = (float)(cx) / 800.0f;
			float yRatio = (float)(cy) / 800.0f;

			_InitViewMax = CVector (5.5f*xRatio*_CellSize, 5.5f*yRatio*_CellSize, 0.0f);
			_InitViewMin = CVector (-5.5f*xRatio*_CellSize, -5.5f*yRatio*_CellSize, 0.0f);
		}
	}
	invalidateLeftView ();
}

// ***************************************************************************
LRESULT CDisplay::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	if (!DontUse3D && CNELU::Driver)
	{
		typedef void (*winProc)(NL3D::IDriver *drv, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		winProc	drvWndProc= (winProc)CNELU::Driver->getWindowProc();
		drvWndProc (CNELU::Driver, m_hWnd, message, wParam, lParam);
	}

	return CView::WindowProc(message, wParam, lParam);
}

// ***************************************************************************

BOOL CDisplay::OnEraseBkgnd (CDC *pC)
{
	return TRUE;
}

// ***************************************************************************

CDisplay *getDisplay ()
{
	CMainFrame *mainFrame = getMainFrame ();
	if (mainFrame)
		return dynamic_cast<CDisplay*>(getMainFrame ()->m_wndSplitter.GetPane(0,0));
	else
		return NULL;
}


// ***************************************************************************
BOOL CDisplay::PreTranslateMessage(MSG *pmsg)
{
	if (pmsg->message == WM_KEYDOWN)
	{
		if (pmsg->wParam >= VK_LEFT && pmsg->wParam <=VK_DOWN)
		{
			CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(getMainFrame ()->m_wndSplitter.GetPane(0,1));

			if (toolWnd)
			{
				toolWnd->GetTreeCtrl()->SendMessage(WM_KEYDOWN, pmsg->wParam, 0);
			}

			return TRUE;
		
		}
	}

	return CView::PreTranslateMessage(pmsg);
}

// ***************************************************************************

void CDisplay::updateCursor ()
{
	if (!_ValidMouse)
	{
		// Choose a cursor
		HCURSOR cursor = LoadCursor (NULL, IDC_ARROW);

		bool ctrlPushed = ( ( GetAsyncKeyState (VK_LCONTROL) | GetAsyncKeyState (VK_RCONTROL) ) & 0x8000 ) != 0;

		// No click 
		switch (_MouseMode)
		{
		case Nothing:
			{
				// Mode prim ?
				if (_MainFrame->_Mode == 1)
				{
					CVector pickPosMin ((float)_LastMousePos.x, (float)(_WindowHeight-_LastMousePos.y-1), 0);
					pixelToWorld (pickPosMin);

					bool actionActived = false;

					// Add sub element vertex mode ?
					bool addPointSub = (_MainFrame->getTransformMode () == CMainFrame::AddPoint) && (_MainFrame->isSelectionLocked ());
					if ( !addPointSub )
					{
						if (ctrlPushed)
						{
							cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_COPY));
						}
						else
						{
							// Pick a primitive that exist
							std::vector<class CDatabaseLocatorPointer> result;
							if (pickPoint (pickPosMin, result, false))
							{
								if (_MainFrame->getTransformMode () != CMainFrame::Select)
									cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_MOVE + _MainFrame->getTransformMode () - 1));
								actionActived = true;
							}
						}

						// Over something selected ?
						/*result.clear ();
						if (pickPoint (pickPosMin, result, true))
						{
							// Translate or rotate ?
							switch (_MainFrame->getTransformMode ())
							{
							case CMainFrame::Move:
								{
								}
								break;
							case CMainFrame::Rotate:
								{
								}
								break;
							case CMainFrame::Turn:
								{
								}
								break;
							case CMainFrame::Scale:
								{
								}
								break;
							}
							actionActived = true;
						}*/
					}
					else
					{
						if (_MainFrame->isSelectionLocked ())
						{
							cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_ADD_POINT));
						}
					}
					
					// No action actived ?
					if (!actionActived)
					{
						// *** Try to add vertices in sub mode

						// Are we in sub mode ?
						if (_MainFrame->isSelectionLocked ())
						{
							// Pick the vertex before the clicked edge
							std::vector<class CDatabaseLocatorPointer> result;
							if (pickEdgePoint (pickPosMin, result))
							{
								cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_INSERT_POINT));
							}
						}
					}
				}
			}
			break;

		// Moved with middle click ?
		case DragView:
			cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_HAND));
			break;

		// Moved with left click ?
		case TransformPrimitive:
			{
				if (_TranformAction)
				{
					switch (_MainFrame->getTransformMode ())
					{
						case CMainFrame::Move:
							cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_MOVE));
							break;
						case CMainFrame::Rotate:
							cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_ROTATE));
							break;
						case CMainFrame::Turn:
							cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_TURN));
							break;
						case CMainFrame::Scale:
							cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_SCALE));
							break;
						case CMainFrame::Radius:
							cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_RADIUS));
							break;
					}
				}
				else
				{
					if (ctrlPushed)
						cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_SELECT_COPY));
					else
						cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_SELECT));
				}
			}
			break;

		// Zoom ?
		case ZoomView:
			cursor = theApp.LoadCursor (MAKEINTRESOURCE(IDC_ZOOM));
			break;
		}

		_ValidMouse = true;

		// Set the cursor
		SetCursor (cursor);
	}
}

// ***************************************************************************

void CDisplay::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Invalidate cursor
	invalidateCursor ();

	// Update data
	_MainFrame->updateData ();
}

// ***************************************************************************

void CDisplay::setDisplayRegion (const CVector &min, const CVector &max)
{
	// Set the new offset
	_Offset = (min+max)/2;

	// Clac new factor
	_Factor = std::max ((min.x-_Offset.x)/_InitViewMin.x, (min.y-_Offset.y)/_InitViewMin.y);
	NLMISC::clamp (_Factor, 0.015625f, 8.0f);

	// Recalc view parameters
	calcCurView();

	// Invalidate
	invalidateCursor ();

	// Invalidate left view
	invalidateLeftView ();
}

// ***************************************************************************

void CDisplay::setDisplayCenter (const CVector &center, bool zoom)
{
	// Set the new offset
	_Offset = center;

	if (zoom)
	{
		// set min view factor 
		_Factor = 0.015625f;
	}

	// Recalc view parameters
	calcCurView();

	// Invalidate
	invalidateCursor ();

	// Invalidate left view
	invalidateLeftView ();
}


// ***************************************************************************

void CDisplay::DrawCollisionTexture(sint32 count, float x1, float y1)
{
	if (DontUse3D)
		return;

	if ((x1 >= 0) && (x1 <= 255*_CellSize) && (y1 <= 0) && (y1 >= -255*_CellSize))
	{
		sint32 x2, y2;
		x2 = (sint32)floor(x1 / _CellSize);
		y2 = (sint32)floor(y1 / _CellSize);
		
		if (x2 != _lastXCollisionTexture[count] || y2 != _lastYCollisionTexture[count])
		{
			// lookup in cache to see if the texture is already there
			std::list<TCollisionTextureCache>::iterator first(_collisionTextureCache.begin()), last(_collisionTextureCache.end());
			CTextureFile *tex = 0;
			for (; first != last; ++first)
			{
				if (first->PosX == x2 && first->PosY == y2)
				{
					// we found it, bring it in first place in the vector
					tex = first->Texture;
	
					_collisionTextureCache.push_front(*first);
					_collisionTextureCache.erase(first);
					break;
				}
			}
			
			// load the texture
			_lastXCollisionTexture[count] = x2;
			_lastYCollisionTexture[count] = y2;
			string Name;
			getZoneNameFromXY(x2, y2, Name);
			_collisionTexture[count] = NULL;
			if (tex != NULL)
			{
				_collisionTexture[count] = tex;
			}
			else
			{
				string sDirBackup = NLMISC::CPath::getCurrentPath();

				string dir = getDocument ()->getDataDir ();
				if (dir.empty()) dir = _MainFrame->_ExeDir;
				dir += "\\collisionmap\\";
				SetCurrentDirectory (dir.c_str());

				if(NLMISC::CFile::fileExists(Name+".tga") || NLMISC::CFile::fileExists(Name+".png"))
				{
					string filename = getTextureFile(Name);

					_collisionTexture[count] = new CTextureFile;
					_collisionTexture[count]->setUploadFormat(ITexture::RGB565);
					// we don't want compression
					_collisionTexture[count]->setAllowDegradation(false);
					_collisionTexture[count]->setFilterMode(ITexture::Nearest,ITexture::NearestMipMapOff);
				
					_collisionTexture[count]->setFileName (filename);
					_collisionTexture[count]->setReleasable (true);
					_collisionTexture[count]->generate ();

					// store the texture in cache
					if (_collisionTextureCache.size() > COLLISION_TEXTURE_CACHE_SIZE)
						_collisionTextureCache.pop_back();
					TCollisionTextureCache ctc;
					ctc.PosX = x2;
					ctc.PosY = y2;
					ctc.Texture = _collisionTexture[count];
						
					_collisionTextureCache.push_front(ctc);
				}

				NLMISC::CPath::setCurrentPath(sDirBackup);
			}
		}
		
		// Create a material
		CMaterial material;
		material.initUnlit ();
		material.setTexture (0, _collisionTexture[count]);
		material.setZFunc (CMaterial::always);
		material.setZWrite (false);
		
		// Blend function
		material.setBlend (true);
		//material.setBlendFunc (CMaterial::srcalpha, CMaterial::invsrcalpha);
		//material.setBlendFunc (CMaterial::srcalpha, CMaterial::one);
		material.setBlendFunc (CMaterial::one, CMaterial::one);
		
		float minx = x2*_CellSize -0.5f;
		float miny = y2*_CellSize - 0.5f;
		float maxx = minx + _CellSize;
		float maxy = miny + _CellSize;
		
#define PIXEL_RATIO (160.0/256.0)
		
		// Create the vertices
		std::vector<NLMISC::CTriangleUV> vertices (2);
		vertices[0].V0.x = minx;
		vertices[0].V0.y = maxy;
		vertices[0].V0.z = 0;
		vertices[0].Uv0.U = 0;
		vertices[0].Uv0.V = 0;
		vertices[0].V1.x = minx;
		vertices[0].V1.y = miny;
		vertices[0].V1.z = 0;
		vertices[0].Uv1.U = 0;
		vertices[0].Uv1.V = PIXEL_RATIO;
		vertices[0].V2.x = maxx;
		vertices[0].V2.y = miny;
		vertices[0].V2.z = 0;
		vertices[0].Uv2.U = PIXEL_RATIO;
		vertices[0].Uv2.V = PIXEL_RATIO;
		vertices[1].V0.x = maxx;
		vertices[1].V0.y = miny;
		vertices[1].V0.z = 0;
		vertices[1].Uv0.U = PIXEL_RATIO;
		vertices[1].Uv0.V = PIXEL_RATIO;
		vertices[1].V1.x = maxx;
		vertices[1].V1.y = maxy;
		vertices[1].V1.z = 0;
		vertices[1].Uv1.U = PIXEL_RATIO;
		vertices[1].Uv1.V = 0;
		vertices[1].V2.x = minx;
		vertices[1].V2.y = maxy;
		vertices[1].V2.z = 0;
		vertices[1].Uv2.U = 0;
		vertices[1].Uv2.V = 0;
		
		// Display it
		CDRU::drawTrianglesUnlit (vertices, material, *CNELU::Driver);
	}
}

// ***************************************************************************

void CDisplay::updateMouseMode (UINT nFlags)
{
	// Zoom ?
	if (_MouseMidDown || ((_MouseLeftDown) && ((GetAsyncKeyState (VK_MENU) ) & 0x8000)))
	{
		_MouseMode = DragView;
		return;
	}

	if ((_MouseLeftDown) && (nFlags&MK_SHIFT))
	{
		_MouseMode = ZoomView;
		_OrigineZoom = _CurPos;
		return;
	}

	if (_MouseRightDown)
	{
		// Choose the mode
		switch (_MainFrame->_Mode)
		{
		case 0:
			_MouseMode = RemoveTiles;
			return;
		case 1:
			_MouseMode = MenuPrimitive;
			return;
		case 3:
			_MouseMode = PluginMove;
			return;
		}
	}

	if (_MouseLeftDown)
	{
		// Choose the mode
		switch (_MainFrame->_Mode)
		{
		case 0:
			_MouseMode = DrawTiles;
			return;
		case 1:
			_MouseMode = TransformPrimitive;
			return;
		case 2:
			_MouseMode = DrawTransition;
			return;
		case 3:
			_MouseMode = PluginMove;
			return;
		}
	}

	_MouseMode = Nothing;
}

// ***************************************************************************

void CDisplay::setFactor (float newFactor, bool follow, const CVector &position)
{
	static const float factorMin = 0.015625f;
	static const float factorMax = 8.0f;
	static const float logFactorMin = 0.015625f; //(float)log(0.015625f);
	static const float logFactorMax = 8.0f; //(float)log(8.0f);
	NLMISC::clamp (newFactor, factorMin, factorMax);

	// Follow the zoom with a drag ?
	if (follow)
	{
		// Position delta
		/*float oldPos = ((float)log(_Factor) - logFactorMin) / (logFactorMax-logFactorMin);
		float newPos = ((float)log(newFactor) - logFactorMin) / (logFactorMax-logFactorMin);*/
		float oldPos = (_Factor - logFactorMin) / (logFactorMax-logFactorMin);
		float newPos = (newFactor - logFactorMin) / (logFactorMax-logFactorMin);

		// Source position
		if (oldPos == 0)
		{
			_Offset = position;
		}
		else
		{
			CVector src = (_Offset - position) / oldPos + position;

			_Offset = (position - src) * (1.f-newPos) + src;
		}
	}

	_Factor = newFactor;

	invalidateLeftView ();
}

// ***************************************************************************

void CDisplay::displayPoints ()
{
	if (DontUse3D)
		return;

	if (CNELU::Driver)
	{
		// Size of the pixel, must be > 0.5 meter
		uint32 width, height;
		sint32	step = 1;
		float	xsize = 1.0f;
		CNELU::Driver->getWindowSize (width, height);
		if (width == 0)
			return;
		if ( (_CurViewMax.x-_CurViewMin.x)/(float)width > 0.25f*16*16 )
			return;
		if ( (_CurViewMax.x-_CurViewMin.x)/(float)width > 0.25f*16 )
		{
			step = 256;
			xsize = 64.0f;
		}
		else if ( (_CurViewMax.x-_CurViewMin.x)/(float)width > 0.25f )
		{
			step = 16;
			xsize = 4.0f;
		}

		{
			initRenderProxy ();
			// Setup matrix
			setupWorldMatrixAndFrustum ();

			sint startX = (sint)(_CurViewMin.x) & (~(step-1));
			sint startY = (sint)(_CurViewMin.y) & (~(step-1));
			sint endX = (sint)(_CurViewMax.x) & (~(step-1));
			sint endY = (sint)(_CurViewMax.y) & (~(step-1));
			if (startX > endX) swap(startX, endX);
			if (startY > endY) swap(startY, endY);
			endX++;
			endY++;
			sint x, y;
			for (y=startY; y<endY; y+=step)
			for (x=startX; x<endX; x+=step)
			{
				if ((x&0xff)==0 && (y&0xff)==0)
				{
					if (step < 256)
					{
						if (y-256 < startY)
							lineRenderProxy(CRGBA (255,255,0,90), CVector (float(x), y-256.0f,0.f), CVector (float(x), y+256.0f,0.f), 0);
						else
							lineRenderProxy(CRGBA (255,255,0,90), CVector (float(x), float(y),0.f), CVector (float(x), y+256.0f,0.f), 0);
						if (x-256 < startX)
							lineRenderProxy(CRGBA (255,255,0,90), CVector (x-256.0f, float(y),0.f), CVector (x+256.0f, float(y),0.f), 0);
						else
							lineRenderProxy(CRGBA (255,255,0,90), CVector (float(x), float(y),0.f), CVector (x+256.0f, float(y),0.f), 0);
//						lineRenderProxy(CRGBA (255,255,0,255), CVector (x+xsize*0.5f, float(y),0.f), CVector (x-xsize*0.5f, float(y),0.f), 0);
//						lineRenderProxy(CRGBA (255,255,0,255), CVector (float(x), y-xsize*0.5f,0.f), CVector (float(x), y+xsize*0.5f,0.f), 0);
//						lineRenderProxy(CRGBA (255,255,0,50), CVector (float(x), float(y),0.f), CVector (x+256.0f, float(y),0.f), 0);
//						lineRenderProxy(CRGBA (255,255,0,50), CVector (float(x), float(y),0.f), CVector (float(x), y+256.0f,0.f), 0);
					}
					else
						pointRenderProxy (CRGBA (0,0,0,255), CVector ((float)x, (float)y, 0));
				}
				else if ((x&0xf)==0 && (y&0xf)==0)
				{
					if (step < 16)
					{
						if (y-16< startY)
							lineRenderProxy((x&0xff)==0 ? CRGBA (255,255,0,90) : CRGBA (255,0,0,70), CVector (float(x), y-16.0f,0.f), CVector (float(x), y+16.0f,0.f), 0);
						else
							lineRenderProxy((x&0xff)==0 ? CRGBA (255,255,0,90) : CRGBA (255,0,0,70), CVector (float(x), float(y),0.f), CVector (float(x), y+16.0f,0.f), 0);
						if (x-16< startX)
							lineRenderProxy((y&0xff)==0 ? CRGBA (255,255,0,90) : CRGBA (255,0,0,70), CVector (x-16.0f, float(y),0.f), CVector (x+16.0f, float(y),0.f), 0);
						else
							lineRenderProxy((y&0xff)==0 ? CRGBA (255,255,0,90) : CRGBA (255,0,0,70), CVector (float(x), float(y),0.f), CVector (x+16.0f, float(y),0.f), 0);
//						lineRenderProxy(CRGBA (255,0,0,255), CVector (x+xsize*0.25f, float(y),0.f), CVector (x-xsize*0.25f, float(y),0.f), 0);
//						lineRenderProxy(CRGBA (255,0,0,255), CVector (float(x), y-xsize*0.25f,0.f), CVector (float(x), y+xsize*0.25f,0.f), 0);
//						lineRenderProxy(CRGBA (255,0,0,50), CVector (float(x), float(y),0.f), CVector (x+16.0f, float(y),0.f), 0);
//						lineRenderProxy(CRGBA (255,0,0,50), CVector (float(x), float(y),0.f), CVector (float(x), y+16.0f,0.f), 0);
					}
					else
						pointRenderProxy (CRGBA (0,0,0,255), CVector ((float)x, (float)y, 0));
				}
				else
					pointRenderProxy (CRGBA (0,0,0,255), CVector ((float)x, (float)y, 0));
			}
			flushRenderProxy ();
		}
	}
}

// ***************************************************************************

bool CDisplay::getActionText (string &text)
{
	if (_TranformAction)
		return _TranformAction->getText (text);
	return false;
}

// ***************************************************************************

bool CDisplay::getActionHelp (string &text)
{
	if (_TranformAction)
		return _TranformAction->getHelp (text);
	return false;
}

// ***************************************************************************

void CDisplay::setLayerTexture(uint primitiveLayer, CPrimTexture *tex)
{
	nlassert(primitiveLayer < NUM_PRIM_LAYER);
	_ProxyTexture[primitiveLayer] = tex;
}


// ***************************************************************************

void CPrimTexture::buildFromFile(const std::string &filename)
{
	// Load tex size
	std::string path = CPath::lookup(filename, false, false);
	_Width = _Height = 0;
	if (!path.empty())
	{
		try
		{
			uint32 width, height;
			NLMISC::CBitmap::loadSize(path, width, height);
			_Width = (uint) width;
			_Height = (uint) height;
		}
		catch (NLMISC::EStream)
		{
			nlwarning("Couldn't retrieve size for %s", filename.c_str());
		}
	}	
	NL3D::CTextureFile *tex = new NL3D::CTextureFile(filename);
	_Texture = tex;
	_Texture->setFilterMode(NL3D::ITexture::Nearest, NL3D::ITexture::NearestMipMapOff);	
}


// ***************************************************************************

void CPrimTexture::buildFromNLBitmap(const NLMISC::CBitmap &bm)
{
	_Width = _Height = 0;
	class CTextureFromNLBitmap : public NL3D::ITexture
	{
	public:
		NLMISC_DECLARE_CLASS(CTextureFromNLBitmap);
		NLMISC::CBitmap _SrcBM;
		virtual void doGenerate(bool async = false)
		{
			(NLMISC::CBitmap &) *this = _SrcBM;
		}
	};
	CTextureFromNLBitmap *tfbm = new CTextureFromNLBitmap;
	tfbm->_SrcBM = bm;
	_Texture = tfbm;
	_Texture->setFilterMode(NL3D::ITexture::Nearest, NL3D::ITexture::NearestMipMapOff);	
	_Width = bm.getWidth();
	_Height = bm.getHeight();
}




