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
#include <nelqt/displayer/command_log.h>

// STL includes

// Qt includes
#include <QVBoxLayout>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/path.h>

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

	DebugLog->addDisplayer(this);
	InfoLog->addDisplayer(this);
	WarningLog->addDisplayer(this);
	AssertLog->addDisplayer(this);
	ErrorLog->addDisplayer(this);
}

CCommandLog::~CCommandLog()
{
	DebugLog->removeDisplayer(this);
	InfoLog->removeDisplayer(this);
	WarningLog->removeDisplayer(this);
	AssertLog->removeDisplayer(this);
	ErrorLog->removeDisplayer(this);
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

	bool needSpace = false;
	//stringstream ss;
	string str;

	if (args.LogType != CLog::LOG_NO)
	{
		str += logTypeToString(args.LogType);
		needSpace = true;
	}

	// Write thread identifier
	if (args.ThreadId != 0)
	{
		if (needSpace) { str += " "; needSpace = false; }
#ifdef NL_OS_WINDOWS
		str += NLMISC::toString("%4x", args.ThreadId);
#else
		str += NLMISC::toString("%08x", args.ThreadId);
#endif
		needSpace = true;
	}

	if (args.FileName != NULL)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString("%20s", CFile::getFilename(args.FileName).c_str());
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString("%4u", args.Line);
		//ss << setw(4) << args.Line;
		needSpace = true;
	}

	if (args.FuncName != NULL)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString("%20s", args.FuncName);
		needSpace = true;
	}

	if (needSpace) { str += ": "; needSpace = false; }

	uint nbl = 1;

	char *npos, *pos = const_cast<char *>(message);
	while ((npos = strchr (pos, '\n')))
	{
		*npos = '\0';
		str += pos;
		/*if (needSlashR)
			str += "\r";*/
		str += "\n";
		*npos = '\n';
		pos = npos+1;
		nbl++;
	}
	str += pos;

	pos = const_cast<char *>(args.CallstackAndLog.c_str());
	while ((npos = strchr (pos, '\n')))
	{
		*npos = '\0';
		str += pos;
		/*if (needSlashR)
			str += "\r";*/
		str += "\n";
		*npos = '\n';
		pos = npos+1;
		nbl++;
	}
	str += pos;

	tSigDisplay(color, str.substr(0, str.size() - 1).c_str());
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

	std::string cmd = text.toLocal8Bit().data();
	ICommand::execute(cmd, InfoLog());

	m_CommandInput->clear();
}

} /* namespace NLQT */

/* end of file */
