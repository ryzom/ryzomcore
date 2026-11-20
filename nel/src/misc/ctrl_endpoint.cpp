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
#include "nel/misc/mutex.h"
#include "nel/misc/path.h"
#include "nel/misc/thread.h"

#include <deque>
#include <set>

#ifndef NL_OS_WINDOWS
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
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
// every connection has its own queue of answers, written without waiting: a reader that takes none of it for this
// long, or lets it grow past the size after it, is closed, and nobody else waits on it meanwhile
const TTime s_StallTimeoutMs = 10000;
const size_t s_MaxQueuedBytes = 128 * 1024 * 1024;
// the last answers when the endpoint stops: a reader that does not take them at once does not hold up the exit
const TTime s_FinalFlushMs = 500;
// the data that goes with one answer, at most
const size_t s_MaxAttachmentBytes = 64 * 1024 * 1024;
const uint s_TickMs = 5;
const sint64 s_DefaultWaitMs = 30000;
const sint64 s_MaxWaitMs = 600000;
const uint s_DefaultMaxLines = 200;
const uint s_MaxLines = 5000;
// what the tools may have going at once: connections, and per connection calls not answered yet with the size of
// their arguments, and waits
const uint s_MaxConnections = 32;
const uint s_MaxPendingCalls = 64;
const size_t s_MaxPendingBytes = 32 * 1024 * 1024;
const uint s_MaxWaiters = 32;
// a peer that sends this many messages of no known type is out of step, and closed
const uint s_MaxUnknownMessages = 100;
const size_t s_MaxIdLength = 128;
const size_t s_MaxNameLength = 128;
// what ctrl.run_command keeps of the output of a command
const uint s_MaxCommandLines = 5000;
const size_t s_MaxCommandBytes = 4 * 1024 * 1024;

sint64 argInt(const CJsonValue &args, const char *name, sint64 defaultValue)
{
	return args.get(name).getInt(defaultValue);
}

std::string indexText(uint64 index)
{
	return toString("%" NL_I64 "u", index);
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
    , m_AttachmentBytes(0)
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
	if (m_Done)
		return CJsonValue();
	if (bytes.size() > s_MaxAttachmentBytes - m_AttachmentBytes)
	{
		// nothing of it goes out: the answer is the error
		uint total = (uint)((m_AttachmentBytes + bytes.size()) / (1024 * 1024));
		m_Attachments.clear();
		m_AttachmentBytes = 0;
		fail(toString("the data of the answer would be %u MB, more than the %u MB an answer carries", total, (uint)(s_MaxAttachmentBytes / (1024 * 1024))));
		return CJsonValue();
	}
	m_Attachments.push_back(TAttachment(contentType, std::string()));
	m_Attachments.back().second = bytes;
	m_AttachmentBytes += bytes.size();
	CJsonValue ref = CJsonValue::object();
	ref.set("data", m_Id + toString(".%u", (uint)m_Attachments.size()));
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
    , m_Privileged(false)
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

/// A call for the main thread
struct CInCall
{
	uint32 Connection;
	std::string Id;
	std::string Method;
	CJsonValue Args;

	CInCall()
	    : Connection(0)
	{
	}

	void swap(CInCall &other)
	{
		std::swap(Connection, other.Connection);
		Id.swap(other.Id);
		Method.swap(other.Method);
		Args.swap(other.Args);
	}
};

/// An answer of the main thread, with its data
struct COutMessage
{
	uint32 Connection;
	std::string Id;
	bool Error;
	std::string Message;
	std::string Json;
	std::vector<CCtrlCall::TAttachment> Attachments;

	COutMessage()
	    : Connection(0)
	    , Error(false)
	{
	}

	void swap(COutMessage &other)
	{
		std::swap(Connection, other.Connection);
		Id.swap(other.Id);
		std::swap(Error, other.Error);
		Message.swap(other.Message);
		Json.swap(other.Json);
		Attachments.swap(other.Attachments);
	}
};

// connection and call id
typedef std::pair<uint32, std::string> TCallRef;

struct CMethodInfo
{
	std::string Name;
	std::string Description;
	CJsonValue Schema;
	bool ReadOnly;
	bool Privileged;
	bool BuiltIn;
};

/// A call of a connection that went to the main thread and has no answer yet
struct CPendingCall
{
	bool Control;
	size_t Bytes;
	bool CancelSent;
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
	// by id, its calls the main thread has not answered yet; how many of them need the lease, and the size of their
	// arguments
	std::map<std::string, CPendingCall> Pending;
	uint ControlPending;
	size_t PendingBytes;
	uint UnknownMessages;

	CConnection()
	    : Id(0)
	    , Pipe(nullptr)
	    , Reader(s_MaxInLine, 0)
	    , LastCallEvent(0)
	    , ControlPending(0)
	    , PendingBytes(0)
	    , UnknownMessages(0)
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
	// what a scan said of indexes the ring no longer held
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

/// The state as it was at one moment, read under the lock and turned into JSON after
struct CStateCopy
{
	std::string State;
	std::string Detail;
	bool Settled;
	uint64 Since;
	uint64 Frame;
	TTime LastUpdate;

	CStateCopy()
	    : Settled(true)
	    , Since(0)
	    , Frame(0)
	    , LastUpdate(0)
	{
	}
};

/// What a query of a ring selected: copies of the lines, taken under the lock of the ring, and where the selection
/// stands; the JSON is built after, without the lock
template <class T>
struct CSelection
{
	std::vector<T> Picked;
	uint64 First;
	uint64 NextIdx;
	uint64 Newest;
	bool Truncated;
	std::string Missed;

	CSelection()
	    : First(0)
	    , NextIdx(0)
	    , Newest(0)
	    , Truncated(false)
	{
	}
};

bool matchesState(const CJsonValue &args, const std::string &state)
{
	bool match = false;
	if (args.get("state").isString())
		match = state == args.get("state").getString();
	const CJsonValue &states = args.get("states");
	for (uint i = 0; i < states.size(); ++i)
		match = match || state == states.at(i).getString();
	if (args.get("not").isString())
		match = match || (state != args.get("not").getString() && !args.has("state") && !args.has("states"));
	return match;
}

CJsonValue stateJson(const CStateCopy &copy)
{
	CJsonValue s = CJsonValue::object();
	s.set("state", copy.State);
	CJsonValue detail;
	std::string error;
	if (!copy.Detail.empty() && detail.parse(copy.Detail, error))
		s.set("detail", detail);
	s.set("since", copy.Since);
	s.set("frame", copy.Frame);
	TTime now = CTime::getLocalTime();
	sint64 idle = copy.LastUpdate == 0 ? -1 : (sint64)(now - copy.LastUpdate);
	s.set("main_thread_idle_ms", idle);
	// a frame of a client is far below this; a main thread that does not come back is stuck in something
	std::string mainThread = "running";
	if (idle < 0)
		mainThread = "not started";
	else if (idle > 2000)
		mainThread = toString("busy for %u s in ", (uint)(idle / 1000)) + copy.State;
	s.set("main_thread", mainThread);
	return s;
}

CJsonValue toJson(const CLogLine &l)
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

CJsonValue toJson(const CEventLine &e)
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

template <class T>
CJsonValue selectionJson(const CSelection<T> &selection, const char *listName)
{
	CJsonValue r = CJsonValue::object();
	r.set("first_idx", selection.First);
	r.set("next_idx", selection.NextIdx);
	r.set("newest_idx", selection.Newest);
	if (!selection.Missed.empty())
		r.set("missed", selection.Missed);
	r.set("truncated", selection.Truncated);
	CJsonValue list = CJsonValue::array();
	for (size_t i = 0; i < selection.Picked.size(); ++i)
		list.push(toJson(selection.Picked[i]));
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

/// The lines of a ring a query selects; T has Idx and Time. The ring is locked by the caller.
template <class T, class TPredicate>
void selectLines(const std::deque<T> &ring, uint64 next, const CJsonValue &args, const TPredicate &pred, CSelection<T> &selection)
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
	selection.First = first;
	selection.NextIdx = end > begin ? picked[end - 1]->Idx + 1 : (hasFrom ? std::max(std::max(from, first), scanEnd) : next);
	selection.Newest = next - 1;
	selection.Truncated = truncated;
	// indexes start at 1: from_idx 0 asks for everything since the start too
	if (hasFrom && std::max(from, (uint64)1) < first)
		selection.Missed = "from_idx " + indexText(from) + " is no longer held: the first index still held is " + indexText(first);
	selection.Picked.reserve(end - begin);
	for (size_t i = begin; i < end; ++i)
		selection.Picked.push_back(*picked[i]);
}

} // anonymous namespace

class CCtrlEndpoint::CImpl : public IRunnable
{
public:
	CImpl()
	    : m_Pid(0)
	    , m_StartTime(0)
	    , m_Thread(nullptr)
	    , m_Running(false)
	    , m_LogNext(1)
	    , m_LogBytes(0)
	    , m_EventNext(1)
	    , m_EventBytes(0)
	    , m_StateSettled(true)
	    , m_StateSince(0)
	    , m_Frame(0)
	    , m_LastUpdate(0)
	    , m_StopPending(false)
	    , m_NextConnection(1)
	    , m_RefusedWarned(false)
	    , m_LeaseHolder(0)
	    , m_LeaseHeld(false)
	    , m_Trusted(false)
	{
	}

	CLocalPipeListener m_Listener;
	CCtrlNameLock m_NameLock;
	std::string m_RecordPath;
	std::string m_EndpointPath;
	uint32 m_Pid;
	uint64 m_StartTime;
	IThread *m_Thread;
	volatile bool m_Running;

	// identity, as the hello and the record carry it
	CMutex m_IdentityMutex;
	CJsonValue m_Identity;

	CMutex m_LogMutex;
	std::deque<CLogLine> m_Log;
	uint64 m_LogNext;
	size_t m_LogBytes;

	CMutex m_EventMutex;
	std::deque<CEventLine> m_Events;
	uint64 m_EventNext;
	size_t m_EventBytes;

	CMutex m_StateMutex;
	std::string m_State;
	std::string m_StateDetail;
	bool m_StateSettled;
	uint64 m_StateSince;
	uint64 m_Frame;
	TTime m_LastUpdate;

	CMutex m_MethodsMutex;
	std::vector<CMethodInfo> m_Methods;

	CMutex m_InboxMutex;
	std::deque<CInCall> m_Inbox;
	std::vector<TCallRef> m_Cancels;
	std::vector<uint32> m_Closed;
	// set when the endpoint stops: the answer of every call that comes after, and of the waits still open
	std::string m_ExitMessage;

	CMutex m_OutboxMutex;
	std::deque<COutMessage> m_Outbox;

	// (MT) the calls a partial update left for a full one
	std::deque<CInCall> m_Deferred;
	// (MT) stop() was called from a method: update() finishes it as it returns
	bool m_StopPending;

	// (CT)
	std::map<uint32, CConnection *> m_Connections;
	uint32 m_NextConnection;
	bool m_RefusedWarned;
	std::vector<CWaiter> m_Waiters;
	uint32 m_LeaseHolder;
	// (CT writes, AT reads) whether any connection holds it
	volatile bool m_LeaseHeld;
	// (MT writes before the thread starts, CT reads) whether the content of the process is trusted
	bool m_Trusted;

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
		CAutoMutex<CMutex> lock(m_LogMutex);
		for (;;)
		{
			const char *end = strchr(start, '\n');
			size_t len = end ? (size_t)(end - start) : strlen(start);
			if (len > 0 && start[len - 1] == '\r')
				--len;
			if (len > 0)
			{
				CLogLine line;
				line.Idx = m_LogNext++;
				line.Time = now;
				line.Level = level;
				line.Text.assign(start, len);
				line.Origin = origin;
				m_LogBytes += line.Text.size() + line.Origin.size() + 64;
				m_Log.push_back(line);
			}
			if (end == nullptr || end[1] == 0)
				break;
			start = end + 1;
		}
		while (m_LogBytes > s_LogBudget && !m_Log.empty())
		{
			m_LogBytes -= m_Log.front().Text.size() + m_Log.front().Origin.size() + 64;
			m_Log.pop_front();
		}
	}

	void addEvent(const std::string &kind, const CJsonValue &data)
	{
		CEventLine e;
		e.Time = getCtrlTimeMs();
		e.Kind = kind;
		e.Data = data.toString();
		CAutoMutex<CMutex> lock(m_EventMutex);
		e.Idx = m_EventNext++;
		m_EventBytes += e.Kind.size() + e.Data.size() + 48;
		m_Events.push_back(e);
		while (m_EventBytes > s_EventBudget && !m_Events.empty())
		{
			m_EventBytes -= m_Events.front().Kind.size() + m_Events.front().Data.size() + 48;
			m_Events.pop_front();
		}
	}

	// *** state (AT) ***

	CStateCopy readState()
	{
		CStateCopy s;
		CAutoMutex<CMutex> lock(m_StateMutex);
		s.State = m_State;
		s.Detail = m_StateDetail;
		s.Settled = m_StateSettled;
		s.Since = m_StateSince;
		s.Frame = m_Frame;
		s.LastUpdate = m_LastUpdate;
		return s;
	}

	/// What every answer carries: the frame, the clock and the state, so it lines up with the log
	CJsonValue wrap(const CJsonValue &result)
	{
		uint64 frame;
		std::string state;
		{
			CAutoMutex<CMutex> lock(m_StateMutex);
			frame = m_Frame;
			state = m_State;
		}
		CJsonValue o = CJsonValue::object();
		o.set("result", result);
		o.set("frame", frame);
		o.set("time", getCtrlTimeMs());
		o.set("state", state);
		return o;
	}

	/// (MT) An answer for the channel thread to send; m is left empty
	void postAnswer(COutMessage &m)
	{
		CAutoMutex<CMutex> lock(m_OutboxMutex);
		m_Outbox.push_back(COutMessage());
		m_Outbox.back().swap(m);
	}

	/// (MT) An error for a call that gets no other answer
	void postError(uint32 connection, const std::string &id, const std::string &message)
	{
		COutMessage m;
		m.Connection = connection;
		m.Id = id;
		m.Error = true;
		m.Message = message;
		m.Json = wrap(CJsonValue()).toString();
		postAnswer(m);
	}

	// *** the thread ***

	virtual void run() NL_OVERRIDE
	{
		while (m_Running)
		{
			bool busy = false;
			acceptConnections();
			for (std::map<uint32, CConnection *>::iterator it = m_Connections.begin(); it != m_Connections.end(); ++it)
				busy = readConnection(*it->second) || busy;
			takeOutbox();
			busy = flushConnections() || busy;
			checkWaiters();
			dropClosed();
			if (!busy)
				nlSleep(s_TickMs);
		}
		// the answers given meanwhile, the one to a quit method among them, and why the waits end, then everyone is told
		// by the pipe closing
		takeOutbox();
		std::string exitMessage;
		{
			CAutoMutex<CMutex> lock(m_InboxMutex);
			exitMessage = m_ExitMessage;
		}
		for (uint i = 0; i < m_Waiters.size(); ++i)
		{
			std::map<uint32, CConnection *>::iterator it = m_Connections.find(m_Waiters[i].Connection);
			if (it != m_Connections.end())
				sendError(*it->second, m_Waiters[i].Id, exitMessage);
		}
		m_Waiters.clear();
		TTime deadline = CTime::getLocalTime() + s_FinalFlushMs;
		for (;;)
		{
			bool queued = false;
			for (std::map<uint32, CConnection *>::iterator it = m_Connections.begin(); it != m_Connections.end(); ++it)
			{
				CLocalPipe *pipe = it->second->Pipe;
				if (pipe->flush() && pipe->getQueuedBytes() > 0)
					queued = true;
			}
			if (!queued || CTime::getLocalTime() >= deadline)
				break;
			nlSleep(1);
		}
		{
			CAutoMutex<CMutex> lock(m_InboxMutex);
			for (std::map<uint32, CConnection *>::iterator it = m_Connections.begin(); it != m_Connections.end(); ++it)
				m_Closed.push_back(it->first);
		}
		for (std::map<uint32, CConnection *>::iterator it = m_Connections.begin(); it != m_Connections.end(); ++it)
		{
			delete it->second->Pipe;
			delete it->second;
		}
		m_Connections.clear();
		m_Waiters.clear();
		m_LeaseHolder = 0;
		m_LeaseHeld = false;
	}

	void acceptConnections()
	{
		for (;;)
		{
			CLocalPipe *pipe = m_Listener.accept();
			if (pipe == nullptr)
				return;
			if (m_Connections.size() >= s_MaxConnections)
			{
				if (!m_RefusedWarned)
					nlwarning("CTRL: refused a connection: %u are open, the most this endpoint takes", (uint)m_Connections.size());
				m_RefusedWarned = true;
				delete pipe;
				continue;
			}
			m_RefusedWarned = false;
			CConnection *c = new CConnection();
			c->Id = m_NextConnection++;
			c->Pipe = pipe;
			c->Name = toString("connection %u", c->Id);
			m_Connections[c->Id] = c;
			std::vector<std::string> fields;
			fields.push_back("hello");
			fields.push_back(toString("%u", CtrlProtocolVersion));
			fields.push_back(helloIdentity(c->Id).toString());
			queueLine(*c, joinCtrlLine(fields));
			nlinfo("CTRL: %s opened", c->Name.c_str());
		}
	}

	CJsonValue helloIdentity(uint32 connection)
	{
		CJsonValue id;
		{
			CAutoMutex<CMutex> lock(m_IdentityMutex);
			id = m_Identity;
		}
		// the endpoint's own, never what the identity of the process says
		id.set("trusted", m_Trusted);
		id.set("connection", connection);
		return id;
	}

	/// Queues a line for the peer; flushConnections() writes it as the peer reads
	void queueLine(CConnection &c, const std::string &line)
	{
		if (!c.Pipe->isOpen())
			return;
		std::string data = line;
		data += '\n';
		c.Pipe->queueSwap(data);
	}

	/// Writes what each peer takes now; true when any took something
	bool flushConnections()
	{
		bool busy = false;
		for (std::map<uint32, CConnection *>::iterator it = m_Connections.begin(); it != m_Connections.end(); ++it)
		{
			CConnection &c = *it->second;
			CLocalPipe &pipe = *c.Pipe;
			size_t before = pipe.getQueuedBytes();
			if (before == 0 || !pipe.flush())
				continue;
			size_t after = pipe.getQueuedBytes();
			if (after < before)
				busy = true;
			if (after > s_MaxQueuedBytes)
			{
				nlwarning("CTRL: %s has %u MB of answers it did not read, closing it", c.Name.c_str(), (uint)(after / (1024 * 1024)));
				pipe.close();
			}
			else if (after > 0 && pipe.getStalledMs() > s_StallTimeoutMs)
			{
				nlwarning("CTRL: %s took none of its answers for %u s, closing it", c.Name.c_str(), (uint)(pipe.getStalledMs() / 1000));
				pipe.close();
			}
		}
		return busy;
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
		queueLine(c, joinCtrlLine(fields));
	}

	void sendError(CConnection &c, const std::string &id, const std::string &message, const CJsonValue &details = CJsonValue())
	{
		std::vector<std::string> fields;
		fields.push_back("error");
		fields.push_back(id);
		fields.push_back(message);
		fields.push_back(wrap(details).toString());
		queueLine(c, joinCtrlLine(fields));
	}

	bool hasWaiter(uint32 connection, const std::string &id, uint &count)
	{
		bool found = false;
		count = 0;
		for (uint i = 0; i < m_Waiters.size(); ++i)
		{
			if (m_Waiters[i].Connection != connection)
				continue;
			++count;
			found = found || m_Waiters[i].Id == id;
		}
		return found;
	}

	void handleMessage(CConnection &c, CCtrlMessage &message)
	{
		const std::vector<std::string> &f = message.Fields;
		if (message.IsData || f.empty())
			return;
		if (f[0] == "hello")
		{
			// hello <version> <name>: the tool says who it is, so a lease holder has a name
			if (f.size() >= 3 && !f[2].empty())
			{
				if (f[2].size() > s_MaxNameLength)
					nlwarning("CTRL: %s gives a name of %u bytes, more than %u: it keeps its number", c.Name.c_str(), (uint)f[2].size(), (uint)s_MaxNameLength);
				else
					c.Name = f[2] + toString(" (connection %u)", c.Id);
			}
			return;
		}
		if (f[0] == "cancel")
		{
			if (f.size() < 2)
				return;
			for (uint i = 0; i < m_Waiters.size(); ++i)
			{
				if (m_Waiters[i].Connection == c.Id && m_Waiters[i].Id == f[1])
				{
					sendError(c, f[1], "cancelled");
					m_Waiters.erase(m_Waiters.begin() + i);
					return;
				}
			}
			// a call of the main thread gets its answer from there; a cancel of a call that is not pending reaches nothing
			std::map<std::string, CPendingCall>::iterator pending = c.Pending.find(f[1]);
			if (pending == c.Pending.end() || pending->second.CancelSent)
				return;
			pending->second.CancelSent = true;
			CAutoMutex<CMutex> lock(m_InboxMutex);
			m_Cancels.push_back(TCallRef(c.Id, f[1]));
			return;
		}
		if (f[0] != "call")
		{
			if (c.UnknownMessages == 0)
			{
				nldebug("CTRL: %s sent an unknown message type '%s'", c.Name.c_str(), f[0].substr(0, s_MaxNameLength).c_str());
			}
			if (++c.UnknownMessages >= s_MaxUnknownMessages)
			{
				nlwarning("CTRL: %s sent %u messages of no known type, closing it", c.Name.c_str(), c.UnknownMessages);
				c.Pipe->close();
			}
			return;
		}
		if (f.size() < 3 || f[1].size() > s_MaxIdLength)
		{
			sendError(c, f.size() > 1 && f[1].size() <= s_MaxIdLength ? f[1] : std::string(),
			    toString("a call needs an id of %u bytes at most and a method", (uint)s_MaxIdLength));
			return;
		}
		const std::string &id = f[1];
		const std::string &method = f[2];
		if (method.size() > s_MaxNameLength)
		{
			sendError(c, id, toString("a method name is %u bytes at most; ctrl.methods lists them", (uint)s_MaxNameLength));
			return;
		}
		size_t argBytes = f.size() >= 4 ? f[3].size() : 0;
		CJsonValue args = CJsonValue::object();
		if (argBytes > 0)
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
		bool privileged = false;
		bool found = false;
		{
			CAutoMutex<CMutex> lock(m_MethodsMutex);
			for (uint i = 0; i < m_Methods.size(); ++i)
			{
				if (m_Methods[i].Name == method)
				{
					found = true;
					readOnly = m_Methods[i].ReadOnly;
					privileged = m_Methods[i].Privileged;
					break;
				}
			}
		}
		if (!found)
		{
			sendError(c, id, "no method '" + method + "'; ctrl.methods lists them");
			return;
		}
		// what the process shows was written by others, and what calls one of these methods may have read it: the call
		// never reaches the main thread, nor takes the lease
		if (privileged && !m_Trusted)
		{
			sendError(c, id, method + " runs only in a client started with --ctrl-trusted: the content of this one is not trusted");
			return;
		}
		uint waiters;
		if (c.Pending.find(id) != c.Pending.end() || hasWaiter(c.Id, id, waiters))
		{
			sendError(c, id, "a call of this connection with the id '" + id + "' is not answered yet: an id is for one call at a time");
			return;
		}
		if (c.Pending.size() >= s_MaxPendingCalls || c.PendingBytes + argBytes > s_MaxPendingBytes)
		{
			sendError(c, id,
			    toString("%u calls of this connection (%u MB of arguments) are not answered yet, the most an endpoint takes: wait for their answers", (uint)c.Pending.size(),
			        (uint)(c.PendingBytes / (1024 * 1024))));
			return;
		}
		if (!readOnly && !takeLease(c, id))
			return;

		// once the endpoint stops, nothing more reaches the main thread: the call is told why at once
		std::string exitMessage;
		uint64 eventNext = ringNext(false);
		{
			CAutoMutex<CMutex> lock(m_InboxMutex);
			if (m_ExitMessage.empty())
			{
				m_Inbox.push_back(CInCall());
				CInCall &in = m_Inbox.back();
				in.Connection = c.Id;
				in.Id = id;
				in.Method = method;
				in.Args.swap(args);
			}
			else
			{
				exitMessage = m_ExitMessage;
			}
		}
		if (!exitMessage.empty())
		{
			sendError(c, id, exitMessage);
			return;
		}
		CPendingCall pending;
		pending.Control = !readOnly;
		pending.Bytes = argBytes;
		pending.CancelSent = false;
		c.Pending[id] = pending;
		if (!readOnly)
			++c.ControlPending;
		c.PendingBytes += argBytes;
		c.LastCallEvent = eventNext;
	}

	/// The answer of a pending call is on its way: the call counts no more
	void answered(CConnection &c, const std::string &id)
	{
		std::map<std::string, CPendingCall>::iterator it = c.Pending.find(id);
		if (it == c.Pending.end())
			return;
		if (it->second.Control)
			--c.ControlPending;
		c.PendingBytes -= it->second.Bytes;
		c.Pending.erase(it);
	}

	std::string connectionName(uint32 id)
	{
		std::map<uint32, CConnection *>::iterator it = m_Connections.find(id);
		return it != m_Connections.end() ? it->second->Name : toString("connection %u", id);
	}

	bool takeLease(CConnection &c, const std::string &id)
	{
		if (m_LeaseHolder == 0 || m_LeaseHolder == c.Id)
		{
			if (m_LeaseHolder == 0)
				nlinfo("CTRL: %s took the lease", c.Name.c_str());
			m_LeaseHolder = c.Id;
			m_LeaseHeld = true;
			return true;
		}
		std::string holder = connectionName(m_LeaseHolder);
		CJsonValue details = CJsonValue::object();
		details.set("holder", holder);
		sendError(c, id, "the lease is held by " + holder + "; only read-only methods are open to other connections", details);
		return false;
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
			sendResult(c, id, stateJson(readState()));
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
				if (m_LeaseHolder == c.Id)
				{
					// those calls would go on driving the process while another connection drives it too
					if (c.ControlPending > 0)
					{
						CJsonValue details = CJsonValue::object();
						details.set("unanswered", c.ControlPending);
						sendError(c, id,
						    toString("%u calls of this connection that need the lease are not answered yet: wait for their answers or cancel them, then release it",
						        c.ControlPending),
						    details);
						return true;
					}
					m_LeaseHolder = 0;
					m_LeaseHeld = false;
					nlinfo("CTRL: %s gave the lease back", c.Name.c_str());
				}
			}
			else if (action != "status")
			{
				sendError(c, id, "action is take, release or status");
				return true;
			}
			CJsonValue r = CJsonValue::object();
			r.set("holder", m_LeaseHolder == 0 ? CJsonValue() : CJsonValue(connectionName(m_LeaseHolder)));
			r.set("yours", m_LeaseHolder == c.Id);
			sendResult(c, id, r);
			return true;
		}
		if (method == "ctrl.wait_state" || method == "ctrl.log" || method == "ctrl.events")
		{
			uint waiters;
			if (hasWaiter(c.Id, id, waiters) || c.Pending.find(id) != c.Pending.end())
			{
				sendError(c, id, "a call of this connection with the id '" + id + "' is not answered yet: an id is for one call at a time");
				return true;
			}
			if (waiters >= s_MaxWaiters)
			{
				sendError(c, id, toString("%u waits of this connection are open, the most an endpoint takes", waiters));
				return true;
			}
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
				CStateCopy state = readState();
				w.StartState = state.State;
				w.LastState = state.State;
				// from since_idx, else from the last call of the connection, else from now
				w.EventStart = w.Args.has("since_idx") ? (uint64)std::max((sint64)1, w.Args.get("since_idx").getInt(1))
				                                       : (c.LastCallEvent != 0 ? c.LastCallEvent : ringNext(false));
				w.EventScanned = w.EventStart;
			}
			if (!answerWaiter(c, w, waitMs == 0))
				m_Waiters.push_back(w);
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
		CAutoMutex<CMutex> lock(m_MethodsMutex);
		for (uint i = 0; i < m_Methods.size(); ++i)
		{
			const CMethodInfo &m = m_Methods[i];
			if (!prefix.empty() && m.Name.compare(0, prefix.size(), prefix) != 0)
				continue;
			if (!contains.empty() && toLowerAscii(m.Name + " " + m.Description).find(contains) == std::string::npos)
				continue;
			CJsonValue e = CJsonValue::object();
			e.set("name", m.Name);
			e.set("description", m.Description);
			e.set("read_only", m.ReadOnly);
			e.set("privileged", m.Privileged);
			e.set("available", !m.Privileged || m_Trusted);
			if (schemas)
				e.set("schema", m.Schema);
			list.push(e);
		}
		CJsonValue r = CJsonValue::object();
		r.set("trusted", m_Trusted);
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
			// the state is copied, not turned into JSON, at every look; the JSON is made for the answer only
			CStateCopy state = readState();
			bool match = matchesState(w.Args, state.State);
			std::string stoppedBy;
			if (!match && !expired)
			{
				// what makes the state unreachable ends the wait: an event of the kinds watched (a message box that
				// refuses, the exit), or a settled state the process went to instead and stays in
				stoppedBy = stoppingEvent(w);
				if (state.State != w.StartState)
					w.Changed = true;
				if (state.State != w.LastState || !state.Settled)
					w.SettledSince = now;
				w.LastState = state.State;
				sint64 settleMs = (sint64)(w.Args.get("settle_seconds").getNumber(5.0) * 1000.0);
				if (stoppedBy.empty() && w.Changed && state.Settled && settleMs > 0 && (sint64)(now - w.SettledSince) >= settleMs)
					stoppedBy = "the state went to " + state.State + " instead and stayed there";
				if (stoppedBy.empty())
					return false;
			}
			CJsonValue snapshot = stateJson(state);
			snapshot.set("matched", match);
			if (!stoppedBy.empty())
				snapshot.set("stopped", stoppedBy);
			if (!w.Missed.empty())
				snapshot.set("missed", w.Missed);
			snapshot.set("waited_ms", (sint64)(now - w.Start));
			sendResult(c, w.Id, snapshot);
			return true;
		}
		// nothing new since the last look: no scan, the ring is not locked for long while the process logs
		bool log = w.Method == "ctrl.log";
		uint64 next = ringNext(log);
		if (w.Seen == next && !expired)
			return false;
		CJsonValue r = log ? queryLog(w.Args) : queryEvents(w.Args);
		bool none = r.get(log ? "lines" : "events").size() == 0;
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

	/// The first event of the kinds that end the wait (message_box and exit by default) since the one it watches
	/// from; each event is looked at once. Events the ring dropped before they were looked at are said in Missed.
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
		CAutoMutex<CMutex> lock(m_EventMutex);
		uint64 first = m_Events.empty() ? m_EventNext : m_Events.front().Idx;
		if (w.EventScanned < first)
		{
			if (w.Missed.empty())
				w.Missed = "the events from " + indexText(w.EventScanned) + " on were no longer held: the watch starts at " + indexText(first);
			w.EventScanned = first;
		}
		for (size_t pos = (size_t)std::min((uint64)m_Events.size(), w.EventScanned - first); pos < m_Events.size(); ++pos)
		{
			const CEventLine &e = m_Events[pos];
			if (kinds.find(e.Kind) != kinds.end())
				return "event " + indexText(e.Idx) + ": " + e.Kind + " " + e.Data;
		}
		w.EventScanned = m_EventNext;
		return std::string();
	}

	uint64 ringNext(bool log)
	{
		if (log)
		{
			CAutoMutex<CMutex> lock(m_LogMutex);
			return m_LogNext;
		}
		CAutoMutex<CMutex> lock(m_EventMutex);
		return m_EventNext;
	}

	void checkWaiters()
	{
		for (uint i = 0; i < m_Waiters.size();)
		{
			std::map<uint32, CConnection *>::iterator it = m_Connections.find(m_Waiters[i].Connection);
			if (it == m_Connections.end() || !it->second->Pipe->isOpen() || answerWaiter(*it->second, m_Waiters[i], false))
				m_Waiters.erase(m_Waiters.begin() + i);
			else
				++i;
		}
	}

	CJsonValue queryLog(const CJsonValue &args)
	{
		CLogPredicate pred;
		const CJsonValue &levels = args.get("levels");
		for (uint i = 0; i < levels.size(); ++i)
			pred.Levels.insert(toUpperAscii(levels.at(i).getString()));
		pred.Contains = toLowerAscii(args.get("contains").getString());
		CSelection<CLogLine> selection;
		{
			// every thread that logs takes this lock: it is held for the scan and the copies only
			CAutoMutex<CMutex> lock(m_LogMutex);
			selectLines(m_Log, m_LogNext, args, pred, selection);
		}
		return selectionJson(selection, "lines");
	}

	CJsonValue queryEvents(const CJsonValue &args)
	{
		CEventPredicate pred;
		const CJsonValue &kinds = args.get("kinds");
		for (uint i = 0; i < kinds.size(); ++i)
			pred.Kinds.insert(kinds.at(i).getString());
		pred.Contains = toLowerAscii(args.get("contains").getString());
		CSelection<CEventLine> selection;
		{
			CAutoMutex<CMutex> lock(m_EventMutex);
			selectLines(m_Events, m_EventNext, args, pred, selection);
		}
		return selectionJson(selection, "events");
	}

	/// The answers of the main thread go to the queues of their connections, the data frames before the result
	void takeOutbox()
	{
		std::deque<COutMessage> out;
		{
			CAutoMutex<CMutex> lock(m_OutboxMutex);
			out.swap(m_Outbox);
		}
		for (size_t i = 0; i < out.size(); ++i)
		{
			COutMessage &m = out[i];
			std::map<uint32, CConnection *>::iterator it = m_Connections.find(m.Connection);
			// the connection went away meanwhile; nobody waits for the answer
			if (it == m_Connections.end())
				continue;
			CConnection &c = *it->second;
			answered(c, m.Id);
			if (!c.Pipe->isOpen())
				continue;
			for (uint k = 0; k < m.Attachments.size(); ++k)
			{
				std::vector<std::string> fields;
				fields.push_back("data");
				fields.push_back(m.Id + toString(".%u", k + 1));
				fields.push_back(m.Attachments[k].first);
				fields.push_back(indexText((uint64)m.Attachments[k].second.size()));
				queueLine(c, joinCtrlLine(fields));
				c.Pipe->queueSwap(m.Attachments[k].second);
			}
			std::vector<std::string> fields;
			fields.push_back(m.Error ? "error" : "result");
			fields.push_back(m.Id);
			if (m.Error)
				fields.push_back(m.Message);
			fields.push_back(m.Json);
			queueLine(c, joinCtrlLine(fields));
		}
	}

	void dropClosed()
	{
		for (std::map<uint32, CConnection *>::iterator it = m_Connections.begin(); it != m_Connections.end();)
		{
			CConnection *c = it->second;
			if (c->Pipe->isOpen())
			{
				++it;
				continue;
			}
			nlinfo("CTRL: %s closed", c->Name.c_str());
			if (m_LeaseHolder == c->Id)
			{
				m_LeaseHolder = 0;
				m_LeaseHeld = false;
			}
			{
				CAutoMutex<CMutex> lock(m_InboxMutex);
				m_Closed.push_back(c->Id);
			}
			delete c->Pipe;
			delete c;
			m_Connections.erase(it++);
		}
	}
};

namespace {

/**
 * The displayer on the logs of the process. A log changes its list of displayers under its mutex, which a message
 * holds only while it is positioned (the log macros): a log written without a position goes through the list
 * without it, so taking a displayer off while other threads log is still not safe. This one is put on once and
 * stays, and an endpoint that stops only takes itself off it, under its lock, after which no thread is still in
 * that endpoint's ring. It is never deleted, since a log may display through it until the process ends. It serves
 * one endpoint at a time.
 */
class CCtrlLogForwarder : public IDisplayer
{
public:
	CCtrlLogForwarder()
	    : IDisplayer("CTRL_ENDPOINT")
	    , m_Target(nullptr)
	{
	}

	/// The logs go to the ring of this endpoint from now on; false when another endpoint has them
	bool claim(CCtrlEndpoint::CImpl *target)
	{
		CAutoMutex<CMutex> lock(m_Mutex);
		if (m_Target != nullptr && m_Target != target)
			return false;
		m_Target = target;
		return true;
	}

	bool isClaimedByOther(CCtrlEndpoint::CImpl *target)
	{
		CAutoMutex<CMutex> lock(m_Mutex);
		return m_Target != nullptr && m_Target != target;
	}

	void release(CCtrlEndpoint::CImpl *target)
	{
		CAutoMutex<CMutex> lock(m_Mutex);
		if (m_Target == target)
			m_Target = nullptr;
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

enum TMethodStep
{
	StepCall,
	StepPoll,
	StepCancel
};

/// Runs one step of a method. Nothing is caught: what a method throws is a crash of the process with its report, as
/// anywhere else on the main thread, and not an answer that leaves the process running after an unwind through code
/// that was not written for one.
void runMethodStep(ICtrlMethod *method, CCtrlCall &call, TMethodStep step)
{
	switch (step)
	{
	case StepCall: method->call(call); break;
	case StepPoll: method->poll(call); break;
	case StepCancel: method->cancel(call); break;
	}
}

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
	CCommandCollector(const std::string &prefix, const std::string &contains, uint limit)
	    : m_Prefix(prefix)
	    , m_Contains(contains)
	    , m_Limit(limit)
	    , m_Total(0)
	    , m_List(CJsonValue::array())
	{
	}

	const CJsonValue &getList() const { return m_List; }
	uint getTotal() const { return m_Total; }

	virtual void onCommandAdded(const std::string &name, TCommandEntryKind kind, const std::string &args, const std::string &help) NL_OVERRIDE
	{
		if (!m_Prefix.empty() && name.compare(0, m_Prefix.size(), m_Prefix) != 0)
			return;
		if (!m_Contains.empty() && toLowerAscii(name + " " + help).find(m_Contains) == std::string::npos)
			return;
		++m_Total;
		if (m_List.size() >= m_Limit)
			return;
		CJsonValue e = CJsonValue::object();
		e.set("name", name);
		e.set("kind", kind == HandlerEntryKind ? "handler" : (kind == HandlerCommandEntryKind ? "member" : "command"));
		if (!args.empty())
			e.set("args", args);
		if (!help.empty())
			e.set("help", help);
		m_List.push(e);
	}

	virtual void onCommandRemoved(const std::string & /* name */) NL_OVERRIDE { }

private:
	std::string m_Prefix;
	std::string m_Contains;
	uint m_Limit;
	uint m_Total;
	CJsonValue m_List;
};

/// What a command writes to its log, line by line, up to a count and a size; the lines beyond are counted only
class CCommandOutput : public IDisplayer
{
public:
	CCommandOutput()
	    : IDisplayer("CTRL_COMMAND_OUTPUT")
	    , m_Bytes(0)
	    , m_Total(0)
	{
	}

	const std::vector<std::string> &getLines() const { return m_Lines; }
	uint getTotal() const { return m_Total; }

protected:
	virtual void doDisplay(const CLog::TDisplayInfo & /* args */, const char *message) NL_OVERRIDE
	{
		if (message == nullptr)
			return;
		++m_Total;
		size_t length = strlen(message);
		while (length > 0 && (message[length - 1] == '\n' || message[length - 1] == '\r'))
			--length;
		if (m_Lines.size() >= s_MaxCommandLines || length > s_MaxCommandBytes - m_Bytes)
			return;
		m_Bytes += length;
		m_Lines.push_back(std::string(message, length));
	}

private:
	std::vector<std::string> m_Lines;
	size_t m_Bytes;
	uint m_Total;
};

/// The record of a process that is gone
struct CGoneRecord
{
	std::string Path;
	std::string Name;
	uint64 Updated;
};

/// The process of a running record of a name, 0 when none is known
uint32 findRunningHolder(const std::string &dir, const std::string &name, const std::string &ownRecord)
{
	std::vector<std::string> records;
	CCtrlRegistry::listRecords(dir, records);
	for (uint i = 0; i < records.size(); ++i)
	{
		CJsonValue record;
		std::string error;
		if (records[i] == ownRecord || !CCtrlRegistry::readRecord(records[i], record, error) || record.get("name").getString() != name)
			continue;
		uint32 pid = (uint32)record.get("pid").getInt(0);
		if (CCtrlRegistry::probeProcess(pid, (uint64)record.get("start_time").getInt(0)) == CCtrlRegistry::ProcessRunning)
			return pid;
	}
	return 0;
}

} // anonymous namespace

// *** CCtrlEndpoint ***

CCtrlEndpoint::CCtrlEndpoint()
    : m_Impl(new CImpl())
    , m_Started(false)
    , m_Frame(0)
    , m_InUpdate(false)
    , m_Capture(nullptr)
    , m_CallingConnection(0)
    , m_ConnectionClosed(nullptr)
{
	m_Impl->m_Pid = CCtrlRegistry::getCurrentProcessId();
	m_Impl->m_StartTime = CCtrlRegistry::getProcessStartTime(m_Impl->m_Pid);

	CMethodInfo info;
	info.ReadOnly = true;
	info.Privileged = false;
	info.BuiltIn = true;
	info.Name = "ctrl.identity";
	info.Description = "Who this process is: name, process id and start time, program, build, log file, and what it adds (account, character).";
	info.Schema = CCtrlSchema();
	m_Impl->m_Methods.push_back(info);
	info.Name = "ctrl.state";
	info.Description = "The state of the process, with its detail, the frame, and whether the main thread is running or busy for how long in which state. Answered without the main thread.";
	m_Impl->m_Methods.push_back(info);
	info.Name = "ctrl.wait_state";
	info.Description = "Waits until the state is one of those given (state or states), or differs from 'not', up to timeout_ms (30 s by default, 600 s at most). It ends early with matched false and "
	                   "what stopped it when an event of the stop_on kinds comes (message_box and exit by default), or when the state left the one it had and stays in another settled state for "
	                   "settle_seconds (5 by default; 0 waits on): what was to lead to the state did not. missed says when events it was to watch were no longer held.";
	info.Schema = CCtrlSchema()
	                  .str("state", "The state to wait for.")
	                  .strings("states", "Any of these states.")
	                  .str("not", "Wait until the state is no longer this one.")
	                  .integer("timeout_ms", "How long to wait at most.")
	                  .strings("stop_on", "Event kinds that end the wait (message_box and exit by default; an empty list for none).")
	                  .integer("since_idx", "The first event index that counts (by default the events since the last call of this connection).")
	                  .number("settle_seconds", "How long another settled state ends the wait (5 by default, 0 never).");
	m_Impl->m_Methods.push_back(info);
	info.Name = "ctrl.log";
	info.Description = "Log lines of the process, indexed from 1 since the endpoint started, each with its time (ms since the epoch), level and text. Select by index (from_idx, to_idx), time "
	                   "(from_time, to_time) or the last N; filter by levels (INF, WRN, ERR, DBG, AST, STT) and a case-insensitive substring. From an index the oldest lines come first, up to max "
	                   "(200 by default); with wait_ms, waits for a first matching line. next_idx continues; missed says when the ring no longer holds from_idx.";
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
	m_Impl->m_Methods.push_back(info);
	info.Name = "ctrl.events";
	info.Description = "Events of the process, indexed like the log: state changes and what the process reports (kind and data). Same selection as ctrl.log, with kinds instead of levels; with "
	                   "wait_ms it is a long poll from from_idx.";
	info.Schema = CCtrlSchema()
	                  .integer("from_idx", "First index, inclusive.")
	                  .integer("to_idx", "Last index, inclusive.")
	                  .integer("last", "The last N events of the selection.")
	                  .strings("kinds", "Only these kinds.")
	                  .str("contains", "Only events whose data contains this, case-insensitive.")
	                  .integer("max", "At most this many events (200 by default, 5000 at most).")
	                  .integer("wait_ms", "Wait up to this long for a first event when none matches yet.");
	m_Impl->m_Methods.push_back(info);
	info.Name = "ctrl.methods";
	info.Description = "The methods of this endpoint, filtered by name prefix or a word of the name or description; schemas only when asked. Each says whether it is read-only, whether it "
	                   "is privileged (Lua, action handlers, commands, database writes), and whether it is available: a privileged method runs only when the endpoint is trusted "
	                   "(trusted in the answer; a client started with --ctrl-trusted).";
	info.Schema = CCtrlSchema().str("prefix", "Only names starting with this.").str("contains", "Only methods whose name or description contains this.").boolean("schemas", "Include the argument schemas.");
	m_Impl->m_Methods.push_back(info);
	info.Name = "ctrl.lease";
	info.Description = "The lease that methods which change something need: take, release, or status. A connection that calls such a method takes it when nobody holds it; it is released when "
	                   "that connection closes, or gives it back, which it cannot while a call of its own that needs the lease is not answered.";
	info.Schema = CCtrlSchema().choice("action", "take|release|status", "What to do (status by default).");
	m_Impl->m_Methods.push_back(info);

	// the built-in methods of the main thread, there once for every start of the endpoint
	addMethod(new CEndpointMethod(this, &CCtrlEndpoint::runCommandsMethod, "ctrl.commands",
	    "The command table of the process: commands and variables with their argument syntax and help, named handler instances and their member commands (instance.command). Filter by "
	    "prefix or a word of the name or help.",
	    CCtrlSchema()
	        .str("prefix", "Only names starting with this (case sensitive, as the commands are).")
	        .str("contains", "Only entries whose name or help contains this (case-insensitive).")
	        .integer("limit", "At most this many entries (300 by default)."),
	    true));
	ICtrlMethod *runCommand = new CEndpointMethod(this, &CCtrlEndpoint::runRunCommandMethod, "ctrl.run_command",
	    "Runs a command, or reads or sets a variable, of the command registry, with the arguments as a list: they reach the command as given, nothing is parsed. Returns whether it "
	    "succeeded, what it wrote to its log (the first 5000 lines, 4 MB at most; truncated and output_lines say when there was more), and what it showed elsewhere meanwhile (the chat "
	    "of a client). Privileged: it runs only when the endpoint is trusted.",
	    CCtrlSchema().str("command", "The command or variable, instance.command for a member command.", true).strings("args", "The arguments, one per entry."), false);
	// a command can do whatever the process can
	runCommand->setPrivileged(true);
	addMethod(runCommand);
}

CCtrlEndpoint::~CCtrlEndpoint()
{
	nlassertex(!m_InUpdate, ("CTRL: the endpoint '%s' is deleted from inside its update()", m_Name.c_str()));
	if (m_Started)
		stop(m_ExitReason.empty() ? std::string("the endpoint was destroyed") : m_ExitReason);
	for (std::map<std::string, ICtrlMethod *>::iterator it = m_Methods.begin(); it != m_Methods.end(); ++it)
		delete it->second;
	delete m_Impl;
}

void CCtrlEndpoint::runCommandsMethod(CCtrlCall &call)
{
	CCommandCollector collector(call.getArgs().get("prefix").getString(), toLowerAscii(call.getArgs().get("contains").getString()),
	    (uint)std::max((sint64)1, call.getArgs().get("limit").getInt(300)));
	CCommandRegistry::getInstance().listCommands(collector);
	CJsonValue r = CJsonValue::object();
	r.set("commands", collector.getList());
	r.set("total", collector.getTotal());
	r.set("truncated", collector.getTotal() > collector.getList().size());
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
	CCommandOutput output;
	log.addDisplayer(&output);
	// and what it shows outside its log meanwhile; a command run from inside a command keeps the outer list
	std::vector<std::string> shown;
	std::vector<std::string> *previous = m_Capture;
	m_Capture = &shown;
	bool ok = ICommand::executeArgs(command, args, log, true, true);
	m_Capture = previous;
	log.removeDisplayer(&output);

	CJsonValue lines = CJsonValue::array();
	for (uint i = 0; i < output.getLines().size(); ++i)
		lines.push(CJsonValue(output.getLines()[i]));
	CJsonValue r = CJsonValue::object();
	r.set("command", command);
	r.set("ok", ok);
	r.set("output", lines);
	r.set("output_lines", output.getTotal());
	r.set("truncated", output.getTotal() > output.getLines().size());
	CJsonValue shownList = CJsonValue::array();
	for (uint i = 0; i < shown.size(); ++i)
		shownList.push(CJsonValue(shown[i]));
	r.set("shown", shownList);
	call.reply(r);
}

void CCtrlEndpoint::setTrusted(bool trusted)
{
	// the channel thread reads it without a lock, and the record says it from the start
	nlassertex(!m_Started, ("CTRL: the trust of the endpoint '%s' is set after its start", m_Name.c_str()));
	m_Impl->m_Trusted = trusted;
}

bool CCtrlEndpoint::isTrusted() const
{
	return m_Impl->m_Trusted;
}

bool CCtrlEndpoint::start(const std::string &name, const CJsonValue &identity, std::string &error)
{
	nlassert(!m_Started);
	if (!CCtrlRegistry::isValidName(name))
	{
		error = "'" + name + "' is not a name for an endpoint: letters, digits, '_', '-' and '.', 64 at most";
		return false;
	}
	// the log of the process goes to one ring
	if (s_LogForwarder != nullptr && s_LogForwarder->isClaimedByOther(m_Impl))
	{
		error = "another endpoint of this process is started: a process has one";
		return false;
	}
	m_Name = name;
	std::string dir;
	if (!CCtrlRegistry::getDirectory(dir, true, error))
		return false;

	m_Impl->m_RecordPath = CCtrlRegistry::recordPath(dir, name, m_Impl->m_Pid);
	m_Impl->m_EndpointPath = CCtrlRegistry::endpointPath(dir, name, m_Impl->m_Pid);

	CJsonValue id = identity.isObject() ? identity : CJsonValue::object();
	id.set("name", name);
	id.set("pid", m_Impl->m_Pid);
	id.set("start_time", m_Impl->m_StartTime);
	id.set("endpoint", m_Impl->m_EndpointPath);
	id.set("protocol", CtrlProtocolVersion);
	{
		CAutoMutex<CMutex> lock(m_Impl->m_IdentityMutex);
		m_Impl->m_Identity = id;
	}
	{
		CAutoMutex<CMutex> lock(m_Impl->m_InboxMutex);
		m_Impl->m_ExitMessage.clear();
	}

	// tools address endpoints by name: the name is taken before the record says it is ours, and kept until stop()
	if (!m_Impl->m_NameLock.lock(dir, name, error))
	{
		uint32 holder = findRunningHolder(dir, name, m_Impl->m_RecordPath);
		if (holder != 0)
			error += toString("; process %u has it", holder);
		{
			CAutoMutex<CMutex> lock(m_Impl->m_IdentityMutex);
			m_Impl->m_Identity.set("error", error);
		}
		writeRecord("failed");
		return false;
	}
	pruneRecords(dir);

	if (!m_Impl->m_Listener.listen(m_Impl->m_EndpointPath, error))
	{
		{
			CAutoMutex<CMutex> lock(m_Impl->m_IdentityMutex);
			m_Impl->m_Identity.set("error", error);
		}
		writeRecord("failed");
		m_Impl->m_NameLock.unlock();
		return false;
	}

	// every log of the process, from here on
	if (s_LogForwarder == nullptr)
		s_LogForwarder = new CCtrlLogForwarder();
	INelContext &context = INelContext::getInstance();
	CLog *logs[] = { context.getErrorLog(), context.getWarningLog(), context.getInfoLog(), context.getDebugLog(), context.getAssertLog() };
	for (uint i = 0; i < sizeof(logs) / sizeof(logs[0]); ++i)
		if (logs[i] != nullptr && !logs[i]->attached(s_LogForwarder))
			logs[i]->addDisplayer(s_LogForwarder);
	s_LogForwarder->claim(m_Impl);

	m_Impl->m_Running = true;
	m_Impl->m_Thread = IThread::create(m_Impl);
	m_Impl->m_Thread->start();
	m_Started = true;
	writeRecord("running");
	nlinfo("CTRL: endpoint '%s' listens on '%s'; whoever opens it acts as this user", name.c_str(), m_Impl->m_EndpointPath.c_str());
	return true;
}

void CCtrlEndpoint::pruneRecords(const std::string &dir)
{
	std::vector<CGoneRecord> gone;
	std::vector<std::string> records;
	CCtrlRegistry::listRecords(dir, records);
	for (uint i = 0; i < records.size(); ++i)
	{
		if (records[i] == m_Impl->m_RecordPath)
			continue;
		CJsonValue record;
		std::string error;
		if (!CCtrlRegistry::readRecord(records[i], record, error))
		{
			nlwarning("CTRL: %s", error.c_str());
			continue;
		}
		// a process the system says nothing about may run: its record stays
		uint32 pid = (uint32)record.get("pid").getInt(0);
		if (CCtrlRegistry::probeProcess(pid, (uint64)record.get("start_time").getInt(0)) != CCtrlRegistry::ProcessGone)
			continue;
		CGoneRecord g;
		g.Path = records[i];
		g.Name = record.get("name").getString();
		g.Updated = (uint64)record.get("updated").getInt(0);
		gone.push_back(g);
#ifndef NL_OS_WINDOWS
		// the socket a process that crashed left: the one its name and id give, nothing a record names
		if (CCtrlRegistry::isValidName(g.Name))
		{
			std::string socketPath = CCtrlRegistry::endpointPath(dir, g.Name, pid);
			struct stat st;
			if (lstat(socketPath.c_str(), &st) == 0 && S_ISSOCK(st.st_mode) && ::unlink(socketPath.c_str()) != 0)
				nlwarning("CTRL: cannot remove '%s', the socket of a process that ended", socketPath.c_str());
		}
#endif
	}
	// the record written last of each name stays, so why that process ended can still be read
	std::map<std::string, uint> last;
	for (uint i = 0; i < gone.size(); ++i)
	{
		std::map<std::string, uint>::iterator it = last.find(gone[i].Name);
		if (it == last.end() || gone[i].Updated > gone[it->second].Updated)
			last[gone[i].Name] = i;
	}
	uint removed = 0;
	for (uint i = 0; i < gone.size(); ++i)
	{
		if (last[gone[i].Name] == i)
			continue;
		if (CFile::deleteFile(gone[i].Path))
			++removed;
		else
			nlwarning("CTRL: cannot remove '%s', the record of a process that ended", gone[i].Path.c_str());
	}
	if (removed > 0)
		nlinfo("CTRL: removed %u records of processes that ended from '%s'", removed, dir.c_str());
}

void CCtrlEndpoint::stop(const std::string &exitReason)
{
	if (!m_Started)
		return;
	m_Started = false;
	nlinfo("CTRL: endpoint '%s' stops: %s", m_Name.c_str(), exitReason.c_str());
	{
		CAutoMutex<CMutex> lock(m_Impl->m_IdentityMutex);
		m_Impl->m_Identity.set("exit_reason", exitReason);
	}
	writeRecord("exited");

	// every call gets an answer before the connections close: those queued and deferred here, the open ones in
	// finishStop(), and the channel thread those that come from now on and its waits
	std::string message = "the process exits: " + exitReason;
	std::deque<CInCall> queued;
	{
		CAutoMutex<CMutex> lock(m_Impl->m_InboxMutex);
		m_Impl->m_ExitMessage = message;
		queued.swap(m_Impl->m_Inbox);
		m_Impl->m_Cancels.clear();
	}
	for (size_t i = 0; i < queued.size(); ++i)
		m_Impl->postError(queued[i].Connection, queued[i].Id, message);
	for (size_t i = 0; i < m_Impl->m_Deferred.size(); ++i)
		m_Impl->postError(m_Impl->m_Deferred[i].Connection, m_Impl->m_Deferred[i].Id, message);
	m_Impl->m_Deferred.clear();

	// a method that stops the endpoint runs inside update(), which finishes the stop as it returns
	if (m_InUpdate)
	{
		m_Impl->m_StopPending = true;
		return;
	}
	finishStop();
}

void CCtrlEndpoint::finishStop()
{
	m_Impl->m_StopPending = false;
	// the open calls are cancelled; what a method does not answer as it cancels is told why
	std::vector<CCtrlCall *> open;
	open.swap(m_Open);
	for (uint i = 0; i < open.size(); ++i)
	{
		CCtrlCall *call = open[i];
		if (!call->isDone())
		{
			std::map<std::string, ICtrlMethod *>::iterator it = m_Methods.find(call->getMethod());
			m_CallingConnection = call->getConnection();
			if (it != m_Methods.end())
				runMethodStep(it->second, *call, StepCancel);
			m_CallingConnection = 0;
			call->fail(m_Impl->m_ExitMessage);
		}
		postAnswer(*call);
		delete call;
	}

	// the channel thread sends the last answers, then closes the connections
	m_Impl->m_Running = false;
	if (m_Impl->m_Thread != nullptr)
	{
		m_Impl->m_Thread->wait();
		delete m_Impl->m_Thread;
		m_Impl->m_Thread = nullptr;
	}
	m_Impl->m_Listener.close();
	m_Impl->m_NameLock.unlock();

	// the forwarder stays on the logs; once it no longer points here, no thread is in the ring
	s_LogForwarder->release(m_Impl);

	std::vector<uint32> closedList;
	{
		CAutoMutex<CMutex> lock(m_Impl->m_InboxMutex);
		m_Impl->m_Inbox.clear();
		m_Impl->m_Cancels.clear();
		closedList.swap(m_Impl->m_Closed);
	}
	{
		CAutoMutex<CMutex> lock(m_Impl->m_OutboxMutex);
		m_Impl->m_Outbox.clear();
	}
	m_Impl->m_Deferred.clear();
	if (m_ConnectionClosed != nullptr)
		for (uint i = 0; i < closedList.size(); ++i)
			m_ConnectionClosed(closedList[i]);
}

void CCtrlEndpoint::postAnswer(CCtrlCall &call)
{
	COutMessage m;
	m.Connection = call.m_Connection;
	m.Id = call.m_Id;
	m.Error = call.m_Error;
	m.Message = call.m_Message;
	m.Json = m_Impl->wrap(call.m_Result).toString();
	m.Attachments.swap(call.m_Attachments);
	m_Impl->postAnswer(m);
}

void CCtrlEndpoint::writeRecord(const std::string &state)
{
	CJsonValue record;
	{
		CAutoMutex<CMutex> lock(m_Impl->m_IdentityMutex);
		record = m_Impl->m_Identity;
	}
	// the endpoint's own, never what the identity of the process says
	record.set("trusted", m_Impl->m_Trusted);
	record.set("state", state);
	record.set("updated", getCtrlTimeMs());
	std::string error;
	if (!CCtrlRegistry::writeRecord(m_Impl->m_RecordPath, record, error))
		nlwarning("CTRL: %s", error.c_str());
}

void CCtrlEndpoint::setIdentity(const std::string &key, const CJsonValue &value)
{
	{
		CAutoMutex<CMutex> lock(m_Impl->m_IdentityMutex);
		if (m_Impl->m_Identity.get(key).toString() == value.toString())
			return;
		m_Impl->m_Identity.set(key, value);
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
	info.Privileged = method->isPrivileged();
	info.BuiltIn = false;
	CAutoMutex<CMutex> lock(m_Impl->m_MethodsMutex);
	m_Impl->m_Methods.push_back(info);
}

void CCtrlEndpoint::setState(const std::string &state, const CJsonValue &detail, bool settled)
{
	std::string detailText = detail.isNull() ? std::string() : detail.toString();
	std::string previous;
	{
		CAutoMutex<CMutex> lock(m_Impl->m_StateMutex);
		m_Impl->m_StateSettled = settled;
		if (m_Impl->m_State == state && m_Impl->m_StateDetail == detailText)
			return;
		previous = m_Impl->m_State;
		bool changed = previous != state;
		m_Impl->m_State = state;
		m_Impl->m_StateDetail = detailText;
		if (changed)
			m_Impl->m_StateSince = getCtrlTimeMs();
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
	if (m_Capture != nullptr && !line.empty() && m_Capture->size() < s_MaxCommandLines)
		m_Capture->push_back(line);
}

uint32 CCtrlEndpoint::getCallingConnection() const
{
	return m_CallingConnection;
}

bool CCtrlEndpoint::isLeaseHeld() const
{
	return m_Started && m_Impl->m_LeaseHeld;
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

bool isCancelled(const std::vector<TCallRef> &cancels, uint32 connection, const std::string &id)
{
	for (uint k = 0; k < cancels.size(); ++k)
		if (cancels[k].first == connection && cancels[k].second == id)
			return true;
	return false;
}

} // anonymous namespace

void CCtrlEndpoint::update(bool allowControl)
{
	if (!m_Started)
		return;
	// the main thread is alive, also when a method pumps the events of the process and lands here again
	{
		CAutoMutex<CMutex> lock(m_Impl->m_StateMutex);
		m_Impl->m_Frame = m_Frame;
		m_Impl->m_LastUpdate = CTime::getLocalTime();
	}
	if (m_InUpdate)
		return;
	CUpdateGuard guard(m_InUpdate);

	// the calls that waited for an update that lets them run come first, in the order they came
	std::deque<CInCall> inbox;
	inbox.swap(m_Impl->m_Deferred);
	std::vector<TCallRef> cancels;
	std::vector<uint32> closedList;
	{
		CAutoMutex<CMutex> lock(m_Impl->m_InboxMutex);
		while (!m_Impl->m_Inbox.empty())
		{
			inbox.push_back(CInCall());
			inbox.back().swap(m_Impl->m_Inbox.front());
			m_Impl->m_Inbox.pop_front();
		}
		cancels.swap(m_Impl->m_Cancels);
		closedList.swap(m_Impl->m_Closed);
	}
	// every call of a connection that closed is dropped in this update, queued, deferred or open: none is left
	// for a later one, so the ids are not kept
	std::set<uint32> closed(closedList.begin(), closedList.end());

	// calls of a connection that closed, and calls cancelled, are dropped: those still queued too. A method that stops
	// the endpoint ends the update, and the stop is finished as it returns.
	for (uint i = 0; i < m_Open.size() && m_Started;)
	{
		CCtrlCall *call = m_Open[i];
		bool gone = closed.find(call->getConnection()) != closed.end();
		if (!gone && !isCancelled(cancels, call->getConnection(), call->getId()))
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
			m_Impl->postError(call->getConnection(), call->getId(), "cancelled");
		delete call;
		m_Open.erase(m_Open.begin() + i);
	}

	// the calls open since an earlier update are polled; those that start now are not, call() just ran. A pump deep in
	// other code (a loading screen) runs what only reads; the rest waits for the next full update, and so does every
	// later call of a connection that has one waiting, so the calls of a connection run in the order they came.
	size_t polled = m_Open.size();
	std::set<uint32> held;
	size_t next = 0;
	for (; next < inbox.size() && m_Started; ++next)
	{
		CInCall &in = inbox[next];
		if (closed.find(in.Connection) != closed.end())
			continue;
		if (isCancelled(cancels, in.Connection, in.Id))
		{
			m_Impl->postError(in.Connection, in.Id, "cancelled");
			continue;
		}
		std::map<std::string, ICtrlMethod *>::iterator it = m_Methods.find(in.Method);
		if (it == m_Methods.end())
		{
			// the channel thread checks the name against the same table; the call is answered all the same
			m_Impl->postError(in.Connection, in.Id, "no method '" + in.Method + "' on the main thread");
			continue;
		}
		if (!allowControl && (!it->second->isReadOnly() || held.find(in.Connection) != held.end()))
		{
			held.insert(in.Connection);
			m_Impl->m_Deferred.push_back(CInCall());
			m_Impl->m_Deferred.back().swap(in);
			continue;
		}
		CCtrlCall *call = new CCtrlCall();
		call->m_Connection = in.Connection;
		call->m_Id.swap(in.Id);
		call->m_Method.swap(in.Method);
		call->m_Args.swap(in.Args);
		call->m_StartTime = CTime::getLocalTime();
		m_CallingConnection = call->m_Connection;
		runMethodStep(it->second, *call, StepCall);
		m_CallingConnection = 0;
		if (call->isDone())
		{
			postAnswer(*call);
			delete call;
		}
		else
		{
			m_Open.push_back(call);
		}
	}
	// stopped by a method: the calls it did not reach are told why
	for (; next < inbox.size(); ++next)
		if (closed.find(inbox[next].Connection) == closed.end())
			m_Impl->postError(inbox[next].Connection, inbox[next].Id, m_Impl->m_ExitMessage);

	// the open calls: an answered one goes out
	for (uint i = 0; i < m_Open.size() && m_Started;)
	{
		CCtrlCall *call = m_Open[i];
		if (!call->isDone() && i < polled)
		{
			// a pump deep in other code polls what only reads: a poll of the others may act (turn the character in a
			// loading), or answer for an input the loading took
			std::map<std::string, ICtrlMethod *>::iterator method = m_Methods.find(call->getMethod());
			if (method != m_Methods.end() && (allowControl || method->second->isReadOnly()))
			{
				m_CallingConnection = call->getConnection();
				runMethodStep(method->second, *call, StepPoll);
				m_CallingConnection = 0;
			}
		}
		if (!call->isDone())
		{
			++i;
			continue;
		}
		if (i < polled)
			--polled;
		postAnswer(*call);
		delete call;
		m_Open.erase(m_Open.begin() + i);
	}

	// the connections that closed, once their calls are gone
	if (m_ConnectionClosed != nullptr)
		for (uint i = 0; i < closedList.size(); ++i)
			m_ConnectionClosed(closedList[i]);

	if (m_Impl->m_StopPending)
		finishStop();
}

} // NLMISC

/* End of ctrl_endpoint.cpp */
