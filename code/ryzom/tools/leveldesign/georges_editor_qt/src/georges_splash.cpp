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

#include "georges_splash.h"

// Qt includes
#include <QtGui/QWidget>
#include <QDebug>
#include <QFileInfo>

// NeL includes

// Project includes

namespace NLMISC {
	class IProgressCallback;
}

namespace NLQT
{

	CGeorgesSplash::CGeorgesSplash(QWidget *parent)
		: QWidget(parent)
	{
		_ui.setupUi(this);

		setWindowFlags(Qt::SplashScreen);
		_ui.imageLabel->setPixmap(
			QPixmap(":/images/georges_logo.png").
			scaledToHeight(_ui.imageLabel->height(),Qt::SmoothTransformation));
		//setWindowIcon(QIcon(":/images/georges_logo.png"));
	}

	CGeorgesSplash::~CGeorgesSplash() 
	{

	}

	void CGeorgesSplash::progress (float progressValue)
	{
		QString display = DisplayString.c_str();

		// UNCOMMENT if shorter strings are needed
		//QFileInfo info(display);
		//display = info.filePath();
		//QString sec = display.section("/",-2,-1,
		//	QString::SectionSkipEmpty | 
		//	QString::SectionIncludeTrailingSep |
		//	QString::SectionIncludeLeadingSep);
		//if(display != sec)
		//	display = sec.prepend("...");

		_ui.splashLabel->setText(QString(tr("Adding Folder:\n%1")).arg(display));
		_ui.progressBar->setValue(getCropedValue(progressValue) * 100);
		QApplication::processEvents();
	}
} /* namespace NLQT */