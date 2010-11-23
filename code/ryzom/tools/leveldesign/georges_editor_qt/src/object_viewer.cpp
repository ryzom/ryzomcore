/*
Georges Editor Qt
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

#include "object_viewer.h"

// STL includes

// NeL includes
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/path.h>
#include <nel/3d/u_driver.h>
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

//#include <nel/3d/event_mouse_listener.h>

// Project includes
#include "modules.h"
#include "configuration.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace NLQT 
{

	CObjectViewer::CObjectViewer() 
	: _Driver(NULL), _Light(0),
	_phi(0), _psi(0),_dist(2),
	_CameraFocal(75),
	_CurrentInstance(""),
	_BloomEffect(false), _Scene(0)
	{

	}

	CObjectViewer::~CObjectViewer()
	{

	}

	void CObjectViewer::reinit(nlWindow wnd, uint16 w, uint16 h)
	{
		nldebug("CObjectViewert::reinit");

		//release();
		//init(wnd, w, h);
		//_Driver->setDisplay(wnd, NL3D::UDriver::CMode(w, h, 32));
	}

	void CObjectViewer::init(nlWindow wnd, uint16 w, uint16 h)
	{
		nldebug("CObjectViewer::init");

		// set background color from config
		NLMISC::CConfigFile::CVar v = Modules::config().getConfigFile().getVar("BackgroundColor");
		_BackgroundColor = CRGBA(v.asInt(0), v.asInt(1), v.asInt(2));

		// set graphics driver from config
		NLMISC::CConfigFile::CVar v2 = Modules::config().getConfigFile().getVar("GraphicsDriver");
		// Choose driver opengl to work correctly under Linux example
		_Direct3D = false; //_Driver = OpenGL;

#ifdef NL_OS_WINDOWS
		std::string driver = v2.asString();
		if (driver == "Direct3D") _Direct3D = true; //m_Driver = Direct3D;
		else if (driver == "OpenGL") _Direct3D = false; //m_Driver = OpenGL;
		else nlwarning("Invalid driver specified, defaulting to OpenGL");
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

	void CObjectViewer::release()
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

	void CObjectViewer::updateInput()
	{
		_Driver->EventServer.pump();

		// New matrix from camera
		_Scene->getCam().setTransformMode(NL3D::UTransformable::DirectMatrix);
		_Scene->getCam().setMatrix (_MouseListener->getViewMatrix());

		//nldebug("%s",_Scene->getCam().getMatrix().getPos().asString().c_str());
	}

	void CObjectViewer::renderDriver()
	{
		// Render the scene.
		if((NL3D::CBloomEffect::instance().getDriver() != NULL) && (_BloomEffect))
		{
			NL3D::CBloomEffect::instance().initBloom();
		}
		_Driver->clearBuffers(_BackgroundColor);
	}

	void CObjectViewer::renderScene()
	{
		// render the scene
		_Scene->render();

		if((NL3D::CBloomEffect::instance().getDriver() != NULL) && (_BloomEffect))
		{
			NL3D::CBloomEffect::instance().endBloom();
			NL3D::CBloomEffect::instance().endInterfacesDisplayBloom();
		}
	}

	void CObjectViewer::renderDebug2D()
	{
	}

	void CObjectViewer::saveScreenshot(const std::string &nameFile, bool jpg, bool png, bool tga)
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

	bool CObjectViewer::loadMesh(const std::string &meshFileName, const std::string &skelFileName)
	{
		std::string fileName = CFile::getFilenameWithoutExtension(meshFileName);
		if ( _Entities.count(fileName) != 0) 
			return false;

		CPath::addSearchPath(CFile::getPath(meshFileName), false, false);

		// create instance of the mesh character
		UInstance Entity = _Scene->createInstance(meshFileName);
		
		CAABBox bbox;
		Entity.getShapeAABBox(bbox);
		setCamera(bbox , Entity, true);

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

	void CObjectViewer::resetScene()
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

	void CObjectViewer::updateCamera(float deltaPsi, float deltaPhi, float deltaDist)
	{
		
	}

	void CObjectViewer::setBackgroundColor(NLMISC::CRGBA backgroundColor)
	{
		_BackgroundColor = backgroundColor;

		// config file variable changes
		Modules::config().getConfigFile().getVar("BackgroundColor").setAsInt(_BackgroundColor.R, 0);
		Modules::config().getConfigFile().getVar("BackgroundColor").setAsInt(_BackgroundColor.G, 1);
		Modules::config().getConfigFile().getVar("BackgroundColor").setAsInt(_BackgroundColor.B, 2);
	}

	void CObjectViewer::setGraphicsDriver(bool Direct3D)
	{
		_Direct3D = Direct3D;

		if (_Direct3D) Modules::config().getConfigFile().getVar("GraphicsDriver").setAsString("Direct3D");
		else Modules::config().getConfigFile().getVar("GraphicsDriver").setAsString("OpenGL");
	}

	void CObjectViewer::setSizeViewport(uint16 w, uint16 h)
	{
		_Scene->getCam().setPerspective(_CameraFocal * (float)Pi/180.f, (float)w/h, 0.1f, 1000);
	}

	void CObjectViewer::setCurrentObject(const std::string &name)
	{
		if ((_Entities.count(name) != 0) || ( name.empty() )) _CurrentInstance = name;
		else nlerror ("Entity %s not found", name.c_str());
		nlinfo("set current entity %s", _CurrentInstance.c_str());
	}

	CEntity& CObjectViewer::getEntity(const std::string &name)
	{
		if ( _Entities.count(name) == 0) nlerror("Entity %s not found", name.c_str());
		EIT eit = _Entities.find (name);
		return (*eit).second;
	}

	void CObjectViewer::getListObjects(std::vector<std::string> &listObj)
	{
		listObj.clear();
		for (EIT eit = _Entities.begin(); eit != _Entities.end(); ++eit)
			listObj.push_back((*eit).second._Name);
	}

	void CObjectViewer::deleteEntity(CEntity &entity)
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

	void CObjectViewer::deleteEntities()
	{
		for (EIT eit = _Entities.begin(); eit != _Entities.end(); ++eit)
		{
			CEntity	&entity = (*eit).second;
			deleteEntity(entity);
		}
		_Entities.clear();
	}

	void CObjectViewer::setCamera(CAABBox &bbox, UTransform &entity, bool high_z)
	{
		CVector pos(0.f, 0.f, 0.f);
		CQuat quat(0.f, 0.f, 0.f, 0.f);
		NL3D::UInstance inst;
		inst.cast(entity);
		if (!inst.empty())
		{
			inst.getDefaultPos(pos);
			inst.getDefaultRotQuat(quat);
			/*
			if (quat.getAxis().isNull())
			{
			quat.set(0, 0, 0, 0);
			inst.setRotQuat(quat);
			}
			*/
			//		quat.set(1.f, 1.f, 0.f, 0.f);

			//		inst.setRotQuat(quat);
			//		inst.getRotQuat(quat);

			// check for presence of all textures from each sets
			//bool allGood = true;

			//for(uint s = 0; s < 5; ++s)
			//{
			//	inst.selectTextureSet(s);

			//	uint numMat = inst.getNumMaterials();

			//	// by default, all textures are present
			//	allGood = true;

			//	for(uint i = 0; i < numMat; ++i)
			//	{
			//		UInstanceMaterial mat = inst.getMaterial(i);

			//		for(sint j = 0; j <= mat.getLastTextureStage(); ++j)
			//		{
			//			// if a texture is missing
			//			if (mat.isTextureFile(j) && mat.getTextureFileName(j) == "CTextureMultiFile:Dummy")
			//				allGood = false;
			//		}
			//	}

			//	// if all textures have been found for this set, skip other sets
			//	if (allGood)
			//		break;
			//}
		}

		// fix scale (some shapes have a different value)
		entity.setScale(1.f, 1.f, 1.f);

		UCamera Camera = _Scene->getCam();
		CVector max_radius = bbox.getHalfSize();

		CVector center = bbox.getCenter();
		entity.setPivot(center);
		center += pos;

		//_Scene->getCam().setPerspective(_CameraFocal * (float)Pi/180.f, (float)w/h, 0.1f, 1000);
		float fov = float(_CameraFocal * (float)Pi/180.0);
		//Camera.setPerspective (fov, 1.0f, 0.1f, 1000.0f);
		float radius = max(max(max_radius.x, max_radius.y), max_radius.z);
		if (radius == 0.f) radius = 1.f;
		float left, right, bottom, top, znear, zfar;
		Camera.getFrustum(left, right, bottom, top, znear, zfar);
		float dist = radius / (tan(fov/2));
		CVector eye(center);
		/*	if (axis == CVector::I)
		eye.y -= dist+radius;
		else if (axis == CVector::J)
		eye.x += dist+radius;
		*/
		//	quat.normalize();

		CVector ax(quat.getAxis());

		//	float angle = quat.getAngle();
		/*
		if (ax.isNull())
		{
		if (int(angle*100.f) == int(NLMISC::Pi * 200.f))
		{
		ax = CVector::J;
		}
		}
		else 
		*/
		if (ax.isNull() || ax == CVector::I)
		{
			ax = CVector::J;
		}
		else if (ax == -CVector::K)
		{
			ax = -CVector::J;
		}
		/*	else if (ax.x < -0.9f && ax.y == 0.f && ax.z == 0.f)
		{
		ax = -CVector::J ;
		}
		*/
		//	ax.normalize();

		eye -= ax * (dist+radius);
		if (high_z)
			eye.z += max_radius.z/1.0f;
		Camera.lookAt(eye, center);
		setupLight(eye, center - eye);
	}

	bool CObjectViewer::setupLight(const CVector &position, const CVector &direction)
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

} /* namespace NLQT */
