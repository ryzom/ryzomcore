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

#include "nel/misc/path.h"
#include "nel/misc/file.h"

#include "nel/3d/nelu.h"
#include "nel/3d/dru.h"
#include "nel/3d/camera.h"
#include "nel/3d/register_3d.h"
#include "nel/3d/init_3d.h"
#include "nel/3d/vertex_stream_manager.h"
#include "nel/misc/debug.h"

using namespace std;
using namespace NLMISC;


namespace NL3D
{

const float		CNELU::DefLx=0.26f;
const float		CNELU::DefLy=0.2f;
const float		CNELU::DefLzNear=0.15f;
const float		CNELU::DefLzFar=1000.0f;

IDriver				*CNELU::Driver=NULL;
CScene				*CNELU::Scene=NULL;
CShapeBank			*CNELU::ShapeBank=NULL;
CVertexStreamManager*CNELU::MeshSkinManager=NULL;
CRefPtr<CCamera>	CNELU::Camera;
CEventServer		CNELU::EventServer;
CEventListenerAsync	CNELU::AsyncListener;


bool			CNELU::initDriver (uint w, uint h, uint bpp, bool windowed, nlWindow systemWindow, bool offscreen, bool direct3d) throw(EDru)
{
	// Init debug system
//	NLMISC::InitDebug();

	ShapeBank = new CShapeBank;

	CNELU::Driver = NULL;

	// Init driver.
#ifdef NL_OS_WINDOWS
	if (direct3d)
	{
		CNELU::Driver= CDRU::createD3DDriver();
	}
#endif

	if (!CNELU::Driver)
	{
		CNELU::Driver= CDRU::createGlDriver();
	}

	if (!CNELU::Driver)
	{
		nlwarning ("CNELU::initDriver: no driver found");
		return false;
	}

	if (!CNELU::Driver->init())
	{
		nlwarning ("CNELU::initDriver: init() failed");
		return false;
	}
	if (!CNELU::Driver->setDisplay(systemWindow, GfxMode(uint16(w), uint16(h), uint8(bpp), windowed, offscreen)))
	{
		nlwarning ("CNELU::initDriver: setDisplay() failed");
		return false;
	}
	if (!CNELU::Driver->activate())
	{
		nlwarning ("CNELU::initDriver: activate() failed");
		return false;
	}

	// Create a skin manager
	MeshSkinManager = new CVertexStreamManager;

	return true;
}


void			CNELU::initScene(CViewport viewport)
{
	// Register basic csene.
	CScene::registerBasics();

	if (CNELU::Scene == NULL)
		CNELU::Scene = new CScene(false);

	// init default Roots.
	CNELU::Scene->initDefaultRoots();

	// Set driver.
	CNELU::Scene->setDriver(CNELU::Driver);

	// Set viewport
	CNELU::Scene->setViewport (viewport);

	// init QuadGridClipManager
	CNELU::Scene->initQuadGridClipManager ();

	// Create/link a camera.
	CNELU::Camera= (CCamera*)CNELU::Scene->createModel(NL3D::CameraId);
	CNELU::Scene->setCam(CNELU::Camera);
	CNELU::Camera->setFrustum(DefLx, DefLy, DefLzNear, DefLzFar);

	// Link to the shape bank
	CNELU::Scene->setShapeBank(CNELU::ShapeBank);

	// set the MeshSkin Vertex Streams
	CNELU::Scene->getRenderTrav().setMeshSkinManager(MeshSkinManager);
}


void			CNELU::initEventServer()
{
	CNELU::AsyncListener.reset ();
	CNELU::EventServer.addEmitter(CNELU::Driver->getEventEmitter());
	CNELU::AsyncListener.addToServer(CNELU::EventServer);
}


void			CNELU::releaseEventServer()
{
	CNELU::AsyncListener.removeFromServer(CNELU::EventServer);
	if (CNELU::Driver != NULL)
	{
		CNELU::EventServer.removeEmitter(CNELU::Driver->getEventEmitter());
	}
}


void			CNELU::releaseScene()
{
	// Release the camera.
	CNELU::Camera= NULL;

	// "Release" the Scene.
	CNELU::Scene->setDriver(NULL);
	CNELU::Scene->release();
}


void			CNELU::releaseDriver()
{
	if (MeshSkinManager)
		delete MeshSkinManager;

	// "Release" the driver.
	if (CNELU::Driver != NULL)
	{
		CNELU::Driver->release();
		delete CNELU::Driver;
		CNELU::Driver = NULL;
	}
	if( CNELU::ShapeBank != NULL )
	{
		delete CNELU::ShapeBank;
		CNELU::ShapeBank = NULL;
	}
}

bool			CNELU::init (uint w, uint h, CViewport viewport, uint bpp, bool windowed, nlWindow systemWindow, bool offscreen, bool direct3d) throw(EDru)
{
	NL3D::registerSerial3d();
	if (initDriver(w,h,bpp,windowed,systemWindow,offscreen,direct3d))
	{
		initScene(viewport);
		initEventServer();
		return true;
	}
	else
		return false;
}

void			CNELU::release()
{
	releaseEventServer();
	releaseScene();
	releaseDriver();
}

void			CNELU::screenshot()
{
	if (AsyncListener.isKeyPushed(KeyF12))
	{
		CBitmap btm;
		CNELU::Driver->getBuffer(btm);
		string filename = CFile::findNewFile ("screenshot.tga");
		COFile fs(filename);
		btm.writeTGA (fs,24);
		nlinfo("Screenshot '%s' saved", filename.c_str());
	}
}


void			CNELU::clearBuffers(CRGBA col)
{
	CNELU::Driver->clear2D(col);
	CNELU::Driver->clearZBuffer();
}

void			CNELU::swapBuffers()
{
	CNELU::Driver->swapBuffers();
}



} // NL3D
