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
#include <nelqt/displayer/service_displayer.h>

// STL includes

// Qt includes
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/path.h>

// Project includes
#include <nelqt/common.h>
#include <nelqt/displayer/command_log.h>
#include <nelqt/displayer/service_window.h>

using namespace std;
using namespace NLMISC;

namespace NLQT {

CServiceDisplayer::CServiceDisplayer(const char *displayerName) : CWindowDisplayer(displayerName), m_DelayLog("CServiceDisplayer::m_DelayLog"), m_SetTitleBar("CServiceDisplayer::m_SetTitleBar"), m_CommandLog(NULL), m_DoSetTitleBar(false)
{
	createLabel("@Clear|CLEAR");

	INelContext::getInstance().setWindowedApplication(true);
}

CServiceDisplayer::~CServiceDisplayer()
{
	
}

void CServiceDisplayer::doDisplay(const CLog::TDisplayInfo& args, const char *message)
{
	if (!m_Log)
		return;

	if (m_CommandLog)
	{
		m_CommandLog->doDisplay(args, message);
	}
	else
	{
		NLMISC::CSynchronized<std::vector<std::pair<CLog::TDisplayInfo, std::string> > >::CAccessor access(&m_DelayLog);
		access.value().push_back(std::pair<CLog::TDisplayInfo, std::string>(args, message));
	}
}

void CServiceDisplayer::open(int argc, char **argv, std::string titleBar, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww, NLMISC::CLog *log)
{
	if (w == -1)
		w = 700;
	if (h == -1)
		h = 300;
	if (hs == -1)
		hs = 1000;

	m_Log = log;

	NLQT::preApplication();
	QApplication app(argc, argv);
	NLQT::postApplication();

	CServiceWindow serviceWindow;
	// serviceWindow.move(x, y);
	serviceWindow.resize(w, h);
	serviceWindow.setWindowTitle(QString::fromUtf8(titleBar.c_str()));
	m_ServiceWindow = &serviceWindow;
	; {
		NLMISC::CSynchronized<std::vector<std::pair<CLog::TDisplayInfo, std::string> > >::CAccessor access(&m_DelayLog);
		for (std::vector<std::pair<CLog::TDisplayInfo, std::string> >::iterator it(access.value().begin()), end(access.value().end()); it != end; ++it)
			serviceWindow.commandLog()->doDisplay(it->first, it->second.c_str());
		access.value().clear();
	}
	m_CommandLog = serviceWindow.commandLog();
	; {
		NLMISC::CSynchronized<std::vector<std::pair<CLog::TDisplayInfo, std::string> > >::CAccessor access(&m_DelayLog);
		for (std::vector<std::pair<CLog::TDisplayInfo, std::string> >::iterator it(access.value().begin()), end(access.value().end()); it != end; ++it)
			serviceWindow.commandLog()->doDisplay(it->first, it->second.c_str());
		access.value().clear();
	}
	serviceWindow.setButtonCallback(TDisplayerButtonCallback(this, &CServiceDisplayer::buttonCallback));
	serviceWindow.setTimerCallback(TDisplayerTimerCallback(this, &CServiceDisplayer::timerCallback));
	serviceWindow.setExitCallback(TDisplayerExitCallback(this, &CServiceDisplayer::exitCallback));
	m_CommandLog->setExecCommand(TCommandExecute(this, &CServiceDisplayer::commandExecute));
	serviceWindow.show();
	app.exec();
}

void CServiceDisplayer::display_main()
{
	
}

void CServiceDisplayer::commandExecute(const std::string &cmd) // DT
{
	m_Log->displayRawNL("> %s", cmd.c_str());

	{
		CSynchronized<std::vector<std::string> >::CAccessor access(&_CommandsToExecute);
		access.value().push_back(cmd);
	}
}

void CServiceDisplayer::setTitleBar(const std::string &titleBar)
{
	NLMISC::CSynchronized<std::string>::CAccessor access(&m_SetTitleBar);
	access.value() = titleBar;
	m_DoSetTitleBar = true;
}

namespace {
QString rstrip(const QString& str)
{
	int n = str.size() - 1;
	for (; n >= 0; --n)
	{
		if (!str.at(n).isSpace())
		{
			return str.left(n + 1);
		}
	}
	return "";
}
}

void CServiceDisplayer::buttonCallback(QWidget *sender) // DT
{
	CSynchronized<std::vector<CWindowDisplayer::CLabelEntry> >::CAccessor access(&_Labels);
	for (uint i = 0; i < access.value().size(); i++)
	{
		if (access.value()[i].Hwnd == sender)
		{
			if (access.value()[i].Value == "@Clear|CLEAR")
			{
				// special commands because the clear must be called by the display thread and not main thread
				m_CommandLog->clear();
			}
			else
			{
				// the button was found, add the command in the command stack
				CSynchronized<std::vector<std::string> >::CAccessor accessCommands(&_CommandsToExecute);
				string str;
				nlassert(!access.value()[i].Value.empty());
				nlassert(access.value()[i].Value[0] == '@');

				string::size_type pos = access.value()[i].Value.find("|");
				if (pos != string::npos)
				{
					str = access.value()[i].Value.substr(pos + 1);
				}
				else
				{
					str = access.value()[i].Value.substr(1);
				}
				if (!str.empty())
					accessCommands.value().push_back(str);
			}
			break;
		}
	}
}

void CServiceDisplayer::timerCallback() // DT
{
	if (m_DoSetTitleBar)
	{
		NLMISC::CSynchronized<std::string>::CAccessor access(&m_SetTitleBar);
		m_ServiceWindow->setWindowTitle(QString::fromUtf8(access.value().c_str()));
		m_DoSetTitleBar = false;
		access.value().clear();
	}
	; {
		CSynchronized<std::vector<CLabelEntry> >::CAccessor access(&_Labels);
		for (uint i = 0; i < access.value().size(); i++)
		{
			if (access.value()[i].NeedUpdate && !access.value()[i].Value.empty())
			{
				if (access.value()[i].Hwnd == NULL)
				{
					// empty char (on previous) is separator
					if (i > 0 && access.value()[i - 1].Value.empty())
					{
						m_ServiceWindow->addLine();
					}

					// create a button for command and label for variables
					if (access.value()[i].Value[0] == '@')
					{
						access.value()[i].Hwnd = m_ServiceWindow->addButton();
					}
					else
					{
						access.value()[i].Hwnd = m_ServiceWindow->addLabel();
					}
				}

				string n;

				// do this tricks to be sure that windows will clear what is after the number
				if (access.value()[i].Value[0] != '@')
					n = access.value()[i].Value + "                                                 ";
				else
				{
					string::size_type pos = access.value()[i].Value.find('|');
					if (pos != string::npos)
					{
						n = access.value()[i].Value.substr(1, pos - 1);
					}
					else
					{
						n = access.value()[i].Value.substr(1);
					}
				}

				if (access.value()[i].Value[0] == '@')
				{
					((QPushButton *)access.value()[i].Hwnd)->setText(rstrip(QString::fromUtf8(n.c_str())));
				}
				else
				{
					((QLabel *)access.value()[i].Hwnd)->setText(rstrip(QString::fromUtf8(n.c_str())));
				}
			}
		}
	}
}

void CServiceDisplayer::exitCallback() // DT
{
	_Continue = false;
	m_Log = NULL;
	m_CommandLog = NULL;
	m_ServiceWindow = NULL;
}

} /* namespace NLQT */

/* end of file */
