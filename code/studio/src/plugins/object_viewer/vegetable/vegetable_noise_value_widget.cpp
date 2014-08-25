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
#include "vegetable_noise_value_widget.h"

// NeL includes

#define	NL_VEGETABLE_EDIT_SLIDER_NVS_SIZE_THRESHOLD	0.03f
#define	NL_VEGETABLE_EDIT_SLIDER_NVS_SCALE 3.0f

namespace NLQT
{

CVegetNoiseValueWidget::CVegetNoiseValueWidget(QWidget *parent)
	: QGroupBox(parent),
	  _emit(true)
{
	_ui.setupUi(this);

	connect(_ui.verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(setSlider(int)));
	connect(_ui.verticalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(_ui.absWidget, SIGNAL(valueChanged(float)), this, SLOT(setAbsValue(float)));
	connect(_ui.randWidget, SIGNAL(valueChanged(float)), this, SLOT(setRandValue(float)));
	connect(_ui.freqWidget, SIGNAL(valueChanged(float)), this, SLOT(setFreqValue(float)));

	setDefaultRangeAbs(0, 1);
	setDefaultRangeRand(0, 1);
	setDefaultRangeFreq(0, 1);

	setNoiseValue(NLMISC::CNoiseValue(), false);
}

CVegetNoiseValueWidget::~CVegetNoiseValueWidget()
{
}

void CVegetNoiseValueWidget::setNoiseValue(const NLMISC::CNoiseValue &value, bool emit)
{
	_emit = false;
	_ui.absWidget->setValue(value.Abs);
	_ui.randWidget->setValue(value.Rand);
	_ui.freqWidget->setValue(value.Frequency);
	_emit = true;
	if (emit)
		noiseValueChanged(_NoiseValue);
}

void CVegetNoiseValueWidget::setDefaultRangeAbs(float defRangeMin, float defRangeMax)
{
	_ui.absWidget->setRange(defRangeMin, defRangeMax);
}

void CVegetNoiseValueWidget::setDefaultRangeRand(float defRangeMin, float defRangeMax)
{
	_ui.randWidget->setRange(defRangeMin, defRangeMax);
}

void CVegetNoiseValueWidget::setDefaultRangeFreq(float defRangeMin, float defRangeMax)
{
	_ui.freqWidget->setRange(defRangeMin, defRangeMax);
}

void CVegetNoiseValueWidget::setSlider(int value)
{
}

void CVegetNoiseValueWidget::sliderReleased()
{
}

void CVegetNoiseValueWidget::setAbsValue(float value)
{
	_NoiseValue.Abs = value;
	if (_emit)
		noiseValueChanged(_NoiseValue);
}

void CVegetNoiseValueWidget::setRandValue(float value)
{
	_NoiseValue.Rand = value;
	if (_emit)
		noiseValueChanged(_NoiseValue);

}

void CVegetNoiseValueWidget::setFreqValue(float value)
{
	_NoiseValue.Frequency = value;
	if (_emit)
		noiseValueChanged(_NoiseValue);
}

} /* namespace NLQT */