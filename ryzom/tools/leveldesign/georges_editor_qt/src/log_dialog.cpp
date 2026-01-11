/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

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

#include "log_dialog.h"

// Qt includes
#include <QtGui/QWidget>
#include <QFile>
#include <QDateTime>
#include <QTextStream>

// NeL includes

// Project includes
#include "qt_displayer.h"

namespace NLQT
{

	CGeorgesLogDialog::CGeorgesLogDialog(QWidget *parent): 
		QDockWidget(parent)
		{

	_ui.setupUi(this);

	_displayer = new CQtDisplayer(_ui.plainTextEdit);
	NLMISC::ErrorLog->addDisplayer(_displayer);
    NLMISC::WarningLog->addDisplayer(_displayer);
    NLMISC::DebugLog->addDisplayer(_displayer);
    NLMISC::AssertLog->addDisplayer(_displayer);
	NLMISC::InfoLog->addDisplayer(_displayer);

}

CGeorgesLogDialog::~CGeorgesLogDialog() 
{
	NLMISC::ErrorLog->removeDisplayer(_displayer);
    NLMISC::WarningLog->removeDisplayer(_displayer);
    NLMISC::DebugLog->removeDisplayer(_displayer);
    NLMISC::AssertLog->removeDisplayer(_displayer);
	NLMISC::InfoLog->removeDisplayer(_displayer);
	delete _displayer;
}

} /* namespace NLQT */