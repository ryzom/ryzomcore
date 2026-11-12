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

#include "stdpch.h"
#include "rt_scenario_check.h"

#include "object.h"

#include "nel/misc/common.h"
#include "nel/misc/string_common.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <vector>

using namespace NLMISC;

namespace R2 {

namespace {

/// Largest part of a text of the tree that is copied into the description of a refusal
const uint32 s_MaxQuotedTextSize = 64;

/// Largest sheet id (a sheet id is 32 bits)
const double s_MaxSheetId = 4294967295.0;

/// Largest weight of an action (it is read as a signed 32 bit value)
const double s_MaxActionWeight = 2147483647.0;

// The names of the attributes that are read. They are kept as strings, so that looking one up
// does not make a string each time.
const std::string s_Empty;
const std::string s_KeyAction("Action");
const std::string s_KeyActDescription("ActDescription");
const std::string s_KeyActionsId("ActionsId");
const std::string s_KeyActs("Acts");
const std::string s_KeyAiMovement("AiMovement");
const std::string s_KeyAiStates("AiStates");
const std::string s_KeyAngle("Angle");
const std::string s_KeyChatParameters("ChatParameters");
const std::string s_KeyChildren("Children");
const std::string s_KeyComment("Comment");
const std::string s_KeyDescription("Description");
const std::string s_KeyEntryPoint("EntryPoint");
const std::string s_KeyEvents("Events");
const std::string s_KeyGrp("Grp");
const std::string s_KeyId("Id");
const std::string s_KeyIsland("Island");
const std::string s_KeyKeywords("Keywords");
const std::string s_KeyLocationId("LocationId");
const std::string s_KeyLocations("Locations");
const std::string s_KeyName("Name");
const std::string s_KeyNpcs("Npcs");
const std::string s_KeyParameters("Parameters");
const std::string s_KeyPlotItems("PlotItems");
const std::string s_KeyPreActDescription("PreActDescription");
const std::string s_KeyPt("Pt");
const std::string s_KeyPts("Pts");
const std::string s_KeySeason("Season");
const std::string s_KeySheet("Sheet");
const std::string s_KeySheetClient("SheetClient");
const std::string s_KeySheetId("SheetId");
const std::string s_KeyText("Text");
const std::string s_KeyTexts("Texts");
const std::string s_KeyUserTriggers("UserTriggers");
const std::string s_KeyWeatherValue("WeatherValue");
const std::string s_KeyWeight("Weight");
const std::string s_KeyX("x");
const std::string s_KeyY("y");
const std::string s_KeyZ("z");

/// A text of the tree for the description of a refusal: shortened, and without control characters
std::string quoteText(const std::string &text)
{
	std::string out;
	uint32 size = (uint32)std::min((size_t)s_MaxQuotedTextSize, text.size());
	out.reserve(size + 5);
	out += '\'';
	for (uint32 i = 0; i < size; ++i)
	{
		char c = text[i];
		out += ((uint8)c < 0x20 || (uint8)c == 0x7f) ? '?' : c;
	}
	out += '\'';
	if (text.size() > size)
		out += "...";
	return out;
}

const CObject *getAttrPtr(const CObject *table, const std::string &name)
{
	return table->getAttr(name).getPtr();
}

bool isTable(const CObject *object)
{
	return object && object->isTable(s_Empty);
}

/// The text of a string or of a ref id of the tree, without a copy; NULL for anything else
const std::string *getText(const CObject *object)
{
	const CObjectString *text = dynamic_cast<const CObjectString *>(object);
	return text ? &text->getValue() : nullptr;
}

/// A number of the tree as a double. Nothing is converted to an integer type here, so a number of
/// any size, an infinity or a NaN can be read and compared.
bool getNumber(const CObject *object, double &value)
{
	const CObjectNumber *number = dynamic_cast<const CObjectNumber *>(object);
	if (!number)
		return false;
	value = number->getNumberValue();
	return true;
}

/// A number of the tree that is neither an infinity nor a NaN and has no fractional part
bool getWholeNumber(const CObject *object, double &value)
{
	if (!getNumber(object, value))
		return false;
	return isValidDouble(value) && value == floor(value);
}

std::string describeNumber(double value)
{
	if (isValidDouble(value) && value == floor(value) && fabs(value) < 1e15)
		return toString("%.0f", value);
	return toString("%g", value);
}

/// What a value of the tree is, for the description of a refusal
std::string describeValue(const CObject *object)
{
	if (!object)
		return "missing";
	double number;
	if (getNumber(object, number))
		return "the number " + describeNumber(number);
	if (object->isTable(s_Empty))
		return toString("a table of %u values", object->getSize());
	const std::string *text = getText(object);
	if (text)
		return "the text " + quoteText(*text);
	return "a value of an unknown type";
}

struct CWalkFrame
{
	const CObject *Table;
	uint32 Next;
	uint32 Size;
	/// False below a table whose id was already taken: the instance map of the animation module
	/// leaves such a table and what it holds out
	bool Register;
};

/// A table of the tree that has an id, and what was found about it so far
struct CIdEntry
{
	const CObject *Table;
	/// A state names it as one of its groups
	bool NamedAsGroup;
	/// A group names it as one of its npcs
	bool NamedAsNpc;
	/// An event names it as one of its actions, or it is an action below another one
	bool NamedAsAction;

	CIdEntry(const CObject *table)
	    : Table(table)
	    , NamedAsGroup(false)
	    , NamedAsNpc(false)
	    , NamedAsAction(false)
	{
	}
};

/// An order of the ids: by their size, then by their bytes. Any order does, as long as it is one.
struct CIdLess
{
	bool operator()(const std::string *a, const std::string *b) const
	{
		if (a->size() != b->size())
			return a->size() < b->size();
		return memcmp(a->data(), b->data(), a->size()) < 0;
	}
};

/// The tables of the tree by their id. The ids are not copied: they are texts of the tree, which
/// does not change while it is checked. A tree can have half a million of them, and whoever sends
/// the tree chooses them: they are kept in order, so that finding one takes the same number of
/// steps whatever the ids are.
typedef std::map<const std::string *, CIdEntry, CIdLess> TIds;

struct CActCounts
{
	/// Size of the Npcs table: what the service has always counted
	uint32 Npcs;
	/// Bots the translation makes: one for each npc of a group of a state
	uint32 Bots;
	/// Groups the states name
	uint32 Groups;
	/// States, groups, bots and events: each takes an alias in the primitive file of the act
	uint32 Aliases;
	/// Actions the events start, with the actions below them
	uint32 Actions;

	CActCounts()
	    : Npcs(0)
	    , Bots(0)
	    , Groups(0)
	    , Aliases(0)
	    , Actions(0)
	{
	}
};

/**
 * Every table is looked at once: a group, an npc and an action can be named once. An attribute is
 * found by going through its table, so the lookups of one table cost its size a few times, and those
 * of the whole check the size of the tree a few times.
 *
 * A refusal is described where it is found, and the functions it returns through put the place in
 * front of it: nothing is formatted for a tree that passes.
 */

class CRtScenarioChecker
{
public:
	CRtScenarioChecker(const CRtScenarioLimits &limits)
	    : m_Limits(limits)
	    , m_Verdict(RtScenarioOk)
	    , m_Located(false)
	    , m_ActionNamed(false)
	    , m_LocationCount(0)
	    , m_PrimNodes(0)
	    , m_ActIndex(0)
	{
	}

	TRtScenarioVerdict run(const CObject *root);

	const std::string &getError() const { return m_Error; }

private:
	bool walkTree(const CObject *root);
	bool registerId(const CObject *table);
	CIdEntry *findId(const std::string &id);

	bool checkPlotItems(const CObject *root);
	bool checkPlotItem(const CObject *plotItem);
	bool checkLocations(const CObject *root);
	bool checkLocation(const CObject *location);
	bool checkTexts(const CObject *root);
	bool checkActs(const CObject *root);
	bool checkAct(const CObject *act, CActCounts &counts);
	bool checkAiState(const CObject *state, CActCounts &counts);
	bool checkGroup(const CObject *group, CActCounts &counts);
	bool checkBot(const CObject *bot);
	bool checkEvent(const CObject *event, CActCounts &counts);
	bool checkAction(const CObject *action, const std::string *id, CIdEntry *entry, uint32 &count);
	bool checkActionContent(const CObject *action, uint32 &count);
	bool checkUserTriggers(const CObject *act);
	bool checkPoint(const CObject *point);

	bool requireTable(const CObject *table, const std::string &name, const CObject *&value);
	bool requireText(const CObject *table, const std::string &name, const std::string *&text);
	bool requireText(const CObject *table, const std::string &name);
	bool requireWholeNumber(const CObject *table, const std::string &name, double minValue, double maxValue, double &value);
	bool checkRangeIfNumber(const CObject *table, const std::string &name, double minValue, double maxValue);

	/// Counts the nodes of the primitives the translation makes. The size of the tree does not bound
	/// them exactly: an event that holds nothing is one value, and two nodes.
	bool addPrimNodes(uint32 count);

	/// Records the refusal, with what is wrong (" is ...", or ".Attribute is ..."), and returns false
	bool refuse(TRtScenarioVerdict verdict, const std::string &what);
	bool invalid(const std::string &what) { return refuse(RtScenarioInvalid, what); }
	/// Records a refusal that is about the act that is being checked as a whole
	bool refuseAct(TRtScenarioVerdict verdict, const std::string &what);
	/// Puts a part of the place in front of the refusal, and returns false
	bool prepend(const std::string &text);
	/// Puts the place in front of the refusal, unless a function nearer to it did, and returns false
	bool locate(const std::string &where);

	const CRtScenarioLimits &m_Limits;
	TRtScenarioVerdict m_Verdict;
	std::string m_Error;
	bool m_Located;
	bool m_ActionNamed;
	/// The tables of the tree by their Id, as CInstanceMap("Id") finds them
	TIds m_Ids;
	uint32 m_LocationCount;
	uint32 m_PrimNodes;
	uint32 m_ActIndex;
};

// ****************************************************************************
bool CRtScenarioChecker::refuse(TRtScenarioVerdict verdict, const std::string &what)
{
	if (m_Verdict == RtScenarioOk)
	{
		m_Verdict = verdict;
		m_Error = what;
	}
	return false;
}

// ****************************************************************************
bool CRtScenarioChecker::refuseAct(TRtScenarioVerdict verdict, const std::string &what)
{
	if (m_Verdict == RtScenarioOk)
	{
		m_Verdict = verdict;
		m_Error = toString("Acts[%u]", m_ActIndex) + what;
		m_Located = true;
	}
	return false;
}

// ****************************************************************************
bool CRtScenarioChecker::prepend(const std::string &text)
{
	if (!m_Located)
		m_Error = text + m_Error;
	return false;
}

// ****************************************************************************
bool CRtScenarioChecker::locate(const std::string &where)
{
	if (!m_Located)
	{
		m_Error = where + m_Error;
		m_Located = true;
	}
	return false;
}

// ****************************************************************************
bool CRtScenarioChecker::addPrimNodes(uint32 count)
{
	if (count > m_Limits.MaxNodes - std::min(m_PrimNodes, m_Limits.MaxNodes))
		return invalid(toString(" makes the translated scenario larger than %u nodes", m_Limits.MaxNodes));
	m_PrimNodes += count;
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::registerId(const CObject *table)
{
	const std::string *id = getText(getAttrPtr(table, s_KeyId));
	if (!id)
		return true;
	return m_Ids.insert(std::make_pair(id, CIdEntry(table))).second;
}

// ****************************************************************************
CIdEntry *CRtScenarioChecker::findId(const std::string &id)
{
	TIds::iterator found = m_Ids.find(&id);
	return found != m_Ids.end() ? &found->second : nullptr;
}

// ****************************************************************************
// Visits every value once with an explicit stack: the depth and the number of values are checked
// before any function that recurses over the tree runs on it, and the tables are recorded by Id.
bool CRtScenarioChecker::walkTree(const CObject *root)
{
	// The frames of the open tables. They are reached through a pointer, and the array only grows
	// when a table is opened below the deepest one so far.
	std::vector<CWalkFrame> stack(64);
	CWalkFrame *frames = &stack[0];
	uint32 depth = 1;
	uint32 values = 1;

	frames[0].Table = root;
	frames[0].Next = 0;
	frames[0].Size = root->getSize();
	frames[0].Register = registerId(root);

	while (depth != 0)
	{
		CWalkFrame &top = frames[depth - 1];
		if (top.Next == top.Size)
		{
			--depth;
			continue;
		}

		const CObject *child = top.Table->getValueAtPos(top.Next).getPtr();
		++top.Next;
		if (!child)
			return invalid(" has a table that holds an empty value");

		++values;
		if (values > m_Limits.MaxNodes)
			return invalid(toString(" has more than %u values", m_Limits.MaxNodes));

		if (child->isTable(s_Empty))
		{
			if (depth >= m_Limits.MaxDepth)
				return invalid(toString(" has tables nested deeper than %u", m_Limits.MaxDepth));

			// a table that holds nothing has no id, and nothing to visit
			uint32 size = child->getSize();
			if (size != 0)
			{
				bool registered = top.Register && registerId(child);
				if (depth == stack.size())
				{
					stack.resize(stack.size() * 2);
					frames = &stack[0];
				}
				// 'top' is not used after this: the array can have moved
				CWalkFrame &frame = frames[depth];
				frame.Table = child;
				frame.Next = 0;
				frame.Size = size;
				frame.Register = registered;
				++depth;
			}
		}
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::requireTable(const CObject *table, const std::string &name, const CObject *&value)
{
	value = getAttrPtr(table, name);
	if (!isTable(value))
		return invalid("." + name + " is " + describeValue(value) + ", a table is needed");
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::requireText(const CObject *table, const std::string &name, const std::string *&text)
{
	const CObject *value = getAttrPtr(table, name);
	text = getText(value);
	if (!text)
		return invalid("." + name + " is " + describeValue(value) + ", a text is needed");
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::requireText(const CObject *table, const std::string &name)
{
	const std::string *text;
	return requireText(table, name, text);
}

// ****************************************************************************
bool CRtScenarioChecker::requireWholeNumber(const CObject *table, const std::string &name, double minValue, double maxValue, double &value)
{
	const CObject *object = getAttrPtr(table, name);
	if (!getWholeNumber(object, value))
		return invalid("." + name + " is " + describeValue(object) + ", a whole number is needed");
	if (value < minValue || value > maxValue)
		return invalid("." + name + " is " + describeNumber(value) + ", outside " + describeNumber(minValue) + " to " + describeNumber(maxValue));
	return true;
}

// ****************************************************************************
// A value that is only read when it is a number: then it must be one in the range. The test is
// written so that a NaN fails it.
bool CRtScenarioChecker::checkRangeIfNumber(const CObject *table, const std::string &name, double minValue, double maxValue)
{
	double value;
	if (getNumber(getAttrPtr(table, name), value) && !(value >= minValue && value <= maxValue))
		return invalid("." + name + " is " + describeNumber(value) + ", outside " + describeNumber(minValue) + " to " + describeNumber(maxValue));
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkPoint(const CObject *point)
{
	// the coordinates are read from a table; anything else is left alone by the translation
	if (!isTable(point))
		return true;
	return checkRangeIfNumber(point, s_KeyX, m_Limits.MinX, m_Limits.MaxX)
	    && checkRangeIfNumber(point, s_KeyY, m_Limits.MinY, m_Limits.MaxY)
	    && checkRangeIfNumber(point, s_KeyZ, -RtScenarioMaxCoordinate, RtScenarioMaxCoordinate);
}

// ****************************************************************************
bool CRtScenarioChecker::checkPlotItem(const CObject *plotItem)
{
	if (!isTable(plotItem))
		return invalid(" is " + describeValue(plotItem) + ", a table is needed");

	// whether the sheet is one of the plot item sheets is for the service to say: it has the sheets
	double sheetId;
	if (!requireWholeNumber(plotItem, s_KeySheetId, 0.0, s_MaxSheetId, sheetId))
		return false;

	const std::string *const names[] = { &s_KeyName, &s_KeyDescription, &s_KeyComment };
	for (uint32 t = 0; t < sizeof(names) / sizeof(names[0]); ++t)
	{
		const std::string *text;
		if (!requireText(plotItem, *names[t], text))
			return false;
		if (text->size() > m_Limits.MaxPlotItemTextSize)
			return invalid("." + *names[t] + toString(" has %u bytes, the limit is %u", (uint32)text->size(), m_Limits.MaxPlotItemTextSize));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkPlotItems(const CObject *root)
{
	const CObject *plotItems;
	if (!requireTable(root, s_KeyPlotItems, plotItems))
		return locate("the scenario");

	uint32 count = plotItems->getSize();
	if (count > m_Limits.MaxPlotItems)
	{
		invalid(toString(" has %u plot items, the limit is %u", count, m_Limits.MaxPlotItems));
		return locate("PlotItems");
	}

	for (uint32 i = 0; i < count; ++i)
	{
		if (!checkPlotItem(plotItems->getValueAtPos(i).getPtr()))
			return locate(toString("PlotItems[%u]", i));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkLocation(const CObject *location)
{
	if (!isTable(location))
		return invalid(" is " + describeValue(location) + ", a table is needed");

	double season;
	return requireWholeNumber(location, s_KeySeason, 0.0, (double)RtScenarioMaxSeason, season)
	    && requireText(location, s_KeyIsland)
	    && requireText(location, s_KeyEntryPoint);
}

// ****************************************************************************
bool CRtScenarioChecker::checkLocations(const CObject *root)
{
	const CObject *locations;
	if (!requireTable(root, s_KeyLocations, locations))
		return locate("the scenario");

	m_LocationCount = locations->getSize();
	for (uint32 i = 0; i < m_LocationCount; ++i)
	{
		if (!checkLocation(locations->getValueAtPos(i).getPtr()))
			return locate(toString("Locations[%u]", i));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkTexts(const CObject *root)
{
	// a scenario without a text manager has no texts
	const CObject *textManager = getAttrPtr(root, s_KeyTexts);
	if (!textManager)
		return true;
	if (!textManager->isTable(s_Empty))
	{
		invalid(" is " + describeValue(textManager) + ", a table is needed");
		return locate("Texts");
	}

	const CObject *texts;
	if (!requireTable(textManager, s_KeyTexts, texts))
		return locate("Texts");

	uint32 count = texts->getSize();
	if (count > m_Limits.MaxTexts)
	{
		refuse(RtScenarioOverQuota, toString(" has %u texts, the limit is %u", count, m_Limits.MaxTexts));
		return locate("Texts.Texts");
	}

	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *entry = texts->getValueAtPos(i).getPtr();
		if (!isTable(entry))
			invalid(" is " + describeValue(entry) + ", a table is needed");
		else if (requireText(entry, s_KeyId) && requireText(entry, s_KeyText))
			continue;
		return locate(toString("Texts.Texts[%u]", i));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkBot(const CObject *bot)
{
	const std::string *const names[] = { &s_KeyName, &s_KeyChatParameters, &s_KeyKeywords, &s_KeySheetClient, &s_KeySheet };
	for (uint32 t = 0; t < sizeof(names) / sizeof(names[0]); ++t)
	{
		if (!requireText(bot, *names[t]))
			return false;
	}

	if (!checkPoint(getAttrPtr(bot, s_KeyPt)))
		return prepend(".Pt");
	return checkRangeIfNumber(bot, s_KeyAngle, -RtScenarioMaxAngle, RtScenarioMaxAngle);
}

// ****************************************************************************
bool CRtScenarioChecker::checkGroup(const CObject *group, CActCounts &counts)
{
	const CObject *children;
	if (!requireTable(group, s_KeyChildren, children))
		return false;

	// the group and its parameters
	if (!addPrimNodes(2))
		return false;
	++counts.Aliases;

	uint32 count = children->getSize();
	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *child = children->getValueAtPos(i).getPtr();
		const std::string *botId = getText(child);
		if (!botId)
			return invalid(toString(".Children[%u] is ", i) + describeValue(child) + ", the id of an npc is needed");

		// the count stops at the first bot over the limit, however long the list is
		++counts.Bots;
		++counts.Aliases;
		if (counts.Bots > m_Limits.MaxBotsPerAct)
			return refuseAct(RtScenarioOverQuota, toString(" has more than %u bots", m_Limits.MaxBotsPerAct));
		if (!addPrimNodes(1))
			return false;

		// An npc belongs to one group. The editor never names one twice, and the translation would
		// make a bot of it each time.
		CIdEntry *bot = findId(*botId);
		if (!bot)
			invalid(" is not in the scenario");
		else if (bot->NamedAsNpc)
			invalid(" is named a second time, an npc belongs to one group");
		else
		{
			bot->NamedAsNpc = true;
			if (checkBot(bot->Table))
				continue;
		}
		return prepend(": the npc " + quoteText(*botId));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkAiState(const CObject *state, CActCounts &counts)
{
	if (!isTable(state))
		return invalid(" is " + describeValue(state) + ", a table is needed");

	if (!requireText(state, s_KeyAiMovement))
		return false;

	const CObject *points = getAttrPtr(state, s_KeyPts);
	if (points)
	{
		if (!points->isTable(s_Empty))
			return invalid(".Pts is " + describeValue(points) + ", a table is needed");
		uint32 pointCount = points->getSize();
		for (uint32 i = 0; i < pointCount; ++i)
		{
			if (!checkPoint(points->getValueAtPos(i).getPtr()))
				return prepend(toString(".Pts[%u]", i));
		}
	}

	const CObject *children;
	if (!requireTable(state, s_KeyChildren, children))
		return false;

	if (!addPrimNodes(1))
		return false;
	++counts.Aliases;

	uint32 count = children->getSize();
	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *child = children->getValueAtPos(i).getPtr();
		const std::string *groupId = getText(child);
		if (!groupId)
			return invalid(toString(".Children[%u] is ", i) + describeValue(child) + ", the id of a group is needed");

		// A group belongs to one state. The editor never names one twice, and the translation would
		// make the group and its bots each time.
		CIdEntry *group = findId(*groupId);
		if (!group)
			invalid(" is not in the scenario");
		else if (group->NamedAsGroup)
			invalid(" is named a second time, a group belongs to one state");
		else
		{
			group->NamedAsGroup = true;
			++counts.Groups;
			if (counts.Groups > m_Limits.MaxGroupsPerAct)
				return refuseAct(RtScenarioOverQuota, toString(" has more than %u groups", m_Limits.MaxGroupsPerAct));
			if (checkGroup(group->Table, counts))
				continue;
		}
		return prepend(": the group " + quoteText(*groupId));
	}
	return true;
}

// ****************************************************************************
// An action and the actions below it; count receives how many they are. The actions are nested
// tables, so the recursion is as deep as the tree, which walkTree() has checked.
//
// An action belongs to one event, or to one action above it. The editor never names one twice, and
// the translation would make it and what is below it each time: an action that has an id, and so
// can be named, is refused the second time it is reached, before anything is read from it. id and
// entry are the id an event names the action by and what the id gave; they are NULL for an action
// that is below another one, whose own id is looked up here.
bool CRtScenarioChecker::checkAction(const CObject *action, const std::string *id, CIdEntry *entry, uint32 &count)
{
	bool ok = true;
	if (!entry)
	{
		if (!isTable(action))
			return invalid(": an action is " + describeValue(action) + ", a table is needed");
		id = getText(getAttrPtr(action, s_KeyId));
		if (id)
		{
			// the table that has the id, which another table of the same id is not
			entry = findId(*id);
			if (entry && entry->Table != action)
				entry = nullptr;
		}
	}

	if (entry && entry->NamedAsAction)
		ok = invalid(" is named a second time, an action belongs to one event or to one action above it");
	else
	{
		if (entry)
			entry->NamedAsAction = true;
		ok = checkActionContent(action, count);
	}

	// the action the refusal is about is named, not the ones it is below
	if (!ok && !m_ActionNamed)
	{
		m_ActionNamed = true;
		prepend(id ? ": the action " + quoteText(*id) : std::string(": an action"));
	}
	return ok;
}

// ****************************************************************************
bool CRtScenarioChecker::checkActionContent(const CObject *action, uint32 &count)
{
	const std::string *name;
	if (!requireText(action, s_KeyAction, name))
		return false;

	// these actions read their parameters without a test of the type
	if (*name == "modify_variable" || *name == "condition_if" || *name == "condition_if_else" || *name == "dynamic_if")
	{
		if (!requireText(action, s_KeyParameters))
			return false;
	}

	if (getAttrPtr(action, s_KeyWeight))
	{
		double weight;
		if (!requireWholeNumber(action, s_KeyWeight, 0.0, s_MaxActionWeight, weight))
			return false;
	}

	const CObject *children;
	if (!requireTable(action, s_KeyChildren, children))
		return false;

	count = 1;
	uint32 childCount = children->getSize();
	for (uint32 i = 0; i < childCount; ++i)
	{
		uint32 below = 0;
		if (!checkAction(children->getValueAtPos(i).getPtr(), nullptr, nullptr, below))
			return false;
		count += below;
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkEvent(const CObject *event, CActCounts &counts)
{
	if (!isTable(event))
		return invalid(" is " + describeValue(event) + ", a table is needed");

	// the event, and the action that holds the others when there are several
	if (!addPrimNodes(2))
		return false;
	++counts.Aliases;

	const CObject *actionIds = getAttrPtr(event, s_KeyActionsId);
	if (!actionIds)
		return true;
	if (!actionIds->isTable(s_Empty))
		return invalid(".ActionsId is " + describeValue(actionIds) + ", a table is needed");

	uint32 count = actionIds->getSize();
	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *value = actionIds->getValueAtPos(i).getPtr();
		const std::string *actionId = getText(value);
		if (!actionId)
			return invalid(toString(".ActionsId[%u] is ", i) + describeValue(value) + ", the id of an action is needed");

		CIdEntry *action = findId(*actionId);
		if (!action)
			return invalid(": the action " + quoteText(*actionId) + " is not in the scenario");

		uint32 actions = 0;
		if (!checkAction(action->Table, actionId, action, actions))
			return false;

		if (actions > m_Limits.MaxActionsPerAct - std::min(counts.Actions, m_Limits.MaxActionsPerAct))
			return refuseAct(RtScenarioOverQuota, toString(" has more than %u actions", m_Limits.MaxActionsPerAct));
		counts.Actions += actions;
		if (!addPrimNodes(actions))
			return false;
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkUserTriggers(const CObject *act)
{
	const CObject *userTriggers;
	if (!requireTable(act, s_KeyUserTriggers, userTriggers))
		return false;

	uint32 count = userTriggers->getSize();
	if (count > m_Limits.MaxUserTriggersPerAct)
		return refuseAct(RtScenarioOverQuota, toString(" has %u user triggers, the limit is %u", count, m_Limits.MaxUserTriggersPerAct));

	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *userTrigger = userTriggers->getValueAtPos(i).getPtr();
		if (!isTable(userTrigger))
			invalid(" is " + describeValue(userTrigger) + ", a table is needed");
		else if (requireText(userTrigger, s_KeyName) && requireText(userTrigger, s_KeyGrp))
			continue;
		return locate(toString("Acts[%u].UserTriggers[%u]", m_ActIndex, i));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkAct(const CObject *act, CActCounts &counts)
{
	if (!isTable(act))
		return invalid(" is " + describeValue(act) + ", a table is needed");

	// The location is an index into Locations. The base act has no location of its own and the
	// editor leaves its index at 0, which is read like any other when the weather and the season of
	// an act are looked up: it must exist too.
	double locationId;
	const CObject *locationObject = getAttrPtr(act, s_KeyLocationId);
	if (!getWholeNumber(locationObject, locationId))
		return invalid(".LocationId is " + describeValue(locationObject) + ", a whole number is needed");
	if (locationId < 0.0 || locationId >= (double)m_LocationCount)
		return invalid(".LocationId is " + describeNumber(locationId) + toString(", the scenario has %u locations", m_LocationCount));

	if (!requireText(act, s_KeyName)
	    || !requireText(act, s_KeyActDescription)
	    || !requireText(act, s_KeyPreActDescription))
		return false;

	// the weather is read as the whole part of a number
	const CObject *weatherObject = getAttrPtr(act, s_KeyWeatherValue);
	if (weatherObject)
	{
		double weather;
		if (!getNumber(weatherObject, weather) || !isValidDouble(weather))
			return invalid(".WeatherValue is " + describeValue(weatherObject) + ", a number is needed");
		if (weather < 0.0 || weather >= (double)RtScenarioMaxWeatherValue + 1.0)
			return invalid(".WeatherValue is " + describeNumber(weather) + toString(", outside 0 to %u", RtScenarioMaxWeatherValue));
	}

	const CObject *npcs;
	if (!requireTable(act, s_KeyNpcs, npcs))
		return false;
	counts.Npcs = npcs->getSize();

	// the two managers of an act take an alias each
	counts.Aliases = 2;

	const CObject *states;
	if (!requireTable(act, s_KeyAiStates, states))
		return false;
	uint32 stateCount = states->getSize();
	if (stateCount > m_Limits.MaxStatesPerAct)
		return refuseAct(RtScenarioOverQuota, toString(" has %u states, the limit is %u", stateCount, m_Limits.MaxStatesPerAct));
	for (uint32 i = 0; i < stateCount; ++i)
	{
		if (!checkAiState(states->getValueAtPos(i).getPtr(), counts))
			return locate(toString("Acts[%u].AiStates[%u]", m_ActIndex, i));
	}

	const CObject *events = getAttrPtr(act, s_KeyEvents);
	if (events)
	{
		if (!events->isTable(s_Empty))
			return invalid(".Events is " + describeValue(events) + ", a table is needed");
		uint32 eventCount = events->getSize();
		if (eventCount > m_Limits.MaxEventsPerAct)
			return refuseAct(RtScenarioOverQuota, toString(" has %u events, the limit is %u", eventCount, m_Limits.MaxEventsPerAct));
		for (uint32 i = 0; i < eventCount; ++i)
		{
			if (!checkEvent(events->getValueAtPos(i).getPtr(), counts))
				return locate(toString("Acts[%u].Events[%u]", m_ActIndex, i));
		}
	}

	if (counts.Aliases > m_Limits.MaxAliasesPerAct)
		return invalid(toString(" needs %u aliases, the primitive file of an act has %u", counts.Aliases, m_Limits.MaxAliasesPerAct));

	return checkUserTriggers(act);
}

// ****************************************************************************
bool CRtScenarioChecker::checkActs(const CObject *root)
{
	const CObject *acts;
	if (!requireTable(root, s_KeyActs, acts))
		return locate("the scenario");

	uint32 count = acts->getSize();
	if (count == 0)
	{
		invalid(" is empty, a scenario has at least its base act");
		return locate("Acts");
	}
	if (count > m_Limits.MaxActs)
	{
		invalid(toString(" has %u acts, the limit is %u", count, m_Limits.MaxActs));
		return locate("Acts");
	}

	// The base act (index 0) stays loaded while one of the other acts runs: the limit is for the
	// content of the two together.
	CActCounts base;
	for (m_ActIndex = 0; m_ActIndex < count; ++m_ActIndex)
	{
		CActCounts counts;
		if (!checkAct(acts->getValueAtPos(m_ActIndex).getPtr(), counts))
			return locate(toString("Acts[%u]", m_ActIndex));

		if (m_ActIndex == 0)
			base = counts;

		uint32 npcs = counts.Npcs + (m_ActIndex != 0 ? base.Npcs : 0);
		if (npcs > m_Limits.MaxBotsPerAct)
			return refuseAct(RtScenarioOverQuota, toString(" has %u npcs with those of the base act, the limit is %u", npcs, m_Limits.MaxBotsPerAct));

		uint32 bots = counts.Bots + (m_ActIndex != 0 ? base.Bots : 0);
		if (bots > m_Limits.MaxBotsPerAct)
			return refuseAct(RtScenarioOverQuota, toString(" has %u bots with those of the base act, the limit is %u", bots, m_Limits.MaxBotsPerAct));
	}
	return true;
}

// ****************************************************************************
TRtScenarioVerdict CRtScenarioChecker::run(const CObject *root)
{
	if (!root)
	{
		invalid("the upload holds no run time scenario (it is empty, or its compressed data could not be read)");
		return m_Verdict;
	}
	if (!root->isTable(s_Empty))
	{
		invalid("the scenario is " + describeValue(root) + ", a table is needed");
		return m_Verdict;
	}

	if (!walkTree(root))
	{
		locate("the scenario");
		return m_Verdict;
	}

	// the locations come before the acts, which index them
	if (checkPlotItems(root)
	    && checkLocations(root)
	    && checkTexts(root)
	    && checkActs(root))
	{
		return RtScenarioOk;
	}
	return m_Verdict;
}

} // namespace

// ****************************************************************************
TRtScenarioVerdict checkRtScenario(const CObject *rtScenario, const CRtScenarioLimits &limits, std::string &error)
{
	CRtScenarioChecker checker(limits);
	TRtScenarioVerdict verdict = checker.run(rtScenario);
	error = checker.getError();
	return verdict;
}

// ****************************************************************************
TRtScenarioVerdict checkRtScenarioStartingAct(const CObject *rtScenario, uint32 startingAct, std::string &error)
{
	error.clear();
	const CObject *acts = isTable(rtScenario) ? getAttrPtr(rtScenario, s_KeyActs) : nullptr;
	if (!isTable(acts))
	{
		error = "the scenario has no table of acts";
		return RtScenarioInvalid;
	}

	uint32 count = acts->getSize();
	if (startingAct == 0 || startingAct >= count)
	{
		error = toString("the starting act is %u, the scenario has %u acts with its base act, which is never started", startingAct, count);
		return RtScenarioInvalid;
	}
	return RtScenarioOk;
}

} // namespace R2
