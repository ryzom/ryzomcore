/*
Object Viewer Qt Widget
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "object_viewer_widget.h"

// STL includes

// NeL includes
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/path.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_light.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_play_list.h>
#include <nel/3d/u_animation_set.h>
#include <nel/3d/u_animation.h>
#include <nel/3d/u_play_list_manager.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/bloom_effect.h>

// Qt includes
#include <QIcon>

// Project includes

Q_EXPORT_PLUGIN2(object_viewer_widget_qt, NLQT::CObjectViewerWidget)

using namespace NLMISC;
using namespace NL3D;
using namespace std;

namespace NLQT
{
	CObjectViewerWidget *CObjectViewerWidget::_objectViewerWidget = NULL;

	CObjectViewerWidget::CObjectViewerWidget(QWidget *parent)
		: _isGraphicsInitialized(false), _isGraphicsEnabled(false),
		_Driver(NULL), _Light(0), _phi(0), _psi(0),_dist(2),
		_CameraFocal(75), _CurrentInstance(""), _BloomEffect(false),
		_Scene(0), QWidget(parent)
	{
		setMouseTracking(true);
		setFocusPolicy(Qt::StrongFocus);
		_objectViewerWidget = this;

		_isGraphicsEnabled = true;

		// As a special case, a QTimer with a timeout of 0 will time out as soon as all the events in the window system's event queue have been processed.
		// This can be used to do heavy work while providing a snappy user interface.
		_mainTimer = new QTimer(this);
		connect(_mainTimer, SIGNAL(timeout()), this, SLOT(updateRender()));
		// timer->start(); // <- timeout 0
		// it's heavy on cpu, though, when no 3d driver initialized :)
		_mainTimer->start(25); // 25fps
	}

	CObjectViewerWidget::~CObjectViewerWidget()
	{
		release();
	}

	void CObjectViewerWidget::showEvent ( QShowEvent * event )
	{
		if (!_mainTimer->isActive()) 
		{
			_mainTimer->start(25);
		}
	}

	void CObjectViewerWidget::setNelContext(NLMISC::INelContext &nelContext) 
	{
		_LibContext = new CLibraryContext(nelContext);
	}

	void CObjectViewerWidget::init()
	{

		connect(this, SIGNAL(topLevelChanged(bool)),
			this, SLOT(topLevelChanged(bool)));
		//H_AUTO2
		//nldebug("%d %d %d",_nlw->winId(), width(), height());


#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
		//dynamic_cast<QNLWidget*>(widget())->makeCurrent();
#endif // defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

		nlWindow wnd = (nlWindow)winId();
		uint16 w = width();
		uint16 h = height();

		setMouseTracking(true);

		// set background color from config
		//NLMISC::CConfigFile::CVar v = Modules::config().getConfigFile().getVar("BackgroundColor");
		//_BackgroundColor = CRGBA(v.asInt(0), v.asInt(1), v.asInt(2));
		_BackgroundColor = CRGBA(255, 255, 255);

		// set graphics driver from config
		//NLMISC::CConfigFile::CVar v2 = Modules::config().getConfigFile().getVar("GraphicsDriver");
		// Choose driver opengl to work correctly under Linux example
		_Direct3D = false; //_Driver = OpenGL;

#ifdef NL_OS_WINDOWS
		//std::string driver = v2.asString();
		//if (driver == "Direct3D") _Direct3D = true; //m_Driver = Direct3D;
		//else if (driver == "OpenGL") _Direct3D = false; //m_Driver = OpenGL;
		//else nlwarning("Invalid driver specified, defaulting to OpenGL");
#endif

		//Modules::config().setAndCallback("CameraFocal",CConfigCallback(this,&CObjectViewer::cfcbCameraFocal));
		//Modules::config().setAndCallback("BloomEffect",CConfigCallback(this,&CObjectViewer::cfcbBloomEffect));

		// create the driver
		nlassert(!_Driver);

		_Driver = UDriver::createDriver(0, _Direct3D, 0);
		nlassert(_Driver);

		// initialize the window with config file values
		_Driver->setDisplay(wnd, NL3D::UDriver::CMode(w, h, 32));

		//_Light = ULight::createLight();

		//// set mode of the light
		//_Light->setMode(ULight::DirectionalLight);

		//// set position of the light
		//_Light->setPosition(CVector(-20.f, 30.f, 10.f));

		//// white light
		//_Light->setAmbiant(CRGBA(255, 255, 255));

		//// set and enable the light
		//_Driver->setLight(0, *_Light);
		//_Driver->enableLight(0);

		// Create a scene
		_Scene = _Driver->createScene(true);

		_PlayListManager = _Scene->createPlayListManager();

		//_Scene->enableLightingSystem(true);

		// create the camera
		UCamera camera = _Scene->getCam();

		camera.setTransformMode (UTransformable::DirectMatrix);

		setSizeViewport(w, h);

		// camera will look at entities
		camera.lookAt(NLMISC::CVector(_dist,0,1), NLMISC::CVector(0,0,0.5));

		NLMISC::CVector hotSpot=NLMISC::CVector(0,0,0);

		_MouseListener = _Driver->create3dMouseListener();
		_MouseListener->setMatrix(_Scene->getCam().getMatrix());
		_MouseListener->setFrustrum(_Scene->getCam().getFrustum());
		_MouseListener->setHotSpot(hotSpot);
		_MouseListener->setMouseMode(U3dMouseListener::edit3d);

		NL3D::CBloomEffect::instance().setDriver(_Driver);
		NL3D::CBloomEffect::instance().setScene(_Scene);
		NL3D::CBloomEffect::instance().init(!_Direct3D);
		//NL3D::CBloomEffect::instance().setDensityBloom(Modules::config().getConfigFile().getVar("BloomDensity").asInt());
		//NL3D::CBloomEffect::instance().setSquareBloom(Modules::config().getConfigFile().getVar("BloomSquare").asBool());
	}

	void CObjectViewerWidget::release()
	{
		//H_AUTO2
		nldebug("");

		_Driver->delete3dMouseListener(_MouseListener);

		// delete all entities
		deleteEntities();

		_Scene->deletePlayListManager(_PlayListManager);

		// delete the scene
		_Driver->deleteScene(_Scene);

		// delete the light
		delete _Light;

		// release driver
		nlassert(_Driver);
		_Driver->release();
		delete _Driver;
		_Driver = NULL;
	}

	void CObjectViewerWidget::updateRender()
	{
		//nldebug("CMainWindow::updateRender");
		updateInitialization(isVisible());

		//QModelIndex index = _dirModel->setRootPath("D:/Dev/Ryzom/code/ryzom/common/data_leveldesign/leveldesign");
		//_dirTree->setRootIndex(index);

		if (isVisible())
		{
			// call all update functions
			// 01. Update Utilities (configuration etc)

			// 02. Update Time (deltas)
			// ...

			// 03. Update Receive (network, servertime, receive messages)
			// ...

			// 04. Update Input (keyboard controls, etc)
			if (_isGraphicsInitialized)
				updateInput();

			// 05. Update Weather (sky, snow, wind, fog, sun)
			// ...

			// 06. Update Entities (movement, do after possible tp from incoming messages etc)
			//      - Move other entities
			//      - Update self entity
			//      - Move bullets
			// ...

			// 07. Update Landscape (async zone loading near entity)
			// ...

			// 08. Update Collisions (entities)
			//      - Update entities
			//      - Update move container (swap with Update entities? todo: check code!)
			//      - Update bullets
			// ...

			// 09. Update Animations (playlists)
			//      - Needs to be either before or after entities, not sure, 
			//        there was a problem with wrong order a while ago!!!


			//updateAnimation(_AnimationDialog->getTime());
			updateAnimatePS();
			// 10. Update Camera (depends on entities)
			// ...

			// 11. Update Interface (login, ui, etc)
			// ...

			// 12. Update Sound (sound driver)
			// ...

			// 13. Update Send (network, send new position etc)
			// ...

			// 14. Update Debug (stuff for dev)
			// ...

			if (_isGraphicsInitialized && !getDriver()->isLost())
			{
				// 01. Render Driver (background color)			
				renderDriver(); // clear all buffers

				// 02. Render Sky (sky scene)
				// ...

				// 04. Render Scene (entity scene)
				renderScene();

				// 05. Render Effects (flare)
				// ...

				// 06. Render Interface 3D (player names)
				// ...

				// 07. Render Debug 3D
				// ...

				// 08. Render Interface 2D (chatboxes etc, optionally does have 3d)
				// ...

				// 09. Render Debug 2D (stuff for dev)
				renderDebug2D();

				// swap 3d buffers
				getDriver()->swapBuffers();
			}
		}
	}

	void CObjectViewerWidget::updateInitialization(bool visible)
	{
		//nldebug("CMainWindow::updateInitialization");
		bool done;
		do
		{
			done = true; // set false whenever change
			bool wantGraphics = _isGraphicsEnabled && visible;
			// bool wantLandscape = wantGraphics && m_IsGraphicsInitialized && isLandscapeEnabled;

			// .. stuff that depends on other stuff goes on top to prioritize deinitialization

			// Landscape
			// ...

			// Graphics (Driver)
			if (_isGraphicsInitialized)
			{
				if (!wantGraphics)
				{
					//_isGraphicsInitialized = false;
					//release();
					_mainTimer->stop();
					//done = false;
				}
			}
			else
			{
				if (wantGraphics)
				{
					init();
					_isGraphicsInitialized = true;
					_mainTimer->start(25);
					//done = false;
				}
			}
		}
		while (!done);
	}

	void CObjectViewerWidget::updateInput()
	{
		_Driver->EventServer.pump();

		// New matrix from camera
		_Scene->getCam().setTransformMode(NL3D::UTransformable::DirectMatrix);
		_Scene->getCam().setMatrix (_MouseListener->getViewMatrix());

		//nldebug("%s",_Scene->getCam().getMatrix().getPos().asString().c_str());
	}

	void CObjectViewerWidget::renderDriver()
	{
		// Render the scene.
		if((NL3D::CBloomEffect::instance().getDriver() != NULL) && (_BloomEffect))
		{
			NL3D::CBloomEffect::instance().initBloom();
		}
		_Driver->clearBuffers(_BackgroundColor);
	}

	void CObjectViewerWidget::renderScene()
	{
		// render the scene
		_Scene->render();

		if((NL3D::CBloomEffect::instance().getDriver() != NULL) && (_BloomEffect))
		{
			NL3D::CBloomEffect::instance().endBloom();
			NL3D::CBloomEffect::instance().endInterfacesDisplayBloom();
		}
	}

	void CObjectViewerWidget::renderDebug2D()
	{
	}

	void CObjectViewerWidget::saveScreenshot(const std::string &nameFile, bool jpg, bool png, bool tga)
	{
		//H_AUTO2

		// FIXME: create screenshot path if it doesn't exist!

		// empty bitmap
		CBitmap bitmap;
		// copy the driver buffer to the bitmap
		_Driver->getBuffer(bitmap);
		// create the file name
		string filename = std::string("./") + nameFile;
		// write the bitmap as a jpg, png or tga to the file
		if (jpg)
		{
			string newfilename = CFile::findNewFile(filename + ".jpg");
			COFile outputFile(newfilename);
			bitmap.writeJPG(outputFile, 100);
			nlinfo("Screenshot '%s' saved", newfilename.c_str());
		}
		if (png)
		{
			string newfilename = CFile::findNewFile(filename + ".png");
			COFile outputFile(newfilename);
			bitmap.writePNG(outputFile, 24);
			nlinfo("Screenshot '%s' saved", newfilename.c_str());
		}
		if (tga)
		{
			string newfilename = CFile::findNewFile(filename + ".tga");
			COFile outputFile(newfilename);
			bitmap.writeTGA(outputFile, 24, false);
			nlinfo("Screenshot '%s' saved", newfilename.c_str());
		}
	}

	bool CObjectViewerWidget::loadMesh(const std::string &meshFileName, const std::string &skelFileName)
	{
		std::string fileName = CFile::getFilenameWithoutExtension(meshFileName);
		if ( _Entities.count(fileName) != 0) 
			return false;

		CPath::addSearchPath(CFile::getPath(meshFileName), false, false);

		// create instance of the mesh character
		UInstance Entity = _Scene->createInstance(meshFileName);
		
		CAABBox bbox;
		Entity.getShapeAABBox(bbox);
		setCamera(_Scene, bbox , Entity, true);

		_MouseListener->setMatrix(_Scene->getCam().getMatrix());

		USkeleton Skeleton = _Scene->createSkeleton(skelFileName);

		// if we can't create entity, skip it
		if (Entity.empty()) return false;

		// create a new entity
		EIT eit = (_Entities.insert (make_pair (fileName, CEntity()))).first;
		CEntity	&entity = (*eit).second;

		// set the entity up
		entity._Name = fileName;
		entity._FileNameShape = meshFileName;
		entity._FileNameSkeleton = skelFileName;
		entity._Instance = Entity;
		if (!Skeleton.empty()) 
		{
			entity._Skeleton = Skeleton;
			entity._Skeleton.bindSkin (entity._Instance);
		}
		entity._AnimationSet = _Driver->createAnimationSet(false);
		entity._PlayList = _PlayListManager->createPlayList(entity._AnimationSet);
		return true;
	}

	void CObjectViewerWidget::setVisible(bool visible)
	{
		// called by show()
		// code assuming visible window needed to init the 3d driver
		nldebug("%d", visible);
		if (visible)
		{
			QWidget::setVisible(true);
		}
		else
		{
			QWidget::setVisible(false);
		}
	}

	void CObjectViewerWidget::resetScene()
	{
		deleteEntities();

		// Reset camera.
		//..

		// to load files with the same name but located in different directories
		//CPath::clearMap();

		// load and set search paths from config
		//Modules::config().configSearchPaths();

		_CurrentInstance = "";

		nlinfo("Scene cleared");
	}

	void CObjectViewerWidget::setBackgroundColor(NLMISC::CRGBA backgroundColor)
	{
		_BackgroundColor = backgroundColor;

		// config file variable changes
		//Modules::config().getConfigFile().getVar("BackgroundColor").setAsInt(_BackgroundColor.R, 0);
		//Modules::config().getConfigFile().getVar("BackgroundColor").setAsInt(_BackgroundColor.G, 1);
		//Modules::config().getConfigFile().getVar("BackgroundColor").setAsInt(_BackgroundColor.B, 2);
	}

	void CObjectViewerWidget::setGraphicsDriver(bool Direct3D)
	{
		//_Direct3D = Direct3D;

		//if (_Direct3D) Modules::config().getConfigFile().getVar("GraphicsDriver").setAsString("Direct3D");
		//else Modules::config().getConfigFile().getVar("GraphicsDriver").setAsString("OpenGL");
	}

	void CObjectViewerWidget::setSizeViewport(uint16 w, uint16 h)
	{
		_Scene->getCam().setPerspective(_CameraFocal * (float)Pi/180.f, (float)w/h, 0.1f, 1000);
	}

	void CObjectViewerWidget::setCurrentObject(const std::string &name)
	{
		if ((_Entities.count(name) != 0) || ( name.empty() )) _CurrentInstance = name;
		else nlerror ("Entity %s not found", name.c_str());
		nlinfo("set current entity %s", _CurrentInstance.c_str());
	}

	CEntity& CObjectViewerWidget::getEntity(const std::string &name)
	{
		if ( _Entities.count(name) == 0) nlerror("Entity %s not found", name.c_str());
		EIT eit = _Entities.find (name);
		return (*eit).second;
	}

	void CObjectViewerWidget::getListObjects(std::vector<std::string> &listObj)
	{
		listObj.clear();
		for (EIT eit = _Entities.begin(); eit != _Entities.end(); ++eit)
			listObj.push_back((*eit).second._Name);
	}

	void CObjectViewerWidget::deleteEntity(CEntity &entity)
	{
		if (entity._PlayList != NULL)
		{
			_PlayListManager->deletePlayList (entity._PlayList);
			entity._PlayList = NULL;
		}

		if (entity._AnimationSet != NULL)
		{
			_Driver->deleteAnimationSet(entity._AnimationSet);
			entity._AnimationSet = NULL;
		}

		if (!entity._Skeleton.empty())
		{
			entity._Skeleton.detachSkeletonSon(entity._Instance);

			_Scene->deleteSkeleton(entity._Skeleton);
			entity._Skeleton = NULL;
		}

		if (!entity._Instance.empty())
		{
			_Scene->deleteInstance(entity._Instance);
			entity._Instance = NULL;
		}
	}

	void CObjectViewerWidget::deleteEntities()
	{
		for (EIT eit = _Entities.begin(); eit != _Entities.end(); ++eit)
		{
			CEntity	&entity = (*eit).second;
			deleteEntity(entity);
		}
		_Entities.clear();
	}

	void CObjectViewerWidget::setCamera(NL3D::UScene *scene, CAABBox &bbox, UTransform &entity, bool high_z)
	{
		CVector pos(0.f, 0.f, 0.f);
		CQuat quat(0.f, 0.f, 0.f, 0.f);
		NL3D::UInstance inst;
		inst.cast(entity);
		if (!inst.empty())
		{
			inst.getDefaultPos(pos);
			inst.getDefaultRotQuat(quat);
		}

		// fix scale (some shapes have a different value)
		entity.setScale(1.f, 1.f, 1.f);

		UCamera Camera = scene->getCam();
		CVector max_radius = bbox.getHalfSize();

		CVector center = bbox.getCenter();
		entity.setPivot(center);
		center += pos;

		//scene->getCam().setPerspective(_CameraFocal * (float)Pi/180.f, (float)w/h, 0.1f, 1000);
		float fov = float(_CameraFocal * (float)Pi/180.0);
		//Camera.setPerspective (fov, 1.0f, 0.1f, 1000.0f);
		float radius = max(max(max_radius.x, max_radius.y), max_radius.z);
		if (radius == 0.f) radius = 1.f;
		float left, right, bottom, top, znear, zfar;
		Camera.getFrustum(left, right, bottom, top, znear, zfar);
		float dist = (radius / (tan(fov/2))) * 0.2;
		CVector eye(center);
		CVector ax(quat.getAxis());

		if (ax.isNull() || ax == CVector::I)
		{
			ax = CVector::J;
		}
		else if (ax == -CVector::K)
		{
			ax = -CVector::J;
		}

		eye -= ax * (dist+radius);
		if (high_z)
			eye.z += max_radius.z/1.0f;
		Camera.lookAt(eye, center);
		setupLight(eye, center - eye);
	}

	bool CObjectViewerWidget::setupLight(const CVector &position, const CVector &direction)
	{
		if (!_Light)
		_Light = ULight::createLight();
		if (!_Light) return false;

		// set mode of the light
		_Light->setMode(ULight::DirectionalLight);

		// set position of the light
		//	Light->setupDirectional(settings.light_ambiant, settings.light_diffuse, settings.light_specular, settings.light_direction);
		NLMISC::CRGBA light_ambiant = CRGBA(0,0,0);
		NLMISC::CRGBA light_diffuse = CRGBA(255,255,255);
		NLMISC::CRGBA light_specular = CRGBA(255,255,255);
		NLMISC::CVector light_direction = CVector(0.25, 0.25, 0.25);
		_Light->setupPointLight(light_ambiant, light_diffuse, light_specular, position, direction + light_direction);

		// set and enable the light
		_Driver->setLight(0, *_Light);
		_Driver->enableLight(0);

		return true;
	}

	QIcon* CObjectViewerWidget::saveOneImage(string shapename)
	{
		int output_width = 128;
		int output_height = 128;

		// Create a scene
		NL3D::UScene* Scene = _Driver->createScene(true);
		if (!Scene) return 0;

		// get scene camera
		if (Scene->getCam().empty())
		{
			nlwarning("can't get camera from scene");
			return 0;
		}

		// add an entity to the scene
		UInstance Entity = Scene->createInstance(shapename.c_str());

		// if we can't create entity, skip it
		if (Entity.empty())
		{
			nlwarning("can't create instance from %s", shapename.c_str());
			return 0;
		}

		// get AABox of Entity
		CAABBox bbox;
		Entity.getShapeAABBox(bbox);
		setCamera(Scene, bbox , Entity, true);
		Scene->getCam().setPerspective(_CameraFocal * (float)Pi/180.f, (float)output_width/output_height, 0.1f, 1000);

		string filename = CPath::standardizePath("") + toString("%s.%s", shapename.c_str(), "png");

		// the background is white
		_Driver->clearBuffers();

		// render the scene
		Scene->render();

		CBitmap btm;
		_Driver->getBuffer(btm);

		btm.resample(output_width, output_height);

		COFile fs;

		if (fs.open(filename))
		{
			if (!btm.writePNG(fs, 24))
			{
				nlwarning("can't save image to PNG");
				return 0;
			}
		}
		else
		{
			nlwarning("can't create %s", "test.png");
			return 0;
		}
		fs.close();

		QIcon *icon	= new QIcon(QString(filename.c_str()));
		//CFile::deleteFile(filename);
		return icon;
	}

	void CObjectViewerWidget::updateAnimatePS(uint64 deltaTime)
	{
		static sint64 firstTime = NLMISC::CTime::getLocalTime();
		static sint64 lastTime = NLMISC::CTime::getLocalTime();
		if (deltaTime == 0)
		{
			deltaTime = NLMISC::CTime::getLocalTime() - lastTime;
		}
		lastTime += deltaTime;
		float fdelta = 0.001f * (float) (lastTime - firstTime);
		_Scene->animate ( fdelta);
	}

#if defined(NL_OS_WINDOWS)

	typedef bool (*winProc)(NL3D::IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool CObjectViewerWidget::winEvent(MSG * message, long * result)
	{
		if (getDriver() && getDriver()->isActive())
		{
			NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser*>(getDriver())->getDriver();
			if (driver)
			{
				winProc proc = (winProc)driver->getWindowProc();
				return proc(driver, message->hwnd, message->message, message->wParam, message->lParam);
			}
		}

		return false;
	}

#elif defined(NL_OS_MAC)

	typedef bool (*cocoaProc)(NL3D::IDriver*, const void* e);

	bool CObjectViewerWidget::macEvent(EventHandlerCallRef caller, EventRef event)
	{
		if(caller)
			nlerror("You are using QtCarbon! Only QtCocoa supported, please upgrade Qt");

		if (getDriver() && getDriver()->isActive())
		{
			NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser*>(getDriver())->getDriver();
			if (driver)
			{
				cocoaProc proc = (cocoaProc)driver->getWindowProc();
				return proc(driver, event);
			}
		}

		return false;
	}

#elif defined(NL_OS_UNIX)

	typedef bool (*x11Proc)(NL3D::IDriver *drv, XEvent *e);

	bool CObjectViewerWidget::x11Event(XEvent *event)
	{
		if (getDriver() && getDriver()->isActive())
		{
			NL3D::IDriver *driver = dynamic_cast<NL3D::CDriverUser*>(getDriver())->getDriver();
			if (driver)
			{
				x11Proc proc = (x11Proc)driver->getWindowProc();
				return proc(driver, event);
			}
		}

		return false;
	}
#endif

} /* namespace NLQT */
