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
// instances, their state, commands, log, events and methods, and nothing of what the process is.
// Usage: nel_mcp [--profile=<file>]
// The profile is a JSON file with the programs it may launch and the accounts they log in with; there
// is no default one. It is read when a tool needs it, so it can change while the server runs.

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

// *** output ***

void writeMessage(const CJsonValue &message)
{
	std::string text = message.toString();
	text += '\n';
	fwrite(text.data(), 1, text.size(), stdout);
	fflush(stdout);
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
			CSynchronized<std::deque<std::string>>::CAccessor access(&Lines);
			access.value().push_back(line);
		}
		Closed = true;
	}

	bool take(std::deque<std::string> &out)
	{
		CSynchronized<std::deque<std::string>>::CAccessor access(&Lines);
		out.swap(access.value());
		return !out.empty();
	}

	CSynchronized<std::deque<std::string>> Lines;
	volatile bool Closed;
};

// *** instances ***

struct CInstance
{
	std::string Name;
	std::string RecordPath;
	CJsonValue Record;
	uint32 Pid;
	uint64 StartTime;
	bool Alive;
};

/// The records of the registry, with whether their process still runs
bool scanInstances(std::vector<CInstance> &out, std::string &error)
{
	out.clear();
	std::string dir;
	if (!CCtrlRegistry::getDirectory(dir, true, error))
		return false;
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
		instance.Alive = instance.Pid != 0 && instance.StartTime != 0 && CCtrlRegistry::getProcessStartTime(instance.Pid) == instance.StartTime;
		out.push_back(instance);
	}
	return true;
}

// *** links to endpoints ***

struct CAttachment
{
	std::string Id;
	std::string Type;
	std::string Bytes;
};

class CLink
{
public:
	CLink()
	    : Reader(256 * 1024 * 1024, 256 * 1024 * 1024)
	    , Pid(0)
	    , StartTime(0)
	    , NextCall(1)
	{
	}

	std::string Name;
	CLocalPipe Pipe;
	CCtrlMessageReader Reader;
	uint32 Pid;
	uint64 StartTime;
	CJsonValue Hello;
	uint32 NextCall;
	// data frames by call id, until the answer of the call comes
	std::map<std::string, std::vector<CAttachment>> Data;
};

std::map<std::string, CLink *> s_Links;

/// One message of a link, or false when there is none yet; the link is closed on a protocol error
bool readLink(CLink &link, CCtrlMessage &message)
{
	std::string error;
	if (link.Reader.next(message, error))
		return true;
	if (!error.empty())
	{
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
		link.Pipe.close();
		link.Reader.clear();
	}
	return false;
}

/// The link to a running instance, connected and greeted
CLink *getLink(const CInstance &instance, std::string &error)
{
	std::map<std::string, CLink *>::iterator it = s_Links.find(instance.Name);
	if (it != s_Links.end())
	{
		CLink *link = it->second;
		if (link->Pipe.isOpen() && link->Pid == instance.Pid && link->StartTime == instance.StartTime)
			return link;
		delete link;
		s_Links.erase(it);
	}
	const std::string &path = instance.Record.get("endpoint").getString();
	CLink *link = new CLink();
	link->Name = instance.Name;
	link->Pid = instance.Pid;
	link->StartTime = instance.StartTime;
	if (!link->Pipe.connect(path, error))
	{
		error = toString("cannot connect to '%s' at '%s': %s", instance.Name.c_str(), path.c_str(), error.c_str());
		delete link;
		return nullptr;
	}
	// the greeting first: the version tells whether we speak the same protocol
	TTime deadline = CTime::getLocalTime() + s_HelloTimeoutMs;
	CCtrlMessage message;
	bool greeted = false;
	while (CTime::getLocalTime() < deadline && link->Pipe.isOpen())
	{
		if (readLink(*link, message))
		{
			if (!message.IsData && message.Fields.size() >= 3 && message.Fields[0] == "hello")
			{
				uint version = 0;
				fromString(message.Fields[1], version);
				if (version > CtrlProtocolVersion)
				{
					error = toString("'%s' speaks version %u of the control protocol, this server version %u: use a newer nel_mcp", instance.Name.c_str(), version,
					    CtrlProtocolVersion);
					delete link;
					return nullptr;
				}
				std::string parseError;
				link->Hello.parse(message.Fields[2], parseError);
				greeted = true;
				break;
			}
			continue;
		}
		nlSleep(2);
	}
	if (!greeted)
	{
		error = toString("'%s' did not greet within %u ms", instance.Name.c_str(), s_HelloTimeoutMs);
		delete link;
		return nullptr;
	}
	std::vector<std::string> fields;
	fields.push_back("hello");
	fields.push_back(toString("%u", CtrlProtocolVersion));
	fields.push_back(s_ServerName);
	std::string line = joinCtrlLine(fields) + "\n";
	link->Pipe.write(line, s_WriteTimeoutMs);
	s_Links[instance.Name] = link;
	return link;
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
	std::string CallId;
	TFormat Format;
	TTime Deadline;
	bool Launch;
	uint32 LaunchPid;
	std::string LaunchName;
	uint64 LaunchStart;
	TTime NextCheck;
#ifdef NL_OS_WINDOWS
	void *LaunchProcess;
#endif

	COp()
	    : Format(FormatJson)
	    , Deadline(0)
	    , Launch(false)
	    , LaunchPid(0)
	    , LaunchStart(0)
	    , NextCheck(0)
#ifdef NL_OS_WINDOWS
	    , LaunchProcess(nullptr)
#endif
	{
	}
};

std::list<COp> s_Ops;
std::string s_ProfilePath;

std::string renderLogLines(const CJsonValue &result)
{
	std::string text;
	const CJsonValue &lines = result.get("lines");
	for (uint i = 0; i < lines.size(); ++i)
	{
		const CJsonValue &l = lines.at(i);
		text += toString("%u %s %s %s", (uint)l.get("idx").getInt(0), formatTime((uint64)l.get("time").getInt(0)).c_str(), l.get("level").getString().c_str(),
		    l.get("text").getString().c_str());
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
		text += toString("%u %s %s %s\n", (uint)e.get("idx").getInt(0), formatTime((uint64)e.get("time").getInt(0)).c_str(), e.get("kind").getString().c_str(),
		    e.get("data").toString().c_str());
	}
	text += toString("-- %u events; next_idx %u, first held %u, newest %u%s", events.size(), (uint)result.get("next_idx").getInt(0), (uint)result.get("first_idx").getInt(0),
	    (uint)result.get("newest_idx").getInt(0), result.get("truncated").getBool() ? ", truncated" : "");
	if (result.has("missed"))
		text += "\n-- " + result.get("missed").getString();
	return text;
}

std::string renderCommand(const CJsonValue &result)
{
	std::string text = toString("%s: %s\n", result.get("command").getString().c_str(), result.get("ok").getBool() ? "ok" : "failed");
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
	CJsonValue wrapper;
	std::string parseError;
	std::string json = isError ? (message.Fields.size() >= 4 ? message.Fields[3] : std::string()) : (message.Fields.size() >= 3 ? message.Fields[2] : std::string());
	wrapper.parse(json, parseError);
	const CJsonValue &result = wrapper.get("result");
	std::string header = toString("[%s frame %u, %s, %s]", op.Instance.c_str(), (uint)wrapper.get("frame").getInt(0), formatTime((uint64)wrapper.get("time").getInt(0)).c_str(),
	    wrapper.get("state").getString().c_str());

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
	std::map<std::string, std::vector<CAttachment>>::iterator it = link.Data.find(op.CallId);
	if (it != link.Data.end())
	{
		for (uint i = 0; i < it->second.size(); ++i)
		{
			const CAttachment &a = it->second[i];
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
				content.push(textContent(toString("[%s, %s]\n", a.Id.c_str(), a.Type.c_str()) + a.Bytes));
			}
			else
			{
				content.push(textContent(toString("[%s: %u bytes of %s, not shown]", a.Id.c_str(), (uint)a.Bytes.size(), a.Type.c_str())));
			}
		}
		link.Data.erase(it);
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
	std::string names;
	uint alive = 0;
	for (uint i = 0; i < instances.size(); ++i)
	{
		if (!instances[i].Alive || instances[i].Record.get("state").getString() != "running")
			continue;
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
		error = instanceName.empty() ? std::string("no instance runs; launch one, or start a client with --ctrl-listen=<name>")
		                             : toString("no running instance '%s'; running: %s", instanceName.c_str(), names.empty() ? "none" : names.c_str());
		return false;
	}
	CLink *link = getLink(*instance, error);
	if (link == nullptr)
		return false;
	std::string callId = toString("%u", link->NextCall++);
	std::vector<std::string> fields;
	fields.push_back("call");
	fields.push_back(callId);
	fields.push_back(method);
	fields.push_back(args.isNull() ? std::string("{}") : args.toString());
	if (!link->Pipe.write(joinCtrlLine(fields) + "\n", s_WriteTimeoutMs))
	{
		error = toString("'%s' does not take the call: the connection closed", instance->Name.c_str());
		return false;
	}
	COp op;
	op.RpcId = rpcId;
	op.Instance = instance->Name;
	op.CallId = callId;
	op.Format = format;
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
		error = toString("cannot read the profile '%s'", s_ProfilePath.c_str());
		return false;
	}
	std::string text;
	char buffer[4096];
	size_t n;
	while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0)
		text.append(buffer, n);
	fclose(f);
	if (!profile.parse(text, error) || !profile.isObject())
	{
		error = toString("the profile '%s' is not a JSON object: %s", s_ProfilePath.c_str(), error.c_str());
		return false;
	}
	return true;
}

std::string expandVariables(const std::string &value)
{
	// ${NAME} is the variable of this process
	std::string result;
	std::string::size_type pos = 0;
	for (;;)
	{
		std::string::size_type start = value.find("${", pos);
		if (start == std::string::npos)
		{
			result += value.substr(pos);
			return result;
		}
		std::string::size_type end = value.find('}', start);
		if (end == std::string::npos)
		{
			result += value.substr(pos);
			return result;
		}
		result += value.substr(pos, start - pos);
		std::string name = value.substr(start + 2, end - start - 2);
#ifdef NL_OS_WINDOWS
		const wchar_t *v = _wgetenv(nlUtf8ToWide(name));
		if (v != nullptr)
			result += wideToUtf8(v);
#else
		const char *v = getenv(name.c_str());
		if (v != nullptr)
			result += v;
#endif
		pos = end + 1;
	}
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
#endif

#ifndef NL_OS_WINDOWS
// the exit codes of the processes this server started, reaped as they end so none stays a zombie
std::map<uint32, sint32> s_ExitCodes;
#endif

/// Starts a process detached from our stdio; its pid, or 0 with the error
uint32 startProcess(const std::string &program, const std::vector<std::string> &args, const std::string &cwd, const CJsonValue &env, void **processHandle, std::string &error)
{
#ifdef NL_OS_WINDOWS
	std::wstring commandLine = quoteArgument(utf8ToWide(program));
	for (uint i = 0; i < args.size(); ++i)
		commandLine += L" " + quoteArgument(utf8ToWide(args[i]));

	// the environment of this process, with the profile's variables over it
	std::vector<wchar_t> block;
	const CJsonValue::TMembers &members = env.members();
	if (!members.empty())
	{
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
			std::wstring upper = key;
			for (uint k = 0; k < upper.size(); ++k)
				upper[k] = towupper(upper[k]);
			vars[upper] = entry.substr(eq + 1);
			names[upper] = key;
		}
		FreeEnvironmentStringsW(current);
		for (uint i = 0; i < members.size(); ++i)
		{
			std::wstring key = utf8ToWide(members[i].first);
			std::wstring upper = key;
			for (uint k = 0; k < upper.size(); ++k)
				upper[k] = towupper(upper[k]);
			vars[upper] = utf8ToWide(expandVariables(members[i].second.getString()));
			if (names.find(upper) == names.end())
				names[upper] = key;
		}
		for (std::map<std::wstring, std::wstring>::iterator it = vars.begin(); it != vars.end(); ++it)
		{
			std::wstring entry = names[it->first] + L"=" + it->second;
			block.insert(block.end(), entry.begin(), entry.end());
			block.push_back(0);
		}
		block.push_back(0);
	}

	STARTUPINFOW si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	std::vector<wchar_t> line(commandLine.begin(), commandLine.end());
	line.push_back(0);
	std::wstring wcwd = utf8ToWide(cwd);
	// no inherited handles: the stdio of this server is the MCP channel
	if (!CreateProcessW(nullptr, &line[0], nullptr, nullptr, FALSE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_PROCESS_GROUP, block.empty() ? nullptr : &block[0],
	        cwd.empty() ? nullptr : wcwd.c_str(), &si, &pi))
	{
		error = toString("cannot start '%s': %s", program.c_str(), formatErrorMessage(getLastError()).c_str());
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
	const CJsonValue::TMembers &members = env.members();
	for (char **e = environ; *e != nullptr; ++e)
	{
		std::string entry = *e;
		std::string key = entry.substr(0, entry.find('='));
		bool replaced = false;
		for (uint i = 0; i < members.size() && !replaced; ++i)
			replaced = members[i].first == key;
		if (!replaced)
			envStrings.push_back(entry);
	}
	for (uint i = 0; i < members.size(); ++i)
		envStrings.push_back(members[i].first + "=" + expandVariables(members[i].second.getString()));
	std::vector<char *> envp;
	for (uint i = 0; i < envStrings.size(); ++i)
		envp.push_back(const_cast<char *>(envStrings[i].c_str()));
	envp.push_back(nullptr);
	std::vector<char *> argv;
	argv.push_back(const_cast<char *>(program.c_str()));
	for (uint i = 0; i < args.size(); ++i)
		argv.push_back(const_cast<char *>(args[i].c_str()));
	argv.push_back(nullptr);

	pid_t pid = fork();
	if (pid < 0)
	{
		error = toString("cannot start '%s': %s", program.c_str(), formatErrorMessage(getLastError()).c_str());
		return 0;
	}
	if (pid == 0)
	{
		setsid();
		int devnull = open("/dev/null", O_RDWR);
		if (devnull >= 0)
		{
			dup2(devnull, 0);
			dup2(devnull, 1);
			dup2(devnull, 2);
		}
		if (!cwd.empty() && chdir(cwd.c_str()) != 0)
			_exit(127);
		execve(program.c_str(), &argv[0], &envp[0]);
		_exit(127);
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
#endif
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
		replyToolText(rpcId, toString("no program '%s' in the profile; it has: %s", programName.c_str(), names.empty() ? "none" : names.c_str()), true);
		return;
	}
	const std::string &name = args.get("name").getString();
	if (!CCtrlRegistry::isValidName(name))
	{
		replyToolText(rpcId, "name: letters, digits, '_', '-' and '.', 64 at most", true);
		return;
	}
	std::vector<CInstance> instances;
	if (!scanInstances(instances, error))
	{
		replyToolText(rpcId, error, true);
		return;
	}
	for (uint i = 0; i < instances.size(); ++i)
	{
		if (instances[i].Alive && instances[i].Name == name)
		{
			replyToolText(rpcId, toString("an instance '%s' runs already (process %u)", name.c_str(), instances[i].Pid), true);
			return;
		}
	}

	std::vector<std::string> argv;
	const CJsonValue &fixed = program.get("args");
	for (uint i = 0; i < fixed.size(); ++i)
		argv.push_back(fixed.at(i).getString());
	// an account goes first, as the client reads it: login password [shard] [slot]
	const std::string &accountName = args.get("account").getString();
	if (!accountName.empty())
	{
		const CJsonValue &account = profile.get("accounts").get(accountName);
		if (!account.isObject())
		{
			replyToolText(rpcId, toString("no account '%s' in the profile", accountName.c_str()), true);
			return;
		}
		argv.insert(argv.begin(), account.get("login").getString());
		argv.insert(argv.begin() + 1, account.get("password").getString());
		uint pos = 2;
		std::string shard = account.get("shard").isNumber() ? toString("%d", (sint)account.get("shard").getInt(0)) : account.get("shard").getString();
		if (!shard.empty())
			argv.insert(argv.begin() + pos++, shard);
		if (args.has("slot"))
			argv.insert(argv.begin() + pos++, toString("%d", (sint)args.get("slot").getInt(0)));
	}
	const CJsonValue &extra = args.get("args");
	for (uint i = 0; i < extra.size(); ++i)
		argv.push_back(extra.at(i).getString());
	argv.push_back("--ctrl-listen=" + name);

	COp op;
	op.RpcId = rpcId;
	op.Launch = true;
	op.LaunchName = name;
	void *handle = nullptr;
	op.LaunchPid = startProcess(expandVariables(program.get("program").getString()), argv, expandVariables(program.get("cwd").getString()), program.get("env"), &handle, error);
	if (op.LaunchPid == 0)
	{
		replyToolText(rpcId, error, true);
		return;
	}
#ifdef NL_OS_WINDOWS
	op.LaunchProcess = handle;
#endif
	sint64 waitSeconds = args.get("wait_seconds").getInt(60);
	op.Deadline = CTime::getLocalTime() + (TTime)std::max((sint64)1, std::min(waitSeconds, (sint64)600)) * 1000;
	s_Ops.push_back(op);
}

/// A launch is answered when the endpoint of its process is up, or the process ended, or time ran out
bool checkLaunch(COp &op)
{
	std::vector<CInstance> instances;
	std::string error;
	scanInstances(instances, error);
	for (uint i = 0; i < instances.size(); ++i)
	{
		const CInstance &instance = instances[i];
		if (instance.Pid != op.LaunchPid || instance.Name != op.LaunchName)
			continue;
		const std::string &state = instance.Record.get("state").getString();
		if (state == "failed")
		{
			replyToolText(op.RpcId, toString("process %u started, but its endpoint did not: %s", op.LaunchPid, instance.Record.get("error").getString().c_str()), true);
			return true;
		}
		if (state == "running")
		{
			CLink *link = getLink(instance, error);
			if (link == nullptr)
				return false;
			CJsonValue r = CJsonValue::object();
			r.set("instance", op.LaunchName);
			r.set("pid", op.LaunchPid);
			r.set("identity", link->Hello);
			replyToolText(op.RpcId, r.toString(), false);
			return true;
		}
	}
	sint32 exitCode = 0;
	if (!processRunning(op, exitCode))
	{
		replyToolText(op.RpcId, toString("process %u exited with code %d before its endpoint came up; see its log", op.LaunchPid, exitCode), true);
		return true;
	}
	if (CTime::getLocalTime() >= op.Deadline)
	{
		replyToolText(op.RpcId, toString("process %u runs, but its endpoint did not come up in time; it is still starting, or it was built without the endpoint (a final build)", op.LaunchPid),
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
	    "The instances on this machine: name, state (running, exited with its reason, gone without one after a crash, failed to start its endpoint), process id, program, kind, log file, and what it reports (account, character). Records of processes that are gone are shown once, then removed.",
	    CJsonValue(), "", true));
	CJsonValue p = CJsonValue::object();
	p.set("program", prop("string", "The program entry of the profile."));
	p.set("name", prop("string", "The instance name to give it: letters, digits, '_', '-', '.'."));
	p.set("account", prop("string", "An account entry of the profile to log in with; its password is on the command line of the process, so only for throwaway development accounts."));
	p.set("slot", prop("integer", "The character slot to play, with an account."));
	p.set("args", stringsProp("More arguments for the program."));
	p.set("wait_seconds", prop("integer", "How long to wait for the endpoint (60 by default)."));
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
	tools.push(tool("kill_instance", "Kills the process of an instance at once, without letting it save anything. A client method quits it properly.", p, "instance", false));
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
	p.set("wait_seconds", prop("number", "How long to wait at most (30 by default, 600 at most)."));
	p.set("stop_on", stringsProp("Event kinds that end the wait early (message_box and exit by default; an empty list for none)."));
	p.set("settle_seconds", prop("number", "When the state left the one it had and stays this long in another settled state, the wait ends (5 by default, 0 never)."));
	p.set("since_idx", prop("integer", "The first event index that counts for stop_on (by default the events since the last call of this connection)."));
	tools.push(tool("wait_for_state",
	    "Waits until an instance reaches a state, or leaves one. It does not wait out its time when the state cannot come: it ends with matched false and what stopped it when a message box opens or "
	    "the instance exits (stop_on; by default the events since the last call of this connection count, so a refusal the call itself brought ends the wait), or when the state went to another settled state and stays there (settle_seconds), such as a client back at the character selection instead of in the game. "
	    "Returns the state with matched false when the time ran out.",
	    p, "", true));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("prefix", prop("string", "Only names starting with this."));
	p.set("contains", prop("string", "Only entries whose name or help contains this (case-insensitive)."));
	p.set("limit", prop("integer", "At most this many entries (300 by default)."));
	tools.push(tool("list_commands", "The command table of an instance: commands and variables with their argument syntax and help, handler instances and their member commands.", p, "", true));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("command", prop("string", "The command or variable, instance.command for a member command."));
	p.set("args", stringsProp("The arguments, one per entry; they reach the command as given."));
	p.set("wait_seconds", prop("number", "How long to wait for the answer (60 by default)."));
	tools.push(tool("run_command",
	    "Runs a command of an instance, or reads a variable by its name alone and sets it with one argument. Returns whether it succeeded, its output, and the system messages it showed meanwhile.",
	    p, "command", false));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("from_idx", prop("integer", "First line index, inclusive; the oldest lines come first from there."));
	p.set("to_idx", prop("integer", "Last line index, inclusive."));
	p.set("last", prop("integer", "The last N lines of the selection (100 when nothing else selects)."));
	p.set("levels", stringsProp("Only these levels: INF, WRN, ERR, DBG, AST, STT."));
	p.set("contains", prop("string", "Only lines containing this (case-insensitive)."));
	p.set("max_lines", prop("integer", "At most this many lines (200 by default, 5000 at most)."));
	p.set("wait_seconds", prop("number", "When no line matches yet, wait up to this long for one (with from_idx: the next one)."));
	tools.push(tool("get_log",
	    "Log lines of an instance: index, time, level, text, origin. Indices count from 1 since the process started and never change; continue from next_idx. With from_idx and wait_seconds it waits for the next matching line, so a log wording is waited for without sleeping.",
	    p, "", true));
	p = CJsonValue::object();
	p.set("instance", instanceProp());
	p.set("from_idx", prop("integer", "First event index, inclusive."));
	p.set("last", prop("integer", "The last N events."));
	p.set("kinds", stringsProp("Only these kinds (state, system_info, exit, ...)."));
	p.set("contains", prop("string", "Only events whose data contains this (case-insensitive)."));
	p.set("max", prop("integer", "At most this many events (200 by default)."));
	p.set("wait_seconds", prop("number", "When no event matches yet, wait up to this long for one."));
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
	p.set("wait_seconds", prop("number", "How long to wait for the answer (60 by default)."));
	tools.push(tool("call_method", "Calls a method of an instance and returns its result, with images and other data it attaches.", p, "method", false));
	return tools;
}

uint waitMs(const CJsonValue &args, double defaultSeconds)
{
	double seconds = args.get("wait_seconds").getNumber(defaultSeconds);
	if (seconds < 0)
		seconds = 0;
	if (seconds > 600)
		seconds = 600;
	return (uint)(seconds * 1000.0);
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
		std::string state = instance.Record.get("state").getString();
		if (state == "running" && !instance.Alive)
			state = "gone without a reason (it crashed or was killed)";
		else if (state == "exited")
			state = "exited: " + instance.Record.get("exit_reason").getString();
		else if (state == "failed")
			state = "failed: " + instance.Record.get("error").getString();
		e.set("state", state);
		static const char *const keys[] = { "pid", "kind", "program", "cwd", "log", "version", "build", "account", "shard", "character" };
		for (uint k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k)
			if (instance.Record.has(keys[k]))
				e.set(keys[k], instance.Record.get(keys[k]));
		list.push(e);
		// a record of a process that is gone is shown once
		if (!instance.Alive)
			CFile::deleteFile(instance.RecordPath);
	}
	replyToolText(rpcId, list.toString(), false);
}

void killInstance(const CJsonValue &rpcId, const std::string &name)
{
	std::vector<CInstance> instances;
	std::string error;
	scanInstances(instances, error);
	for (uint i = 0; i < instances.size(); ++i)
	{
		const CInstance &instance = instances[i];
		if (instance.Name != name || !instance.Alive)
			continue;
#ifdef NL_OS_WINDOWS
		HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)instance.Pid);
		bool ok = process != NULL && TerminateProcess(process, 1);
		if (process != NULL)
			CloseHandle(process);
#else
		bool ok = kill((pid_t)instance.Pid, SIGKILL) == 0;
#endif
		replyToolText(rpcId, ok ? toString("killed '%s' (process %u)", name.c_str(), instance.Pid) : toString("cannot kill process %u", instance.Pid), !ok);
		return;
	}
	replyToolText(rpcId, toString("no running instance '%s'", name.c_str()), true);
}

void callTool(const CJsonValue &rpcId, const std::string &name, const CJsonValue &args)
{
	std::string instance = args.get("instance").getString();
	std::string error;
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
			replyToolText(rpcId, toString("no account '%s' in the profile", accountName.c_str()), true);
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
		CJsonValue a = CJsonValue::object();
		if (args.has("state"))
			a.set("state", args.get("state"));
		if (args.has("states"))
			a.set("states", args.get("states"));
		if (args.has("not"))
			a.set("not", args.get("not"));
		if (args.has("stop_on"))
			a.set("stop_on", args.get("stop_on"));
		if (args.has("settle_seconds"))
			a.set("settle_seconds", args.get("settle_seconds"));
		if (args.has("since_idx"))
			a.set("since_idx", args.get("since_idx"));
		uint ms = waitMs(args, 30);
		a.set("timeout_ms", ms);
		ok = startCall(rpcId, instance, "ctrl.wait_state", a, FormatJson, ms + 10000, error);
	}
	else if (name == "list_commands")
	{
		CJsonValue a = CJsonValue::object();
		if (args.has("prefix"))
			a.set("prefix", args.get("prefix"));
		if (args.has("contains"))
			a.set("contains", args.get("contains"));
		if (args.has("limit"))
			a.set("limit", args.get("limit"));
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
		if (args.has("prefix"))
			a.set("prefix", args.get("prefix"));
		if (args.has("contains"))
			a.set("contains", args.get("contains"));
		if (args.has("schemas"))
			a.set("schemas", args.get("schemas"));
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
		replyToolText(rpcId, "unknown tool: " + name, true);
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
			if (link != s_Links.end() && link->second->Pipe.isOpen())
			{
				std::vector<std::string> fields;
				fields.push_back("cancel");
				fields.push_back(op->CallId);
				link->second->Pipe.write(joinCtrlLine(fields) + "\n", s_WriteTimeoutMs);
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
	if (!message.parse(line, error) || !message.isObject())
	{
		replyError(CJsonValue(), -32700, "parse error: " + error);
		return;
	}
	const std::string &method = message.get("method").getString();
	if (method == "notifications/cancelled")
	{
		cancelRequest(message.get("params").get("requestId"));
		return;
	}
	if (!message.has("id") || method.empty() || method.compare(0, 14, "notifications/") == 0)
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
		reply(id, CJsonValue::object().set("tools", toolList()));
	}
	else if (method == "tools/call")
	{
		callTool(id, params.get("name").getString(), params.get("arguments"));
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
	// the processes launched earlier that have ended since; an exit code nobody asks for is not kept
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
	for (std::map<std::string, CLink *>::iterator it = s_Links.begin(); it != s_Links.end(); ++it)
	{
		CLink &link = *it->second;
		CCtrlMessage message;
		while (readLink(link, message))
		{
			busy = true;
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
				if (!op->Launch && op->Instance == link.Name && op->CallId == message.Fields[1])
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
	}
	// the operations whose instance went away or that took too long
	TTime now = CTime::getLocalTime();
	for (std::list<COp>::iterator op = s_Ops.begin(); op != s_Ops.end();)
	{
		bool done = false;
		if (op->Launch)
		{
			// the registry is read a few times a second, not at every turn
			if (now >= op->NextCheck)
			{
				op->NextCheck = now + 200;
				done = checkLaunch(*op);
				if (done)
					closeLaunch(*op);
			}
		}
		else
		{
			std::map<std::string, CLink *>::iterator link = s_Links.find(op->Instance);
			if (link == s_Links.end() || !link->second->Pipe.isOpen())
			{
				replyToolText(op->RpcId, toString("'%s' closed the connection before it answered; list_instances says why", op->Instance.c_str()), true);
				done = true;
			}
			else if (now >= op->Deadline)
			{
				std::vector<std::string> fields;
				fields.push_back("cancel");
				fields.push_back(op->CallId);
				link->second->Pipe.write(joinCtrlLine(fields) + "\n", s_WriteTimeoutMs);
				replyToolText(op->RpcId, toString("'%s' did not answer in time; the call was cancelled (wait_seconds gives it longer)", op->Instance.c_str()), true);
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
	createDebug(nullptr, false);
	CStderrDisplayer stderrDisplayer;
	CLog *logs[] = { INelContext::getInstance().getInfoLog(), INelContext::getInstance().getDebugLog(), INelContext::getInstance().getWarningLog(),
		INelContext::getInstance().getErrorLog(), INelContext::getInstance().getAssertLog() };
	for (uint i = 0; i < sizeof(logs) / sizeof(logs[0]); ++i)
	{
		logs[i]->removeDisplayer("DEFAULT_SD");
		if (i >= 2)
			logs[i]->addDisplayer(&stderrDisplayer);
	}

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg.compare(0, 10, "--profile=") == 0)
		{
			s_ProfilePath = arg.substr(10);
		}
		else if (arg == "--help" || arg == "-h")
		{
			fprintf(stderr, "nel_mcp [--profile=<file>]: an MCP server over stdio for the control endpoints of NeL processes on this machine\n");
			return 0;
		}
		else
		{
			fprintf(stderr, "nel_mcp: unknown argument '%s'\n", arg.c_str());
			return 1;
		}
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
		std::deque<std::string> lines;
		bool busy = reader.take(lines);
		for (std::deque<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
			if (!it->empty())
				handleRpc(*it);
		if (closed && lines.empty())
			break;
		busy = pumpLinks() || busy;
		if (!busy)
			nlSleep(2);
	}

	// the client closing stdin ends the server; the instances stay
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
