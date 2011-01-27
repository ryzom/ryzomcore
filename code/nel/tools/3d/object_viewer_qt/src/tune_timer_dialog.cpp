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

#include "stdpch.h"
#include "tune_timer_dialog.h"

// Project includes
#include "modules.h"

namespace NLQT
{

CTuneTimerDialog::CTuneTimerDialog(QWidget *parent)
	: QDockWidget(parent)
{
	_ui.setupUi(this);

	connect(_ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SIGNAL(changeInterval(int)));
}

CTuneTimerDialog::~CTuneTimerDialog()
{
}

void CTuneTimerDialog::setInterval(int value)
{
	_ui.horizontalSlider->setValue(value);
}

} /* namespace NLQT */
