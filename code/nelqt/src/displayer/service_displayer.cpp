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
	string wn;
	if (!titleBar.empty())
	{
		wn += titleBar;
		wn += ": ";
	}
	wn += "Nel Service Console (compiled " __DATE__ " " __TIME__ " in " + nlMode + " mode)";

	nldebug("SERVICE: Set title bar to '%s'", wn.c_str());

	NLMISC::CSynchronized<std::string>::CAccessor access(&m_SetTitleBar);
	access.value() = wn;
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
