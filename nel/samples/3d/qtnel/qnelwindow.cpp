// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "qnelwindow.h"

#include <QSplitter>

#include "qnelwidget.h"

QNelWindow::QNelWindow (QWidget *parent, Qt::WFlags f) : QMainWindow (parent, f) 
{
	initWindow();
}

void QNelWindow::initWindow()
{

	/// Create ogre widget.

	/// Create our gui stuff
	QSplitter *splitter = new QSplitter (Qt::Vertical);
	//myWidget = new CameraTrackWidget (splitter);
	QNelWidget *nelWidget = new QNelWidget();
	splitter->addWidget((QWidget*)nelWidget);
	//myWidget2 = new CameraTrackWidget (splitter);
	//splitter->addWidget (myWidget2);

	nelWidget->show();
	//myWidget2->show();
	splitter->show();

	setCentralWidget (splitter);
}
