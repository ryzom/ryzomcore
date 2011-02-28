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

#ifndef VEGETABLE_APPEARANCE_PAGE_H
#define VEGETABLE_APPEARANCE_PAGE_H

#include "ui_vegetable_apperance_form.h"

// STL includes

// NeL includes

// Project includes

namespace NL3D
{
class CVegetable;
}

namespace NLQT
{

class CVegetableApperancePage: public QWidget
{
	Q_OBJECT

public:
	CVegetableApperancePage(QWidget *parent = 0);
	~CVegetableApperancePage();

	/// set the vegetble to edit. NULL will disable all the controls.
	void setVegetableToEdit(NL3D::CVegetable *vegetable);

private Q_SLOTS:
	void setNoiseValueBendPhase(const NLMISC::CNoiseValue &value);
	void setNoiseValueBendFactor(const NLMISC::CNoiseValue &value);
	void setNoiseValueColor(const NLMISC::CNoiseValue &value);
	void browseColor(QListWidgetItem *item);
	void addNewColor();
	void insNewColor();
	void removeColor();
	void getFromListColors();

private:
	void updateColorList();

	// The vegetable to edit.
	NL3D::CVegetable *_Vegetable;

	Ui::CVegetableApperancePage _ui;

}; /* class CVegetableApperancePage */

} /* namespace NLQT */

#endif // VEGETABLE_APPEARANCE_PAGE_H
