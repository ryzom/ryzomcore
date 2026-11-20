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

#include "nel/misc/ctrl_endpoint.h"

#include "nel/misc/command.h"
#include "nel/misc/common.h"
#include "nel/misc/ctrl_protocol.h"
#include "nel/misc/debug.h"
#include "nel/misc/displayer.h"
#include "nel/misc/local_pipe.h"
#include "nel/misc/log.h"
#include "nel/misc/mem_displayer.h"
#include "nel/misc/mutex.h"
#include "nel/misc/path.h"
#include "nel/misc/thread.h"

#include <deque>
#include <set>

#ifndef NL_OS_WINDOWS
#include <sys/time.h>
#endif

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC {

namespace {

// the rings: what a reader can still get after a while away
const size_t s_LogBudget = 16 * 1024 * 1024;
const size_t s_EventBudget = 4 * 1024 * 1024;
// a Lua chunk or a long list of arguments fits; nothing a tool sends is larger
const size_t s_MaxInLine = 8 * 1024 * 1024;
// a reader that does not read for this long is dropped, so the channel thread never stalls on it
const uint s_WriteTimeoutMs = 10000;
const uint s_TickMs = 5;
const sint64 s_DefaultWaitMs = 30000;
const sint64 s_MaxWaitMs = 600000;
const uint s_DefaultMaxLines = 200;
const uint s_MaxLines = 5000;

sint64 argInt(const CJsonValue &args, const char *name, sint64 defaultValue)
{
	return args.get(name).getInt(defaultValue);
}

/// Whether the text holds the needle, ignoring the case of ASCII letters; the needle is lower case already.
/// No copy of the text: the rings are scanned with their lock held.
bool containsLowerAscii(const std::string &text, const std::string &needle)
{
	if (needle.size() > text.size())
		return false;
	size_t last = text.size() - needle.size();
	for (size_t i = 0; i <= last; ++i)
	{
		size_t k = 0;
		while (k < needle.size())
		{
			char c = text[i + k];
			if (c >= 'A' && c <= 'Z')
				c = (char)(c - 'A' + 'a');
			if (c != needle[k])
				break;
			++k;
		}
		if (k == needle.size())
			return true;
	}
	return false;
}

} // anonymous namespace

uint64 getCtrlTimeMs()
{
#ifdef NL_OS_WINDOWS
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	uint64 ticks = ((uint64)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	return (ticks - 116444736000000000ULL) / 10000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64)tv.tv_sec * 1000 + (uint64)tv.tv_usec / 1000;
#endif
}

// *** CCtrlCall ***

CCtrlCall::CCtrlCall()
    : m_Connection(0)
    , m_StartTime(0)
    , m_Done(false)
    , m_Error(false)
    , m_Data(nullptr)
{
}

CCtrlCall::~CCtrlCall()
{
	delete m_Data;
}

TTime CCtrlCall::getElapsed() const
{
	return CTime::getLocalTime() - m_StartTime;
}

void CCtrlCall::reply(const CJsonValue &result)
{
	if (m_Done)
		return;
	m_Done = true;
	m_Error = false;
	m_Result = result;
}

void CCtrlCall::fail(const std::string &message, const CJsonValue &details)
{
	if (m_Done)
		return;
	m_Done = true;
	m_Error = true;
	m_Message = message;
	m_Result = details;
}

CJsonValue CCtrlCall::attach(const std::string &contentType, const std::string &bytes)
{
	m_Attachments.push_back(std::make_pair(contentType, bytes));
	CJsonValue ref = CJsonValue::object();
	ref.set("data", toString("%s.%u", m_Id.c_str(), (uint)m_Attachments.size()));
	ref.set("type", contentType);
	ref.set("bytes", (uint64)bytes.size());
	return ref;
}

void CCtrlCall::setData(ICtrlCallData *data)
{
	if (data != m_Data)
		delete m_Data;
	m_Data = data;
}

// *** methods ***

ICtrlMethod::ICtrlMethod(const std::string &name, const std::string &description, const CJsonValue &schema, bool readOnly)
    : m_Name(name)
    , m_Description(description)
    , m_Schema(schema.isNull() ? CJsonValue(CCtrlSchema()) : schema)
    , m_ReadOnly(readOnly)
{
}

CCtrlFunctionMethod::CCtrlFunctionMethod(const std::string &name, const std::string &description, const CJsonValue &schema, bool readOnly, TFunction callFunction,
    TFunction pollFunction, TFunction cancelFunction)
    : ICtrlMethod(name, description, schema, readOnly)
    , m_Call(callFunction)
    , m_Poll(pollFunction)
    , m_Cancel(cancelFunction)
{
	nlassert(m_Call != nullptr);
}

void CCtrlFunctionMethod::call(CCtrlCall &call)
{
	m_Call(call);
}

void CCtrlFunctionMethod::poll(CCtrlCall &call)
{
	if (m_Poll != nullptr)
		m_Poll(call);
}

void CCtrlFunctionMethod::cancel(CCtrlCall &call)
{
	if (m_Cancel != nullptr)
		m_Cancel(call);
}

// *** schemas ***

CCtrlSchema::CCtrlSchema()
{
	m_Schema = CJsonValue::object();
	m_Schema.set("type", "object");
	m_Schema.set("properties", CJsonValue::object());
}

CCtrlSchema &CCtrlSchema::add(const char *name, const CJsonValue &property, bool required)
{
	m_Schema.member("properties").set(name, property);
	if (required)
		m_Schema.member("required").push(CJsonValue(name));
	return *this;
}

static CJsonValue typedProperty(const char *type, const char *description)
{
	CJsonValue p = CJsonValue::object();
	p.set("type", type);
	p.set("description", description);
	return p;
}

CCtrlSchema &CCtrlSchema::str(const char *name, const char *description, bool required)
{
	return add(name, typedProperty("string", description), required);
}

CCtrlSchema &CCtrlSchema::number(const char *name, const char *description, bool required)
{
	return add(name, typedProperty("number", description), required);
}

CCtrlSchema &CCtrlSchema::integer(const char *name, const char *description, bool required)
{
	return add(name, typedProperty("integer", description), required);
}

CCtrlSchema &CCtrlSchema::boolean(const char *name, const char *description, bool required)
{
	return add(name, typedProperty("boolean", description), required);
}

CCtrlSchema &CCtrlSchema::choice(const char *name, const char *values, const char *description, bool required)
{
	CJsonValue p = typedProperty("string", description);
	std::vector<std::string> list;
	explode(std::string(values), std::string("|"), list, true);
	CJsonValue e = CJsonValue::array();
	for (uint i = 0; i < list.size(); ++i)
		e.push(CJsonValue(list[i]));
	p.set("enum", e);
	return add(name, p, required);
}

CCtrlSchema &CCtrlSchema::strings(const char *name, const char *description, bool required)
{
	CJsonValue p = typedProperty("array", description);
	p.set("items", CJsonValue::object().set("type", "string"));
	return add(name, p, required);
}

CCtrlSchema &CCtrlSchema::numbers(const char *name, const char *description, bool required)
{
	CJsonValue p = typedProperty("array", description);
	p.set("items", CJsonValue::object().set("type", "number"));
	return add(name, p, required);
}

CCtrlSchema &CCtrlSchema::any(const char *name, const char *description, bool required)
{
	CJsonValue p = CJsonValue::object();
	p.set("description", description);
	return add(name, p, required);
}

CCtrlSchema &CCtrlSchema::object(const char *name, const char *description, bool required)
{
	return add(name, typedProperty("object", description), required);
}

// *** the channel ***

namespace {

struct CLogLine
{
	uint64 Idx;
	uint64 Time;
	const char *Level;
	std::string Text;
	std::string Origin;
};

struct CEventLine
{
	uint64 Idx;
	uint64 Time;
	std::string Kind;
	std::string Data; // JSON text
};

struct CInCall
{
	uint32 Connection;
	std::string Id;
	std::string Method;
	CJsonValue Args;
};

struct COutMessage
{
	uint32 Connection;
	std::string Id;
	bool Error;
	std::string Message;
	std::string Json;
	std::vector<std::pair<std::string, std::string>> Attachments;
};

struct CMethodInfo
{
	std::string Name;
	std::string Description;
	CJsonValue Schema;
	bool ReadOnly;
	bool BuiltIn;
};

struct CConnection
{
	uint32 Id;
	CLocalPipe *Pipe;
	CCtrlMessageReader Reader;
	std::string Name;
	// the next event index when its last call to the main thread was queued: a wait that follows that call
	// watches the events from there, so a refusal the call itself brought (a message box) is not missed
	uint64 LastCallEvent;

	CConnection()
	    : Id(0)
	    , Pipe(nullptr)
	    , Reader(s_MaxInLine, 0)
	    , LastCallEvent(0)
	{
	}
};

struct CWaiter
{
	uint32 Connection;
	std::string Id;
	std::string Method;
	CJsonValue Args;
	TTime Deadline;
	TTime Start;
	// the next index of the ring at the last scan that found nothing; 0 before the first
	uint64 Seen;
	// what the first scan said of a from_idx the ring no longer held
	std::string Missed;
	// a wait for a state: where it started, the state seen last and since when, the first event it watches, and
	// the next one it has not looked at yet
	std::string StartState;
	std::string LastState;
	bool Changed;
	TTime SettledSince;
	uint64 EventStart;
	uint64 EventScanned;

	CWaiter()
	    : Connection(0)
	    , Deadline(0)
	    , Start(0)
	    , Seen(0)
	    , Changed(false)
	    , SettledSince(0)
	    , EventStart(0)
	    , EventScanned(0)
	{
	}
};

} // anonymous namespace

class CCtrlEndpoint::CImpl : public IRunnable
{
public:
	CImpl(CCtrlEndpoint *owner)
	    : Owner(owner)
	    , Thread(nullptr)
	    , Running(false)
	    , LogNext(1)
	    , LogBytes(0)
	    , EventNext(1)
	    , EventBytes(0)
	    , StateSettled(true)
	    , StateSince(0)
	    , Frame(0)
	    , LastUpdate(0)
	    , NextConnection(1)
	    , LeaseHolder(0)
	    , LeaseHeld(false)
	    , WriteTimeoutMs(s_WriteTimeoutMs)
	{
	}

	CCtrlEndpoint *Owner;
	CLocalPipeListener Listener;
	std::string RecordPath;
	std::string EndpointPath;
	uint32 Pid;
	uint64 StartTime;
	IThread *Thread;
	volatile bool Running;

	// identity, as the hello and the record carry it
	CMutex IdentityMutex;
	CJsonValue Identity;

	CMutex LogMutex;
	std::deque<CLogLine> Log;
	uint64 LogNext;
	size_t LogBytes;

	CMutex EventMutex;
	std::deque<CEventLine> Events;
	uint64 EventNext;
	size_t EventBytes;

	CMutex StateMutex;
	std::string State;
	std::string StateDetail;
	bool StateSettled;
	uint64 StateSince;
	uint64 Frame;
	TTime LastUpdate;

	CMutex MethodsMutex;
	std::vector<CMethodInfo> Methods;

	CMutex InboxMutex;
	std::vector<CInCall> Inbox;
	std::vector<std::pair<uint32, std::string>> Cancels;
	std::vector<uint32> Closed;

	CMutex OutboxMutex;
	std::vector<COutMessage> Outbox;

	// (MT) the calls a partial update left for a full one
	std::vector<CInCall> Deferred;
	// (CT) how long a write may wait on a reader; short for the last answers when the endpoint stops
	uint WriteTimeoutMs;

	// (CT)
	std::map<uint32, CConnection *> Connections;
	uint32 NextConnection;
	std::vector<CWaiter> Waiters;
	uint32 LeaseHolder;
	// (CT writes, AT reads) whether any connection holds it
	volatile bool LeaseHeld;

	virtual void getName(std::string &result) const NL_OVERRIDE
	{
		result = "CtrlEndpoint";
	}

	// *** log ring (AT) ***

	void addLog(const CLog::TDisplayInfo &args, const char *message)
	{
		if (message == nullptr)
			return;
		uint64 now = getCtrlTimeMs();
		const char *level = IDisplayer::logTypeToString(args.LogType);
		std::string origin;
		if (args.FileName != nullptr && args.Line >= 0)
			origin = toString("%s:%d", CFile::getFilename(args.FileName).c_str(), args.Line);
		// one entry per line, as the log file has them
		const char *start = message;
		CAutoMutex<CMutex> lock(LogMutex);
		for (;;)
		{
			const char *end = strchr(start, '\n');
			size_t len = end ? (size_t)(end - start) : strlen(start);
			if (len > 0 && start[len - 1] == '\r')
				--len;
			if (len > 0 || end == nullptr)
			{
				if (len > 0)
				{
					CLogLine line;
					line.Idx = LogNext++;
					line.Time = now;
					line.Level = level;
					line.Text.assign(start, len);
					line.Origin = origin;
					LogBytes += line.Text.size() + line.Origin.size() + 64;
					Log.push_back(line);
				}
			}
			if (end == nullptr || end[1] == 0)
				break;
			start = end + 1;
		}
		while (LogBytes > s_LogBudget && !Log.empty())
		{
			LogBytes -= Log.front().Text.size() + Log.front().Origin.size() + 64;
			Log.pop_front();
		}
	}

	void addEvent(const std::string &kind, const CJsonValue &data)
	{
		CEventLine e;
		e.Time = getCtrlTimeMs();
		e.Kind = kind;
		e.Data = data.toString();
		CAutoMutex<CMutex> lock(EventMutex);
		e.Idx = EventNext++;
		EventBytes += e.Kind.size() + e.Data.size() + 48;
		Events.push_back(e);
		while (EventBytes > s_EventBudget && !Events.empty())
		{
			EventBytes -= Events.front().Kind.size() + Events.front().Data.size() + 48;
			Events.pop_front();
		}
	}

	// *** the thread ***

	virtual void run() NL_OVERRIDE
	{
		while (Running)
		{
			bool busy = false;
			acceptConnections();
			for (std::map<uint32, CConnection *>::iterator it = Connections.begin(); it != Connections.end(); ++it)
				busy = readConnection(*it->second) || busy;
			busy = flushOutbox() || busy;
			checkWaiters();
			dropClosed();
			if (!busy)
				nlSleep(s_TickMs);
		}
		// the answers given meanwhile, the one to a quit method among them, then everyone is told by the pipe
		// closing; a reader that does not take them at once does not hold up the exit
		WriteTimeoutMs = 500;
		flushOutbox();
		for (std::map<uint32, CConnection *>::iterator it = Connections.begin(); it != Connections.end(); ++it)
		{
			delete it->second->Pipe;
			delete it->second;
		}
		Connections.clear();
		Waiters.clear();
	}

	void acceptConnections()
	{
		for (;;)
		{
			CLocalPipe *pipe = Listener.accept();
			if (pipe == nullptr)
				return;
			CConnection *c = new CConnection();
			c->Id = NextConnection++;
			c->Pipe = pipe;
			c->Name = toString("connection %u", c->Id);
			Connections[c->Id] = c;
			std::vector<std::string> fields;
			fields.push_back("hello");
			fields.push_back(toString("%u", CtrlProtocolVersion));
			fields.push_back(helloIdentity(c->Id).toString());
			sendLine(*c, joinCtrlLine(fields));
			nlinfo("CTRL: %s opened", c->Name.c_str());
		}
	}

	CJsonValue helloIdentity(uint32 connection)
	{
		CJsonValue id;
		{
			CAutoMutex<CMutex> lock(IdentityMutex);
			id = Identity;
		}
		id.set("connection", connection);
		return id;
	}

	bool sendLine(CConnection &c, const std::string &line)
	{
		if (c.Pipe == nullptr || !c.Pipe->isOpen())
			return false;
		std::string data = line;
		data += '\n';
		return c.Pipe->write(data, WriteTimeoutMs);
	}

	bool readConnection(CConnection &c)
	{
		if (!c.Pipe->isOpen())
			return false;
		size_t before = c.Reader.buffer().size();
		c.Pipe->readAvailable(c.Reader.buffer(), 1024 * 1024);
		bool got = c.Reader.buffer().size() != before;
		CCtrlMessage message;
		std::string error;
		while (c.Pipe->isOpen() && c.Reader.next(message, error))
			handleMessage(c, message);
		if (!error.empty())
		{
			nlwarning("CTRL: %s sent %s, closing it", c.Name.c_str(), error.c_str());
			c.Pipe->close();
		}
		return got;
	}

	void sendResult(CConnection &c, const std::string &id, const CJsonValue &result)
	{
		std::vector<std::string> fields;
		fields.push_back("result");
		fields.push_back(id);
		fields.push_back(wrap(result).toString());
		sendLine(c, joinCtrlLine(fields));
	}

	void sendError(CConnection &c, const std::string &id, const std::string &message, const CJsonValue &details = CJsonValue())
	{
		std::vector<std::string> fields;
		fields.push_back("error");
		fields.push_back(id);
		fields.push_back(message);
		fields.push_back(wrap(details).toString());
		sendLine(c, joinCtrlLine(fields));
	}

	/// What every answer carries: the frame, the clock and the state, so it lines up with the log
	CJsonValue wrap(const CJsonValue &result)
	{
		CJsonValue o = CJsonValue::object();
		o.set("result", result);
		CAutoMutex<CMutex> lock(StateMutex);
		o.set("frame", Frame);
		o.set("time", getCtrlTimeMs());
		o.set("state", State);
		return o;
	}

	void handleMessage(CConnection &c, const CCtrlMessage &message)
	{
		const std::vector<std::string> &f = message.Fields;
		if (message.IsData || f.empty())
			return;
		if (f[0] == "hello")
		{
			// hello <version> <name>: the tool says who it is, so a lease holder has a name
			if (f.size() >= 3 && !f[2].empty())
				c.Name = toString("%s (connection %u)", f[2].c_str(), c.Id);
			return;
		}
		if (f[0] == "cancel")
		{
			if (f.size() < 2)
				return;
			for (uint i = 0; i < Waiters.size(); ++i)
			{
				if (Waiters[i].Connection == c.Id && Waiters[i].Id == f[1])
				{
					sendError(c, f[1], "cancelled");
					Waiters.erase(Waiters.begin() + i);
					return;
				}
			}
			CAutoMutex<CMutex> lock(InboxMutex);
			Cancels.push_back(std::make_pair(c.Id, f[1]));
			return;
		}
		if (f[0] != "call")
		{
			nldebug("CTRL: %s sent an unknown message type '%s'", c.Name.c_str(), f[0].c_str());
			return;
		}
		if (f.size() < 3)
		{
			sendError(c, f.size() > 1 ? f[1] : std::string(), "a call needs an id and a method");
			return;
		}
		const std::string &id = f[1];
		const std::string &method = f[2];
		CJsonValue args = CJsonValue::object();
		if (f.size() >= 4 && !f[3].empty())
		{
			std::string error;
			if (!args.parse(f[3], error))
			{
				sendError(c, id, "the arguments are not JSON: " + error);
				return;
			}
			if (args.isNull())
				args = CJsonValue::object();
			if (!args.isObject())
			{
				sendError(c, id, "the arguments must be a JSON object");
				return;
			}
		}

		if (handleBuiltIn(c, id, method, args))
			return;

		bool readOnly = true;
		bool found = false;
		{
			CAutoMutex<CMutex> lock(MethodsMutex);
			for (uint i = 0; i < Methods.size(); ++i)
			{
				if (Methods[i].Name == method)
				{
					found = true;
					readOnly = Methods[i].ReadOnly;
					break;
				}
			}
		}
		if (!found)
		{
			sendError(c, id, toString("no method '%s'; ctrl.methods lists them", method.c_str()));
			return;
		}
		if (!readOnly && !takeLease(c, id))
			return;

		CInCall in;
		in.Connection = c.Id;
		in.Id = id;
		in.Method = method;
		in.Args = args;
		c.LastCallEvent = ringNext(false);
		CAutoMutex<CMutex> lock(InboxMutex);
		Inbox.push_back(in);
	}

	bool takeLease(CConnection &c, const std::string &id)
	{
		if (LeaseHolder == 0 || LeaseHolder == c.Id)
		{
			if (LeaseHolder == 0)
				nlinfo("CTRL: %s took the lease", c.Name.c_str());
			LeaseHolder = c.Id;
			LeaseHeld = true;
			return true;
		}
		std::map<uint32, CConnection *>::iterator it = Connections.find(LeaseHolder);
		std::string holder = it != Connections.end() ? it->second->Name : toString("connection %u", LeaseHolder);
		CJsonValue details = CJsonValue::object();
		details.set("holder", holder);
		sendError(c, id, toString("the lease is held by %s; only read-only methods are open to other connections", holder.c_str()), details);
		return false;
	}

	CJsonValue stateSnapshot()
	{
		CJsonValue s = CJsonValue::object();
		CAutoMutex<CMutex> lock(StateMutex);
		s.set("state", State);
		CJsonValue detail;
		std::string error;
		if (!StateDetail.empty() && detail.parse(StateDetail, error))
			s.set("detail", detail);
		s.set("since", StateSince);
		s.set("frame", Frame);
		TTime now = CTime::getLocalTime();
		sint64 idle = LastUpdate == 0 ? -1 : (sint64)(now - LastUpdate);
		s.set("main_thread_idle_ms", idle);
		// a frame of a client is far below this; a main thread that does not come back is stuck in something
		s.set("main_thread", idle < 0 ? "not started" : (idle > 2000 ? toString("busy for %u s in %s", (uint)(idle / 1000), State.c_str()) : std::string("running")));
		return s;
	}

	bool handleBuiltIn(CConnection &c, const std::string &id, const std::string &method, const CJsonValue &args)
	{
		if (method == "ctrl.identity")
		{
			sendResult(c, id, helloIdentity(c.Id));
			return true;
		}
		if (method == "ctrl.state")
		{
			sendResult(c, id, stateSnapshot());
			return true;
		}
		if (method == "ctrl.methods")
		{
			sendResult(c, id, listMethods(args));
			return true;
		}
		if (method == "ctrl.lease")
		{
			std::string action = args.get("action").getString("status");
			if (action == "take")
			{
				if (!takeLease(c, id))
					return true;
			}
			else if (action == "release")
			{
				if (LeaseHolder == c.Id)
				{
					LeaseHolder = 0;
					LeaseHeld = false;
					nlinfo("CTRL: %s gave the lease back", c.Name.c_str());
				}
			}
			else if (action != "status")
			{
				sendError(c, id, "action is take, release or status");
				return true;
			}
			CJsonValue r = CJsonValue::object();
			std::map<uint32, CConnection *>::iterator it = Connections.find(LeaseHolder);
			r.set("holder", LeaseHolder == 0 ? CJsonValue() : CJsonValue(it != Connections.end() ? it->second->Name : toString("connection %u", LeaseHolder)));
			r.set("yours", LeaseHolder == c.Id);
			sendResult(c, id, r);
			return true;
		}
		if (method == "ctrl.wait_state" || method == "ctrl.log" || method == "ctrl.events")
		{
			CWaiter w;
			w.Connection = c.Id;
			w.Id = id;
			w.Method = method;
			w.Args = args;
			w.Start = CTime::getLocalTime();
			sint64 waitMs = argInt(args, method == "ctrl.wait_state" ? "timeout_ms" : "wait_ms", method == "ctrl.wait_state" ? s_DefaultWaitMs : 0);
			if (waitMs < 0)
				waitMs = 0;
			if (waitMs > s_MaxWaitMs)
				waitMs = s_MaxWaitMs;
			w.Deadline = w.Start + waitMs;
			if (method == "ctrl.wait_state")
			{
				{
					CAutoMutex<CMutex> lock(StateMutex);
					w.StartState = State;
					w.LastState = State;
				}
				// from since_idx, else from the last call of the connection, else from now
				w.EventStart = w.Args.has("since_idx") ? (uint64)std::max((sint64)1, w.Args.get("since_idx").getInt(1))
				                                       : (c.LastCallEvent != 0 ? c.LastCallEvent : ringNext(false));
				w.EventScanned = w.EventStart;
			}
			if (!answerWaiter(c, w, waitMs == 0))
				Waiters.push_back(w);
			return true;
		}
		return false;
	}

	CJsonValue listMethods(const CJsonValue &args)
	{
		std::string prefix = args.get("prefix").getString();
		std::string contains = toLowerAscii(args.get("contains").getString());
		bool schemas = args.get("schemas").getBool(false);
		CJsonValue list = CJsonValue::array();
		CAutoMutex<CMutex> lock(MethodsMutex);
		for (uint i = 0; i < Methods.size(); ++i)
		{
			const CMethodInfo &m = Methods[i];
			if (!prefix.empty() && m.Name.compare(0, prefix.size(), prefix) != 0)
				continue;
			if (!contains.empty() && toLowerAscii(m.Name + " " + m.Description).find(contains) == std::string::npos)
				continue;
			CJsonValue e = CJsonValue::object();
			e.set("name", m.Name);
			e.set("description", m.Description);
			e.set("read_only", m.ReadOnly);
			if (schemas)
				e.set("schema", m.Schema);
			list.push(e);
		}
		CJsonValue r = CJsonValue::object();
		r.set("methods", list);
		return r;
	}

	/// Answers a waiting call when its condition holds, or when it is out of time (or final)
	bool answerWaiter(CConnection &c, CWaiter &w, bool final)
	{
		TTime now = CTime::getLocalTime();
		bool expired = final || now >= w.Deadline;
		if (w.Method == "ctrl.wait_state")
		{
			CJsonValue snapshot = stateSnapshot();
			const std::string &state = snapshot.get("state").getString();
			bool match = false;
			const CJsonValue &states = w.Args.get("states");
			if (w.Args.get("state").isString())
				match = state == w.Args.get("state").getString();
			for (uint i = 0; i < states.size(); ++i)
				match = match || state == states.at(i).getString();
			if (w.Args.get("not").isString())
				match = match || (state != w.Args.get("not").getString() && !w.Args.has("state") && !w.Args.has("states"));
			std::string stoppedBy;
			if (!match && !expired)
			{
				// what makes the state unreachable ends the wait: an event of the kinds watched (a message box that
				// refuses, the exit), or a settled state the process went to instead and stays in
				stoppedBy = stoppingEvent(w);
				bool settled;
				{
					CAutoMutex<CMutex> lock(StateMutex);
					settled = StateSettled;
				}
				if (state != w.StartState)
					w.Changed = true;
				if (state != w.LastState || !settled)
					w.SettledSince = now;
				w.LastState = state;
				sint64 settleMs = (sint64)(w.Args.get("settle_seconds").getNumber(5.0) * 1000.0);
				if (stoppedBy.empty() && w.Changed && settled && settleMs > 0 && (sint64)(now - w.SettledSince) >= settleMs)
					stoppedBy = toString("the state went to %s instead and stayed there", state.c_str());
				if (stoppedBy.empty())
					return false;
			}
			snapshot.set("matched", match);
			if (!stoppedBy.empty())
				snapshot.set("stopped", stoppedBy);
			snapshot.set("waited_ms", (sint64)(now - w.Start));
			sendResult(c, w.Id, snapshot);
			return true;
		}
		// nothing new since the last look: no scan, the ring is not locked for long while the process logs
		uint64 next = ringNext(w.Method == "ctrl.log");
		if (w.Seen == next && !expired)
			return false;
		CJsonValue r = w.Method == "ctrl.log" ? queryLog(w.Args) : queryEvents(w.Args);
		bool none = r.get(w.Method == "ctrl.log" ? "lines" : "events").size() == 0;
		// a range that ends before the next line can get nothing more
		bool closed = w.Args.has("to_idx") && (uint64)std::max((sint64)0, w.Args.get("to_idx").getInt(0)) < next;
		if (none && !expired && !closed)
		{
			// what was scanned held nothing: the next look starts after it, never before the index asked for. A
			// from_idx the ring no longer held is said in the answer all the same.
			if (w.Missed.empty() && r.has("missed"))
				w.Missed = r.get("missed").getString();
			w.Seen = next;
			uint64 from = w.Args.has("from_idx") ? (uint64)std::max((sint64)0, w.Args.get("from_idx").getInt(0)) : 0;
			if (next > from)
				w.Args.set("from_idx", next);
			return false;
		}
		r.set("waited_ms", (sint64)(now - w.Start));
		if (!w.Missed.empty() && !r.has("missed"))
			r.set("missed", w.Missed);
		sendResult(c, w.Id, r);
		return true;
	}

	/// The first event since the wait began of the kinds that end it (message_box and exit by default)
	/// The first event of the kinds that end the wait (message_box and exit by default) since it watches; each event
	/// is looked at once
	std::string stoppingEvent(CWaiter &w)
	{
		std::set<std::string> kinds;
		const CJsonValue &stopOn = w.Args.get("stop_on");
		if (stopOn.isArray())
		{
			for (uint i = 0; i < stopOn.size(); ++i)
				kinds.insert(stopOn.at(i).getString());
		}
		else if (stopOn.isString())
		{
			kinds.insert(stopOn.getString());
		}
		else
		{
			kinds.insert("message_box");
			kinds.insert("exit");
		}
		if (kinds.empty())
			return std::string();
		CAutoMutex<CMutex> lock(EventMutex);
		if (Events.empty())
			return std::string();
		uint64 first = Events.front().Idx;
		size_t pos = w.EventScanned > first ? (size_t)std::min((uint64)Events.size(), w.EventScanned - first) : 0;
		for (; pos < Events.size(); ++pos)
		{
			const CEventLine &e = Events[pos];
			if (kinds.find(e.Kind) != kinds.end())
				return toString("event %u: %s %s", (uint)e.Idx, e.Kind.c_str(), e.Data.c_str());
		}
		w.EventScanned = EventNext;
		return std::string();
	}

	uint64 ringNext(bool log)
	{
		if (log)
		{
			CAutoMutex<CMutex> lock(LogMutex);
			return LogNext;
		}
		CAutoMutex<CMutex> lock(EventMutex);
		return EventNext;
	}

	void checkWaiters()
	{
		for (uint i = 0; i < Waiters.size();)
		{
			std::map<uint32, CConnection *>::iterator it = Connections.find(Waiters[i].Connection);
			if (it == Connections.end() || !it->second->Pipe->isOpen() || answerWaiter(*it->second, Waiters[i], false))
				Waiters.erase(Waiters.begin() + i);
			else
				++i;
		}
	}

	/// The lines of a ring a query selects; T has Idx and Time
	template <class T, class TPredicate>
	CJsonValue select(const std::deque<T> &ring, uint64 next, const CJsonValue &args, const TPredicate &pred, const char *listName)
	{
		uint64 first = ring.empty() ? next : ring.front().Idx;
		sint64 last = argInt(args, "last", -1);
		bool hasFrom = args.has("from_idx");
		uint64 from = hasFrom ? (uint64)std::max((sint64)0, argInt(args, "from_idx", 0)) : 0;
		uint64 to = args.has("to_idx") ? (uint64)std::max((sint64)0, argInt(args, "to_idx", 0)) : (uint64)-1;
		sint64 fromTime = argInt(args, "from_time", 0);
		sint64 toTime = argInt(args, "to_time", 0);
		uint maxLines = (uint)std::min((sint64)s_MaxLines, std::max((sint64)1, argInt(args, "max", s_DefaultMaxLines)));
		if (!hasFrom && last < 0 && fromTime == 0)
			last = 100;

		std::vector<const T *> picked;
		size_t startPos = 0;
		if (hasFrom && from > first)
			startPos = (size_t)std::min((uint64)ring.size(), from - first);
		for (size_t i = startPos; i < ring.size(); ++i)
		{
			const T &e = ring[i];
			if (e.Idx > to)
				break;
			if (fromTime != 0 && e.Time < (uint64)fromTime)
				continue;
			if (toTime != 0 && e.Time > (uint64)toTime)
				continue;
			if (!pred(e))
				continue;
			picked.push_back(&e);
		}
		bool truncated = false;
		size_t begin = 0, end = picked.size();
		if (last >= 0 && picked.size() > (size_t)last)
		{
			begin = picked.size() - (size_t)last;
			truncated = true;
		}
		if (end - begin > maxLines)
		{
			// paging forward from an index keeps the oldest, everything else the newest
			if (hasFrom && last < 0)
				end = begin + maxLines;
			else
				begin = end - maxLines;
			truncated = true;
		}
		// with nothing picked, the scan went to the end of the selection: a reader that continues starts there
		uint64 scanEnd = to == (uint64)-1 ? next : std::min(to + 1, next);
		CJsonValue r = CJsonValue::object();
		r.set("first_idx", first);
		r.set("next_idx", end > begin ? picked[end - 1]->Idx + 1 : (hasFrom ? std::max(std::max(from, first), scanEnd) : next));
		r.set("newest_idx", next - 1);
		if (hasFrom && from < first && from > 0)
			r.set("missed", toString("from_idx %u is no longer held: the first index still held is %u", (uint)from, (uint)first));
		r.set("truncated", truncated);
		CJsonValue list = CJsonValue::array();
		for (size_t i = begin; i < end; ++i)
			list.push(toJson(*picked[i]));
		r.set(listName, list);
		return r;
	}

	struct CLogPredicate
	{
		std::set<std::string> Levels;
		std::string Contains;
		bool operator()(const CLogLine &l) const
		{
			if (!Levels.empty() && Levels.find(l.Level) == Levels.end())
				return false;
			if (!Contains.empty() && !containsLowerAscii(l.Text, Contains))
				return false;
			return true;
		}
	};

	struct CEventPredicate
	{
		std::set<std::string> Kinds;
		std::string Contains;
		bool operator()(const CEventLine &e) const
		{
			if (!Kinds.empty() && Kinds.find(e.Kind) == Kinds.end())
				return false;
			if (!Contains.empty() && !containsLowerAscii(e.Data, Contains))
				return false;
			return true;
		}
	};

	static CJsonValue toJson(const CLogLine &l)
	{
		CJsonValue o = CJsonValue::object();
		o.set("idx", l.Idx);
		o.set("time", l.Time);
		o.set("level", l.Level);
		o.set("text", l.Text);
		if (!l.Origin.empty())
			o.set("origin", l.Origin);
		return o;
	}

	static CJsonValue toJson(const CEventLine &e)
	{
		CJsonValue o = CJsonValue::object();
		o.set("idx", e.Idx);
		o.set("time", e.Time);
		o.set("kind", e.Kind);
		CJsonValue data;
		std::string error;
		if (data.parse(e.Data, error))
			o.set("data", data);
		return o;
	}

	CJsonValue queryLog(const CJsonValue &args)
	{
		CLogPredicate pred;
		const CJsonValue &levels = args.get("levels");
		for (uint i = 0; i < levels.size(); ++i)
			pred.Levels.insert(toUpperAscii(levels.at(i).getString()));
		pred.Contains = toLowerAscii(args.get("contains").getString());
		CAutoMutex<CMutex> lock(LogMutex);
		return select(Log, LogNext, args, pred, "lines");
	}

	CJsonValue queryEvents(const CJsonValue &args)
	{
		CEventPredicate pred;
		const CJsonValue &kinds = args.get("kinds");
		for (uint i = 0; i < kinds.size(); ++i)
			pred.Kinds.insert(kinds.at(i).getString());
		pred.Contains = toLowerAscii(args.get("contains").getString());
		CAutoMutex<CMutex> lock(EventMutex);
		return select(Events, EventNext, args, pred, "events");
	}

	bool flushOutbox()
	{
		std::vector<COutMessage> out;
		{
			CAutoMutex<CMutex> lock(OutboxMutex);
			out.swap(Outbox);
		}
		for (uint i = 0; i < out.size(); ++i)
		{
			COutMessage &m = out[i];
			std::map<uint32, CConnection *>::iterator it = Connections.find(m.Connection);
			// the connection went away meanwhile; nobody waits for the answer
			if (it == Connections.end())
				continue;
			CConnection &c = *it->second;
			for (uint k = 0; k < m.Attachments.size(); ++k)
			{
				std::vector<std::string> fields;
				fields.push_back("data");
				fields.push_back(toString("%s.%u", m.Id.c_str(), k + 1));
				fields.push_back(m.Attachments[k].first);
				fields.push_back(toString("%u", (uint)m.Attachments[k].second.size()));
				if (!sendLine(c, joinCtrlLine(fields)) || !c.Pipe->write(m.Attachments[k].second, WriteTimeoutMs))
					break;
			}
			std::vector<std::string> fields;
			fields.push_back(m.Error ? "error" : "result");
			fields.push_back(m.Id);
			if (m.Error)
				fields.push_back(m.Message);
			fields.push_back(m.Json);
			sendLine(c, joinCtrlLine(fields));
		}
		return !out.empty();
	}

	void dropClosed()
	{
		for (std::map<uint32, CConnection *>::iterator it = Connections.begin(); it != Connections.end();)
		{
			CConnection *c = it->second;
			if (c->Pipe->isOpen())
			{
				++it;
				continue;
			}
			nlinfo("CTRL: %s closed", c->Name.c_str());
			if (LeaseHolder == c->Id)
			{
				LeaseHolder = 0;
				LeaseHeld = false;
			}
			{
				CAutoMutex<CMutex> lock(InboxMutex);
				Closed.push_back(c->Id);
			}
			delete c->Pipe;
			delete c;
			Connections.erase(it++);
		}
	}
};

namespace {

/**
 * The displayer on the logs of the process. A log's list of displayers has no lock, so taking one off while
 * other threads log is not safe: this one is put on once and stays, and an endpoint that stops only takes
 * itself off it, under its lock, after which no thread is still in that endpoint's ring. It is never deleted,
 * since a log may display through it until the process ends.
 */
class CCtrlLogForwarder : public IDisplayer
{
public:
	CCtrlLogForwarder()
	    : IDisplayer("CTRL_ENDPOINT")
	    , m_Target(nullptr)
	{
	}

	void setTarget(CCtrlEndpoint::CImpl *target)
	{
		CAutoMutex<CMutex> lock(m_Mutex);
		m_Target = target;
	}

protected:
	virtual void doDisplay(const CLog::TDisplayInfo &args, const char *message) NL_OVERRIDE
	{
		CAutoMutex<CMutex> lock(m_Mutex);
		if (m_Target != nullptr)
			m_Target->addLog(args, message);
	}

private:
	CMutex m_Mutex;
	CCtrlEndpoint::CImpl *m_Target;
};

CCtrlLogForwarder *s_LogForwarder = nullptr;

} // anonymous namespace

namespace {

enum TMethodStep
{
	StepCall,
	StepPoll,
	StepCancel
};

/// Runs one step of a method: what it throws is the call's error, so one method cannot stop the others
void runMethodStep(ICtrlMethod *method, CCtrlCall &call, TMethodStep step)
{
	try
	{
		switch (step)
		{
		case StepCall: method->call(call); break;
		case StepPoll: method->poll(call); break;
		case StepCancel: method->cancel(call); break;
		}
	}
	catch (const std::exception &e)
	{
		nlwarning("CTRL: the method '%s' threw: %s", method->getName().c_str(), e.what());
		call.fail(toString("the method threw: %s", e.what()));
	}
	catch (...)
	{
		nlwarning("CTRL: the method '%s' threw an exception", method->getName().c_str());
		call.fail("the method threw an exception");
	}
}

} // anonymous namespace

// *** CCtrlEndpoint ***

CCtrlEndpoint::CCtrlEndpoint()
    : m_Impl(new CImpl(this))
    , m_Started(false)
    , m_Frame(0)
    , m_InUpdate(false)
    , m_Capture(nullptr)
    , m_CallingConnection(0)
    , m_ConnectionClosed(nullptr)
{
	m_Impl->Pid = CCtrlRegistry::getCurrentProcessId();
	m_Impl->StartTime = CCtrlRegistry::getProcessStartTime(m_Impl->Pid);

	CMethodInfo info;
	info.ReadOnly = true;
	info.BuiltIn = true;
	info.Name = "ctrl.identity";
	info.Description = "Who this process is: name, process id and start time, program, build, log file, and what it adds (account, character).";
	info.Schema = CCtrlSchema();
	m_Impl->Methods.push_back(info);
	info.Name = "ctrl.state";
	info.Description = "The state of the process, with its detail, the frame, and whether the main thread is running or busy for how long in which state. Answered without the main thread.";
	m_Impl->Methods.push_back(info);
	info.Name = "ctrl.wait_state";
	info.Description = "Waits until the state is one of those given (state or states), or differs from 'not', up to timeout_ms (30 s by default, 600 s at most). It ends early with matched false and "
	                   "what stopped it when an event of the stop_on kinds comes (message_box and exit by default), or when the state left the one it had and stays in another settled state for "
	                   "settle_seconds (5 by default; 0 waits on): what was to lead to the state did not.";
	info.Schema = CCtrlSchema()
	                  .str("state", "The state to wait for.")
	                  .strings("states", "Any of these states.")
	                  .str("not", "Wait until the state is no longer this one.")
	                  .integer("timeout_ms", "How long to wait at most.")
	                  .strings("stop_on", "Event kinds that end the wait (message_box and exit by default; an empty list for none).")
	                  .integer("since_idx", "The first event index that counts (by default the events since the last call of this connection).")
	                  .number("settle_seconds", "How long another settled state ends the wait (5 by default, 0 never).");
	m_Impl->Methods.push_back(info);
	info.Name = "ctrl.log";
	info.Description = "Log lines of the process, indexed from 1 since it started, each with its time (ms since the epoch), level and text. Select by index (from_idx, to_idx), time (from_time, to_time) or the last N; filter by levels (INF, WRN, ERR, DBG, AST, STT) and a case-insensitive substring. From an index the oldest lines come first, up to max (200 by default); with wait_ms, waits for a first matching line. next_idx continues; missed says when the ring no longer holds from_idx.";
	info.Schema = CCtrlSchema()
	                  .integer("from_idx", "First index, inclusive.")
	                  .integer("to_idx", "Last index, inclusive.")
	                  .integer("from_time", "Earliest time, ms since the epoch.")
	                  .integer("to_time", "Latest time, ms since the epoch.")
	                  .integer("last", "The last N lines of the selection (100 when nothing else selects).")
	                  .strings("levels", "Only these levels: INF, WRN, ERR, DBG, AST, STT.")
	                  .str("contains", "Only lines containing this, case-insensitive.")
	                  .integer("max", "At most this many lines (200 by default, 5000 at most).")
	                  .integer("wait_ms", "Wait up to this long for a first line when none matches yet.");
	m_Impl->Methods.push_back(info);
	info.Name = "ctrl.events";
	info.Description = "Events of the process, indexed like the log: state changes and what the process reports (kind and data). Same selection as ctrl.log, with kinds instead of levels; with wait_ms it is a long poll from from_idx.";
	info.Schema = CCtrlSchema()
	                  .integer("from_idx", "First index, inclusive.")
	                  .integer("to_idx", "Last index, inclusive.")
	                  .integer("last", "The last N events of the selection.")
	                  .strings("kinds", "Only these kinds.")
	                  .str("contains", "Only events whose data contains this, case-insensitive.")
	                  .integer("max", "At most this many events (200 by default, 5000 at most).")
	                  .integer("wait_ms", "Wait up to this long for a first event when none matches yet.");
	m_Impl->Methods.push_back(info);
	info.Name = "ctrl.methods";
	info.Description = "The methods of this endpoint, filtered by name prefix or a word of the name or description; schemas only when asked.";
	info.Schema = CCtrlSchema().str("prefix", "Only names starting with this.").str("contains", "Only methods whose name or description contains this.").boolean("schemas", "Include the argument schemas.");
	m_Impl->Methods.push_back(info);
	info.Name = "ctrl.lease";
	info.Description = "The lease that methods which change something need: take, release, or status. A connection that calls such a method takes it when nobody holds it; it is released when that connection closes.";
	info.Schema = CCtrlSchema().choice("action", "take|release|status", "What to do (status by default).");
	m_Impl->Methods.push_back(info);
}

CCtrlEndpoint::~CCtrlEndpoint()
{
	if (m_Started)
		stop(m_ExitReason.empty() ? std::string("the endpoint was destroyed") : m_ExitReason);
	for (std::map<std::string, ICtrlMethod *>::iterator it = m_Methods.begin(); it != m_Methods.end(); ++it)
		delete it->second;
	delete m_Impl;
}

namespace {

/// A built-in method that runs on the main thread, as a member of the endpoint
class CEndpointMethod : public ICtrlMethod
{
public:
	typedef void (CCtrlEndpoint::*TMember)(CCtrlCall &call);

	CEndpointMethod(CCtrlEndpoint *endpoint, TMember member, const std::string &name, const std::string &description, const CJsonValue &schema, bool readOnly)
	    : ICtrlMethod(name, description, schema, readOnly)
	    , m_Endpoint(endpoint)
	    , m_Member(member)
	{
	}

	virtual void call(CCtrlCall &call) NL_OVERRIDE
	{
		(m_Endpoint->*m_Member)(call);
	}

private:
	CCtrlEndpoint *m_Endpoint;
	TMember m_Member;
};

class CCommandCollector : public ICommandRegistryListener
{
public:
	std::string Prefix;
	std::string Contains;
	uint Limit;
	uint Total;
	CJsonValue List;

	CCommandCollector()
	    : Limit(300)
	    , Total(0)
	    , List(CJsonValue::array())
	{
	}

	virtual void onCommandAdded(const std::string &name, TCommandEntryKind kind, const std::string &args, const std::string &help) NL_OVERRIDE
	{
		if (!Prefix.empty() && name.compare(0, Prefix.size(), Prefix) != 0)
			return;
		if (!Contains.empty() && toLowerAscii(name + " " + help).find(Contains) == std::string::npos)
			return;
		++Total;
		if (List.size() >= Limit)
			return;
		CJsonValue e = CJsonValue::object();
		e.set("name", name);
		e.set("kind", kind == HandlerEntryKind ? "handler" : (kind == HandlerCommandEntryKind ? "member" : "command"));
		if (!args.empty())
			e.set("args", args);
		if (!help.empty())
			e.set("help", help);
		List.push(e);
	}

	virtual void onCommandRemoved(const std::string & /* name */) NL_OVERRIDE { }
};

} // anonymous namespace

void CCtrlEndpoint::runCommandsMethod(CCtrlCall &call)
{
	CCommandCollector collector;
	collector.Prefix = call.getArgs().get("prefix").getString();
	collector.Contains = toLowerAscii(call.getArgs().get("contains").getString());
	collector.Limit = (uint)std::max((sint64)1, call.getArgs().get("limit").getInt(300));
	CCommandRegistry::getInstance().listCommands(collector);
	CJsonValue r = CJsonValue::object();
	r.set("commands", collector.List);
	r.set("total", collector.Total);
	r.set("truncated", collector.Total > collector.List.size());
	call.reply(r);
}

void CCtrlEndpoint::runRunCommandMethod(CCtrlCall &call)
{
	const std::string &command = call.getArgs().get("command").getString();
	if (command.empty())
	{
		call.fail("command is required");
		return;
	}
	// a whole command line as the name would run nothing: with a dot in it, it reads as an object of commands that is
	// not there, which NeL counts as success
	if (command.find_first_of(" \t") != std::string::npos)
	{
		call.fail("command is the name of the command only; its arguments go in args");
		return;
	}
	std::vector<std::string> args;
	const CJsonValue &list = call.getArgs().get("args");
	for (uint i = 0; i < list.size(); ++i)
	{
		const CJsonValue &a = list.at(i);
		args.push_back(a.isString() ? a.getString() : a.toString());
	}

	// a private log around the command, as a remote admin command of a client has it: the lines
	// of other threads meanwhile do not end up in the output
	CLog log;
	CLightMemDisplayer output;
	output.setParam(100000);
	log.addDisplayer(&output);
	// and what it shows outside its log meanwhile; a command run from inside a command keeps the outer list
	std::vector<std::string> shown;
	std::vector<std::string> *previous = m_Capture;
	m_Capture = &shown;
	bool ok = ICommand::executeArgs(command, args, log, true, true);
	m_Capture = previous;
	log.removeDisplayer(&output);

	CJsonValue lines = CJsonValue::array();
	const std::deque<std::string> &strings = output.lockStrings();
	for (std::deque<std::string>::const_iterator it = strings.begin(); it != strings.end(); ++it)
	{
		std::string line = *it;
		while (!line.empty() && (line[line.size() - 1] == '\n' || line[line.size() - 1] == '\r'))
			line.erase(line.size() - 1);
		lines.push(CJsonValue(line));
	}
	output.unlockStrings();

	CJsonValue r = CJsonValue::object();
	r.set("command", command);
	r.set("ok", ok);
	r.set("output", lines);
	CJsonValue shownList = CJsonValue::array();
	for (uint i = 0; i < shown.size(); ++i)
		shownList.push(CJsonValue(shown[i]));
	r.set("shown", shownList);
	call.reply(r);
}

bool CCtrlEndpoint::start(const std::string &name, const CJsonValue &identity, std::string &error)
{
	nlassert(!m_Started);
	if (!CCtrlRegistry::isValidName(name))
	{
		error = toString("'%s' is not a name for an endpoint: letters, digits, '_', '-' and '.', 64 at most", name.c_str());
		return false;
	}
	m_Name = name;
	std::string dir;
	if (!CCtrlRegistry::getDirectory(dir, true, error))
		return false;

	m_Impl->RecordPath = CCtrlRegistry::recordPath(dir, name, m_Impl->Pid);
	m_Impl->EndpointPath = CCtrlRegistry::endpointPath(dir, name, m_Impl->Pid);

	CJsonValue id = identity.isObject() ? identity : CJsonValue::object();
	id.set("name", name);
	id.set("pid", m_Impl->Pid);
	id.set("start_time", m_Impl->StartTime);
	id.set("endpoint", m_Impl->EndpointPath);
	id.set("protocol", CtrlProtocolVersion);
	{
		CAutoMutex<CMutex> lock(m_Impl->IdentityMutex);
		m_Impl->Identity = id;
	}

	// another live process may use the name already: tools address endpoints by name
	std::vector<std::string> records;
	CCtrlRegistry::listRecords(dir, records);
	for (uint i = 0; i < records.size(); ++i)
	{
		CJsonValue other;
		std::string readError;
		if (records[i] == m_Impl->RecordPath || !CCtrlRegistry::readRecord(records[i], other, readError))
			continue;
		if (other.get("name").getString() != name || other.get("state").getString() != "running")
			continue;
		uint32 pid = (uint32)other.get("pid").getInt(0);
		uint64 started = (uint64)other.get("start_time").getInt(0);
		if (pid != 0 && started != 0 && CCtrlRegistry::getProcessStartTime(pid) == started)
		{
			error = toString("the endpoint name '%s' is used by process %u", name.c_str(), pid);
			m_Impl->Identity.set("error", error);
			writeRecord("failed");
			return false;
		}
	}

	if (!m_Impl->Listener.listen(m_Impl->EndpointPath, error))
	{
		m_Impl->Identity.set("error", error);
		writeRecord("failed");
		return false;
	}

	addMethod(new CEndpointMethod(this, &CCtrlEndpoint::runCommandsMethod, "ctrl.commands",
	    "The command table of the process: commands and variables with their argument syntax and help, named handler instances and their member commands (instance.command). Filter by prefix or a word of the name or help.",
	    CCtrlSchema()
	        .str("prefix", "Only names starting with this (case sensitive, as the commands are).")
	        .str("contains", "Only entries whose name or help contains this (case-insensitive).")
	        .integer("limit", "At most this many entries (300 by default)."),
	    true));
	addMethod(new CEndpointMethod(this, &CCtrlEndpoint::runRunCommandMethod, "ctrl.run_command",
	    "Runs a command, or reads or sets a variable, of the command registry, with the arguments as a list: they reach the command as given, nothing is parsed. Returns whether it succeeded, what it wrote to its log, and what it showed elsewhere meanwhile (the chat of a client).",
	    CCtrlSchema().str("command", "The command or variable, instance.command for a member command.", true).strings("args", "The arguments, one per entry."),
	    false));

	// every log of the process, from here on
	if (s_LogForwarder == nullptr)
		s_LogForwarder = new CCtrlLogForwarder();
	INelContext &context = INelContext::getInstance();
	CLog *logs[] = { context.getErrorLog(), context.getWarningLog(), context.getInfoLog(), context.getDebugLog(), context.getAssertLog() };
	for (uint i = 0; i < sizeof(logs) / sizeof(logs[0]); ++i)
		if (logs[i] != nullptr && !logs[i]->attached(s_LogForwarder))
			logs[i]->addDisplayer(s_LogForwarder);
	s_LogForwarder->setTarget(m_Impl);

	m_Impl->Running = true;
	m_Impl->Thread = IThread::create(m_Impl);
	m_Impl->Thread->start();
	m_Started = true;
	writeRecord("running");
	nlinfo("CTRL: endpoint '%s' listens on '%s'; whoever opens it acts as this user", name.c_str(), m_Impl->EndpointPath.c_str());
	return true;
}

void CCtrlEndpoint::stop(const std::string &exitReason)
{
	if (!m_Started)
		return;
	m_Started = false;
	nlinfo("CTRL: endpoint '%s' stops: %s", m_Name.c_str(), exitReason.c_str());
	{
		CAutoMutex<CMutex> lock(m_Impl->IdentityMutex);
		m_Impl->Identity.set("exit_reason", exitReason);
	}
	writeRecord("exited");

	m_Impl->Running = false;
	if (m_Impl->Thread != nullptr)
	{
		m_Impl->Thread->wait();
		delete m_Impl->Thread;
		m_Impl->Thread = nullptr;
	}
	m_Impl->Listener.close();

	// the forwarder stays on the logs; once it no longer points here, no thread is in the ring
	s_LogForwarder->setTarget(nullptr);
	m_Impl->Deferred.clear();

	for (uint i = 0; i < m_Open.size(); ++i)
	{
		std::map<std::string, ICtrlMethod *>::iterator it = m_Methods.find(m_Open[i]->getMethod());
		if (it != m_Methods.end())
			runMethodStep(it->second, *m_Open[i], StepCancel);
		delete m_Open[i];
	}
	m_Open.clear();
}

void CCtrlEndpoint::writeRecord(const std::string &state)
{
	CJsonValue record;
	{
		CAutoMutex<CMutex> lock(m_Impl->IdentityMutex);
		record = m_Impl->Identity;
	}
	record.set("state", state);
	record.set("updated", getCtrlTimeMs());
	std::string error;
	if (!CCtrlRegistry::writeRecord(m_Impl->RecordPath, record, error))
		nlwarning("CTRL: %s", error.c_str());
}

void CCtrlEndpoint::setIdentity(const std::string &key, const CJsonValue &value)
{
	{
		CAutoMutex<CMutex> lock(m_Impl->IdentityMutex);
		if (m_Impl->Identity.get(key).toString() == value.toString())
			return;
		m_Impl->Identity.set(key, value);
	}
	if (m_Started)
		writeRecord("running");
}

void CCtrlEndpoint::setExitReason(const std::string &reason)
{
	m_ExitReason = reason;
}

void CCtrlEndpoint::addMethod(ICtrlMethod *method)
{
	nlassert(method != nullptr);
	std::map<std::string, ICtrlMethod *>::iterator it = m_Methods.find(method->getName());
	nlassertex(it == m_Methods.end(), ("CTRL: the method '%s' is added twice", method->getName().c_str()));
	m_Methods[method->getName()] = method;
	CMethodInfo info;
	info.Name = method->getName();
	info.Description = method->getDescription();
	info.Schema = method->getSchema();
	info.ReadOnly = method->isReadOnly();
	info.BuiltIn = false;
	CAutoMutex<CMutex> lock(m_Impl->MethodsMutex);
	m_Impl->Methods.push_back(info);
}

void CCtrlEndpoint::setState(const std::string &state, const CJsonValue &detail, bool settled)
{
	std::string detailText = detail.isNull() ? std::string() : detail.toString();
	std::string previous;
	{
		CAutoMutex<CMutex> lock(m_Impl->StateMutex);
		m_Impl->StateSettled = settled;
		if (m_Impl->State == state && m_Impl->StateDetail == detailText)
			return;
		previous = m_Impl->State;
		bool changed = previous != state;
		m_Impl->State = state;
		m_Impl->StateDetail = detailText;
		if (changed)
			m_Impl->StateSince = getCtrlTimeMs();
		if (!changed)
			return;
	}
	CJsonValue e = CJsonValue::object();
	e.set("from", previous);
	e.set("to", state);
	if (!detail.isNull())
		e.set("detail", detail);
	m_Impl->addEvent("state", e);
}

void CCtrlEndpoint::setFrame(uint64 frame)
{
	m_Frame = frame;
}

void CCtrlEndpoint::addEvent(const std::string &kind, const CJsonValue &data)
{
	m_Impl->addEvent(kind, data);
}

void CCtrlEndpoint::captureOutput(const std::string &line)
{
	if (m_Capture != nullptr && !line.empty())
		m_Capture->push_back(line);
}

uint32 CCtrlEndpoint::getCallingConnection() const
{
	return m_CallingConnection;
}

bool CCtrlEndpoint::isLeaseHeld() const
{
	return m_Started && m_Impl->LeaseHeld;
}

void CCtrlEndpoint::setConnectionClosedCallback(TConnectionClosed callback)
{
	m_ConnectionClosed = callback;
}

namespace {

/// The re-entrancy guard of update(), reset however update() leaves
class CUpdateGuard
{
public:
	CUpdateGuard(bool &flag)
	    : m_Flag(flag)
	{
		m_Flag = true;
	}
	~CUpdateGuard() { m_Flag = false; }

private:
	bool &m_Flag;
};

COutMessage cancelledMessage(CCtrlEndpoint::CImpl *impl, uint32 connection, const std::string &id);

} // anonymous namespace

void CCtrlEndpoint::update(bool allowControl)
{
	if (!m_Started)
		return;
	// the main thread is alive, also when a method pumps the events of the process and lands here again
	{
		CAutoMutex<CMutex> lock(m_Impl->StateMutex);
		m_Impl->Frame = m_Frame;
		m_Impl->LastUpdate = CTime::getLocalTime();
	}
	if (m_InUpdate)
		return;
	CUpdateGuard guard(m_InUpdate);

	// the calls that waited for an update that lets them run come first, in the order they came
	std::vector<CInCall> inbox;
	inbox.swap(m_Impl->Deferred);
	std::vector<std::pair<uint32, std::string>> cancels;
	std::vector<uint32> closedList;
	{
		CAutoMutex<CMutex> lock(m_Impl->InboxMutex);
		inbox.insert(inbox.end(), m_Impl->Inbox.begin(), m_Impl->Inbox.end());
		m_Impl->Inbox.clear();
		cancels.swap(m_Impl->Cancels);
		closedList.swap(m_Impl->Closed);
	}
	// every call of a connection that closed is dropped in this update, queued, deferred or open: none is left
	// for a later one, so the ids are not kept
	std::set<uint32> closed(closedList.begin(), closedList.end());

	std::vector<COutMessage> out;

	// calls of a connection that closed, and calls cancelled, are dropped: those still queued too
	for (uint i = 0; i < m_Open.size();)
	{
		CCtrlCall *call = m_Open[i];
		bool gone = closed.find(call->getConnection()) != closed.end();
		bool cancelled = false;
		for (uint k = 0; k < cancels.size() && !cancelled; ++k)
			cancelled = cancels[k].first == call->getConnection() && cancels[k].second == call->getId();
		if (!gone && !cancelled)
		{
			++i;
			continue;
		}
		std::map<std::string, ICtrlMethod *>::iterator method = m_Methods.find(call->getMethod());
		m_CallingConnection = call->getConnection();
		if (method != m_Methods.end())
			runMethodStep(method->second, *call, StepCancel);
		m_CallingConnection = 0;
		if (!gone)
			out.push_back(cancelledMessage(m_Impl, call->getConnection(), call->getId()));
		delete call;
		m_Open.erase(m_Open.begin() + i);
	}

	// the calls open since an earlier update are polled; those that start now are not, call() just ran
	size_t polled = m_Open.size();
	for (uint i = 0; i < inbox.size(); ++i)
	{
		const CInCall &in = inbox[i];
		if (closed.find(in.Connection) != closed.end())
			continue;
		bool cancelled = false;
		for (uint k = 0; k < cancels.size() && !cancelled; ++k)
			cancelled = cancels[k].first == in.Connection && cancels[k].second == in.Id;
		if (cancelled)
		{
			out.push_back(cancelledMessage(m_Impl, in.Connection, in.Id));
			continue;
		}
		std::map<std::string, ICtrlMethod *>::iterator it = m_Methods.find(in.Method);
		if (it == m_Methods.end())
			continue;
		// a pump deep in other code (a loading screen) runs what only reads; the rest waits for the next full update
		if (!allowControl && !it->second->isReadOnly())
		{
			m_Impl->Deferred.push_back(in);
			continue;
		}
		CCtrlCall *call = new CCtrlCall();
		call->m_Connection = in.Connection;
		call->m_Id = in.Id;
		call->m_Method = in.Method;
		call->m_Args = in.Args;
		call->m_StartTime = CTime::getLocalTime();
		m_CallingConnection = in.Connection;
		runMethodStep(it->second, *call, StepCall);
		m_CallingConnection = 0;
		m_Open.push_back(call);
	}

	// the open calls: an answered one goes out
	for (uint i = 0; i < m_Open.size();)
	{
		CCtrlCall *call = m_Open[i];
		if (!call->isDone() && i < polled)
		{
			m_CallingConnection = call->getConnection();
			runMethodStep(m_Methods[call->getMethod()], *call, StepPoll);
			m_CallingConnection = 0;
		}
		if (!call->isDone())
		{
			++i;
			continue;
		}
		if (i < polled)
			--polled;
		COutMessage m;
		m.Connection = call->getConnection();
		m.Id = call->getId();
		m.Error = call->m_Error;
		m.Message = call->m_Message;
		m.Json = m_Impl->wrap(call->m_Result).toString();
		m.Attachments.swap(call->m_Attachments);
		out.push_back(m);
		delete call;
		m_Open.erase(m_Open.begin() + i);
	}

	if (!out.empty())
	{
		CAutoMutex<CMutex> lock(m_Impl->OutboxMutex);
		m_Impl->Outbox.insert(m_Impl->Outbox.end(), out.begin(), out.end());
	}

	// the connections that closed, once their calls are gone
	if (m_ConnectionClosed != nullptr)
		for (uint i = 0; i < closedList.size(); ++i)
			m_ConnectionClosed(closedList[i]);
}

namespace {

COutMessage cancelledMessage(CCtrlEndpoint::CImpl *impl, uint32 connection, const std::string &id)
{
	COutMessage m;
	m.Connection = connection;
	m.Id = id;
	m.Error = true;
	m.Message = "cancelled";
	m.Json = impl->wrap(CJsonValue()).toString();
	return m;
}

} // anonymous namespace

} // NLMISC

/* End of ctrl_endpoint.cpp */
