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
#include "tune_mrm_dialog.h"

// NeL includes
#include <nel/3d/u_scene.h>

// Project includes
#include "modules.h"

const int sliderStepSize = 5000;

namespace NLQT
{

CTuneMRMDialog::CTuneMRMDialog(QWidget *parent)
	: QDockWidget(parent)
{
	_ui.setupUi(this);

	connect(_ui.maxValueSlider, SIGNAL(valueChanged(int)), this, SLOT(setMaxValue(int)));
	connect(_ui.currentValueSlider, SIGNAL(valueChanged(int)), this, SLOT(setCurrentValue(int)));

	_ui.maxValueSlider->setValue(_ui.maxValueSlider->maximum());
}

CTuneMRMDialog::~CTuneMRMDialog()
{
}

void CTuneMRMDialog::setMaxValue(int value)
{
	int actualMaxValue = value * sliderStepSize;
	int actualValue = float(actualMaxValue) * _ui.currentValueSlider->value() / _ui.currentValueSlider->maximum();

	_ui.currentValueSlider->setMaximum(actualMaxValue);
	_ui.currentValueSlider->setValue(actualValue);
	_ui.maxValueSpinBox->setValue(actualMaxValue);
}

void CTuneMRMDialog::setCurrentValue(int value)
{
	Modules::objView().getScene()->setGroupLoadMaxPolygon("Skin", value);
	_ui.currentValueSpinBox->setValue(value);
}

} /* namespace NLQT */
