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

#ifndef TUNE_MRM_DIALOG_H
#define TUNE_MRM_DIALOG_H

#include "ui_tune_mrm_form.h"

// STL includes

// NeL includes

// Project includes

namespace NLQT
{

class CTuneMRMDialog: public QDockWidget
{
	Q_OBJECT

public:
	CTuneMRMDialog(QWidget *parent = 0);
	~CTuneMRMDialog();

private Q_SLOTS:
	void setMaxValue(int value);
	void setCurrentValue(int value);

private:

	Ui::CTuneMRMDialog _ui;

}; /* class CTuneMRMDialog */

} /* namespace NLQT */

#endif // TUNE_MRM_DIALOG_H
