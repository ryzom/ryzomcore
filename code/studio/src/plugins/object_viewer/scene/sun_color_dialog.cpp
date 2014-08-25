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
#include "sun_color_dialog.h"

// NeL includes
#include "nel/3d/u_scene.h"

// Project includes
#include "modules.h"

namespace NLQT
{

CSunColorDialog::CSunColorDialog(QWidget *parent)
	: QDockWidget(parent)
{
	_ui.setupUi(this);

	_ui.ambientWidget->setColor(Modules::objView().getScene()->getSunAmbient());
	_ui.diffuseWidget->setColor(Modules::objView().getScene()->getSunDiffuse());
	_ui.specularWidget->setColor(Modules::objView().getScene()->getSunSpecular());

	//connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(updateSunColor(bool)));
	connect(_ui.ambientWidget, SIGNAL(colorChanged(NLMISC::CRGBA)), this, SLOT(setAmbientSunColor(NLMISC::CRGBA)));
	connect(_ui.diffuseWidget, SIGNAL(colorChanged(NLMISC::CRGBA)), this, SLOT(setDiffuseSunColor(NLMISC::CRGBA)));
	connect(_ui.specularWidget, SIGNAL(colorChanged(NLMISC::CRGBA)), this, SLOT(setSpecularSunColor(NLMISC::CRGBA)));
}

CSunColorDialog::~CSunColorDialog()
{
}

void CSunColorDialog::setAmbientSunColor(NLMISC::CRGBA color)
{
	Modules::objView().getScene()->setSunAmbient(color);
}

void CSunColorDialog::setDiffuseSunColor(NLMISC::CRGBA color)
{
	Modules::objView().getScene()->setSunDiffuse(color);
}

void CSunColorDialog::setSpecularSunColor(NLMISC::CRGBA color)
{
	Modules::objView().getScene()->setSunSpecular(color);
}

} /* namespace NLQT */