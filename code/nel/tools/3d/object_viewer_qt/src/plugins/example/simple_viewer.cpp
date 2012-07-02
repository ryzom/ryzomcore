// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "simple_viewer.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QMessageBox>

// NeL includes

// Project includes

namespace Plugin
{

SimpleViewer::SimpleViewer(QWidget *parent)
	: QWidget(parent)
{
	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setObjectName(QString::fromUtf8("gridLayoutSimpleViewer"));
	gridLayout->setContentsMargins(0, 0, 0, 0);
	NLQT::QNLWidget *m_nelWidget = new NLQT::QNLWidget(this);
	gridLayout->addWidget(m_nelWidget, 0, 0, 1, 1);

	m_undoStack = new QUndoStack(this);
}

bool ExampleCoreListener::closeMainWindow() const
{
	int ret = QMessageBox::question(0, tr("Example close event hook"),
									tr("Do you want to close window?"),
									QMessageBox::Yes | QMessageBox::No);

	if (ret == QMessageBox::Yes)
		return true;
	else
		return false;
}

} /* namespace Plugin */