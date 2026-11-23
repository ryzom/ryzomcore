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
#include "time_nl.h"

#include <deque>
#include <string>

namespace NLMISC {

/**
 * A byte stream to another process of the same user on this machine: a named pipe on Windows,
 * a Unix domain socket elsewhere. Nothing here waits on the other side for longer than asked:
 * write() gives up once its time is used up, whatever the peer takes meanwhile, and queue() with
 * flush() never waits at all. One thread uses a pipe at a time.
 */
class CLocalPipe
{
public:
	CLocalPipe();
	~CLocalPipe();

	/// Whether a path names a pipe on this machine. Windows: \\.\pipe\<name>, where the name has no '/' and
	/// no component of dots only, which the system would read as a way out of the pipes (to another machine).
	/// Elsewhere: a socket path within the length limit of a Unix domain socket.
	static bool checkPath(const std::string &path, std::string &error);

	/// Connects to a listening pipe; false with the reason in error when the path is not a pipe on this machine
	/// (see checkPath), or when nobody listens there (yet)
	bool connect(const std::string &path, std::string &error);
	/// Closes the pipe; what is queued and not written yet is dropped
	void close();
	bool isOpen() const;

	/// Writes every byte, those queued before first, within timeoutMs in all; the pipe is closed on failure
	bool write(const void *data, size_t size, uint timeoutMs);
	bool write(const std::string &data, uint timeoutMs) { return write(data.data(), data.size(), timeoutMs); }

	/// Queues bytes without waiting; flush() writes them as the peer takes them
	void queue(const std::string &data);
	/// Queues bytes without copying them: data is left empty
	void queueSwap(std::string &data);
	/// Writes what the peer takes now of the queue, without waiting; false when the pipe failed, and is closed
	bool flush();
	/// The bytes queued and not written yet
	size_t getQueuedBytes() const { return m_Queued; }
	/// For how long the peer has taken none of the queued bytes; 0 when nothing is queued
	TTime getStalledMs() const;

	/// Appends what has arrived to the buffer without waiting, at most maxBytes; false when the pipe closed
	bool readAvailable(std::string &buffer, size_t maxBytes);

	/// The process at the other end, as the system knows it: a dialler checks it reached the process it meant
	bool getPeerProcessId(uint32 &pid, std::string &error) const;

private:
	friend class CLocalPipeListener;

	CLocalPipe(const CLocalPipe &);
	CLocalPipe &operator=(const CLocalPipe &);

	void consumed(size_t bytes);
	/// Waits at most timeoutMs for the peer to take more of the queue
	void waitWritable(uint timeoutMs);

	// the bytes to write; the first string is written from m_OutOffset on
	std::deque<std::string> m_Out;
	size_t m_OutOffset;
	size_t m_Queued;
	TTime m_LastProgress;
#ifdef NL_OS_WINDOWS
	void *m_Handle;
	void *m_Event;
	// the write in flight, from the first string of m_Out, which stays as it is until the write completes
	void *m_WriteEvent;
	void *m_WriteOverlapped;
	bool m_WritePending;
	// the listening side of the pipe, whose peer is a client process
	bool m_Server;
#else
	int m_Socket;
#endif
};

/**
 * The listening side of CLocalPipe. Only the current user can connect:
 * - Windows: the pipe rejects remote clients (a named pipe is reachable over SMB otherwise), the
 *   first instance must be new (no other process holds the name) and the listener holds an instance
 *   at all times after it, its access list holds the current user only, and the pipe of an elevated
 *   process has the integrity level of that process, so a process of the user that is not elevated
 *   cannot open it.
 * - Elsewhere: the socket is made mode 0600, and a connection whose peer runs as another user, or whose
 *   user cannot be read, is closed. The folder that holds the socket should be 0700 as well.
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
	void *createPipe(bool first, std::string &error);
	/// Waits for the next peer on the pending instance
	bool startConnect(std::string &error);
	void warnOnce(const std::string &error);
	void *m_Pending;
	void *m_Event;
	void *m_Overlapped;
	// the pending instance waits for a peer, or has one that is handed out once another instance waits
	bool m_Connecting;
	bool m_Connected;
	bool m_Warned;
	void *m_SecurityDescriptor;
	void *m_Dacl;
	void *m_Sacl;
#else
	int m_Socket;
	// the socket file this listener made, so close() removes that one and not another put at the path since
	bool m_Unlink;
	uint64 m_Device;
	uint64 m_Inode;
#endif
};

} // NLMISC

#endif // NL_LOCAL_PIPE_H

/* End of local_pipe.h */
