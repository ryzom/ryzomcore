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

#ifndef VEGETABLE_ROTATE_PAGE_H
#define VEGETABLE_ROTATE_PAGE_H

#include "ui_vegetable_rotate_form.h"

// STL includes

// NeL includes

// Project includes

namespace NL3D
{
class CVegetable;
}

namespace NLQT
{

class CVegetableRotatePage: public QWidget
{
	Q_OBJECT

public:
	CVegetableRotatePage(QWidget *parent = 0);
	~CVegetableRotatePage();

	/// set the vegetble to edit. NULL will disable all the controls.
	void setVegetableToEdit(NL3D::CVegetable *vegetable);

private Q_SLOTS:
	void setNoiseValueRotateX(const NLMISC::CNoiseValue &value);
	void setNoiseValueRotateY(const NLMISC::CNoiseValue &value);
	void setNoiseValueRotateZ(const NLMISC::CNoiseValue &value);

private:

	// The vegetable to edit.
	NL3D::CVegetable *_Vegetable;

	Ui::CVegetableRotatePage _ui;

}; /* class CVegetableRotatePage */

} /* namespace NLQT */
#endif // VEGETABLE_ROTATE_PAGE_H
