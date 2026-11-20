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

#ifndef NL_LOCAL_PIPE_H
#define NL_LOCAL_PIPE_H

#include "types_nl.h"

#include <string>

namespace NLMISC {

/**
 * A byte stream to another process of the same user on this machine: a named pipe on Windows,
 * a Unix domain socket elsewhere. Nothing here waits on the other side for longer than asked:
 * a peer that stops reading cannot block the caller's thread. One thread uses a pipe at a time.
 */
class CLocalPipe
{
public:
	CLocalPipe();
	~CLocalPipe();

	/// Whether a path can name a pipe at all (a Unix domain socket path has a length limit)
	static bool checkPath(const std::string &path, std::string &error);

	/// Connects to a listening pipe; false when nobody listens there (yet), with the reason in error
	bool connect(const std::string &path, std::string &error);
	void close();
	bool isOpen() const;

	/// Writes every byte, giving a peer that is slow to read at most timeoutMs; the pipe is closed on failure
	bool write(const void *data, size_t size, uint timeoutMs);
	bool write(const std::string &data, uint timeoutMs) { return write(data.data(), data.size(), timeoutMs); }

	/// Appends what has arrived to the buffer without waiting, at most maxBytes; false when the pipe closed
	bool readAvailable(std::string &buffer, size_t maxBytes);

private:
	friend class CLocalPipeListener;

	CLocalPipe(const CLocalPipe &);
	CLocalPipe &operator=(const CLocalPipe &);

#ifdef NL_OS_WINDOWS
	void *m_Handle;
	void *m_Event;
#else
	int m_Socket;
#endif
};

/**
 * The listening side of CLocalPipe. Only the current user can connect:
 * - Windows: the pipe rejects remote clients (a named pipe is reachable over SMB otherwise), the
 *   first instance must be new (no other process holds the name), and its access list holds the
 *   current user only.
 * - Elsewhere: the socket is made mode 0600, and a connection whose peer runs as another user is
 *   closed. The folder that holds the socket should be 0700 as well.
 * No network listener of any kind is involved, loopback included.
 */
class CLocalPipeListener
{
public:
	CLocalPipeListener();
	~CLocalPipeListener();

	bool listen(const std::string &path, std::string &error);
	void close();
	bool isListening() const;
	const std::string &getPath() const { return m_Path; }

	/// A peer that connected since the last call, or nullptr; the caller owns it
	CLocalPipe *accept();

private:
	CLocalPipeListener(const CLocalPipeListener &);
	CLocalPipeListener &operator=(const CLocalPipeListener &);

	std::string m_Path;
#ifdef NL_OS_WINDOWS
	bool createInstance(bool first, std::string &error);
	void *m_Pending;
	void *m_Event;
	void *m_Overlapped;
	bool m_Connecting;
	void *m_SecurityDescriptor;
	void *m_Acl;
#else
	int m_Socket;
	bool m_Unlink;
#endif
};

} // NLMISC

#endif // NL_LOCAL_PIPE_H

/* End of local_pipe.h */
