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

#include "nel/misc/local_pipe.h"

#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/string_common.h"
#include "nel/misc/time_nl.h"

#ifndef NL_OS_WINDOWS
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#if defined(NL_OS_WINDOWS) && !defined(PIPE_REJECT_REMOTE_CLIENTS)
// declared for Vista and later only; a 32-bit build targets an older version, and the flag is required
#define PIPE_REJECT_REMOTE_CLIENTS 0x00000008
#endif

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC {

#ifndef NL_OS_WINDOWS
namespace {

void setNonBlocking(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	// a process this one starts must not hold the connection: its peer would never see it close
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD, 0) | FD_CLOEXEC);
#ifdef SO_NOSIGPIPE
	// no MSG_NOSIGNAL on this platform; a peer that went away gives an error instead of a signal
	int one = 1;
	setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
}

bool fillAddress(sockaddr_un &addr, const std::string &path, std::string &error)
{
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (path.size() >= sizeof(addr.sun_path))
	{
		error = toString("the socket path '%s' is too long for a Unix domain socket (%u bytes at most)", path.c_str(), (uint)sizeof(addr.sun_path) - 1);
		return false;
	}
	strcpy(addr.sun_path, path.c_str());
	return true;
}

} // anonymous namespace
#endif

// *** CLocalPipe ***

CLocalPipe::CLocalPipe()
#ifdef NL_OS_WINDOWS
    : m_Handle(INVALID_HANDLE_VALUE)
    , m_Event(NULL)
#else
    : m_Socket(-1)
#endif
{
#ifdef NL_OS_WINDOWS
	m_Event = CreateEventW(NULL, TRUE, FALSE, NULL);
#endif
}

CLocalPipe::~CLocalPipe()
{
	close();
#ifdef NL_OS_WINDOWS
	if (m_Event != NULL)
		CloseHandle((HANDLE)m_Event);
#endif
}

bool CLocalPipe::checkPath(const std::string &path, std::string &error)
{
#ifdef NL_OS_WINDOWS
	if (path.compare(0, 9, "\\\\.\\pipe\\") != 0)
	{
		error = toString("'%s' is not a local pipe name (\\\\.\\pipe\\...)", path.c_str());
		return false;
	}
	return true;
#else
	sockaddr_un addr;
	return fillAddress(addr, path, error);
#endif
}

bool CLocalPipe::connect(const std::string &path, std::string &error)
{
	close();
#ifdef NL_OS_WINDOWS
	if (m_Event == NULL)
	{
		error = "no event for overlapped I/O";
		return false;
	}
	// overlapped, so a peer that stops reading cannot block this thread; identification only, so the
	// process behind the name cannot act as this user
	DWORD flags = FILE_FLAG_OVERLAPPED | SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION;
	HANDLE h = CreateFileW(nlUtf8ToWide(path), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, flags, NULL);
	if (h == INVALID_HANDLE_VALUE && GetLastError() == ERROR_PIPE_BUSY)
	{
		// the listener is between two instances
		if (WaitNamedPipeW(nlUtf8ToWide(path), 500))
			h = CreateFileW(nlUtf8ToWide(path), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, flags, NULL);
	}
	if (h == INVALID_HANDLE_VALUE)
	{
		error = formatErrorMessage(getLastError());
		return false;
	}
	m_Handle = h;
#else
	sockaddr_un addr;
	if (!fillAddress(addr, path, error))
		return false;
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
	{
		error = formatErrorMessage(getLastError());
		return false;
	}
	// non-blocking from the start: a listener that does not accept (stopped in a debugger, its backlog full)
	// gets a second, not this thread
	setNonBlocking(fd);
	// EAGAIN: the backlog of the listener is full, and the socket is not connected (Linux never gives EINPROGRESS
	// for a local socket): the connect is tried again for a second. EINPROGRESS (elsewhere): its end is waited for.
	bool connected = false;
	TTime deadline = CTime::getLocalTime() + 1000;
	for (;;)
	{
		// a signal interrupts a connect that goes on (EINTR, then EALREADY or EISCONN when tried again)
		if (::connect(fd, (const sockaddr *)&addr, sizeof(addr)) == 0 || errno == EISCONN)
		{
			connected = true;
			break;
		}
		if (errno == EINTR)
			continue;
		if (errno == EAGAIN && CTime::getLocalTime() < deadline)
		{
			nlSleep(5);
			continue;
		}
		if (errno == EINPROGRESS || errno == EALREADY)
		{
			pollfd pfd;
			pfd.fd = fd;
			pfd.events = POLLOUT;
			pfd.revents = 0;
			int soError = 0;
			socklen_t len = sizeof(soError);
			int polled;
			do
			{
				polled = poll(&pfd, 1, 1000);
			} while (polled < 0 && errno == EINTR);
			connected = polled > 0 && (pfd.revents & POLLOUT) && !(pfd.revents & (POLLHUP | POLLERR))
			    && getsockopt(fd, SOL_SOCKET, SO_ERROR, &soError, &len) == 0 && soError == 0;
			if (!connected)
				error = "the listener does not accept the connection";
			break;
		}
		error = errno == EAGAIN ? std::string("the listener does not accept the connection") : formatErrorMessage(getLastError());
		break;
	}
	if (!connected)
	{
		::close(fd);
		return false;
	}
	m_Socket = fd;
#endif
	return true;
}

void CLocalPipe::close()
{
#ifdef NL_OS_WINDOWS
	if (m_Handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle((HANDLE)m_Handle);
		m_Handle = INVALID_HANDLE_VALUE;
	}
#else
	if (m_Socket >= 0)
	{
		::close(m_Socket);
		m_Socket = -1;
	}
#endif
}

bool CLocalPipe::isOpen() const
{
#ifdef NL_OS_WINDOWS
	return m_Handle != INVALID_HANDLE_VALUE;
#else
	return m_Socket >= 0;
#endif
}

bool CLocalPipe::write(const void *data, size_t size, uint timeoutMs)
{
	if (!isOpen())
		return false;
	const char *ptr = (const char *)data;
	size_t left = size;

#ifdef NL_OS_WINDOWS
	while (left > 0)
	{
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = (HANDLE)m_Event;
		ResetEvent(overlapped.hEvent);
		DWORD chunk = (DWORD)std::min(left, (size_t)(1024 * 1024));
		DWORD written = 0;
		if (!WriteFile((HANDLE)m_Handle, ptr, chunk, &written, &overlapped))
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				close();
				return false;
			}
			if (WaitForSingleObject(overlapped.hEvent, timeoutMs) != WAIT_OBJECT_0)
			{
				// the peer is not reading; only this thread does I/O on the pipe, so CancelIo covers the write
				CancelIo((HANDLE)m_Handle);
				GetOverlappedResult((HANDLE)m_Handle, &overlapped, &written, TRUE);
				close();
				return false;
			}
		}
		// the byte count of an overlapped operation comes from here, also when it completed at once
		if (!GetOverlappedResult((HANDLE)m_Handle, &overlapped, &written, FALSE))
		{
			close();
			return false;
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
				// the peer is slow to read; wait a little before giving up on it
				pollfd pfd;
				pfd.fd = m_Socket;
				pfd.events = POLLOUT;
				pfd.revents = 0;
				if (poll(&pfd, 1, (int)timeoutMs) > 0 && (pfd.revents & POLLOUT))
					continue;
			}
			close();
			return false;
		}
		ptr += written;
		left -= written;
	}
#endif
	return true;
}

bool CLocalPipe::readAvailable(std::string &buffer, size_t maxBytes)
{
	if (!isOpen())
		return false;

	char chunk[16384];
	size_t total = 0;

#ifdef NL_OS_WINDOWS
	while (total < maxBytes)
	{
		DWORD available = 0;
		if (!PeekNamedPipe((HANDLE)m_Handle, NULL, 0, NULL, &available, NULL))
		{
			close();
			return false;
		}
		if (available == 0)
			break;
		// the bytes are there, so this read completes without waiting on the peer
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = (HANDLE)m_Event;
		ResetEvent(overlapped.hEvent);
		DWORD read = 0;
		DWORD want = (DWORD)std::min(std::min((size_t)available, sizeof(chunk)), maxBytes - total);
		if (!ReadFile((HANDLE)m_Handle, chunk, want, &read, &overlapped))
		{
			if (GetLastError() != ERROR_IO_PENDING || !GetOverlappedResult((HANDLE)m_Handle, &overlapped, &read, TRUE))
			{
				close();
				return false;
			}
		}
		else if (!GetOverlappedResult((HANDLE)m_Handle, &overlapped, &read, FALSE))
		{
			close();
			return false;
		}
		if (read == 0)
		{
			close();
			return false;
		}
		buffer.append(chunk, read);
		total += read;
	}
#else
	while (total < maxBytes)
	{
		ssize_t read = recv(m_Socket, chunk, std::min(sizeof(chunk), maxBytes - total), 0);
		if (read == 0)
		{
			close();
			return false;
		}
		if (read < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			if (errno == EINTR)
				continue;
			close();
			return false;
		}
		buffer.append(chunk, read);
		total += read;
	}
#endif
	return true;
}

// *** CLocalPipeListener ***

CLocalPipeListener::CLocalPipeListener()
#ifdef NL_OS_WINDOWS
    : m_Pending(INVALID_HANDLE_VALUE)
    , m_Event(NULL)
    , m_Overlapped(nullptr)
    , m_Connecting(false)
    , m_SecurityDescriptor(nullptr)
    , m_Acl(nullptr)
#else
    : m_Socket(-1)
    , m_Unlink(false)
#endif
{
}

CLocalPipeListener::~CLocalPipeListener()
{
	close();
}

bool CLocalPipeListener::isListening() const
{
#ifdef NL_OS_WINDOWS
	return m_Event != NULL;
#else
	return m_Socket >= 0;
#endif
}

#ifdef NL_OS_WINDOWS

bool CLocalPipeListener::listen(const std::string &path, std::string &error)
{
	close();
	m_Path = path;

	// an access list that holds the current user only; the default one of a pipe lets everyone read
	HANDLE token = NULL;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
	{
		error = toString("cannot read the process token: %s", formatErrorMessage(getLastError()).c_str());
		return false;
	}
	DWORD length = 0;
	GetTokenInformation(token, TokenUser, NULL, 0, &length);
	std::vector<uint8> tokenUser(length > 0 ? length : 1);
	if (length == 0 || !GetTokenInformation(token, TokenUser, &tokenUser[0], length, &length))
	{
		error = toString("cannot read the user of the process: %s", formatErrorMessage(getLastError()).c_str());
		CloseHandle(token);
		return false;
	}
	CloseHandle(token);
	PSID sid = ((TOKEN_USER *)&tokenUser[0])->User.Sid;
	DWORD aclSize = (DWORD)(sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(sid) - sizeof(DWORD));
	m_Acl = malloc(aclSize);
	m_SecurityDescriptor = malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (!InitializeAcl((PACL)m_Acl, aclSize, ACL_REVISION)
	    || !AddAccessAllowedAce((PACL)m_Acl, ACL_REVISION, GENERIC_ALL, sid)
	    || !InitializeSecurityDescriptor((PSECURITY_DESCRIPTOR)m_SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION)
	    || !SetSecurityDescriptorDacl((PSECURITY_DESCRIPTOR)m_SecurityDescriptor, TRUE, (PACL)m_Acl, FALSE))
	{
		error = toString("cannot build the access list of the pipe: %s", formatErrorMessage(getLastError()).c_str());
		close();
		return false;
	}

	m_Event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (m_Event == NULL)
	{
		error = toString("cannot create an event: %s", formatErrorMessage(getLastError()).c_str());
		close();
		return false;
	}
	m_Overlapped = new OVERLAPPED;
	if (!createInstance(true, error))
	{
		close();
		return false;
	}
	return true;
}

bool CLocalPipeListener::createInstance(bool first, std::string &error)
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = m_SecurityDescriptor;
	sa.bInheritHandle = FALSE;
	DWORD openMode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | (first ? FILE_FLAG_FIRST_PIPE_INSTANCE : 0);
	DWORD pipeMode = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS;
	HANDLE h = CreateNamedPipeW(nlUtf8ToWide(m_Path), openMode, pipeMode, PIPE_UNLIMITED_INSTANCES, 64 * 1024, 64 * 1024, 0, &sa);
	if (h == INVALID_HANDLE_VALUE)
	{
		int code = getLastError();
		if (first && code == ERROR_ACCESS_DENIED)
			error = toString("the pipe '%s' already exists, another process holds the name", m_Path.c_str());
		else
			error = toString("cannot create the pipe '%s': %s", m_Path.c_str(), formatErrorMessage(code).c_str());
		return false;
	}
	OVERLAPPED *overlapped = (OVERLAPPED *)m_Overlapped;
	memset(overlapped, 0, sizeof(OVERLAPPED));
	overlapped->hEvent = (HANDLE)m_Event;
	ResetEvent((HANDLE)m_Event);
	m_Connecting = false;
	if (!ConnectNamedPipe(h, overlapped))
	{
		DWORD code = GetLastError();
		if (code == ERROR_IO_PENDING)
		{
			m_Connecting = true;
		}
		else if (code != ERROR_PIPE_CONNECTED)
		{
			// ERROR_PIPE_CONNECTED: a peer came between the create and the connect, it is there already
			error = toString("cannot wait for a connection on the pipe '%s': %s", m_Path.c_str(), formatErrorMessage((int)code).c_str());
			CloseHandle(h);
			return false;
		}
	}
	m_Pending = h;
	return true;
}

CLocalPipe *CLocalPipeListener::accept()
{
	if (m_Event == NULL)
		return nullptr;
	if (m_Pending == INVALID_HANDLE_VALUE)
	{
		std::string error;
		if (!createInstance(false, error))
			return nullptr;
	}
	if (m_Connecting)
	{
		DWORD dummy = 0;
		if (!GetOverlappedResult((HANDLE)m_Pending, (OVERLAPPED *)m_Overlapped, &dummy, FALSE))
		{
			if (GetLastError() == ERROR_IO_INCOMPLETE)
				return nullptr;
			// the peer left before we saw it; start over with a fresh instance
			CloseHandle((HANDLE)m_Pending);
			m_Pending = INVALID_HANDLE_VALUE;
			return nullptr;
		}
	}
	CLocalPipe *pipe = new CLocalPipe();
	pipe->m_Handle = m_Pending;
	m_Pending = INVALID_HANDLE_VALUE;
	std::string error;
	if (!createInstance(false, error))
		nlwarning("%s", error.c_str());
	return pipe;
}

void CLocalPipeListener::close()
{
	if (m_Pending != INVALID_HANDLE_VALUE)
	{
		if (m_Connecting)
		{
			CancelIo((HANDLE)m_Pending);
			DWORD dummy = 0;
			GetOverlappedResult((HANDLE)m_Pending, (OVERLAPPED *)m_Overlapped, &dummy, TRUE);
		}
		CloseHandle((HANDLE)m_Pending);
		m_Pending = INVALID_HANDLE_VALUE;
	}
	m_Connecting = false;
	if (m_Event != NULL)
	{
		CloseHandle((HANDLE)m_Event);
		m_Event = NULL;
	}
	delete (OVERLAPPED *)m_Overlapped;
	m_Overlapped = nullptr;
	free(m_SecurityDescriptor);
	m_SecurityDescriptor = nullptr;
	free(m_Acl);
	m_Acl = nullptr;
}

#else // NL_OS_WINDOWS

bool CLocalPipeListener::listen(const std::string &path, std::string &error)
{
	close();
	m_Path = path;
	sockaddr_un addr;
	if (!fillAddress(addr, path, error))
		return false;

	// a socket left behind by a process that crashed; anything else at that path is not ours to remove
	struct stat st;
	if (lstat(path.c_str(), &st) == 0)
	{
		if (!S_ISSOCK(st.st_mode))
		{
			error = toString("'%s' exists and is not a socket", path.c_str());
			return false;
		}
		::unlink(path.c_str());
	}

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
	{
		error = toString("cannot create a socket: %s", formatErrorMessage(getLastError()).c_str());
		return false;
	}
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD, 0) | FD_CLOEXEC);
	if (bind(fd, (const sockaddr *)&addr, sizeof(addr)) != 0)
	{
		error = toString("cannot bind the socket '%s': %s", path.c_str(), formatErrorMessage(getLastError()).c_str());
		::close(fd);
		return false;
	}
	m_Unlink = true;
	if (chmod(path.c_str(), 0600) != 0 || ::listen(fd, 8) != 0)
	{
		error = toString("cannot listen on the socket '%s': %s", path.c_str(), formatErrorMessage(getLastError()).c_str());
		::close(fd);
		::unlink(path.c_str());
		m_Unlink = false;
		return false;
	}
	setNonBlocking(fd);
	m_Socket = fd;
	return true;
}

CLocalPipe *CLocalPipeListener::accept()
{
	if (m_Socket < 0)
		return nullptr;
	for (;;)
	{
		int fd = ::accept(m_Socket, NULL, NULL);
		if (fd < 0)
			return nullptr;
		// the mode of the socket and its folder already keep other users out; this is the second lock
		bool sameUser = true;
#if defined(SO_PEERCRED)
		struct ucred cred;
		socklen_t len = sizeof(cred);
		if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) != 0 || cred.uid != geteuid())
			sameUser = false;
#elif defined(NL_OS_MAC) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
		uid_t uid;
		gid_t gid;
		if (getpeereid(fd, &uid, &gid) != 0 || uid != geteuid())
			sameUser = false;
#endif
		if (!sameUser)
		{
			nlwarning("Refused a connection on '%s' from another user", m_Path.c_str());
			::close(fd);
			continue;
		}
		setNonBlocking(fd);
		CLocalPipe *pipe = new CLocalPipe();
		pipe->m_Socket = fd;
		return pipe;
	}
}

void CLocalPipeListener::close()
{
	if (m_Socket >= 0)
	{
		::close(m_Socket);
		m_Socket = -1;
	}
	if (m_Unlink)
	{
		::unlink(m_Path.c_str());
		m_Unlink = false;
	}
}

#endif // NL_OS_WINDOWS

} // NLMISC

/* End of local_pipe.cpp */
