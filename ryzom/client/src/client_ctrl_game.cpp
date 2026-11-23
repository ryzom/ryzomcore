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

// The game methods of the control endpoint: the in-game browser (web.list, web.page, web.browse,
// web.back), the inventories and the journal (inventory.list, missions.list), and the Ring scenario
// editor (ring.state, ring.tree, ring.set, ring.create, ring.erase, ring.move, ring.undo, ring.redo,
// ring.select). The Ring methods go through the editor's own requests, so they reach the server and
// the co-editors as the editor's buttons do.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/gui/action_handler.h"
#include "nel/gui/group_html.h"
#include "nel/gui/reflect.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/cdb_branch.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/common.h"
#include "nel/misc/sheet_id.h"

#include "game_share/slot_equipment.h"
#include "nel/gui/db_manager.h"
#include "r2/editor.h"
#include "r2/tool_draw_prim.h"
#include "sheet_manager.h"
#include "client_sheets/item_sheet.h"
#include "string_manager_client.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace NLGUI;

namespace ClientCtrl {

namespace {

// the bound of the world coordinates the methods take: a value beyond it is a mistake of the caller
const double s_MaxCoordinate = 1e6;
// the links web.page gives at most
const uint s_MaxLinks = 1000;

/// Reads a number of the arguments into value: absent or null leaves value as it is; false with the call failed when
/// it is not a number, not finite, or beyond limit either way
bool readNumber(CCtrlCall &call, const char *name, double limit, double &value)
{
	const CJsonValue &arg = call.getArgs().get(name);
	if (arg.isNull())
		return true;
	if (!arg.isNumber() || !isValidDouble(arg.getNumber()) || fabs(arg.getNumber()) > limit)
	{
		call.fail(toString("%s is a number from %g to %g", name, -limit, limit));
		return false;
	}
	value = arg.getNumber();
	return true;
}

/// Whether every number in a value is finite: the editor sends what it is given to the server as it is
bool finiteNumbers(const CJsonValue &value)
{
	if (value.isNumber())
		return isValidDouble(value.getNumber()) != 0.0;
	for (uint i = 0; i < value.items().size(); ++i)
		if (!finiteNumbers(value.items()[i]))
			return false;
	for (uint i = 0; i < value.members().size(); ++i)
		if (!finiteNumbers(value.members()[i].second))
			return false;
	return true;
}

/// The place of the first null item of a list in a value, as path followed by the members and the indexes from 0 of the
/// JSON; false when there is none. A null member of an object stands for one not given; a null item of a list ends the
/// list in Lua, which loses what follows it.
bool findNullItem(const CJsonValue &value, const std::string &path, std::string &place)
{
	for (uint i = 0; i < value.items().size(); ++i)
	{
		std::string item = toString("%s[%u]", path.c_str(), i);
		if (value.items()[i].isNull())
		{
			place = item;
			return true;
		}
		if (findNullItem(value.items()[i], item, place))
			return true;
	}
	for (uint i = 0; i < value.members().size(); ++i)
		if (findNullItem(value.members()[i].second, path + "." + value.members()[i].first, place))
			return true;
	return false;
}

// *** the browser ***

void findHtml(CInterfaceGroup *group, std::vector<CGroupHTML *> &out)
{
	if (group == nullptr)
		return;
	if (CGroupHTML *html = dynamic_cast<CGroupHTML *>(group))
		out.push_back(html);
	const std::vector<CInterfaceGroup *> &groups = group->getGroups();
	for (uint i = 0; i < groups.size(); ++i)
		findHtml(groups[i], out);
}

void allHtml(std::vector<CGroupHTML *> &out, bool activeOnly)
{
	std::vector<CWidgetManager::SMasterGroup> &masters = CWidgetManager::getInstance()->getAllMasterGroup();
	for (uint m = 0; m < masters.size(); ++m)
	{
		if (masters[m].Group == nullptr)
			continue;
		std::vector<CGroupHTML *> found;
		findHtml(masters[m].Group, found);
		for (uint i = 0; i < found.size(); ++i)
			if (!activeOnly || found[i]->isActiveThroughParents())
				out.push_back(found[i]);
	}
}

/// The title of a browser view, as the interface reads it: getTitle is not public, the reflected property title is
std::string htmlTitle(CGroupHTML *html)
{
	// a missing property is warned about by getReflectedProperty
	const CReflectedProperty *prop = html->getReflectedProperty("title");
	if (prop == nullptr)
		return std::string();
	if (prop->Type != CReflectedProperty::String)
	{
		nlwarning("CTRL: the reflected property title of %s is not a string", html->getReflectedClassName());
		return std::string();
	}
	return (html->*(prop->GetMethod.GetString))();
}

CGroupHTML *findHtmlById(CCtrlCall &call)
{
	const std::string &id = call.getArgs().get("id").getString();
	CGroupHTML *html = dynamic_cast<CGroupHTML *>(CWidgetManager::getInstance()->getElementFromId(id));
	if (html == nullptr)
		call.fail(toString("'%s' is not a browser view; web.list lists them", id.c_str()));
	return html;
}

void methodWebList(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	std::vector<CGroupHTML *> views;
	allHtml(views, !call.getArgs().get("all").getBool(false));
	CJsonValue list = CJsonValue::array();
	for (uint i = 0; i < views.size(); ++i)
	{
		CGroupHTML *html = views[i];
		CJsonValue o = CJsonValue::object();
		o.set("id", html->getId());
		o.set("url", html->getURL());
		o.set("title", htmlTitle(html));
		o.set("loading", html->isBrowsing());
		o.set("shown", html->isActiveThroughParents());
		list.push(o);
	}
	call.reply(CJsonValue::object().set("views", list));
}

/// The text of HTML without its tags, scripts and styles, with the common entities, and its links (at most s_MaxLinks,
/// linksTruncated beyond)
void htmlText(const std::string &html, std::string &text, CJsonValue &links, bool &linksTruncated)
{
	// the tags are compared in lower case: the document is lowered once, the original gives the text and the addresses
	std::string lower = toLowerAscii(html);
	std::string::size_type i = 0;
	std::string linkHref;
	std::string linkText;
	bool inLink = false;
	while (i < html.size())
	{
		char c = html[i];
		if (c == '<')
		{
			std::string::size_type end = html.find('>', i);
			if (end == std::string::npos)
				break;
			std::string tag = lower.substr(i + 1, end - i - 1);
			bool closing = !tag.empty() && tag[0] == '/';
			std::string::size_type nameStart = closing ? 1 : 0;
			std::string::size_type nameEnd = tag.find_first_of(" \t\r\n/", nameStart);
			std::string name = tag.substr(nameStart, nameEnd == std::string::npos ? std::string::npos : nameEnd - nameStart);
			if (!closing && (name == "script" || name == "style"))
			{
				// to the end of the element; a document cut before it ends there
				std::string::size_type close = lower.find("</" + name, end);
				std::string::size_type closeEnd = close == std::string::npos ? std::string::npos : html.find('>', close);
				i = closeEnd == std::string::npos ? html.size() : closeEnd + 1;
				continue;
			}
			if (!closing && name == "a")
			{
				std::string::size_type h = tag.find("href=");
				if (h != std::string::npos && h + 5 < tag.size())
				{
					// the original case of the address, from the same place in the tag
					std::string rawTag = html.substr(i + 1, end - i - 1);
					char quote = rawTag[h + 5];
					std::string::size_type start = h + 5 + ((quote == '"' || quote == '\'') ? 1 : 0);
					std::string::size_type stop = (quote == '"' || quote == '\'') ? rawTag.find(quote, start) : rawTag.find_first_of(" >", start);
					linkHref = rawTag.substr(start, stop == std::string::npos ? std::string::npos : stop - start);
					linkText.clear();
					inLink = true;
				}
			}
			else if (closing && name == "a" && inLink)
			{
				if (links.size() < s_MaxLinks)
					links.push(CJsonValue::object().set("href", linkHref).set("text", trim(linkText)));
				else
					linksTruncated = true;
				inLink = false;
			}
			// a line break, and one at either end of a block
			if (name == "br" || name == "p" || name == "div" || name == "tr" || name == "li" || name == "h1" || name == "h2" || name == "h3")
				text += '\n';
			i = end + 1;
			continue;
		}
		std::string piece(1, c);
		if (c == '&')
		{
			std::string::size_type semi = html.find(';', i);
			if (semi != std::string::npos && semi - i < 8)
			{
				std::string entity = html.substr(i, semi - i + 1);
				piece = entity == "&amp;" ? "&" : entity == "&lt;" ? "<"
				    : entity == "&gt;"                             ? ">"
				    : entity == "&quot;"                           ? "\""
				    : entity == "&nbsp;"                           ? " "
				    : entity == "&#39;"                            ? "'"
				                                                   : entity;
				i = semi;
			}
		}
		text += piece;
		if (inLink)
			linkText += piece;
		++i;
	}
}

void methodWebPage(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	CGroupHTML *html = findHtmlById(call);
	if (html == nullptr)
		return;
	std::string source = html->getHTML();
	std::string format = call.getArgs().get("format").getString("text");
	if (format != "text" && format != "html")
	{
		call.fail("format is text or html");
		return;
	}
	size_t maxBytes = (size_t)std::max((sint64)256, std::min((sint64)4 * 1024 * 1024, call.getArgs().get("max_bytes").getInt(64 * 1024)));
	CJsonValue r = CJsonValue::object();
	r.set("id", html->getId());
	r.set("url", html->getURL());
	r.set("title", htmlTitle(html));
	r.set("loading", html->isBrowsing());
	std::string body;
	if (format == "html")
	{
		body = source;
	}
	else
	{
		CJsonValue links = CJsonValue::array();
		bool linksTruncated = false;
		htmlText(source, body, links, linksTruncated);
		if (linksTruncated)
			r.set("links_truncated", toString("the first %u links", s_MaxLinks));
		// the blank lines a layout leaves
		std::string compact;
		uint blank = 0;
		std::vector<std::string> lines;
		explode(body, std::string("\n"), lines, false);
		for (uint i = 0; i < lines.size(); ++i)
		{
			std::string line = trim(lines[i]);
			if (line.empty() && ++blank > 1)
				continue;
			if (!line.empty())
				blank = 0;
			compact += line + "\n";
		}
		body = compact;
		r.set("links", links);
	}
	r.set("bytes", (uint64)body.size());
	if (body.size() > maxBytes)
	{
		body.resize(maxBytes);
		r.set("truncated", toString("the first %u bytes: max_bytes gives more", (uint)maxBytes));
	}
	r.set(format == "html" ? "html" : "text", body);
	call.reply(r);
}

void methodWebBrowse(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	CGroupHTML *html = findHtmlById(call);
	if (html == nullptr)
		return;
	const std::string &url = call.getArgs().get("url").getString();
	if (url.empty())
	{
		call.fail("url is required");
		return;
	}
	html->browse(url.c_str());
	call.reply(CJsonValue::object().set("id", html->getId()).set("url", url).set("next", "web.list says when it is no longer loading"));
}

void methodWebBack(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	CGroupHTML *html = findHtmlById(call);
	if (html == nullptr)
		return;
	if (call.getArgs().get("refresh").getBool(false))
		html->refresh();
	else
		html->browseUndo();
	call.reply(CJsonValue::object().set("id", html->getId()));
}

// *** inventories and journal ***

CCDBNodeLeaf *leaf(const std::string &path)
{
	return CDBManager::getInstance()->getDbProp(path, false);
}

sint64 leafValue(const std::string &path, sint64 defaultValue = 0)
{
	CCDBNodeLeaf *l = leaf(path);
	return l != nullptr ? l->getValue64() : defaultValue;
}

std::string dynString(uint32 id, bool &arrived)
{
	std::string text;
	arrived = id == 0 || STRING_MANAGER::CStringManagerClient::instance()->getDynString(id, text);
	return text;
}

/// The item of a slot (its SHEET, QUANTITY... below base) into o; false for an empty slot
bool itemJson(const std::string &base, CJsonValue &o)
{
	uint32 sheet = (uint32)leafValue(base + ":SHEET");
	if (sheet == 0)
		return false;
	CSheetId id(sheet);
	o.set("sheet", id.toString());
	// a plot item of a Ring scenario is named by the scenario, as the item's slot shows it (CDBCtrlSheet::getItemActualName)
	const CItemSheet *itemSheet = dynamic_cast<const CItemSheet *>(SheetMngr.get(id));
	const R2::TMissionItem *plotItem = itemSheet && itemSheet->Family == ITEMFAMILY::SCROLL_R2 ? R2::getEditor().getPlotItemInfos(sheet) : nullptr;
	const char *localized = STRING_MANAGER::CStringManagerClient::getItemLocalizedName(id);
	if (plotItem)
		o.set("name", plotItem->Name.toUtf8());
	else if (localized != nullptr)
		o.set("name", localized);
	o.set("quantity", leafValue(base + ":QUANTITY"));
	o.set("quality", leafValue(base + ":QUALITY"));
	if (leafValue(base + ":LOCKED") != 0)
		o.set("locked", leafValue(base + ":LOCKED"));
	return true;
}

/// Whether a node of an inventory is an item slot: those are numbered, the other nodes (SESSION, the TM_ team members
/// of the share) are not
bool isSlotName(const std::string &name)
{
	if (name.empty())
		return false;
	for (uint i = 0; i < name.size(); ++i)
		if (name[i] < '0' || name[i] > '9')
			return false;
	return true;
}

void methodInventory(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	std::string name = toUpperAscii(call.getArgs().get("inventory").getString("bag"));
	static const char *const s_Known[] = { "BAG", "EQUIP", "HAND", "PACK_ANIMAL0", "PACK_ANIMAL1", "PACK_ANIMAL2", "PACK_ANIMAL3", "ROOM", "TEMP", "SHARE", "GUILD" };
	bool known = false;
	for (uint i = 0; i < sizeof(s_Known) / sizeof(s_Known[0]); ++i)
		known = known || name == s_Known[i];
	if (!known)
	{
		call.fail("inventory is bag, equip, hand, pack_animal0 to 3, room, temp, share or guild");
		return;
	}
	// the guild's is in the guild's bank, the others in the character's
	std::string path = name == "GUILD" ? std::string("SERVER:GUILD:INVENTORY") : "SERVER:INVENTORY:" + name;
	// the hands and the equipment hold where the item is in the bag, not the item
	bool inBag = name == "EQUIP" || name == "HAND";
	CCDBNodeBranch *branch = dynamic_cast<CCDBNodeBranch *>(CDBManager::getInstance()->getDB()->getNode(ICDBNode::CTextId(path), false));
	if (branch == nullptr)
	{
		call.fail("there is no " + path);
		return;
	}
	CJsonValue items = CJsonValue::array();
	uint slots = 0;
	for (uint i = 0; i < branch->getNbNodes(); ++i)
	{
		ICDBNode *slot = branch->getNode((uint16)i);
		if (slot == nullptr || slot->isLeaf() || !isSlotName(*slot->getName()))
			continue;
		++slots;
		std::string base = path + ":" + *slot->getName();
		CJsonValue o = CJsonValue::object();
		o.set("slot", *slot->getName());
		if (inBag)
		{
			// 0 for nothing, otherwise the bag slot plus one
			sint64 index = leafValue(base + ":INDEX_IN_BAG");
			if (index <= 0)
				continue;
			uint place = 0;
			fromString(*slot->getName(), place);
			if (name == "HAND")
				o.set("place", place == 0 ? "right" : "left");
			else if (place < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT)
				o.set("place", toLowerAscii(SLOT_EQUIPMENT::toString((SLOT_EQUIPMENT::TSlotEquipment)place)));
			o.set("bag_slot", index - 1);
			// the bag slot can be empty while the bag is on its way
			itemJson(toString("SERVER:INVENTORY:BAG:%u", (uint)(index - 1)), o);
		}
		else if (!itemJson(base, o))
		{
			continue;
		}
		items.push(o);
	}
	CJsonValue r = CJsonValue::object();
	r.set("inventory", toLowerAscii(name));
	r.set("slots", slots);
	r.set("items", items);
	r.set("database", path + (inBag ? ":<slot>:INDEX_IN_BAG, the bag slot plus one (0: nothing); ui.drag moves items between the slots of the windows" : ":<slot>; ui.drag moves items between the slots of the windows"));
	call.reply(r);
}

/// The missions of a journal (the character's SERVER:MISSIONS, the group's SERVER:GROUP:MISSIONS) into list, with the
/// journal index the CSR commands take: firstIndex and on
void listMissions(const char *journal, uint firstIndex, const char *journalName, CJsonValue &list, bool &allArrived)
{
	for (uint i = 0; i < 64; ++i)
	{
		std::string base = toString("%s:%u", journal, i);
		if (leaf(base + ":TITLE") == nullptr)
			break;
		uint32 titleId = (uint32)leafValue(base + ":TITLE");
		if (titleId == 0)
			continue;
		bool arrived;
		CJsonValue m = CJsonValue::object();
		m.set("index", firstIndex + i);
		m.set("journal", journalName);
		m.set("title", dynString(titleId, arrived));
		allArrived = allArrived && arrived;
		// 0 under way, 1 succeeded, 2 failed
		sint64 finished = leafValue(base + ":FINISHED");
		m.set("finished", finished);
		m.set("state", finished == 0 ? "under_way" : (finished == 1 ? "succeeded" : (finished == 2 ? "failed" : "unknown")));
		uint32 detailId = (uint32)leafValue(base + ":DETAIL_TEXT");
		if (detailId != 0)
		{
			m.set("detail", dynString(detailId, arrived));
			allArrived = allArrived && arrived;
		}
		CJsonValue goals = CJsonValue::array();
		for (uint g = 0; g < 32; ++g)
		{
			std::string goal = toString("%s:GOALS:%u", base.c_str(), g);
			if (leaf(goal + ":TEXT") == nullptr)
				break;
			uint32 textId = (uint32)leafValue(goal + ":TEXT");
			if (textId == 0)
				continue;
			CJsonValue o = CJsonValue::object();
			o.set("text", dynString(textId, arrived));
			allArrived = allArrived && arrived;
			if (leaf(goal + ":NPC_ALIAS") != nullptr)
				o.set("npc_alias", leafValue(goal + ":NPC_ALIAS"));
			goals.push(o);
		}
		m.set("goals", goals);
		CJsonValue targets = CJsonValue::array();
		for (uint t = 0; t < 8; ++t)
		{
			std::string target = toString("%s:TARGET%u", base.c_str(), t);
			if (leaf(target + ":TITLE") == nullptr)
				break;
			uint32 targetTitle = (uint32)leafValue(target + ":TITLE");
			if (targetTitle == 0)
				continue;
			CJsonValue o = CJsonValue::object();
			o.set("title", dynString(targetTitle, arrived));
			allArrived = allArrived && arrived;
			o.set("x", leafValue(target + ":X") / 1000.0);
			o.set("y", leafValue(target + ":Y") / 1000.0);
			targets.push(o);
		}
		if (targets.size() > 0)
			m.set("compass_targets", targets);
		list.push(m);
	}
}

void methodMissions(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	CJsonValue list = CJsonValue::array();
	bool allArrived = true;
	// the group's journal, the team's missions and then the guild's, comes after the character's 15 entries (commands.cpp
	// setMission, CAdminProperties::getMission)
	listMissions("SERVER:MISSIONS", 0, "character", list, allArrived);
	listMissions("SERVER:GROUP:MISSIONS", 15, "group", list, allArrived);
	CJsonValue r = CJsonValue::object();
	r.set("missions", list);
	r.set("note", "index is the journal index the CSR commands take (failMission, progressMission): 0 to 14 the character's, 15 and on the group's (the team's missions, then the guild's)");
	if (!allArrived)
		r.set("strings_arrived", false);
	call.reply(r);
}

// *** the Ring scenario editor ***

bool requireEditor(CCtrlCall &call)
{
	if (!R2::getEditor().isInitialized() || R2::getEditor().getMode() == R2::CEditor::NotInitialized)
	{
		call.fail("the scenario editor is not running");
		return false;
	}
	return true;
}

/// The scenario of the editor is there to read and change in edition mode only: a test or an animation holds the
/// translated scenario, and the edited one is erased until the editor comes back.
bool requireEdition(CCtrlCall &call)
{
	if (!requireEditor(call))
		return false;
	if (R2::getEditor().getMode() != R2::CEditor::EditionMode)
	{
		call.fail("the editor is not in edition mode: a test or an animation runs, and the scenario is read and changed in edition mode only (ring.state gives the mode and the act)");
		return false;
	}
	return true;
}

/// Runs a Lua chunk of the editor's methods and answers with its first result. The chunk bounds what it returns
/// itself (ring.tree has depth and max); the conversion only stops a runaway one, and says so.
void ringLua(CCtrlCall &call, const char *code, const CJsonValue &args)
{
	CJsonValue results;
	std::string error;
	bool truncated = false;
	if (!runLua(code, args, 40, 200000, results, error, &truncated))
	{
		call.fail(error);
		return;
	}
	CJsonValue r = results.at(0);
	if (truncated && r.isObject())
		r.set("conversion_truncated", "the result was larger than the conversion takes: narrow it");
	call.reply(r);
}

// the same, with a prelude of Lua definitions run before the code, in the same chunk
void ringLua(CCtrlCall &call, const char *prelude, const char *code, const CJsonValue &args)
{
	ringLua(call, (std::string(prelude) + code).c_str(), args);
}

// in a test or an animation the edited scenario is erased: the act is the index the server last gave (0 the
// permanent act), and the other fields are left out; in edition mode r2.getCurrentActIndex means nothing, and the
// index is the current act's place among the scenario's acts (classAct.getActId), as the translation numbers the
// acts (r2:getActId)
const char *const s_RingState = "local edition = ...\n"
                                "local r = {}\n"
                                "if not edition then\n"
                                "  r.current_act_index = r2.getCurrentActIndex()\n"
                                "  return r\n"
                                "end\n"
                                "r.can_undo = r2:canUndo()\n"
                                "r.can_redo = r2:canRedo()\n"
                                "r.selected = r2:getSelectedInstanceId()\n"
                                "if r2.Scenario then\n"
                                "  r.scenario = r2.Scenario.InstanceId\n"
                                "  local act = r2.Scenario:getCurrentAct()\n"
                                "  if act then r.current_act = act.InstanceId r.current_act_name = act.Name r.current_act_index = act:getActId() end\n"
                                "  r.acts = r2.Scenario.Acts.Size\n"
                                "end\n"
                                "return r\n";

const char *const s_RingTree = "local id, maxDepth, maxItems = ...\n"
                               "local root = r2.Scenario\n"
                               "if id and id ~= '' then root = r2:getInstanceFromId(id) end\n"
                               "if not root then error('no instance ' .. tostring(id)) end\n"
                               "local count = 0\n"
                               "local function walk(node, depth)\n"
                               "  count = count + 1\n"
                               "  if count > maxItems then return '<truncated>' end\n"
                               "  if not r2.isTable(node) then return node end\n"
                               "  if depth >= maxDepth then return { InstanceId = node.InstanceId, Class = node.Class, Name = node.Name, more = 'deeper than depth' } end\n"
                               "  local out = {}\n"
                               "  for k, v in specPairs(node) do\n"
                               "    if r2.isTable(v) then out[k] = walk(v, depth + 1) else out[k] = v end\n"
                               "  end\n"
                               "  return out\n"
                               "end\n"
                               "local tree = walk(root, 0)\n"
                               "return { tree = tree, items = count, truncated = count > maxItems }\n";

void methodRingState(CCtrlCall &call)
{
	if (!requireEditor(call))
		return;
	CJsonValue args = CJsonValue::array();
	args.push(CJsonValue(R2::getEditor().getMode() == R2::CEditor::EditionMode));
	CJsonValue results;
	std::string error;
	if (!runLua(s_RingState, args, 4, 1000, results, error))
	{
		call.fail(error);
		return;
	}
	CJsonValue r = results.at(0);
	static const char *const s_Modes[] = { "not_initialized", "edition", "going_to_test", "going_to_edition", "test", "dm", "animation_loading", "animation_waiting_for_loading",
		"animation_dm", "animation_play", "animation_going_to_dm", "animation_going_to_play" };
	uint mode = (uint)R2::getEditor().getMode();
	r.set("mode", mode < sizeof(s_Modes) / sizeof(s_Modes[0]) ? s_Modes[mode] : "unknown");
	call.reply(r);
}

void methodRingTree(CCtrlCall &call)
{
	if (!requireEdition(call))
		return;
	CJsonValue args = CJsonValue::array();
	args.push(call.getArgs().get("id").getString());
	args.push(CJsonValue((sint64)std::max((sint64)1, std::min((sint64)30, call.getArgs().get("depth").getInt(4)))));
	args.push(CJsonValue((sint64)std::max((sint64)10, std::min((sint64)100000, call.getArgs().get("max").getInt(3000)))));
	ringLua(call, s_RingTree, args);
}

// The values of ring.set and ring.create as the editor stores them: {"ref": id} is a reference to an instance
// (r2.RefId), an object with Class a new component of that class (r2.newComponent, then its own values), an object
// without Class fills the sub-object of that name (Position, Event, Action), and an array appends its items to the
// list of that name (Actions, Components, Conditions), as the editor's own code builds a handler with its steps. A
// value is checked against the type its class declares for the property (r2.Classes, Prop): a scalar, a reference or
// a component never replaces a list or a sub-object, a Number or a String property takes a number or a string, a
// reference an instance id; a property the class does not declare is checked by what it holds. The items of a list the
// editor fills with components are components, and a region's or a route's Points and a group's Components take what
// the tools put there. List items count from 0 in the messages, as the JSON of the caller does.
const char *const s_RingValues = "local function isRef(v)\n"
                                 "  if type(v) ~= 'table' or type(v.ref) ~= 'string' then return false end\n"
                                 "  for k in pairs(v) do if k ~= 'ref' then return false end end\n"
                                 "  return true\n"
                                 "end\n"
                                 "local function kindOf(cls, what)\n"
                                 "  local c = r2.Classes[cls]\n"
                                 "  while c do\n"
                                 "    if c.Name == what then return true end\n"
                                 "    c = c.BaseClass and r2.Classes[c.BaseClass] or nil\n"
                                 "  end\n"
                                 "  return false\n"
                                 "end\n"
                                 "local function propType(inst, name)\n"
                                 "  local cls = r2.Classes[inst.Class]\n"
                                 "  while cls do\n"
                                 "    for _, prop in ipairs(cls.Prop or {}) do\n"
                                 "      if prop.Name == name then return prop.Type end\n"
                                 "    end\n"
                                 "    cls = cls.BaseClass and r2.Classes[cls.BaseClass] or nil\n"
                                 "  end\n"
                                 "  return nil\n"
                                 "end\n"
                                 "local function newComponent(class, where)\n"
                                 "  local c = r2.newComponent(class)\n"
                                 "  if type(c) ~= 'table' or c.InstanceId == nil then error(where .. ': ' .. tostring(class) .. ' is not a component class') end\n"
                                 "  return c\n"
                                 "end\n"
                                 "-- the lists the editor fills with components (the others, such as ActivitiesId, hold ids)\n"
                                 "local componentLists = { Points = true, Components = true, Actions = true, Conditions = true, Activities = true, ChatSequences = true, Features = true,\n"
                                 "  Acts = true, Ghosts = true }\n"
                                 "-- a value of an entity not made yet: its own, else its palette element's (r2.getPropertyValue follows the Base chain,\n"
                                 "-- as an instance does)\n"
                                 "local function entityValue(item, name)\n"
                                 "  if item[name] ~= nil then return item[name] end\n"
                                 "  if type(item.Base) == 'string' and r2.isInPalette(item.Base) then return r2.getPropertyValue({ Base = item.Base }, name) end\n"
                                 "  return nil\n"
                                 "end\n"
                                 "-- what a list takes from the editor's tools, or nil: a region's Points its RegionVertex, a route's its WayPoint, and a\n"
                                 "-- group's Components the NPCs and creatures grouping takes (r2:testCanGroup, CAutoGroup: no object, no plant by its\n"
                                 "-- class or its SheetClient, as isBotObject and isPlant test them); item has Class, and Base and SheetClient when given\n"
                                 "local function wrongItem(ownerClass, listName, item)\n"
                                 "  if listName == 'Points' then\n"
                                 "    local want = (type(ownerClass) == 'string' and kindOf(ownerClass, 'Road')) and 'WayPoint' or 'RegionVertex'\n"
                                 "    if not kindOf(item.Class, want) then\n"
                                 "      return 'the Points of a ' .. (want == 'WayPoint' and 'route' or 'region') .. ' take ' .. want .. ' components, not ' .. tostring(item.Class)\n"
                                 "    end\n"
                                 "  elseif listName == 'Components' and type(ownerClass) == 'string' and kindOf(ownerClass, 'NpcGrpFeature') then\n"
                                 "    local sheet = entityValue(item, 'SheetClient')\n"
                                 "    local base = type(item.Base) == 'string' and item.Base or ''\n"
                                 "    if not kindOf(item.Class, 'Npc') or kindOf(item.Class, 'NpcPlant') or string.sub(base, 1, 27) == 'palette.entities.botobjects'\n"
                                 "      or (type(sheet) == 'string' and (r2:isNPCPlant(sheet) or r2:isBotObject(sheet))) then\n"
                                 "      return 'the Components of a group take the NPCs and creatures grouping takes (no object, no plant), not ' .. tostring(item.Class)\n"
                                 "        .. (base ~= '' and (' ' .. base) or '') .. (type(sheet) == 'string' and (' (' .. sheet .. ')') or '')\n"
                                 "    end\n"
                                 "  end\n"
                                 "  return nil\n"
                                 "end\n"
                                 "-- the Category and SubCategory of an entity, which the members of a group share (r2:testCanGroup, CAutoGroup)\n"
                                 "local function entityKind(item)\n"
                                 "  return tostring(entityValue(item, 'Category')), tostring(entityValue(item, 'SubCategory'))\n"
                                 "end\n"
                                 "-- what a property holds, by the type its class declares, else by what it holds now: 'list' (Table), 'sub' and its\n"
                                 "-- class (a sub-object: Position, Behavior...), or nil for a value\n"
                                 "local function shapeOf(ptype, cur)\n"
                                 "  if ptype == 'Table' then return 'list' end\n"
                                 "  if ptype ~= nil and r2.Classes[ptype] ~= nil then return 'sub', ptype end\n"
                                 "  if ptype ~= 'Number' and ptype ~= 'String' and ptype ~= 'RefId' and r2.isTable(cur) and not r2.isRefId(cur) then\n"
                                 "    if cur.Class == nil then return 'list' end\n"
                                 "    return 'sub', cur.Class\n"
                                 "  end\n"
                                 "  return nil\n"
                                 "end\n"
                                 "-- (ring.create) the Positions whose height was given, by the caller or by the createComponent of a feature, and the\n"
                                 "-- place in the caller's values of each component made from them\n"
                                 "local heightGiven = {}\n"
                                 "local fromValues = {}\n"
                                 "local fill\n"
                                 "local function toRing(v, path, ptype, cur)\n"
                                 "  local shape, subClass = shapeOf(ptype, cur)\n"
                                 "  if type(v) ~= 'table' or isRef(v) then\n"
                                 "    local what = isRef(v) and 'a reference' or ('the ' .. type(v) .. ' ' .. tostring(v))\n"
                                 "    if shape == 'list' then error(path .. ': a list, which takes an array, not ' .. what) end\n"
                                 "    if shape == 'sub' then error(path .. ': a sub-object (' .. tostring(subClass) .. '), which takes an object of values, not ' .. what) end\n"
                                 "    if ptype == 'RefId' then\n"
                                 "      if isRef(v) then return r2.RefId(v.ref) end\n"
                                 "      if type(v) == 'string' then return r2.RefId(v) end\n"
                                 "      error(path .. ': a reference, which takes an instance id or {ref = id}, not ' .. what)\n"
                                 "    end\n"
                                 "    if isRef(v) then\n"
                                 "      if ptype ~= nil then error(path .. ': a ' .. tostring(ptype) .. ' property, which takes no reference') end\n"
                                 "      return r2.RefId(v.ref)\n"
                                 "    end\n"
                                 "    if (ptype == 'Number' and type(v) ~= 'number') or (ptype == 'String' and type(v) ~= 'string') then\n"
                                 "      error(path .. ': a ' .. ptype .. ' property, which takes a ' .. string.lower(ptype) .. ', not ' .. what)\n"
                                 "    end\n"
                                 "    return v\n"
                                 "  end\n"
                                 "  if type(v.Class) ~= 'string' then error(path .. ': an object here needs Class (a new component) or ref (a reference)') end\n"
                                 "  if shape == 'list' then error(path .. ': a list, which takes an array of components, not one component') end\n"
                                 "  if shape == 'sub' and not kindOf(v.Class, subClass) then error(path .. ': a sub-object of class ' .. tostring(subClass) .. ', not ' .. v.Class) end\n"
                                 "  if ptype == 'RefId' or ptype == 'Number' or ptype == 'String' then error(path .. ': a ' .. ptype .. ' property, which takes no component') end\n"
                                 "  local c = newComponent(v.Class, path)\n"
                                 "  fromValues[c] = path\n"
                                 "  fill(c, v, path)\n"
                                 "  return c\n"
                                 "end\n"
                                 "fill = function(inst, values, path)\n"
                                 "  for k, v in pairs(values) do\n"
                                 "    if k ~= 'Class' then\n"
                                 "      local where = path .. '.' .. tostring(k)\n"
                                 "      local cur = inst[k]\n"
                                 "      if type(v) == 'table' and not isRef(v) and type(v.Class) ~= 'string' then\n"
                                 "        if r2.isRefId(cur) then error(where .. ': a reference, which takes an instance id or {ref = id}') end\n"
                                 "        if type(cur) ~= 'table' then error(where .. ': not a sub-object nor a list of the component') end\n"
                                 "        if v[1] ~= nil then\n"
                                 "          if cur.Class ~= nil then error(where .. ': a sub-object, which takes an object of values, not a list') end\n"
                                 "          for i, item in ipairs(v) do\n"
                                 "            local at = where .. '[' .. (i - 1) .. ']'\n"
                                 "            if componentLists[k] then\n"
                                 "              if type(item) ~= 'table' or isRef(item) or type(item.Class) ~= 'string' then\n"
                                 "                error(at .. ': ' .. tostring(k) .. ' holds components, which an object with Class makes, not ' .. (isRef(item) and 'a reference' or ('the ' .. type(item) .. ' ' .. tostring(item))))\n"
                                 "              end\n"
                                 "              local wrong = wrongItem(inst.Class, k, item)\n"
                                 "              if wrong then error(at .. ': ' .. wrong) end\n"
                                 "            end\n"
                                 "            table.insert(cur, toRing(item, at))\n"
                                 "          end\n"
                                 "        elseif next(v) ~= nil then\n"
                                 "          if cur.Class == nil then error(where .. ': a list, which takes an array of components, not an object') end\n"
                                 "          fill(cur, v, where)\n"
                                 "        end\n"
                                 "      else\n"
                                 "        inst[k] = toRing(v, where, propType(inst, k), cur)\n"
                                 "      end\n"
                                 "      if k == 'Position' and type(v) == 'table' and v.z ~= nil and type(inst[k]) == 'table' then heightGiven[inst[k]] = true end\n"
                                 "    end\n"
                                 "  end\n"
                                 "end\n";

void methodRingSet(CCtrlCall &call)
{
	if (!requireEdition(call))
		return;
	// value is required by the schema: a missing one or a null is refused before this, as nothing would be requested
	if (!finiteNumbers(call.getArgs().get("value")))
	{
		call.fail("value is not a finite number");
		return;
	}
	CJsonValue args = CJsonValue::array();
	args.push(call.getArgs().get("id"));
	args.push(call.getArgs().get("property"));
	args.push(call.getArgs().get("value"));
	ringLua(call, s_RingValues,
	    "local id, prop, value = ...\n"
	    "local inst = r2:getInstanceFromId(id)\n"
	    "if not inst then error('no instance ' .. tostring(id)) end\n"
	    "if type(value) == 'table' and not isRef(value) then error('value: a string, a number or {ref = id}; ring.create makes components') end\n"
	    "local v = toRing(value, 'value', propType(inst, prop), inst[prop])\n"
	    "r2.requestNewAction('Set ' .. tostring(prop))\n"
	    "r2.requestSetNode(id, prop, v)\n"
	    "return { id = id, property = prop, requested = true }\n",
	    args);
}

void methodRingCreate(CCtrlCall &call)
{
	if (!requireEdition(call))
		return;
	if (!finiteNumbers(call.getArgs().get("values")) || !finiteNumbers(call.getArgs().get("args")))
	{
		call.fail("values or args holds a number that is not finite");
		return;
	}
	if (!call.getArgs().get("args").isNull() && !call.getArgs().get("args").isArray())
	{
		call.fail("args is a list of the arguments of the feature's createComponent after x and y");
		return;
	}
	// a null in a list of the values would end that list in Lua, and lose what follows it without a word
	std::string nullPlace;
	if (findNullItem(call.getArgs().get("values"), "values", nullPlace))
	{
		call.fail(nullPlace + " is null, which a list of the values cannot hold: it holds components, references or values");
		return;
	}
	// the schema has checked the types: class, base, parent and property are strings, values an object
	CJsonValue args = CJsonValue::array();
	args.push(CJsonValue(call.getArgs().get("class").getString()));
	args.push(CJsonValue(call.getArgs().get("base").getString()));
	args.push(CJsonValue(call.getArgs().get("parent").getString()));
	args.push(CJsonValue(call.getArgs().get("property").getString()));
	args.push(CJsonValue((sint64)call.getArgs().get("position").getInt(-1)));
	args.push(call.getArgs().get("values").isObject() ? call.getArgs().get("values") : CJsonValue::object());
	args.push(call.getArgs().get("args").isArray() ? call.getArgs().get("args") : CJsonValue::array());
	// the count of args, for a null among them: Lua's length of the table stops at the first one
	args.push(CJsonValue((sint64)(call.getArgs().get("args").isArray() ? call.getArgs().get("args").size() : 0)));
	// the most points the drawing and new vertex tools give a region or a route, and the fewest the drawing tool makes
	// a route and a region with
	args.push(CJsonValue((sint64)R2::CToolDrawPrim::PrimMaxNumPoints));
	args.push(CJsonValue((sint64)R2::CToolDrawPrim::RoadMinNumPoints));
	args.push(CJsonValue((sint64)R2::CToolDrawPrim::RegionMinNumPoints));
	// an entity of the scenario is made as the palette tool makes it (CToolCreateEntity): the class the palette
	// element names (a creature's is NpcCreature or NpcPlant), Base the element, the element's name; without a base
	// the editor cannot display it. A custom NPC (the civilians) gets the editor's random look of its element. A
	// feature (TalkTo, Timer, ZoneTrigger...) is made by its own createComponent at x, y, as its palette tool makes it,
	// with the parts it needs (a zone, a Base), from arguments checked as its form bounds them. Each goes where the
	// editor's tools put it and is named as they name it: a feature, a dialog (ChatSequence, which has no
	// createComponent) and a group into the current act's Features, a region and a route into the permanent act's
	// default feature.
	ringLua(call, s_RingValues,
	    "local class, base, parent, prop, pos, values, extra, nextra, maxPoints, roadMinPoints, regionMinPoints = ...\n"
	    "local botObject = string.sub(base, 1, 27) == 'palette.entities.botobjects'\n"
	    "local creator, featureComponent\n"
	    "if class ~= '' then\n"
	    "  for _, f in pairs(r2.Features) do\n"
	    "    if type(f) == 'table' and type(f.Components) == 'table' and f.Components[class] ~= nil then\n"
	    "      creator = f.Components[class].createComponent or f.createComponent\n"
	    "      if creator then featureComponent = f.Components[class] break end\n"
	    "    end\n"
	    "  end\n"
	    "end\n"
	    "if creator and base ~= '' then error(class .. ' is a feature: it is made by its class, without base') end\n"
	    "local grouped = class == 'NpcGrpFeature'\n"
	    "local drawn = class == 'Region' or class == 'Road'\n"
	    "local zoned = class == 'ZoneTrigger' or class == 'BanditCamp' or class == 'Fauna'\n"
	    "-- where the editor puts it: a feature and a dialog where their tools put them, and a group where grouping puts a new\n"
	    "-- one (CAutoGroup::group, r2:group), into the current act's Features; an entity into the current act's default\n"
	    "-- feature and an object into the permanent act's, as the palette puts them; a region and a route into the permanent\n"
	    "-- act's default feature, as the drawing tool puts them (CToolDrawPrim::commit)\n"
	    "local actLevel = creator ~= nil or class == 'ChatSequence' or grouped\n"
	    "local parentId, property = parent, prop\n"
	    "if parentId == '' then\n"
	    "  if actLevel then\n"
	    "    parentId = r2.Scenario:getCurrentAct().InstanceId\n"
	    "  else\n"
	    "    local act = (botObject or drawn) and r2.Scenario:getBaseAct() or r2.Scenario:getCurrentAct()\n"
	    "    parentId = act.Features[0].InstanceId\n"
	    "  end\n"
	    "end\n"
	    "local parentInst = r2:getInstanceFromId(parentId)\n"
	    "if not parentInst then error('no parent instance ' .. tostring(parentId)) end\n"
	    "if property == '' then\n"
	    "  -- a point goes into the Points of its region or route, as the new vertex tool puts it\n"
	    "  if (class == 'RegionVertex' or class == 'WayPoint') and r2.isTable(parentInst.Points) then\n"
	    "    property = 'Points'\n"
	    "  else\n"
	    "    property = actLevel and 'Features' or 'Components'\n"
	    "  end\n"
	    "end\n"
	    "-- the world position of the parent, which the positions below it are relative to\n"
	    "local offX, offY, offZ = 0, 0, 0\n"
	    "if parentInst.getWorldPos then\n"
	    "  local wp = parentInst:getWorldPos()\n"
	    "  if type(wp) == 'table' and type(wp.x) == 'number' then offX, offY, offZ = wp.x, wp.y, tonumber(wp.z) or 0 end\n"
	    "end\n"
	    "-- the parent holds it in a list of that name, at a place the list has (-1 the end): the scenario service drops an\n"
	    "-- insert into anything else with a line in its log only\n"
	    "local list = parentInst[property]\n"
	    "if not (r2.isTable(list) and not r2.isRefId(list) and list.Class == nil) then\n"
	    "  error(tostring(parentId) .. ' has no list ' .. tostring(property) .. ' to put it in')\n"
	    "end\n"
	    "if pos < -1 or pos > list.Size then\n"
	    "  error('position ' .. tostring(pos) .. ' is not in ' .. tostring(parentId) .. '.' .. tostring(property) .. ', which has ' .. tostring(list.Size) .. ' items (-1 is the end)')\n"
	    "end\n"
	    "-- the drawing and new vertex tools give a region or a route maxPoints points at most (CToolDrawPrim::PrimMaxNumPoints)\n"
	    "if property == 'Points' and list.Size >= maxPoints then\n"
	    "  error(tostring(parentId) .. ' has ' .. list.Size .. ' points, the most the editor\\'s tools give a region or a route')\n"
	    "end\n"
	    "local elt\n"
	    "if base ~= '' then\n"
	    "  if not r2.isInPalette(base) then error('no palette element ' .. base .. '; ring.palette lists them') end\n"
	    "  elt = r2.getPaletteElement(base)\n"
	    "  if class == '' then\n"
	    "    class = (elt and elt.Class) or 'Npc'\n"
	    "    if string.sub(base, 1, 26) == 'palette.entities.creatures' then\n"
	    "      class = 'NpcCreature'\n"
	    "      if elt and elt.SheetClient and r2:isNPCPlant(elt.SheetClient) then class = 'NpcPlant' end\n"
	    "    end\n"
	    "  end\n"
	    "elseif class == '' or class == 'Npc' or class == 'NpcCreature' or class == 'NpcPlant' or class == 'NpcCustom' then\n"
	    "  error('an entity needs base, a palette element id (ring.palette lists them): the editor never makes one without')\n"
	    "end\n"
	    "-- what goes in a region's or a route's Points or in a group, as the tools put it there\n"
	    "local wrong = wrongItem(parentInst.Class, property, { Class = class, Base = base ~= '' and base or values.Base, SheetClient = values.SheetClient })\n"
	    "if wrong then error(tostring(parentId) .. '.' .. tostring(property) .. ': ' .. wrong) end\n"
	    "-- the arguments of a feature's createComponent after x and y, as its palette tool and its form give them, within the\n"
	    "-- form's bounds: what each is, its kind, its bounds or its palette branch, and whether it may be left out; the other\n"
	    "-- features take none\n"
	    "local featureArgs = {\n"
	    "  BanditCamp = { { 'the count of bandits', 'integer', 1, 12 }, { 'the race (0 Fyros, 1 Matis, 2 Tryker, 3 Zorai)', 'integer', 0, 3 },\n"
	    "    { 'the level (0 for 20, 1 for 70, 2 for 120, 3 for 170, 4 for 220)', 'integer', 0, 4 } },\n"
	    "  Fauna = { { 'the count of carnivores', 'integer', 1, 12 }, { 'the carnivore', 'creature', 'creatures_predators' }, { 'the name of the carnivores', 'text' },\n"
	    "    { 'the count of herbivores', 'integer', 1, 12 }, { 'the herbivore', 'creature', 'creatures_passive' }, { 'the name of the herbivores', 'text' } },\n"
	    "  Creature = { { 'the count of creatures', 'integer', 1, 12 }, { 'the creature', 'creature' }, { 'the name of the creatures', 'text' },\n"
	    "    { 'the sleep zone', 'region' }, { 'the food zone', 'region' } },\n"
	    "  Counter = { { 'the value it starts from', 'integer', 1 }, { 'the value it triggers at', 'integer', 0 } },\n"
	    "  Timer = { { 'the seconds', 'integer', 0, 999, true }, { 'the minutes', 'integer', 0, 120, true }, { 'cyclic (0 or 1)', 'integer', 0, 1, true } },\n"
	    "}\n"
	    "-- the ecosystems the Fauna form offers (its Lakes is Lacustre in the palette)\n"
	    "local formEcosystems = { Desert = true, Forest = true, Jungle = true, Lacustre = true, PrimeRoots = true, Goo = true }\n"
	    "if creator then\n"
	    "  local spec = featureArgs[class] or {}\n"
	    "  if nextra > #spec then error(class .. ' takes ' .. #spec .. ' args after x and y, not ' .. nextra) end\n"
	    "  for i, a in ipairs(spec) do\n"
	    "    local v, where = extra[i], 'args[' .. (i - 1) .. '], ' .. a[1]\n"
	    "    if v == nil then\n"
	    "      if not a[5] then error(where .. ', is required') end\n"
	    "    elseif a[2] == 'integer' then\n"
	    "      if type(v) ~= 'number' or v ~= math.floor(v) then error(where .. ', is an integer, not ' .. tostring(v)) end\n"
	    "      if v < a[3] or (a[4] ~= nil and v > a[4]) then\n"
	    "        error(where .. ', is ' .. (a[4] ~= nil and ('from ' .. a[3] .. ' to ' .. a[4]) or ('at least ' .. a[3])) .. ', not ' .. tostring(v))\n"
	    "      end\n"
	    "    elseif a[2] == 'text' then\n"
	    "      if type(v) ~= 'string' or v == '' then error(where .. ', is a text') end\n"
	    "    elseif a[2] == 'creature' then\n"
	    "      local found = type(v) == 'string' and r2.isInPalette(v) and string.sub(v, 1, 26) == 'palette.entities.creatures'\n"
	    "      if found and a[3] then\n"
	    "        -- one the form can list: of its branch of the palette, allowed by the Ring access, of an ecosystem and a level\n"
	    "        -- its choices hold\n"
	    "        local entries = r2.Palette.Entries.creature and r2.Palette.Entries.creature[a[3]]\n"
	    "        found = false\n"
	    "        for _, e in pairs(entries and entries.instances or {}) do\n"
	    "          if e.Id == v then found = true end\n"
	    "        end\n"
	    "        local e = found and r2.getPaletteElement(v)\n"
	    "        found = e and e.RingAccess and r2.RingAccess.testAccess(e.RingAccess) and formEcosystems[e.Ecosystem]\n"
	    "          and type(e.Level) == 'number' and e.Level >= 1 and e.Level <= 250\n"
	    "      end\n"
	    "      if not found then\n"
	    "        error(where .. ', is ' .. (a[3] and ('a creature the Fauna form lists (r2.Palette.Entries.creature.' .. a[3] .. ', allowed by the Ring access, of the form\\'s ecosystems and levels 1 to 250)')\n"
	    "          or 'a palette element of the creatures') .. ', not ' .. tostring(v))\n"
	    "      end\n"
	    "    elseif a[2] == 'region' then\n"
	    "      local zone = type(v) == 'string' and r2:getInstanceFromId(v)\n"
	    "      if not zone or not zone:isKindOf('Region') then error(where .. ', is the instance id of a region, not ' .. tostring(v)) end\n"
	    "    end\n"
	    "  end\n"
	    "  if class == 'BanditCamp' then\n"
	    "    -- the form offers the levels whose bandits the palette has and the Ring access allows (updateBanditCampEnum)\n"
	    "    local id = 'palette.entities.npcs.bandits.' .. featureComponent.RacesId[extra[2] + 1] .. '_melee_dd_' .. featureComponent.LevelsId[extra[3] + 1]\n"
	    "    local e = r2.isInPalette(id) and r2.getPaletteElement(id)\n"
	    "    if not (e and e.RingAccess and r2.RingAccess.testAccess(e.RingAccess)) then\n"
	    "      error('args[2], the level: the form offers no bandits of that race and level (' .. id .. ' is not in the palette, or the Ring access does not allow it)')\n"
	    "    end\n"
	    "  end\n"
	    "end\n"
	    "-- the room left, as the palette tools test it (r2:getLeftQuota, r2:checkAiQuota, r2:checkStaticQuota), for what it\n"
	    "-- makes and the entities its values hold (a group's members): an object on the static quota, the others on the AI\n"
	    "-- quota; a feature as its tool counts it, a bandit camp its bandits, a fauna feature its herds and itself, the others\n"
	    "-- one, and these tools only test that the smaller of the two quotas is not spent (r2:getLeftQuota() <= 0). The tools\n"
	    "-- of TalkTo, RewardProvider, NpcInteraction, SceneryObjectInteraction and SceneryObjectInteractionTaskStep warn and\n"
	    "-- never refuse (they drop what r2:checkAiQuota returns): those are refused on the AI quota all the same, as the act\n"
	    "-- has no room for them\n"
	    "local smallerQuota = { HuntTask = true, KillNpc = true, LootSpawner = true, ProximityDialog = true, Quest = true, SceneryObjectRemover = true,\n"
	    "  TargetMob = true, VisitZone = true }\n"
	    "local needAi, needStatic = 0, 0\n"
	    "local function countEntities(v)\n"
	    "  if type(v) ~= 'table' or isRef(v) then return end\n"
	    "  if type(v.Class) == 'string' and kindOf(v.Class, 'Npc') then\n"
	    "    if type(v.Base) == 'string' and string.sub(v.Base, 1, 27) == 'palette.entities.botobjects' then needStatic = needStatic + 1 else needAi = needAi + 1 end\n"
	    "  end\n"
	    "  for _, sub in pairs(v) do countEntities(sub) end\n"
	    "end\n"
	    "for k, v in pairs(values) do\n"
	    "  if k ~= 'Class' then countEntities(v) end\n"
	    "end\n"
	    "local left, leftAi, leftStatic = r2:getLeftQuota()\n"
	    "if base ~= '' then\n"
	    "  if botObject then needStatic = needStatic + 1 else needAi = needAi + 1 end\n"
	    "elseif creator then\n"
	    "  if smallerQuota[class] then\n"
	    "    if left < 1 then error('no room left in the act: ' .. math.floor(left) .. ' of the smaller of its AI and static quotas, which the tool of ' .. class .. ' tests') end\n"
	    "  elseif class == 'BanditCamp' then needAi = needAi + extra[1]\n"
	    "  elseif class == 'Fauna' then needAi = needAi + extra[1] + extra[4] + 1\n"
	    "  else needAi = needAi + 1 end\n"
	    "end\n"
	    "if needAi > 0 and leftAi < needAi then error('no room left in the act for ' .. needAi .. ' more of its AI quota (' .. math.floor(leftAi) .. ' left)') end\n"
	    "if needStatic > 0 and leftStatic < needStatic then error('no room left in the act for ' .. needStatic .. ' more objects (' .. math.floor(leftStatic) .. ' left of its static quota)') end\n"
	    "local p = values.Position\n"
	    "local at = type(p) == 'table' and not isRef(p) and p.Class == nil and type(p.x) == 'number' and type(p.y) == 'number'\n"
	    "local stands = base ~= '' or creator ~= nil or class == 'ChatSequence'\n"
	    "if stands and not at then error('values.Position {x, y}: where it stands') end\n"
	    "-- the heights a feature's createComponent gives what it makes are its tool's\n"
	    "local function markHeights(node, depth)\n"
	    "  if depth > 20 then error('the createComponent of ' .. class .. ' made components nested deeper than 20 levels') end\n"
	    "  for k, v in pairs(node) do\n"
	    "    if type(v) == 'table' and not r2.isRefId(v) then\n"
	    "      if k == 'Position' then heightGiven[v] = true else markHeights(v, depth + 1) end\n"
	    "    end\n"
	    "  end\n"
	    "end\n"
	    "local inst\n"
	    "if creator then\n"
	    "  -- made at its place in the world, as its tool makes it (its ground, and what it makes that does not inherit its\n"
	    "  -- position, are taken there), then stored relative to the parent\n"
	    "  inst = creator(offX + p.x, offY + p.y, (table.unpack or unpack)(extra, 1, nextra))\n"
	    "  if type(inst) ~= 'table' or inst.InstanceId == nil then error('the createComponent of ' .. class .. ' made nothing') end\n"
	    "  if inst.InheritPos ~= 0 then\n"
	    "    inst.Position.x, inst.Position.y = inst.Position.x - offX, inst.Position.y - offY\n"
	    "    inst.Position.z = (tonumber(inst.Position.z) or 0) - offZ\n"
	    "  end\n"
	    "  markHeights(inst, 0)\n"
	    "  if p.z ~= nil then values.Position = { z = p.z } else values.Position = nil end\n"
	    "else\n"
	    "  if class == 'NpcCustom' then\n"
	    "    -- the palette tool copies the look of the entity it shows while placing; without one the editor's own random\n"
	    "    -- look of the element is taken, as a bandit camp makes its bandits\n"
	    "    inst = r2:randomNPC(base)\n"
	    "  else\n"
	    "    inst = newComponent(class, 'class')\n"
	    "  end\n"
	    "  if base ~= '' and class ~= 'NpcCustom' then\n"
	    "    inst.Base = base\n"
	    "    inst.Name = r2.PaletteIdToTranslation[base] or class\n"
	    "    -- as the palette names them: a creature keeps its name, the others get a number\n"
	    "    if string.sub(base, 1, 26) ~= 'palette.entities.creatures' then inst.Name = r2:genInstanceName(inst.Name) end\n"
	    "  elseif class == 'ChatSequence' then\n"
	    "    -- as the dialog tool makes one (r2.dialogs:newSequenceInst)\n"
	    "    inst.Base = r2.Translator.getDebugBase('palette.entities.botobjects.dialog')\n"
	    "    inst.Name = r2:genInstanceName(i18n.get('uiR2EDDialog'))\n"
	    "  elseif grouped then\n"
	    "    -- as grouping names a new group (r2:group), from the kind of its first member\n"
	    "    local first = type(values.Components) == 'table' and values.Components[1]\n"
	    "    local kindName = type(first) == 'table' and type(first.Base) == 'string' and r2.PaletteIdToGroupTranslation[first.Base] or nil\n"
	    "    local groupName = i18n.get('uiR2EDNameGroup')\n"
	    "    if kindName ~= nil then groupName = kindName .. ' ' .. groupName end\n"
	    "    inst.Name = r2:genInstanceName(groupName)\n"
	    "  elseif drawn then\n"
	    "    -- as the drawing tool names them\n"
	    "    inst.Name = r2:genInstanceName(i18n.get(class == 'Road' and 'uiR2EDNameBotRoad' or 'uiR2EDNameBotRegion'))\n"
	    "  end\n"
	    "end\n"
	    "fill(inst, values, 'values')\n"
	    "if class == 'NpcCustom' and elt and (elt.SheetModel or '') ~= '' then\n"
	    "  -- the combat sheet that goes with the weapons it holds, as the palette tool sets it (r2:searchSheet)\n"
	    "  local sheet = r2:searchSheet(r2.getPropertyValue(inst, 'Equipment'), inst.WeaponRightHand, inst.WeaponLeftHand, r2.getPropertyValue(inst, 'Sheet'), elt.SheetModel)\n"
	    "  if sheet == '' then error('no combat sheet of ' .. base .. ' goes with the weapons it holds') end\n"
	    "  inst.Sheet = sheet\n"
	    "end\n"
	    "-- the position tests of the palette and drawing tools (r2:isValidPos, r2:isValidPoly), on ground the AI can use:\n"
	    "-- what stands in the world, a feature and a dialog where it is put, an entity given in values, every point of a\n"
	    "-- region or a route, and the zones of a trigger zone, a bandit camp or a herd, inside and at every point (their\n"
	    "-- palette tools test them; the other features' tools test their own position only). What is tested stands on the\n"
	    "-- ground: a height not given is the ground's at its place in the world, relative to what holds it, since the AI\n"
	    "-- service puts a bot or a point on the layer nearest to its height. A region or a route has maxPoints points at most,\n"
	    "-- and the members of a group stay within r2.MaxGroupRadius of each other, as grouping and the move tool keep them\n"
	    "-- (r2:testCanGroup, r2.checkGroupDistance)\n"
	    "local bad = {}\n"
	    "local groups = {}\n"
	    "local function test(x, y, path)\n"
	    "  if not r2:isValidPos(x, y) then table.insert(bad, string.format('%s at %.1f, %.1f', path, x, y)) end\n"
	    "end\n"
	    "local function walk(node, ox, oy, oz, path, depth)\n"
	    "  if depth > 20 then error(path .. ': nested deeper than 20 levels, past what the position tests follow') end\n"
	    "  local p = node.Position\n"
	    "  if type(p) == 'table' and not r2.isRefId(p) then\n"
	    "    if type(p.x) ~= 'number' or type(p.y) ~= 'number' then error(path .. '.Position: x and y are numbers, which the position tests need') end\n"
	    "    if node.InheritPos == 0 then ox, oy, oz = 0, 0, 0 end\n"
	    "    local wx, wy = ox + p.x, oy + p.y\n"
	    "    local tested\n"
	    "    if depth == 0 then\n"
	    "      -- a point added to a region or a route on its own is tested as the new vertex tool tests it\n"
	    "      tested = stands or class == 'RegionVertex' or class == 'WayPoint'\n"
	    "    elseif (drawn or zoned) and (node.Class == 'RegionVertex' or node.Class == 'WayPoint') then\n"
	    "      tested = true\n"
	    "    else\n"
	    "      tested = creator == nil and type(node.Class) == 'string' and kindOf(node.Class, 'Npc')\n"
	    "    end\n"
	    "    if tested then\n"
	    "      test(wx, wy, path)\n"
	    "      if not heightGiven[p] then p.z = r2:snapZToGround(wx, wy) - oz end\n"
	    "    end\n"
	    "    ox, oy, oz = wx, wy, oz + (tonumber(p.z) or 0)\n"
	    "  end\n"
	    "  if (node.Class == 'Region' or node.Class == 'Road') and type(node.Points) == 'table' then\n"
	    "    -- the drawing tool commits a route with roadMinPoints points at least, a region with regionMinPoints\n"
	    "    -- (CToolDrawPrim::canTerminate), and stops at maxPoints\n"
	    "    local fewest = node.Class == 'Road' and roadMinPoints or regionMinPoints\n"
	    "    if #node.Points < fewest then\n"
	    "      error(path .. '.Points: ' .. #node.Points .. ' points, fewer than the ' .. fewest .. ' the editor\\'s drawing tool makes a ' .. (node.Class == 'Road' and 'route' or 'region') .. ' with')\n"
	    "    end\n"
	    "    if #node.Points > maxPoints then\n"
	    "      error(path .. '.Points: ' .. #node.Points .. ' points, more than the ' .. maxPoints .. ' the editor\\'s drawing tool gives a region or a route')\n"
	    "    end\n"
	    "  end\n"
	    "  if node.Class == 'NpcGrpFeature' and type(node.Components) == 'table' then table.insert(groups, { node = node, path = path }) end\n"
	    "  if zoned and node.Class == 'Region' and type(node.Points) == 'table' then\n"
	    "    local poly = {}\n"
	    "    for _, v in ipairs(node.Points) do\n"
	    "      if type(v.Position) == 'table' then table.insert(poly, CVector2f(ox + v.Position.x, oy + v.Position.y)) end\n"
	    "    end\n"
	    "    if #poly >= 3 and not r2:isValidPoly(poly) then table.insert(bad, path .. ', inside the zone') end\n"
	    "  end\n"
	    "  for k, v in pairs(node) do\n"
	    "    if type(v) == 'table' and k ~= 'Position' and not r2.isRefId(v) then\n"
	    "      local sub = fromValues[v] or (type(k) == 'number' and (path .. '[' .. (k - 1) .. ']') or (path .. '.' .. tostring(k)))\n"
	    "      walk(v, ox, oy, oz, sub, depth + 1)\n"
	    "    end\n"
	    "  end\n"
	    "end\n"
	    "walk(inst, offX, offY, offZ, 'values', 0)\n"
	    "if #bad > 0 then\n"
	    "  error('off the ground the AI can use (the tops of root mounds and pillars, cliffs, the rim of the island), where the editor\\'s tools refuse it: ' .. table.concat(bad, '; '))\n"
	    "end\n"
	    "-- the group radius, measured in the group's coordinates as r2.checkGroupDistance measures it\n"
	    "local far = {}\n"
	    "local function apart(a, b)\n"
	    "  return math.sqrt((a.x - b.x) ^ 2 + (a.y - b.y) ^ 2)\n"
	    "end\n"
	    "for _, g in ipairs(groups) do\n"
	    "  local members = g.node.Components\n"
	    "  for i = 1, #members do\n"
	    "    for j = i + 1, #members do\n"
	    "      if type(members[i].Position) == 'table' and type(members[j].Position) == 'table' then\n"
	    "        local d = apart(members[i].Position, members[j].Position)\n"
	    "        if d > r2.MaxGroupRadius then\n"
	    "          table.insert(far, string.format('%s is %.1f m from %s', fromValues[members[j]] or (g.path .. '.Components[' .. (j - 1) .. ']'), d,\n"
	    "            fromValues[members[i]] or (g.path .. '.Components[' .. (i - 1) .. ']')))\n"
	    "        end\n"
	    "      end\n"
	    "    end\n"
	    "  end\n"
	    "end\n"
	    "-- a new member of a group that is there: from each of its members\n"
	    "if property == 'Components' and parentInst.isKindOf and parentInst:isKindOf('NpcGrpFeature') and type(inst.Class) == 'string' and kindOf(inst.Class, 'Npc') then\n"
	    "  for i = 0, parentInst.Components.Size - 1 do\n"
	    "    local member = parentInst.Components[i]\n"
	    "    if r2.isTable(member.Position) and type(member.Position.x) == 'number' then\n"
	    "      local d = apart(inst.Position, member.Position)\n"
	    "      if d > r2.MaxGroupRadius then table.insert(far, string.format('values is %.1f m from member %s of %s', d, tostring(i), tostring(parentId))) end\n"
	    "    end\n"
	    "  end\n"
	    "end\n"
	    "if #far > 0 then\n"
	    "  error('farther from the other members of its group than the ' .. tostring(r2.MaxGroupRadius) .. ' m the editor\\'s grouping and move tool allow: ' .. table.concat(far, '; '))\n"
	    "end\n"
	    "-- the members of a group share the Category and SubCategory of its leader (its first member), as grouping requires\n"
	    "local mixed = {}\n"
	    "for _, g in ipairs(groups) do\n"
	    "  local members = g.node.Components\n"
	    "  if #members > 0 then\n"
	    "    local category, subCategory = entityKind(members[1])\n"
	    "    for j = 2, #members do\n"
	    "      local c, s = entityKind(members[j])\n"
	    "      if c ~= category or s ~= subCategory then\n"
	    "        table.insert(mixed, string.format('%s is %s/%s, %s %s/%s', fromValues[members[j]] or (g.path .. '.Components[' .. (j - 1) .. ']'), c, s,\n"
	    "          fromValues[members[1]] or (g.path .. '.Components[0]'), category, subCategory))\n"
	    "      end\n"
	    "    end\n"
	    "  end\n"
	    "end\n"
	    "if property == 'Components' and parentInst.isKindOf and parentInst:isKindOf('NpcGrpFeature') and parentInst.Components.Size > 0 then\n"
	    "  local leader = parentInst.Components[0]\n"
	    "  local category, subCategory = tostring(leader:getCategory()), tostring(leader:getSubCategory())\n"
	    "  local c, s = entityKind(inst)\n"
	    "  if c ~= category or s ~= subCategory then\n"
	    "    table.insert(mixed, string.format('values is %s/%s, the leader of %s %s/%s', c, s, tostring(parentId), category, subCategory))\n"
	    "  end\n"
	    "end\n"
	    "if #mixed > 0 then\n"
	    "  error('not of the Category and SubCategory of its group, which grouping requires: ' .. table.concat(mixed, '; '))\n"
	    "end\n"
	    "if class == 'NpcCustom' and values.Name == nil then\n"
	    "  -- a name of its people from the list the palette tool draws from (randomNPCName), which gives each name once: taken\n"
	    "  -- only now that nothing refuses the request\n"
	    "  local sex = isR2PlayerMale(inst.SheetClient) and r2.male or r2.female\n"
	    "  inst.Name = r2:randomNPCName(getR2PlayerRace(inst.SheetClient), sex)\n"
	    "end\n"
	    "r2.requestNewAction('Create ' .. tostring(inst.Name or class))\n"
	    "r2.requestInsertNode(parentId, property, pos, '', inst)\n"
	    "return { id = inst.InstanceId, class = class, base = base, parent = parentId, property = property, requested = true }\n",
	    args);
}

void methodRingPalette(CCtrlCall &call)
{
	if (!requireEditor(call))
		return;
	CJsonValue args = CJsonValue::array();
	args.push(CJsonValue(toLowerAscii(call.getArgs().get("contains").getString())));
	args.push(CJsonValue(call.getArgs().get("prefix").getString()));
	args.push(CJsonValue((sint64)std::max((sint64)1, std::min((sint64)2000, call.getArgs().get("max").getInt(100)))));
	ringLua(call,
	    "local contains, prefix, max = ...\n"
	    "local ids = {}\n"
	    "for id, name in pairs(r2.PaletteIdToTranslation) do\n"
	    "  local text = tostring(name)\n"
	    "  if (prefix == '' or string.sub(id, 1, string.len(prefix)) == prefix)\n"
	    "    and (contains == '' or string.find(string.lower(id .. ' ' .. text), contains, 1, true)) then\n"
	    "    table.insert(ids, id)\n"
	    "  end\n"
	    "end\n"
	    "table.sort(ids)\n"
	    "local out = {}\n"
	    "for i = 1, math.min(#ids, max) do out[i] = { id = ids[i], name = tostring(r2.PaletteIdToTranslation[ids[i]]) } end\n"
	    "return { elements = out, count = #ids, truncated = #ids > max }\n",
	    args);
}

void methodRingErase(CCtrlCall &call)
{
	if (!requireEdition(call))
		return;
	CJsonValue args = CJsonValue::array();
	args.push(call.getArgs().get("id"));
	ringLua(call,
	    "local id = ...\n"
	    "if not r2:getInstanceFromId(id) then error('no instance ' .. tostring(id)) end\n"
	    "r2.requestNewAction('Delete ' .. tostring(id))\n"
	    "r2.requestEraseNode(id, '', -1)\n"
	    "return { id = id, requested = true }\n",
	    args);
}

void methodRingMove(CCtrlCall &call)
{
	if (!requireEdition(call))
		return;
	double x = 0, y = 0;
	if (!readNumber(call, "x", s_MaxCoordinate, x) || !readNumber(call, "y", s_MaxCoordinate, y))
		return;
	// a new place (x and y) or a new parent (parent, with property and position), never both: one would be dropped
	const CJsonValue &given = call.getArgs();
	bool place = !given.get("x").isNull() || !given.get("y").isNull();
	if (place && (given.get("x").isNull() || given.get("y").isNull()))
	{
		call.fail("x and y go together: they are the new place of the instance");
		return;
	}
	// an empty parent is none, as the chunk takes it (the schema has made parent a string when it is given)
	if (place && !given.get("parent").getString().empty())
	{
		call.fail(toString("ring.move takes x and y (a new place) or parent (a new parent), not both: parent '%s' with x %g, y %g", given.get("parent").getString().c_str(), x, y));
		return;
	}
	// (an empty property and the position -1 are what is taken when they are not given)
	if (place && (!given.get("property").getString().empty() || given.get("position").getInt(-1) != -1))
	{
		call.fail("property and position go with parent, not with x and y");
		return;
	}
	CJsonValue args = CJsonValue::array();
	args.push(call.getArgs().get("id"));
	args.push(call.getArgs().get("parent"));
	// an empty property is one not given, as the description says
	args.push(!call.getArgs().get("property").getString().empty() ? call.getArgs().get("property") : CJsonValue("Components"));
	args.push(CJsonValue((sint64)call.getArgs().get("position").getInt(-1)));
	args.push(call.getArgs().get("x").isNull() ? CJsonValue() : CJsonValue(x));
	args.push(call.getArgs().get("y").isNull() ? CJsonValue() : CJsonValue(y));
	ringLua(call,
	    "local id, parent, prop, pos, x, y = ...\n"
	    "local inst = r2:getInstanceFromId(id)\n"
	    "if not inst then error('no instance ' .. tostring(id)) end\n"
	    "if x and y then\n"
	    "  -- the position test of the move tool (r2:isValidPos), on ground the AI can use: the new place of what stands in\n"
	    "  -- the world, or of every point of a region or a route and every member of a group, which the tool tests when\n"
	    "  -- they all were on that ground before the move (CToolSelectMove, CDisplayerVisualGroup::isAccessible)\n"
	    "  if not r2.isTable(inst.Position) or type(inst.Position.x) ~= 'number' then error(tostring(id) .. ' has no position') end\n"
	    "  local dx, dy = x - inst.Position.x, y - inst.Position.y\n"
	    "  local list, what\n"
	    "  if inst.Class == 'Region' or inst.Class == 'Road' then list, what = inst.Points, 'point '\n"
	    "  elseif inst.Class == 'NpcGrpFeature' then list, what = inst.Components, 'member ' end\n"
	    "  local places = {}\n"
	    "  if list then\n"
	    "    for i = 0, list.Size - 1 do\n"
	    "      if list[i].getWorldPos then table.insert(places, { wp = list[i]:getWorldPos(), name = what .. i }) end\n"
	    "    end\n"
	    "  elseif inst.getWorldPos then\n"
	    "    places[1] = { wp = inst:getWorldPos(), name = tostring(id) }\n"
	    "  end\n"
	    "  local tested = true\n"
	    "  if list then\n"
	    "    for _, pl in ipairs(places) do\n"
	    "      if not r2:isValidPos(pl.wp.x, pl.wp.y) then tested = false end\n"
	    "    end\n"
	    "  end\n"
	    "  local bad = {}\n"
	    "  local landed\n"
	    "  for _, pl in ipairs(places) do\n"
	    "    if r2:isValidPos(pl.wp.x + dx, pl.wp.y + dy) then\n"
	    "      landed = landed or pl\n"
	    "    elseif tested then\n"
	    "      table.insert(bad, string.format('%s at %.1f, %.1f', pl.name, pl.wp.x + dx, pl.wp.y + dy))\n"
	    "    end\n"
	    "  end\n"
	    "  -- partly off that ground before: the tool still takes it by a point on ground, so one at least lands on it\n"
	    "  if not tested and not landed and #places > 0 then table.insert(bad, 'none of its ' .. #places .. ' points or members') end\n"
	    "  if #bad > 0 then\n"
	    "    error('off the ground the AI can use (the tops of root mounds and pillars, cliffs, the rim of the island), where the editor\\'s move tool refuses it: ' .. table.concat(bad, '; '))\n"
	    "  end\n"
	    "  -- the distance test of the move tool (r2.checkGroupDistance): a member of a group stays within r2.MaxGroupRadius of\n"
	    "  -- every other member, all relative to the group\n"
	    "  local group = inst.ParentInstance\n"
	    "  if group and group.isKindOf and group:isKindOf('NpcGrpFeature') then\n"
	    "    local far = {}\n"
	    "    for i = 0, group.Components.Size - 1 do\n"
	    "      local member = group.Components[i]\n"
	    "      if member.InstanceId ~= inst.InstanceId and r2.isTable(member.Position) then\n"
	    "        local d = math.sqrt((x - member.Position.x) ^ 2 + (y - member.Position.y) ^ 2)\n"
	    "        if d > r2.MaxGroupRadius then table.insert(far, string.format('%.1f m from member %s', d, tostring(i))) end\n"
	    "      end\n"
	    "    end\n"
	    "    if #far > 0 then\n"
	    "      error('farther from the other members of its group than the ' .. tostring(r2.MaxGroupRadius) .. ' m the editor\\'s move tool allows: ' .. table.concat(far, '; '))\n"
	    "    end\n"
	    "  end\n"
	    "  -- the move tool also refuses a point that makes the outline of its region cross itself (isValidShape, which the\n"
	    "  -- world map's polygon gives): Lua cannot reach that test, so it is not made here, and the description says so\n"
	    "  -- the height, as the move tool writes the whole Position: the ground at the new place, relative to the parent. A\n"
	    "  -- region, a route or a group is taken by the first of its points or members that lands on that ground, as the tool\n"
	    "  -- takes it by a point on the ground: that one stands on the ground, the others keep their heights relative to it\n"
	    "  local z = inst.Position.z\n"
	    "  if inst.getWorldPos then\n"
	    "    local wp = inst:getWorldPos()\n"
	    "    local parentZ = wp.z - (tonumber(inst.Position.z) or 0)\n"
	    "    if not list then\n"
	    "      z = r2:snapZToGround(wp.x + dx, wp.y + dy) - parentZ\n"
	    "    elseif landed then\n"
	    "      z = r2:snapZToGround(landed.wp.x + dx, landed.wp.y + dy) - parentZ - (landed.wp.z - wp.z)\n"
	    "    end\n"
	    "  end\n"
	    "  r2.requestNewAction('Move ' .. tostring(id))\n"
	    "  r2.requestSetNode(inst.Position.InstanceId, 'x', x)\n"
	    "  r2.requestSetNode(inst.Position.InstanceId, 'y', y)\n"
	    "  if z ~= nil then r2.requestSetNode(inst.Position.InstanceId, 'z', z) end\n"
	    "  return { id = id, x = x, y = y, z = z, requested = true }\n"
	    "end\n"
	    "if not parent or parent == '' then error('parent, or x and y, is required') end\n"
	    "-- the new parent holds it in a list of that name, at a place the list has (-1 the end), as for ring.create\n"
	    "local target = r2:getInstanceFromId(parent)\n"
	    "if not target then error('no parent instance ' .. tostring(parent)) end\n"
	    "local list = target[prop]\n"
	    "if not (r2.isTable(list) and not r2.isRefId(list) and list.Class == nil) then\n"
	    "  error(tostring(parent) .. ' has no list ' .. tostring(prop) .. ' to move it into')\n"
	    "end\n"
	    "-- not below itself or one of its own values, which the service refuses, after the Position sent below would be applied\n"
	    "local up = target\n"
	    "while up do\n"
	    "  if up.InstanceId == inst.InstanceId then error('cannot move ' .. tostring(id) .. ' below itself, into ' .. tostring(parent)) end\n"
	    "  up = up.ParentInstance\n"
	    "end\n"
	    "-- within the list that holds it now, the service takes it out before it puts it back: the list has one place less\n"
	    "local within = false\n"
	    "for i = 0, list.Size - 1 do\n"
	    "  if r2.isTable(list[i]) and list[i].InstanceId == inst.InstanceId then within = true end\n"
	    "end\n"
	    "local last = within and list.Size - 1 or list.Size\n"
	    "if pos < -1 or pos > last then\n"
	    "  error('position ' .. tostring(pos) .. ' is not in ' .. tostring(parent) .. '.' .. tostring(prop) .. ', which has ' .. tostring(list.Size) .. ' items'\n"
	    "    .. (within and ' with the moved one' or '') .. ': -1 (the end) or 0 to ' .. tostring(last))\n"
	    "end\n"
	    "r2.requestNewAction('Move ' .. tostring(id))\n"
	    "-- it keeps its place in the world, as grouping and ungrouping keep it (r2:group, r2:ungroup): its Position, relative to\n"
	    "-- its parent, is made relative to the new one first; a Position that does not inherit its parent's (InheritPos 0: the\n"
	    "-- zones of herds, camps and trigger features) is in the world already, and requestMakePosRelativeTo tests only nil\n"
	    "if inst.requestMakePosRelativeTo and r2.isTable(inst.Position) and inst.InheritPos ~= 0 then inst:requestMakePosRelativeTo(target) end\n"
	    "r2.requestMoveNode(id, '', -1, parent, prop, pos)\n"
	    "return { id = id, parent = parent, property = prop, requested = true }\n",
	    args);
}

void methodRingUndo(CCtrlCall &call)
{
	if (!requireEdition(call))
		return;
	bool redo = call.getArgs().get("redo").getBool(false);
	CAHManager::getInstance()->runActionHandler(redo ? "r2ed_redo" : "r2ed_undo", nullptr, std::string());
	CJsonValue args = CJsonValue::array();
	ringLua(call, "return { can_undo = r2:canUndo(), can_redo = r2:canRedo() }\n", args);
}

void methodRingSelect(CCtrlCall &call)
{
	if (!requireEdition(call))
		return;
	CJsonValue args = CJsonValue::array();
	args.push(call.getArgs().get("id"));
	ringLua(call,
	    "local id = ...\n"
	    "if id ~= '' and not r2:getInstanceFromId(id) then error('no instance ' .. tostring(id)) end\n"
	    "r2:setSelectedInstanceId(id)\n"
	    "return { selected = r2:getSelectedInstanceId() }\n",
	    args);
}

} // anonymous namespace

void registerGameMethods(CCtrlEndpoint &endpoint)
{
	endpoint.addMethod(new CCtrlFunctionMethod("web.list", "The in-game browser views: id, URL, title, whether a page is loading, whether it is shown. all includes the hidden ones.",
	    CCtrlSchema().boolean("all", "Also the views that are not shown."), true, methodWebList));
	endpoint.addMethod(new CCtrlFunctionMethod("web.page",
	    "The page of a browser view: as text with its links (href and text, the first 1000; links_truncated says when there are more), or as HTML. max_bytes bounds it (64 KB by default) and "
	    "truncated says so. Links with ah: or lua: run interface actions; click them in the view (ui.tree with contains, then ui.click) rather than browsing them.",
	    CCtrlSchema()
	        .str("id", "The view, as web.list gives it.", true)
	        .choice("format", "text|html", "text with links (the default) or html.")
	        .integer("max_bytes", "At most this much text."),
	    true, methodWebPage));
	endpoint.addMethod(new CCtrlFunctionMethod("web.browse", "Opens an address in a browser view, as following a link does.",
	    CCtrlSchema().str("id", "The view.", true).str("url", "The address.", true), false, methodWebBrowse));
	endpoint.addMethod(new CCtrlFunctionMethod("web.back", "Goes back in a browser view, or reloads it with refresh.",
	    CCtrlSchema().str("id", "The view.", true).boolean("refresh", "Reload instead."), false, methodWebBack));
	endpoint.addMethod(new CCtrlFunctionMethod("inventory.list",
	    "The items of an inventory, from the database the server fills: slot, sheet, name, quantity, quality. equip and hand hold where the item is in the bag: their items have place and "
	    "bag_slot, and the item of that bag slot. guild is the guild's bank. Equip, move and drop by ui.drag between the slots of the windows, or by their action handlers.",
	    CCtrlSchema().choice("inventory", "bag|equip|hand|pack_animal0|pack_animal1|pack_animal2|pack_animal3|room|temp|share|guild", "Which (bag by default)."), true, methodInventory));
	endpoint.addMethod(new CCtrlFunctionMethod("missions.list",
	    "The journal, the character's missions (journal 'character', index 0 to 14) and the group's, the team's missions and then the guild's (journal 'group', index 15 and on): each with its "
	    "index (the one CSR commands take), title, detail, finished (0 under way, 1 succeeded, 2 failed; state says it in words), goals and compass targets. strings_arrived false means some "
	    "text is on its way: ask again.",
	    CCtrlSchema(), true, methodMissions));
	// the ring methods check their arguments against their schemas (newMethod): an argument name they do not have, or a
	// value of another type, is refused rather than taken for one not given
	endpoint.addMethod(newMethod("ring.state",
	    "The scenario editor: its mode (edition, test, dm, animation...) and the index of the current act (0 the permanent act); in edition mode also the "
	    "scenario and current act, the selection, and whether undo and redo can run.",
	    CCtrlSchema(), true, methodRingState));
	endpoint.addMethod(newMethod("ring.tree",
	    "The scenario tree, or the tree below an instance (id): every property of every instance, with InstanceId and Class; arrays of the editor count from 0. depth (4 by default) and max (3000 items) bound it.",
	    CCtrlSchema().str("id", "An instance id (the scenario by default).").integer("depth", "Levels to follow (4 by default).").integer("max", "Items at most (3000 by default)."), true,
	    methodRingTree));
	// the editor sends its requests to the server, which applies them and sends the change back to every editor
	static const char *const s_RingRequest = " The server applies the request a moment later, and ring.tree shows it from then on; a quit right after it can lose it.";
	// ring.set, ring.create, ring.erase and ring.move make their requests past the checks of the editor's windows and
	// tools (any property, class or value, an instance the tools would not delete or move, any parent): privileged, so
	// they run only in a client whose content is trusted. The co-editors get what they request.
	ICtrlMethod *ringSet = newMethod("ring.set",
	    std::string("Sets a property of an instance through the editor's request, as its property windows do: it reaches the server and the co-editors, and can be undone. "
	                "The value is checked against the type the instance's class declares for the property (a number, a string, a reference); a sub-object or a list is "
	                "not replaced, and nothing else is tested (a Position is written where it is given).")
	        + s_RingRequest,
	    CCtrlSchema().str("id", "The instance id.", true).str("property", "The property name.", true).any("value", "The value: a string, a number, or {ref = id} for a reference to an instance.", true), false, methodRingSet);
	ringSet->setPrivileged(true);
	endpoint.addMethod(ringSet);
	endpoint.addMethod(newMethod("ring.palette",
	    "The elements of the editor's palette (NPCs, creatures, objects): id and name, filtered by a word or an id prefix such as palette.entities.creatures. ring.create takes the id as base.",
	    CCtrlSchema().str("contains", "Only ids or names containing this.").str("prefix", "Only ids starting with this.").integer("max", "At most this many (100 by default)."), true,
	    methodRingPalette));
	// the point limits come from the drawing tool's own constants; the group radius is r2.MaxGroupRadius, which only the
	// editor's Lua holds
	ICtrlMethod *ringCreate = newMethod("ring.create",
	    std::string("Creates an instance in the scenario through the editor's request, as its tools do: an entity (an NPC, a creature, an object) from a palette element (base, as ring.palette lists them), "
	                "the way the palette makes it (a custom NPC, such as a civilian, with a random look and name of its people: set SheetClient, the look properties and Name in values); a feature by class (TalkTo, Timer, ZoneTrigger...) as its own createComponent makes it at values.Position, with args after x and y "
	                "for the ones that take more (BanditCamp, Fauna, Counter, Timer, Creature); other components by class (NpcGrpFeature, Region, Road, ChatSequence...); a group, a region and a route "
	                "are named as the editor names them. values sets properties first (Name, Position {x, y, z}, Angle...), each of the type its class declares: {ref = id} is a "
	                "reference to an instance, an object with Class a new component of that class built with its own values, an object without Class fills that sub-object (Event, Action), "
	                "and an array appends to that list (Actions, Points, Components), so an event handler comes with its steps and a group with its members in one request. The items of "
	                "a list the editor fills with components (Points, Components, Actions, Conditions, Activities, ChatSequences, Features, Acts, Ghosts) are objects with Class; a "
	                "region's Points take RegionVertex, a route's WayPoint, a group's Components the NPCs and creatures grouping takes (no object and no plant, by class or by "
	                "SheetClient as the editor's isBotObject and isPlant test them, all of the Category and SubCategory of its leader), and so does the list a single component "
	                "goes into. It goes where the editor's tools put it unless parent says otherwise: an entity into the current act's default feature, an object, a "
	                "region and a route into the permanent act's; a feature, a group and a dialog into the current act's Features. The parent must hold a list of that property, and "
	                "position must be one it has (-1 the end). What stands in the world needs values.Position. A position off the ground the AI can use (the tops of root mounds and "
	                "pillars, cliffs, the rim) is refused where the editor's tools refuse it: what stands in the world, a feature or a dialog, an entity given in the values of a "
	                "component that is not a feature (a group's members), a point of a region or route, and the zones of a ZoneTrigger, BanditCamp or Fauna (inside and at every "
	                "point); the outline of a region, which the drawing tool refuses when it crosses itself, is not tested. Each of these positions given without z stands on the ground: "
	                "its z is the ground's height at its place in the world, relative to what holds it. ")
	        + toString("A route has %u to %u points and a region %u to %u, as the drawing and new vertex tools give them: fewer or more in values.Points (left out, it holds "
	                   "none), or a point added to a region or a route that has %u, is refused. ",
	            (uint)R2::CToolDrawPrim::RoadMinNumPoints, (uint)R2::CToolDrawPrim::PrimMaxNumPoints, (uint)R2::CToolDrawPrim::RegionMinNumPoints,
	            (uint)R2::CToolDrawPrim::PrimMaxNumPoints, (uint)R2::CToolDrawPrim::PrimMaxNumPoints)
	        + "The members of a group stay within r2.MaxGroupRadius (20 m in the editor's Lua) of each other, as grouping and the move tool keep them: members given in values "
	          "farther apart, or an entity put below a group farther from one of its members, are refused. It is refused when the act has no room left, as the palette tools "
	          "test it: an entity and the entities in values count on the AI quota, objects on the static one. A feature needs room on the AI quota for what it makes (a bandit "
	          "camp its bandits, a fauna feature its herds and itself, the others one), as most feature tools test it; HuntTask, KillNpc, LootSpawner, ProximityDialog, Quest, "
	          "SceneryObjectRemover, TargetMob and VisitZone only need the smaller of the two quotas not spent, as their tools test it; TalkTo, RewardProvider, NpcInteraction, "
	          "SceneryObjectInteraction and SceneryObjectInteractionTaskStep, whose tools warn but never refuse, are refused on the AI quota all the same (Quest and "
	          "NpcInteraction exist only when the editor loads its dynamic features, R2EDLoadDynamicFeatures). Returns its instance id."
	        + s_RingRequest,
	    CCtrlSchema()
	        .str("base", "A palette element id, for an entity.")
	        .str("class", "The class: for a component that is not an entity, or to override the palette's.")
	        .str("parent", "The parent instance id; by default where the editor's tools put it (see above).")
	        .str("property", "The property of the parent it goes in: by default Points for a point (RegionVertex, WayPoint) below a region or a route, Features for a feature, a group "
	                         "or a dialog, Components for the others.")
	        .integer("position", "Where in it (-1, the end, by default).")
	        .object("values", "Properties to set on it first.")
	        .any("args",
	            "For a feature, the arguments of its createComponent after x and y, within the bounds of its form: BanditCamp the count of bandits (1 to 12), the race "
	            "(0 to 3: Fyros, Matis, Tryker, Zorai), the level (0 to 4: 20, 70, 120, 170, 220, one the palette has bandits of and the Ring access allows); Fauna the count "
	            "of carnivores (1 to 12), a carnivore's palette id as the Fauna form lists them (r2.Palette.Entries.creature.creatures_predators, allowed by the Ring access, of "
	            "the form's ecosystems and of level 1 to 250), their name, and the same three for the herbivores (creatures_passive); Counter the value it starts from (at least 1) and the "
	            "value it triggers at (at least 0); Timer the seconds (0 to 999), the minutes (0 to 120) and cyclic (0 or 1), each optional; Creature the count (1 to 12), "
	            "a creature's palette id, their name, the ids of its sleep and food regions. The other features take none."),
	    false, methodRingCreate);
	ringCreate->setPrivileged(true);
	endpoint.addMethod(ringCreate);
	ICtrlMethod *ringErase = newMethod("ring.erase", std::string("Deletes an instance through the editor's request.") + s_RingRequest,
	    CCtrlSchema().str("id", "The instance id.", true), false, methodRingErase);
	ringErase->setPrivileged(true);
	endpoint.addMethod(ringErase);
	ICtrlMethod *ringMove = newMethod("ring.move",
	    std::string("Moves an instance, through the editor's requests: to x, y of its Position, which is relative to its parent (the world for what an act holds, the origin of "
	                "the points of a drawn region or route, the group for its members), or below another parent. With x and y it sets z too, as the move tool writes the whole "
	                "Position: what stands in the world goes on the ground at its new place; a region, a route or a group is raised or lowered so that the first of its points or "
	                "members that lands on that ground stands on it, the others keeping their heights relative to it. Refused as the editor's move tool refuses it: a place off "
	                "the ground the AI can use for what stands in the world, and for every point of a region or route and every member of a group when they all were on that "
	                "ground before (otherwise one of them at least must land on it); a member of a group put farther than r2.MaxGroupRadius (20 m in the editor's Lua) from another "
	                "member of its group. The move tool also refuses a point of a region moved where the outline would cross itself; that test is not made here. A move below "
	                "another parent keeps the instance's place in the world, as grouping and ungrouping do (its Position is made relative to the new parent first); the new parent "
	                "must hold a list of that property, and position must be one the list has (-1 the end; within the list that holds it now, one fewer, as it leaves the list "
	                "first). Nothing else is tested there: neither the kinds and counts of Points, nor the kinds, categories and radius of a group, nor the ground. x and y go "
	                "together and exclude parent, property and position (an empty parent or property, and position -1, count as not given).")
	        + s_RingRequest,
	    CCtrlSchema()
	        .str("id", "The instance id.", true)
	        .number("x", "x of its Position, relative to its parent.")
	        .number("y", "y of its Position, relative to its parent.")
	        .str("parent", "Or a new parent instance id.")
	        .str("property", "Its property (Components by default).")
	        .integer("position", "Where in it (-1 by default)."),
	    false, methodRingMove);
	ringMove->setPrivileged(true);
	endpoint.addMethod(ringMove);
	endpoint.addMethod(newMethod("ring.undo", "Undoes the last editing action, or redoes with redo, as the editor's buttons do; returns whether more can be undone and redone.",
	    CCtrlSchema().boolean("redo", "Redo instead."), false, methodRingUndo));
	endpoint.addMethod(newMethod("ring.select", "Selects an instance in the editor, or none with an empty id.", CCtrlSchema().str("id", "The instance id.", true), false,
	    methodRingSelect));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_game.cpp */
