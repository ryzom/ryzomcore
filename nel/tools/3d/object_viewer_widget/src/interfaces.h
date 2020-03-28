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

#ifndef INTERFACES_H
#define INTERFACES_H

#include "stdpch.h"
#include "entity.h"

#include <QtPlugin>

namespace NL3D 
{
	class UPlayListManager;
	class UDriver;
	class UScene;
}

namespace NLQT 
{

	class IObjectViewer 
	{

	public:

		virtual ~IObjectViewer() {}

		virtual QString name() const = 0;

		// object viewer stuff
		virtual void init() = 0;
		virtual void release() = 0;
		virtual void updateInput() = 0;
		virtual void renderDriver() = 0;
		virtual void renderScene() = 0;
		virtual void renderDebug2D() = 0;
		virtual void saveScreenshot(const std::string &nameFile, bool jpg, bool png, bool tga) = 0;
		virtual bool loadMesh (const std::string &meshFileName, const std::string &skelFileName) = 0;
		virtual void resetScene() = 0;
		virtual void setBackgroundColor(NLMISC::CRGBA backgroundColor) = 0;
		virtual void setGraphicsDriver(bool Direct3D) = 0;
		virtual void setSizeViewport(uint16 w, uint16 h) = 0;
		virtual void setBloomEffect(bool enabled) = 0;
		virtual void setCurrentObject(const std::string &name) = 0;
		virtual const std::string& getCurrentObject() = 0;
		virtual CEntity& getEntity(const std::string &name) = 0;
		virtual void getListObjects(std::vector<std::string> &listObj) = 0;
		virtual NLMISC::CRGBA getBackgroundColor() = 0;
		virtual bool getDirect3D() = 0;
		virtual bool getBloomEffect() const = 0;
		virtual NL3D::UDriver *getDriver() = 0;
		virtual NL3D::UScene *getScene() = 0;
		virtual NL3D::UPlayListManager *getPlayListManager() = 0;
		virtual void setCamera(NL3D::UScene *scene, NLMISC::CAABBox &bbox, NL3D::UTransform &entity, bool high_z) = 0;
		virtual bool setupLight(const NLMISC::CVector &position, const NLMISC::CVector &direction) = 0;
		virtual void setVisible(bool visible) = 0;
		virtual QWidget* getWidget() = 0;
		virtual void setNelContext(NLMISC::INelContext &nelContext) = 0;
		virtual QIcon* saveOneImage(std::string shapename) = 0;

	};

} /* namespace NLQT */

Q_DECLARE_INTERFACE(NLQT::IObjectViewer,"com.ryzom.dev.IObjectViewer/0.1")

#endif 