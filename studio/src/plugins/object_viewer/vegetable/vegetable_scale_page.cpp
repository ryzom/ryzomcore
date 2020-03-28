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
#include "vegetable_scale_page.h"

// NeL includes
#include <nel/3d/vegetable.h>

// Projects include
#include "modules.h"

namespace NLQT
{

CVegetableScalePage::CVegetableScalePage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.scaleXYGroupBox->setDefaultRangeAbs(NL_VEGETABLE_SCALE_RANGE_MIN, NL_VEGETABLE_SCALE_RANGE_MAX);
	_ui.scaleXYGroupBox->setDefaultRangeRand(NL_VEGETABLE_SCALE_RANGE_MIN, NL_VEGETABLE_SCALE_RANGE_MAX);
	_ui.scaleXYGroupBox->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);

	_ui.scaleZGroupBox->setDefaultRangeAbs(NL_VEGETABLE_SCALE_RANGE_MIN, NL_VEGETABLE_SCALE_RANGE_MAX);
	_ui.scaleZGroupBox->setDefaultRangeRand(NL_VEGETABLE_SCALE_RANGE_MIN, NL_VEGETABLE_SCALE_RANGE_MAX);
	_ui.scaleZGroupBox->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);

	_ui.freqFactorWidget->setRange(NL_VEGETABLE_BENDFREQ_RANGE_MIN, NL_VEGETABLE_BENDFREQ_RANGE_MAX);

	setEnabled(false);

	connect(_ui.scaleXYGroupBox, SIGNAL(noiseValueChanged(NLMISC::CNoiseValue)), this, SLOT(setNoiseValueXYSize(NLMISC::CNoiseValue)));
	connect(_ui.scaleZGroupBox, SIGNAL(noiseValueChanged(NLMISC::CNoiseValue)), this, SLOT(setNoiseValueXYSize(NLMISC::CNoiseValue)));
	connect(_ui.freqFactorWidget, SIGNAL(valueChanged(float)), this, SLOT(setFreqFactor(float)));
}

CVegetableScalePage::~CVegetableScalePage()
{
}

void CVegetableScalePage::setVegetableToEdit(NL3D::CVegetable *vegetable)
{
	_Vegetable = vegetable;

	if(_Vegetable)
	{
		setEnabled(true);

		_ui.scaleXYGroupBox->setNoiseValue(_Vegetable->Sxy, false);
		_ui.scaleZGroupBox->setNoiseValue(_Vegetable->Sz, false);

		_ui.freqFactorWidget->setValue(_Vegetable->BendFrequencyFactor, false);
	}
	else
		setEnabled(false);
}

void CVegetableScalePage::setNoiseValueXYSize(const NLMISC::CNoiseValue &value)
{
	_Vegetable->Sxy = value;
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableScalePage::setNoiseValueZSize(const NLMISC::CNoiseValue &value)
{
	_Vegetable->Sz = value;
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableScalePage::setFreqFactor(float value)
{
	_Vegetable->BendFrequencyFactor = value;
	Modules::veget().refreshVegetableDisplay();
}

} /* namespace NLQT */