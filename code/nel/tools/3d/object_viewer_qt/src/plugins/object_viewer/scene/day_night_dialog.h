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

#ifndef DAY_NIGHT_DIALOG_H
#define DAY_NIGHT_DIALOG_H

#include "ui_day_night_form.h"

// STL includes

// NeL includes

// Project includes

namespace NLQT
{

/**
@class CDayNightDialog
@brief Dialog set time of day.
@details Dialogue only affects the water shape.
*/
class CDayNightDialog: public QDockWidget
{
	Q_OBJECT

public:
	CDayNightDialog(QWidget *parent = 0);
	~CDayNightDialog();

private Q_SLOTS:
	void setDayNight(int value);

private:

	Ui::CDayNightDialog _ui;
}; /* class CDayNightDialog */

} /* namespace NLQT */

#endif // DAY_NIGHT_DIALOG_H
