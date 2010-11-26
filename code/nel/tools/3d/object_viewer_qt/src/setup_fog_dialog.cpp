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
#include "setup_fog_dialog.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QColorDialog>

// NeL includes
#include <nel/misc/path.h>
#include <nel/3d/u_driver.h>

// Project includes
#include "modules.h"

using namespace NL3D;
using namespace NLMISC;

namespace NLQT
{

CSetupFog::CSetupFog(QWidget *parent)
	: QDockWidget(parent)
{
	ui.setupUi(this);

	// load fog value from config file
	ui.startDoubleSpinBox->setValue(Modules::config().getValue("FogStart", 0.0));
	ui.endDoubleSpinBox->setValue(Modules::config().getValue("FogEnd", 0.0));
	colorFog = Modules::config().getValue("FogColor",CRGBA(0.0, 0.0, 0.0));

	connect(ui.applyPushButton, SIGNAL(clicked()), this, SLOT(apply()));
	connect(ui.colorPushButton, SIGNAL(clicked()), this, SLOT(setColor()));
}

CSetupFog::~CSetupFog()
{
	// save fog value from config file
	Modules::config().getConfigFile().getVar("FogStart").setAsFloat(ui.startDoubleSpinBox->value());
	Modules::config().getConfigFile().getVar("FogEnd").setAsFloat(ui.endDoubleSpinBox->value());
	Modules::config().getConfigFile().getVar("FogColor").setAsInt(colorFog.R, 0);
	Modules::config().getConfigFile().getVar("FogColor").setAsInt(colorFog.G, 1);
	Modules::config().getConfigFile().getVar("FogColor").setAsInt(colorFog.B, 2);

}

void CSetupFog::apply()
{
	Modules::objView().getDriver()->setupFog(ui.startDoubleSpinBox->value(),
			ui.endDoubleSpinBox->value(),
			colorFog);
	Modules::objView().getDriver()->enableFog(ui.enableFogCheckBox->isChecked());
}

void CSetupFog::setColor()
{
	QColor color = QColorDialog::getColor(QColor(colorFog.R,
										  colorFog.G,
										  colorFog.B));
	colorFog.set(color.red(), color.green(), color.blue());
}

} /* namespace NLQT */
