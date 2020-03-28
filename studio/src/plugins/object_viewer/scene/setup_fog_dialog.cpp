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

// Project includes
#include "stdpch.h"
#include "setup_fog_dialog.h"
#include "object_viewer_constants.h"
#include "../core/icore.h"
#include "modules.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QColorDialog>
#include <QtCore/QSettings>

// NeL includes
#include <nel/misc/path.h>
#include <nel/3d/u_driver.h>

namespace NLQT
{

CSetupFog::CSetupFog(QWidget *parent)
	: QDockWidget(parent)
{
	ui.setupUi(this);

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

	// load fog value from config file
	ui.startDoubleSpinBox->setValue(settings->value("FogStart", 0.0).toDouble());
	ui.endDoubleSpinBox->setValue(settings->value("FogEnd", 0.0).toDouble());

	QColor color = settings->value("FogColor", QColor(80, 80, 80)).value<QColor>();
	colorFog = NLMISC::CRGBA(color.red(), color.green(), color.blue(), color.alpha());

	settings->endGroup();

	connect(ui.applyPushButton, SIGNAL(clicked()), this, SLOT(apply()));
	connect(ui.colorPushButton, SIGNAL(clicked()), this, SLOT(setColor()));
}

CSetupFog::~CSetupFog()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

	settings->setValue("FogStart", ui.startDoubleSpinBox->value());
	settings->setValue("FogEnd", ui.endDoubleSpinBox->value());

	QColor color(colorFog.R, colorFog.G, colorFog.B, colorFog.A);
	settings->setValue("FogColor", color);

	settings->endGroup();
	settings->sync();
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
