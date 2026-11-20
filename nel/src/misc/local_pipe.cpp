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

#include <vector>

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

namespace {

// a write goes to the system in pieces of at most this, the size of the pipe buffer: a slow peer gets the next piece
// once it took one, and each piece it takes counts as progress (an overlapped write is over only once all of it is)
const size_t s_WriteChunk = 64 * 1024;
// a write that waits looks at its time at least this often
const uint s_WaitSliceMs = 100;

#ifdef NL_OS_WINDOWS

typedef BOOL(WINAPI *TCancelIoEx)(HANDLE file, LPOVERLAPPED overlapped);
typedef BOOL(WINAPI *TGetNamedPipeProcessId)(HANDLE pipe, PULONG processId);
typedef BOOL(WINAPI *TAddMandatoryAce)(PACL acl, DWORD aceRevision, DWORD aceFlags, DWORD mandatoryPolicy, PSID labelSid);

/// A function of a system library that Windows has from Vista on, which older headers do not declare
FARPROC getSystemFunction(const wchar_t *module, const char *name)
{
	HMODULE handle = GetModuleHandleW(module);
	return handle != NULL ? GetProcAddress(handle, name) : NULL;
}

/// Cancels an overlapped operation, whichever thread started it. Before Vista only the thread that started it
/// can, and does: a pipe is used by one thread.
void cancelIo(HANDLE handle, OVERLAPPED *overlapped)
{
	static TCancelIoEx s_CancelIoEx = (TCancelIoEx)getSystemFunction(L"kernel32.dll", "CancelIoEx");
	if (s_CancelIoEx != NULL)
		s_CancelIoEx(handle, overlapped);
	else
		CancelIo(handle);
}

/// The label SID of the integrity level of the process when it is above medium (an elevated process), empty
/// otherwise. A pipe without a label has medium, which every process of the user can open.
bool getRaisedIntegrityLabel(HANDLE token, std::vector<uint8> &label, std::string &error)
{
	label.clear();
	// TokenIntegrityLevel gives a TOKEN_MANDATORY_LABEL, which is a SID_AND_ATTRIBUTES; named here for older headers
	const TOKEN_INFORMATION_CLASS tokenIntegrityLevel = (TOKEN_INFORMATION_CLASS)25;
	const DWORD mandatoryHighRid = 0x3000; // SECURITY_MANDATORY_HIGH_RID
	DWORD length = 0;
	GetTokenInformation(token, tokenIntegrityLevel, NULL, 0, &length);
	std::vector<uint8> buffer(length > 0 ? length : 1);
	if (length == 0 || !GetTokenInformation(token, tokenIntegrityLevel, &buffer[0], length, &length))
	{
		error = toString("cannot read the integrity level of the process: %s", formatErrorMessage(getLastError()).c_str());
		return false;
	}
	PSID sid = ((SID_AND_ATTRIBUTES *)&buffer[0])->Sid;
	UCHAR count = *GetSidSubAuthorityCount(sid);
	if (count == 0)
	{
		error = "the integrity level of the process has no level in it";
		return false;
	}
	if (*GetSidSubAuthority(sid, (DWORD)count - 1) < mandatoryHighRid)
		return true;
	const uint8 *bytes = (const uint8 *)sid;
	label.assign(bytes, bytes + GetLengthSid(sid));
	return true;
}

#else

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

/// Whether nobody listens on a socket file, so a process that ended left it; false with the error otherwise
bool isStaleSocket(const sockaddr_un &addr, const std::string &path, std::string &error)
{
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
	{
		error = toString("cannot create a socket: %s", formatErrorMessage(getLastError()).c_str());
		return false;
	}
	// a connect to a local socket ends at once: refused when nobody listens, else connected, or EAGAIN for a full backlog
	setNonBlocking(fd);
	int code = ::connect(fd, (const sockaddr *)&addr, sizeof(addr)) == 0 ? 0 : errno;
	::close(fd);
	if (code == ECONNREFUSED)
		return true;
	if (code == 0 || code == EAGAIN || code == EINPROGRESS || code == EALREADY || code == EISCONN)
		error = toString("the socket '%s' is in use: a process listens on it", path.c_str());
	else
		error = toString("cannot tell whether a process listens on the socket '%s': %s", path.c_str(), formatErrorMessage(code).c_str());
	return false;
}

#endif

} // anonymous namespace

// *** CLocalPipe ***

CLocalPipe::CLocalPipe()
    : m_OutOffset(0)
    , m_Queued(0)
    , m_LastProgress(0)
#ifdef NL_OS_WINDOWS
    , m_Handle(INVALID_HANDLE_VALUE)
    , m_Event(NULL)
    , m_WriteEvent(NULL)
    , m_WriteOverlapped(nullptr)
    , m_WritePending(false)
    , m_Server(false)
#else
    , m_Socket(-1)
#endif
{
#ifdef NL_OS_WINDOWS
	m_Event = CreateEventW(NULL, TRUE, FALSE, NULL);
	m_WriteEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	OVERLAPPED *overlapped = new OVERLAPPED;
	memset(overlapped, 0, sizeof(OVERLAPPED));
	m_WriteOverlapped = overlapped;
#endif
}

CLocalPipe::~CLocalPipe()
{
	close();
#ifdef NL_OS_WINDOWS
	if (m_Event != NULL)
		CloseHandle((HANDLE)m_Event);
	if (m_WriteEvent != NULL)
		CloseHandle((HANDLE)m_WriteEvent);
	delete (OVERLAPPED *)m_WriteOverlapped;
#endif
}

bool CLocalPipe::checkPath(const std::string &path, std::string &error)
{
#ifdef NL_OS_WINDOWS
	// the system reads . and .. in such a path as steps through its namespace, and / as \: \\.\pipe\..\UNC\<host>\...
	// would open a pipe of another machine
	bool ok = path.size() > 9 && startsWithCaseInsensitiveAscii(path, "\\\\.\\pipe\\") && path.find('/') == std::string::npos;
	for (size_t start = 9; ok && start <= path.size();)
	{
		size_t end = path.find('\\', start);
		if (end == std::string::npos)
			end = path.size();
		// an empty part, or one of dots and spaces only
		ok = path.find_first_not_of(". ", start) < end;
		start = end + 1;
	}
	if (!ok)
	{
		error = toString("'%s' is not the name of a pipe on this machine: \\\\.\\pipe\\<name>, without '/' and without a part of dots only", path.c_str());
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
	if (!checkPath(path, error))
		return false;
#ifdef NL_OS_WINDOWS
	if (m_Event == NULL || m_WriteEvent == NULL)
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
	m_Server = false;
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
			connected = polled > 0 && (pfd.revents & POLLOUT) && !(pfd.revents & (POLLHUP | POLLERR)) && getsockopt(fd, SOL_SOCKET, SO_ERROR, &soError, &len) == 0
			    && soError == 0;
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
		if (m_WritePending)
		{
			// the system reads the queue until the write is cancelled and over
			cancelIo((HANDLE)m_Handle, (OVERLAPPED *)m_WriteOverlapped);
			DWORD written = 0;
			GetOverlappedResult((HANDLE)m_Handle, (OVERLAPPED *)m_WriteOverlapped, &written, TRUE);
			m_WritePending = false;
		}
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
	m_Out.clear();
	m_OutOffset = 0;
	m_Queued = 0;
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
	if (size > 0)
	{
		std::string bytes((const char *)data, size);
		queueSwap(bytes);
	}
	TTime deadline = CTime::getLocalTime() + timeoutMs;
	for (;;)
	{
		if (!flush())
			return false;
		if (m_Queued == 0)
			return true;
		TTime now = CTime::getLocalTime();
		if (now >= deadline)
		{
			// the peer does not read, or not fast enough for the time given
			close();
			return false;
		}
		waitWritable((uint)std::min(deadline - now, (TTime)s_WaitSliceMs));
	}
}

void CLocalPipe::queue(const std::string &data)
{
	if (data.empty() || !isOpen())
		return;
	if (m_Queued == 0)
		m_LastProgress = CTime::getLocalTime();
	m_Out.push_back(data);
	m_Queued += data.size();
}

void CLocalPipe::queueSwap(std::string &data)
{
	if (!data.empty() && isOpen())
	{
		if (m_Queued == 0)
			m_LastProgress = CTime::getLocalTime();
		m_Queued += data.size();
		// a string added at the end leaves the first one where it is, with the write in flight from it
		m_Out.push_back(std::string());
		m_Out.back().swap(data);
	}
	data.clear();
}

TTime CLocalPipe::getStalledMs() const
{
	return m_Queued == 0 ? 0 : CTime::getLocalTime() - m_LastProgress;
}

void CLocalPipe::consumed(size_t bytes)
{
	if (bytes == 0)
		return;
	m_Queued -= bytes;
	m_OutOffset += bytes;
	m_LastProgress = CTime::getLocalTime();
	if (m_OutOffset >= m_Out.front().size())
	{
		m_Out.pop_front();
		m_OutOffset = 0;
	}
}

bool CLocalPipe::flush()
{
	if (!isOpen())
		return false;
#ifdef NL_OS_WINDOWS
	HANDLE handle = (HANDLE)m_Handle;
	OVERLAPPED *overlapped = (OVERLAPPED *)m_WriteOverlapped;
	for (;;)
	{
		DWORD written = 0;
		if (m_WritePending)
		{
			if (!GetOverlappedResult(handle, overlapped, &written, FALSE))
			{
				if (GetLastError() == ERROR_IO_INCOMPLETE)
					return true;
				m_WritePending = false;
				close();
				return false;
			}
			m_WritePending = false;
			consumed(written);
			continue;
		}
		if (m_Out.empty())
			return true;
		const std::string &front = m_Out.front();
		DWORD chunk = (DWORD)std::min(front.size() - m_OutOffset, s_WriteChunk);
		memset(overlapped, 0, sizeof(OVERLAPPED));
		overlapped->hEvent = (HANDLE)m_WriteEvent;
		ResetEvent(overlapped->hEvent);
		if (!WriteFile(handle, front.data() + m_OutOffset, chunk, NULL, overlapped))
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				close();
				return false;
			}
			m_WritePending = true;
			continue;
		}
		// done at once; the byte count of an overlapped operation comes from its result
		if (!GetOverlappedResult(handle, overlapped, &written, FALSE))
		{
			close();
			return false;
		}
		// nothing taken: the next flush tries again
		if (written == 0)
			return true;
		consumed(written);
	}
#else
	while (!m_Out.empty())
	{
		const std::string &front = m_Out.front();
		ssize_t written = send(m_Socket, front.data() + m_OutOffset, front.size() - m_OutOffset, MSG_NOSIGNAL);
		if (written < 0)
		{
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return true;
			close();
			return false;
		}
		consumed((size_t)written);
	}
	return true;
#endif
}

void CLocalPipe::waitWritable(uint timeoutMs)
{
#ifdef NL_OS_WINDOWS
	if (m_WritePending)
		WaitForSingleObject((HANDLE)m_WriteEvent, timeoutMs);
	else
		nlSleep(std::min(timeoutMs, 1u));
#else
	pollfd pfd;
	pfd.fd = m_Socket;
	pfd.events = POLLOUT;
	pfd.revents = 0;
	// a signal ends the wait early; the caller looks at the time and waits again
	poll(&pfd, 1, (int)timeoutMs);
#endif
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

bool CLocalPipe::getPeerProcessId(uint32 &pid, std::string &error) const
{
	pid = 0;
	if (!isOpen())
	{
		error = "the pipe is not open";
		return false;
	}
#ifdef NL_OS_WINDOWS
	static TGetNamedPipeProcessId s_GetServer = (TGetNamedPipeProcessId)getSystemFunction(L"kernel32.dll", "GetNamedPipeServerProcessId");
	static TGetNamedPipeProcessId s_GetClient = (TGetNamedPipeProcessId)getSystemFunction(L"kernel32.dll", "GetNamedPipeClientProcessId");
	TGetNamedPipeProcessId get = m_Server ? s_GetClient : s_GetServer;
	if (get == NULL)
	{
		error = "this version of Windows does not tell which process is at the other end of a pipe (Vista and later do)";
		return false;
	}
	ULONG id = 0;
	if (!get((HANDLE)m_Handle, &id))
	{
		error = toString("cannot read which process is at the other end of the pipe: %s", formatErrorMessage(getLastError()).c_str());
		return false;
	}
	pid = (uint32)id;
	return true;
#elif defined(SO_PEERCRED)
	struct ucred cred;
	socklen_t len = sizeof(cred);
	if (getsockopt(m_Socket, SOL_SOCKET, SO_PEERCRED, &cred, &len) != 0)
	{
		error = toString("cannot read which process is at the other end of the socket: %s", formatErrorMessage(getLastError()).c_str());
		return false;
	}
	pid = (uint32)cred.pid;
	return true;
#elif defined(LOCAL_PEERPID)
	pid_t id = 0;
	socklen_t len = sizeof(id);
	if (getsockopt(m_Socket, SOL_LOCAL, LOCAL_PEERPID, &id, &len) != 0)
	{
		error = toString("cannot read which process is at the other end of the socket: %s", formatErrorMessage(getLastError()).c_str());
		return false;
	}
	pid = (uint32)id;
	return true;
#else
	error = "this platform does not tell which process is at the other end of a socket";
	return false;
#endif
}

// *** CLocalPipeListener ***

CLocalPipeListener::CLocalPipeListener()
#ifdef NL_OS_WINDOWS
    : m_Pending(INVALID_HANDLE_VALUE)
    , m_Event(NULL)
    , m_Overlapped(nullptr)
    , m_Connecting(false)
    , m_Connected(false)
    , m_Warned(false)
    , m_SecurityDescriptor(nullptr)
    , m_Dacl(nullptr)
    , m_Sacl(nullptr)
#else
    : m_Socket(-1)
    , m_Unlink(false)
    , m_Device(0)
    , m_Inode(0)
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
	if (!CLocalPipe::checkPath(path, error))
		return false;
	m_Path = path;

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
	std::vector<uint8> label;
	bool labelRead = getRaisedIntegrityLabel(token, label, error);
	CloseHandle(token);
	if (!labelRead)
		return false;

	// an access list that holds the current user only; the default one of a pipe lets everyone read
	PSID sid = ((TOKEN_USER *)&tokenUser[0])->User.Sid;
	DWORD daclSize = (DWORD)(sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(sid) - sizeof(DWORD));
	m_Dacl = malloc(daclSize);
	m_SecurityDescriptor = malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (m_Dacl == nullptr || m_SecurityDescriptor == nullptr || !InitializeAcl((PACL)m_Dacl, daclSize, ACL_REVISION)
	    || !AddAccessAllowedAce((PACL)m_Dacl, ACL_REVISION, GENERIC_ALL, sid)
	    || !InitializeSecurityDescriptor((PSECURITY_DESCRIPTOR)m_SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION)
	    || !SetSecurityDescriptorDacl((PSECURITY_DESCRIPTOR)m_SecurityDescriptor, TRUE, (PACL)m_Dacl, FALSE))
	{
		error = toString("cannot build the access list of the pipe: %s", formatErrorMessage(getLastError()).c_str());
		close();
		return false;
	}
	if (!label.empty())
	{
		// the level of this elevated process, no write up and no read up: a process below it can neither send nor read
		static TAddMandatoryAce s_AddMandatoryAce = (TAddMandatoryAce)getSystemFunction(L"advapi32.dll", "AddMandatoryAce");
		const DWORD noWriteUp = 0x1, noReadUp = 0x2; // SYSTEM_MANDATORY_LABEL_NO_WRITE_UP, SYSTEM_MANDATORY_LABEL_NO_READ_UP
		// a mandatory label ACE is laid out as an access allowed ACE
		DWORD saclSize = (DWORD)(sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + label.size() - sizeof(DWORD));
		m_Sacl = malloc(saclSize);
		if (s_AddMandatoryAce == NULL || m_Sacl == nullptr || !InitializeAcl((PACL)m_Sacl, saclSize, ACL_REVISION)
		    || !s_AddMandatoryAce((PACL)m_Sacl, ACL_REVISION, 0, noWriteUp | noReadUp, (PSID)&label[0])
		    || !SetSecurityDescriptorSacl((PSECURITY_DESCRIPTOR)m_SecurityDescriptor, TRUE, (PACL)m_Sacl, FALSE))
		{
			error = toString("cannot give the pipe the integrity level of this elevated process: %s",
			    s_AddMandatoryAce == NULL ? "the system has no AddMandatoryAce" : formatErrorMessage(getLastError()).c_str());
			close();
			return false;
		}
	}

	m_Event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (m_Event == NULL)
	{
		error = toString("cannot create an event: %s", formatErrorMessage(getLastError()).c_str());
		close();
		return false;
	}
	m_Overlapped = new OVERLAPPED;
	m_Pending = createPipe(true, error);
	if (m_Pending == INVALID_HANDLE_VALUE || !startConnect(error))
	{
		close();
		return false;
	}
	return true;
}

void *CLocalPipeListener::createPipe(bool first, std::string &error)
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
	}
	return h;
}

bool CLocalPipeListener::startConnect(std::string &error)
{
	OVERLAPPED *overlapped = (OVERLAPPED *)m_Overlapped;
	m_Connecting = false;
	m_Connected = false;
	for (uint attempt = 0;; ++attempt)
	{
		memset(overlapped, 0, sizeof(OVERLAPPED));
		overlapped->hEvent = (HANDLE)m_Event;
		ResetEvent((HANDLE)m_Event);
		if (ConnectNamedPipe((HANDLE)m_Pending, overlapped))
		{
			m_Connected = true;
			return true;
		}
		DWORD code = GetLastError();
		if (code == ERROR_IO_PENDING)
		{
			m_Connecting = true;
			return true;
		}
		// a peer came between the create, or the disconnect, and the connect: it is there already
		if (code == ERROR_PIPE_CONNECTED)
		{
			m_Connected = true;
			return true;
		}
		// a peer came and went before the connect: the instance is freed for the next one
		if (code == ERROR_NO_DATA && attempt < 4)
		{
			DisconnectNamedPipe((HANDLE)m_Pending);
			continue;
		}
		error = toString("cannot wait for a connection on the pipe '%s': %s", m_Path.c_str(), formatErrorMessage((int)code).c_str());
		return false;
	}
}

void CLocalPipeListener::warnOnce(const std::string &error)
{
	if (m_Warned)
		return;
	m_Warned = true;
	nlwarning("%s", error.c_str());
}

CLocalPipe *CLocalPipeListener::accept()
{
	if (m_Event == NULL || m_Pending == INVALID_HANDLE_VALUE)
		return nullptr;
	std::string error;
	if (m_Connecting)
	{
		DWORD dummy = 0;
		if (!GetOverlappedResult((HANDLE)m_Pending, (OVERLAPPED *)m_Overlapped, &dummy, FALSE))
		{
			if (GetLastError() == ERROR_IO_INCOMPLETE)
				return nullptr;
			// the peer left before it was seen: the same instance waits for the next one, the name is never let go
			m_Connecting = false;
		}
		else
		{
			m_Connecting = false;
			m_Connected = true;
		}
	}
	if (!m_Connected)
	{
		DisconnectNamedPipe((HANDLE)m_Pending);
		if (!startConnect(error))
		{
			warnOnce(error);
			return nullptr;
		}
		if (!m_Connected)
			return nullptr;
	}
	// the peer gets this instance once another one waits for the next peer: the listener always holds one, so no
	// other process can take the name in between
	HANDLE next = (HANDLE)createPipe(false, error);
	if (next == INVALID_HANDLE_VALUE)
	{
		warnOnce(toString("%s; the peer that connected waits until another instance can be made", error.c_str()));
		return nullptr;
	}
	m_Warned = false;
	CLocalPipe *pipe = new CLocalPipe();
	pipe->m_Handle = m_Pending;
	pipe->m_Server = true;
	m_Pending = next;
	if (!startConnect(error))
		warnOnce(error);
	return pipe;
}

void CLocalPipeListener::close()
{
	if (m_Pending != INVALID_HANDLE_VALUE)
	{
		if (m_Connecting)
		{
			// the connect may have been started by another thread than this one
			cancelIo((HANDLE)m_Pending, (OVERLAPPED *)m_Overlapped);
			DWORD dummy = 0;
			GetOverlappedResult((HANDLE)m_Pending, (OVERLAPPED *)m_Overlapped, &dummy, TRUE);
		}
		CloseHandle((HANDLE)m_Pending);
		m_Pending = INVALID_HANDLE_VALUE;
	}
	m_Connecting = false;
	m_Connected = false;
	m_Warned = false;
	if (m_Event != NULL)
	{
		CloseHandle((HANDLE)m_Event);
		m_Event = NULL;
	}
	delete (OVERLAPPED *)m_Overlapped;
	m_Overlapped = nullptr;
	free(m_SecurityDescriptor);
	m_SecurityDescriptor = nullptr;
	free(m_Dacl);
	m_Dacl = nullptr;
	free(m_Sacl);
	m_Sacl = nullptr;
}

#else // NL_OS_WINDOWS

bool CLocalPipeListener::listen(const std::string &path, std::string &error)
{
	close();
	sockaddr_un addr;
	if (!fillAddress(addr, path, error))
		return false;
	m_Path = path;

	// a socket that a process listens on is that process's; one that refuses connections was left by a process that
	// ended. Anything else at that path is not ours to remove.
	struct stat st;
	if (lstat(path.c_str(), &st) == 0)
	{
		if (!S_ISSOCK(st.st_mode))
		{
			error = toString("'%s' exists and is not a socket", path.c_str());
			return false;
		}
		if (!isStaleSocket(addr, path, error))
			return false;
		if (::unlink(path.c_str()) != 0)
		{
			error = toString("cannot remove the socket '%s' that a process that ended left: %s", path.c_str(), formatErrorMessage(getLastError()).c_str());
			return false;
		}
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
	m_Socket = fd;
	if (lstat(path.c_str(), &st) != 0)
	{
		error = toString("cannot read the socket '%s' just made: %s", path.c_str(), formatErrorMessage(getLastError()).c_str());
		close();
		return false;
	}
	m_Unlink = true;
	m_Device = (uint64)st.st_dev;
	m_Inode = (uint64)st.st_ino;
	if (chmod(path.c_str(), 0600) != 0 || ::listen(fd, 8) != 0)
	{
		error = toString("cannot listen on the socket '%s': %s", path.c_str(), formatErrorMessage(getLastError()).c_str());
		close();
		return false;
	}
	setNonBlocking(fd);
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
		// the mode of the socket and its folder already keep other users out; this is the second lock, and a peer
		// whose user cannot be read does not pass it
		const char *refused = nullptr;
#if defined(SO_PEERCRED)
		struct ucred cred;
		socklen_t len = sizeof(cred);
		if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) != 0)
			refused = "its user cannot be read";
		else if (cred.uid != geteuid())
			refused = "it is another user";
#elif defined(NL_OS_MAC) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
		uid_t uid;
		gid_t gid;
		if (getpeereid(fd, &uid, &gid) != 0)
			refused = "its user cannot be read";
		else if (uid != geteuid())
			refused = "it is another user";
#else
		refused = "this platform does not tell the user of a peer";
#endif
		if (refused != nullptr)
		{
			nlwarning("Refused a connection on '%s': %s", m_Path.c_str(), refused);
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
		// the file this listener made only: another one put at the path since is not ours to remove
		struct stat st;
		if (lstat(m_Path.c_str(), &st) == 0 && (uint64)st.st_dev == m_Device && (uint64)st.st_ino == m_Inode)
			::unlink(m_Path.c_str());
		m_Unlink = false;
	}
}

#endif // NL_OS_WINDOWS

} // NLMISC

/* End of local_pipe.cpp */
