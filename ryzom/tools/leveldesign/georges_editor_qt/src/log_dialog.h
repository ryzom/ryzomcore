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

#ifndef LOG_DIALOG_H
#define LOG_DIALOG_H

// Qt includes
#include <QtGui/QWidget>

// STL includes

// NeL includes
#include <nel/misc/types_nl.h>

// Project includes
#include "ui_log_form.h"

namespace NLQT 
{
	class CQtDisplayer;

	class CGeorgesLogDialog: public QDockWidget
	{

	public:
		CGeorgesLogDialog(QWidget *parent = 0);
		~CGeorgesLogDialog();

	private:
		Ui::CGeorgesLogDialog _ui;

		CQtDisplayer *_displayer;

		friend class CMainWindow;
	}; /* CGeorgesLogDialog */

} /* namespace NLQT */

#endif // LOG_DIALOG_H
