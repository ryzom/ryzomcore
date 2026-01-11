/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef VEGETABLE_SCALE_PAGE_H
#define VEGETABLE_SCALE_PAGE_H

#include "ui_vegetable_scale_form.h"

// STL includes

// NeL includes

// Project includes

namespace NL3D
{
class CVegetable;
}

namespace NLQT
{

class CVegetableScalePage: public QWidget
{
	Q_OBJECT

public:
	CVegetableScalePage(QWidget *parent = 0);
	~CVegetableScalePage();

	/// set the vegetble to edit. NULL will disable all the controls.
	void setVegetableToEdit(NL3D::CVegetable *vegetable);

private Q_SLOTS:
	void setNoiseValueXYSize(const NLMISC::CNoiseValue &value);
	void setNoiseValueZSize(const NLMISC::CNoiseValue &value);
	void setFreqFactor(float value);

private:

	// The vegetable to edit.
	NL3D::CVegetable *_Vegetable;

	Ui::CVegetableScalePage _ui;

}; /* class CVegetableScalePage */

} /* namespace NLQT */

#endif // VEGETABLE_SCALE_PAGE_H
