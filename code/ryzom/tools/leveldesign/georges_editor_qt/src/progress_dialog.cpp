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

#include "progress_dialog.h"

// Qt includes
#include <QGridLayout>
#include <QLabel>
#include <QApplication>
#include <QProgressBar>
#include <QIcon>
// STL includes

// NeL includes
#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>

// Project includes

namespace NLQT 
{

	CProgressDialog::CProgressDialog(QDialog *parent): QDialog(parent) 
	{
		setWindowTitle(tr("Loading Data"));
		QGridLayout* gridLayout = new QGridLayout(this);
		gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
		_infoLabel   = new QLabel("test",this);
		_progressBar = new QProgressBar(this);
		gridLayout->addWidget(_progressBar, 0, 0);
		gridLayout->addWidget(_infoLabel, 1, 0);
		resize(250, 100);
		_progressBar->setMinimum(0);
		_progressBar->setMaximum(100);
		setWindowIcon(QIcon(":/images/georges_logo.png"));
	}

	CProgressDialog::~CProgressDialog() 
	{
	}

	void CProgressDialog::progress (float progressValue)
	{
		_infoLabel->setText(QString(tr("Adding Folder:\n%1")).arg(DisplayString.c_str()));
		_progressBar->setValue(getCropedValue(progressValue) * 100);
		QApplication::processEvents();
	}

} /* namespace NLQT */

