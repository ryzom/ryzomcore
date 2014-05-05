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

#ifndef OBJECT_VIEWER_H
#define OBJECT_VIEWER_H

#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <map>

// NeL includes
#include <nel/misc/rgba.h>
#include <nel/3d/event_mouse_listener.h>

// Project includes
#include "entity.h"

namespace NL3D
{
class UDriver;
class UScene;
class ULight;
class UCamera;
class UTextContext;
class UPlayListManager;
class U3dMouseListener;
class UInstanceGroup;
class CScene;
class IDriver;
}

namespace NLQT
{

/**
@class CObjectViewer
@brief The class initializes the driver and creates a scene, provides basic control functions over the stage driver.
@details The class initializes the driver (by choosing OpenGL or Direct3D), and creates a scene (set an aspect), respectively
creates a light as well, load the font that is available further for
all other subsystems (eg: the signature of the coordinate axes) and Mouse Listener.
Settings are loaded from the configuration file.
Also, the class provides the following features to scene control:
- Loading and displaying of the shape, water shape and the particle system(loadMesh()). As well as the scene reset. (resetScene())
- Provides access to a animation object (getEntity(), getListObjects()).
- Select of current object for various operation (mainly related to the animation and editing skeleton):setCurrentObject(), getCurrentObject().
- Operations with the viewport, setting the correct perspective and creating of a screenshot.
- Function's updating keyboard and mouse (acts on the camera updateInput()), update(updateAnimatePS(), updateAnimation())
and render the scene (renderDriver(), renderScene()).
- Provides access to a general NeL systems (getDriver(), getScene(), getPlayListManager(), getTextContext(), get3dMouseListener()).
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

	void reloadTextures();

	/// Make a screenshot of the current scene and save.
	void saveScreenshot(const std::string &nameFile, bool jpg, bool png, bool tga);

	/// Load a mesh or particle system and add to current scene.
	/// @param meshFileName - name loading shape or ps file.
	/// @param skelFileName - name loading skeletin file.
	/// @return true if file have been loaded, false if file have not been loaded.
	bool loadMesh (const std::string &meshFileName, const std::string &skelFileName);

	bool loadInstanceGroup(const std::string &igName);

	void setCamera(NLMISC::CAABBox &bbox, NL3D::UTransform &entity, bool high_z);

	/// Reset current scene.
	void resetScene();

	/// Update the animation time for Particle System animation.
	/// @param deltaTime - set the manual animation time.
	void updateAnimatePS(uint64 deltaTime = 0);

	/// Update animation from the scene
	/// @param time - current time in second
	void updateAnimation(NL3D::TAnimationTime time);

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

	void setBloomEffect(bool enabled)
	{
		_BloomEffect = enabled;
	}

	/// Select instance from the scene
	/// @param name - name instance,  "" if no instance edited
	void setCurrentObject(const std::string &name);

	/// Get current instance from the scene
	/// @return name current instance, "" if no instance edited
	std::string getCurrentObject() const
	{
		return _CurrentInstance;
	}

	/// Get entity from the scene
	/// @return ref Entity
	CEntity &getEntity(const std::string &name);

	/// Get full list instances from the scene
	/// @param listObj - ref of return list instances
	void getListObjects(std::vector<std::string> &listObj);

	/// Get value background color.
	/// @return  background color.
	inline NLMISC::CRGBA getBackgroundColor() const
	{
		return _BackgroundColor;
	}

	/// Get type driver.
	/// @return true if have used Direct3D driver, false OpenGL driver.
	inline bool getDirect3D() const
	{
		return _Direct3D;
	}

	inline bool getBloomEffect() const
	{
		return _BloomEffect;
	}

	/// Get a game interface for window driver.
	/// @return pointer to the driver.
	inline NL3D::UDriver *getDriver() const
	{
		return _Driver;
	}

	NL3D::IDriver *getIDriver() const
	{
		return _IDriver;
	}

	/// Get a game interface for scene.
	/// @return pointer to the scene.
	inline NL3D::UScene *getScene() const
	{
		return _Scene;
	}

	NL3D::CScene *getCScene() const
	{
		return _CScene;
	}

	/// Get a manager of playlist
	/// @return pointer to the UPlayListManager
	inline NL3D::UPlayListManager *getPlayListManager() const
	{
		return _PlayListManager;
	}

	/// Get a game interface to render string
	/// @return pointer to the UPlayListManager
	inline NL3D::UTextContext *getTextContext() const
	{
		return _TextContext;
	}

	/// Get a 3d mouse listener
	/// @return pointer to the U3dMouseListener
	inline NL3D::U3dMouseListener *get3dMouseListener() const
	{
		return _MouseListener;
	}

private:
	void loadConfig();

	void saveConfig();

	// Delete all entities
	void deleteEntities();

	NLMISC::CRGBA 			_BackgroundColor;
	NL3D::IDriver			*_IDriver;
	NL3D::CScene			*_CScene;

	NL3D::UDriver 			*_Driver;
	NL3D::UScene 			*_Scene;
	NL3D::UPlayListManager	*_PlayListManager;
	NL3D::ULight 			*_Light;
	NL3D::UCamera 			*_Camera;
	NL3D::UTextContext 		*_TextContext;
	NL3D::U3dMouseListener		*_MouseListener;
	std::vector<NL3D::UInstanceGroup *>	_ListIG;

	// The entities storage
	CEntities		_Entities;

	float _CameraFocal;

	std::string _FontName;

	bool _Direct3D;
	bool _BloomEffect;
	int _BloomDensity;
	bool _BloomSquare;

	std::string _CurrentInstance;

};/* class CObjectViewer */

} /* namespace NLQT */

#endif // OBJECT_VIEWER_H
