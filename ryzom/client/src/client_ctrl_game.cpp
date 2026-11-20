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
// permanent act), and the other fields are left out; in edition mode that index means nothing
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
                                "  if act then r.current_act = act.InstanceId r.current_act_name = act.Name end\n"
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
// list of that name (Actions, Components, Conditions), as the editor's own code builds a handler with its steps.
const char *const s_RingValues = "local function isRef(v)\n"
                                 "  if type(v) ~= 'table' or type(v.ref) ~= 'string' then return false end\n"
                                 "  for k in pairs(v) do if k ~= 'ref' then return false end end\n"
                                 "  return true\n"
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
                                 "local fill\n"
                                 "local function toRing(v, path, ptype)\n"
                                 "  if type(v) ~= 'table' then\n"
                                 "    if ptype == 'RefId' and type(v) == 'string' then return r2.RefId(v) end\n"
                                 "    return v\n"
                                 "  end\n"
                                 "  if isRef(v) then return r2.RefId(v.ref) end\n"
                                 "  if type(v.Class) ~= 'string' then error(path .. ': an object here needs Class (a new component) or ref (a reference)') end\n"
                                 "  local c = newComponent(v.Class, path)\n"
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
                                 "          for i, item in ipairs(v) do table.insert(cur, toRing(item, where .. '[' .. i .. ']')) end\n"
                                 "        elseif next(v) ~= nil then\n"
                                 "          if cur.Class == nil then error(where .. ': a list, which takes an array of components, not an object') end\n"
                                 "          fill(cur, v, where)\n"
                                 "        end\n"
                                 "      else\n"
                                 "        inst[k] = toRing(v, where, propType(inst, k))\n"
                                 "      end\n"
                                 "    end\n"
                                 "  end\n"
                                 "end\n";

void methodRingSet(CCtrlCall &call)
{
	if (!requireEdition(call))
		return;
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
	    "if not r2:getInstanceFromId(id) then error('no instance ' .. tostring(id)) end\n"
	    "if type(value) == 'table' and not isRef(value) then error('value: a string, a number or {ref = id}; ring.create makes components') end\n"
	    "r2.requestNewAction('Set ' .. tostring(prop))\n"
	    "r2.requestSetNode(id, prop, toRing(value, 'value', propType(r2:getInstanceFromId(id), prop)))\n"
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
	// an entity of the scenario is made as the palette tool makes it (CToolCreateEntity): the class the palette
	// element names (a creature's is NpcCreature or NpcPlant), Base the element, the element's name; without a base
	// the editor cannot display it. A custom NPC (the civilians) gets the editor's random look of its element. A
	// feature (TalkTo, Timer, ZoneTrigger...) is made by its own createComponent at x, y, as its palette tool makes it,
	// with the parts it needs (a zone, a Base); it and a dialog (ChatSequence, which has no createComponent) go into
	// an act's Features, as the palette puts them.
	ringLua(call, s_RingValues,
	    "local class, base, parent, prop, pos, values, extra, nextra = ...\n"
	    "local botObject = string.sub(base, 1, 27) == 'palette.entities.botobjects'\n"
	    "local creator\n"
	    "if class ~= '' then\n"
	    "  for _, f in pairs(r2.Features) do\n"
	    "    if type(f) == 'table' and type(f.Components) == 'table' and f.Components[class] ~= nil then\n"
	    "      creator = f.Components[class].createComponent or f.createComponent\n"
	    "      if creator then break end\n"
	    "    end\n"
	    "  end\n"
	    "end\n"
	    "if creator and base ~= '' then error(class .. ' is a feature: it is made by its class, without base') end\n"
	    "local actLevel = creator ~= nil or class == 'ChatSequence'\n"
	    "local parentId, property = parent, prop\n"
	    "if parentId == '' then\n"
	    "  if actLevel then\n"
	    "    parentId = r2.Scenario:getCurrentAct().InstanceId\n"
	    "  else\n"
	    "    local act = botObject and r2.Scenario:getBaseAct() or r2.Scenario:getCurrentAct()\n"
	    "    parentId = act.Features[0].InstanceId\n"
	    "  end\n"
	    "end\n"
	    "if property == '' then property = actLevel and 'Features' or 'Components' end\n"
	    "if not r2:getInstanceFromId(parentId) then error('no parent instance ' .. tostring(parentId)) end\n"
	    "if base ~= '' then\n"
	    "  local act, baseAct = r2.Scenario:getCurrentAct(), r2.Scenario:getBaseAct()\n"
	    "  if botObject then\n"
	    "    if r2.getMaxStaticObjects() - baseAct:getLocalStaticCost() - (act ~= baseAct and act:getLocalStaticCost() or 0) < 1 then error('no room left for an object in the scenario') end\n"
	    "  elseif r2.getMaxNpcs() - baseAct:getLocalCost() - (act ~= baseAct and act:getLocalCost() or 0) < 1 then\n"
	    "    error('no room left for an entity in the scenario')\n"
	    "  end\n"
	    "elseif creator then\n"
	    "  -- the AI quota, as the palette tools check it (r2:checkAiQuota): a bandit camp needs its bandits, a fauna\n"
	    "  -- feature its herds and itself\n"
	    "  local need = 1\n"
	    "  if class == 'BanditCamp' then need = tonumber(extra[1]) or 1\n"
	    "  elseif class == 'Fauna' then need = (tonumber(extra[1]) or 0) + (tonumber(extra[4]) or 0) + 1 end\n"
	    "  local _, leftAi = r2:getLeftQuota()\n"
	    "  if leftAi < need then error('no room left in the act for ' .. tostring(need) .. ' more of its AI quota') end\n"
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
	    "local p = values.Position\n"
	    "local at = type(p) == 'table' and not isRef(p) and p.Class == nil and type(p.x) == 'number' and type(p.y) == 'number'\n"
	    "local inst\n"
	    "if creator then\n"
	    "  if not at then error('a feature is made at values.Position {x, y}') end\n"
	    "  inst = creator(p.x, p.y, (table.unpack or unpack)(extra, 1, nextra))\n"
	    "  if type(inst) ~= 'table' or inst.InstanceId == nil then error('the createComponent of ' .. class .. ' made nothing') end\n"
	    "  if p.z ~= nil then values.Position = { z = p.z } else values.Position = nil end\n"
	    "else\n"
	    "  if class == 'NpcCustom' then\n"
	    "    -- the palette tool copies the look of the entity it shows while placing; without one the editor's own\n"
	    "    -- random look of the element is taken, as a bandit camp makes its bandits, and a name of its people from\n"
	    "    -- the list the palette tool draws from (randomNPCName)\n"
	    "    inst = r2:randomNPC(base)\n"
	    "    local sex = isR2PlayerMale(inst.SheetClient) and r2.male or r2.female\n"
	    "    inst.Name = r2:randomNPCName(getR2PlayerRace(inst.SheetClient), sex)\n"
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
	    "  end\n"
	    "  -- what stands in the world (it has a Base) is put on the ground; a position below another is relative\n"
	    "  if at and p.z == nil then\n"
	    "    if (values.Base or inst.Base or '') ~= '' then p.z = r2:snapZToGround(p.x, p.y) else p.z = 0 end\n"
	    "  end\n"
	    "end\n"
	    "fill(inst, values, 'values')\n"
	    "if class == 'NpcCustom' and elt and (elt.SheetModel or '') ~= '' then\n"
	    "  -- the combat sheet that goes with the weapons it holds, as the palette tool sets it (r2:searchSheet)\n"
	    "  local sheet = r2:searchSheet(r2.getPropertyValue(inst, 'Equipment'), inst.WeaponRightHand, inst.WeaponLeftHand, r2.getPropertyValue(inst, 'Sheet'), elt.SheetModel)\n"
	    "  if sheet == '' then error('no combat sheet of ' .. base .. ' goes with the weapons it holds') end\n"
	    "  inst.Sheet = sheet\n"
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
	CJsonValue args = CJsonValue::array();
	args.push(call.getArgs().get("id"));
	args.push(call.getArgs().get("parent"));
	args.push(call.getArgs().get("property").isString() ? call.getArgs().get("property") : CJsonValue("Components"));
	args.push(CJsonValue((sint64)call.getArgs().get("position").getInt(-1)));
	args.push(call.getArgs().get("x").isNull() ? CJsonValue() : CJsonValue(x));
	args.push(call.getArgs().get("y").isNull() ? CJsonValue() : CJsonValue(y));
	ringLua(call,
	    "local id, parent, prop, pos, x, y = ...\n"
	    "local inst = r2:getInstanceFromId(id)\n"
	    "if not inst then error('no instance ' .. tostring(id)) end\n"
	    "if x and y then\n"
	    "  r2.requestNewAction('Move ' .. tostring(id))\n"
	    "  r2.requestSetNode(inst.Position.InstanceId, 'x', x)\n"
	    "  r2.requestSetNode(inst.Position.InstanceId, 'y', y)\n"
	    "  return { id = id, x = x, y = y, requested = true }\n"
	    "end\n"
	    "if not parent or parent == '' then error('parent, or x and y, is required') end\n"
	    "r2.requestNewAction('Move ' .. tostring(id))\n"
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
	endpoint.addMethod(new CCtrlFunctionMethod("ring.state",
	    "The scenario editor: its mode (edition, test, dm, animation...) and the index of the current act (0 the permanent act); in edition mode also the "
	    "scenario and current act, the selection, and whether undo and redo can run.",
	    CCtrlSchema(), true, methodRingState));
	endpoint.addMethod(new CCtrlFunctionMethod("ring.tree",
	    "The scenario tree, or the tree below an instance (id): every property of every instance, with InstanceId and Class; arrays of the editor count from 0. depth (4 by default) and max (3000 items) bound it.",
	    CCtrlSchema().str("id", "An instance id (the scenario by default).").integer("depth", "Levels to follow (4 by default).").integer("max", "Items at most (3000 by default)."), true,
	    methodRingTree));
	// the editor sends its requests to the server, which applies them and sends the change back to every editor
	static const char *const s_RingRequest = " The server applies the request a moment later, and ring.tree shows it from then on; a quit right after it can lose it.";
	// ring.set, ring.create, ring.erase and ring.move make their requests past the checks of the editor's windows and
	// tools (any property, class or value, an instance the tools would not delete or move, any parent): privileged, so
	// they run only in a client whose content is trusted. The co-editors get what they request.
	ICtrlMethod *ringSet = new CCtrlFunctionMethod("ring.set",
	    std::string("Sets a property of an instance through the editor's request, as its property windows do: it reaches the server and the co-editors, and can be undone.") + s_RingRequest,
	    CCtrlSchema().str("id", "The instance id.", true).str("property", "The property name.", true).any("value", "The value: a string, a number, or {ref = id} for a reference to an instance.", true), false, methodRingSet);
	ringSet->setPrivileged(true);
	endpoint.addMethod(ringSet);
	endpoint.addMethod(new CCtrlFunctionMethod("ring.palette",
	    "The elements of the editor's palette (NPCs, creatures, objects): id and name, filtered by a word or an id prefix such as palette.entities.creatures. ring.create takes the id as base.",
	    CCtrlSchema().str("contains", "Only ids or names containing this.").str("prefix", "Only ids starting with this.").integer("max", "At most this many (100 by default)."), true,
	    methodRingPalette));
	ICtrlMethod *ringCreate = new CCtrlFunctionMethod("ring.create",
	    "Creates an instance in the scenario through the editor's request, as its tools do: an entity (an NPC, a creature, an object) from a palette element (base, as ring.palette lists them), "
	    "the way the palette makes it (a custom NPC, such as a civilian, with a random look and name of its people: set SheetClient, the look properties and Name in values); a feature by class (TalkTo, Timer, ZoneTrigger...) as its own createComponent makes it at values.Position, with args after x and y "
	    "for the ones that take more (Timer, Counter, Fauna, BanditCamp); other components by class (Region, Road, ChatSequence...). values sets properties first (Name, Position {x, y, z}, Angle...): {ref = id} is a "
	    "reference to an instance, an object with Class a new component of that class built with its own values, an object without Class fills that sub-object (Event, Action), "
	    "and an array appends to that list (Actions, Points), so an event handler comes with its steps in one request. It goes below a parent: by default an entity "
	    "into the current act's default feature (an object into the permanent act's), a feature and a dialog into the current act's Features. A missing z of what "
	    "stands in the world is the ground's. Returns its instance id."
	        + std::string(s_RingRequest),
	    CCtrlSchema()
	        .str("base", "A palette element id, for an entity.")
	        .str("class", "The class: for a component that is not an entity, or to override the palette's.")
	        .str("parent", "The parent instance id.")
	        .str("property", "The property of the parent it goes in (Components, or Features for an act).")
	        .integer("position", "Where in it (-1, the end, by default).")
	        .object("values", "Properties to set on it first.")
	        .any("args", "For a feature, the arguments of its createComponent after x and y."),
	    false, methodRingCreate);
	ringCreate->setPrivileged(true);
	endpoint.addMethod(ringCreate);
	ICtrlMethod *ringErase = new CCtrlFunctionMethod("ring.erase", std::string("Deletes an instance through the editor's request.") + s_RingRequest,
	    CCtrlSchema().str("id", "The instance id.", true), false, methodRingErase);
	ringErase->setPrivileged(true);
	endpoint.addMethod(ringErase);
	ICtrlMethod *ringMove = new CCtrlFunctionMethod("ring.move", std::string("Moves an instance to x, y in the world, or below another parent, through the editor's requests.") + s_RingRequest,
	    CCtrlSchema()
	        .str("id", "The instance id.", true)
	        .number("x", "World x.")
	        .number("y", "World y.")
	        .str("parent", "Or a new parent instance id.")
	        .str("property", "Its property (Components by default).")
	        .integer("position", "Where in it (-1 by default)."),
	    false, methodRingMove);
	ringMove->setPrivileged(true);
	endpoint.addMethod(ringMove);
	endpoint.addMethod(new CCtrlFunctionMethod("ring.undo", "Undoes the last editing action, or redoes with redo, as the editor's buttons do; returns whether more can be undone and redone.",
	    CCtrlSchema().boolean("redo", "Redo instead."), false, methodRingUndo));
	endpoint.addMethod(new CCtrlFunctionMethod("ring.select", "Selects an instance in the editor, or none with an empty id.", CCtrlSchema().str("id", "The instance id.", true), false,
	    methodRingSelect));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_game.cpp */
