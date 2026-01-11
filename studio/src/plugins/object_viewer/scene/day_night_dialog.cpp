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
#include "day_night_dialog.h"

// NeL incldes
#include "nel/3d/u_water.h"

// Project includes
#include "modules.h"

namespace NLQT
{

CDayNightDialog::CDayNightDialog(QWidget *parent)
	: QDockWidget(parent)
{
	_ui.setupUi(this);

	connect(_ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setDayNight(int)));
}

CDayNightDialog::~CDayNightDialog()
{
}

void CDayNightDialog::setDayNight(int value)
{
	NL3D::UWaterHeightMapManager::setBlendFactor(Modules::objView().getDriver(), float(value) / _ui.horizontalSlider->maximum());
}

} /* namespace NLQT */