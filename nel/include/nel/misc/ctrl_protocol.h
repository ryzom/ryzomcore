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

#ifndef NL_CTRL_PROTOCOL_H
#define NL_CTRL_PROTOCOL_H

#include "types_nl.h"
#include "json.h"

#include <string>
#include <vector>

namespace NLMISC {

/**
 * The control channel between a NeL process and a tool on the same machine.
 *
 * Version 2 is the dashboard channel of CSocketDisplayer, where the service dials the dashboard.
 * Version 3 adds the endpoint a process listens on (CCtrlEndpoint), for tools that come and go:
 * calls with JSON arguments and results, log and event lines indexed by the process itself,
 * and data frames for binary results.
 *
 * Framing, as in version 2: one message per line, fields separated by a tab, a backslash
 * escaping tab, newline, carriage return and backslash inside a field. A data frame is the line
 *   data <id> <content type> <length>
 * followed by exactly <length> raw bytes, then the next message.
 *
 * Messages of version 3:
 *   endpoint to tool: hello <version> <identity json>, result <id> <json>, error <id> <message> [<json>]
 *     and data frames, each sent before the result that names it ({"data": <id>, ...})
 *   tool to endpoint: call <id> <method> <arguments json>, cancel <id>
 */
const uint CtrlProtocolVersion = 3;

std::string escapeCtrlField(const std::string &value);
std::string unescapeCtrlField(const std::string &value);
/// Splits a line on its tabs, unescaping each field
void splitCtrlLine(const std::string &line, std::vector<std::string> &fields);
/// Joins escaped fields into a line, without the newline
std::string joinCtrlLine(const std::vector<std::string> &fields);

/// One message of the channel: the fields of a line, and for a data frame its bytes
struct CCtrlMessage
{
	std::vector<std::string> Fields;
	bool IsData;
	std::string Data;

	CCtrlMessage()
	    : IsData(false)
	{
	}
};

/// Cuts the bytes that arrive into messages
class CCtrlMessageReader
{
public:
	CCtrlMessageReader(size_t maxLine, size_t maxData);

	/// Where the bytes that arrived go
	std::string &buffer() { return m_Buffer; }

	/// The next complete message; false when more bytes are needed or on error, with the error set.
	/// After an error the stream is out of step and should be closed.
	bool next(CCtrlMessage &message, std::string &error);

	void clear();

private:
	std::string m_Buffer;
	size_t m_MaxLine;
	size_t m_MaxData;
	// a data frame whose header is read, waiting for its bytes
	bool m_InData;
	size_t m_DataLength;
	std::vector<std::string> m_DataFields;
};

/**
 * The registry of control endpoints of the current user: one JSON record per process, in
 *   Windows: %LOCALAPPDATA%\nel\ctrl
 *   elsewhere: $XDG_RUNTIME_DIR/nel.ctrl, mode 0700, with the endpoint sockets in it
 * There is no other location: without the variable there is no registry, and the error says so.
 *
 * A record: name, pid, start_time (see getProcessStartTime), endpoint (path), protocol, program,
 * cwd, build, log, state ("running", "exited", "failed"), exit_reason, error.
 */
class CCtrlRegistry
{
public:
	/// The folder, created when asked; false with the error when it cannot be had
	static bool getDirectory(std::string &dir, bool create, std::string &error);
	static std::string recordPath(const std::string &dir, const std::string &name, uint32 pid);
	/// The pipe or socket an endpoint listens on
	static std::string endpointPath(const std::string &dir, const std::string &name, uint32 pid);
	/// Writes a record whole: a reader never sees half of one
	static bool writeRecord(const std::string &path, const CJsonValue &record, std::string &error);
	static bool readRecord(const std::string &path, CJsonValue &record, std::string &error);
	/// The record files in the folder
	static void listRecords(const std::string &dir, std::vector<std::string> &paths);
	/// A name for an endpoint: letters, digits, '_', '-' and '.', 1 to 64 of them
	static bool isValidName(const std::string &name);

	/// When a process started, as a token that tells two processes of the same id apart: ms since the epoch
	/// on Windows and macOS, ms since the boot on Linux (a clock that is set does not move it). 0 when the
	/// process does not run or cannot be read. Compare it for equality only.
	static uint64 getProcessStartTime(uint32 pid);
	static uint32 getCurrentProcessId();
};

} // NLMISC

#endif // NL_CTRL_PROTOCOL_H

/* End of ctrl_protocol.h */
