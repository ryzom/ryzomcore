// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdmisc.h"

#include "nel/misc/socket_displayer.h"

#include "nel/misc/common.h"
#include "nel/misc/ctrl_protocol.h"
#include "nel/misc/debug.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC {

static const char *const s_ProtocolVersion = "2";
static const uint s_DefaultHistory = 1000;
static const uint s_ConnectIntervalMs = 1000;
static const uint s_SendTimeoutMs = 2000;
static const uint s_MaxLineSize = 64 * 1024;

CSocketDisplayer::CSocketDisplayer(const std::string &path, const char *displayerName)
    : CWindowDisplayer(displayerName)
    , m_Path(path)
    , m_TitleBar("CSocketDisplayer::m_TitleBar")
    , m_CommandQueue("CSocketDisplayer::m_CommandQueue")
    , m_ArgCommands("CSocketDisplayer::m_ArgCommands")
    , m_DoneQueue("CSocketDisplayer::m_DoneQueue")
    , m_HistoryMax(s_DefaultHistory)
    , m_Connected(false)
    , m_PathWarned(false)
    , m_LastConnectAttempt(0)
{
	// the dashboard renders the lines, it does not need the carriage returns
	needSlashR = false;
	// the commands registered so far, then every change as it happens
	CCommandRegistry::getInstance().listCommands(*this);
	CCommandRegistry::getInstance().addListener(this);
}

CSocketDisplayer::~CSocketDisplayer()
{
	CCommandRegistry::getInstance().removeListener(this);
	// stop the display thread before our members go away; the base destructor waits again, which is harmless
	_Continue = false;
	if (_Thread != nullptr)
		_Thread->wait();
	disconnect();
}

void CSocketDisplayer::setTitleBar(const std::string &titleBar)
{
	// same text as the Win32 console, so a dashboard ready pattern written for it keeps matching
	std::string wn;
	if (!titleBar.empty())
	{
		wn += titleBar;
		wn += ": ";
	}
	wn += "Nel Service Console (compiled " __DATE__ " " __TIME__ " in " NL_MODE_STR " mode)";

	CSynchronized<std::string>::CAccessor access(&m_TitleBar);
	access.value() = wn;
}

void CSocketDisplayer::open(std::string titleBar, bool /* iconified */, sint /* x */, sint /* y */, sint /* w */, sint /* h */, sint hs, sint /* fs */, const std::string & /* fn */, bool /* ww */, CLog *log)
{
	Log = log;
	m_HistoryMax = (hs > 0) ? (uint)hs : s_DefaultHistory;
	_HistorySize = (sint)m_HistoryMax;
	{
		// the main thread may already have set the real title if this thread started late
		CSynchronized<std::string>::CAccessor access(&m_TitleBar);
		if (!access.value().empty())
			titleBar.clear();
	}
	if (!titleBar.empty())
		setTitleBar(titleBar);
	_Init = true;
}

void CSocketDisplayer::display_main()
{
	nlassert(_Init);

	while (_Continue)
	{
		if (!m_Connected)
		{
			TTime now = CTime::getLocalTime();
			if (now - m_LastConnectAttempt >= (TTime)s_ConnectIntervalMs)
			{
				m_LastConnectAttempt = now;
				if (connectToDashboard())
					replay();
			}
		}

		// the history keeps filling while the dashboard is away, and the command table keeps up
		flushLog();
		flushCommands();
		flushDone();

		if (m_Connected)
		{
			flushTitle();
			flushLabels(false);
			pollIncoming();
		}

		nlSleep(10);
	}

	disconnect();
}

// *** connection ***

bool CSocketDisplayer::connectToDashboard()
{
	std::string error;
	// a path that can never work, or that is not on this machine, is never dialled, and said once; a dashboard
	// that is away is expected
	if (!CLocalPipe::checkPath(m_Path, error))
	{
		if (!m_PathWarned)
			nlwarning("Control socket: %s; the service console is not connected", error.c_str());
		m_PathWarned = true;
		return false;
	}
	// the pipe never blocks this thread on a dashboard that stops reading, and with it the service exit
	if (!m_Pipe.connect(m_Path, error))
		return false;

	m_Connected = true;
	m_InBuffer.clear();
	m_SentTitleBar.clear();
	nlinfo("Connected to the control socket '%s'", m_Path.c_str());
	return true;
}

void CSocketDisplayer::disconnect()
{
	if (!m_Connected)
		return;
	m_Connected = false;
	m_Pipe.close();
	m_InBuffer.clear();
	// the dashboard went away; it is expected to come back and get a replay
	nldebug("Disconnected from the control socket '%s'", m_Path.c_str());
}

bool CSocketDisplayer::sendLine(const std::string &line)
{
	if (!m_Connected)
		return false;

	std::string data = line;
	data += '\n';
	if (!m_Pipe.write(data, s_SendTimeoutMs))
	{
		disconnect();
		return false;
	}
	return true;
}

void CSocketDisplayer::pollIncoming()
{
	if (!m_Connected)
		return;

	if (!m_Pipe.readAvailable(m_InBuffer, s_MaxLineSize * 4))
	{
		disconnect();
		return;
	}

	std::string::size_type pos;
	while ((pos = m_InBuffer.find('\n')) != std::string::npos)
	{
		std::string line = m_InBuffer.substr(0, pos);
		m_InBuffer.erase(0, pos + 1);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		handleLine(line);
	}

	if (m_InBuffer.size() > s_MaxLineSize)
	{
		nlwarning("Control socket '%s' sent an oversized line, dropping the connection", m_Path.c_str());
		disconnect();
	}
}

// *** messages ***

void CSocketDisplayer::handleLine(const std::string &line)
{
	std::string::size_type tab = line.find('\t');
	std::string type = line.substr(0, tab);
	std::string rest = (tab == std::string::npos) ? std::string() : line.substr(tab + 1);

	if (type == "cmd")
	{
		std::string command = unescapeCtrlField(rest);
		if (command.empty())
			return;
		// executed by the main thread in CWindowDisplayer::update
		CSynchronized<std::vector<std::string> >::CAccessor access(&_CommandsToExecute);
		access.value().push_back(command);
	}
	else if (type == "exec")
	{
		// exec <id> <name> [<arg>...]: the fields are the arguments, nothing is parsed
		CArgCommand command;
		std::string::size_type start = 0;
		uint field = 0;
		for (;;)
		{
			std::string::size_type end = rest.find('\t', start);
			std::string value = unescapeCtrlField(rest.substr(start, end == std::string::npos ? std::string::npos : end - start));
			if (field == 0)
				command.Id = value;
			else if (field == 1)
				command.Name = value;
			else
				command.Args.push_back(value);
			++field;
			if (end == std::string::npos)
				break;
			start = end + 1;
		}
		if (command.Name.empty())
		{
			// nothing to run; the caller still gets its answer
			CSynchronized<TDoneQueue>::CAccessor done(&m_DoneQueue);
			done.value().push_back(TDone(command.Id, false));
			return;
		}
		// executed by the main thread in updateExtra
		CSynchronized<TArgCommands>::CAccessor access(&m_ArgCommands);
		access.value().push_back(command);
	}
	else
	{
		nldebug("Control socket '%s' sent an unknown message type '%s'", m_Path.c_str(), type.c_str());
	}
}

void CSocketDisplayer::updateExtra()
{
	std::vector<CArgCommand> commands;
	{
		CSynchronized<TArgCommands>::CAccessor access(&m_ArgCommands);
		commands.swap(access.value());
	}
	for (uint i = 0; i < commands.size(); ++i)
	{
		nlassert(Log != NULL);
		bool ok = ICommand::executeArgs(commands[i].Name, commands[i].Args, *Log);
		// the output of the command is in the log buffer by now; the done marker follows it
		CSynchronized<TDoneQueue>::CAccessor done(&m_DoneQueue);
		done.value().push_back(TDone(commands[i].Id, ok));
	}
}

void CSocketDisplayer::flushDone()
{
	TDoneQueue done;
	{
		CSynchronized<TDoneQueue>::CAccessor access(&m_DoneQueue);
		done.swap(access.value());
	}
	// the log lines of the command went out in flushLog just before; without a dashboard the markers
	// have nobody to reach
	if (!m_Connected)
		return;
	for (uint i = 0; i < done.size(); ++i)
		sendLine(std::string("done\t") + escapeCtrlField(done[i].first) + "\t" + (done[i].second ? "1" : "0"));
}

void CSocketDisplayer::flushLog()
{
	std::list<std::pair<uint32, std::string> > lines;
	{
		CSynchronized<std::list<std::pair<uint32, std::string> > >::CAccessor access(&_Buffer);
		lines.swap(access.value());
	}

	for (std::list<std::pair<uint32, std::string> >::iterator it = lines.begin(); it != lines.end(); ++it)
	{
		m_History.push_back(*it);
		while (m_History.size() > m_HistoryMax)
			m_History.pop_front();
		if (m_Connected)
			sendLine(std::string("log\t") + toString("%06x", it->first & 0xFFFFFF) + "\t" + escapeCtrlField(it->second));
	}
}

void CSocketDisplayer::flushLabels(bool all)
{
	// copied under the lock and sent outside it, so a dashboard that stops reading cannot
	// stall the main thread in setLabel; a label lost to a failed send comes back with the replay
	std::vector<std::pair<uint, std::string> > pending;
	{
		CSynchronized<std::vector<CLabelEntry> >::CAccessor access(&_Labels);
		for (uint i = 0; i < access.value().size(); ++i)
		{
			CLabelEntry &entry = access.value()[i];
			if (!all && !entry.NeedUpdate)
				continue;
			pending.push_back(std::make_pair(i, entry.Value));
			entry.NeedUpdate = false;
		}
	}
	for (uint i = 0; i < pending.size(); ++i)
	{
		if (!sendLine(std::string("label\t") + toString("%u", pending[i].first) + "\t" + escapeCtrlField(pending[i].second)))
			return;
	}
}

void CSocketDisplayer::flushTitle()
{
	std::string title;
	{
		CSynchronized<std::string>::CAccessor access(&m_TitleBar);
		title = access.value();
	}
	if (title == m_SentTitleBar)
		return;
	if (sendLine(std::string("title\t") + escapeCtrlField(title)))
		m_SentTitleBar = title;
}

void CSocketDisplayer::replay()
{
	if (!sendLine(std::string("hello\t") + s_ProtocolVersion))
		return;
	flushTitle();
	flushLabels(true);
	for (std::map<std::string, CCommandEntry>::const_iterator it = m_Commands.begin(); it != m_Commands.end(); ++it)
	{
		if (!sendCommand(it->second))
			return;
	}
	for (std::list<std::pair<uint32, std::string> >::iterator it = m_History.begin(); it != m_History.end(); ++it)
	{
		if (!sendLine(std::string("log\t") + toString("%06x", it->first & 0xFFFFFF) + "\t" + escapeCtrlField(it->second)))
			return;
	}
}

// *** command table ***

void CSocketDisplayer::onCommandAdded(const std::string &name, TCommandEntryKind kind, const std::string &args, const std::string &help)
{
	CCommandEntry entry;
	entry.Name = name;
	entry.Kind = kind;
	entry.Args = args;
	entry.Help = help;
	entry.Removed = false;
	CSynchronized<TCommandQueue>::CAccessor access(&m_CommandQueue);
	access.value().push_back(entry);
}

void CSocketDisplayer::onCommandRemoved(const std::string &name)
{
	CCommandEntry entry;
	entry.Name = name;
	entry.Kind = CommandEntryKind;
	entry.Removed = true;
	CSynchronized<TCommandQueue>::CAccessor access(&m_CommandQueue);
	access.value().push_back(entry);
}

void CSocketDisplayer::flushCommands()
{
	TCommandQueue changes;
	{
		CSynchronized<TCommandQueue>::CAccessor access(&m_CommandQueue);
		changes.swap(access.value());
	}
	for (uint i = 0; i < changes.size(); ++i)
	{
		const CCommandEntry &entry = changes[i];
		if (entry.Removed)
			m_Commands.erase(entry.Name);
		else
			m_Commands[entry.Name] = entry;
		if (m_Connected)
			sendCommand(entry);
	}
}

bool CSocketDisplayer::sendCommand(const CCommandEntry &entry)
{
	if (entry.Removed)
		return sendLine(std::string("uncommand\t") + escapeCtrlField(entry.Name));
	const char *kind = entry.Kind == HandlerEntryKind ? "o" : (entry.Kind == HandlerCommandEntryKind ? "m" : "c");
	return sendLine(std::string("command\t") + escapeCtrlField(entry.Name) + "\t" + kind + "\t" + escapeCtrlField(entry.Args) + "\t" + escapeCtrlField(entry.Help));
}

} // NLMISC

/* End of socket_displayer.cpp */
