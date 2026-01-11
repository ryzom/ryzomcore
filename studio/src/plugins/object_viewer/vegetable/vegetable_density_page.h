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

#ifndef VEGETABLE_DENSITY_PAGE_H
#define VEGETABLE_DENSITY_PAGE_H

#include "ui_vegetable_density_form.h"

// STL includes

// NeL includes

// Project includes

namespace NL3D
{
class CVegetable;
}

namespace NLQT
{

class CVegetableDensityPage: public QWidget
{
	Q_OBJECT

public:
	CVegetableDensityPage(QWidget *parent = 0);
	~CVegetableDensityPage();

	/// set the vegetble to edit. NULL will disable all the controls.
	void setVegetableToEdit(NL3D::CVegetable *vegetable);

Q_SIGNALS:
	void vegetNameChanged();

private Q_SLOTS:
	void browseShapeVeget();
	void setDistanceOfCreat(int value);
	void setEnabledMaxDensity(bool state);
	void updateAngleMode();
	void setDensity(const NLMISC::CNoiseValue &value);
	void setMaxDensity(float value);
	void setAngleMinSlider(int pos);
	void setAngleMaxSlider(int pos);

private:
	void updateAngleMin();
	void updateAngleMax();

	// The vegetable to edit.
	NL3D::CVegetable *_Vegetable;

	float _PrecMaxDensityValue;

	Ui::CVegetableDensityPage _ui;

}; /* class CVegetableDensityPage */

} /* namespace NLQT */

#endif // VEGETABLE_DENSITY_PAGE_H
