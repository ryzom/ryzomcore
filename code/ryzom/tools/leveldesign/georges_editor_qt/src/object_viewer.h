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

#ifndef OBJECT_VIEWER_H
#define OBJECT_VIEWER_H

#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <map>

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/misc/rgba.h>
#include <nel/3d/event_mouse_listener.h>

// Project includes
#include "entity.h"

namespace NL3D {
	class UDriver;
	class UScene;
	class ULight;
	class UInstance;
	class UCamera;
	class USkeleton;
	class UPlayListManager;
	class U3dMouseListener;
}

/**
namespace NLQT
@brief namespace NLQT
*/
namespace NLQT {

/**
@class CObjectViewer
A CObjectViewer class loading and viewing shape, particle system files.
*/
class CObjectViewer
{
public:
	/// Default constructor.
	CObjectViewer();
	
	virtual ~CObjectViewer();

	/// Init a driver and create scene.
	/// @param wnd - handle window.
	/// @param w - width window.
	/// @param h - height window.
	void init(nlWindow wnd, uint16 w, uint16 h);
	
	void reinit(nlWindow wnd, uint16 w, uint16 h);

	/// Release class.
	void release();

	/// Update mouse and keyboard events. And update camera matrix.
	void updateInput();
	
	/// Render Driver (clear all buffers and set background color).
	void renderDriver();
	
	/// Render current scene.
	void renderScene();
	
	/// Render Debug 2D (stuff for dev).
	void renderDebug2D();

	/// Make a screenshot of the current scene and save.
	void saveScreenshot(const std::string &nameFile, bool jpg, bool png, bool tga);
	
	/// Load a mesh or particle system and add to current scene.
	/// @param meshFileName - name loading shape or ps file.
	/// @param skelFileName - name loading skeletin file.
	/// @return true if file have been loaded, false if file have not been loaded.
	bool loadMesh (const std::string &meshFileName, const std::string &skelFileName);
	
	/// Reset current scene.
	void resetScene();
	
	/// Update the navigation camera.
	/// @param deltaPsi - delta angle horizontal (radians).
	/// @param deltaPhi - delta angle vertical (radians).
	/// @param deltaDist - delta distance.
	void updateCamera(float deltaPsi, float deltaPhi, float deltaDist);
	
	/// Set the background color.
	/// @param backgroundColor - background color.
	void setBackgroundColor(NLMISC::CRGBA backgroundColor);
	
	/// Set type driver.
	/// @param Direct3D - type driver (true - Direct3D) or (false -OpenGL)
	void setGraphicsDriver(bool Direct3D);
	
	/// Set size viewport for correct set perspective
	/// @param w - width window.
	/// @param h - height window.
	void setSizeViewport(uint16 w, uint16 h);
	void getSizeViewport(float &left, float &right, float &bottom, float &top, float &znear, float &zfar);
	
	/// Select instance from the scene
	/// @param name - name instance,  "" if no instance edited
	void setCurrentObject(const std::string &name);
	
	/// Get current instance from the scene
	/// @return name current instance, "" if no instance edited
	const std::string& getCurrentObject() { return _CurrentInstance; }
 
	/// Get entity from the scene
	/// @return ref Entity
	CEntity& getEntity(const std::string &name);
	
	/// Get full list instances from the scene
	/// @param listObj - ref of return list instances
	void getListObjects(std::vector<std::string> &listObj);
	
	/// Get value background color.
	/// @return  background color.
	NLMISC::CRGBA getBackgroundColor() { return _BackgroundColor; }
	
	/// Get type driver.
	/// @return true if have used Direct3D driver, false OpenGL driver.
	inline bool getDirect3D() { return _Direct3D; }

	/// Get a pointer to the driver.
	/// @return pointer to the driver.
	inline NL3D::UDriver *getDriver() { return _Driver; }
	
	/// Get a pointer to the scene.
	/// @return pointer to the scene.
	inline NL3D::UScene *getScene() { return _Scene; }
		
	/// Get a manager of playlist
	/// @return pointer to the UPlayListManager
	inline NL3D::UPlayListManager *getPlayListManager() { return _PlayListManager; }
	
private:
	void deleteEntity (CEntity &entity);

	/// Delete all entities
	void deleteEntities();
	
	/// Load background color from config file, intended for CConfiguration.
	void cfcbBackgroundColor(NLMISC::CConfigFile::CVar &var);
	void cfcbGraphicsDriver(NLMISC::CConfigFile::CVar &var);
	
	NLMISC::CRGBA 			_BackgroundColor;

	NL3D::UDriver 			*_Driver;
	NL3D::UScene 			*_Scene;
	NL3D::UPlayListManager		*_PlayListManager;
	NL3D::ULight 			*_Light;
	NL3D::UCamera 			*_Camera;
	NL3D::U3dMouseListener		*_MouseListener;

	// The entities storage
	CEntities		_Entities;
	
	/// Camera parameters.
	float _phi, _psi, _dist;
	
	bool _Direct3D;
	
	std::string _CurrentInstance;

	// a temporary solution, and later remove
	friend class CAnimationSetDialog;
};/* class CObjectViewer */

} /* namespace NLQT */

#endif // OBJECT_VIEWER_H
