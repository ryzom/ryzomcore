/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

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

#include "stdpch.h"
#include "object_viewer.h"

// STL includes

// NeL includes
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
#include <nel/3d/u_instance_group.h>
#include <nel/3d/material.h>
#include <nel/3d/driver.h>
#include <nel/3d/scene_user.h>
#include <nel/3d/bloom_effect.h>

// Project includes
#include "modules.h"
#include "configuration.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace NLQT
{

CObjectViewer::CObjectViewer()
	: _IDriver(0),
	  _CScene(0),
	  _Driver(0),
	  _Scene(0),
	  _TextContext(0),
	  _CameraFocal(75),
	  _CurrentInstance(""),
	  _BloomEffect(false)
{

}

CObjectViewer::~CObjectViewer()
{
}

void CObjectViewer::init(nlWindow wnd, uint16 w, uint16 h)
{
	//H_AUTO2
	nldebug("CObjectViewert::init");

	loadConfig();

	// create the driver
	nlassert(!_Driver);

	_Driver = UDriver::createDriver(0, _Direct3D, 0);
	nlassert(_Driver);

	// initialize the window with config file values
	_Driver->setDisplay(wnd, NL3D::UDriver::CMode(w, h, 32));

	// Create a scene
	_Scene = _Driver->createScene(false);
	_Scene->setPolygonBalancingMode(NL3D::UScene::PolygonBalancingClamp);

	_Driver->enableUsedTextureMemorySum();

	_Light = ULight::createLight();

	// set mode of the light
	_Light->setMode(ULight::DirectionalLight);

	// set position of the light
	_Light->setPosition(CVector(-20.f, 30.f, 10.f));

	// white light
	_Light->setAmbiant(CRGBA(255, 255, 255));

	// set and enable the light
	_Driver->setLight(0, *_Light);
	_Driver->enableLight(0);

	_PlayListManager = _Scene->createPlayListManager();

	_Scene->enableLightingSystem(true);

	NLMISC::CVector hotSpot=NLMISC::CVector(0,0,0);

	_MouseListener = _Driver->create3dMouseListener();
	_MouseListener->setMouseMode(U3dMouseListener::edit3d);

	// set the cache size for the font manager(in bytes)
	_Driver->setFontManagerMaxMemory(2097152);

	// create the text context
	nlassert(!_TextContext);
	_TextContext = _Driver->createTextContext(_FontName);
	nlassert(_TextContext);

	NL3D::CBloomEffect::instance().setDriver(_Driver);
	NL3D::CBloomEffect::instance().setScene(_Scene);
	NL3D::CBloomEffect::instance().init(!_Direct3D);
	NL3D::CBloomEffect::instance().setDensityBloom(Modules::config().getConfigFile().getVar("BloomDensity").asInt());
	NL3D::CBloomEffect::instance().setSquareBloom(Modules::config().getConfigFile().getVar("BloomSquare").asBool());

	NL3D::CDriverUser *driver = dynamic_cast<NL3D::CDriverUser *>(Modules::objView().getDriver());
	_IDriver = driver->getDriver();

	NL3D::CSceneUser *scene = dynamic_cast<NL3D::CSceneUser *>(Modules::objView().getScene());
	_CScene = &scene->getScene();
}

void CObjectViewer::release()
{
	//H_AUTO2
	nldebug("CObjectViewer::release");

	saveConfig();

	// release text context
	nlassert(_TextContext);
	_Driver->deleteTextContext(_TextContext);
	_TextContext = 0;

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
	_Driver = 0;
}

void CObjectViewer::updateInput()
{
	_Driver->EventServer.pump();

	// New matrix from camera
	_Scene->getCam().setTransformMode(NL3D::UTransformable::DirectMatrix);
	_Scene->getCam().setMatrix (_MouseListener->getViewMatrix());
}

void CObjectViewer::renderDriver()
{
	// Render the scene.
	if((NL3D::CBloomEffect::instance().getDriver() != 0) && (_BloomEffect))
	{
		NL3D::CBloomEffect::instance().initBloom();
	}
	_Driver->clearBuffers(_BackgroundColor);
}

void CObjectViewer::renderScene()
{
	// render the scene
	_Scene->render();

	if((NL3D::CBloomEffect::instance().getDriver() != 0) && (_BloomEffect))
	{
		NL3D::CBloomEffect::instance().endBloom();
		NL3D::CBloomEffect::instance().endInterfacesDisplayBloom();
	}
}

void CObjectViewer::renderDebug2D()
{
}

void CObjectViewer::reloadTextures()
{
	// For each instances
	std::vector<std::string> listObjects;
	getListObjects(listObjects);

	for (size_t i = 0; i < listObjects.size(); ++i)
	{
		// Get the shape
		NL3D::UInstance instance = getEntity(listObjects[i]).getInstance();

		// For each material
		if (!instance.empty())
		{
			uint numMaterial = instance.getNumMaterials();
			uint mat;
			for (mat = 0; mat < numMaterial; mat++)
			{
				// Get the material
				NL3D::CMaterial *material = instance.getMaterial(mat).getObjectPtr();

				// For each texture
				int tex;
				for (tex = 0; tex < NL3D::IDRV_MAT_MAXTEXTURES; tex++)
				{
					ITexture *texture = material->getTexture(tex);

					// Touch it!
					if (texture)
						getIDriver()->invalidateShareTexture(*texture);
				}
			}
		}
	}
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
	if (_Entities.count(fileName) != 0)
		return false;

	CPath::addSearchPath(CFile::getPath(meshFileName), false, false);

	// create instance of the mesh character
	UInstance Entity = _Scene->createInstance(meshFileName);

	// if we can't create entity, skip it
	if (Entity.empty()) return false;

	CAABBox bbox;
	Entity.getShapeAABBox(bbox);
	setCamera(bbox , Entity, true);

	_MouseListener->setMatrix(_Scene->getCam().getMatrix());

	USkeleton Skeleton = _Scene->createSkeleton(skelFileName);

	// TODO: remade at typedef std::map<std::string, *CEntity>	CEntities;
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

bool CObjectViewer::loadInstanceGroup(const std::string &igName)
{
	CPath::addSearchPath (CFile::getPath(igName));
	UInstanceGroup *ig = UInstanceGroup::createInstanceGroup(igName);
	if (ig == 0)
		return false;
	ig->addToScene(*_Scene, _Driver);
	ig->unfreezeHRC();
	_ListIG.push_back(ig);
	return true;
}

void CObjectViewer::setCamera(NLMISC::CAABBox &bbox, NL3D::UTransform &entity, bool high_z)
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
	UCamera Camera = _Scene->getCam();
	CVector max_radius = bbox.getHalfSize();
	CVector center = bbox.getCenter();
	entity.setPivot(center);
	center += pos;
	float fov = float(_CameraFocal * (float)Pi/180.0);
	float radius = max(max(max_radius.x, max_radius.y), max_radius.z);
	if (radius == 0.f) radius = 1.f;
	float left, right, bottom, top, znear, zfar;
	Camera.getFrustum(left, right, bottom, top, znear, zfar);
	float dist = radius / (tan(fov/2));
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
	get3dMouseListener()->setHotSpot(center);
	Camera.lookAt(eye, center);
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

void CObjectViewer::updateAnimatePS(uint64 deltaTime)
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

void CObjectViewer::updateAnimation(NL3D::TAnimationTime time)
{
	for (EIT eit = _Entities.begin(); eit != _Entities.end(); ++eit)
	{
		CEntity	&entity = (*eit).second;
		entity.update(time);
	}

	// Animate scene animation
	Modules::objView().getPlayListManager()->setup(time);
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

void CObjectViewer::loadConfig()
{
	// set background color from config
	Modules::config().setAndCallback("BackgroundColor", CConfigCallback(this, &CObjectViewer::cfcbBackgroundColor));

	// set graphics driver from config
	Modules::config().setAndCallback("GraphicsDriver",CConfigCallback(this,&CObjectViewer::cfcbGraphicsDriver));

	Modules::config().setAndCallback("CameraFocal",CConfigCallback(this,&CObjectViewer::cfcbCameraFocal));

	Modules::config().setAndCallback("FontName",CConfigCallback(this,&CObjectViewer::cfcbFontName));

	Modules::config().setAndCallback("BloomEffect",CConfigCallback(this,&CObjectViewer::cfcbBloomEffect));
}

void CObjectViewer::saveConfig()
{
	Modules::config().dropCallback("BackgroundColor");
	Modules::config().dropCallback("GraphicsDriver");
	Modules::config().dropCallback("CameraFocal");
	Modules::config().dropCallback("FontName");
	Modules::config().dropCallback("BloomEffect");
}

void CObjectViewer::deleteEntities()
{
	_Entities.clear();

	for(size_t i = 0; i < _ListIG.size(); ++i)
	{
		_ListIG[i]->removeFromScene(*_Scene);
		delete _ListIG[i];
	}
	_ListIG.clear();
}

void CObjectViewer::cfcbBackgroundColor(NLMISC::CConfigFile::CVar &var)
{
	// read variable from config file
	_BackgroundColor = CRGBA(var.asInt(0), var.asInt(1), var.asInt(2));
}

void CObjectViewer::cfcbGraphicsDriver(NLMISC::CConfigFile::CVar &var)
{
	// Choose driver opengl to work correctly under Linux example
	_Direct3D = false; //_Driver = OpenGL;

#ifdef NL_OS_WINDOWS
	std::string driver = var.asString();
	if (driver == "Direct3D") _Direct3D = true; //m_Driver = Direct3D;
	else if (driver == "OpenGL") _Direct3D = false; //m_Driver = OpenGL;
	else nlwarning("Invalid driver specified, defaulting to OpenGL");
#endif
}

void CObjectViewer::cfcbCameraFocal(NLMISC::CConfigFile::CVar &var)
{
	_CameraFocal = var.asFloat();
}

void CObjectViewer::cfcbFontName(NLMISC::CConfigFile::CVar &var)
{
	_FontName = var.asString();
}

void CObjectViewer::cfcbBloomEffect(NLMISC::CConfigFile::CVar &var)
{
	_BloomEffect = var.asBool();
}

} /* namespace NLQT */
