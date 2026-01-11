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
#include "value_from_emitter_dialog.h"

namespace NLQT
{

CValueFromEmitterDialog::CValueFromEmitterDialog(QWidget *widget, QWidget *parent)
	: QDialog(parent)
{
	_gridLayout = new QGridLayout(this);
	_widget = widget;
	_gridLayout->addWidget(_widget, 0, 0, 1, 1);

	setWindowTitle(tr("Value from emitter"));

	//setFixedHeight(sizeHint().height());
}

CValueFromEmitterDialog::~CValueFromEmitterDialog()
{
}

} /* namespace NLQT */