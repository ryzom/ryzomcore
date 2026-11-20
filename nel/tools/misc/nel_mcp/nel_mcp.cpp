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

// nel_mcp: a Model Context Protocol server over stdio for the control endpoints of NeL processes on
// this machine (a client started with --ctrl-listen=<name>, see nel/misc/ctrl_endpoint.h). It knows
// instances, their state, commands, log, events and methods through the endpoint protocol. Of the Ryzom
// client it knows what launch and login need: the order of the account arguments on its command line
// (login password [shard [slot]]), its login.login method, and the state names and message box codes the
// tool descriptions mention.
// Usage: nel_mcp [--profile=<file>] [--log-dir=<folder>]
// The profile is a JSON file with the programs it may launch and the accounts they log in with; there
// is no default one. It is read when a tool needs it, so it can change while the server runs.
// The log folder gets the log of this server (log.log) and its crash reports. Without one there is no log
// file, and a crash report goes to the working directory, as for every NeL program.

#include "nel/misc/types_nl.h"

#include "nel/misc/base64.h"
#include "nel/misc/common.h"
#include "nel/misc/ctrl_endpoint.h"
#include "nel/misc/ctrl_protocol.h"
#include "nel/misc/debug.h"
#include "nel/misc/displayer.h"
#include "nel/misc/json.h"
#include "nel/misc/local_pipe.h"
#include "nel/misc/mutex.h"
#include "nel/misc/path.h"
#include "nel/misc/string_common.h"
#include "nel/misc/thread.h"
#include "nel/misc/time_nl.h"

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#ifdef NL_OS_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifndef NL_OS_WINDOWS
extern char **environ;
#endif

using namespace NLMISC;

namespace {

const char *const s_ServerName = "nel_mcp";
const char *const s_ServerVersion = "1.0.0";
const char *const s_McpVersions[] = { "2025-06-18", "2025-03-26", "2024-11-05" };
const char *const s_Instructions
    = "Control endpoints of NeL processes on this machine, such as Ryzom clients started with --ctrl-listen=<name>. "
      "Each instance has a name; the instance parameter can be left out while one runs. get_state answers even while "
      "the main thread of the instance is busy, and says for how long; wait_for_state waits for a state instead of "
      "sleeping. Log lines and events are indexed by the process from 1 and never renumbered: get_log and get_events "
      "continue from next_idx, and with wait_seconds they wait for the next matching line or event, which replaces "
      "polling a log file. run_command runs a command of the instance with its arguments as a list and returns its "
      "output and what it showed in the chat. list_methods lists what call_method can do (Lua, action handlers, input, "
      "interface, world, screen grabs...), filtered by a prefix such as 'ui.'; ask for schemas when calling one for the "
      "first time. Methods that change something take the lease of the instance, so one session drives an instance at "
      "a time. launch starts a program of the profile given to this server, logs it in with an account of the "
      "profile, and waits for its endpoint.";
const uint s_HelloTimeoutMs = 3000;
const uint s_WriteTimeoutMs = 10000;
const uint s_LaunchCheckMs = 200;
const double s_MaxWaitSeconds = 600;

typedef std::deque<std::string> TLines;
typedef std::pair<std::string, std::string> TVariable;
typedef std::vector<TVariable> TEnvironment;

// *** output ***

// set when stdout no longer takes what is written: the host went away
bool s_OutputClosed = false;

void writeMessage(const CJsonValue &message)
{
	if (s_OutputClosed)
		return;
	std::string text = message.toString();
	text += '\n';
	if (fwrite(text.data(), 1, text.size(), stdout) != text.size() || fflush(stdout) != 0)
		s_OutputClosed = true;
}

void reply(const CJsonValue &id, const CJsonValue &result)
{
	CJsonValue o = CJsonValue::object();
	o.set("jsonrpc", "2.0");
	o.set("id", id);
	o.set("result", result);
	writeMessage(o);
}

void replyError(const CJsonValue &id, sint32 code, const std::string &message)
{
	CJsonValue error = CJsonValue::object();
	error.set("code", code);
	error.set("message", message);
	CJsonValue o = CJsonValue::object();
	o.set("jsonrpc", "2.0");
	o.set("id", id);
	o.set("error", error);
	writeMessage(o);
}

CJsonValue textContent(const std::string &text)
{
	CJsonValue c = CJsonValue::object();
	c.set("type", "text");
	c.set("text", text);
	return c;
}

void replyTool(const CJsonValue &id, const CJsonValue &content, bool isError)
{
	CJsonValue r = CJsonValue::object();
	r.set("content", content);
	r.set("isError", isError);
	reply(id, r);
}

void replyToolText(const CJsonValue &id, const std::string &text, bool isError)
{
	CJsonValue content = CJsonValue::array();
	content.push(textContent(text));
	replyTool(id, content, isError);
}

std::string formatTime(uint64 ms)
{
	time_t seconds = (time_t)(ms / 1000);
	struct tm local;
#ifdef NL_OS_WINDOWS
	localtime_s(&local, &seconds);
#else
	localtime_r(&seconds, &local);
#endif
	return toString("%02d:%02d:%02d.%03u", local.tm_hour, local.tm_min, local.tm_sec, (uint)(ms % 1000));
}

// *** stdin ***

class CStdinReader : public IRunnable
{
public:
	CStdinReader()
	    : Lines("CStdinReader::Lines")
	    , Closed(false)
	{
	}

	virtual void run() NL_OVERRIDE
	{
		std::string line;
		while (std::getline(std::cin, line))
		{
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			CSynchronized<TLines>::CAccessor access(&Lines);
			access.value().push_back(line);
		}
		Closed = true;
	}

	bool take(TLines &out)
	{
		CSynchronized<TLines>::CAccessor access(&Lines);
		out.swap(access.value());
		return !out.empty();
	}

	CSynchronized<TLines> Lines;
	volatile bool Closed;
};

// *** instances ***

enum TProcess
{
	// the process of the record runs
	ProcessRuns,
	// no process has its id, or one that started at another time has it
	ProcessGone,
	// a process has its id but cannot be inspected
	ProcessUnknown
};

/// Whether the process of a record runs. Gone only when that is certain, so a record is never taken for a dead
/// one because a probe failed.
TProcess probeProcess(uint32 pid, uint64 startTime)
{
	if (pid == 0 || startTime == 0)
		return ProcessUnknown;
	uint64 started = CCtrlRegistry::getProcessStartTime(pid);
	if (started == startTime)
		return ProcessRuns;
	if (started != 0)
		return ProcessGone;
#ifdef NL_OS_WINDOWS
	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)pid);
	if (process == NULL)
		return GetLastError() == ERROR_INVALID_PARAMETER ? ProcessGone : ProcessUnknown;
	// a process that ended keeps its id while others hold a handle of it
	DWORD code = 0;
	bool ended = GetExitCodeProcess(process, &code) && code != STILL_ACTIVE;
	CloseHandle(process);
	return ended ? ProcessGone : ProcessUnknown;
#else
	if (kill((pid_t)pid, 0) != 0 && errno == ESRCH)
		return ProcessGone;
	return ProcessUnknown;
#endif
}

struct CInstance
{
	std::string Name;
	std::string RecordPath;
	CJsonValue Record;
	uint32 Pid;
	uint64 StartTime;
	TProcess Process;
	bool Alive;
	// why the record cannot be used to connect, empty when it can
	std::string Invalid;

	CInstance()
	    : Pid(0)
	    , StartTime(0)
	    , Process(ProcessUnknown)
	    , Alive(false)
	{
	}

	const std::string &getState() const { return Record.get("state").getString(); }
	/// An endpoint that takes calls
	bool isRunning() const { return Alive && Invalid.empty() && getState() == "running"; }
};

/// The records of the registry, with whether their process still runs
bool scanInstances(std::vector<CInstance> &out, std::string &error)
{
	out.clear();
	std::string dir;
	// a read does not create the folder: without it, no endpoint has been opened yet
	if (!CCtrlRegistry::getDirectory(dir, false, error))
	{
		if (!dir.empty() && !CFile::isExists(dir))
		{
			error.clear();
			return true;
		}
		return false;
	}
	if (!CFile::isDirectory(dir))
		return true;
	std::vector<std::string> records;
	CCtrlRegistry::listRecords(dir, records);
	for (uint i = 0; i < records.size(); ++i)
	{
		CInstance instance;
		std::string readError;
		if (!CCtrlRegistry::readRecord(records[i], instance.Record, readError))
			continue;
		instance.RecordPath = records[i];
		instance.Name = instance.Record.get("name").getString();
		instance.Pid = (uint32)instance.Record.get("pid").getInt(0);
		instance.StartTime = (uint64)instance.Record.get("start_time").getInt(0);
		instance.Process = probeProcess(instance.Pid, instance.StartTime);
		instance.Alive = instance.Process == ProcessRuns;
		// the pipe is where the endpoint of that name and process listens, never a path the record chose: a
		// record that named another one would get what a call carries, the password of login included
		const std::string &endpoint = instance.Record.get("endpoint").getString();
		if (!CCtrlRegistry::isValidName(instance.Name))
		{
			instance.Invalid = "the record '" + instance.RecordPath + "' has no valid endpoint name";
		}
		else
		{
			std::string expected = CCtrlRegistry::endpointPath(dir, instance.Name, instance.Pid);
			if (endpoint != expected)
				instance.Invalid = "the record '" + instance.RecordPath + "' names the endpoint '" + endpoint + "', where '" + instance.Name + "' of process "
				    + toString("%u", instance.Pid) + " listens on '" + expected + "'";
		}
		out.push_back(instance);
	}
	return true;
}

// *** operations: tool calls waiting for an answer ***

enum TFormat
{
	FormatJson,
	FormatLog,
	FormatEvents,
	FormatState,
	FormatCommand
};

struct COp
{
	CJsonValue RpcId;
	std::string Instance;
	// the link the call went on, and its id there
	uint32 LinkSerial;
	std::string CallId;
	TFormat Format;
	uint TimeoutMs;
	TTime Deadline;
	bool Launch;
	uint32 LaunchPid;
	std::string LaunchName;
	// why the endpoint of the launched process could not be used yet, for the answer when the time runs out
	std::string LaunchError;
	TTime NextCheck;
#ifdef NL_OS_WINDOWS
	void *LaunchProcess;
#endif

	COp()
	    : LinkSerial(0)
	    , Format(FormatJson)
	    , TimeoutMs(0)
	    , Deadline(0)
	    , Launch(false)
	    , LaunchPid(0)
	    , NextCheck(0)
#ifdef NL_OS_WINDOWS
	    , LaunchProcess(nullptr)
#endif
	{
	}
};

std::list<COp> s_Ops;
std::string s_ProfilePath;

// *** links to endpoints ***

struct CAttachment
{
	std::string Id;
	std::string Type;
	std::string Bytes;
};

typedef std::vector<CAttachment> TAttachments;
typedef std::map<std::string, TAttachments> TAttachmentsByCall;

class CLink
{
public:
	CLink()
	    : Serial(0)
	    , Reader(256 * 1024 * 1024, 256 * 1024 * 1024)
	    , Pid(0)
	    , StartTime(0)
	    , Greeted(false)
	    , GreetDeadline(0)
	{
	}

	std::string Name;
	// tells this link from an earlier one of the same name
	uint32 Serial;
	CLocalPipe Pipe;
	CCtrlMessageReader Reader;
	uint32 Pid;
	uint64 StartTime;
	// the greeting of the endpoint came and was understood; until then what is to be sent waits in Queued, and
	// nothing waits for the greeting on the main loop
	bool Greeted;
	TTime GreetDeadline;
	CJsonValue Hello;
	std::vector<std::string> Queued;
	// why this server closed the link, empty when the endpoint closed it
	std::string Failure;
	// data frames by call id, until the answer of the call comes
	TAttachmentsByCall Data;
};

std::map<std::string, CLink *> s_Links;
uint32 s_NextLinkSerial = 1;
// call ids are unique over all links of this server, so an answer never completes the call of another link
uint32 s_NextCall = 1;

/// One message of a link, or false when there is none yet; the link is closed on a protocol error
bool readLink(CLink &link, CCtrlMessage &message)
{
	std::string error;
	if (link.Reader.next(message, error))
		return true;
	if (!error.empty())
	{
		link.Failure = "the connection to '" + link.Name + "' broke: " + error;
		link.Pipe.close();
		link.Reader.clear();
		return false;
	}
	if (!link.Pipe.isOpen())
		return false;
	// what arrived before the pipe closed is read all the same: the answer to a quit comes just before the close
	link.Pipe.readAvailable(link.Reader.buffer(), 4 * 1024 * 1024);
	if (link.Reader.next(message, error))
		return true;
	if (!error.empty())
	{
		link.Failure = "the connection to '" + link.Name + "' broke: " + error;
		link.Pipe.close();
		link.Reader.clear();
	}
	return false;
}

void failLink(CLink &link, const std::string &reason)
{
	link.Failure = reason;
	link.Pipe.close();
	link.Reader.clear();
	link.Queued.clear();
}

/// Sends a line on a link, or keeps it until the greeting came
bool sendLine(CLink &link, const std::string &line)
{
	if (!link.Pipe.isOpen())
		return false;
	if (!link.Greeted)
	{
		link.Queued.push_back(line);
		return true;
	}
	return link.Pipe.write(line + "\n", s_WriteTimeoutMs);
}

/// Why the operations of a closed link get no answer
std::string closedReason(const CLink &link)
{
	if (!link.Failure.empty())
		return link.Failure;
	return "'" + link.Name + "' closed the connection before it answered; list_instances says why";
}

/// Answers the operations still waiting on a link that goes away
void failLinkOps(const CLink &link)
{
	std::string reason = link.Pipe.isOpen() ? "'" + link.Name + "' was replaced by another process of that name before it answered" : closedReason(link);
	for (std::list<COp>::iterator op = s_Ops.begin(); op != s_Ops.end();)
	{
		if (!op->Launch && op->LinkSerial == link.Serial)
		{
			replyToolText(op->RpcId, reason, true);
			op = s_Ops.erase(op);
		}
		else
		{
			++op;
		}
	}
}

/// The link to a running instance, connected; its greeting may still be on the way
CLink *getLink(const CInstance &instance, std::string &error)
{
	if (!instance.Invalid.empty())
	{
		error = instance.Invalid;
		return nullptr;
	}
	std::map<std::string, CLink *>::iterator it = s_Links.find(instance.Name);
	if (it != s_Links.end())
	{
		CLink *link = it->second;
		if (link->Pipe.isOpen() && link->Pid == instance.Pid && link->StartTime == instance.StartTime)
			return link;
		failLinkOps(*link);
		delete link;
		s_Links.erase(it);
	}
	const std::string &path = instance.Record.get("endpoint").getString();
	CLink *link = new CLink();
	link->Name = instance.Name;
	link->Serial = s_NextLinkSerial++;
	link->Pid = instance.Pid;
	link->StartTime = instance.StartTime;
	if (!link->Pipe.connect(path, error))
	{
		error = "cannot connect to '" + instance.Name + "' at '" + path + "': " + error;
		delete link;
		return nullptr;
	}
	// the process that listens there must be the one of the record: another one holding a pipe of that name would get
	// the calls, and the password the login tool sends
	uint32 peer = 0;
	std::string peerError;
	if (!link->Pipe.getPeerProcessId(peer, peerError))
	{
		error = "cannot tell which process listens for '" + instance.Name + "' at '" + path + "': " + peerError;
		delete link;
		return nullptr;
	}
	if (peer != instance.Pid)
	{
		error = toString("the process that listens for '%s' at '%s' is %u, not the process %u of its record", instance.Name.c_str(), path.c_str(), peer, instance.Pid);
		delete link;
		return nullptr;
	}
	// the greeting of the endpoint first, read by pumpLinks: its version tells whether we speak the same protocol.
	// Ours goes once it came, then the calls.
	std::vector<std::string> fields;
	fields.push_back("hello");
	fields.push_back(toString("%u", CtrlProtocolVersion));
	fields.push_back(s_ServerName);
	link->Queued.push_back(joinCtrlLine(fields));
	link->GreetDeadline = CTime::getLocalTime() + s_HelloTimeoutMs;
	s_Links[instance.Name] = link;
	return link;
}

/// Takes the first message of a link, its greeting; false when the link failed on it
bool greet(CLink &link, const CCtrlMessage &message)
{
	if (message.IsData || message.Fields.size() < 3 || message.Fields[0] != "hello")
	{
		failLink(link, toString("'%s' did not begin with a greeting: it does not speak version %u of the control protocol", link.Name.c_str(), CtrlProtocolVersion));
		return false;
	}
	uint version = 0;
	fromString(message.Fields[1], version);
	if (version > CtrlProtocolVersion)
	{
		failLink(link,
		    toString("'%s' speaks version %u of the control protocol, this server version %u: use a newer nel_mcp", link.Name.c_str(), version, CtrlProtocolVersion));
		return false;
	}
	std::string parseError;
	if (!link.Hello.parse(message.Fields[2], parseError))
	{
		failLink(link, "the greeting of '" + link.Name + "' is not JSON: " + parseError);
		return false;
	}
	if (!link.Hello.isObject())
	{
		failLink(link, "the greeting of '" + link.Name + "' is not a JSON object");
		return false;
	}
	link.Greeted = true;
	for (uint i = 0; i < link.Queued.size(); ++i)
	{
		if (!link.Pipe.write(link.Queued[i] + "\n", s_WriteTimeoutMs))
		{
			failLink(link, "'" + link.Name + "' does not take the calls: the connection closed");
			return false;
		}
	}
	link.Queued.clear();
	return true;
}

// *** answers ***

std::string renderLogLines(const CJsonValue &result)
{
	// by concatenation: a formatted line is cut at 2048 bytes
	std::string text;
	const CJsonValue &lines = result.get("lines");
	for (uint i = 0; i < lines.size(); ++i)
	{
		const CJsonValue &l = lines.at(i);
		text += toString("%u ", (uint)l.get("idx").getInt(0));
		text += formatTime((uint64)l.get("time").getInt(0));
		text += " " + l.get("level").getString() + " " + l.get("text").getString();
		if (l.has("origin"))
			text += "  (" + l.get("origin").getString() + ")";
		text += "\n";
	}
	text += toString("-- %u lines; next_idx %u, first held %u, newest %u%s", lines.size(), (uint)result.get("next_idx").getInt(0), (uint)result.get("first_idx").getInt(0),
	    (uint)result.get("newest_idx").getInt(0), result.get("truncated").getBool() ? ", truncated (more lines in the selection)" : "");
	if (result.has("missed"))
		text += "\n-- " + result.get("missed").getString();
	return text;
}

std::string renderEvents(const CJsonValue &result)
{
	std::string text;
	const CJsonValue &events = result.get("events");
	for (uint i = 0; i < events.size(); ++i)
	{
		const CJsonValue &e = events.at(i);
		text += toString("%u ", (uint)e.get("idx").getInt(0));
		text += formatTime((uint64)e.get("time").getInt(0));
		text += " " + e.get("kind").getString() + " " + e.get("data").toString() + "\n";
	}
	text += toString("-- %u events; next_idx %u, first held %u, newest %u%s", events.size(), (uint)result.get("next_idx").getInt(0), (uint)result.get("first_idx").getInt(0),
	    (uint)result.get("newest_idx").getInt(0), result.get("truncated").getBool() ? ", truncated" : "");
	if (result.has("missed"))
		text += "\n-- " + result.get("missed").getString();
	return text;
}

std::string renderCommand(const CJsonValue &result)
{
	std::string text = result.get("command").getString() + (result.get("ok").getBool() ? ": ok\n" : ": failed\n");
	const CJsonValue &output = result.get("output");
	for (uint i = 0; i < output.size(); ++i)
		text += output.at(i).getString() + "\n";
	const CJsonValue &shown = result.get("shown");
	if (shown.size() > 0)
	{
		text += "-- shown meanwhile:\n";
		for (uint i = 0; i < shown.size(); ++i)
			text += shown.at(i).getString() + "\n";
	}
	return text;
}

/// The answer of an endpoint to an operation, as the content of a tool result
void finishOp(COp &op, CLink &link, const CCtrlMessage &message)
{
	bool isError = message.Fields[0] == "error";
	TAttachmentsByCall::iterator data = link.Data.find(op.CallId);

	// result <id> <json>; error <id> <message> [<json>]
	std::string json = isError ? (message.Fields.size() >= 4 ? message.Fields[3] : std::string()) : (message.Fields.size() >= 3 ? message.Fields[2] : std::string());
	CJsonValue wrapper;
	std::string parseError;
	if ((!isError || !json.empty()) && !wrapper.parse(json, parseError))
	{
		// what the endpoint wrote and this server cannot read, such as a result nested deeper than the reader takes
		if (data != link.Data.end())
			link.Data.erase(data);
		std::string text = "the answer of '" + op.Instance + "' is not JSON: " + parseError;
		if (isError)
			text = message.Fields[2] + "\n" + text;
		replyToolText(op.RpcId, text, true);
		return;
	}
	const CJsonValue &result = wrapper.get("result");
	std::string header = "[" + op.Instance + toString(" frame %u, ", (uint)wrapper.get("frame").getInt(0)) + formatTime((uint64)wrapper.get("time").getInt(0)) + ", "
	    + wrapper.get("state").getString() + "]";

	CJsonValue content = CJsonValue::array();
	if (isError)
	{
		std::string text = message.Fields.size() >= 3 ? message.Fields[2] : std::string("error");
		if (!result.isNull())
			text += "\n" + result.toString();
		content.push(textContent(text + "\n" + header));
	}
	else
	{
		std::string text;
		switch (op.Format)
		{
		case FormatLog: text = renderLogLines(result); break;
		case FormatEvents: text = renderEvents(result); break;
		case FormatCommand: text = renderCommand(result); break;
		default: text = result.toString(); break;
		}
		content.push(textContent(text + "\n" + header));
	}

	// the data frames of the call, before its answer
	if (data != link.Data.end())
	{
		for (uint i = 0; i < data->second.size(); ++i)
		{
			const CAttachment &a = data->second[i];
			if (a.Type.compare(0, 6, "image/") == 0 && a.Type != "image/svg+xml")
			{
				CJsonValue c = CJsonValue::object();
				c.set("type", "image");
				c.set("data", base64::encode(a.Bytes));
				c.set("mimeType", a.Type);
				content.push(c);
			}
			else if (a.Type.compare(0, 5, "text/") == 0 || a.Type == "application/json" || a.Type == "image/svg+xml")
			{
				content.push(textContent("[" + a.Id + ", " + a.Type + "]\n" + a.Bytes));
			}
			else
			{
				content.push(textContent("[" + a.Id + toString(": %u bytes of ", (uint)a.Bytes.size()) + a.Type + ", not shown]"));
			}
		}
		link.Data.erase(data);
	}
	replyTool(op.RpcId, content, isError);
}

/// Sends a call to an instance; the answer completes the operation later
bool startCall(const CJsonValue &rpcId, const std::string &instanceName, const std::string &method, const CJsonValue &args, TFormat format, uint timeoutMs, std::string &error)
{
	std::vector<CInstance> instances;
	if (!scanInstances(instances, error))
		return false;
	const CInstance *instance = nullptr;
	const CInstance *invalid = nullptr;
	std::string names;
	uint alive = 0;
	for (uint i = 0; i < instances.size(); ++i)
	{
		if (!instances[i].isRunning())
		{
			if (instances[i].Alive && !instances[i].Invalid.empty() && instances[i].Name == instanceName)
				invalid = &instances[i];
			continue;
		}
		++alive;
		names += (names.empty() ? "" : ", ") + instances[i].Name;
		if (instanceName.empty() ? alive == 1 : instances[i].Name == instanceName)
			instance = &instances[i];
	}
	if (instanceName.empty() && alive > 1)
	{
		error = "several instances run (" + names + "): name one with the instance parameter";
		return false;
	}
	if (instance == nullptr)
	{
		if (invalid != nullptr)
			error = invalid->Invalid;
		else if (instanceName.empty())
			error = "no instance runs; launch one, or start a client with --ctrl-listen=<name>";
		else
			error = "no running instance '" + instanceName + "'; running: " + (names.empty() ? std::string("none") : names);
		return false;
	}
	CLink *link = getLink(*instance, error);
	if (link == nullptr)
		return false;
	std::string callId = toString("%u", s_NextCall++);
	std::vector<std::string> fields;
	fields.push_back("call");
	fields.push_back(callId);
	fields.push_back(method);
	fields.push_back(args.isNull() ? std::string("{}") : args.toString());
	if (!sendLine(*link, joinCtrlLine(fields)))
	{
		error = "'" + instance->Name + "' does not take the call: the connection closed";
		return false;
	}
	COp op;
	op.RpcId = rpcId;
	op.Instance = instance->Name;
	op.LinkSerial = link->Serial;
	op.CallId = callId;
	op.Format = format;
	op.TimeoutMs = timeoutMs;
	op.Deadline = CTime::getLocalTime() + timeoutMs;
	s_Ops.push_back(op);
	return true;
}

// *** profile and launch ***

bool readProfile(CJsonValue &profile, std::string &error)
{
	if (s_ProfilePath.empty())
	{
		error = "no profile: give nel_mcp --profile=<file> (in .mcp.json), a JSON file with \"programs\" (program, cwd, args, env) and \"accounts\" (login, password, shard)";
		return false;
	}
	FILE *f = nlfopen(s_ProfilePath, "rb");
	if (f == nullptr)
	{
		error = "cannot read the profile '" + s_ProfilePath + "': " + formatErrorMessage(getLastError());
		return false;
	}
	std::string text;
	char buffer[4096];
	size_t n;
	while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0)
		text.append(buffer, n);
	fclose(f);
	// a file editor may begin a UTF-8 file with a byte order mark, which is not JSON
	if (text.compare(0, 3, "\xEF\xBB\xBF") == 0)
		text.erase(0, 3);
	if (!profile.parse(text, error))
	{
		error = "the profile '" + s_ProfilePath + "' is not JSON: " + error;
		return false;
	}
	if (!profile.isObject())
	{
		error = "the profile '" + s_ProfilePath + "' is not a JSON object";
		return false;
	}
	return true;
}

/// Expands ${NAME}, the variable NAME of this process. A variable that is not set, or a ${ without its }, is an
/// error that names it and where it is.
bool expandVariables(const std::string &value, const std::string &where, std::string &result, std::string &error)
{
	result.clear();
	std::string::size_type pos = 0;
	for (;;)
	{
		std::string::size_type start = value.find("${", pos);
		if (start == std::string::npos)
		{
			result += value.substr(pos);
			return true;
		}
		std::string::size_type end = value.find('}', start);
		if (end == std::string::npos)
		{
			error = "'" + value + "' in " + where + " has a ${ without its }";
			return false;
		}
		result += value.substr(pos, start - pos);
		std::string name = value.substr(start + 2, end - start - 2);
		if (name.empty())
		{
			error = "'" + value + "' in " + where + " has a ${} without a name";
			return false;
		}
#ifdef NL_OS_WINDOWS
		const wchar_t *v = _wgetenv(nlUtf8ToWide(name));
		if (v == nullptr)
		{
			error = "the variable " + name + " of '" + value + "' in " + where + " is not set in the environment of nel_mcp";
			return false;
		}
		result += wideToUtf8(v);
#else
		const char *v = getenv(name.c_str());
		if (v == nullptr)
		{
			error = "the variable " + name + " of '" + value + "' in " + where + " is not set in the environment of nel_mcp";
			return false;
		}
		result += v;
#endif
		pos = end + 1;
	}
}

/// A path of a program entry: there, expanded, and absolute, so a launch does not depend on the working
/// directory of this server
bool profilePath(const CJsonValue &entry, const char *key, const std::string &where, std::string &path, std::string &error)
{
	std::string what = toString("\"%s\" of ", key) + where;
	const CJsonValue &value = entry.get(key);
	if (!value.isString() || value.getString().empty())
	{
		error = what + " is missing: it must be an absolute path";
		return false;
	}
	if (!expandVariables(value.getString(), what, path, error))
		return false;
	if (!CPath::isAbsolutePath(path))
	{
		error = what + " is '" + path + "', which is not an absolute path";
		return false;
	}
	return true;
}

/// Whether the client reads an argument as an option, not as a value: CCmdArgs takes "-x" and "--x" (and "/x" on
/// Windows) for options, and has no "--" to end them
bool isReadAsOption(const std::string &arg)
{
#ifdef NL_OS_WINDOWS
	return arg.size() > 1 && (arg[0] == '-' || arg[0] == '/');
#else
	return arg.size() > 1 && arg[0] == '-';
#endif
}

#ifdef NL_OS_WINDOWS
std::wstring quoteArgument(const std::wstring &arg)
{
	// the rules CommandLineToArgvW reads back
	if (!arg.empty() && arg.find_first_of(L" \t\"") == std::wstring::npos)
		return arg;
	std::wstring result = L"\"";
	for (std::wstring::size_type i = 0;; ++i)
	{
		uint backslashes = 0;
		while (i < arg.size() && arg[i] == L'\\')
		{
			++i;
			++backslashes;
		}
		if (i == arg.size())
		{
			result.append(backslashes * 2, L'\\');
			break;
		}
		if (arg[i] == L'"')
		{
			result.append(backslashes * 2 + 1, L'\\');
			result += L'"';
		}
		else
		{
			result.append(backslashes, L'\\');
			result += arg[i];
		}
	}
	result += L'"';
	return result;
}

std::wstring toUpperWide(const std::wstring &s)
{
	std::wstring upper = s;
	for (uint k = 0; k < upper.size(); ++k)
		upper[k] = towupper(upper[k]);
	return upper;
}
#endif

#ifndef NL_OS_WINDOWS
// the exit codes of the processes this server started, reaped as they end so none stays a zombie
std::map<uint32, sint32> s_ExitCodes;
#endif

/// Starts a process detached from our stdio, with an absolute program and working directory and the variables
/// of env over the environment of this process; its pid, or 0 with the error
uint32 startProcess(const std::string &program, const std::vector<std::string> &args, const std::string &cwd, const TEnvironment &env, void **processHandle, std::string &error)
{
#ifdef NL_OS_WINDOWS
	std::wstring wprogram = utf8ToWide(program);
	std::wstring commandLine = quoteArgument(wprogram);
	for (uint i = 0; i < args.size(); ++i)
		commandLine += L" " + quoteArgument(utf8ToWide(args[i]));

	// the environment of this process, with the profile's variables over it, sorted as CreateProcess wants it
	std::map<std::wstring, std::wstring> vars;
	std::map<std::wstring, std::wstring> names;
	wchar_t *current = GetEnvironmentStringsW();
	for (const wchar_t *p = current; *p; p += wcslen(p) + 1)
	{
		std::wstring entry(p);
		std::wstring::size_type eq = entry.find(L'=', 1);
		if (eq == std::wstring::npos)
			continue;
		std::wstring key = entry.substr(0, eq);
		std::wstring upper = toUpperWide(key);
		vars[upper] = entry.substr(eq + 1);
		names[upper] = key;
	}
	FreeEnvironmentStringsW(current);
	for (uint i = 0; i < env.size(); ++i)
	{
		std::wstring key = utf8ToWide(env[i].first);
		std::wstring upper = toUpperWide(key);
		vars[upper] = utf8ToWide(env[i].second);
		if (names.find(upper) == names.end())
			names[upper] = key;
	}
	// NeL sets this in a process whose exception translator is installed, for the modules of that process. A
	// child that inherits it installs none, so its crashes and asserts would go without a report.
	vars.erase(L"NEL_SE_TRANS");
	std::vector<wchar_t> block;
	for (std::map<std::wstring, std::wstring>::iterator it = vars.begin(); it != vars.end(); ++it)
	{
		std::wstring entry = names[it->first] + L"=" + it->second;
		block.insert(block.end(), entry.begin(), entry.end());
		block.push_back(0);
	}
	if (block.empty())
		block.push_back(0);
	block.push_back(0);

	STARTUPINFOW si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	// no standard handles: the stdio of this server is the MCP channel, and a console program started without
	// handles of its own would get copies of it
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = NULL;
	si.hStdOutput = NULL;
	si.hStdError = NULL;
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	std::vector<wchar_t> line(commandLine.begin(), commandLine.end());
	line.push_back(0);
	std::wstring wcwd = utf8ToWide(cwd);
	// the program by its path, never searched for; no inherited handles
	if (!CreateProcessW(wprogram.c_str(), &line[0], nullptr, nullptr, FALSE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_PROCESS_GROUP, &block[0], wcwd.c_str(), &si, &pi))
	{
		error = "cannot start '" + program + "' in '" + cwd + "': " + formatErrorMessage(getLastError());
		return 0;
	}
	CloseHandle(pi.hThread);
	*processHandle = pi.hProcess;
	return (uint32)pi.dwProcessId;
#else
	*processHandle = nullptr;
	// everything the child needs is built before the fork: this process has threads, and the child of one may
	// only call what is safe after a fork until it runs the program (no allocation)
	std::vector<std::string> envStrings;
	for (char **e = environ; *e != nullptr; ++e)
	{
		std::string entry = *e;
		std::string key = entry.substr(0, entry.find('='));
		bool replaced = false;
		for (uint i = 0; i < env.size() && !replaced; ++i)
			replaced = env[i].first == key;
		if (!replaced)
			envStrings.push_back(entry);
	}
	for (uint i = 0; i < env.size(); ++i)
		envStrings.push_back(env[i].first + "=" + env[i].second);
	std::vector<char *> envp;
	for (uint i = 0; i < envStrings.size(); ++i)
		envp.push_back(const_cast<char *>(envStrings[i].c_str()));
	envp.push_back(nullptr);
	std::vector<char *> argv;
	argv.push_back(const_cast<char *>(program.c_str()));
	for (uint i = 0; i < args.size(); ++i)
		argv.push_back(const_cast<char *>(args[i].c_str()));
	argv.push_back(nullptr);
	const char *programPath = program.c_str();
	const char *cwdPath = cwd.c_str();

	// the child writes why it could not run the program on this pipe; a program that runs closes it unwritten
	int report[2];
	if (pipe(report) != 0)
	{
		error = "cannot start '" + program + "': no pipe: " + formatErrorMessage(getLastError());
		return 0;
	}
	fcntl(report[0], F_SETFD, FD_CLOEXEC);
	fcntl(report[1], F_SETFD, FD_CLOEXEC);

	pid_t pid = fork();
	if (pid < 0)
	{
		error = "cannot start '" + program + "': " + formatErrorMessage(getLastError());
		close(report[0]);
		close(report[1]);
		return 0;
	}
	if (pid == 0)
	{
		close(report[0]);
		setsid();
		int devnull = open("/dev/null", O_RDWR);
		if (devnull >= 0)
		{
			dup2(devnull, 0);
			dup2(devnull, 1);
			dup2(devnull, 2);
			if (devnull > 2)
				close(devnull);
		}
		// what failed: 0 the working directory, 1 the program
		int failure[2];
		failure[0] = 0;
		if (chdir(cwdPath) == 0)
		{
			execve(programPath, &argv[0], &envp[0]);
			failure[0] = 1;
		}
		failure[1] = errno;
		ssize_t written = write(report[1], failure, sizeof(failure));
		(void)written;
		_exit(127);
	}
	close(report[1]);
	int failure[2];
	ssize_t n;
	do
	{
		n = read(report[0], failure, sizeof(failure));
	} while (n < 0 && errno == EINTR);
	close(report[0]);
	if (n == (ssize_t)sizeof(failure))
	{
		int status = 0;
		waitpid(pid, &status, 0);
		if (failure[0] == 0)
			error = "cannot enter the working directory '" + cwd + "' of '" + program + "': " + formatErrorMessage(failure[1]);
		else
			error = "cannot run '" + program + "': " + formatErrorMessage(failure[1]);
		return 0;
	}
	// an exit code kept for an earlier process of the same id is not this one's
	s_ExitCodes.erase((uint32)pid);
	return (uint32)pid;
#endif
}

#ifndef NL_OS_WINDOWS
void reapChildren()
{
	int status = 0;
	pid_t pid;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
		s_ExitCodes[(uint32)pid] = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
#endif

/// Whether a process we started is still there, and its exit code when not
bool processRunning(COp &op, sint32 &exitCode)
{
#ifdef NL_OS_WINDOWS
	DWORD code = 0;
	if (op.LaunchProcess != nullptr && GetExitCodeProcess((HANDLE)op.LaunchProcess, &code) && code != STILL_ACTIVE)
	{
		exitCode = (sint32)code;
		return false;
	}
	return true;
#else
	reapChildren();
	std::map<uint32, sint32>::iterator it = s_ExitCodes.find(op.LaunchPid);
	if (it != s_ExitCodes.end())
	{
		exitCode = it->second;
		s_ExitCodes.erase(it);
		return false;
	}
	return true;
#endif
}

void closeLaunch(COp &op)
{
#ifdef NL_OS_WINDOWS
	if (op.LaunchProcess != nullptr)
		CloseHandle((HANDLE)op.LaunchProcess);
	op.LaunchProcess = nullptr;
#else
	(void)op;
#endif
}

/// The command line arguments of an account of the profile, as the client reads them: login password [shard [slot]]
bool accountArguments(const CJsonValue &profile, const std::string &accountName, const CJsonValue &args, std::vector<std::string> &out, std::string &error)
{
	const CJsonValue &account = profile.get("accounts").get(accountName);
	if (!account.isObject())
	{
		error = "no account '" + accountName + "' in the profile '" + s_ProfilePath + "'";
		return false;
	}
	std::string where = "account '" + accountName + "' of the profile '" + s_ProfilePath + "'";
	const CJsonValue &login = account.get("login");
	const CJsonValue &password = account.get("password");
	if (!login.isString() || login.getString().empty())
	{
		error = "the " + where + " has no \"login\"";
		return false;
	}
	if (!password.isString())
	{
		error = "the " + where + " has no \"password\"";
		return false;
	}
	std::string shard;
	const CJsonValue &shardValue = account.get("shard");
	if (shardValue.isNumber() && shardValue.getNumber() == std::floor(shardValue.getNumber()))
		shard = toString("%" NL_I64 "d", shardValue.getInt(0));
	else if (shardValue.isString())
		shard = shardValue.getString();
	else if (!shardValue.isNull())
	{
		error = "\"shard\" of the " + where + " must be the shard id, a whole number or its text";
		return false;
	}
	// the client has no way to take a value that looks like an option as a value
	if (isReadAsOption(login.getString()) || isReadAsOption(password.getString()) || isReadAsOption(shard))
	{
		error = "the login, password or shard of the " + where
		    + " begins as an option does, so the client would read it as one: launch without the account and log in with the login tool";
		return false;
	}
	out.push_back(login.getString());
	out.push_back(password.getString());
	if (!shard.empty())
		out.push_back(shard);
	if (args.has("slot"))
	{
		// the client reads the slot after the shard, by position
		if (shard.empty())
		{
			error = "slot needs the shard of the " + where + ": the client reads login password shard slot";
			return false;
		}
		out.push_back(toString("%" NL_I64 "d", args.get("slot").getInt(0)));
	}
	return true;
}

void startLaunch(const CJsonValue &rpcId, const CJsonValue &args)
{
	CJsonValue profile;
	std::string error;
	if (!readProfile(profile, error))
	{
		replyToolText(rpcId, error, true);
		return;
	}
	const std::string &programName = args.get("program").getString();
	const CJsonValue &program = profile.get("programs").get(programName);
	if (!program.isObject())
	{
		std::string names;
		const CJsonValue::TMembers &m = profile.get("programs").members();
		for (uint i = 0; i < m.size(); ++i)
			names += (i ? ", " : "") + m[i].first;
		replyToolText(rpcId, "no program '" + programName + "' in the profile; it has: " + (names.empty() ? std::string("none") : names), true);
		return;
	}
	const std::string &name = args.get("name").getString();
	if (!CCtrlRegistry::isValidName(name))
	{
		replyToolText(rpcId, "name: letters, digits, '_', '-' and '.', 64 at most", true);
		return;
	}
	if (args.has("slot") && !args.has("account"))
	{
		replyToolText(rpcId, "slot needs an account: the client reads the slot after the login, password and shard", true);
		return;
	}

	// the entry of the profile: program and cwd absolute, args and env text
	std::string where = "program '" + programName + "' of the profile '" + s_ProfilePath + "'";
	std::string programPath;
	std::string cwd;
	if (!profilePath(program, "program", where, programPath, error) || !profilePath(program, "cwd", where, cwd, error))
	{
		replyToolText(rpcId, error, true);
		return;
	}
	if (!CFile::fileExists(programPath) || CFile::isDirectory(programPath))
	{
		replyToolText(rpcId, "\"program\" of " + where + " is '" + programPath + "', which is not a file", true);
		return;
	}
#ifdef NL_OS_WINDOWS
	// cmd.exe would read the arguments of the call, which a model chooses, as commands
	std::string extension = toLowerAscii(CFile::getExtension(programPath));
	if (extension == "bat" || extension == "cmd")
	{
		replyToolText(rpcId, "\"program\" of " + where + " is '" + programPath + "', a batch file, which runs its arguments through cmd.exe: give the program itself", true);
		return;
	}
#endif
	if (!CFile::isDirectory(cwd))
	{
		replyToolText(rpcId, "\"cwd\" of " + where + " is '" + cwd + "', which is not a folder", true);
		return;
	}
	TEnvironment env;
	const CJsonValue &envEntry = program.get("env");
	if (!envEntry.isNull() && !envEntry.isObject())
	{
		replyToolText(rpcId, "\"env\" of " + where + " must be an object of variables and their values", true);
		return;
	}
	for (uint i = 0; i < envEntry.members().size(); ++i)
	{
		const CJsonValue::TMember &m = envEntry.members()[i];
		std::string what = "the variable '" + m.first + "' in \"env\" of " + where;
		if (m.first.empty() || m.first.find('=') != std::string::npos || !m.second.isString())
		{
			replyToolText(rpcId, what + " must have a name without '=' and a text value", true);
			return;
		}
		std::string value;
		if (!expandVariables(m.second.getString(), what, value, error))
		{
			replyToolText(rpcId, error, true);
			return;
		}
		env.push_back(std::make_pair(m.first, value));
	}
	std::vector<std::string> argv;
	const CJsonValue &fixed = program.get("args");
	if (!fixed.isNull() && !fixed.isArray())
	{
		replyToolText(rpcId, "\"args\" of " + where + " must be an array of texts", true);
		return;
	}
	for (uint i = 0; i < fixed.size(); ++i)
	{
		if (!fixed.at(i).isString())
		{
			replyToolText(rpcId, "\"args\" of " + where + " must be an array of texts", true);
			return;
		}
		argv.push_back(fixed.at(i).getString());
	}

	std::vector<CInstance> instances;
	if (!scanInstances(instances, error))
	{
		replyToolText(rpcId, error, true);
		return;
	}
	for (uint i = 0; i < instances.size(); ++i)
	{
		// as the endpoint decides whether its name is free: a live process whose record says running
		if (instances[i].Alive && instances[i].getState() == "running" && instances[i].Name == name)
		{
			replyToolText(rpcId, toString("an instance '%s' runs already (process %u)", name.c_str(), instances[i].Pid), true);
			return;
		}
	}

	// an account goes first, as the client reads it
	const std::string &accountName = args.get("account").getString();
	if (args.has("account"))
	{
		std::vector<std::string> accountArgs;
		if (!accountArguments(profile, accountName, args, accountArgs, error))
		{
			replyToolText(rpcId, error, true);
			return;
		}
		argv.insert(argv.begin(), accountArgs.begin(), accountArgs.end());
	}
	const CJsonValue &extra = args.get("args");
	for (uint i = 0; i < extra.size(); ++i)
		argv.push_back(extra.at(i).getString());
	// with "=": the client takes a name only in that form
	argv.push_back("--ctrl-listen=" + name);

	COp op;
	op.RpcId = rpcId;
	op.Launch = true;
	op.LaunchName = name;
	void *handle = nullptr;
	op.LaunchPid = startProcess(programPath, argv, cwd, env, &handle, error);
	if (op.LaunchPid == 0)
	{
		replyToolText(rpcId, error, true);
		return;
	}
#ifdef NL_OS_WINDOWS
	op.LaunchProcess = handle;
#endif
	op.TimeoutMs = (uint)args.get("wait_seconds").getInt(60) * 1000;
	op.Deadline = CTime::getLocalTime() + op.TimeoutMs;
	s_Ops.push_back(op);
}

/// A launch is answered when the endpoint of its process is up and greeted, or failed, or the process ended, or
/// time ran out
bool checkLaunch(COp &op)
{
	std::vector<CInstance> instances;
	std::string error;
	if (!scanInstances(instances, error))
		op.LaunchError = error;
	for (uint i = 0; i < instances.size(); ++i)
	{
		const CInstance &instance = instances[i];
		if (instance.Pid != op.LaunchPid || instance.Name != op.LaunchName)
			continue;
		const std::string &state = instance.getState();
		if (state == "failed")
		{
			replyToolText(op.RpcId, toString("process %u started, but its endpoint did not: ", op.LaunchPid) + instance.Record.get("error").getString(), true);
			return true;
		}
		if (state != "running")
			continue;
		if (!instance.Invalid.empty())
		{
			replyToolText(op.RpcId, toString("process %u started, but its record cannot be used: ", op.LaunchPid) + instance.Invalid, true);
			return true;
		}
		// a link to this process that failed has the reason; another getLink would only connect again
		std::map<std::string, CLink *>::iterator it = s_Links.find(op.LaunchName);
		if (it != s_Links.end() && it->second->Pid == instance.Pid && it->second->StartTime == instance.StartTime && !it->second->Pipe.isOpen()
		    && !it->second->Failure.empty())
		{
			replyToolText(op.RpcId, toString("process %u started and its endpoint is up, but: ", op.LaunchPid) + it->second->Failure, true);
			return true;
		}
		CLink *link = getLink(instance, error);
		if (link == nullptr)
		{
			op.LaunchError = error;
			return false;
		}
		if (!link->Greeted)
			return false;
		CJsonValue r = CJsonValue::object();
		r.set("instance", op.LaunchName);
		r.set("pid", op.LaunchPid);
		r.set("identity", link->Hello);
		replyToolText(op.RpcId, r.toString(), false);
		return true;
	}
	sint32 exitCode = 0;
	if (!processRunning(op, exitCode))
	{
		replyToolText(op.RpcId, toString("process %u exited with code %d before its endpoint came up; see its log", op.LaunchPid, exitCode), true);
		return true;
	}
	if (CTime::getLocalTime() >= op.Deadline)
	{
		if (!op.LaunchError.empty())
			replyToolText(op.RpcId, toString("process %u runs, but its endpoint could not be used in time: ", op.LaunchPid) + op.LaunchError, true);
		else
			replyToolText(op.RpcId,
			    toString("process %u runs, but its endpoint did not come up in time; it is still starting, or it was built without the endpoint (a final build)", op.LaunchPid),
			    true);
		return true;
	}
	return false;
}

// *** tools ***

CJsonValue prop(const char *type, const char *description)
{
	CJsonValue p = CJsonValue::object();
	p.set("type", type);
	p.set("description", description);
	return p;
}

/// A number with its bounds, which a call is checked against
CJsonValue rangeProp(const char *type, const char *description, double minimum, double maximum, bool exclusiveMinimum = false)
{
	CJsonValue p = prop(type, description);
	p.set(exclusiveMinimum ? "exclusiveMinimum" : "minimum", minimum);
	p.set("maximum", maximum);
	return p;
}

CJsonValue stringsProp(const char *description)
{
	CJsonValue p = prop("array", description);
	p.set("items", CJsonValue::object().set("type", "string"));
	return p;
}

CJsonValue instanceProp()
{
	return prop("string", "The instance name; leave out while one runs.");
}

CJsonValue tool(const char *name, const char *description, const CJsonValue &properties, const char *required, bool readOnly)
{
	CJsonValue schema = CJsonValue::object();
	schema.set("type", "object");
	schema.set("properties", properties.isNull() ? CJsonValue::object() : properties);
	CJsonValue req = CJsonValue::array();
	std::vector<std::string> names;
	explode(std::string(required), std::string(","), names, true);
	for (uint i = 0; i < names.size(); ++i)
		req.push(CJsonValue(names[i]));
	schema.set("required", req);
	CJsonValue t = CJsonValue::object();
	t.set("name", name);
	t.set("description", description);
	t.set("inputSchema", schema);
	CJsonValue annotations = CJsonValue::object();
	annotations.set("readOnlyHint", readOnly);
	t.set("annotations", annotations);
	return t;
}

CJsonValue toolList()
{
	CJsonValue tools = CJsonValue::array();
	tools.push(tool("list_instances",
	    "The instances on this machine: name, state (running, exited with its reason, gone without one after a crash, failed to start its endpoint), whether its process runs, process id, program, kind, log file, and what it reports (account, character). A record whose process is gone is shown once, then removed from the registry.",
	    CJsonValue(), "", false));
	CJsonValue p = CJsonValue::object();
	p.set("program", prop("string", "The program entry of the profile."));
	p.set("name", prop("string", "The instance name to give it: letters, digits, '_', '-', '.'."));
	p.set("account", prop("string", "An account entry of the profile to log in with; its password is on the command line of the process, so only for throwaway development accounts."));
	p.set("slot", rangeProp("integer", "The character slot to play, with an account that has a shard.", 0, 255));
	p.set("args", stringsProp("More arguments for the program."));
	p.set("wait_seconds", rangeProp("integer", "How long to wait for the endpoint (60 by default).", 1, s_MaxWaitSeconds));
	tools.push(tool("launch", "Starts a program of the profile with a control endpoint under the given name, and waits until the endpoint answers. Returns its identity.", p, "program,name", false));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("account", prop("string", "An account entry of the profile."));
	tools.push(tool("login",
	    "Logs a client in on its login screen with an account of the profile: the password goes from the profile to the client, never through the conversation. Returns at once; wait_for_state follows it "
	    "(character_selection), and a refusal comes as a message_box event (2102: the account is still online, try again in about 30 s).",
	    p, "account", false));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	tools.push(tool("kill_instance",
	    "Kills the process of an instance at once, without letting it save anything: the one whose endpoint runs under the name, or else the one process of the name that is still ending after its endpoint closed. A client method quits it properly.",
	    p, "instance", false));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	tools.push(tool("get_state",
	    "The state of an instance (for a client: login, shard_list, connecting, character_selection, entering_game, loading, in_game, far_tp, leaving_game, quitting...) with its detail and frame, and whether its main thread runs or has been busy for how long. Answered even while the main thread is stuck.",
	    p, "", true));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("state", prop("string", "The state to wait for."));
	p.set("states", stringsProp("Any of these states."));
	p.set("not", prop("string", "Wait until the state is no longer this one."));
	p.set("wait_seconds", rangeProp("number", "How long to wait at most (30 by default).", 0, s_MaxWaitSeconds));
	p.set("stop_on", stringsProp("Event kinds that end the wait early (message_box and exit by default; an empty list for none)."));
	p.set("settle_seconds",
	    rangeProp("number", "When the state left the one it had and stays this long in another settled state, the wait ends (5 by default, 0 never).", 0, s_MaxWaitSeconds));
	p.set("since_idx", rangeProp("integer", "The first event index that counts for stop_on (by default the events since the last call of this connection).", 0, 4294967295.0));
	tools.push(tool("wait_for_state",
	    "Waits until an instance reaches a state, or leaves one: give state, states or not. It does not wait out its time when the state cannot come: it ends with matched false and what stopped it when a message box opens or "
	    "the instance exits (stop_on; by default the events since the last call of this connection count, so a refusal the call itself brought ends the wait), or when the state went to another settled state and stays there (settle_seconds), such as a client back at the character selection instead of in the game. "
	    "Returns the state with matched false when the time ran out.",
	    p, "", true));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("prefix", prop("string", "Only names starting with this."));
	p.set("contains", prop("string", "Only entries whose name or help contains this (case-insensitive)."));
	p.set("limit", rangeProp("integer", "At most this many entries (300 by default).", 1, 4294967295.0));
	tools.push(tool("list_commands", "The command table of an instance: commands and variables with their argument syntax and help, handler instances and their member commands.", p, "", true));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("command", prop("string", "The command or variable, instance.command for a member command."));
	p.set("args", stringsProp("The arguments, one per entry; they reach the command as given."));
	p.set("wait_seconds", rangeProp("number", "How long to wait for the answer (60 by default).", 0, s_MaxWaitSeconds, true));
	tools.push(tool("run_command",
	    "Runs a command of an instance, or reads a variable by its name alone and sets it with one argument. Returns whether it succeeded, its output, and the system messages it showed meanwhile.",
	    p, "command", false));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("from_idx", rangeProp("integer", "First line index, inclusive; the oldest lines come first from there.", 0, 4294967295.0));
	p.set("to_idx", rangeProp("integer", "Last line index, inclusive.", 0, 4294967295.0));
	p.set("last", rangeProp("integer", "The last N lines of the selection (100 when nothing else selects).", 0, 4294967295.0));
	p.set("levels", stringsProp("Only these levels: INF, WRN, ERR, DBG, AST, STT."));
	p.set("contains", prop("string", "Only lines containing this (case-insensitive)."));
	p.set("max_lines", rangeProp("integer", "At most this many lines (200 by default, 5000 at most).", 1, 5000));
	p.set("wait_seconds", rangeProp("number", "When no line matches yet, wait up to this long for one (with from_idx: the next one).", 0, s_MaxWaitSeconds));
	tools.push(tool("get_log",
	    "Log lines of an instance: index, time, level, text, origin. Indices count from 1 since the process started and never change; continue from next_idx. With from_idx and wait_seconds it waits for the next matching line, so a log wording is waited for without sleeping.",
	    p, "", true));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("from_idx", rangeProp("integer", "First event index, inclusive.", 0, 4294967295.0));
	p.set("last", rangeProp("integer", "The last N events.", 0, 4294967295.0));
	p.set("kinds", stringsProp("Only these kinds (state, system_info, exit, ...)."));
	p.set("contains", prop("string", "Only events whose data contains this (case-insensitive)."));
	p.set("max", rangeProp("integer", "At most this many events (200 by default).", 1, 4294967295.0));
	p.set("wait_seconds", rangeProp("number", "When no event matches yet, wait up to this long for one.", 0, s_MaxWaitSeconds));
	tools.push(tool("get_events", "Events of an instance, indexed like the log: state changes, system messages, chat lines, windows opened and closed (a client), message boxes, and what the instance reports. With from_idx and wait_seconds it waits for the next one.", p, "",
	    true));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("prefix", prop("string", "Only methods whose name starts with this, such as 'ui.' or 'lua.'."));
	p.set("contains", prop("string", "Only methods whose name or description contains this."));
	p.set("schemas", prop("boolean", "Include the argument schemas."));
	tools.push(tool("list_methods", "The methods of an instance for call_method, with their description and whether they change something; the argument schemas when asked.", p, "", true));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("method", prop("string", "The method, such as lua.run."));
	p.set("args", prop("object", "The arguments of the method, as its schema says."));
	p.set("wait_seconds", rangeProp("number", "How long to wait for the answer (60 by default).", 0, s_MaxWaitSeconds, true));
	tools.push(tool("call_method", "Calls a method of an instance and returns its result, with images and other data it attaches.", p, "method", false));
	return tools;
}

// the tool list, built once
CJsonValue s_Tools;

const CJsonValue &getTools()
{
	if (s_Tools.isNull())
		s_Tools = toolList();
	return s_Tools;
}

const CJsonValue *findTool(const std::string &name)
{
	const CJsonValue &tools = getTools();
	for (uint i = 0; i < tools.size(); ++i)
		if (tools.at(i).get("name").getString() == name)
			return &tools.at(i);
	return nullptr;
}

/// Checks the arguments of a call against the schema of its tool: names, types, bounds, and what is required
bool checkArguments(const CJsonValue &tool, const CJsonValue &args, std::string &error)
{
	const std::string &toolName = tool.get("name").getString();
	const CJsonValue &schema = tool.get("inputSchema");
	const CJsonValue &properties = schema.get("properties");
	const CJsonValue::TMembers &given = args.members();
	for (uint i = 0; i < given.size(); ++i)
	{
		const std::string &key = given[i].first;
		const CJsonValue &value = given[i].second;
		const CJsonValue &property = properties.get(key);
		if (!property.isObject())
		{
			std::string names;
			for (uint k = 0; k < properties.members().size(); ++k)
				names += (k ? ", " : "") + properties.members()[k].first;
			error = "unknown argument '" + key + "' of " + toolName + "; it takes: " + (names.empty() ? std::string("none") : names);
			return false;
		}
		const std::string &type = property.get("type").getString();
		bool ok = true;
		std::string expected;
		if (type == "string")
		{
			ok = value.isString();
			expected = "a string";
		}
		else if (type == "integer")
		{
			ok = value.isNumber() && value.getNumber() == std::floor(value.getNumber());
			expected = "a whole number";
		}
		else if (type == "number")
		{
			ok = value.isNumber();
			expected = "a number";
		}
		else if (type == "boolean")
		{
			ok = value.isBool();
			expected = "true or false";
		}
		else if (type == "object")
		{
			ok = value.isObject();
			expected = "an object";
		}
		else if (type == "array")
		{
			ok = value.isArray();
			expected = "an array of strings";
			for (uint k = 0; ok && k < value.size(); ++k)
				ok = value.at(k).isString();
		}
		if (!ok)
		{
			error = "the argument '" + key + "' of " + toolName + " must be " + expected;
			return false;
		}
		if (value.isNumber())
		{
			double v = value.getNumber();
			if ((property.has("minimum") && v < property.get("minimum").getNumber()) || (property.has("exclusiveMinimum") && v <= property.get("exclusiveMinimum").getNumber())
			    || (property.has("maximum") && v > property.get("maximum").getNumber()))
			{
				std::string bounds = property.has("exclusiveMinimum")
				    ? "above " + property.get("exclusiveMinimum").toString() + " and at most " + property.get("maximum").toString()
				    : "from " + property.get("minimum").toString() + " to " + property.get("maximum").toString();
				error = "the argument '" + key + "' of " + toolName + " is " + value.toString() + ", it must be " + bounds;
				return false;
			}
		}
	}
	const CJsonValue &required = schema.get("required");
	for (uint i = 0; i < required.size(); ++i)
	{
		if (!args.has(required.at(i).getString()))
		{
			error = toolName + " needs the argument '" + required.at(i).getString() + "'";
			return false;
		}
	}
	return true;
}

/// The wait of a call in ms, from wait_seconds, whose bounds the schema checked
uint waitMs(const CJsonValue &args, double defaultSeconds)
{
	return (uint)(args.get("wait_seconds").getNumber(defaultSeconds) * 1000.0);
}

void listInstances(const CJsonValue &rpcId)
{
	std::vector<CInstance> instances;
	std::string error;
	if (!scanInstances(instances, error))
	{
		replyToolText(rpcId, error, true);
		return;
	}
	CJsonValue list = CJsonValue::array();
	for (uint i = 0; i < instances.size(); ++i)
	{
		const CInstance &instance = instances[i];
		CJsonValue e = CJsonValue::object();
		e.set("name", instance.Name);
		std::string state = instance.getState();
		if (!instance.Invalid.empty())
			state = "unusable: " + instance.Invalid;
		else if (state == "running" && instance.Process == ProcessGone)
			state = "gone without a reason (it crashed or was killed)";
		else if (state == "exited")
			state = "exited: " + instance.Record.get("exit_reason").getString();
		else if (state == "failed")
			state = "failed: " + instance.Record.get("error").getString();
		e.set("state", state);
		e.set("process", instance.Process == ProcessRuns ? "runs" : (instance.Process == ProcessGone ? "gone" : "cannot be inspected"));
		static const char *const keys[] = { "pid", "kind", "program", "cwd", "log", "version", "build", "account", "shard", "character" };
		for (uint k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k)
			if (instance.Record.has(keys[k]))
				e.set(keys[k], instance.Record.get(keys[k]));
		list.push(e);
		// the record of a process that is gone is shown once; one whose process cannot be inspected stays
		if (instance.Process == ProcessGone)
			CFile::deleteFile(instance.RecordPath);
	}
	replyToolText(rpcId, list.toString(), false);
}

void killInstance(const CJsonValue &rpcId, const std::string &name)
{
	std::vector<CInstance> instances;
	std::string error;
	if (!scanInstances(instances, error))
	{
		replyToolText(rpcId, error, true);
		return;
	}
	// the instance whose endpoint runs under the name, or else the one process of the name still ending after its
	// endpoint closed; never a process whose endpoint failed under the name, which belongs to another
	const CInstance *target = nullptr;
	std::vector<const CInstance *> ending;
	for (uint i = 0; i < instances.size(); ++i)
	{
		const CInstance &instance = instances[i];
		if (instance.Name != name || !instance.Alive)
			continue;
		if (instance.getState() == "running")
		{
			target = &instance;
			break;
		}
		if (instance.getState() == "exited")
			ending.push_back(&instance);
	}
	if (target == nullptr && ending.size() == 1)
		target = ending[0];
	if (target == nullptr)
	{
		if (ending.size() > 1)
		{
			std::string pids;
			for (uint i = 0; i < ending.size(); ++i)
				pids += (i ? ", " : "") + toString("%u", ending[i]->Pid);
			replyToolText(rpcId, "several processes of '" + name + "' are still ending (" + pids + "), none has its endpoint", true);
		}
		else
		{
			replyToolText(rpcId, "no running instance '" + name + "'", true);
		}
		return;
	}
#ifdef NL_OS_WINDOWS
	HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)target->Pid);
	bool ok = process != NULL && TerminateProcess(process, 1);
	int killError = ok ? 0 : getLastError();
	if (process != NULL)
		CloseHandle(process);
#else
	bool ok = kill((pid_t)target->Pid, SIGKILL) == 0;
	int killError = ok ? 0 : getLastError();
#endif
	if (ok)
		replyToolText(rpcId, toString("killed '%s' (process %u%s)", name.c_str(), target->Pid, target->getState() == "exited" ? ", which was ending" : ""), false);
	else
		replyToolText(rpcId, toString("cannot kill process %u: ", target->Pid) + formatErrorMessage(killError), true);
}

void callTool(const CJsonValue &rpcId, const CJsonValue &params)
{
	// what the protocol carries wrong is a protocol error; what the tool cannot do is a tool result
	const CJsonValue &nameValue = params.get("name");
	if (!nameValue.isString())
	{
		replyError(rpcId, -32602, "tools/call needs the name of a tool");
		return;
	}
	const std::string &name = nameValue.getString();
	const CJsonValue *tool = findTool(name);
	if (tool == nullptr)
	{
		replyError(rpcId, -32602, "unknown tool: " + name);
		return;
	}
	const CJsonValue &args = params.get("arguments");
	if (!args.isNull() && !args.isObject())
	{
		replyError(rpcId, -32602, "the arguments of " + name + " must be an object");
		return;
	}
	std::string error;
	if (!checkArguments(*tool, args, error))
	{
		replyError(rpcId, -32602, error);
		return;
	}

	std::string instance = args.get("instance").getString();
	bool ok = true;
	if (name == "list_instances")
	{
		listInstances(rpcId);
		return;
	}
	if (name == "launch")
	{
		startLaunch(rpcId, args);
		return;
	}
	if (name == "kill_instance")
	{
		killInstance(rpcId, instance);
		return;
	}
	if (name == "login")
	{
		CJsonValue profile;
		if (!readProfile(profile, error))
		{
			replyToolText(rpcId, error, true);
			return;
		}
		const std::string &accountName = args.get("account").getString();
		const CJsonValue &account = profile.get("accounts").get(accountName);
		if (!account.isObject())
		{
			replyToolText(rpcId, "no account '" + accountName + "' in the profile '" + s_ProfilePath + "'", true);
			return;
		}
		if (!account.get("login").isString() || account.get("login").getString().empty() || !account.get("password").isString())
		{
			replyToolText(rpcId, "the account '" + accountName + "' of the profile '" + s_ProfilePath + "' needs a \"login\" and a \"password\"", true);
			return;
		}
		CJsonValue a = CJsonValue::object();
		a.set("login", account.get("login"));
		a.set("password", account.get("password"));
		ok = startCall(rpcId, instance, "login.login", a, FormatJson, 30000, error);
		if (!ok)
			replyToolText(rpcId, error, true);
		return;
	}
	if (name == "get_state")
	{
		ok = startCall(rpcId, instance, "ctrl.state", CJsonValue::object(), FormatJson, 10000, error);
	}
	else if (name == "wait_for_state")
	{
		if (!args.has("state") && !args.has("states") && !args.has("not"))
		{
			replyToolText(rpcId, "wait_for_state needs what to wait for: state, states or not", true);
			return;
		}
		CJsonValue a = CJsonValue::object();
		static const char *const keys[] = { "state", "states", "not", "stop_on", "settle_seconds", "since_idx" };
		for (uint k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k)
			if (args.has(keys[k]))
				a.set(keys[k], args.get(keys[k]));
		uint ms = waitMs(args, 30);
		a.set("timeout_ms", ms);
		ok = startCall(rpcId, instance, "ctrl.wait_state", a, FormatJson, ms + 10000, error);
	}
	else if (name == "list_commands")
	{
		CJsonValue a = CJsonValue::object();
		static const char *const keys[] = { "prefix", "contains", "limit" };
		for (uint k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k)
			if (args.has(keys[k]))
				a.set(keys[k], args.get(keys[k]));
		ok = startCall(rpcId, instance, "ctrl.commands", a, FormatJson, 30000, error);
	}
	else if (name == "run_command")
	{
		CJsonValue a = CJsonValue::object();
		a.set("command", args.get("command"));
		a.set("args", args.get("args").isArray() ? args.get("args") : CJsonValue::array());
		ok = startCall(rpcId, instance, "ctrl.run_command", a, FormatCommand, waitMs(args, 60), error);
	}
	else if (name == "get_log" || name == "get_events")
	{
		CJsonValue a = CJsonValue::object();
		static const char *const keys[] = { "from_idx", "to_idx", "last", "levels", "kinds", "contains" };
		for (uint k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k)
			if (args.has(keys[k]))
				a.set(keys[k], args.get(keys[k]));
		if (args.has("max_lines"))
			a.set("max", args.get("max_lines"));
		if (args.has("max"))
			a.set("max", args.get("max"));
		uint ms = waitMs(args, 0);
		if (ms > 0)
			a.set("wait_ms", ms);
		ok = startCall(rpcId, instance, name == "get_log" ? "ctrl.log" : "ctrl.events", a, name == "get_log" ? FormatLog : FormatEvents, ms + 10000, error);
	}
	else if (name == "list_methods")
	{
		CJsonValue a = CJsonValue::object();
		static const char *const keys[] = { "prefix", "contains", "schemas" };
		for (uint k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k)
			if (args.has(keys[k]))
				a.set(keys[k], args.get(keys[k]));
		ok = startCall(rpcId, instance, "ctrl.methods", a, FormatJson, 10000, error);
	}
	else if (name == "call_method")
	{
		const std::string &method = args.get("method").getString();
		if (method.empty())
		{
			replyToolText(rpcId, "method is required", true);
			return;
		}
		TFormat format = method == "ctrl.log" ? FormatLog : (method == "ctrl.events" ? FormatEvents : (method == "ctrl.run_command" ? FormatCommand : FormatJson));
		ok = startCall(rpcId, instance, method, args.get("args").isObject() ? args.get("args") : CJsonValue::object(), format, waitMs(args, 60), error);
	}
	else
	{
		// a tool of the list without a branch here
		replyError(rpcId, -32603, "the tool " + name + " is listed but not handled by this server");
		return;
	}
	if (!ok)
		replyToolText(rpcId, error, true);
}

/// The MCP client gave up on a request: no answer goes out, and the instance drops the call
void cancelRequest(const CJsonValue &rpcId)
{
	std::string id = rpcId.toString();
	for (std::list<COp>::iterator op = s_Ops.begin(); op != s_Ops.end(); ++op)
	{
		if (op->RpcId.toString() != id)
			continue;
		if (op->Launch)
		{
			// the process started already; it keeps running, as one started by hand would
			closeLaunch(*op);
		}
		else
		{
			std::map<std::string, CLink *>::iterator link = s_Links.find(op->Instance);
			if (link != s_Links.end() && link->second->Serial == op->LinkSerial)
			{
				std::vector<std::string> fields;
				fields.push_back("cancel");
				fields.push_back(op->CallId);
				sendLine(*link->second, joinCtrlLine(fields));
			}
		}
		s_Ops.erase(op);
		return;
	}
}

void handleRpc(const std::string &line)
{
	CJsonValue message;
	std::string error;
	if (!message.parse(line, error))
	{
		replyError(CJsonValue(), -32700, "parse error: " + error);
		return;
	}
	if (message.isArray())
	{
		// the answers of this server go out one by one as their calls end, so a batch cannot have its one answer;
		// MCP has no batches since 2025-06-18
		replyError(CJsonValue(), -32600, "invalid request: a batch (a JSON array) is not taken; send one request per line");
		return;
	}
	if (!message.isObject())
	{
		replyError(CJsonValue(), -32600, "invalid request: a message is a JSON object");
		return;
	}
	const CJsonValue &methodValue = message.get("method");
	if (!methodValue.isString())
	{
		// an answer to a request of this server, which sends none
		if (message.has("result") || message.has("error"))
			return;
		replyError(message.get("id"), -32600, "invalid request: no method");
		return;
	}
	const std::string &method = methodValue.getString();
	if (method == "notifications/cancelled")
	{
		cancelRequest(message.get("params").get("requestId"));
		return;
	}
	// a notification has no answer
	if (!message.has("id"))
		return;
	const CJsonValue &id = message.get("id");
	const CJsonValue &params = message.get("params");
	if (method == "initialize")
	{
		const std::string &requested = params.get("protocolVersion").getString();
		std::string version = s_McpVersions[0];
		for (uint i = 0; i < sizeof(s_McpVersions) / sizeof(s_McpVersions[0]); ++i)
			if (requested == s_McpVersions[i])
				version = requested;
		CJsonValue capabilities = CJsonValue::object();
		capabilities.set("tools", CJsonValue::object().set("listChanged", false));
		CJsonValue info = CJsonValue::object();
		info.set("name", s_ServerName);
		info.set("version", s_ServerVersion);
		CJsonValue r = CJsonValue::object();
		r.set("protocolVersion", version);
		r.set("capabilities", capabilities);
		r.set("serverInfo", info);
		r.set("instructions", s_Instructions);
		reply(id, r);
	}
	else if (method == "ping")
	{
		reply(id, CJsonValue::object());
	}
	else if (method == "tools/list")
	{
		reply(id, CJsonValue::object().set("tools", getTools()));
	}
	else if (method == "tools/call")
	{
		callTool(id, params);
	}
	else if (method == "resources/list")
	{
		reply(id, CJsonValue::object().set("resources", CJsonValue::array()));
	}
	else if (method == "resources/templates/list")
	{
		reply(id, CJsonValue::object().set("resourceTemplates", CJsonValue::array()));
	}
	else if (method == "prompts/list")
	{
		reply(id, CJsonValue::object().set("prompts", CJsonValue::array()));
	}
	else
	{
		replyError(id, -32601, "method not found: " + method);
	}
}

/// Reads what the links have, and completes the operations it answers
bool pumpLinks()
{
	bool busy = false;
#ifndef NL_OS_WINDOWS
	// the processes launched earlier that have ended since
	reapChildren();
	// an exit code that no launch waits for is not kept
	for (std::map<uint32, sint32>::iterator it = s_ExitCodes.begin(); it != s_ExitCodes.end();)
	{
		bool wanted = false;
		for (std::list<COp>::const_iterator op = s_Ops.begin(); op != s_Ops.end() && !wanted; ++op)
			wanted = op->Launch && op->LaunchPid == it->first;
		if (wanted)
			++it;
		else
			s_ExitCodes.erase(it++);
	}
#endif
	TTime now = CTime::getLocalTime();
	for (std::map<std::string, CLink *>::iterator it = s_Links.begin(); it != s_Links.end(); ++it)
	{
		CLink &link = *it->second;
		CCtrlMessage message;
		while (readLink(link, message))
		{
			busy = true;
			if (!link.Greeted)
			{
				if (!greet(link, message))
					break;
				continue;
			}
			if (message.IsData)
			{
				// data <call>.<n> <type> <length>
				CAttachment a;
				a.Id = message.Fields[1];
				a.Type = message.Fields[2];
				a.Bytes.swap(message.Data);
				std::string call = a.Id.substr(0, a.Id.find('.'));
				link.Data[call].push_back(a);
				continue;
			}
			if (message.Fields.size() < 2 || (message.Fields[0] != "result" && message.Fields[0] != "error"))
				continue;
			bool matched = false;
			for (std::list<COp>::iterator op = s_Ops.begin(); op != s_Ops.end(); ++op)
			{
				if (!op->Launch && op->LinkSerial == link.Serial && op->CallId == message.Fields[1])
				{
					finishOp(*op, link, message);
					s_Ops.erase(op);
					matched = true;
					break;
				}
			}
			// the answer to a call given up on: its data goes with it
			if (!matched)
				link.Data.erase(message.Fields[1]);
		}
		if (!link.Greeted && link.Pipe.isOpen() && now >= link.GreetDeadline)
		{
			failLink(link, toString("'%s' did not greet within %u ms: its process may be stopped in a debugger or hung", link.Name.c_str(), s_HelloTimeoutMs));
			busy = true;
		}
	}
	// the operations whose instance went away or that took too long
	for (std::list<COp>::iterator op = s_Ops.begin(); op != s_Ops.end();)
	{
		bool done = false;
		if (op->Launch)
		{
			// the registry is read a few times a second, not at every turn
			if (now >= op->NextCheck)
			{
				op->NextCheck = now + s_LaunchCheckMs;
				done = checkLaunch(*op);
				if (done)
					closeLaunch(*op);
			}
		}
		else
		{
			std::map<std::string, CLink *>::iterator link = s_Links.find(op->Instance);
			if (link == s_Links.end() || link->second->Serial != op->LinkSerial)
			{
				replyToolText(op->RpcId, "'" + op->Instance + "' closed the connection before it answered; list_instances says why", true);
				done = true;
			}
			else if (!link->second->Pipe.isOpen())
			{
				replyToolText(op->RpcId, closedReason(*link->second), true);
				done = true;
			}
			else if (now >= op->Deadline)
			{
				std::vector<std::string> fields;
				fields.push_back("cancel");
				fields.push_back(op->CallId);
				sendLine(*link->second, joinCtrlLine(fields));
				replyToolText(op->RpcId,
				    toString("'%s' did not answer within %u ms, and a cancel was sent: a call that has not run is dropped and one that waits stops, but what it did already "
				             "stays done (wait_seconds gives it longer)",
				        op->Instance.c_str(), op->TimeoutMs),
				    true);
				done = true;
			}
		}
		if (done)
		{
			op = s_Ops.erase(op);
			busy = true;
		}
		else
		{
			++op;
		}
	}
	return busy;
}

/// The warnings and errors of this server, on stderr: stdout is the MCP channel
class CStderrDisplayer : public IDisplayer
{
public:
	CStderrDisplayer()
	    : IDisplayer("STDERR")
	{
	}

protected:
	virtual void doDisplay(const CLog::TDisplayInfo &args, const char *message) NL_OVERRIDE
	{
		fprintf(stderr, "nel_mcp %s: %s", logTypeToString(args.LogType), message);
		fflush(stderr);
	}
};

} // anonymous namespace

int main(int argc, char **argv)
{
	CApplicationContext appContext;

	// the arguments first: the log folder is where the debug setup writes
	std::string logDir;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg.compare(0, 10, "--profile=") == 0)
		{
			s_ProfilePath = arg.substr(10);
		}
		else if (arg.compare(0, 10, "--log-dir=") == 0)
		{
			logDir = arg.substr(10);
		}
		else if (arg == "--help" || arg == "-h")
		{
			fprintf(stderr,
			    "nel_mcp [--profile=<file>] [--log-dir=<folder>]: an MCP server over stdio for the control endpoints of NeL processes on this machine\n"
			    "  --profile  the programs launch may start and the accounts they log in with (JSON)\n"
			    "  --log-dir  the folder of the log of this server (log.log) and its crash reports; without it there is no log file, and a crash report goes to the working directory\n");
			return 0;
		}
		else
		{
			fprintf(stderr, "nel_mcp: unknown argument '%s'\n", arg.c_str());
			return 1;
		}
	}
	if (!logDir.empty() && !CFile::isDirectory(logDir))
	{
		fprintf(stderr, "nel_mcp: the log folder '%s' (--log-dir) is not a folder: create it, or give another\n", logDir.c_str());
		return 1;
	}

	createDebug(logDir.empty() ? nullptr : logDir.c_str(), !logDir.empty());
	CStderrDisplayer stderrDisplayer;
	CLog *logs[] = { INelContext::getInstance().getInfoLog(), INelContext::getInstance().getDebugLog(), INelContext::getInstance().getWarningLog(),
		INelContext::getInstance().getErrorLog(), INelContext::getInstance().getAssertLog() };
	for (uint i = 0; i < sizeof(logs) / sizeof(logs[0]); ++i)
	{
		logs[i]->removeDisplayer("DEFAULT_SD");
		if (i >= 2)
			logs[i]->addDisplayer(&stderrDisplayer);
	}

#ifdef NL_OS_WINDOWS
	// newline framed JSON; no text mode translation either way
	_setmode(_fileno(stdout), _O_BINARY);
	_setmode(_fileno(stdin), _O_BINARY);
#endif

	CStdinReader reader;
	IThread *thread = IThread::create(&reader);
	thread->start();

	for (;;)
	{
		// read before the lines are taken: the lines that came before the end of stdin are all handled
		bool closed = reader.Closed;
		TLines lines;
		bool busy = reader.take(lines);
		for (TLines::iterator it = lines.begin(); it != lines.end(); ++it)
			if (!it->empty())
				handleRpc(*it);
		// the end of stdin ends the server once every request taken has its answer (each has a deadline), or at once
		// when no answer can be written any more
		if (closed && lines.empty() && (s_Ops.empty() || s_OutputClosed))
			break;
		busy = pumpLinks() || busy;
		if (!busy)
			nlSleep(2);
	}

	// the instances stay
	for (std::list<COp>::iterator op = s_Ops.begin(); op != s_Ops.end(); ++op)
		closeLaunch(*op);
	s_Ops.clear();
	for (std::map<std::string, CLink *>::iterator it = s_Links.begin(); it != s_Links.end(); ++it)
		delete it->second;
	s_Links.clear();
	thread->wait();
	delete thread;
	for (uint i = 2; i < sizeof(logs) / sizeof(logs[0]); ++i)
		logs[i]->removeDisplayer(&stderrDisplayer);
	return 0;
}

/* End of nel_mcp.cpp */
