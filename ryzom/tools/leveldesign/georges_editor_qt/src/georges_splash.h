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

#ifndef GEORGES_SPLASH_H
#define GEORGES_SPLASH_H

// Qt includes
#include <QtGui/QWidget>

// STL includes

// NeL includes

// NeL includes
#include <nel/misc/progress_callback.h>

// Project includes
#include "ui_splash.h"

namespace NLMISC {
	class IProgressCallback;
}

namespace NLQT 
{
	class CGeorgesSplash: public QWidget, public NLMISC::IProgressCallback
	{
		Q_OBJECT

		Ui::CGeorgesSplash _ui;

		void progress (float progressValue);

	public:
		CGeorgesSplash(QWidget *parent = 0);
		~CGeorgesSplash();
	}; /* CGeorgesSplash */

} /* namespace NLQT */

#endif // GEORGES_SPLASH_H
