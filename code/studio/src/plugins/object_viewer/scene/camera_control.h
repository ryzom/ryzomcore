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

#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

// STL includes

// Qt includes
#include <QtCore/QSignalMapper>
#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>

// NeL includes
#include <nel/3d/u_camera.h>
#include "nel/misc/vector.h"

// Project includes

namespace NLQT
{

class CCameraItem
{
public:
	CCameraItem(const QString &name);
	~CCameraItem();

	void setSpeed(float value);
	float getSpeed()
	{
		return _speed;
	}
	void setActive(bool active);
	void setName(const QString &name)
	{
		_name = name;
	}
	QString getName() const
	{
		return _name;
	}
	void reset();

private:
	void setupListener();

	NL3D::UCamera _camera;
	NLMISC::CVector _hotSpot;

	float _cameraFocal;
	float _speed;
	bool _active;
	QString _name;
};

class CCameraControl: public QObject
{
	Q_OBJECT

public:
	CCameraControl(QWidget *parent = 0);
	~CCameraControl();

	QToolBar *getToolBar() const
	{
		return _camToolBar;
	}

public Q_SLOTS:
	void setEditMode();
	void setFirstPersonMode();
	void addCamera();
	void delCamera();
	void setSpeed(int value);
	void changeCamera(int index);
	void setRenderMode(int value);
	void setRenderMode();
	void resetCamera();

private:
	int createCamera(const QString &name);

	QAction *_fpsAction;
	QAction *_edit3dAction;
	QAction *_pointRenderModeAction;
	QAction *_lineRenderModeAction;
	QAction *_fillRenderModeAction;
	QAction *_addCamAction;
	QAction *_delCamAction;
	QAction *_resetCamAction;
	QSpinBox *_speedSpinBox;
	QComboBox *_listCamComboBox;
	QMenu *_renderModeMenu;
	QLabel *_speedLabel;
	QToolBar *_camToolBar;

	CCameraItem *_currentCamera;
	std::vector<CCameraItem *> _cameraList;

}; /* class CCameraControl */

} /* namespace NLQT */

#endif // CAMERA_CONTROL_H
