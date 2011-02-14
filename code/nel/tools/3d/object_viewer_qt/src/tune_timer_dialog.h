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

#ifndef TUNE_TIMER_DIALOG_H
#define TUNE_TIMER_DIALOG_H

#include "ui_tune_timer_form.h"

// STL includes

// NeL includes

// Project includes

namespace NLQT
{

class CTuneTimerDialog: public QDockWidget
{
	Q_OBJECT

public:
	CTuneTimerDialog(QWidget *parent = 0);
	~CTuneTimerDialog();

	void setInterval(int value);

Q_SIGNALS:
	void changeInterval(int value);

private:

	Ui::CTuneTimerDialog _ui;

}; /* class CTuneTimerDialog */

} /* namespace NLQT */

#endif // TUNE_TIMER_DIALOG_H
