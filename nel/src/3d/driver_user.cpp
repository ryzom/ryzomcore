// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
// Copyright (C) 2013-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
// Copyright (C) 2015  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

// ***************************************************************************
// THIS FILE IS DIVIDED IN TWO PARTS BECAUSE IT MAKES VISUAL CRASH.
// fatal error C1076: compiler limit : internal heap limit reached; use /Zm to specify a higher limit
// ***************************************************************************

#include "nel/3d/driver_user.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/dru.h"
#include "nel/3d/scene.h"
#include "nel/3d/text_context_user.h"
#include "nel/3d/texture_user.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/scene_user.h"
#include "nel/3d/init_3d.h"
#include "nel/3d/water_env_map_user.h"
#include "nel/3d/water_pool_manager.h"
#include "nel/3d/u_camera.h"
#include "nel/3d/debug_vb.h"

#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/event_emitter.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


H_AUTO_DECL( NL3D_UI_Driver )
H_AUTO_DECL( NL3D_Render_DriverClearBuffer )
H_AUTO_DECL( NL3D_Render_DriverSwapBuffer )
H_AUTO_DECL( NL3D_Texture_Driver )

#define	NL3D_HAUTO_UI_DRIVER				H_AUTO_USE( NL3D_UI_Driver )
#define	NL3D_HAUTO_CLEAR_DRIVER				H_AUTO_USE( NL3D_Render_DriverClearBuffer )
#define	NL3D_HAUTO_SWAP_DRIVER				H_AUTO_USE( NL3D_Render_DriverSwapBuffer )
#define	NL3D_HAUTO_TEX_DRIVER				H_AUTO_USE( NL3D_Texture_Driver )

// ***************************************************************************
// ***************************************************************************
// UDriver implementation (gcc link bug force to not put them in .h).
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
UDriver::UDriver()
{
}
// ***************************************************************************
UDriver::~UDriver()
{
	purgeMemory();
}

// ***************************************************************************
void					UDriver::setMatrixMode2D11()
{
	setMatrixMode2D(CFrustum(0.0f,1.0f,0.0f,1.0f,-1.0f,1.0f,false));
}

// ***************************************************************************
void					UDriver::setMatrixMode2D43()
{
	setMatrixMode2D(CFrustum(0.0f,4.0f/3.0f,0.0f,1.0f,-1.0f,1.0f,false));
}

// ***************************************************************************
UDriver					*UDriver::createDriver(uintptr_t windowIcon, bool direct3d, emptyProc exitFunc)
{
	return new CDriverUser (windowIcon, direct3d ? CDriverUser::Direct3d:CDriverUser::OpenGl, exitFunc);
}

// ***************************************************************************
UDriver					*UDriver::createDriver(uintptr_t windowIcon, TDriver driver, emptyProc exitFunc)
{
	return new CDriverUser (windowIcon, (CDriverUser::TDriver)driver, exitFunc);
}

// ***************************************************************************
void					UDriver::purgeMemory()
{
	CPointLight::purge ();
	GetWaterPoolManager().reset();
	contReset(CLandscapeGlobals::PassTriArray);
}


// ***************************************************************************
// ***************************************************************************
// WINDOWS Management.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CDriverUser::CDriverUser (uintptr_t windowIcon, TDriver driver, emptyProc exitFunc)
{
	// The enum of IDriver and UDriver MUST be the same!!!
	nlassert((uint)IDriver::idCount == (uint)UDriver::idCount);
	nlassert((uint)IDriver::typeCount == (uint)UDriver::typeCount);
	nlassert((uint)IDriver::iconCount == (uint)UDriver::iconCount);


	NL3D::init3d();

	_Driver = NULL;

	// Create/Init Driver.
#if defined(NL_OS_WINDOWS)
	if (driver == Direct3d)
		_Driver= CDRU::createD3DDriver();
#endif

	if (!_Driver && driver == OpenGl)
		_Driver= CDRU::createGlDriver();

	if (!_Driver && driver == OpenGlEs)
		_Driver= CDRU::createGlEsDriver();

	nlassert(_Driver);
	_Driver->init (windowIcon, exitFunc);

	_WindowInit= false;

	// Init of VBuffers/PBs for 2D/3D interface.
	_VBFlat.setVertexFormat(CVertexBuffer::PositionFlag);
	_VBColor.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag);
	_VBUv.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
	_VBColorUv.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag);
	_VBQuadsColUv.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag);
	_VBTrisColUv.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag);
	_VBQuadsColUv2.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag);
	// max is quad.
	_VBFlat.setNumVertices(4);
	_VBColor.setNumVertices(4);
	_VBUv.setNumVertices(4);
	_VBColorUv.setNumVertices(4);
	// memory management
	_VBFlat.setPreferredMemory(CVertexBuffer::RAMVolatile, false);
	_VBColor.setPreferredMemory(CVertexBuffer::RAMVolatile, false);
	_VBUv.setPreferredMemory(CVertexBuffer::RAMVolatile, false);
	_VBColorUv.setPreferredMemory(CVertexBuffer::RAMVolatile, false);
	_VBQuadsColUv.setPreferredMemory (CVertexBuffer::RAMVolatile, false);
	_VBTrisColUv.setPreferredMemory (CVertexBuffer::RAMVolatile, false);
	_VBQuadsColUv2.setPreferredMemory (CVertexBuffer::RAMVolatile, false);
	// names
	_VBFlat.setName("_VBFlat");
	_VBColor.setName("_VBColor");
	_VBUv.setName("_VBUv");
	_VBColorUv.setName("_VBColorUv");
	_VBQuadsColUv.setName("_VBQuadsColUv");
	_VBTrisColUv.setName("_VBTrisColUv");
	_VBQuadsColUv2.setName("_VBQuadsColUv2");

	_PBLine.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	_PBLine.setNumIndexes(2);
	CIndexBufferReadWrite iba;
	_PBLine.lock (iba);
	iba.setLine(0, 0, 1);
	_PBTri.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	_PBTri.setNumIndexes(3);
	_PBTri.lock (iba);
	iba.setTri(0, 0, 1, 2);

	_RenderTargetManager.m_Driver = this;
	_ShapeBank._DriverUser = this;

	_EffectRenderTarget = NULL;

	NL_SET_IB_NAME(_PBLine, "CDriverUser::_PBLine");
	NL_SET_IB_NAME(_PBTri, "CDriverUser::_PBTri");
}
// ***************************************************************************
CDriverUser::~CDriverUser()
{
	release();

	delete _Driver;
	_Driver= NULL;
}

// ***************************************************************************
void			CDriverUser::disableHardwareVertexProgram()
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->disableHardwareVertexProgram();
}
void			CDriverUser::disableHardwarePixelProgram()
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->disableHardwarePixelProgram();
}
void			CDriverUser::disableHardwareVertexArrayAGP()
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->disableHardwareVertexArrayAGP();
}
void			CDriverUser::disableHardwareTextureShader()
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->disableHardwareTextureShader();
}

// ***************************************************************************
bool			CDriverUser::setDisplay(const CMode &mode, bool show, bool resizeable)
{
	NL3D_HAUTO_UI_DRIVER;

	return setDisplay(EmptyWindow, mode, show, resizeable);
}

// ***************************************************************************
bool			CDriverUser::setDisplay(nlWindow wnd, const CMode &mode, bool show, bool resizeable)
{
	NL3D_HAUTO_UI_DRIVER;

	// window init.
	if (_Driver->setDisplay(wnd, GfxMode(mode.Width, mode.Height, mode.Depth, mode.Windowed, false, mode.Frequency, mode.AntiAlias, mode.DisplayDevice), show, resizeable))
	{
		// Always true
		nlverify (activate());

		_WindowInit= true;

		// Event init.
		AsyncListener.reset ();
		EventServer.addEmitter(_Driver->getEventEmitter());
		AsyncListener.addToServer(EventServer);

		// Matrix Context (2D).
		_CurrentMatrixContext.Viewport.initFullScreen();
		_CurrentMatrixContext.Scissor.initFullScreen();
		setMatrixMode2D11();

		// 2D Material.
		_MatFlat.attach (&_MatFlatInternal);
		_MatFlat.initUnlit();
		_MatFlat.setZFunc(UMaterial::always);
		_MatFlat.setZWrite(false);
		_MatText.attach (&_MatFlatInternal);
		_MatText.initUnlit();
		_MatText.setZFunc(UMaterial::always);
		_MatText.setZWrite(false);
		_MatStretchText.attach (&_MatTextStretchInternal);
		_MatStretchText.initUnlit();
		_MatStretchText.setZFunc(UMaterial::always);
		_MatStretchText.setZWrite(false);
		_MatTextStretchInternal.setDoubleSided(true);

		// make CTextureFile aware if 3d driver support non power of two textures
		NL3D::CTextureFile::setSupportNonPowerOfTwoTextures(_Driver->supportNonPowerOfTwoTextures());

		// Done
		return true;
	}
	return false;
}

// ***************************************************************************
bool CDriverUser::setMode(const CMode& mode)
{
	return _Driver->setMode(GfxMode(mode.Width, mode.Height, mode.Depth, mode.Windowed, false, mode.Frequency, mode.AntiAlias, mode.DisplayDevice));
}

// ----------------------------------------------------------------------------
struct CModeSorter
{
	bool operator()(const UDriver::CMode &mode1, const UDriver::CMode &mode2) const
	{
		if (mode1.Width == mode2.Width)
		{
			if (mode1.Height == mode2.Height) return mode1.Frequency < mode2.Frequency;

			return mode1.Height < mode2.Height;
		}

		return mode1.Width < mode2.Width;
	}
};

// ***************************************************************************
bool CDriverUser::getModes(std::vector<CMode> &modes)
{
	std::vector<GfxMode> vTmp;
	bool res = _Driver->getModes(vTmp);
	modes.clear();
	for (uint i = 0; i < vTmp.size(); ++i)
		modes.push_back(CMode(vTmp[i].Width, vTmp[i].Height, vTmp[i].Depth, vTmp[i].Windowed, vTmp[i].Frequency, vTmp[i].AntiAlias, vTmp[i].DisplayDevice));

	std::sort(modes.begin(), modes.end(), CModeSorter());

	return res;
}

// ***************************************************************************
bool CDriverUser::getCurrentScreenMode(CMode &mode)
{
	GfxMode		gfxMode;
	bool		res= _Driver->getCurrentScreenMode(gfxMode);
	mode.Windowed= gfxMode.Windowed;
	mode.Width= gfxMode.Width;
	mode.Height= gfxMode.Height;
	mode.Depth= gfxMode.Depth;
	mode.Frequency= gfxMode.Frequency;
	mode.AntiAlias= gfxMode.AntiAlias;
	return res;
}

// ***************************************************************************
void CDriverUser::setWindowTitle(const ucstring &title)
{
	NL3D_HAUTO_UI_DRIVER;
	_Driver->setWindowTitle(title);
}

// ***************************************************************************
void CDriverUser::setWindowIcon(const std::vector<NLMISC::CBitmap> &bitmaps)
{
	NL3D_HAUTO_UI_DRIVER;
	_Driver->setWindowIcon(bitmaps);
}

// ***************************************************************************
void CDriverUser::setWindowPos(sint32 x, sint32 y)
{
	NL3D_HAUTO_UI_DRIVER;
	_Driver->setWindowPos(x, y);
}

// ***************************************************************************
void CDriverUser::showWindow(bool show)
{
	NL3D_HAUTO_UI_DRIVER;
	_Driver->showWindow(show);
}

// ***************************************************************************
void			CDriverUser::release()
{
	NL3D_HAUTO_UI_DRIVER;

	if(!_WindowInit)
		return;

	// 2D Material.
	_MatFlat.initUnlit();
	_MatText.initUnlit();
	_MatStretchText.initUnlit();

	// delete Texture, mat ... list.
	_Textures.clear();
	_TextContexts.clear();
	_Scenes.clear();

	// release event.
	AsyncListener.removeFromServer(EventServer);
	EventServer.removeEmitter(_Driver->getEventEmitter());

	// release window.
	_Driver->release();
	_WindowInit= false;
}


// ***************************************************************************
bool			CDriverUser::activate(void)
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->activate();
}

// ***************************************************************************
bool			CDriverUser::isActive()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->isActive();
}


// ***************************************************************************
nlWindow		CDriverUser::getDisplay ()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getDisplay ();
}



// ***************************************************************************
// ***************************************************************************
// Matrix context.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDriverUser::restoreMatrixContextMatrixOnly()
{
	NL3D_HAUTO_UI_DRIVER;

	CFrustum	&f= _CurrentMatrixContext.Frustum;
	_Driver->setFrustum(f.Left, f.Right, f.Bottom, f.Top, f.Near, f.Far, f.Perspective);
	_Driver->setupViewMatrix(_CurrentMatrixContext.ViewMatrix);
	_Driver->setupModelMatrix(_CurrentMatrixContext.ModelMatrix);
}

// ***************************************************************************
void			CDriverUser::setupMatrixContext()
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setupScissor(_CurrentMatrixContext.Scissor);
	_Driver->setupViewport(_CurrentMatrixContext.Viewport);
	CFrustum	&f= _CurrentMatrixContext.Frustum;
	_Driver->setFrustum(f.Left, f.Right, f.Bottom, f.Top, f.Near, f.Far, f.Perspective);
	_Driver->setupViewMatrix(_CurrentMatrixContext.ViewMatrix);
	_Driver->setupModelMatrix(_CurrentMatrixContext.ModelMatrix);
}
// ***************************************************************************
void			CDriverUser::setScissor(const CScissor &sc)
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.Scissor= sc;
	setupMatrixContext();
}
// ***************************************************************************
CScissor		CDriverUser::getScissor()
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.Scissor;
}
// ***************************************************************************
void			CDriverUser::setViewport(const CViewport &vp)
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.Viewport= vp;
	setupMatrixContext();
}
// ***************************************************************************
CViewport		CDriverUser::getViewport()
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.Viewport;
}
// ***************************************************************************
void			CDriverUser::setFrustum(const CFrustum &frust)
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.Frustum= frust;
	setupMatrixContext();
}
// ***************************************************************************
CFrustum		CDriverUser::getFrustum()
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.Frustum;
}
// ***************************************************************************
void			CDriverUser::setFrustumMatrix(CMatrix &frust)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setFrustumMatrix(frust);
}
// ***************************************************************************
CMatrix			CDriverUser::getFrustumMatrix()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getFrustumMatrix();
}
// ***************************************************************************

float			CDriverUser::getClipSpaceZMin() const
{
	return _Driver->getClipSpaceZMin();
}
// ***************************************************************************

void			CDriverUser::setViewMatrix(const CMatrix &mat)
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.ViewMatrix= mat;
	setupMatrixContext();
}
// ***************************************************************************
CMatrix			CDriverUser::getViewMatrix()
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.ViewMatrix;
}
// ***************************************************************************
void			CDriverUser::setModelMatrix(const CMatrix &mat)
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.ModelMatrix= mat;
	setupMatrixContext();
}
// ***************************************************************************
CMatrix			CDriverUser::getModelMatrix()
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.ModelMatrix;
}



// ***************************************************************************
void			CDriverUser::setMatrixMode2D(const CFrustum &frust)
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.Frustum= frust;
	// We still work in NL3D coordinates, so must convert y to z.
	CVector		I(1,0,0);
	CVector		J(0,0,1);
	CVector		K(0,-1,0);

	_CurrentMatrixContext.ViewMatrix.identity();
	_CurrentMatrixContext.ViewMatrix.setRot(I,J,K, true);
	_CurrentMatrixContext.ModelMatrix.identity();

	setupMatrixContext();
}
// ***************************************************************************
void			CDriverUser::setMatrixMode3D(UCamera &camera)
{
	NL3D_HAUTO_UI_DRIVER;

	// Retrieve the matrix and frustum from the camera.
	CMatrix		viewMat;
	viewMat= camera.getMatrix();
	viewMat.invert();
	setViewMatrix(viewMat);
	setModelMatrix(CMatrix::Identity);
	setFrustum(camera.getFrustum());
}

// ***************************************************************************
void CDriverUser::setDepthRange(float znear, float zfar)
{
	NL3D_HAUTO_UI_DRIVER;
	_Driver->setDepthRange(znear, zfar);
}

// ***************************************************************************
void CDriverUser::getDepthRange(float & znear, float & zfar)
{
	NL3D_HAUTO_UI_DRIVER;
	_Driver->getDepthRange(znear, zfar);
}

// ***************************************************************************
void CDriverUser::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
	_Driver->setColorMask(bRed, bGreen, bBlue, bAlpha);
}

// ***************************************************************************
// ***************************************************************************
// 2D/3D INTERFACE.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void			CDriverUser::drawLine(const NLMISC::CLine &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBFlat;
	CIndexBuffer		&pb= _PBLine;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->activeIndexBuffer(pb);
	_Driver->renderLines(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawLine(const NLMISC::CLineColor &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBColor;
	CIndexBuffer		&pb= _PBLine;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setColor(0, shp.Color0);
		vba.setColor(1, shp.Color1);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->activeIndexBuffer(pb);
	_Driver->renderLines(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawLine(const NLMISC::CLineUV &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBUv;
	CIndexBuffer		&pb= _PBLine;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setTexCoord (0, 0, shp.Uv0);
		vba.setTexCoord (1, 0, shp.Uv1);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->activeIndexBuffer(pb);
	_Driver->renderLines(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawLine(const NLMISC::CLineColorUV &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBColorUv;
	CIndexBuffer		&pb= _PBLine;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setColor(0, shp.Color0);
		vba.setColor(1, shp.Color1);
		vba.setTexCoord (0, 0, shp.Uv0);
		vba.setTexCoord (1, 0, shp.Uv1);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->activeIndexBuffer(pb);
	_Driver->renderLines(convMat(mat), 0, 1);
}



// ***************************************************************************
void			CDriverUser::drawTriangle(const NLMISC::CTriangle &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBFlat;
	CIndexBuffer		&pb= _PBTri;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setVertexCoord (2, shp.V2);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->activeIndexBuffer(pb);
	_Driver->renderTriangles(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawTriangle(const NLMISC::CTriangleColor &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBColor;
	CIndexBuffer		&pb= _PBTri;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setVertexCoord (2, shp.V2);
		vba.setColor(0, shp.Color0);
		vba.setColor(1, shp.Color1);
		vba.setColor(2, shp.Color2);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->activeIndexBuffer(pb);
	_Driver->renderTriangles(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawTriangle(const NLMISC::CTriangleUV &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBUv;
	CIndexBuffer		&pb= _PBTri;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setVertexCoord (2, shp.V2);
		vba.setTexCoord (0, 0, shp.Uv0);
		vba.setTexCoord (1, 0, shp.Uv1);
		vba.setTexCoord (2, 0, shp.Uv2);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->activeIndexBuffer(pb);
	_Driver->renderTriangles(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawTriangle(const NLMISC::CTriangleColorUV &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBColorUv;
	CIndexBuffer		&pb= _PBTri;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setVertexCoord (2, shp.V2);
		vba.setColor(0, shp.Color0);
		vba.setColor(1, shp.Color1);
		vba.setColor(2, shp.Color2);
		vba.setTexCoord (0, 0, shp.Uv0);
		vba.setTexCoord (1, 0, shp.Uv1);
		vba.setTexCoord (2, 0, shp.Uv2);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->activeIndexBuffer(pb);
	_Driver->renderTriangles(convMat(mat), 0, 1);
}



// ***************************************************************************
void			CDriverUser::drawQuad(const NLMISC::CQuad &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBFlat;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setVertexCoord (2, shp.V2);
		vba.setVertexCoord (3, shp.V3);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->renderRawQuads(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawQuad(const NLMISC::CQuadColor &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBColor;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setVertexCoord (2, shp.V2);
		vba.setVertexCoord (3, shp.V3);
		vba.setColor(0, shp.Color0);
		vba.setColor(1, shp.Color1);
		vba.setColor(2, shp.Color2);
		vba.setColor(3, shp.Color3);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->renderRawQuads(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawQuad(const NLMISC::CQuadUV &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBUv;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setVertexCoord (2, shp.V2);
		vba.setVertexCoord (3, shp.V3);
		vba.setTexCoord (0, 0, shp.Uv0);
		vba.setTexCoord (1, 0, shp.Uv1);
		vba.setTexCoord (2, 0, shp.Uv2);
		vba.setTexCoord (3, 0, shp.Uv3);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->renderRawQuads(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawQuad(const NLMISC::CQuadColorUV &shp, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb= _VBColorUv;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		vba.setVertexCoord (0, shp.V0);
		vba.setVertexCoord (1, shp.V1);
		vba.setVertexCoord (2, shp.V2);
		vba.setVertexCoord (3, shp.V3);
		vba.setColor(0, shp.Color0);
		vba.setColor(1, shp.Color1);
		vba.setColor(2, shp.Color2);
		vba.setColor(3, shp.Color3);
		vba.setTexCoord (0, 0, shp.Uv0);
		vba.setTexCoord (1, 0, shp.Uv1);
		vba.setTexCoord (2, 0, shp.Uv2);
		vba.setTexCoord (3, 0, shp.Uv3);
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->renderRawQuads(convMat(mat), 0, 1);
}
// ***************************************************************************
void			CDriverUser::drawQuads(const std::vector<NLMISC::CQuadColorUV> &q, UMaterial &mat)
{
	H_AUTO2;

	const CQuadColorUV *qptr = &(q[0]);
	drawQuads(qptr , (uint32)q.size(), mat);
}

// ***************************************************************************
void			CDriverUser::drawQuads(const std::vector<NLMISC::CQuadColorUV2> &q, UMaterial &mat)
{
	H_AUTO2;

	const CQuadColorUV2 *qptr = &(q[0]);
	drawQuads(qptr , (uint32)q.size(), mat);
}

// ***************************************************************************
void			CDriverUser::drawQuads(const NLMISC::CQuadColorUV *quads, uint32 nbQuads, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb = _VBQuadsColUv;

	vb.setNumVertices (4*nbQuads);
	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);

		uint8	*dstPtr= (uint8*)vba.getVertexCoordPointer();
		uint32	colorOfs= vb.getColorOff();
		uint32	uvOfs= vb.getTexCoordOff();
		uint32	vSize= vb.getVertexSize();

		if (vb.getVertexColorFormat() == CVertexBuffer::TRGBA)
		{
			for (uint32 i = 0; i < nbQuads; ++i)
			{
				const NLMISC::CQuadColorUV &qcuv = quads[i];
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V0;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs)= qcuv.Uv0;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= qcuv.Color0;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V1;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs)= qcuv.Uv1;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= qcuv.Color1;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V2;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs)= qcuv.Uv2;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= qcuv.Color2;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V3;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs)= qcuv.Uv3;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= qcuv.Color3;
				dstPtr+= vSize;
			}
		}
		else
		{
			for (uint32 i = 0; i < nbQuads; ++i)
			{
				const NLMISC::CQuadColorUV &qcuv = quads[i];
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V0;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs)= qcuv.Uv0;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= qcuv.Color0;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V1;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs)= qcuv.Uv1;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= qcuv.Color1;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V2;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs)= qcuv.Uv2;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CBGRA))
				*(CBGRA*)(dstPtr+colorOfs)= qcuv.Color2;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V3;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs)= qcuv.Uv3;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= qcuv.Color3;
				dstPtr+= vSize;
			}
		}
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->renderRawQuads(convMat(mat), 0, nbQuads);
}


// ***************************************************************************
void			CDriverUser::drawQuads(const NLMISC::CQuadColorUV2 *quads, uint32 nbQuads, UMaterial &mat)
{
	H_AUTO2;

	CVertexBuffer		&vb = _VBQuadsColUv2;

	vb.setNumVertices (4*nbQuads);
	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		uint8	*dstPtr= (uint8*)vba.getVertexCoordPointer();
		uint32	colorOfs= vb.getColorOff();
		uint32	uvOfs0= vb.getTexCoordOff(0);
		uint32	uvOfs1= vb.getTexCoordOff(1);
		uint32	vSize= vb.getVertexSize();

		if (vb.getVertexColorFormat() == CVertexBuffer::TRGBA)
		{
			for (uint32 i = 0; i < nbQuads; ++i)
			{
				const NLMISC::CQuadColorUV2 &qcuv = quads[i];
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V0;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= qcuv.Uv0;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs1, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs1)= qcuv.Uv02;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= qcuv.Color0;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V1;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= qcuv.Uv1;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs1, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs1)= qcuv.Uv12;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= qcuv.Color1;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V2;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= qcuv.Uv2;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs1, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs1)= qcuv.Uv22;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= qcuv.Color2;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V3;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= qcuv.Uv3;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs1, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs1)= qcuv.Uv32;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= qcuv.Color3;
				dstPtr+= vSize;
			}
		}
		else
		{
			for (uint32 i = 0; i < nbQuads; ++i)
			{
				const NLMISC::CQuadColorUV2 &qcuv = quads[i];
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V0;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= qcuv.Uv0;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs1, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs1)= qcuv.Uv02;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= qcuv.Color0;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V1;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= qcuv.Uv1;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs1, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs1)= qcuv.Uv12;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= qcuv.Color1;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V2;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= qcuv.Uv2;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs1, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs1)= qcuv.Uv22;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= qcuv.Color2;
				dstPtr+= vSize;
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= qcuv.V3;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= qcuv.Uv3;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs1, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs1)= qcuv.Uv32;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= qcuv.Color3;
				dstPtr+= vSize;
			}
		}
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->renderRawQuads(convMat(mat), 0, nbQuads);
}


// ***************************************************************************
void CDriverUser::drawTriangles(const std::vector<NLMISC::CTriangleColorUV> &tris, UMaterial &mat)
{
	drawTriangles(&tris[0], (uint32) tris.size(), mat);
}

// ***************************************************************************
void CDriverUser::drawTriangles(const NLMISC::CTriangleColorUV *tris, uint32 nbTris, UMaterial &mat)
{
	if (nbTris == 0) return;
	CVertexBuffer		&vb= _VBTrisColUv;
	vb.setNumVertices (3*nbTris);
	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		uint8	*dstPtr= (uint8*)vba.getVertexCoordPointer();
		uint32	colorOfs= vb.getColorOff();
		uint32	uvOfs0= vb.getTexCoordOff(0);
		uint32	vSize= vb.getVertexSize();
		const NLMISC::CTriangleColorUV *lastTri = tris + nbTris;
		if (vb.getVertexColorFormat() == CVertexBuffer::TRGBA)
		{
			do
			{
				//
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= tris->V0;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= tris->Uv0;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= tris->Color0;
				dstPtr+= vSize;
				//
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= tris->V1;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= tris->Uv1;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= tris->Color1;
				dstPtr+= vSize;
				//
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= tris->V2;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= tris->Uv2;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CRGBA*)(dstPtr+colorOfs)= tris->Color2;
				dstPtr+= vSize;
				++ tris;
			}
			while (tris != lastTri);
		}
		else
		{
			do
			{
				//
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= tris->V0;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= tris->Uv0;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= tris->Color0;
				dstPtr+= vSize;
				//
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= tris->V1;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= tris->Uv1;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= tris->Color1;
				dstPtr+= vSize;
				//
				CHECK_VBA_RANGE(vba, dstPtr+0, sizeof(CVector))
				*(CVector*)(dstPtr+0)= tris->V2;
				CHECK_VBA_RANGE(vba, dstPtr+uvOfs0, sizeof(CUV))
				*(CUV*)(dstPtr+uvOfs0)= tris->Uv2;
				CHECK_VBA_RANGE(vba, dstPtr+colorOfs, sizeof(CRGBA))
				*(CBGRA*)(dstPtr+colorOfs)= tris->Color2;
				dstPtr+= vSize;
				++ tris;
			}
			while (tris != lastTri);
		}
	}

	_Driver->activeVertexBuffer(vb);
	_Driver->renderRawTriangles(convMat(mat), 0, nbTris);
}

// ***************************************************************************
// ***************************************************************************
// 2D TOOLS.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDriverUser::drawBitmap (float x, float y, float width, float height, class UTexture& texture, bool blend, CRGBA col)
{
	H_AUTO2;

	_MatText.setTexture(0, &texture);
	_MatText.setColor(col);
	_MatText.setBlend(blend);

	CQuadUV		quad;
	quad.V0.set(x,y,0);
	quad.V1.set(x+width,y,0);
	quad.V2.set(x+width,y+height,0);
	quad.V3.set(x,y+height,0);
	quad.Uv0.U= 0.f;
	quad.Uv0.V= 1.f;
	quad.Uv1.U= 1.f;
	quad.Uv1.V= 1.f;
	quad.Uv2.U= 1.f;
	quad.Uv2.V= 0.f;
	quad.Uv3.U= 0.f;
	quad.Uv3.V= 0.f;

	drawQuad(quad, _MatText);
}
// ***************************************************************************
void			CDriverUser::drawLine (float x0, float y0, float x1, float y1, CRGBA col)
{
	H_AUTO2;

	_MatFlat.setColor(col);
	_MatFlat.setBlend(true);

	CLine		line;
	line.V0.set(x0,y0,0);
	line.V1.set(x1,y1,0);

	drawLine(line, _MatFlat);
}
// ***************************************************************************
void			CDriverUser::drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, CRGBA col)
{
	H_AUTO2;

	_MatFlat.setColor(col);
	_MatFlat.setBlend(true);

	CTriangle		tri;
	tri.V0.set(x0,y0,0);
	tri.V1.set(x1,y1,0);
	tri.V2.set(x2,y2,0);

	drawTriangle(tri, _MatFlat);
}
// ***************************************************************************
void			CDriverUser::drawQuad (float x0, float y0, float x1, float y1, CRGBA col)
{
	H_AUTO2;

	_MatFlat.setColor(col);
	_MatFlat.setBlend(true);
	_MatFlat.setBlendFunc(UMaterial::srcalpha, UMaterial::invsrcalpha);

	CQuad		quad;
	quad.V0.set(x0,y0,0);
	quad.V1.set(x1,y0,0);
	quad.V2.set(x1,y1,0);
	quad.V3.set(x0,y1,0);

	drawQuad(quad, _MatFlat);
}
// ***************************************************************************
void			CDriverUser::drawQuad (float xcenter, float ycenter, float radius, CRGBA col)
{
	H_AUTO2;

	drawQuad(xcenter-radius, ycenter-radius, xcenter+radius, ycenter+radius, col);
}
// ***************************************************************************
void			CDriverUser::drawWiredQuad (float x0, float y0, float x1, float y1, CRGBA col)
{
	H_AUTO2;

	// v-left
	drawLine(x0,y0,x0,y1,col);
	// v-right
	drawLine(x1,y0,x1,y1,col);
	// h-up
	drawLine(x0,y1,x1,y1,col);
	// h-bottom
	drawLine(x0,y0,x1,y0,col);
}
// ***************************************************************************
void			CDriverUser::drawWiredQuad (float xcenter, float ycenter, float radius, CRGBA col)
{
	H_AUTO2;

	drawWiredQuad(xcenter-radius, ycenter-radius, xcenter+radius, ycenter+radius, col);
}


// ***************************************************************************
// ***************************************************************************
// MISC.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
UDriver::TMessageBoxId	CDriverUser::systemMessageBox (const char* message, const char* title, TMessageBoxType type, TMessageBoxIcon icon)
{

	IDriver::TMessageBoxId		dret;
	IDriver::TMessageBoxType	dtype= (IDriver::TMessageBoxType)(uint32)type;
	IDriver::TMessageBoxIcon	dicon= (IDriver::TMessageBoxIcon)(uint32)icon;
	dret= _Driver->systemMessageBox (message, title, dtype, dicon);

	return (UDriver::TMessageBoxId)(uint32)dret;
}


// ***************************************************************************
CMaterial		&CDriverUser::convMat(UMaterial &mat)
{

	return *mat.getObjectPtr();
}


// ***************************************************************************
void			CDriverUser::clearRGBABuffer(CRGBA col)
{
	NL3D_HAUTO_CLEAR_DRIVER;

	_Driver->clear2D(col);
}
// ***************************************************************************
void			CDriverUser::clearZBuffer()
{
	NL3D_HAUTO_CLEAR_DRIVER;

	_Driver->clearZBuffer();
}
// ***************************************************************************
void			CDriverUser::clearBuffers(CRGBA col)
{
	NL3D_HAUTO_CLEAR_DRIVER;

	_Driver->clear2D(col);
	_Driver->clearZBuffer();
	_Driver->clearStencilBuffer();
}
// ***************************************************************************
void			CDriverUser::swapBuffers()
{
	NL3D_HAUTO_SWAP_DRIVER;

	_Driver->swapBuffers();
	_RenderTargetManager.cleanup();
}

// ***************************************************************************
void CDriverUser::finish()
{
	NL3D_HAUTO_SWAP_DRIVER;

	_Driver->finish();
}

// ***************************************************************************
void CDriverUser::flush()
{
	NL3D_HAUTO_SWAP_DRIVER;

	_Driver->flush();
}

// ***************************************************************************
bool			CDriverUser::fogEnabled()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->fogEnabled();
}
// ***************************************************************************
void			CDriverUser::enableFog(bool enable)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->enableFog(enable);
}
// ***************************************************************************
void			CDriverUser::setupFog(float start, float end, CRGBA color)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setupFog(start, end, color);
}


// ***************************************************************************
void			CDriverUser::setLight (uint8 num, const ULight& light)
{
	NL3D_HAUTO_UI_DRIVER;

	CLightUser	*plight= (CLightUser*)&light;
	_Driver->setLight (num, plight->_Light);
}
// ***************************************************************************
void			CDriverUser::enableLight (uint8 num, bool enable)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->enableLight (num, enable);
}
// ***************************************************************************
void			CDriverUser::setAmbientColor (CRGBA color)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setAmbientColor (color);
}


// ***************************************************************************
void			CDriverUser::setPolygonMode (TPolygonMode mode)
{
	NL3D_HAUTO_UI_DRIVER;

	IDriver::TPolygonMode	dmode=IDriver::Filled;
	switch(mode)
	{
		case Filled:	dmode= IDriver::Filled; break;
		case Line:		dmode= IDriver::Line; break;
		case Point:		dmode= IDriver::Point; break;
		default: nlstop;
	};
	_Driver->setPolygonMode (dmode);
}
U3dMouseListener*	CDriverUser::create3dMouseListener ()
{
	NL3D_HAUTO_UI_DRIVER;

	// Alloc the listener
	CEvent3dMouseListener *listener=new CEvent3dMouseListener();

	// register it
	listener->addToServer (EventServer);

	return listener;
}
void CDriverUser::delete3dMouseListener (U3dMouseListener *listener)
{
	NL3D_HAUTO_UI_DRIVER;

	// Unregister
	((CEvent3dMouseListener*)listener)->removeFromServer (EventServer);

	delete (CEvent3dMouseListener*)listener;
}
UDriver::TPolygonMode 	CDriverUser::getPolygonMode ()
{
	NL3D_HAUTO_UI_DRIVER;

	IDriver::TPolygonMode	dmode;
	UDriver::TPolygonMode	umode=UDriver::Filled;
	dmode= _Driver->getPolygonMode();
	switch(dmode)
	{
		case IDriver::Filled:	umode= UDriver::Filled; break;
		case IDriver::Line:		umode= UDriver::Line; break;
		case IDriver::Point:	umode= UDriver::Point; break;
		default: nlstop;
	};

	return umode;
}

void			CDriverUser::forceDXTCCompression(bool dxtcComp)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->forceDXTCCompression(dxtcComp);
}

void			CDriverUser::setAnisotropicFilter(sint filter)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setAnisotropicFilter(filter);
}

uint			CDriverUser::getAnisotropicFilter() const
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getAnisotropicFilter();
}

uint			CDriverUser::getAnisotropicFilterMaximum() const
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getAnisotropicFilterMaximum();
}

void			CDriverUser::forceTextureResize(uint divisor)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->forceTextureResize(divisor);
}
bool			CDriverUser::setMonitorColorProperties (const CMonitorColorProperties &properties)
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->setMonitorColorProperties (properties);
}



// ***************************************************************************
// ***************************************************************************
/// Driver information/Queries
// ***************************************************************************
// ***************************************************************************

uint32			CDriverUser::getImplementationVersion () const
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getImplementationVersion ();
}
const char*		CDriverUser::getDriverInformation ()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getDriverInformation();
}
const char*		CDriverUser::getVideocardInformation ()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getVideocardInformation ();
}
sint			CDriverUser::getTotalVideoMemory () const
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getTotalVideoMemory ();
}
uint			CDriverUser::getNbTextureStages()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getNbTextureStages();
}
void			CDriverUser::getWindowSize (uint32 &width, uint32 &height)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getWindowSize (width, height);
}
uint			CDriverUser::getWindowWidth ()
{
	NL3D_HAUTO_UI_DRIVER;

	uint32 width, height;
	_Driver->getWindowSize (width, height);
	return width;
}
uint			CDriverUser::getWindowHeight ()
{
	NL3D_HAUTO_UI_DRIVER;

	uint32 width, height;
	_Driver->getWindowSize (width, height);
	return height;
}
void			CDriverUser::getWindowPos (sint32 &x, sint32 &y)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getWindowPos (x, y);
}
uint32			CDriverUser::getAvailableVertexAGPMemory ()
{
	return _Driver->getAvailableVertexAGPMemory ();
}
uint32			CDriverUser::getAvailableVertexVRAMMemory ()
{
	return _Driver->getAvailableVertexVRAMMemory ();
}
void			CDriverUser::getBuffer (CBitmap &bitmap)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getBuffer (bitmap) ;
}
void			CDriverUser::getZBuffer (std::vector<float>  &zbuffer)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getZBuffer (zbuffer) ;
}
void			CDriverUser::getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getBufferPart (bitmap, rect) ;
}
void			CDriverUser::getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getZBufferPart (zbuffer, rect) ;
}
bool			CDriverUser::fillBuffer (CBitmap &bitmap)
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->fillBuffer(bitmap);
}


// ***************************************************************************
// ***************************************************************************
// Mouse / Keyboards / Game devices
// ***************************************************************************
// ***************************************************************************

void			CDriverUser::showCursor (bool b)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->showCursor(b);
}
void			CDriverUser::setMousePos (float x, float y)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setMousePos (x, y);
}
void			CDriverUser::setCapture (bool b)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setCapture (b);
}

bool			CDriverUser::isSystemCursorCaptured()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->isSystemCursorCaptured();
}

void			CDriverUser::addCursor(const std::string &name, const NLMISC::CBitmap &bitmap)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->addCursor(name, bitmap);
}

void			CDriverUser::setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setCursor(name, col, rot, hotSpotX, hotSpotY, forceRebuild);
}

void			CDriverUser::setCursorScale(float scale)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setCursorScale(scale);
}

// ***************************************************************************
// ***************************************************************************
// Async Texture loading mgt
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void				CDriverUser::setupAsyncTextureLod(uint baseLevel, uint maxLevel)
{
	NL3D_HAUTO_TEX_DRIVER;

	_AsyncTextureManager.setupLod(baseLevel, maxLevel);
}
// ***************************************************************************
void				CDriverUser::setupAsyncTextureMaxUploadPerFrame(uint maxup)
{
	NL3D_HAUTO_TEX_DRIVER;

	_AsyncTextureManager.setupMaxUploadPerFrame(maxup);
}
// ***************************************************************************
void				CDriverUser::updateAsyncTexture()
{
	NL3D_HAUTO_TEX_DRIVER;

	_AsyncTextureManager.update(getDriver());
}


// ***************************************************************************
void				CDriverUser::setupMaxTotalAsyncTextureSize(uint maxText)
{
	NL3D_HAUTO_TEX_DRIVER;

	_AsyncTextureManager.setupMaxTotalTextureSize(maxText);
}
// ***************************************************************************
uint				CDriverUser::getTotalAsyncTextureSizeAsked() const
{
	NL3D_HAUTO_TEX_DRIVER;

	return _AsyncTextureManager.getTotalTextureSizeAsked();
}
// ***************************************************************************
uint				CDriverUser::getLastAsyncTextureSizeGot() const
{
	NL3D_HAUTO_TEX_DRIVER;

	return _AsyncTextureManager.getLastTextureSizeGot();
}

// ***************************************************************************
void				CDriverUser::setupMaxHLSColoringPerFrame(uint maxCol)
{
	NL3D_HAUTO_TEX_DRIVER;

	_AsyncTextureManager.setupMaxHLSColoringPerFrame(maxCol);
}

// ***************************************************************************
void				CDriverUser::loadHLSBank(const std::string &fileName)
{
	NL3D_HAUTO_TEX_DRIVER;

	// load it.
	CHLSTextureBank			*hlsBank= new CHLSTextureBank;
	try
	{
		std::string	path= CPath::lookup(fileName);
		CIFile	fIn;
		if(!fIn.open(path))
			throw EPathNotFound(path);
		fIn.serial(*hlsBank);
	}
	catch(const Exception &)
	{
		delete hlsBank;
		throw;
	}

	// add it to the manager.
	_AsyncTextureManager.HLSManager.addBank(hlsBank);
}


// ***************************************************************************
void				CDriverUser::setSwapVBLInterval(uint interval)
{
	NL3D_HAUTO_UI_DRIVER

	_Driver->setSwapVBLInterval(interval);
}

// ***************************************************************************
uint				CDriverUser::getSwapVBLInterval()
{
	NL3D_HAUTO_UI_DRIVER

	return _Driver->getSwapVBLInterval();
}

// ***************************************************************************
bool CDriverUser::supportMADOperator() const
{
	NL3D_HAUTO_UI_DRIVER

	return _Driver->supportMADOperator();
}

// ***************************************************************************
bool CDriverUser::supportBloomEffect() const
{
	NL3D_HAUTO_UI_DRIVER

	return _Driver->supportBloomEffect();
}

// ***************************************************************************

void CDriverUser::startBench (bool wantStandardDeviation, bool quick, bool reset)
{
	_Driver->startBench (wantStandardDeviation, quick, reset);
}

// ***************************************************************************

void CDriverUser::endBench ()
{
	_Driver->endBench ();
}

// ***************************************************************************

void CDriverUser::displayBench (class NLMISC::CLog *log)
{
	_Driver->displayBench (log);
}

// ***************************************************************************
UWaterEnvMap *CDriverUser::createWaterEnvMap()
{
	CWaterEnvMapUser *wem = new CWaterEnvMapUser;
	wem->EnvMap.Driver = this;
	return wem;
}

// ***************************************************************************
void CDriverUser::deleteWaterEnvMap(UWaterEnvMap *map)
{
	delete (CWaterEnvMapUser *) map;
}

// ***************************************************************************
void CDriverUser::setCullMode(TCullMode cullMode)
{
	NL3D_HAUTO_UI_DRIVER
	_Driver->setCullMode((IDriver::TCullMode) cullMode);
}

// ***************************************************************************
UDriver::TCullMode CDriverUser::getCullMode() const
{
	NL3D_HAUTO_UI_DRIVER
	return (TCullMode) _Driver->getCullMode();
}

// ***************************************************************************
bool CDriverUser::isLost() const
{
	NL3D_HAUTO_UI_DRIVER
	return _Driver->isLost();
}

// ***************************************************************************
void CDriverUser::beginDialogMode()
{
	NL3D_HAUTO_UI_DRIVER
	_Driver->beginDialogMode();
}

// ***************************************************************************
void CDriverUser::endDialogMode()
{
	NL3D_HAUTO_UI_DRIVER
	_Driver->endDialogMode();
}

// ***************************************************************************
void CDriverUser::enableStencilTest(bool enable)
{
	NL3D_HAUTO_UI_DRIVER
	_Driver->enableStencilTest(enable);
}

// ***************************************************************************
bool CDriverUser::isStencilTestEnabled() const
{
	NL3D_HAUTO_UI_DRIVER
	return _Driver->isStencilTestEnabled();
}

// ***************************************************************************
void CDriverUser::stencilFunc(TStencilFunc stencilFunc, int ref, uint mask)
{
	NL3D_HAUTO_UI_DRIVER
	_Driver->stencilFunc((IDriver::TStencilFunc)stencilFunc, ref, mask);
}

// ***************************************************************************
void CDriverUser::stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass)
{
	NL3D_HAUTO_UI_DRIVER
	_Driver->stencilOp((IDriver::TStencilOp)fail, (IDriver::TStencilOp)zfail,
						(IDriver::TStencilOp)zpass);
}

// ***************************************************************************
void CDriverUser::stencilMask(uint mask)
{
	NL3D_HAUTO_UI_DRIVER
	_Driver->stencilMask(mask);
}

// ***************************************************************************
uint64 CDriverUser::getSwapBufferCounter()
{
	NL3D_HAUTO_UI_DRIVER
	return _Driver->getSwapBufferCounter();
}

// ***************************************************************************

bool CDriverUser::stretchRect(UScene * scene, class UTexture & srcUText, NLMISC::CRect &srcRect,
		class UTexture & destUText, NLMISC::CRect &destRect)
{
	NL3D_HAUTO_UI_DRIVER

	ITexture * srcText = (dynamic_cast<CTextureUser *>(&srcUText))->getITexture();
	ITexture * destText = (dynamic_cast<CTextureUser *>(&destUText))->getITexture();

	if(!_Driver->stretchRect(srcText, srcRect, destText, destRect))
	{
		setRenderTarget(destUText, 0, 0, destRect.Width, destRect.Height);

		// init quad
		NLMISC::CQuadUV		quad;
		quad.V0 = CVector(0.f, 0.f,	0.5f);
		quad.V1 = CVector(1.f,	0.f,	0.5f);
		quad.V2 = CVector(1.f,	1.f,	0.5f);
		quad.V3 = CVector(0.f,	1.f,	0.5f);
		float newU = _Driver->isTextureRectangle(srcText) ? (float)srcRect.Width : 1.f;
		float newV = _Driver->isTextureRectangle(srcText) ? (float)srcRect.Height : 1.f;
		quad.Uv0 = CUV(0.0f,	0.0f);
		quad.Uv1 = CUV(newU,	0.0f);
		quad.Uv2 = CUV(newU,	newV);
		quad.Uv3 = CUV(0.0f,	newV);

		// init material
		_MatTextStretchInternal.setTexture(0, srcText);

		// display
		UCamera	pCam = scene->getCam();
		setMatrixMode2D11();
		drawQuad(quad, _MatStretchText);
		setMatrixMode3D(pCam);

		_MatTextStretchInternal.setTexture(0, NULL);
		CTextureUser cu;
		setRenderTarget(cu, 0, 0, 0, 0);
	}

	return true;
}

// ***************************************************************************
bool CDriverUser::setRenderTarget(class UTexture & uTex, uint32 x, uint32 y, uint32 width, uint32 height, uint32 mipmapLevel, uint32 cubeFace)
{
	ITexture * tex = (dynamic_cast<CTextureUser *>(&uTex))->getITexture();

	if(tex!=NULL)
	{
		CTextureUser cu;
		setRenderTarget(cu);
	}

	bool result = _Driver->setRenderTarget(tex, x, y, width, height, mipmapLevel, cubeFace);

	setupMatrixContext();

	return result;
}
bool CDriverUser::copyTextToClipboard(const std::string &text)
{
	return _Driver->copyTextToClipboard(text);
}

bool CDriverUser::pasteTextFromClipboard(std::string &text)
{
	return _Driver->pasteTextFromClipboard(text);
}

} // NL3D
