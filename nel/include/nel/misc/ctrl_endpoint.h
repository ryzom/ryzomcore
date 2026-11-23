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

#ifndef NL_CTRL_ENDPOINT_H
#define NL_CTRL_ENDPOINT_H

#include "types_nl.h"
#include "json.h"
#include "time_nl.h"

#include <map>
#include <string>
#include <vector>

namespace NLMISC {

class CCtrlEndpoint;
class CLog;

/// The time on the clock that log lines, events and results carry: ms since the epoch
uint64 getCtrlTimeMs();

/// Something a method keeps with a call that completes later; deleted with the call
class ICtrlCallData
{
public:
	virtual ~ICtrlCallData() { }
};

/// A call to a method of an endpoint, from its arrival to its answer (main thread)
class CCtrlCall
{
public:
	uint32 getConnection() const { return m_Connection; }
	const std::string &getId() const { return m_Id; }
	const std::string &getMethod() const { return m_Method; }
	const CJsonValue &getArgs() const { return m_Args; }
	/// When the call arrived, on CTime::getLocalTime
	TTime getStartTime() const { return m_StartTime; }
	/// The ms since the call arrived
	TTime getElapsed() const;

	/// Answers the call; the endpoint adds the frame and the clock
	void reply(const CJsonValue &result);
	/// Answers with an error, and optionally what is known about it
	void fail(const std::string &message, const CJsonValue &details = CJsonValue());
	/// Binary data that goes with the result; returns the reference to put in the result. The data of a call is
	/// bounded (64 MB in all): an attachment beyond it fails the call, and the result given after is not sent.
	CJsonValue attach(const std::string &contentType, const std::string &bytes);
	bool isDone() const { return m_Done; }

	void setData(ICtrlCallData *data);
	ICtrlCallData *getData() const { return m_Data; }

	~CCtrlCall();

	typedef std::pair<std::string, std::string> TAttachment;

private:
	friend class CCtrlEndpoint;
	CCtrlCall();
	CCtrlCall(const CCtrlCall &);
	CCtrlCall &operator=(const CCtrlCall &);

	uint32 m_Connection;
	std::string m_Id;
	std::string m_Method;
	CJsonValue m_Args;
	TTime m_StartTime;
	bool m_Done;
	bool m_Error;
	CJsonValue m_Result;
	std::string m_Message;
	std::vector<TAttachment> m_Attachments;
	size_t m_AttachmentBytes;
	ICtrlCallData *m_Data;
};

/**
 * A method of an endpoint. Methods are reachable from the endpoint only: they are not
 * commands, and nothing else (the command registry, a script, the server) can call them.
 * They run on the main thread, in CCtrlEndpoint::update. What a method throws is not caught: a crash in a method
 * is a crash of the process, with its report. A method fails a call with CCtrlCall::fail.
 */
class ICtrlMethod
{
public:
	/// name: a group and a name, "ui.click"; readOnly: it changes nothing, so every connection may call it
	ICtrlMethod(const std::string &name, const std::string &description, const CJsonValue &schema, bool readOnly);
	virtual ~ICtrlMethod() { }

	const std::string &getName() const { return m_Name; }
	const std::string &getDescription() const { return m_Description; }
	const CJsonValue &getSchema() const { return m_Schema; }
	bool isReadOnly() const { return m_ReadOnly; }

	/// A privileged method makes the process do what its user cannot through its interface (arbitrary script, arbitrary
	/// action handlers, commands, raw database writes): it runs only in an endpoint whose content is trusted (see
	/// CCtrlEndpoint::setTrusted). Set before CCtrlEndpoint::addMethod, which takes it with the method.
	void setPrivileged(bool privileged) { m_Privileged = privileged; }
	bool isPrivileged() const { return m_Privileged; }

	/// Starts a call: answer it, or leave it open, and poll() follows at every update until it is answered
	virtual void call(CCtrlCall &call) = 0;
	virtual void poll(CCtrlCall & /* call */) { }
	/// The call is dropped before its answer: its connection closed, or the endpoint stops
	virtual void cancel(CCtrlCall & /* call */) { }

private:
	std::string m_Name;
	std::string m_Description;
	CJsonValue m_Schema;
	bool m_ReadOnly;
	bool m_Privileged;
};

/// A method made of functions
class CCtrlFunctionMethod : public ICtrlMethod
{
public:
	typedef void (*TFunction)(CCtrlCall &call);

	CCtrlFunctionMethod(const std::string &name, const std::string &description, const CJsonValue &schema, bool readOnly, TFunction callFunction,
	    TFunction pollFunction = nullptr, TFunction cancelFunction = nullptr);

	virtual void call(CCtrlCall &call) NL_OVERRIDE;
	virtual void poll(CCtrlCall &call) NL_OVERRIDE;
	virtual void cancel(CCtrlCall &call) NL_OVERRIDE;

private:
	TFunction m_Call;
	TFunction m_Poll;
	TFunction m_Cancel;
};

/// Builds the JSON schema of the arguments of a method: an object with typed properties
class CCtrlSchema
{
public:
	CCtrlSchema();

	CCtrlSchema &str(const char *name, const char *description, bool required = false);
	CCtrlSchema &number(const char *name, const char *description, bool required = false);
	CCtrlSchema &integer(const char *name, const char *description, bool required = false);
	CCtrlSchema &boolean(const char *name, const char *description, bool required = false);
	/// A string out of a list, given as "a|b|c"
	CCtrlSchema &choice(const char *name, const char *values, const char *description, bool required = false);
	CCtrlSchema &strings(const char *name, const char *description, bool required = false);
	CCtrlSchema &numbers(const char *name, const char *description, bool required = false);
	CCtrlSchema &any(const char *name, const char *description, bool required = false);
	CCtrlSchema &object(const char *name, const char *description, bool required = false);

	operator const CJsonValue &() const { return m_Schema; }
	const CJsonValue &json() const { return m_Schema; }

private:
	CCtrlSchema &add(const char *name, const CJsonValue &property, bool required);
	CJsonValue m_Schema;
};

/**
 * The control endpoint of a process: it listens on a local pipe (see CLocalPipeListener for who
 * can connect), and writes its record to the registry (see CCtrlRegistry), so a tool on the same
 * machine finds it by name. Protocol: see ctrl_protocol.h.
 *
 * What it serves:
 * - Its built-in methods, answered by the channel thread, so they work while the main thread is
 *   busy or hung: ctrl.identity, ctrl.state, ctrl.wait_state, ctrl.log, ctrl.events, ctrl.methods,
 *   ctrl.lease. Each connection has its own queue of answers, so a tool that does not read stalls
 *   nobody else; one that takes none of its answers for 10 s is closed.
 * - The log lines of the process, indexed from 1 since the endpoint started, in a ring held by a
 *   byte budget. A read below the ring says which index is the first one still held.
 * - Events, a second ring of the same kind: what the process reports with addEvent, and the
 *   changes of its state.
 * - The methods the process adds, and ctrl.commands and ctrl.run_command over the command
 *   registry, all run by the main thread in update().
 *
 * The lease: a method that is not read-only needs it. A connection that calls one takes the lease
 * when nobody holds it; the lease is released when that connection closes or gives it back, which
 * it cannot while such a call of its own is not answered.
 *
 * Trust: what a process shows may be written by others (the players and web pages of a shard), and
 * a tool that reads it takes it in. The content of an endpoint is untrusted unless whoever started
 * the process says otherwise (setTrusted, from its command line); an untrusted endpoint refuses the
 * privileged methods (ICtrlMethod::setPrivileged), ctrl.run_command among them. The record and the
 * hello say "trusted", so a tool knows how to take what the endpoint gives.
 *
 * One endpoint a process: the log of the process goes to one ring.
 *
 * MT = main thread, CT = channel thread, AT = any thread.
 */
class CCtrlEndpoint
{
public:
	CCtrlEndpoint();
	/// Not from inside update(): a method does not delete the endpoint that runs it
	~CCtrlEndpoint();

	/// (MT) Before start(): everything the process will show comes from trusted authors, so the privileged methods
	/// run. Untrusted by default. The endpoint writes it into its record and its hello itself, whatever the identity
	/// given to start() or setIdentity() holds.
	void setTrusted(bool trusted);
	bool isTrusted() const;

	/// (MT) Listens and writes the record; identity holds what the record and the hello carry
	/// (program, build, log, ...). False with the error when it cannot; the record then says why.
	bool start(const std::string &name, const CJsonValue &identity, std::string &error);
	/// (MT) Writes the reason into the record, answers every call that has no answer yet with it (open calls are
	/// cancelled first), closes the connections and stops listening. From a method (inside update()), the stop is
	/// finished as update() returns.
	void stop(const std::string &exitReason);
	bool isStarted() const { return m_Started; }
	const std::string &getName() const { return m_Name; }

	/// (MT) An identity value, such as the account or the character; the record follows
	void setIdentity(const std::string &key, const CJsonValue &value);
	/// (MT) Why the process is about to exit, for the record if stop() is not reached with one
	void setExitReason(const std::string &reason);

	/// (MT) Adds a method; the endpoint owns it
	void addMethod(ICtrlMethod *method);

	/// (MT) Runs the calls that arrived and polls those still open. Re-entrant calls (a method
	/// that pumps the events of the process) return at once and leave the calls queued.
	/// allowControl false, for a pump deep in other code (a loading screen): only read-only methods
	/// run and are polled, the others wait for an update that allows them, and so do the calls of their
	/// connection that come after them: the calls of a connection run in the order they came.
	void update(bool allowControl = true);
	/// (MT) The state the process is in, with what goes with it; a change is an event too. settled false for a
	/// state the process only passes through (a loading): a wait that is not for it does not end in it
	void setState(const std::string &state, const CJsonValue &detail = CJsonValue(), bool settled = true);
	/// (MT) Counts a rendered frame
	void setFrame(uint64 frame);
	uint64 getFrame() const { return m_Frame; }

	/// (AT) Adds an event to the event ring
	void addEvent(const std::string &kind, const CJsonValue &data);

	/// (MT) The output a running command shows outside its log (the chat of a client); kept with the
	/// result of ctrl.run_command while one runs, ignored otherwise
	void captureOutput(const std::string &line);

	/// (MT) The connection of the call being run, so a method can tell who called; 0 outside a call
	uint32 getCallingConnection() const;

	/// (AT) Whether a connection holds the lease: the process is driven by a tool
	bool isLeaseHeld() const;

	/// (MT) Called in update() for each connection that closed, after its calls are dropped: what a tool
	/// left held (a key, a mouse button) can be let go of
	typedef void (*TConnectionClosed)(uint32 connection);
	void setConnectionClosedCallback(TConnectionClosed callback);

	class CImpl;

private:
	CCtrlEndpoint(const CCtrlEndpoint &);
	CCtrlEndpoint &operator=(const CCtrlEndpoint &);

	void writeRecord(const std::string &state);
	/// Removes the records of processes that are gone, but the last one of each name
	void pruneRecords(const std::string &dir);
	/// The rest of stop(): the open calls are cancelled and answered, then the channel closes. update() calls it as
	/// it returns when a method stopped the endpoint.
	void finishStop();
	/// Gives the answer of a call to the channel thread
	void postAnswer(CCtrlCall &call);
	void runCommandsMethod(CCtrlCall &call);
	void runRunCommandMethod(CCtrlCall &call);

	friend class CImpl;
	CImpl *m_Impl;
	bool m_Started;
	std::string m_Name;
	uint64 m_Frame;
	bool m_InUpdate;
	std::string m_ExitReason;
	std::vector<std::string> *m_Capture;
	uint32 m_CallingConnection;
	TConnectionClosed m_ConnectionClosed;
	std::map<std::string, ICtrlMethod *> m_Methods;
	// (MT) the calls left open
	std::vector<CCtrlCall *> m_Open;
};

} // NLMISC

#endif // NL_CTRL_ENDPOINT_H

/* End of ctrl_endpoint.h */
