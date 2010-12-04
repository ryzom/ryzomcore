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
#include "bin_op_dialog.h"

namespace NLQT
{

CBinOpDialog::CBinOpDialog(QWidget *widget1, QWidget *widget2, QWidget *parent)
	: QDialog(parent)
{
	resize(350, 330);
	_gridLayout = new QGridLayout(this);
	_gridLayout->addWidget(widget1, 0, 0, 1, 2);

	_comboBox = new QComboBox(this);
	_gridLayout->addWidget(_comboBox, 1, 0, 1, 1);

	_horizontalSpacer = new QSpacerItem(267, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	_gridLayout->addItem(_horizontalSpacer, 1, 1, 1, 1);

	_gridLayout->addWidget(widget2, 2, 0, 1, 2);

	setWindowTitle(tr("Bin operator"));
	_comboBox->clear();

	qobject_cast<QGroupBox *>(widget1)->setTitle(tr("Arg1"));
	qobject_cast<QGroupBox *>(widget2)->setTitle(tr("Arg2"));
	connect(_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setNewOp(int)));
}

CBinOpDialog::~CBinOpDialog()
{
}

void CBinOpDialog::setNewOp(int index)
{
	newOp(uint32(index));
}

} /* namespace NLQT */