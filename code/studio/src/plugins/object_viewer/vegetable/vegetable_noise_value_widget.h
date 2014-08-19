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

#ifndef VEGETABLE_NOISE_VALUE_WIDGET_H
#define VEGETABLE_NOISE_VALUE_WIDGET_H

#include "ui_vegetable_noise_value_form.h"

// STL includes

// NeL includes
#include <nel/misc/noise_value.h>

// Project includes

namespace NLQT
{

class CVegetNoiseValueWidget: public QGroupBox
{
	Q_OBJECT

public:
	CVegetNoiseValueWidget(QWidget *parent = 0);
	~CVegetNoiseValueWidget();

	void setDefaultRangeAbs(float defRangeMin, float defRangeMax);
	void setDefaultRangeRand(float defRangeMin, float defRangeMax);
	void setDefaultRangeFreq(float defRangeMin, float defRangeMax);

Q_SIGNALS:
	void noiseValueChanged(const NLMISC::CNoiseValue &value);

public Q_SLOTS:
	void setNoiseValue(const NLMISC::CNoiseValue &value, bool emit = true);

private Q_SLOTS:
	void setSlider(int value);
	void sliderReleased();
	void setAbsValue(float value);
	void setRandValue(float value);
	void setFreqValue(float value);

private:
	NLMISC::CNoiseValue _NoiseValue;

	bool _emit;

	Ui::CVegetNoiseValueWidget _ui;

}; /* class CVegetNoiseValueWidget */

} /* namespace NLQT */

#endif // VEGETABLE_NOISE_VALUE_WIDGET_H
