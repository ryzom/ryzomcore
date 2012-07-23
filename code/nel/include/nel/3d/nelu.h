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

#ifndef NL_NELU_H
#define NL_NELU_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/scene.h"
#include "nel/3d/shape_bank.h"
#include "nel/3d/dru.h"
#include "nel/misc/event_server.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/rgba.h"


namespace NL3D
{


using NLMISC::CSmartPtr;


/**
 * 3d Engine Utilities. Simple Open / Close framework.
 * Designed to work only with a mono-threaded / mono-scene / single-windowed app.
 *
 * If your app want to register other Models with basics CScene traversals, it could use CNELU, and register his
 * models after, or even before CNELU::init3d().
 *
 * If your app want to add funky traversals, it MUST NOT use CNELU (see CScene for more information...).
 * NB: actually it may use yet initDriver() and initEventServer() but not initScene()...
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CNELU
{
public:
	// Default Perspective of camera.
	static const float		DefLx;		//=0.26f;
	static const float		DefLy;		//=0.2f;
	static const float		DefLzNear;	//=0.15f;
	static const float		DefLzFar;	//=1000.0f;


public:
	/** Init all that we need for a single GL window:
	 * - create / init / openWindow / activate a IDriver.
	 *
	 * You can access the driver with CNELU::Driver.
	 */
	static bool		initDriver(uint w, uint h, uint bpp=32, bool windowed=true, nlWindow systemWindow=EmptyWindow, bool offscreen=false, bool direct3d=false) throw(EDru);

	/** Init all that we need for a Scene.
	 * - register scene basics models,
	 * - init the scene, with basic Traversals,
	 * - create a default camera, linked to the scene, and with default frustum as specified above.
	 *
	 * After creation, use the CNELU::Camera to manipulates the camera of scene (but you may change all you want
	 * to this camera or create/use another camera if you want...)
	 * \param viewport the viewport, fullscreen by default.
	 */
	static void		initScene(CViewport viewport=CViewport());

	/** Init all that we need for a window message processing:
	 * - a server.
	 * - an asynclistener for get async key states.
	 */
	static void		initEventServer();


	/** Release / delete the driver. (close window etc...)
	 */
	static void		releaseDriver();

	/** Release the scene.
	 */
	static void		releaseScene();

	/** Release the event server and the asynclistener.
	 */
	static void		releaseEventServer();

	/** Check if you press F12 and if yes, take a screenshot
	 */
	static void		screenshot();

public:

	/** Init the registry, and init all NELU
	 * - NL3D::registerSerial3d().
	 * - initDriver();
	 * - initScene();
	 * - initEventServer();
	 */
	static bool		init(uint w, uint h, CViewport viewport=CViewport(), uint bpp=32, bool windowed=true, nlWindow systemWindow=EmptyWindow, bool offscreen = false, bool direct3d = false) throw(EDru);

	/** Delete all:
	 * - releaseEventServer();
	 * - releaseScene()
	 * - releaseDriver().
	 */
	static void		release();


	/// Shortcut to clear ZBuffer and color buffer of CNELU::Driver.
	static void		clearBuffers(NLMISC::CRGBA col= NLMISC::CRGBA(0,0,0,0));
	/// Shortcut to swapBuffers of CNELU::Driver.
	static void		swapBuffers();



public:
	static IDriver				*Driver;
	static CScene				*Scene;
	static CShapeBank			*ShapeBank;
	// There is one MeshSkin Vertex Stream per driver, and for all scenes.
	static CVertexStreamManager	*MeshSkinManager;

	static CRefPtr<CCamera>		Camera;
	static NLMISC::CEventServer			EventServer;
	static NLMISC::CEventListenerAsync	AsyncListener;

};


} // NL3D


#endif // NL_NELU_H

/* End of nelu.h */
