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
#include "vegetable_landscape_page.h"

// Project includes
#include "modules.h"

#define NL_VEGETABLE_EDIT_WIND_MAX_POWER 10.0
#define NL_VEGETABLE_EDIT_WIND_MAX_FREQUENCY 10.0
#define NL_VEGETABLE_EDIT_WIND_MAX_BENDSTART 1.0

namespace NLQT
{

CVegetableLandscapePage::CVegetableLandscapePage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	connect(_ui.powerHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setPowerSlider(int)));
	connect(_ui.bendStartHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setBendStartSlider(int)));
	connect(_ui.frequencyHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setFrequencySlider(int)));
	connect(_ui.showLandscapeCheckBox, SIGNAL(toggled(bool)), this, SLOT(setVisibleLandscape(bool)));
	connect(_ui.snapCameraToGroundCheckBox, SIGNAL(toggled(bool)), this, SLOT(setSnapCameraToGround(bool)));
	connect(_ui.diffuseColorWidget, SIGNAL(colorChanged(NLMISC::CRGBA)), this, SLOT(setColorDiffuse(NLMISC::CRGBA)));
	connect(_ui.ambientColorwidget, SIGNAL(colorChanged(NLMISC::CRGBA)), this, SLOT(setColorAmbient(NLMISC::CRGBA)));

	double windPower = Modules::veget().getVegetableWindPower();
	NLMISC::clamp(windPower, 0, NL_VEGETABLE_EDIT_WIND_MAX_POWER);
	_ui.powerHorizontalSlider->setValue(int(windPower * _ui.powerHorizontalSlider->maximum() / NL_VEGETABLE_EDIT_WIND_MAX_POWER));

	double bendStar = Modules::veget().getVegetableWindBendStart();
	NLMISC::clamp(bendStar, 0, NL_VEGETABLE_EDIT_WIND_MAX_BENDSTART);
	_ui.bendStartHorizontalSlider->setValue(int(bendStar * _ui.bendStartHorizontalSlider->maximum() / NL_VEGETABLE_EDIT_WIND_MAX_BENDSTART));

	double frequency = Modules::veget().getVegetableWindFrequency();
	NLMISC::clamp(frequency, 0, NL_VEGETABLE_EDIT_WIND_MAX_FREQUENCY);
	_ui.frequencyHorizontalSlider->setValue(int(frequency * _ui.frequencyHorizontalSlider->maximum() / NL_VEGETABLE_EDIT_WIND_MAX_FREQUENCY));

	_ui.diffuseColorWidget->setColor(Modules::veget().getVegetableDiffuseLight(), false);
	_ui.ambientColorwidget->setColor(Modules::veget().getVegetableAmbientLight(), false);
}

CVegetableLandscapePage::~CVegetableLandscapePage()
{
}

void CVegetableLandscapePage::setPowerSlider(int value)
{
	float windPower = float(value * NL_VEGETABLE_EDIT_WIND_MAX_POWER / _ui.powerHorizontalSlider->maximum());
	_ui.powerDoubleSpinBox->setValue(windPower);
	Modules::veget().setVegetableWindPower(windPower);
}

void CVegetableLandscapePage::setBendStartSlider(int value)
{
	float bendStar = float(value * NL_VEGETABLE_EDIT_WIND_MAX_BENDSTART / _ui.bendStartHorizontalSlider->maximum());
	_ui.bendStartDoubleSpinBox->setValue(bendStar);
	Modules::veget().setVegetableWindBendStart(bendStar);
}

void CVegetableLandscapePage::setFrequencySlider(int value)
{
	float frequency = float(value * NL_VEGETABLE_EDIT_WIND_MAX_FREQUENCY / _ui.frequencyHorizontalSlider->maximum());
	_ui.frequiencyDoubleSpinBox->setValue(frequency);
	Modules::veget().setVegetableWindFrequency(frequency);
}

void CVegetableLandscapePage::setVisibleLandscape(bool state)
{
	if(state)
	{
		// Landscape not created ??
		if(!Modules::veget().isVegetableLandscapeCreated())
		{
			// if success to create / Load the landscape.
			if(Modules::veget().createVegetableLandscape())
			{
				_ui.windGroupBox->setEnabled(true);
				_ui.colorTabWidget->setEnabled(true);
				_ui.snapCameraToGroundCheckBox->setEnabled(true);
				// refresh view, independently of checkBox
				//doRefreshVegetableDisplay();
			}
			else
			{
				// Failed in load, never retry.
				_ui.showLandscapeCheckBox->setChecked(false);
			}
		}

		// show the landscape
		Modules::veget().showVegetableLandscape();
		Q_EMIT changeVisibleLandscape(true);
	}
	else
	{
		Modules::veget().hideVegetableLandscape();
		Q_EMIT changeVisibleLandscape(false);
	}
}

void CVegetableLandscapePage::setSnapCameraToGround(bool state)
{
	Modules::veget().snapToGroundVegetableLandscape(state);
}

void CVegetableLandscapePage::setColorAmbient(NLMISC::CRGBA color)
{
	Modules::veget().setVegetableAmbientLight(color);
}

void CVegetableLandscapePage::setColorDiffuse(NLMISC::CRGBA color)
{
	Modules::veget().setVegetableDiffuseLight(color);
}

} /* namespace NLQT */