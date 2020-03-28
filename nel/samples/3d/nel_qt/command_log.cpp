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

#include <nel/misc/types_nl.h>
#include "command_log.h"

// STL includes

// Qt includes
#include <QtGui/QVBoxLayout>

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
	switch (args.LogType)
	{
	case CLog::LOG_DEBUG:
		m_DisplayerOutput->setTextColor(Qt::darkGray);
		break;
	case CLog::LOG_STAT:
		m_DisplayerOutput->setTextColor(Qt::darkGreen);
		break;
	case CLog::LOG_NO:
	case CLog::LOG_UNKNOWN:
	case CLog::LOG_INFO:
		m_DisplayerOutput->setTextColor(Qt::black);
		break;
	case CLog::LOG_WARNING:
		m_DisplayerOutput->setTextColor(Qt::darkBlue);
		break;
	case CLog::LOG_ERROR:
	case CLog::LOG_ASSERT:
		m_DisplayerOutput->setTextColor(Qt::darkRed);
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

	m_DisplayerOutput->append(str.substr(0, str.size() - 1).c_str());
}

void CCommandLog::returnPressed()
{
	QString text = m_CommandInput->text();
	if (text.isEmpty())
		return;

	std::string cmd = text.toAscii().data();
	ICommand::execute(cmd, InfoLog());

	m_CommandInput->clear();
}

} /* namespace NLQT */

/* end of file */
