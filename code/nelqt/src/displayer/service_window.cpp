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

#include <nel/misc/types_nl.h>
#include <nelqt/displayer/service_window.h>

// STL includes

// Qt includes
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QFontDatabase>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/path.h>

// Project includes
#include <nelqt/displayer/command_log.h>

using namespace std;
using namespace NLMISC;

namespace NLQT {

	CServiceWindow::CServiceWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags), m_Font(QFontDatabase::systemFont(QFontDatabase::FixedFont))
{
	m_LabelVBox = new QVBoxLayout(this);
	m_LabelHBox = new QHBoxLayout(this);
	m_LabelVBox->addLayout(m_LabelHBox);
	
	QWidget *widget = new QWidget(this);
	QVBoxLayout *vbox = new QVBoxLayout(widget);
	vbox->addLayout(m_LabelVBox);

	m_CommandLog = new CCommandLog(this);
	m_CommandLog->layout()->setMargin(0);
	vbox->addWidget(m_CommandLog);

	widget->setLayout(vbox);
	setCentralWidget(widget);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerCallback()));
	timer->start(25);
}

QWidget *CServiceWindow::addLabel()
{
	QLabel *label = new QLabel(this);
	label->setFont(m_Font);
	m_LabelHBox->addWidget(label);
	return label;
}

QWidget *CServiceWindow::addButton()
{
	QPushButton *button = new QPushButton(this);
	// button->setFont(m_Font);
	connect(button, SIGNAL(clicked()), this, SLOT(buttonCallback()));
	m_LabelHBox->addWidget(button);
	return button;
}

void CServiceWindow::addLine()
{
	m_LabelHBox = new QHBoxLayout(this);
	m_LabelVBox->addLayout(m_LabelHBox);
}

CServiceWindow::~CServiceWindow()
{
	if (m_ExitCallback)
	{
		m_ExitCallback();
	}
}

void CServiceWindow::timerCallback()
{
	if (m_TimerCallback)
	{
		m_TimerCallback();
	}
}

void CServiceWindow::buttonCallback()
{
	if (m_ButtonCallback)
	{
		QWidget *s = (QWidget *)sender();
		m_ButtonCallback(s);
	}
}

} /* namespace NLQT */

/* end of file */
