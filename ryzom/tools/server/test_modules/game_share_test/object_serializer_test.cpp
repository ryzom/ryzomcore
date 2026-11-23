// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#endif

#include "nel/misc/common.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/sha1.h"
#include "nel/misc/thread.h"
#include "game_share/object.h"
#include "game_share/scenario.h"
#include "game_share/scenario_text_parser.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdio>
#include <limits>
#include <map>
#include <string>
#include <vector>

using namespace R2;

namespace {

// The kinds of values of the binary form, as they are on the wire. A stream written by any version of the
// code has these values: they do not change.
enum
{
	TagNull = 0,
	TagString = 1,
	TagTable = 3,
	TagNumberZero = 4,
	TagNumberUInt8 = 10,
	TagStringEmpty = 12,
	TagString8 = 13,
	TagTablePosition = 15,
	TagTableNpc = 16,
	TagTableRegion = 20,
	TagTableRtNpcGrp = 38,
	TagTableRtNpc = 39,
	TagNumberDouble = 49,
	TagHeader = 50
};

typedef std::vector<uint8> TBytes;

void put8(TBytes &bytes, uint8 value)
{
	bytes.push_back(value);
}

void put32(TBytes &bytes, uint32 value)
{
	bytes.push_back((uint8)(value & 0xff));
	bytes.push_back((uint8)((value >> 8) & 0xff));
	bytes.push_back((uint8)((value >> 16) & 0xff));
	bytes.push_back((uint8)((value >> 24) & 0xff));
}

void putString(TBytes &bytes, const std::string &value)
{
	put32(bytes, (uint32)value.size());
	bytes.insert(bytes.end(), value.begin(), value.end());
}

// what is in front of a tree that is not compressed: the flag, and the header with the version
void putHeader(TBytes &bytes)
{
	put8(bytes, 0);
	put8(bytes, TagHeader);
	put32(bytes, 1);
}

// offset of the kind of the root value in a stream that starts with putHeader
const size_t RootOffset = 6;

// writes a tree as a message does
TBytes writeTree(CObject *tree, bool compress = false)
{
	CObjectSerializerServer serializer(tree);
	if (compress) serializer.compress();
	NLMISC::CMemStream out;
	out.serial(serializer);
	return TBytes(out.buffer(), out.buffer() + out.length());
}

// Reads a stream as a message handler does. False if the read threw the stream exception, which is what the
// module layer catches per message; the serializer then holds no tree.
bool readStream(const TBytes &bytes, CObject::TSmartPtr &tree)
{
	NLMISC::CMemStream in(true);
	if (!bytes.empty()) in.fill(&bytes[0], (uint32)bytes.size());
	CObjectSerializerServer serializer;
	try
	{
		in.serial(serializer);
	}
	catch (const NLMISC::EStream &)
	{
		EXPECT_TRUE(serializer.getData() == nullptr);
		tree = nullptr;
		return false;
	}
	tree = serializer.getData();
	return true;
}

bool sameTree(const CObject *a, const CObject *b)
{
	// the test of a string accepts a reference id and not the other way round: both ways
	return a && b && a->equal(b) && b->equal(a);
}

// writes the tree, reads it back and expects an equal tree; gives the stream for a look at its bytes
TBytes expectRoundTrip(CObject *tree, const char *what)
{
	TBytes bytes = writeTree(tree);
	CObject::TSmartPtr back;
	EXPECT_TRUE(readStream(bytes, back)) << what;
	EXPECT_TRUE(sameTree(tree, back)) << what;
	return bytes;
}

void expectRefused(const TBytes &bytes, const char *what)
{
	CObject::TSmartPtr tree;
	EXPECT_FALSE(readStream(bytes, tree)) << what;
	EXPECT_TRUE(tree == nullptr) << what;
}

CObject::TSmartPtr newString(const std::string &value)
{
	return CObject::TSmartPtr(new CObjectString(value));
}

CObject::TSmartPtr newPosition(const std::string &instanceId, double x, double y, double z)
{
	CObject::TSmartPtr position(new CObjectTable());
	position->add("Class", std::string("Position"));
	position->add("InstanceId", instanceId);
	position->add("x", x);
	position->add("y", y);
	position->add("z", z);
	return position;
}

// A tree with every kind of value and tables of the classes that have a form of their own, with the entries of
// those in the order of their class, which is the order they are read back in.
CObject::TSmartPtr buildLegitimateTree(bool veryLongStrings = true)
{
	CObject::TSmartPtr root(new CObjectTable());
	root->add("Class", std::string("Scenario"));
	root->add("InstanceId", std::string("Client1_1"));
	root->add("Empty", std::string(""));
	root->add("Short", std::string("a short string"));
	root->add("Long", std::string(300, 'l'));
	root->add("Ref", CObject::TSmartPtr(new CObjectRefId("Client1_12")));
	root->add("EmptyRef", CObject::TSmartPtr(new CObjectRefId("")));
	root->add("LongRef", CObject::TSmartPtr(new CObjectRefId(std::string(300, 'r'))));
	if (veryLongStrings)
	{
		root->add("VeryLong", std::string(70000, 'v'));
		root->add("VeryLongRef", CObject::TSmartPtr(new CObjectRefId(std::string(70000, 'R'))));
	}
	root->add("Zero", (sint64)0);
	root->add("UInt8", (sint64)200);
	root->add("UInt16", (sint64)60000);
	root->add("UInt32", (sint64)4000000000LL);
	root->add("SInt8", (sint64)-100);
	root->add("SInt16", (sint64)-30000);
	root->add("SInt32", (sint64)-2000000000LL);
	root->add("Ratio", 1.5);
	root->add("Small", -0.125);
	root->add("Huge", 1e300);
	root->add("IntegralDouble", 42.0);

	CObject::TSmartPtr values(new CObjectTable());
	values->add("", std::string("first"));
	values->add("", (sint64)2);
	values->add("", CObject::TSmartPtr(new CObjectTable()));
	root->add("Values", values);

	CObject::TSmartPtr behavior(new CObjectTable());
	behavior->add("Class", std::string("Behavior"));
	behavior->add("InstanceId", std::string("Client1_4"));
	behavior->add("Type", std::string(""));
	behavior->add("ZoneId", std::string(""));

	CObject::TSmartPtr npc(new CObjectTable());
	npc->add("Class", std::string("Npc"));
	npc->add("InstanceId", std::string("Client1_2"));
	npc->add("Base", std::string("palette.entities.npcs.guards.f_guard_45"));
	npc->add("Position", newPosition("Client1_3", 12.5, -7.25, 0.0));
	npc->add("Angle", 1.571);
	npc->add("Behavior", behavior);
	npc->add("Sex", (sint64)1);
	npc->add("Function", std::string("guard"));
	npc->add("", std::string("an unnamed value"));
	npc->add("Name", std::string("Bob"));
	root->add("Npc", npc);

	// a custom npc that does not have all the properties of its class
	CObject::TSmartPtr npcCustom(new CObjectTable());
	npcCustom->add("Class", std::string("NpcCustom"));
	npcCustom->add("InstanceId", std::string("Client1_5"));
	npcCustom->add("Base", std::string("palette.entities.npcs.civils.f_civil_45"));
	npcCustom->add("Position", newPosition("Client1_6", 0.001, 2147483.647, -2147483.648));
	npcCustom->add("Angle", (sint64)-3);
	npcCustom->add("HairType", (sint64)5);
	npcCustom->add("Name", std::string("Alice"));
	root->add("NpcCustom", npcCustom);

	CObject::TSmartPtr points(new CObjectTable());
	for (uint i = 0; i < 3; ++i)
	{
		CObject::TSmartPtr vertex(new CObjectTable());
		vertex->add("Class", std::string("RegionVertex"));
		vertex->add("InstanceId", NLMISC::toString("Client2_%u", 10 + 2 * i));
		vertex->add("Position", newPosition(NLMISC::toString("Client2_%u", 11 + 2 * i), 1.0 * i, 2.0 * i, -1.0));
		points->add("", vertex);
	}
	CObject::TSmartPtr region(new CObjectTable());
	region->add("Class", std::string("Region"));
	region->add("InstanceId", std::string("Client2_9"));
	region->add("Name", std::string("Place 1"));
	region->add("Points", points);
	root->add("Region", region);

	CObject::TSmartPtr step(new CObjectTable());
	step->add("Class", std::string("ChatStep"));
	step->add("InstanceId", std::string("Client255_16777215"));
	step->add("Time", (sint64)3);
	step->add("Actions", CObject::TSmartPtr(new CObjectTable()));
	step->add("Name", std::string(""));
	root->add("Step", step);

	// the run time classes: an id of their own form, and a name that is twice in the list of RtNpc
	CObject::TSmartPtr rtNpc(new CObjectTable());
	rtNpc->add("Class", std::string("RtNpc"));
	rtNpc->add("Id", std::string("RtNpc_9"));
	rtNpc->add("Name", std::string("npc"));
	rtNpc->add("Keywords", std::string("keyword"));
	rtNpc->add("Sheet", std::string("object_chest_wisdom_std_sel.creature"));
	rtNpc->add("Pt", newString("not a class that is registered"));
	CObject::TSmartPtr rtNpcs(new CObjectTable());
	rtNpcs->add("", rtNpc);
	CObject::TSmartPtr rtGrp(new CObjectTable());
	rtGrp->add("Class", std::string("RtNpcGrp"));
	rtGrp->add("Id", std::string("RtNpcGrp_2147483647"));
	rtGrp->add("Name", std::string("group"));
	rtGrp->add("Children", CObject::TSmartPtr(new CObjectTable()));
	CObject::TSmartPtr rtGrps(new CObjectTable());
	rtGrps->add("", rtGrp);
	CObject::TSmartPtr rtAct(new CObjectTable());
	rtAct->add("Class", std::string("RtAct"));
	rtAct->add("Id", std::string("RtAct_0"));
	rtAct->add("NpcGrps", rtGrps);
	rtAct->add("Npcs", rtNpcs);
	root->add("RtAct", rtAct);

	return root;
}

// a table of a class and nothing else, for the cases that do not have the form of the class
CObject::TSmartPtr newClassTable(const char *className)
{
	CObject::TSmartPtr table(new CObjectTable());
	table->add("Class", std::string(className));
	return table;
}

// a zlib stream that holds the data as it is, in stored blocks
uint32 adler32(const TBytes &data)
{
	uint32 a = 1, b = 0;
	for (size_t i = 0; i < data.size(); ++i)
	{
		a = (a + data[i]) % 65521;
		b = (b + a) % 65521;
	}
	return (b << 16) | a;
}

TBytes zlibStore(const TBytes &data)
{
	TBytes out;
	put8(out, 0x78);
	put8(out, 0x01);
	size_t pos = 0;
	do
	{
		size_t len = std::min<size_t>(data.size() - pos, 65535);
		bool last = pos + len == data.size();
		put8(out, last ? 1 : 0);
		put8(out, (uint8)(len & 0xff));
		put8(out, (uint8)(len >> 8));
		put8(out, (uint8)(~len & 0xff));
		put8(out, (uint8)((~len >> 8) & 0xff));
		out.insert(out.end(), data.begin() + pos, data.begin() + pos + len);
		pos += len;
	} while (pos < data.size());
	uint32 sum = adler32(data);
	put8(out, (uint8)(sum >> 24));
	put8(out, (uint8)(sum >> 16));
	put8(out, (uint8)(sum >> 8));
	put8(out, (uint8)sum);
	return out;
}

// the stream of a compressed tree: the flag, the two sizes, the compressed data
TBytes compressedStream(const TBytes &compressed, uint32 compressedLen, uint32 uncompressedLen)
{
	TBytes bytes;
	put8(bytes, 1);
	put32(bytes, compressedLen);
	put32(bytes, uncompressedLen);
	bytes.insert(bytes.end(), compressed.begin(), compressed.end());
	return bytes;
}

// the binary form of a tree without the flag in front, which is what is compressed
TBytes treeBody(const TBytes &stream)
{
	return TBytes(stream.begin() + 1, stream.end());
}

// Reads a compressed stream: the read keeps the data, and the tree is made when it is asked for. Returns
// whether the read accepted the stream; tree is what getData() gives.
bool readCompressed(const TBytes &bytes, CObject::TSmartPtr &tree)
{
	NLMISC::CMemStream in(true);
	in.fill(&bytes[0], (uint32)bytes.size());
	CObjectSerializerServer serializer;
	try
	{
		in.serial(serializer);
	}
	catch (const NLMISC::EStream &)
	{
		EXPECT_TRUE(serializer.getData() == nullptr);
		tree = nullptr;
		return false;
	}
	EXPECT_TRUE(serializer.isCompresed());
	serializer.uncompress();
	tree = serializer.getData();
	return true;
}

// generic tables nested in each other, the innermost one empty
TBytes nestedTables(uint32 depth)
{
	TBytes bytes;
	putHeader(bytes);
	for (uint32 i = 1; i < depth; ++i)
	{
		put8(bytes, TagTable);
		put32(bytes, 1);
		putString(bytes, "");
	}
	put8(bytes, TagTable);
	put32(bytes, 0);
	return bytes;
}

// a table of count entries, each the number zero without a key
TBytes tableOfZeros(uint32 count)
{
	TBytes bytes;
	bytes.reserve(16 + 5 * (size_t)count);
	putHeader(bytes);
	put8(bytes, TagTable);
	put32(bytes, count);
	for (uint32 i = 0; i < count; ++i)
	{
		put32(bytes, 0);
		put8(bytes, TagNumberZero);
	}
	return bytes;
}

// tables of a run time class nested in each other through a property of the class, which is the form that
// recurses through the most functions per table
CObject::TSmartPtr nestedClassTables(uint32 depth)
{
	CObject::TSmartPtr inner;
	for (uint32 i = 0; i < depth; ++i)
	{
		CObject::TSmartPtr table(new CObjectTable());
		table->add("Class", std::string("RtNpcGrp"));
		table->add("Id", NLMISC::toString("RtNpcGrp_%u", i));
		if (inner) table->add("Children", inner);
		inner = table;
	}
	return inner;
}


std::string sha1Of(const TBytes &bytes)
{
	return NLMISC::getSHA1(bytes.empty() ? nullptr : &bytes[0], (uint32)bytes.size()).toString();
}

TBytes bytesOfHex(const char *hex)
{
	TBytes bytes;
	for (size_t i = 0; hex[i] && hex[i + 1]; i += 2)
	{
		uint8 byte = 0;
		for (size_t j = 0; j < 2; ++j)
		{
			char c = hex[i + j];
			byte = (uint8)(byte * 16 + (c <= '9' ? c - '0' : c - 'a' + 10));
		}
		bytes.push_back(byte);
	}
	return bytes;
}

// the classes of the tables of a tree that are written in the form of their class, with how many of each
void collectClassForms(CObject *tree, std::map<std::string, uint> &classes)
{
	if (!tree || !tree->isTable()) return;
	if (tree->isString("Class") && writeTree(tree)[RootOffset] != TagTable)
	{
		++classes[tree->toString("Class")];
	}
	for (uint32 i = 0; i < tree->getSize(); ++i)
	{
		collectClassForms(tree->getValueAtPos(i), classes);
	}
}

// The tree of the tests of the golden bytes (GOLDEN-TREE-BEGIN, to GOLDEN-TREE-END: the part that is built
// with the old code). Its stream was written once by the code before the limits of the reader
// (see the tests): what builds it here is not to change, or the bytes have to be made again with that code.

CObject::TSmartPtr goldenClass(const char *className)
{
	CObject::TSmartPtr table(new CObjectTable());
	table->add("Class", std::string(className));
	return table;
}

CObject::TSmartPtr goldenId(uint id)
{
	return CObject::TSmartPtr(new CObjectString(NLMISC::toString("Client1_%u", id)));
}

CObject::TSmartPtr goldenRef(const std::string &value)
{
	return CObject::TSmartPtr(new CObjectRefId(value));
}

CObject::TSmartPtr goldenPosition(uint id, double x, double y, double z)
{
	CObject::TSmartPtr position = goldenClass("Position");
	position->add("InstanceId", goldenId(id));
	position->add("x", x);
	position->add("y", y);
	position->add("z", z);
	return position;
}

CObject::TSmartPtr goldenBehavior(uint id)
{
	CObject::TSmartPtr behavior = goldenClass("Behavior");
	behavior->add("InstanceId", goldenId(id));
	behavior->add("Type", std::string(""));
	behavior->add("ZoneId", std::string(""));
	return behavior;
}

CObject::TSmartPtr goldenRtPosition(double x, double y, double z)
{
	// a class that has no form of its own
	CObject::TSmartPtr position = goldenClass("RtPosition");
	position->add("x", x);
	position->add("y", y);
	position->add("z", z);
	return position;
}

// Every class that has a form of its own at least once, each with its entries in the order of the class
// (the needed properties, the optional ones, the values without a key, the other properties), some without
// some of their needed properties; with the strings of the lengths at which the form of a string changes
// when longStrings is set.
CObject::TSmartPtr buildGoldenTree(bool longStrings)
{
	CObject::TSmartPtr root(new CObjectTable());

	{
		CObject::TSmartPtr npc = goldenClass("Npc");
		npc->add("InstanceId", goldenId(1));
		npc->add("Base", std::string("palette.entities.npcs.guards.f_guard_45"));
		npc->add("Position", goldenPosition(2, 12.5, -7.25, 0.0));
		npc->add("Angle", 1.571);
		npc->add("Behavior", goldenBehavior(3));
		npc->add("GabaritHeight", (sint64)3);
		npc->add("Sex", (sint64)1);
		npc->add("Function", std::string("guard"));
		npc->add("BotAttackable", (sint64)0);
		npc->add("", std::string("a value without a key"));
		npc->add("", (sint64)2);
		npc->add("Name", std::string("Bob"));
		CObject::TSmartPtr extra(new CObjectTable());
		extra->add("Key", std::string("value"));
		extra->add("", -0.5);
		npc->add("Extra", extra);
		root->add("Npc", npc);
	}
	{
		CObject::TSmartPtr npc = goldenClass("NpcCustom");
		npc->add("InstanceId", goldenId(4));
		npc->add("Base", std::string("palette.entities.npcs.civils.f_civil_45"));
		npc->add("Position", goldenPosition(5, 0.001, 2147483.0, -2147483.0));
		npc->add("Angle", (sint64)-3);
		npc->add("Behavior", goldenBehavior(6));
		npc->add("GabaritHeight", (sint64)1);
		npc->add("HairType", (sint64)5);
		npc->add("HairColor", (sint64)300);
		npc->add("FeetColor", (sint64)70000);
		npc->add("Name", std::string("Alice"));
		npc->add("Sex", (sint64)0);
		npc->add("Notes", std::string(""));
		root->add("NpcCustom", npc);
	}
	{
		CObject::TSmartPtr feature = goldenClass("NpcGrpFeature");
		feature->add("InstanceId", goldenId(7));
		feature->add("Name", std::string("Group 1"));
		CObject::TSmartPtr components(new CObjectTable());
		CObject::TSmartPtr member = goldenClass("Npc");
		member->add("InstanceId", goldenId(8));
		member->add("Angle", 0.0);
		components->add("", member);
		feature->add("Components", components);
		feature->add("ActivitiesId", CObject::TSmartPtr(new CObjectTable()));
		feature->add("Cost", (sint64)3);
		root->add("NpcGrpFeature", feature);
	}
	{
		CObject::TSmartPtr wayPoint = goldenClass("WayPoint");
		wayPoint->add("InstanceId", goldenId(9));
		wayPoint->add("Position", goldenPosition(10, -1.0, 2.0, -3.0));
		root->add("WayPoint", wayPoint);
	}
	{
		CObject::TSmartPtr points(new CObjectTable());
		for (uint i = 0; i < 3; ++i)
		{
			CObject::TSmartPtr vertex = goldenClass("RegionVertex");
			vertex->add("InstanceId", goldenId(12 + 2 * i));
			vertex->add("Position", goldenPosition(13 + 2 * i, 1.0 * i, 2.0 * i, -1.0));
			points->add("", vertex);
		}
		CObject::TSmartPtr region = goldenClass("Region");
		region->add("InstanceId", goldenId(11));
		region->add("Name", std::string("Place 1"));
		region->add("Points", points);
		root->add("Region", region);
	}
	{
		CObject::TSmartPtr points(new CObjectTable());
		for (uint i = 0; i < 2; ++i)
		{
			CObject::TSmartPtr wayPoint = goldenClass("WayPoint");
			wayPoint->add("InstanceId", goldenId(21 + 2 * i));
			wayPoint->add("Position", goldenPosition(22 + 2 * i, 100.0 + i, 200.125, 0.0));
			points->add("", wayPoint);
		}
		CObject::TSmartPtr road = goldenClass("Road");
		road->add("InstanceId", goldenId(20));
		road->add("Name", std::string("Route 1"));
		road->add("Points", points);
		root->add("Road", road);
	}
	{
		CObject::TSmartPtr step = goldenClass("ActivityStep");
		step->add("InstanceId", goldenId(31));
		step->add("Type", std::string("None"));
		step->add("TimeLimitValue", std::string("10"));
		step->add("EventsIds", CObject::TSmartPtr(new CObjectTable()));
		step->add("Chat", goldenRef(""));
		step->add("ActivityZoneId", goldenRef("Client1_11"));
		step->add("Name", std::string("Activity 1 : Wander for 10s"));
		step->add("TimeLimit", std::string("Few Sec"));
		step->add("Activity", std::string("Wander"));
		CObject::TSmartPtr components(new CObjectTable());
		components->add("", step);
		CObject::TSmartPtr sequence = goldenClass("ActivitySequence");
		sequence->add("InstanceId", goldenId(30));
		sequence->add("Name", std::string("Seq1"));
		sequence->add("Repeating", (sint64)1);
		sequence->add("Components", components);
		root->add("ActivitySequence", sequence);
	}
	{
		CObject::TSmartPtr action = goldenClass("ChatAction");
		action->add("InstanceId", goldenId(42));
		action->add("Emote", std::string(""));
		action->add("Who", goldenRef("Client1_1"));
		action->add("Facing", goldenRef(""));
		action->add("Says", std::string("Client1_60"));
		CObject::TSmartPtr actions(new CObjectTable());
		actions->add("", action);
		CObject::TSmartPtr step = goldenClass("ChatStep");
		step->add("InstanceId", goldenId(41));
		step->add("Time", (sint64)3);
		step->add("Actions", actions);
		step->add("Name", std::string(""));
		CObject::TSmartPtr components(new CObjectTable());
		components->add("", step);
		CObject::TSmartPtr sequence = goldenClass("ChatSequence");
		sequence->add("InstanceId", goldenId(40));
		sequence->add("Name", std::string("Dialog 1"));
		sequence->add("Components", components);
		root->add("ChatSequence", sequence);
	}
	{
		CObject::TSmartPtr conditionType = goldenClass("ConditionType");
		conditionType->add("InstanceId", goldenId(52));
		conditionType->add("Type", std::string("is active"));
		conditionType->add("Value", goldenRef(""));
		CObject::TSmartPtr conditionStep = goldenClass("ConditionStep");
		conditionStep->add("InstanceId", goldenId(51));
		conditionStep->add("Entity", goldenRef("Client1_1"));
		conditionStep->add("Condition", conditionType);
		CObject::TSmartPtr conditions(new CObjectTable());
		conditions->add("", conditionStep);

		CObject::TSmartPtr actionType = goldenClass("ActionType");
		actionType->add("InstanceId", goldenId(54));
		actionType->add("Type", std::string("begin activity sequence"));
		actionType->add("Value", goldenRef("Client1_30"));
		CObject::TSmartPtr actionStep = goldenClass("ActionStep");
		actionStep->add("InstanceId", goldenId(53));
		actionStep->add("Entity", goldenRef("Client1_1"));
		actionStep->add("Action", actionType);
		CObject::TSmartPtr actions(new CObjectTable());
		actions->add("", actionStep);

		CObject::TSmartPtr eventType = goldenClass("EventType");
		eventType->add("InstanceId", goldenId(55));
		eventType->add("Type", std::string("end of activity step"));
		eventType->add("Value", goldenRef("Client1_31"));

		CObject::TSmartPtr logicAction = goldenClass("LogicEntityAction");
		logicAction->add("InstanceId", goldenId(50));
		logicAction->add("Conditions", conditions);
		logicAction->add("Actions", actions);
		logicAction->add("Event", eventType);
		logicAction->add("Name", std::string(""));
		root->add("LogicEntityAction", logicAction);
	}
	{
		CObject::TSmartPtr reaction = goldenClass("LogicEntityReaction");
		reaction->add("InstanceId", goldenId(56));
		reaction->add("LogicEntityAction", std::string("Client1_50"));
		reaction->add("ActionStep", std::string("Client1_53"));
		reaction->add("Name", std::string(""));
		root->add("LogicEntityReaction", reaction);
	}
	{
		CObject::TSmartPtr entry = goldenClass("TextManagerEntry");
		entry->add("InstanceId", goldenId(60));
		entry->add("Count", (sint64)2);
		entry->add("Text", std::string("Hello, homin!"));
		root->add("TextManagerEntry", entry);
	}
	{
		CObject::TSmartPtr children(new CObjectTable());
		children->add("", std::string("RtNpc_3"));
		CObject::TSmartPtr group = goldenClass("RtNpcGrp");
		group->add("Id", std::string("RtNpcGrp_2"));
		group->add("Name", std::string("group"));
		group->add("Children", children);
		group->add("AutoSpawn", (sint64)1);
		group->add("BotChat_parameters", std::string(""));
		group->add("BotEquipment", std::string(""));
		group->add("BotSheetClient", std::string(""));
		group->add("BotVerticalPos", std::string("auto"));
		group->add("Count", (sint64)1);
		group->add("GrpKeywords", std::string(""));
		group->add("GrpParameters", std::string("notxt\n"));
		group->add("Extra", std::string("not a property of the class"));
		CObject::TSmartPtr groups(new CObjectTable());
		groups->add("", group);

		CObject::TSmartPtr points(new CObjectTable());
		points->add("", goldenRtPosition(1.5, 2.5, 0.0));
		points->add("", goldenRtPosition(-1.5, -2.5, 0.25));
		CObject::TSmartPtr stateChildren(new CObjectTable());
		stateChildren->add("", std::string("RtNpcGrp_2"));
		CObject::TSmartPtr state = goldenClass("RtAiState");
		state->add("Id", std::string("RtAiState_4"));
		state->add("Name", std::string("state"));
		state->add("Children", stateChildren);
		state->add("AiActivity", std::string("normal"));
		state->add("AiMovement", std::string("stand_on_start_point"));
		state->add("AiProfileParams", (sint64)0);
		state->add("Keywords", std::string(""));
		state->add("VerticalPos", std::string("auto"));
		state->add("Pts", points);
		CObject::TSmartPtr states(new CObjectTable());
		states->add("", state);

		CObject::TSmartPtr npc = goldenClass("RtNpc");
		npc->add("Id", std::string("RtNpc_3"));
		npc->add("Name", std::string("npc"));
		npc->add("Children", CObject::TSmartPtr(new CObjectTable()));
		npc->add("ChatParameters", std::string(""));
		npc->add("Equipment", std::string(""));
		npc->add("IsStuck", (sint64)0);
		npc->add("Keywords", std::string("keyword"));
		npc->add("Sheet", std::string("object_chest_wisdom_std_sel.creature"));
		npc->add("SheetClient", std::string(""));
		npc->add("BotVerticalPos", std::string("auto"));
		npc->add("Angle", 0.5);
		npc->add("Pt", goldenRtPosition(21570.5, -1363.25, 0.0));
		CObject::TSmartPtr npcs(new CObjectTable());
		npcs->add("", npc);

		CObject::TSmartPtr actionsId(new CObjectTable());
		actionsId->add("", std::string("RtNpcEventHandlerAction_6"));
		CObject::TSmartPtr handler = goldenClass("RtNpcEventHandler");
		handler->add("Id", std::string("RtNpcEventHandler_5"));
		handler->add("Name", std::string("handler"));
		handler->add("Event", std::string("timer_t0_triggered"));
		handler->add("StatesByName", std::string("state"));
		handler->add("GroupsByName", std::string("group"));
		handler->add("ActionsId", actionsId);
		CObject::TSmartPtr handlers(new CObjectTable());
		handlers->add("", handler);

		CObject::TSmartPtr handlerAction = goldenClass("RtNpcEventHandlerAction");
		handlerAction->add("Id", std::string("RtNpcEventHandlerAction_6"));
		handlerAction->add("Action", std::string("set_timer_t0"));
		handlerAction->add("Name", std::string("action"));
		handlerAction->add("Parameters", std::string("100"));
		handlerAction->add("Children", CObject::TSmartPtr(new CObjectTable()));
		handlerAction->add("Weight", (sint64)1);
		CObject::TSmartPtr handlerActions(new CObjectTable());
		handlerActions->add("", handlerAction);

		CObject::TSmartPtr act = goldenClass("RtAct");
		act->add("Id", std::string("RtAct_2147483647"));
		act->add("NpcGrps", groups);
		act->add("FaunaGrps", CObject::TSmartPtr(new CObjectTable()));
		act->add("AiStates", states);
		act->add("Npcs", npcs);
		act->add("Events", handlers);
		act->add("Actions", handlerActions);
		root->add("RtAct", act);
	}

	if (longStrings)
	{
		const uint lengths[] = { 254, 255, 256, 65534, 65535, 65536 };
		CObject::TSmartPtr strings(new CObjectTable());
		for (uint i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i)
		{
			strings->add(NLMISC::toString("String%u", lengths[i]), std::string(lengths[i], (char)('a' + i)));
			strings->add(NLMISC::toString("RefId%u", lengths[i]), goldenRef(std::string(lengths[i], (char)('A' + i))));
		}
		root->add("Strings", strings);
	}

	return root;
}
// GOLDEN-TREE-END

} // namespace

// ***************************************************************************
// The wire

TEST(ObjectSerializer, KnownBytesOfATable)
{
	// { a = 1, "hi" } as every version writes it
	TBytes expected;
	putHeader(expected);
	put8(expected, TagTable);
	put32(expected, 2);
	putString(expected, "a");
	put8(expected, TagNumberUInt8);
	put8(expected, 1);
	putString(expected, "");
	put8(expected, TagString8);
	put8(expected, 2);
	put8(expected, 'h');
	put8(expected, 'i');

	CObject::TSmartPtr table(new CObjectTable());
	table->add("a", (sint64)1);
	table->add("", std::string("hi"));
	EXPECT_EQ(expected, writeTree(table));

	CObject::TSmartPtr back;
	ASSERT_TRUE(readStream(expected, back));
	EXPECT_TRUE(sameTree(table, back));
}

TEST(ObjectSerializer, KnownBytesOfAClassTable)
{
	// a position as every version writes it: the id of the class, the kinds of entries, the instance id as a
	// number and the coordinates in thousandths
	TBytes expected;
	putHeader(expected);
	put8(expected, TagTablePosition);
	put8(expected, 1);
	put32(expected, (1u << 24) | 5u);
	put32(expected, 1500);
	put32(expected, (uint32)-2250);
	put32(expected, 0);

	CObject::TSmartPtr position = newPosition("Client1_5", 1.5, -2.25, 0.0);
	EXPECT_EQ(expected, writeTree(position));

	CObject::TSmartPtr back;
	ASSERT_TRUE(readStream(expected, back));
	EXPECT_TRUE(sameTree(position, back));
}

TEST(ObjectSerializer, StreamWithoutHeaderIsTheFirstVersion)
{
	// old session files have no header, and their coordinates are in 64ths
	TBytes bytes;
	put8(bytes, 0);
	put8(bytes, TagTablePosition);
	put8(bytes, 1);
	put32(bytes, (1u << 24) | 5u);
	put32(bytes, 96);
	put32(bytes, (uint32)-128);
	put32(bytes, 0);

	CObject::TSmartPtr back;
	ASSERT_TRUE(readStream(bytes, back));
	EXPECT_TRUE(sameTree(newPosition("Client1_5", 1.5, -2.0, 0.0), back));
}

TEST(ObjectSerializer, NoTree)
{
	TBytes bytes = writeTree(nullptr);
	TBytes expected;
	putHeader(expected);
	put8(expected, TagNull);
	EXPECT_EQ(expected, bytes);

	CObject::TSmartPtr back;
	EXPECT_TRUE(readStream(bytes, back));
	EXPECT_TRUE(back == nullptr);
}

// ***************************************************************************
// The wire, as the code before the limits of the reader wrote it
//
// The streams of these tests were written by ryzom/common/src/game_share/object.cpp as it is at commit
// 2f83cc644 (last changed in 0f6f9dc26), built in a program of its own with the builder of the tree above
// (buildGoldenTree) and with the text parser for the shipped scenarios. They pin the ids of the kinds and of
// the classes, the lists of properties of each class and their order, the widths of the counts and the
// lengths at which a string changes form: the writer still writes these bytes, and the reader reads them.

// the stream of buildGoldenTree(false), 1822 bytes
static const char *const GoldenStream =
	"003201000000030c000000030000004e7063100f010000010d2770616c657474652e656e7469746965732e6e7063732e"
	"6775617264732e665f67756172645f34350f0102000001d4300000aee3ffff00000000230600001701030000010c0c04"
	"000a03110a011e0d056775617264230402000d15612076616c756520776974686f75742061206b65790a020200040000"
	"004e616d650d03426f620500000045787472610302000000030000004b65790d0576616c756500000000310000000000"
	"00e0bf090000004e7063437573746f6d110b040000010d2770616c657474652e656e7469746965732e6e7063732e6369"
	"76696c732e665f636976696c5f34350f01050000010100000078fdff7f8802008048f4ffff1701060000010c0c0a0100"
	"0000000a05082c0100000000000000000000000000000000000000067011010002000d05416c69636509040100050000"
	"004e6f7465730c0d0000004e7063477270466561747572651609070000010d0747726f75702031030100000000000000"
	"100108000001000000000000000300000000010004000000436f73740a0308000000576179506f696e74120109000001"
	"0f010a00000118fcffffd007000048f4ffff06000000526567696f6e14090b0000010d07506c61636520310100060000"
	"00506f696e747303030000000000000013010c0000010f010d000001000000000000000018fcffff0000000013010e00"
	"00010f010f000001e8030000d007000018fcffff000000001301100000010f0111000001d0070000a00f000018fcffff"
	"04000000526f61641509140000010d07526f7574652031010006000000506f696e747303020000000000000012011500"
	"00010f0116000001a0860100bd0d030000000000000000001201170000010f0118000001888a0100bd0d030000000000"
	"10000000416374697669747953657175656e636518011e0000010d04536571310a0103010000000000000019011f0000"
	"010d044e6f6e650d02313003000000002d2e0a436c69656e74315f31310d1b41637469766974792031203a2057616e64"
	"657220666f72203130730d07466577205365630d0657616e6465720c0000004368617453657175656e63651a01280000"
	"010d084469616c6f6720310301000000000000001b01290000010a030301000000000000001c012a0000010c2e09436c"
	"69656e74315f312d0d0a436c69656e74315f36300c110000004c6f676963456e74697479416374696f6e1d0132000001"
	"0301000000000000002301330000012e09436c69656e74315f312401340000010d096973206163746976652d03010000"
	"00000000001e09350000012e09436c69656e74315f31010006000000416374696f6e1f01360000010d17626567696e20"
	"61637469766974792073657175656e63652e0a436c69656e74315f33302001370000010d14656e64206f662061637469"
	"7669747920737465702e0a436c69656e74315f33310c130000004c6f676963456e746974795265616374696f6e210138"
	"0000010d0a436c69656e74315f35300d0a436c69656e74315f35330c10000000546578744d616e61676572456e747279"
	"22013c0000010a020d0d48656c6c6f2c20686f6d696e210500000052744163742501ffffff7f03010000000000000026"
	"09020000000d0567726f75700301000000000000000d0752744e70635f330a010c0c0c0d046175746f0a010c0d066e6f"
	"7478740a01000500000045787472610d1b6e6f7420612070726f7065727479206f662074686520636c61737303000000"
	"000301000000000000002a01040000000d0573746174650301000000000000000d0a52744e70634772705f320d066e6f"
	"726d616c0d147374616e645f6f6e5f73746172745f706f696e74040c0d046175746f0302000000000000000304000000"
	"05000000436c6173730d0a5274506f736974696f6e010000007831000000000000f83f01000000793100000000000004"
	"40010000007a0400000000030400000005000000436c6173730d0a5274506f736974696f6e0100000078310000000000"
	"00f8bf01000000793100000000000004c0010000007a31000000000000d03f0301000000000000002701030000000d03"
	"6e706303000000000c0c040d076b6579776f72640d076b6579776f72640d246f626a6563745f63686573745f77697364"
	"6f6d5f7374645f73656c2e63726561747572650c0d046175746f31000000000000e03f030400000005000000436c6173"
	"730d0a5274506f736974696f6e01000000783100000000a010d54001000000793100000000004d95c0010000007a0403"
	"01000000000000002b01050000000d0768616e646c65720d1274696d65725f74305f7472696767657265640d05737461"
	"74650d0567726f75700301000000000000000d1952744e70634576656e7448616e646c6572416374696f6e5f36030100"
	"0000000000002c01060000000d0c7365745f74696d65725f74300d06616374696f6e0d0331303003000000000a01";

TEST(ObjectSerializer, GoldenBytesOfEveryClassForm)
{
	TBytes golden = bytesOfHex(GoldenStream);
	ASSERT_EQ(1822u, golden.size());
	EXPECT_EQ("DEDBCE96E833BAF713CE48328A2C985CA93D7B1B", sha1Of(golden));

	CObject::TSmartPtr tree = buildGoldenTree(false);
	EXPECT_EQ(golden, writeTree(tree));

	CObject::TSmartPtr back;
	ASSERT_TRUE(readStream(golden, back));
	EXPECT_TRUE(sameTree(tree, back));

	// every class that has a form of its own is in the tree, in that form
	const char *classes[] = { "Npc", "NpcCustom", "NpcGrpFeature", "Position", "WayPoint", "RegionVertex", "Region",
		"Road", "Behavior", "RtAct", "RtNpcGrp", "RtNpc", "RtAiState", "RtNpcEventHandler",
		"RtNpcEventHandlerAction", "ActivitySequence", "ActivityStep", "ChatSequence", "ChatStep", "ChatAction",
		"LogicEntityAction", "ActionStep", "ActionType", "EventType", "LogicEntityReaction", "TextManagerEntry",
		"ConditionType", "ConditionStep" };
	std::map<std::string, uint> found;
	collectClassForms(tree, found);
	EXPECT_EQ(sizeof(classes) / sizeof(classes[0]), found.size());
	for (uint i = 0; i < sizeof(classes) / sizeof(classes[0]); ++i)
	{
		EXPECT_EQ(1u, (uint)found.count(classes[i])) << classes[i];
	}
}

TEST(ObjectSerializer, GoldenHashOfTheStringForms)
{
	// The same tree with strings and reference ids of 254, 255, 256, 65534, 65535 and 65536 bytes: the
	// stream is 396782 bytes, so its hash is here and not its bytes.
	CObject::TSmartPtr tree = buildGoldenTree(true);
	TBytes bytes = writeTree(tree);
	EXPECT_EQ(396782u, bytes.size());
	EXPECT_EQ("A7C3B1E075C85EE9E460885F8B1CA614B55DF3D3", sha1Of(bytes));

	CObject::TSmartPtr back;
	ASSERT_TRUE(readStream(bytes, back));
	EXPECT_TRUE(sameTree(tree, back));
}

// ***************************************************************************
// The writer

TEST(ObjectSerializer, LegitimateTreeRoundTrip)
{
	CObject::TSmartPtr tree = buildLegitimateTree();
	TBytes bytes = expectRoundTrip(tree, "legitimate tree");

	// the tables of the classes are written in the form of their class
	EXPECT_EQ(TagTablePosition, writeTree(newPosition("Client1_3", 12.5, -7.25, 0.0))[RootOffset]);
	EXPECT_EQ(TagTableNpc, writeTree(tree->getAttr("Npc"))[RootOffset]);
	EXPECT_EQ(TagTableRegion, writeTree(tree->getAttr("Region"))[RootOffset]);
	EXPECT_EQ(TagTableRtNpc, writeTree(tree->findAttr("RtAct", "Npcs", "0"))[RootOffset]);

	// and the same through the compressed form
	TBytes compressed = writeTree(tree, true);
	ASSERT_FALSE(compressed.empty());
	EXPECT_EQ(1, compressed[0]);
	EXPECT_LT(compressed.size(), bytes.size());
	CObject::TSmartPtr back;
	ASSERT_TRUE(readCompressed(compressed, back));
	EXPECT_TRUE(sameTree(tree, back));
}

TEST(ObjectSerializer, ClassFormKeepsThousandths)
{
	// the coordinates of a position are written in thousandths: that is the form, a table is not written
	// without its class for it
	CObject::TSmartPtr position = newPosition("Client1_5", 1.23456, -0.0004, 100.9996);
	TBytes bytes = writeTree(position);
	EXPECT_EQ(TagTablePosition, bytes[RootOffset]);
	CObject::TSmartPtr back;
	ASSERT_TRUE(readStream(bytes, back));
	ASSERT_TRUE(back != nullptr);
	EXPECT_DOUBLE_EQ(1.235, back->toNumber("x"));
	EXPECT_DOUBLE_EQ(0.0, back->toNumber("y"));
	EXPECT_DOUBLE_EQ(101.0, back->toNumber("z"));
}

TEST(ObjectSerializer, ClassFormOrdersNamedEntries)
{
	// the entries of a class table are read back in the order of the class, whatever order they were in
	CObject::TSmartPtr shuffled(new CObjectTable());
	shuffled->add("z", 3.0);
	shuffled->add("InstanceId", std::string("Client1_5"));
	shuffled->add("Class", std::string("Position"));
	shuffled->add("y", 2.0);
	shuffled->add("x", 1.0);
	TBytes bytes = writeTree(shuffled);
	EXPECT_EQ(TagTablePosition, bytes[RootOffset]);
	CObject::TSmartPtr back;
	ASSERT_TRUE(readStream(bytes, back));
	EXPECT_TRUE(sameTree(newPosition("Client1_5", 1.0, 2.0, 3.0), back));
}

TEST(ObjectSerializer, ClassTableWithoutItsProperties)
{
	// the two trees that made the writer crash
	{
		CObject::TSmartPtr root(new CObjectTable());
		root->add("Unread", newClassTable("RtNpcGrp"));
		TBytes bytes = expectRoundTrip(root, "{ Unread = { Class = RtNpcGrp } }");
		// the table is the value of the first entry: after the kind of the root, its size and the key
		EXPECT_EQ(TagTable, bytes[RootOffset + 1 + 4 + 4 + 6]);
	}
	{
		CObject::TSmartPtr position = newClassTable("Position");
		TBytes bytes = expectRoundTrip(position, "{ Class = Position }");
		EXPECT_EQ(TagTable, bytes[RootOffset]);
	}
	// every class that is registered, with nothing but its name
	const char *classes[] = { "Npc", "NpcCustom", "NpcGrpFeature", "Position", "WayPoint", "RegionVertex", "Region",
		"Road", "Behavior", "RtAct", "RtNpcGrp", "RtNpc", "RtPosition", "RtAiState", "RtNpcEventHandler",
		"RtNpcEventHandlerAction", "ActivitySequence", "ActivityStep", "ChatSequence", "ChatStep", "ChatAction",
		"LogicEntityAction", "ActionStep", "ActionType", "EventType", "LogicEntityReaction", "TextManagerEntry",
		"ConditionType", "ConditionStep" };
	for (uint i = 0; i < sizeof(classes) / sizeof(classes[0]); ++i)
	{
		CObject::TSmartPtr table = newClassTable(classes[i]);
		expectRoundTrip(table, classes[i]);

		// and with every property the class may write in a form of its own holding a table
		const char *properties[] = { "InstanceId", "Id", "x", "y", "z", "Angle", "Position", "Name" };
		for (uint j = 0; j < sizeof(properties) / sizeof(properties[0]); ++j)
		{
			table->add(properties[j], CObject::TSmartPtr(new CObjectTable()));
		}
		expectRoundTrip(table, classes[i]);
	}
}

TEST(ObjectSerializer, ClassTableWithValuesOfAnotherForm)
{
	struct CCase
	{
		const char *What;
		CObject::TSmartPtr Tree;
	};
	std::vector<CCase> cases;

	const char *instanceIds[] = { "foo", "Client0_0", "Client01_1", "Client1_01", "Client256_1", "Client1_16777216",
		"Client1_2_3", "Client_1", "Client1_", "Client1", "client1_1", "Client1_1 ", "Client-1_1" };
	for (uint i = 0; i < sizeof(instanceIds) / sizeof(instanceIds[0]); ++i)
	{
		CCase c = { instanceIds[i], newPosition(instanceIds[i], 1.0, 2.0, 3.0) };
		cases.push_back(c);
	}
	{
		CObject::TSmartPtr tree = newClassTable("Position");
		tree->add("InstanceId", CObject::TSmartPtr(new CObjectRefId("Client1_5")));
		tree->add("x", 1.0);
		tree->add("y", 2.0);
		tree->add("z", 3.0);
		CCase c = { "instance id that is a reference id", tree };
		cases.push_back(c);
	}
	{
		CObject::TSmartPtr tree = newClassTable("Position");
		tree->add("InstanceId", (sint64)5);
		tree->add("x", 1.0);
		tree->add("y", 2.0);
		tree->add("z", 3.0);
		CCase c = { "instance id that is a number", tree };
		cases.push_back(c);
	}
	{
		CObject::TSmartPtr tree = newClassTable("Position");
		tree->add("InstanceId", std::string("Client1_5"));
		tree->add("x", std::string("1.0"));
		tree->add("y", 2.0);
		tree->add("z", 3.0);
		CCase c = { "coordinate that is a string", tree };
		cases.push_back(c);
	}
	{
		CCase c = { "coordinate too large for its form", newPosition("Client1_5", 1.0, 2147483.648, 3.0) };
		cases.push_back(c);
	}
	{
		CCase c = { "coordinate too small for its form", newPosition("Client1_5", -1e300, 2.0, 3.0) };
		cases.push_back(c);
	}
	{
		CObject::TSmartPtr tree = newPosition("Client1_5", 1.0, 2.0, 3.0);
		tree->add("x", 4.0);
		CCase c = { "needed property twice", tree };
		cases.push_back(c);
	}
	{
		CObject::TSmartPtr tree = newPosition("Client1_5", 1.0, 2.0, 3.0);
		tree->add("Class", std::string("Position"));
		CCase c = { "class twice", tree };
		cases.push_back(c);
	}
	{
		CObject::TSmartPtr tree(new CObjectTable());
		tree->add("Class", CObject::TSmartPtr(new CObjectRefId("Position")));
		tree->add("InstanceId", std::string("Client1_5"));
		tree->add("x", 1.0);
		tree->add("y", 2.0);
		tree->add("z", 3.0);
		CCase c = { "class that is a reference id", tree };
		cases.push_back(c);
	}
	{
		CObject::TSmartPtr tree = newClassTable("Npc");
		tree->add("InstanceId", std::string("Client1_5"));
		tree->add("Angle", std::string("north"));
		CCase c = { "angle that is a string", tree };
		cases.push_back(c);
	}
	{
		CObject::TSmartPtr tree = newClassTable("NpcCustom");
		tree->add("InstanceId", std::string("Client1_5"));
		CCase c = { "custom npc without an angle", tree };
		cases.push_back(c);
	}
	const char *rtIds[] = { "foo", "RtNpc_007", "RtNpc_4294967295", "RtNpc_2147483648", "RtNpc_", "RtNpc_1x",
		"xRtNpc_1", "RtNpcGrp_1", "RtNpc_-1", "" };
	for (uint i = 0; i < sizeof(rtIds) / sizeof(rtIds[0]); ++i)
	{
		CObject::TSmartPtr tree = newClassTable("RtNpc");
		tree->add("Id", std::string(rtIds[i]));
		CCase c = { rtIds[i], tree };
		cases.push_back(c);
	}
	{
		CObject::TSmartPtr tree = newClassTable("RtAct");
		tree->add("Id", (sint64)3);
		CCase c = { "run time id that is a number", tree };
		cases.push_back(c);
	}

	for (size_t i = 0; i < cases.size(); ++i)
	{
		TBytes bytes = expectRoundTrip(cases[i].Tree, cases[i].What);
		EXPECT_EQ(TagTable, bytes[RootOffset]) << cases[i].What;
	}
}

TEST(ObjectSerializer, RunTimeClassTableWithAnInstanceId)
{
	// "InstanceId" is not a property of the run time classes. The readers before this one read it in the form
	// it has in the other classes, a number, and it is written as any value: a table that has one is written
	// without its class, which they read as it is.
	const char *classes[] = { "RtAct", "RtNpcGrp", "RtNpc", "RtAiState", "RtNpcEventHandler", "RtNpcEventHandlerAction" };
	for (uint i = 0; i < sizeof(classes) / sizeof(classes[0]); ++i)
	{
		CObject::TSmartPtr table = newClassTable(classes[i]);
		table->add("Id", std::string(classes[i]) + "_7");
		table->add("Name", std::string("name"));
		TBytes bytes = expectRoundTrip(table, classes[i]);
		EXPECT_NE(TagTable, bytes[RootOffset]) << classes[i];

		table->add("InstanceId", std::string("Client1_5"));
		bytes = expectRoundTrip(table, classes[i]);
		EXPECT_EQ(TagTable, bytes[RootOffset]) << classes[i];
	}

	// A stream of the writers before this one, which wrote such a table in the form of its class: the id of
	// the class, needed and other properties, the eleven needed ones (the id as a number, the others
	// without a value), then one other property with its key and its value as any value.
	TBytes bytes;
	putHeader(bytes);
	put8(bytes, TagTableRtNpcGrp);
	put8(bytes, 1 | 8);
	put32(bytes, 7);
	for (uint i = 0; i < 10; ++i) put8(bytes, TagNull);
	put8(bytes, 1);
	put8(bytes, 0);
	putString(bytes, "InstanceId");
	const std::string instanceId = "Client1_5";
	put8(bytes, TagString8);
	put8(bytes, (uint8)instanceId.size());
	bytes.insert(bytes.end(), instanceId.begin(), instanceId.end());
	CObject::TSmartPtr back;
	ASSERT_TRUE(readStream(bytes, back));
	CObject::TSmartPtr expected = newClassTable("RtNpcGrp");
	expected->add("Id", std::string("RtNpcGrp_7"));
	expected->add("InstanceId", std::string("Client1_5"));
	EXPECT_TRUE(sameTree(expected, back));
}

TEST(ObjectSerializer, FixedPointBounds)
{
	// the thousandths of a coordinate are 32 bits: the largest and the smallest are still in the class form
	struct CBound
	{
		double Value;
		bool Fits;
	};
	const CBound bounds[] = {
		{ 2147483.647, true }, { 2147483.648, false }, { -2147483.648, true }, { -2147483.649, false },
		{ 2147483.0, true }, { -2147483.0, true }, { 1e10, false }, { -1e10, false }
	};
	for (uint i = 0; i < sizeof(bounds) / sizeof(bounds[0]); ++i)
	{
		CObject::TSmartPtr position = newPosition("Client1_5", bounds[i].Value, 0.0, 0.0);
		TBytes bytes = expectRoundTrip(position, "coordinate at the bounds of its form");
		EXPECT_EQ(bounds[i].Fits ? (int)TagTablePosition : (int)TagTable, (int)bytes[RootOffset]) << bounds[i].Value;
	}
}

TEST(ObjectSerializer, CompressionWithinTheLimitsOfTheReader)
{
	CObject::TSmartPtr back;

	// A tree whose binary form is larger than what the reader expands, and which compresses to very little:
	// it is not compressed, and it is read.
	{
		CObject::TSmartPtr tree(new CObjectTable());
		const uint32 count = CObjectSerializer::MaxUncompressedSize / CObjectSerializer::MaxStringLength + 1;
		for (uint32 i = 0; i < count; ++i)
		{
			tree->add("", std::string(CObjectSerializer::MaxStringLength, (char)('a' + i % 26)));
		}
		TBytes bytes = writeTree(tree, true);
		ASSERT_GT(bytes.size(), (size_t)CObjectSerializer::MaxUncompressedSize);
		EXPECT_EQ(0, bytes[0]);
		ASSERT_TRUE(readStream(bytes, back));
		EXPECT_TRUE(sameTree(tree, back));

		// one string less: within what the reader expands, compressed, and read
		tree->take(0);
		bytes = writeTree(tree, true);
		EXPECT_EQ(1, bytes[0]);
		EXPECT_LT(bytes.size(), (size_t)1024 * 1024);
		ASSERT_TRUE(readCompressed(bytes, back));
		EXPECT_TRUE(sameTree(tree, back));
	}

	// A tree that the reader would expand, and that compresses (64 values of a byte: to three quarters) to more
	// than the reader takes: not compressed
	{
		CObject::TSmartPtr tree(new CObjectTable());
		uint32 seed = 12345;
		const uint32 count = CObjectSerializer::MaxCompressedSize / CObjectSerializer::MaxStringLength + 2;
		for (uint32 i = 0; i < count; ++i)
		{
			std::string noise(CObjectSerializer::MaxStringLength, ' ');
			for (size_t j = 0; j < noise.size(); ++j)
			{
				seed = seed * 1664525u + 1013904223u;
				noise[j] = (char)('0' + (seed >> 26));
			}
			tree->add("", noise);
		}
		TBytes bytes = writeTree(tree, true);
		ASSERT_GT(bytes.size(), (size_t)CObjectSerializer::MaxCompressedSize);
		ASSERT_LT(bytes.size(), (size_t)CObjectSerializer::MaxUncompressedSize);
		EXPECT_EQ(0, bytes[0]);
		ASSERT_TRUE(readStream(bytes, back));
		EXPECT_TRUE(sameTree(tree, back));
	}
}

TEST(ObjectSerializer, NumberThatIsNotOne)
{
	// not equal to itself, so not to what is read back: it is written and read without harm
	CObject::TSmartPtr position = newPosition("Client1_5", std::numeric_limits<double>::quiet_NaN(), 2.0, 3.0);
	TBytes bytes = writeTree(position);
	EXPECT_EQ(TagTable, bytes[RootOffset]);
	CObject::TSmartPtr back;
	ASSERT_TRUE(readStream(bytes, back));
	ASSERT_TRUE(back != nullptr);
	EXPECT_TRUE(back->isNumber("x"));
	EXPECT_TRUE(back->toNumber("x") != back->toNumber("x"));
	EXPECT_DOUBLE_EQ(2.0, back->toNumber("y"));

	CObject::TSmartPtr infinite(new CObjectTable());
	infinite->add("Up", std::numeric_limits<double>::infinity());
	infinite->add("Down", -std::numeric_limits<double>::infinity());
	expectRoundTrip(infinite, "infinities");
}

TEST(ObjectSerializer, ClassTableWithMoreEntriesThanItsCounts)
{
	// more unnamed values than the 16 bit count of the class form
	{
		CObject::TSmartPtr region = newClassTable("Region");
		region->add("InstanceId", std::string("Client1_5"));
		region->add("Name", std::string("Place 1"));
		for (uint i = 0; i < 65535; ++i) region->add("", (sint64)i);
		TBytes bytes = expectRoundTrip(region, "65535 unnamed values");
		EXPECT_EQ(TagTableRegion, bytes[RootOffset]);
		region->add("", (sint64)65535);
		bytes = expectRoundTrip(region, "65536 unnamed values");
		EXPECT_EQ(TagTable, bytes[RootOffset]);
	}
	// more properties that are not of the class than the 16 bit count
	{
		CObject::TSmartPtr region = newClassTable("Region");
		region->add("InstanceId", std::string("Client1_5"));
		region->add("Name", std::string("Place 1"));
		for (uint i = 0; i < 65535; ++i) region->add(NLMISC::toString("Other%u", i), (sint64)i);
		TBytes bytes = expectRoundTrip(region, "65535 other properties");
		EXPECT_EQ(TagTableRegion, bytes[RootOffset]);
		region->add("OneMore", (sint64)65535);
		bytes = expectRoundTrip(region, "65536 other properties");
		EXPECT_EQ(TagTable, bytes[RootOffset]);
	}
	// more optional properties than the 8 bit count
	{
		CObject::TSmartPtr npc = newClassTable("Npc");
		npc->add("InstanceId", std::string("Client1_5"));
		npc->add("Angle", 0.5);
		for (uint i = 0; i < 255; ++i) npc->add("Sex", (sint64)i);
		TBytes bytes = expectRoundTrip(npc, "255 optional properties");
		EXPECT_EQ(TagTableNpc, bytes[RootOffset]);
		npc->add("Sex", (sint64)255);
		bytes = expectRoundTrip(npc, "256 optional properties");
		EXPECT_EQ(TagTable, bytes[RootOffset]);
	}
}

TEST(ObjectSerializer, IntegersOfMoreThan32Bits)
{
	CObject::TSmartPtr table(new CObjectTable());
	table->add("UInt32Max", (sint64)4294967295LL);
	table->add("AboveUInt32", (sint64)4294967296LL);
	table->add("SInt32Min", (sint64)-2147483648LL);
	table->add("BelowSInt32", (sint64)-2147483649LL);
	table->add("Large", (sint64)1 << 40);
	table->add("LargeNegative", -((sint64)1 << 40));
	table->add("LargestExact", (sint64)1 << 53);
	table->add("IntegralDouble", 1e12);
	expectRoundTrip(table, "integers of more than 32 bits");
}

// ***************************************************************************
// The reader

TEST(ObjectSerializer, TruncatedStream)
{
	// every stream that ends before the end of the tree is refused, and nothing is kept
	TBytes whole = writeTree(buildLegitimateTree(false));
	ASSERT_LT(whole.size(), 4000u);
	for (size_t length = 0; length < whole.size(); ++length)
	{
		TBytes cut(whole.begin(), whole.begin() + length);
		CObject::TSmartPtr tree;
		EXPECT_FALSE(readStream(cut, tree)) << "cut at " << length << " of " << whole.size();
	}
	CObject::TSmartPtr tree;
	EXPECT_TRUE(readStream(whole, tree));

	// and a compressed stream that ends before the end of its data
	TBytes compressed = writeTree(buildLegitimateTree(false), true);
	ASSERT_EQ(1, compressed[0]);
	for (size_t length = 1; length < compressed.size(); ++length)
	{
		TBytes cut(compressed.begin(), compressed.begin() + length);
		EXPECT_FALSE(readCompressed(cut, tree)) << "cut at " << length << " of " << compressed.size();
	}
	EXPECT_TRUE(readCompressed(compressed, tree));
	EXPECT_TRUE(tree != nullptr);
}

TEST(ObjectSerializer, UnknownKindOfValue)
{
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, 0xee);
		expectRefused(bytes, "unknown kind as the tree");
	}
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagTable);
		put32(bytes, 2);
		putString(bytes, "a");
		put8(bytes, TagNumberZero);
		putString(bytes, "b");
		put8(bytes, 51);
		expectRefused(bytes, "unknown kind in a table");
	}
	{
		// in each kind of entries of a class table: needed, optional, unnamed, other
		for (uint kind = 0; kind < 4; ++kind)
		{
			TBytes bytes;
			putHeader(bytes);
			put8(bytes, TagTableNpc);
			put8(bytes, (uint8)(1 << kind));
			if (kind == 0)
			{
				put32(bytes, 0);
			}
			else if (kind == 1)
			{
				put8(bytes, 1);
				put8(bytes, 0);
			}
			else if (kind == 2)
			{
				put8(bytes, 1);
				put8(bytes, 0);
			}
			else
			{
				put8(bytes, 1);
				put8(bytes, 0);
				putString(bytes, "Other");
			}
			put8(bytes, 0xee);
			expectRefused(bytes, "unknown kind in a class table");
		}
	}
	{
		// an optional property that the class does not have
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagTableNpc);
		put8(bytes, 2);
		put8(bytes, 1);
		put8(bytes, 200);
		put8(bytes, TagNumberZero);
		expectRefused(bytes, "optional property out of the list");
	}
}

TEST(ObjectSerializer, EntryWithoutValue)
{
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagTable);
		put32(bytes, 1);
		putString(bytes, "a");
		put8(bytes, TagNull);
		expectRefused(bytes, "entry of a table without a value");
	}
	for (uint kind = 1; kind < 4; ++kind)
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagTableNpc);
		put8(bytes, (uint8)(1 << kind));
		put8(bytes, 1);
		put8(bytes, 0);
		if (kind == 3) putString(bytes, "Other");
		put8(bytes, TagNull);
		expectRefused(bytes, "entry of a class table without a value");
	}
	{
		// a needed property without a value is how a table that does not have it is written
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagTableRegion);
		put8(bytes, 1);
		put32(bytes, (1u << 24) | 5u);
		put8(bytes, TagNull);
		CObject::TSmartPtr tree;
		ASSERT_TRUE(readStream(bytes, tree));
		CObject::TSmartPtr expected = newClassTable("Region");
		expected->add("InstanceId", std::string("Client1_5"));
		EXPECT_TRUE(sameTree(expected, tree));
	}
}

TEST(ObjectSerializer, HeaderOnlyInFront)
{
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagHeader);
		put32(bytes, 1);
		put8(bytes, TagNumberZero);
		expectRefused(bytes, "two headers");
	}
	{
		// as many headers as would overflow the stack if each one recursed
		TBytes bytes;
		put8(bytes, 0);
		for (uint i = 0; i < 1000000; ++i)
		{
			put8(bytes, TagHeader);
			put32(bytes, 1);
		}
		put8(bytes, TagNumberZero);
		expectRefused(bytes, "a million headers");
	}
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagTable);
		put32(bytes, 1);
		putString(bytes, "a");
		put8(bytes, TagHeader);
		put32(bytes, 1);
		put8(bytes, TagNumberZero);
		expectRefused(bytes, "header in a table");
	}
}

TEST(ObjectSerializer, DepthLimit)
{
	const uint32 maxDepth = CObjectSerializer::MaxDepth;

	// as deep as the limit: read, and safe for what walks it
	{
		CObject::TSmartPtr tree;
		ASSERT_TRUE(readStream(nestedTables(maxDepth), tree));
		ASSERT_TRUE(tree != nullptr);
		uint32 depth = 0, nodes = 0;
		EXPECT_TRUE(CObjectSerializer::measure(tree, depth, nodes));
		EXPECT_EQ(maxDepth, depth);
		EXPECT_EQ(maxDepth - 1, nodes);
		CObject::TSmartPtr copy = tree->clone();
		EXPECT_TRUE(sameTree(tree, copy));
		expectRoundTrip(tree, "tree as deep as the limit");
	}
	// one deeper, and much deeper than the stack is
	expectRefused(nestedTables(maxDepth + 1), "one table deeper than the limit");
	expectRefused(nestedTables(1000000), "a million nested tables");

	// the same with the tables of a class
	{
		CObject::TSmartPtr tree = nestedClassTables(maxDepth);
		TBytes bytes = expectRoundTrip(tree, "class tables as deep as the limit");
		EXPECT_EQ(TagTableRtNpcGrp, bytes[RootOffset]);
	}
	{
		CObject::TSmartPtr tree = nestedClassTables(maxDepth + 1);
		TBytes bytes = writeTree(tree);
		EXPECT_EQ(TagTableRtNpcGrp, bytes[RootOffset]);
		expectRefused(bytes, "class tables one deeper than the limit");
		uint32 depth = 0, nodes = 0;
		EXPECT_FALSE(CObjectSerializer::measure(tree, depth, nodes));
	}
}

TEST(ObjectSerializer, NodeLimit)
{
	const uint32 maxNodes = CObjectSerializer::MaxNodes;
	{
		CObject::TSmartPtr tree;
		ASSERT_TRUE(readStream(tableOfZeros(maxNodes), tree));
		ASSERT_TRUE(tree != nullptr);
		EXPECT_EQ(maxNodes, tree->getSize());
		uint32 depth = 0, nodes = 0;
		EXPECT_TRUE(CObjectSerializer::measure(tree, depth, nodes));
		EXPECT_EQ(1u, depth);
		EXPECT_EQ(maxNodes, nodes);
	}
	expectRefused(tableOfZeros(maxNodes + 1), "one entry more than the limit");

	// a table that says it has more entries than the stream can hold
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagTable);
		put32(bytes, 0xffffffffu);
		putString(bytes, "a");
		put8(bytes, TagNumberZero);
		expectRefused(bytes, "table size larger than the stream");
	}

	// the entries of the tables of a class count as those of any table: what is read can be written in either form
	{
		CObject::TSmartPtr tree = nestedClassTables(3);
		uint32 depth = 0, nodes = 0;
		EXPECT_TRUE(CObjectSerializer::measure(tree, depth, nodes));
		CObject::TSmartPtr back;
		ASSERT_TRUE(readStream(writeTree(tree), back));
		uint32 backDepth = 0, backNodes = 0;
		EXPECT_TRUE(CObjectSerializer::measure(back, backDepth, backNodes));
		EXPECT_EQ(depth, backDepth);
		EXPECT_EQ(nodes, backNodes);
	}
}

TEST(ObjectSerializer, OversizedLengths)
{
	const uint32 maxCompressed = CObjectSerializer::MaxCompressedSize;
	const uint32 maxUncompressed = CObjectSerializer::MaxUncompressedSize;
	const uint32 maxString = CObjectSerializer::MaxStringLength;
	CObject::TSmartPtr tree;
	TBytes someData(64, 0);

	// the sizes of compressed data, with little or no data behind
	EXPECT_FALSE(readCompressed(compressedStream(someData, 0xffffffffu, 0xffffffffu), tree));
	EXPECT_FALSE(readCompressed(compressedStream(someData, maxCompressed + 1, maxCompressed + 1), tree));
	EXPECT_FALSE(readCompressed(compressedStream(someData, 64, maxUncompressed + 1), tree));
	EXPECT_FALSE(readCompressed(compressedStream(someData, 0xffffffffu, 64), tree));
	// more than deflate can expand to
	EXPECT_FALSE(readCompressed(compressedStream(someData, 64, 64 * 1032 + 1), tree));
	// sizes within the limits, and less data than they say
	EXPECT_FALSE(readCompressed(compressedStream(someData, maxCompressed, maxUncompressed), tree));
	EXPECT_FALSE(readCompressed(compressedStream(someData, 65, 65), tree));

	// a string and a key longer than the stream
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagString);
		put32(bytes, 0xffffffffu);
		put8(bytes, 'a');
		expectRefused(bytes, "string longer than the stream");
	}
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagTable);
		put32(bytes, 1);
		put32(bytes, 0x7fffffffu);
		put8(bytes, 'a');
		expectRefused(bytes, "key longer than the stream");
	}
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagString8);
		put8(bytes, 200);
		put8(bytes, 'a');
		expectRefused(bytes, "short string longer than the stream");
	}

	// a string and a key that are in the stream and that the writer would not write again
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagString);
		putString(bytes, std::string(maxString, 's'));
		ASSERT_TRUE(readStream(bytes, tree));
		ASSERT_TRUE(tree != nullptr);
		EXPECT_EQ(maxString, tree->toString().size());
		expectRoundTrip(tree, "longest string");
	}
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagString);
		putString(bytes, std::string(maxString + 1, 's'));
		expectRefused(bytes, "string longer than the limit");
	}
	{
		TBytes bytes;
		putHeader(bytes);
		put8(bytes, TagTable);
		put32(bytes, 1);
		putString(bytes, std::string(maxString + 1, 'k'));
		put8(bytes, TagNumberZero);
		expectRefused(bytes, "key longer than the limit");
	}
}

TEST(ObjectSerializer, CompressedDataThatDisagreesWithItsSizes)
{
	CObject::TSmartPtr expected = buildLegitimateTree();
	TBytes body = treeBody(writeTree(expected));
	TBytes stored = zlibStore(body);
	CObject::TSmartPtr tree;

	// the data as it is
	ASSERT_TRUE(readCompressed(compressedStream(stored, (uint32)stored.size(), (uint32)body.size()), tree));
	EXPECT_TRUE(sameTree(expected, tree));

	// said to expand to more, and to less, than it does: read, and no tree
	EXPECT_TRUE(readCompressed(compressedStream(stored, (uint32)stored.size(), (uint32)body.size() + 1), tree));
	EXPECT_TRUE(tree == nullptr);
	EXPECT_TRUE(readCompressed(compressedStream(stored, (uint32)stored.size(), (uint32)body.size() - 1), tree));
	EXPECT_TRUE(tree == nullptr);
	EXPECT_TRUE(readCompressed(compressedStream(stored, (uint32)stored.size(), 1), tree));
	EXPECT_TRUE(tree == nullptr);

	// said to expand to nothing: refused when it is read, as nothing expands to more than 1032 times its size
	EXPECT_FALSE(readCompressed(compressedStream(TBytes(), 0, 1), tree));
	// nothing that expands to nothing
	EXPECT_TRUE(readCompressed(compressedStream(TBytes(), 0, 0), tree));
	EXPECT_TRUE(tree == nullptr);

	// data that is not what zlib writes
	{
		TBytes broken = stored;
		broken[0] = 0;
		EXPECT_TRUE(readCompressed(compressedStream(broken, (uint32)broken.size(), (uint32)body.size()), tree));
		EXPECT_TRUE(tree == nullptr);
	}
	{
		TBytes broken = stored;
		broken[broken.size() - 1] ^= 0xff;
		EXPECT_TRUE(readCompressed(compressedStream(broken, (uint32)broken.size(), (uint32)body.size()), tree));
		EXPECT_TRUE(tree == nullptr);
	}
	// the data of the legitimate writer, with one byte changed
	{
		TBytes compressed = writeTree(expected, true);
		ASSERT_EQ(1, compressed[0]);
		compressed[compressed.size() / 2] ^= 0x55;
		EXPECT_TRUE(readCompressed(compressed, tree));
		EXPECT_TRUE(tree == nullptr);
	}

	// data that expands as said to something that is not a tree
	{
		TBytes notATree;
		put8(notATree, TagHeader);
		put32(notATree, 1);
		put8(notATree, TagTable);
		put32(notATree, 3);
		putString(notATree, "a");
		put8(notATree, 0xee);
		TBytes storedNotATree = zlibStore(notATree);
		EXPECT_TRUE(readCompressed(compressedStream(storedNotATree, (uint32)storedNotATree.size(), (uint32)notATree.size()), tree));
		EXPECT_TRUE(tree == nullptr);
	}
	// and to a tree that is nested too deep
	{
		TBytes deep = treeBody(nestedTables(CObjectSerializer::MaxDepth + 1));
		TBytes storedDeep = zlibStore(deep);
		EXPECT_TRUE(readCompressed(compressedStream(storedDeep, (uint32)storedDeep.size(), (uint32)deep.size()), tree));
		EXPECT_TRUE(tree == nullptr);
	}
}

// ***************************************************************************
// The scenario that the values go in

TEST(ObjectSerializer, ScenarioStaysWithinTheLimits)
{
	CObject::TSmartPtr root(new CObjectTable());
	root->add("Class", std::string("Scenario"));
	root->add("InstanceId", std::string("Client1_1"));
	root->add("Acts", CObject::TSmartPtr(new CObjectTable()));
	root->add("Name", std::string("name"));
	CScenario scenario(root);

	// a message with no value
	EXPECT_FALSE(scenario.setNode("Client1_1", "Name", nullptr));
	EXPECT_FALSE(scenario.insertNode("Client1_1", "Acts", -1, "", nullptr));
	EXPECT_TRUE(scenario.setNode("Client1_1", "Name", newString("other name")));
	EXPECT_EQ("other name", root->toString("Name"));

	// a chain of 200 tables, the innermost one with an instance id: under the 2 tables above it
	CObject::TSmartPtr chain(new CObjectTable());
	chain->add("InstanceId", std::string("Client1_2"));
	for (uint i = 1; i < 200; ++i)
	{
		CObject::TSmartPtr outer(new CObjectTable());
		outer->add("", chain);
		chain = outer;
	}
	EXPECT_TRUE(scenario.insertNode("Client1_1", "Acts", -1, "", chain));

	// each value is within the limits of a read, the scenario would not be
	CObject::TSmartPtr tree;
	ASSERT_TRUE(readStream(nestedTables(55), tree));
	EXPECT_FALSE(scenario.insertNode("Client1_2", "", -1, "Deeper", tree));
	EXPECT_FALSE(scenario.setNode("Client1_2", "Deeper", tree));
	// 54 more tables, the last but one with an instance id: the scenario is as deep as the limit
	CObject::TSmartPtr deeper(new CObjectTable());
	deeper->add("InstanceId", std::string("Client1_9"));
	deeper->add("", CObject::TSmartPtr(new CObjectTable()));
	for (uint i = 2; i < 54; ++i)
	{
		CObject::TSmartPtr outer(new CObjectTable());
		outer->add("", deeper);
		deeper = outer;
	}
	EXPECT_TRUE(scenario.insertNode("Client1_2", "", -1, "Deeper", deeper));
	uint32 depth = 0, nodes = 0;
	EXPECT_TRUE(CObjectSerializer::measure(root, depth, nodes));
	EXPECT_EQ((uint32)CObjectSerializer::MaxDepth, depth);

	// the whole of it is written and read back
	expectRoundTrip(root, "scenario at the depth limit");

	// a table is not moved under itself
	CObject::TSmartPtr act(new CObjectTable());
	act->add("InstanceId", std::string("Client1_3"));
	CObject::TSmartPtr inner(new CObjectTable());
	inner->add("InstanceId", std::string("Client1_4"));
	act->add("Inner", inner);
	EXPECT_TRUE(scenario.insertNode("Client1_1", "Acts", -1, "", act));
	EXPECT_FALSE(scenario.moveNode("Client1_3", "", -1, "Client1_4", "", -1));
	EXPECT_FALSE(scenario.moveNode("Client1_3", "", -1, "Client1_3", "", -1));
	// nor to where the scenario would be nested too deep: its 2 tables under the 255 of the target
	EXPECT_FALSE(scenario.moveNode("Client1_3", "", -1, "Client1_9", "", -1));
	EXPECT_TRUE(scenario.moveNode("Client1_4", "", -1, "Client1_1", "Acts", -1));
	EXPECT_EQ(3u, root->getAttr("Acts")->getSize());
	EXPECT_TRUE(scenario.moveNode("Client1_4", "", -1, "Client1_9", "", -1));
	EXPECT_EQ(2u, root->getAttr("Acts")->getSize());

	// The entries of a table that is 255 deep are replaced by those of a table: the tables in it are one level
	// below, at the limit, and not two.
	CObject::TSmartPtr twoDeep(new CObjectTable());
	twoDeep->add("InstanceId", std::string("Client1_9"));
	twoDeep->add("", CObject::TSmartPtr(new CObjectTable()));
	CObject::TSmartPtr threeDeep = twoDeep->clone();
	threeDeep->getValueAtPos(1)->add("", CObject::TSmartPtr(new CObjectTable()));
	EXPECT_FALSE(scenario.setNode("Client1_9", "", threeDeep));
	EXPECT_TRUE(scenario.setNode("Client1_9", "", twoDeep));
	EXPECT_TRUE(CObjectSerializer::measure(root, depth, nodes));
	EXPECT_EQ((uint32)CObjectSerializer::MaxDepth, depth);

	// as many entries as the limit, and no more: the table is one entry, and it has the others
	const uint32 bulk = CObjectSerializer::MaxNodes - nodes - 1;
	ASSERT_TRUE(readStream(tableOfZeros(bulk), tree));
	EXPECT_TRUE(scenario.setNode("Client1_1", "Bulk", tree));
	EXPECT_TRUE(CObjectSerializer::measure(root, depth, nodes));
	EXPECT_EQ((uint32)CObjectSerializer::MaxNodes, nodes);
	EXPECT_FALSE(scenario.insertNode("Client1_1", "Acts", -1, "", newString("one more")));
	EXPECT_FALSE(scenario.setNode("Client1_1", "OneMore", newString("one more")));
	// a string takes another value, and the entries of a table are replaced by as many: nothing more
	EXPECT_TRUE(scenario.setNode("Client1_1", "Name", newString("a third name")));
	EXPECT_TRUE(scenario.setNode("Client1_1", "Bulk", tree));
	tree->add("", (sint64)0);
	EXPECT_FALSE(scenario.setNode("Client1_1", "Bulk", tree));
	EXPECT_TRUE(CObjectSerializer::measure(root, depth, nodes));
	EXPECT_EQ((uint32)CObjectSerializer::MaxNodes, nodes);
	EXPECT_EQ(bulk, root->getAttr("Bulk")->getSize());
}

// ***************************************************************************
// What a refused stream leaves behind

#if defined(NL_OS_WINDOWS) && defined(_DEBUG)

namespace {

void readRefusedStreams()
{
	CObject::TSmartPtr tree;

	// cut in the middle of nested tables of both forms
	TBytes whole = writeTree(buildLegitimateTree());
	for (size_t length = whole.size() - 600; length < whole.size(); length += 13)
	{
		readStream(TBytes(whole.begin(), whole.begin() + length), tree);
	}
	// an unknown kind, an entry without a value and a header deep in a tree
	for (uint kind = 0; kind < 3; ++kind)
	{
		TBytes bytes = nestedTables(100);
		bytes.resize(bytes.size() - 5);
		put8(bytes, TagTable);
		put32(bytes, 2);
		putString(bytes, "a");
		put8(bytes, TagString8);
		put8(bytes, 1);
		put8(bytes, 'a');
		putString(bytes, "b");
		put8(bytes, kind == 0 ? 0xee : kind == 1 ? TagNull : TagHeader);
		readStream(bytes, tree);
	}
	readStream(nestedTables(CObjectSerializer::MaxDepth + 1), tree);
	readStream(writeTree(nestedClassTables(CObjectSerializer::MaxDepth + 1)), tree);
	readStream(tableOfZeros(CObjectSerializer::MaxNodes + 1), tree);

	// compressed data: sizes out of range, less data than said, sizes that disagree, and not a tree
	TBytes body = treeBody(whole);
	TBytes stored = zlibStore(body);
	readCompressed(compressedStream(stored, 0xffffffffu, 64), tree);
	readCompressed(compressedStream(TBytes(64, 0), 1000, 1000), tree);
	readCompressed(compressedStream(stored, (uint32)stored.size(), (uint32)body.size() + 1), tree);
	readCompressed(compressedStream(stored, (uint32)stored.size(), (uint32)body.size() - 1), tree);
	TBytes deep = treeBody(nestedTables(CObjectSerializer::MaxDepth + 1));
	TBytes storedDeep = zlibStore(deep);
	readCompressed(compressedStream(storedDeep, (uint32)storedDeep.size(), (uint32)deep.size()), tree);
}

} // namespace

TEST(ObjectSerializer, RefusedStreamsLeakNothing)
{
	// The first run allocates what stays for the life of the process (the log, the class forms). After it, the
	// debug heap has as many blocks after a run as before it.
	readRefusedStreams();

	_CrtMemState before, after, difference;
	_CrtMemCheckpoint(&before);
	readRefusedStreams();
	_CrtMemCheckpoint(&after);
	bool different = _CrtMemDifference(&difference, &before, &after) != 0;
	if (different) _CrtMemDumpStatistics(&difference);
	EXPECT_EQ(0, (int)difference.lCounts[_NORMAL_BLOCK]);
	EXPECT_EQ(0, (int)difference.lSizes[_NORMAL_BLOCK]);
}

#endif

// ***************************************************************************
// The stack that the deepest tree takes

#ifdef NL_OS_WINDOWS

namespace {

size_t stackUsed()
{
	NT_TIB *tib = (NT_TIB *)NtCurrentTeb();
	return (size_t)((char *)tib->StackBase - (char *)tib->StackLimit);
}

// Reads, measures, copies, registers, writes and deletes a tree as deep as the limit on a thread of its own,
// and gives the stack that was committed for it (the stack only grows)
class CDeepTreeRunnable : public NLMISC::IRunnable
{
public:
	TBytes Stream;
	bool Ok;
	size_t Used[6];

	void run() NL_OVERRIDE
	{
		Ok = false;
		Used[0] = stackUsed();
		{
			CObject::TSmartPtr tree;
			if (!readStream(Stream, tree) || !tree) return;
			Used[1] = stackUsed();
			CObject::TSmartPtr copy = tree->clone();
			Used[2] = stackUsed();
			CScenario scenario(copy);
			Used[3] = stackUsed();
			TBytes again = writeTree(tree);
			Used[4] = stackUsed();
			Ok = again == Stream && sameTree(tree, copy);
		}
		Used[5] = stackUsed();
	}
};

} // namespace

TEST(ObjectSerializer, StackOfTheDeepestTree)
{
	const char *steps[] = { "thread start", "read", "clone", "instance map", "write", "equal and delete" };
	const uint32 testDepth = CObjectSerializer::MaxDepth;
	for (uint form = 0; form < 2; ++form)
	{
		CDeepTreeRunnable runnable;
		runnable.Stream = form == 0 ? nestedTables(testDepth) : writeTree(nestedClassTables(testDepth));
		NLMISC::IThread *thread = NLMISC::IThread::create(&runnable);
		thread->start();
		thread->wait();
		delete thread;
		ASSERT_TRUE(runnable.Ok);
		for (uint i = 0; i < 6; ++i)
		{
			printf("stack of a tree of %u %s tables after %s: %u KiB\n", testDepth,
				form == 0 ? "plain" : "class", steps[i], (uint)(runnable.Used[i] / 1024));
		}
		// The main thread of a service or of the client has 1 MiB on this platform, and the handler of a
		// message is not deep in it. A debug build, which this is written for, takes several times the stack
		// of a release build.
		EXPECT_LT(runnable.Used[5], 640u * 1024u);
	}
}

#endif

// ***************************************************************************
// The scenarios that ship with the client

#ifdef RYZOM_EXAMPLES_DIR
TEST(ObjectSerializer, ShippedExampleScenarios)
{
	const char *files[] = { "Example_01_-_A_Hearty_Meal.r2", "Plight_Of_Trigio.r2" };
	// The size and the SHA-1 of the stream that the code at commit 2f83cc644 writes for each file (see the
	// golden bytes above for how they were made). The SHA-256 of the same streams:
	// 941c5628c1cc03d9e6c23e65bc01bb09ef17f56bcd21235df9b0e9ab9fc41f66
	// faf765d9e8814196685e255a591ec7193cb05a67cb80d033b3f33f10c9884d4d
	const uint32 sizes[] = { 92429, 150519 };
	const char *hashes[] = { "9BE24A57C792C5B719281B2E4223502640084CC0", "5EC528B7DA67B3147A1C9E2FE80AACF82DED5D0E" };
	for (uint i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
	{
		std::string path = std::string(RYZOM_EXAMPLES_DIR) + "/" + files[i];
		CScenarioValidator validator;
		CScenarioValidator::TValues values;
		std::string md5, signature;
		ASSERT_TRUE(validator.setScenarioToLoad(path, values, md5, signature, false)) << path;
		std::string filename, body;
		validator.applyLoad(filename, body, values);
		std::string error;
		CObject::TSmartPtr parsed = CScenarioTextParser::parse(body, nullptr, error);
		ASSERT_TRUE(parsed != nullptr) << path << ": " << error;

		uint32 depth = 0, nodes = 0;
		EXPECT_TRUE(CObjectSerializer::measure(parsed, depth, nodes)) << path;

		// The class forms round the coordinates and order the entries, so what is read back is not the tree
		// of the file entry for entry: it has as many tables and entries, and it is then read back as it is.
		TBytes bytes = writeTree(parsed);
		EXPECT_EQ(sizes[i], (uint32)bytes.size()) << path;
		EXPECT_EQ(hashes[i], sha1Of(bytes)) << path;
		CObject::TSmartPtr first;
		ASSERT_TRUE(readStream(bytes, first)) << path;
		uint32 firstDepth = 0, firstNodes = 0;
		EXPECT_TRUE(CObjectSerializer::measure(first, firstDepth, firstNodes)) << path;
		EXPECT_EQ(depth, firstDepth) << path;
		EXPECT_EQ(nodes, firstNodes) << path;
		TBytes again = expectRoundTrip(first, files[i]);
		EXPECT_EQ(bytes, again) << path;

		TBytes compressed = writeTree(parsed, true);
		CObject::TSmartPtr fromCompressed;
		ASSERT_TRUE(readCompressed(compressed, fromCompressed)) << path;
		EXPECT_TRUE(sameTree(first, fromCompressed)) << path;

		printf("%s: text %u bytes, %u table entries, tables nested %u deep, binary %u bytes, compressed %u bytes\n",
			files[i], (uint)body.size(), nodes, depth, (uint)bytes.size(), (uint)compressed.size());
	}
}
#endif
