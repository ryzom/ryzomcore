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

#ifndef VEGETABLE_LANDSCAPE_PAGE_H
#define VEGETABLE_LANDSCAPE_PAGE_H

#include "ui_vegetable_landscape_form.h"

// STL includes

// NeL includes

// Project includes

namespace NLQT
{

class CVegetableLandscapePage: public QWidget
{
	Q_OBJECT

public:
	CVegetableLandscapePage(QWidget *parent = 0);
	~CVegetableLandscapePage();

Q_SIGNALS:
	void changeVisibleLandscape(bool state);

private Q_SLOTS:
	void setPowerSlider(int value);
	void setBendStartSlider(int value);
	void setFrequencySlider(int value);
	void setVisibleLandscape(bool state);
	void setSnapCameraToGround(bool state);
	void setColorAmbient(NLMISC::CRGBA color);
	void setColorDiffuse(NLMISC::CRGBA color);

private:

	Ui::CVegetableLandscapePage _ui;
}; /* class CVegetableLandscapePage */

} /* namespace NLQT */

#endif // VEGETABLE_LANDSCAPE_PAGE_H
