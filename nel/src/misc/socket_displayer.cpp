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
#include "nel/misc/debug.h"
#include "nel/misc/string_common.h"

#ifndef NL_OS_WINDOWS
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC {

static const char *const s_ProtocolVersion = "1";
static const uint s_DefaultHistory = 1000;
static const uint s_ConnectIntervalMs = 1000;
static const uint s_SendTimeoutMs = 2000;
static const uint s_MaxLineSize = 64 * 1024;

CSocketDisplayer::CSocketDisplayer(const std::string &path, const char *displayerName)
    : CWindowDisplayer(displayerName)
    , m_Path(path)
    , m_TitleBar("CSocketDisplayer::m_TitleBar")
    , m_HistoryMax(s_DefaultHistory)
    , m_Connected(false)
    , m_PathWarned(false)
    , m_LastConnectAttempt(0)
#ifdef NL_OS_WINDOWS
    , m_Pipe(INVALID_HANDLE_VALUE)
    , m_Event(NULL)
#else
    , m_Socket(-1)
#endif
{
	// the dashboard renders the lines, it does not need the carriage returns
	needSlashR = false;
#ifdef NL_OS_WINDOWS
	m_Event = CreateEventW(NULL, TRUE, FALSE, NULL);
#endif
}

CSocketDisplayer::~CSocketDisplayer()
{
	// stop the display thread before our members go away; the base destructor waits again, which is harmless
	_Continue = false;
	if (_Thread != nullptr)
		_Thread->wait();
	disconnect();
#ifdef NL_OS_WINDOWS
	if (m_Event != NULL)
		CloseHandle((HANDLE)m_Event);
#endif
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

		// the history keeps filling while the dashboard is away
		flushLog();

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
#ifdef NL_OS_WINDOWS
	if (m_Event == NULL)
		return false;
	// overlapped, so a dashboard that stops reading cannot block this thread, and with it the service exit
	HANDLE h = CreateFileW(nlUtf8ToWide(m_Path), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	m_Pipe = h;
#else
	sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (m_Path.size() >= sizeof(addr.sun_path))
	{
		if (!m_PathWarned)
		{
			m_PathWarned = true;
			nlwarning("Control socket path '%s' is too long for a Unix domain socket", m_Path.c_str());
		}
		return false;
	}
	strcpy(addr.sun_path, m_Path.c_str());

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		return false;
	if (connect(fd, (const sockaddr *)&addr, sizeof(addr)) != 0)
	{
		close(fd);
		return false;
	}
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	m_Socket = fd;
#endif

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
#ifdef NL_OS_WINDOWS
	CloseHandle((HANDLE)m_Pipe);
	m_Pipe = INVALID_HANDLE_VALUE;
#else
	close(m_Socket);
	m_Socket = -1;
#endif
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
	const char *ptr = data.c_str();
	size_t left = data.size();

#ifdef NL_OS_WINDOWS
	while (left > 0)
	{
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = (HANDLE)m_Event;
		ResetEvent(overlapped.hEvent);
		DWORD written = 0;
		if (!WriteFile((HANDLE)m_Pipe, ptr, (DWORD)left, &written, &overlapped))
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				disconnect();
				return false;
			}
			if (WaitForSingleObject(overlapped.hEvent, s_SendTimeoutMs) != WAIT_OBJECT_0)
			{
				// the dashboard is not reading; only this thread does I/O on the pipe, so CancelIo covers the write
				CancelIo((HANDLE)m_Pipe);
				GetOverlappedResult((HANDLE)m_Pipe, &overlapped, &written, TRUE);
				disconnect();
				return false;
			}
			if (!GetOverlappedResult((HANDLE)m_Pipe, &overlapped, &written, FALSE))
			{
				disconnect();
				return false;
			}
		}
		ptr += written;
		left -= written;
	}
#else
	while (left > 0)
	{
		ssize_t written = send(m_Socket, ptr, left, MSG_NOSIGNAL);
		if (written < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// the dashboard is slow to read; wait a little before giving up on it
				pollfd pfd;
				pfd.fd = m_Socket;
				pfd.events = POLLOUT;
				pfd.revents = 0;
				if (poll(&pfd, 1, (int)s_SendTimeoutMs) > 0 && (pfd.revents & POLLOUT))
					continue;
			}
			disconnect();
			return false;
		}
		ptr += written;
		left -= written;
	}
#endif
	return true;
}

void CSocketDisplayer::pollIncoming()
{
	if (!m_Connected)
		return;

	char buffer[4096];

#ifdef NL_OS_WINDOWS
	for (;;)
	{
		DWORD available = 0;
		if (!PeekNamedPipe((HANDLE)m_Pipe, NULL, 0, NULL, &available, NULL))
		{
			disconnect();
			return;
		}
		if (available == 0)
			break;
		// the bytes are there, so this read completes without waiting on the dashboard
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = (HANDLE)m_Event;
		ResetEvent(overlapped.hEvent);
		DWORD read = 0;
		if (!ReadFile((HANDLE)m_Pipe, buffer, (DWORD)std::min((size_t)available, sizeof(buffer)), &read, &overlapped))
		{
			if (GetLastError() != ERROR_IO_PENDING || !GetOverlappedResult((HANDLE)m_Pipe, &overlapped, &read, TRUE))
			{
				disconnect();
				return;
			}
		}
		if (read == 0)
		{
			disconnect();
			return;
		}
		m_InBuffer.append(buffer, read);
	}
#else
	for (;;)
	{
		ssize_t read = recv(m_Socket, buffer, sizeof(buffer), 0);
		if (read == 0)
		{
			disconnect();
			return;
		}
		if (read < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			disconnect();
			return;
		}
		m_InBuffer.append(buffer, read);
	}
#endif

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
		std::string command = unescapeField(rest);
		if (command.empty())
			return;
		// executed by the main thread in CWindowDisplayer::update
		CSynchronized<std::vector<std::string> >::CAccessor access(&_CommandsToExecute);
		access.value().push_back(command);
	}
	else
	{
		nldebug("Control socket '%s' sent an unknown message type '%s'", m_Path.c_str(), type.c_str());
	}
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
			sendLine(std::string("log\t") + toString("%06x", it->first & 0xFFFFFF) + "\t" + escapeField(it->second));
	}
}

void CSocketDisplayer::flushLabels(bool all)
{
	CSynchronized<std::vector<CLabelEntry> >::CAccessor access(&_Labels);
	for (uint i = 0; i < access.value().size(); ++i)
	{
		CLabelEntry &entry = access.value()[i];
		if (!all && !entry.NeedUpdate)
			continue;
		if (!sendLine(std::string("label\t") + toString("%u", i) + "\t" + escapeField(entry.Value)))
			return;
		entry.NeedUpdate = false;
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
	if (sendLine(std::string("title\t") + escapeField(title)))
		m_SentTitleBar = title;
}

void CSocketDisplayer::replay()
{
	if (!sendLine(std::string("hello\t") + s_ProtocolVersion))
		return;
	flushTitle();
	flushLabels(true);
	for (std::list<std::pair<uint32, std::string> >::iterator it = m_History.begin(); it != m_History.end(); ++it)
	{
		if (!sendLine(std::string("log\t") + toString("%06x", it->first & 0xFFFFFF) + "\t" + escapeField(it->second)))
			return;
	}
}

// *** field escaping ***

std::string CSocketDisplayer::escapeField(const std::string &value)
{
	std::string result;
	result.reserve(value.size());
	for (std::string::size_type i = 0; i < value.size(); ++i)
	{
		char c = value[i];
		switch (c)
		{
		case '\\': result += "\\\\"; break;
		case '\t': result += "\\t"; break;
		case '\n': result += "\\n"; break;
		case '\r': result += "\\r"; break;
		default: result += c; break;
		}
	}
	return result;
}

std::string CSocketDisplayer::unescapeField(const std::string &value)
{
	std::string result;
	result.reserve(value.size());
	for (std::string::size_type i = 0; i < value.size(); ++i)
	{
		char c = value[i];
		if (c == '\\' && i + 1 < value.size())
		{
			++i;
			switch (value[i])
			{
			case 't': result += '\t'; break;
			case 'n': result += '\n'; break;
			case 'r': result += '\r'; break;
			default: result += value[i]; break;
			}
		}
		else
		{
			result += c;
		}
	}
	return result;
}

} // NLMISC

/* End of socket_displayer.cpp */
