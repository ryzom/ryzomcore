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
#include "global_wind_dialog.h"

// Project includes
#include "modules.h"
#include <nel/3d/u_scene.h>

namespace NLQT {

CGlobalWindDialog::CGlobalWindDialog(QWidget *parent)
    : QDockWidget(parent)
{
	_ui.setupUi(this);
	
	//_ui.directionWidget->setWrapper(&_DirectionWrapper);
	
	connect(_ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setWndPower(int)));
	connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWnd(bool)));
}

CGlobalWindDialog::~CGlobalWindDialog()
{
}

void CGlobalWindDialog::setWndPower(int value)
{
	float fValue = float(value) / _ui.horizontalSlider->maximum();
	_ui.doubleSpinBox->setValue(fValue);
	Modules::objView().getScene()->setGlobalWindPower(fValue);
}

void CGlobalWindDialog::updateWnd(bool visible)
{
	if (!visible)
		return;

	_ui.horizontalSlider->setValue(int(Modules::objView().getScene()->getGlobalWindPower() * _ui.horizontalSlider->maximum()));
//	_ui.directionWidget->updateUi();
}
/*
NLMISC::CVector CGlobalWindDialog::CDirectionWrapper::get(void) const
{
	return Modules::objView().getScene()->getGlobalWindDirection();
}

void CGlobalWindDialog::CDirectionWrapper::set(const NLMISC::CVector &d)
{
	Modules::objView().getScene()->setGlobalWindDirection(d);
}
*/
} /* namespace NLQT */