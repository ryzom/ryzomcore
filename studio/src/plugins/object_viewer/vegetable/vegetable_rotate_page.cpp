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
#include "vegetable_rotate_page.h"

// NeL includes
#include <nel/3d/vegetable.h>

// Projects include
#include "modules.h"

namespace NLQT
{

CVegetableRotatePage::CVegetableRotatePage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.rotateXGroupBox->setDefaultRangeAbs(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_ui.rotateXGroupBox->setDefaultRangeRand(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_ui.rotateXGroupBox->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);

	_ui.rotateYGroupBox->setDefaultRangeAbs(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_ui.rotateYGroupBox->setDefaultRangeRand(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_ui.rotateYGroupBox->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);

	_ui.rotateZGroupBox->setDefaultRangeAbs(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_ui.rotateZGroupBox->setDefaultRangeRand(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_ui.rotateZGroupBox->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);

	setEnabled(false);

	connect(_ui.rotateXGroupBox, SIGNAL(noiseValueChanged(NLMISC::CNoiseValue)), this, SLOT(setNoiseValueRotateX(NLMISC::CNoiseValue)));
	connect(_ui.rotateYGroupBox, SIGNAL(noiseValueChanged(NLMISC::CNoiseValue)), this, SLOT(setNoiseValueRotateX(NLMISC::CNoiseValue)));
	connect(_ui.rotateZGroupBox, SIGNAL(noiseValueChanged(NLMISC::CNoiseValue)), this, SLOT(setNoiseValueRotateZ(NLMISC::CNoiseValue)));
}

CVegetableRotatePage::~CVegetableRotatePage()
{
}

void CVegetableRotatePage::setVegetableToEdit(NL3D::CVegetable *vegetable)
{
	_Vegetable = vegetable;

	if(_Vegetable)
	{
		setEnabled(true);

		_ui.rotateXGroupBox->setNoiseValue(_Vegetable->Rx, false);
		_ui.rotateYGroupBox->setNoiseValue(_Vegetable->Ry, false);
		_ui.rotateZGroupBox->setNoiseValue(_Vegetable->Rz, false);
	}
	else
		setEnabled(false);
}

void CVegetableRotatePage::setNoiseValueRotateX(const NLMISC::CNoiseValue &value)
{
	_Vegetable->Rx = value;
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableRotatePage::setNoiseValueRotateY(const NLMISC::CNoiseValue &value)
{
	_Vegetable->Ry = value;
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableRotatePage::setNoiseValueRotateZ(const NLMISC::CNoiseValue &value)
{
	_Vegetable->Rz = value;
	Modules::veget().refreshVegetableDisplay();
}

} /* namespace NLQT */