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
#include "value_blender_dialog.h"

namespace NLQT
{

CValueBlenderDialog::CValueBlenderDialog(IValueBlenderDialogClient *createInterface,
		CWorkspaceNode *ownerNode,
		bool destroyInterface,
		QWidget *parent)
	: QDialog(parent),
	  _CreateInterface(createInterface),
	  _Node(ownerNode),
	  _DestroyInterface(destroyInterface)
{
	_gridLayout = new QGridLayout(this);
	_startLabel = new QLabel(this);

	_gridLayout->addWidget(_startLabel, 0, 0, 1, 1);

	_startWidget = _CreateInterface->createDialog(0, _Node, this);
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(_startWidget->sizePolicy().hasHeightForWidth());
	_startWidget->setSizePolicy(sizePolicy);

	_gridLayout->addWidget(_startWidget, 1, 0, 1, 1);

	_endLabel = new QLabel(this);

	_gridLayout->addWidget(_endLabel, 2, 0, 1, 1);

	_endWidget = _CreateInterface->createDialog(1, _Node, this);
	sizePolicy.setHeightForWidth(_endWidget->sizePolicy().hasHeightForWidth());
	_endWidget->setSizePolicy(sizePolicy);

	_gridLayout->addWidget(_endWidget, 3, 0, 1, 1);

	setWindowTitle(tr("Value blender"));
	_startLabel->setText(tr("Start value:"));
	_endLabel->setText(tr("End value:"));

	setFixedHeight(sizeHint().height());
}

CValueBlenderDialog::~CValueBlenderDialog()
{
	if (_DestroyInterface) delete _CreateInterface;
}

} /* namespace NLQT */