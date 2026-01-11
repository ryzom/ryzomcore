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
#include "camera_control.h"

// STL includes

// Qt includes

// NeL includes
#include "nel/misc/debug.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include <nel/3d/u_camera.h>
#include <nel/3d/u_3d_mouse_listener.h>

// Project includes
#include "modules.h"
#include "object_viewer_constants.h"

static int camId = 0;

namespace NLQT
{

CCameraItem::CCameraItem(const QString &name):
	_cameraFocal(75),
	_speed(5.0),
	_active(false),
	_name(name)
{
	_camera = Modules::objView().getScene()->createCamera();
	_camera.setTransformMode (NL3D::UTransformable::DirectMatrix);
	reset();
}

CCameraItem::~CCameraItem()
{
	Modules::objView().getScene()->deleteCamera(_camera);
}

void CCameraItem::setActive(bool active)
{
	if (active)
	{
		sint w = Modules::objView().getDriver()->getWindowWidth();
		sint h = Modules::objView().getDriver()->getWindowHeight();
		_camera.setPerspective(_cameraFocal * float(NLMISC::Pi) / 180.f, float(w) / h, 0.1f, 1000);
		Modules::objView().getScene()->setCam(_camera);
		setupListener();
	}
	else
	{
		_hotSpot = Modules::objView().get3dMouseListener()->getHotSpot();
	}
	_active = active;
}

void CCameraItem::setSpeed(float value)
{
	_speed = value;
	Modules::objView().get3dMouseListener()->setSpeed(_speed);
}

void CCameraItem::reset()
{
	_hotSpot = NLMISC::CVector(0, 0, 0);
	float radius=10.f;

	// Setup camera
	_camera.lookAt(_hotSpot + NLMISC::CVector(0.57735f, 0.57735f, 0.57735f) * radius, _hotSpot);

	if (_active)
		setupListener();
}

void CCameraItem::setupListener()
{
	NL3D::U3dMouseListener *_mouseListener = Modules::objView().get3dMouseListener();
	_mouseListener->setMatrix (_camera.getMatrix());
	_mouseListener->setFrustrum (_camera.getFrustum());
	_mouseListener->setViewport (NL3D::CViewport());
	_mouseListener->setHotSpot (_hotSpot);
	Modules::objView().get3dMouseListener()->setSpeed(_speed);
}

CCameraControl::CCameraControl(QWidget *parent)
	: QObject(parent),
	  _currentCamera(0)
{
	_camToolBar = new QToolBar(tr("CameraControl"), parent);

	_fpsAction = _camToolBar->addAction(tr("Camera frs"));
	_fpsAction->setIcon(QIcon(Constants::ICON_CAMERA_FPS));
	_fpsAction->setStatusTip(tr("Set firstPerson camera mode"));
	_fpsAction->setCheckable(true);

	_edit3dAction = _camToolBar->addAction(tr("Camera 3dEdit"));
	_edit3dAction->setIcon(QIcon(Constants::ICON_CAMERA_3DEDIT));
	_edit3dAction->setStatusTip(tr("Set edit3d camera mode"));
	_edit3dAction->setCheckable(true);

	QActionGroup *cameraModeGroup = new QActionGroup(this);
	cameraModeGroup->addAction(_fpsAction);
	cameraModeGroup->addAction(_edit3dAction);
	_edit3dAction->setChecked(true);

	connect(_fpsAction, SIGNAL(triggered()), this, SLOT(setFirstPersonMode()));
	connect(_edit3dAction, SIGNAL(triggered()), this, SLOT(setEditMode()));

	_renderModeMenu = new QMenu(tr("Render Mode"), _camToolBar);
	_renderModeMenu->setIcon(QIcon(":/images/polymode.png"));
	_camToolBar->addAction(_renderModeMenu->menuAction());
	connect(_renderModeMenu->menuAction(), SIGNAL(triggered()), this, SLOT(setRenderMode()));

	QSignalMapper *modeMapper = new QSignalMapper(this);

	_pointRenderModeAction = _renderModeMenu->addAction(tr("Point mode"));
	_pointRenderModeAction->setIcon(QIcon(":/images/rmpoints.png"));
	_pointRenderModeAction->setStatusTip(tr("Set point render mode"));
	connect(_pointRenderModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	modeMapper->setMapping(_pointRenderModeAction, 0);

	_lineRenderModeAction = _renderModeMenu->addAction(tr("Line mode"));
	_lineRenderModeAction->setStatusTip(tr("Set line render mode"));
	_lineRenderModeAction->setIcon(QIcon(":/images/rmline.png"));
	connect(_lineRenderModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	modeMapper->setMapping(_lineRenderModeAction, 1);

	_fillRenderModeAction = _renderModeMenu->addAction(tr("Fill mode"));
	_fillRenderModeAction->setIcon(QIcon(":/images/rmfill.png"));
	_fillRenderModeAction->setStatusTip(tr("Set fill render mode"));
	connect(_fillRenderModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	modeMapper->setMapping(_fillRenderModeAction, 2);

	connect(modeMapper, SIGNAL(mapped(int)), this, SLOT(setRenderMode(int)));

	_camToolBar->addSeparator();
	_speedLabel = new QLabel(tr("Speed:"), _camToolBar);
	_camToolBar->addWidget(_speedLabel);
	_speedSpinBox = new QSpinBox(_camToolBar);
	_speedSpinBox->setMinimum(1);
	_speedSpinBox->setMaximum(1000);
	_camToolBar->addWidget(_speedSpinBox);
	connect(_speedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setSpeed(int)));

	_camToolBar->addSeparator();
	_addCamAction = _camToolBar->addAction(tr("Create camera"));
	_addCamAction->setIcon(QIcon(Constants::ICON_CAMERA_ADD));
	_addCamAction->setStatusTip(tr("Create new camera"));
	connect(_addCamAction, SIGNAL(triggered()), this, SLOT(addCamera()));

	_delCamAction = _camToolBar->addAction(tr("Delete camera"));
	_delCamAction->setIcon(QIcon(Constants::ICON_CAMERA_DEL));
	_delCamAction->setStatusTip(tr("Delete current camera"));
	connect(_delCamAction, SIGNAL(triggered()), this, SLOT(delCamera()));

	_listCamComboBox = new QComboBox(_camToolBar);
	connect(_listCamComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCamera(int)));
	_listCamComboBox->setCurrentIndex(createCamera(tr("defaultCamera")));
	_camToolBar->addWidget(_listCamComboBox);

	_camToolBar->addSeparator();
	_resetCamAction = _camToolBar->addAction(tr("Reset camera"));
	_resetCamAction->setIcon(QIcon(Constants::ICON_RESET_CAMERA));
	_resetCamAction->setStatusTip(tr("Reset current camera"));
	//_resetCamAction->setShortcut(tr("Ctrl+R"));
	connect(_resetCamAction, SIGNAL(triggered()), this, SLOT(resetCamera()));
}

CCameraControl::~CCameraControl()
{
	for(size_t i = 0; i < _cameraList.size(); ++i)
		delete _cameraList[i];
	_cameraList.clear();
}

void CCameraControl::setEditMode()
{
	Modules::objView().get3dMouseListener()->setMouseMode(NL3D::U3dMouseListener::edit3d);
}

void CCameraControl::setFirstPersonMode()
{
	Modules::objView().get3dMouseListener()->setMouseMode(NL3D::U3dMouseListener::firstPerson);
}

void CCameraControl::addCamera()
{
	_listCamComboBox->setCurrentIndex(createCamera(tr("%1_Camera").arg(++camId)));
}

void CCameraControl::delCamera()
{
	int index = _listCamComboBox->currentIndex();
	_listCamComboBox->setCurrentIndex(index - 1);

	_listCamComboBox->removeItem(index);
	delete _cameraList[index];
	_cameraList.erase(_cameraList.begin() + index);
}

void CCameraControl::setSpeed(int value)
{
	nlassert(_currentCamera);
	_currentCamera->setSpeed(value);
}

void CCameraControl::changeCamera(int index)
{
	if (_currentCamera)
		_currentCamera->setActive(false);

	if (index == 0)
		_delCamAction->setEnabled(false);
	else
		_delCamAction->setEnabled(true);

	_currentCamera = _cameraList[index];

	nlassert(_currentCamera);
	_currentCamera->setActive(true);
	_speedSpinBox->setValue(int(_currentCamera->getSpeed()));
}

void CCameraControl::setRenderMode(int value)
{
	switch (value)
	{
	case 0:
		Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Point);
		break;
	case 1:
		Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Line);
		break;
	case 2:
		Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Filled);
		break;
	}
}

void CCameraControl::setRenderMode()
{
	switch (Modules::objView().getDriver()->getPolygonMode())
	{
	case NL3D::UDriver::Filled:
		Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Line);
		break;
	case NL3D::UDriver::Line:
		Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Point);
		break;
	case NL3D::UDriver::Point:
		Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Filled);
		break;
	}
}

void CCameraControl::resetCamera()
{
	nlassert(_currentCamera);
	_currentCamera->reset();
}

int CCameraControl::createCamera(const QString &name)
{
	CCameraItem *newCamera = new CCameraItem(name);
	_cameraList.push_back(newCamera);
	_listCamComboBox->addItem(newCamera->getName());
	return _cameraList.size() - 1;
}

} /* namespace NLQT */
