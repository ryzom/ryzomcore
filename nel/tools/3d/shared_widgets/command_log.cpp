/*

Copyright (C) 2010-2015  by authors
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
#include "command_log.h"

// STL includes

// Qt includes
#include <QVBoxLayout>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/path.h>
#include <nel/misc/window_displayer.h>

// Project includes

using namespace std;
using namespace NLMISC;

namespace NLQT {

CCommandLog::CCommandLog(QWidget *parent) : QWidget(parent)
{
	m_DisplayerOutput = new QTextEdit();
	m_DisplayerOutput->setReadOnly(true);
	m_DisplayerOutput->setFocusPolicy(Qt::NoFocus);
	m_CommandInput = new QLineEdit();

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_DisplayerOutput);
	layout->addWidget(m_CommandInput);
	setLayout(layout);

	connect(m_CommandInput, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
	connect(this, SIGNAL(tSigDisplay(const QColor &, const QString &)), this, SLOT(tSlotDisplay(const QColor &, const QString &)));

}

CCommandLog::~CCommandLog()
{

}

void CCommandLog::doDisplay(const CLog::TDisplayInfo& args, const char *message)
{
	QColor color;
	switch (args.LogType)
	{
	case CLog::LOG_DEBUG:
		color = Qt::gray;
		break;
	case CLog::LOG_STAT:
		color = Qt::green;
		break;
	case CLog::LOG_NO:
	case CLog::LOG_UNKNOWN:
	case CLog::LOG_INFO:
		color = Qt::white;
		break;
	case CLog::LOG_WARNING:
		color = Qt::yellow;
		break;
	case CLog::LOG_ERROR:
	case CLog::LOG_ASSERT:
		color = Qt::red;
		break;
	default:
		color = Qt::black;
		break;
	}

	std::string str = NLMISC::CWindowDisplayer::stringifyMessage(args, message);

	emit tSigDisplay(color, QString::fromUtf8(str.substr(0, str.size() - 1).c_str()));
}

void CCommandLog::tSlotDisplay(const QColor &c, const QString &text)
{
	m_DisplayerOutput->setTextColor(c);
	m_DisplayerOutput->append(text);
}

void CCommandLog::returnPressed()
{
	QString text = m_CommandInput->text();
	if (text.isEmpty())
		return;

	emit execCommand(text);
	if (m_Func) m_Func(text.toUtf8().constData());

	m_CommandInput->clear();
}

CCommandLogDisplayer::CCommandLogDisplayer(QWidget *parent) : CCommandLog(parent)
{
	connect(this, SIGNAL(execCommand(const std::string &)), this, SLOT(execCommandLog(const std::string &)));
	DebugLog->addDisplayer(this);
	InfoLog->addDisplayer(this);
	WarningLog->addDisplayer(this);
	AssertLog->addDisplayer(this);
	ErrorLog->addDisplayer(this);
	m_Log.addDisplayer(this);
}

CCommandLogDisplayer::~CCommandLogDisplayer()
{
	DebugLog->removeDisplayer(this);
	InfoLog->removeDisplayer(this);
	WarningLog->removeDisplayer(this);
	AssertLog->removeDisplayer(this);
	ErrorLog->removeDisplayer(this);
	m_Log.removeDisplayer(this);
}

void CCommandLogDisplayer::doDisplay(const NLMISC::CLog::TDisplayInfo& args, const char *message)
{
	CCommandLog::doDisplay(args, message);
}

void CCommandLogDisplayer::execCommandLog(const QString &cmd)
{
	std::string str = cmd.toUtf8().constData();

	m_Log.displayRawNL("> %s", str.c_str());
	ICommand::execute(str, m_Log);
}

} /* namespace NLQT */

/* end of file */
