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

#ifndef VEGETABLE_DIALOG_H
#define VEGETABLE_DIALOG_H

#include "ui_vegetable_dialog_form.h"

// STL includes

// NeL includes

// Project includes
#include "vegetable_node.h"

namespace NL3D
{
class CTileVegetableDesc;
}

namespace NLQT
{

class CVegetableDialog: public QDockWidget
{
	Q_OBJECT

public:
	CVegetableDialog(QWidget *parent = 0);
	~CVegetableDialog();

private Q_SLOTS:
	void loadVegetset();
	void appendVegetset();
	void saveVegetset();

	void addVegetList();
	void removeVegetList();
	void insVegetList();
	void clearVegetList();

	void copyVegetable();
	void loadVegetdesc();
	void saveVegetdesc();

	void setVisibleVegetables(bool state);

	void setCurrentItem(int row);

private:
	void updateVegetList();

	Ui::CVegetableDialog _ui;

}; /* class CVegetableDialog */

} /* namespace NLQT */

#endif // VEGETABLE_DIALOG_H
