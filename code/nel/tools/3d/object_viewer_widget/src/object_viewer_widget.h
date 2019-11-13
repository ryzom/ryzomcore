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

#ifndef OBJECT_VIEWER_WIDGET_H
#define OBJECT_VIEWER_WIDGET_H

// STL includes

// Qt includes
#include <QtOpenGL/QGLWidget>
#include <QTimer>

// NeL includes
#include <nel/misc/rgba.h>
#include <nel/misc/aabbox.h>

// Project includes
#include "entity.h"
#include "interfaces.h"

namespace NL3D
{
	class UDriver;
	class UScene;
	class ULight;
	class UInstance;
	class UCamera;
	class USkeleton;
	class UPlayListManager;
	class U3dMouseListener;
}

class QIcon;
/**
namespace NLQT
@brief namespace NLQT
*/
namespace NLQT
{
	class CObjectViewerWidget:
		public QWidget,
		public IObjectViewer
	{
		Q_OBJECT
		Q_INTERFACES(NLQT::IObjectViewer)

	public:
		/// Default constructor.
		CObjectViewerWidget(QWidget *parent = 0);
		virtual ~CObjectViewerWidget();

		virtual QPaintEngine* paintEngine() const { return NULL; }
		virtual void showEvent ( QShowEvent * event );

		void setNelContext(NLMISC::INelContext &nelContext);

		static CObjectViewerWidget  &objViewWid() { return *_objectViewerWidget; }

		/// Init a driver and create scene.
		void init();

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

		void setBloomEffect(bool enabled) { _BloomEffect = enabled; }

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

		inline bool getBloomEffect() const { return _BloomEffect; }

		/// Get a pointer to the driver.
		/// @return pointer to the driver.
		inline NL3D::UDriver *getDriver() { return _Driver; }

		/// Get a pointer to the scene.
		/// @return pointer to the scene.
		inline NL3D::UScene *getScene() { return _Scene; }

		/// Get a manager of playlist
		/// @return pointer to the UPlayListManager
		inline NL3D::UPlayListManager *getPlayListManager() { return _PlayListManager; }

		void setCamera(NL3D::UScene *scene, NLMISC::CAABBox &bbox, NL3D::UTransform &entity, bool high_z=false);
		bool setupLight(const NLMISC::CVector &position, const NLMISC::CVector &direction);

		QIcon* saveOneImage(std::string shapename);

		virtual void setVisible(bool visible);

		QWidget* getWidget() {return this;}

		virtual QString name() const {return ("ObjectViewerWidget");}

	protected:
#ifdef USE_QT5
		virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#else

#if defined(NL_OS_WINDOWS)
		virtual bool winEvent(MSG * message, long * result);
#elif defined(NL_OS_MAC)
		virtual bool macEvent(EventHandlerCallRef caller, EventRef event);
#elif defined(NL_OS_UNIX)
		virtual bool x11Event(XEvent *event);
#endif

#endif

	private Q_SLOTS:
		void updateRender();

	private:

		/// Update the animation time for Particle System animation.
		/// @param deltaTime - set the manual animation time.
		void updateAnimatePS(uint64 deltaTime = 0);

		static CObjectViewerWidget  *_objectViewerWidget;

		NLMISC::CLibraryContext *_LibContext;

		// render stuff
		QTimer *_mainTimer;
		bool _isGraphicsInitialized, _isGraphicsEnabled;

		void updateInitialization(bool visible);

		void deleteEntity (CEntity &entity);

		/// Delete all entities
		void deleteEntities();

		NLMISC::CRGBA 			_BackgroundColor;

		NL3D::UDriver 			*_Driver;
		NL3D::UScene 			*_Scene;
		NL3D::UPlayListManager  *_PlayListManager;
		NL3D::ULight 			*_Light;
		NL3D::UCamera 			*_Camera;
		NL3D::U3dMouseListener  *_MouseListener;

		// The entities storage
		CEntities		_Entities;

		/// Camera parameters.
		float _phi, _psi, _dist;
		float _CameraFocal;

		bool _Direct3D;
		bool _BloomEffect;

		std::string _CurrentInstance;

		// a temporary solution, and later remove
		friend class CAnimationSetDialog;

	};/* class CObjectViewerWidget */

} /* namespace NLQT */

#endif // OBJECT_VIEWER_WIDGET_H
