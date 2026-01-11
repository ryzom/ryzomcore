/*

Copyright (C) 2015  by authors
Author: Jan Boon <jan.boon@kaetemi.be>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef NLQT_COMMON_H
#define NLQT_COMMON_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QPalette>

// NeL includes

// Project includes

namespace NLQT {
namespace {

void preApplication()
{
	QCoreApplication::libraryPaths();
	QString app_location = QCoreApplication::applicationFilePath();
	app_location.truncate(app_location.lastIndexOf(QLatin1Char('/')));
	app_location = QDir(app_location).canonicalPath();
	QCoreApplication::removeLibraryPath(app_location);
	QCoreApplication::addLibraryPath("./platforms");
	QCoreApplication::addLibraryPath("./qtwebengine");
	QCoreApplication::addLibraryPath("./imageformats");
	QCoreApplication::addLibraryPath("./iconengines");
	QCoreApplication::addLibraryPath("./designer");
}

void postApplication()
{
	QApplication::setStyle(QStyleFactory::create("Fusion"));
	QPalette palette = qApp->palette();
	palette.setColor(QPalette::Window, QColor(64, 64, 64));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(48, 48, 48));
	palette.setColor(QPalette::AlternateBase, QColor(64, 64, 64));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::white);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(64, 64, 64));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Highlight, QColor(64, 128, 96));
	palette.setColor(QPalette::HighlightedText, Qt::white);
	qApp->setPalette(palette);
}

}
} /* namespace NLQT */

#endif /* #ifndef NLQT_SERVICE_WINDOW_H */

/* end of file */
