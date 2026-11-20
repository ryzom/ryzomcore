// Ryzom - MMORPG Framework <https://wiki.ryzom.dev/>
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

// The network methods of the control endpoint: net.record and net.stats. The record is of impulses,
// the messages of msg.xml: received where impulseCallBack hands them to their handlers, sent where
// CNetManager::push takes them (the position of the character, POSITION, among them). The rest of the traffic,
// the actions the connection sends itself (targets, pickups), visual properties and acknowledgements, is not in
// it. In local mode nothing is sent.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/misc/bit_mem_stream.h"

#include "net_manager.h"

#include <deque>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace ClientCtrl {

namespace {

struct CImpulse
{
	uint64 Idx;
	uint64 Time;
	bool Out;
	std::string Name;
	uint32 Bytes;
	uint32 Tick;
};

struct CCount
{
	uint32 In, Out;
	uint64 BytesIn, BytesOut;
	CCount()
	    : In(0)
	    , Out(0)
	    , BytesIn(0)
	    , BytesOut(0)
	{
	}
};

const size_t s_MaxRecord = 20000;
std::deque<CImpulse> s_Record;
uint64 s_NextIdx = 1;
std::map<std::string, CCount> s_Counts;

void methodRecord(CCtrlCall &call)
{
	const CJsonValue &args = call.getArgs();
	std::string contains = toLowerAscii(args.get("contains").getString());
	std::string direction = args.get("direction").getString();
	uint max = (uint)std::max((sint64)1, std::min((sint64)5000, args.get("max").getInt(200)));
	bool hasFrom = args.has("from_idx");
	uint64 from = (uint64)std::max((sint64)0, args.get("from_idx").getInt(0));
	std::vector<const CImpulse *> picked;
	for (std::deque<CImpulse>::const_iterator it = s_Record.begin(); it != s_Record.end(); ++it)
	{
		if (hasFrom && it->Idx < from)
			continue;
		if (direction == "in" && it->Out)
			continue;
		if (direction == "out" && !it->Out)
			continue;
		if (!contains.empty() && toLowerAscii(it->Name).find(contains) == std::string::npos)
			continue;
		picked.push_back(&*it);
	}
	size_t begin = 0, end = picked.size();
	bool truncated = false;
	if (end - begin > max)
	{
		// from an index the oldest first, otherwise the newest
		if (hasFrom)
			end = begin + max;
		else
			begin = end - max;
		truncated = true;
	}
	CJsonValue list = CJsonValue::array();
	for (size_t i = begin; i < end; ++i)
	{
		const CImpulse &e = *picked[i];
		CJsonValue o = CJsonValue::object();
		o.set("idx", e.Idx);
		o.set("time", e.Time);
		o.set("dir", e.Out ? "out" : "in");
		o.set("name", e.Name);
		o.set("bytes", e.Bytes);
		o.set("tick", e.Tick);
		list.push(o);
	}
	CJsonValue r = CJsonValue::object();
	r.set("impulses", list);
	r.set("first_idx", s_Record.empty() ? s_NextIdx : s_Record.front().Idx);
	r.set("next_idx", end > begin ? picked[end - 1]->Idx + 1 : s_NextIdx);
	r.set("truncated", truncated);
	r.set("covers", "impulses of msg.xml only: the actions the connection sends itself (targets, pickups), visual properties and acknowledgements are not recorded");
	call.reply(r);
}

void methodStats(CCtrlCall &call)
{
	CJsonValue counts = CJsonValue::array();
	for (std::map<std::string, CCount>::const_iterator it = s_Counts.begin(); it != s_Counts.end(); ++it)
	{
		CJsonValue o = CJsonValue::object();
		o.set("name", it->first);
		o.set("in", it->second.In);
		o.set("out", it->second.Out);
		o.set("bytes_in", it->second.BytesIn);
		o.set("bytes_out", it->second.BytesOut);
		counts.push(o);
	}
	CJsonValue r = CJsonValue::object();
	r.set("connection", NetMngr.getConnectionStateCStr());
	r.set("server_tick", (uint32)NetMngr.getCurrentServerTick());
	r.set("ping_ms", NetMngr.getPing());
	r.set("impulses", counts);
	call.reply(r);
}

} // anonymous namespace

void netImpulse(bool out, const CBitMemStream &message)
{
	if (getEndpoint() == nullptr)
		return;
	// the name is read from a copy: the stream goes on to its handler, or to the connection, untouched. A message
	// being sent is read from its start (invert keeps the bit position of the writer, resetBufPos clears it); a
	// message received is read from where its handler starts, as the copy has it.
	CBitMemStream copy(message);
	if (out)
	{
		copy.invert();
		copy.resetBufPos();
	}
	std::string name;
	try
	{
		GenericMsgHeaderMngr.popNameFromStream(name, copy);
	}
	catch (const EStream &)
	{
		name = "<unreadable>";
	}
	CImpulse e;
	e.Idx = s_NextIdx++;
	e.Time = getCtrlTimeMs();
	e.Out = out;
	e.Name = name.empty() ? std::string("<unknown>") : name;
	e.Bytes = (uint32)message.length();
	e.Tick = (uint32)NetMngr.getCurrentServerTick();
	s_Record.push_back(e);
	while (s_Record.size() > s_MaxRecord)
		s_Record.pop_front();
	CCount &count = s_Counts[e.Name];
	if (out)
	{
		++count.Out;
		count.BytesOut += e.Bytes;
	}
	else
	{
		++count.In;
		count.BytesIn += e.Bytes;
	}
}

void registerNetMethods(CCtrlEndpoint &endpoint)
{
	endpoint.addMethod(new CCtrlFunctionMethod("net.record",
	    "The impulses received and sent (the messages of msg.xml), newest last: index, time, direction, name, size, server tick. Filter by a word of the name and the direction; from_idx continues. "
	    "The actions the connection sends itself (targets, pickups), visual properties and acknowledgements are not impulses and are not in it.",
	    CCtrlSchema()
	        .integer("from_idx", "First index, inclusive.")
	        .str("contains", "Only names containing this (STRING_MANAGER, CONNECTION...).")
	        .choice("direction", "in|out", "Only one direction.")
	        .integer("max", "At most this many (200 by default)."),
	    true, methodRecord));
	endpoint.addMethod(new CCtrlFunctionMethod("net.stats", "The connection: its state, the server tick, the ping, and the impulses counted by name in each direction since the endpoint started.",
	    CCtrlSchema(), true, methodStats));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_net.cpp */
