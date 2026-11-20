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

// The chat methods of the control endpoint: chat.send, chat.channels. What the chat shows arrives as
// events: chat (a line of a channel or a tell, with its sender) and system_info; ctrl.events with the
// kinds chat and system_info reads them from a cursor.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/gui/db_manager.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/ucstring.h"

#include "client_cfg.h"
#include "client_chat_manager.h"
#include "string_manager_client.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

extern CClientChatManager ChatMngr;

namespace ClientCtrl {

namespace {

void methodSend(CCtrlCall &call)
{
	if (!requireInGame(call))
		return;
	if (ClientCfg.Local)
	{
		call.fail("the client runs in local mode: there is no shard to send a line to");
		return;
	}
	const CJsonValue &args = call.getArgs();
	const std::string channel = args.get("channel").getString("say");
	const std::string &text = args.get("text").getString();
	if (text.empty())
	{
		call.fail("text is required");
		return;
	}
	if (text[0] == '/')
	{
		// a chat command would run as one: that is run_command's business, with its arguments as a list
		call.fail("the text starts with '/': it would run as a command; use run_command for commands, or start the text otherwise");
		return;
	}
	// the client cuts a longer line to what the shard takes: it is refused here rather than sent cut
	uint length = (uint)ucstring(text).size();
	if (length > (uint)CChatGroup::MaxClientTextLength)
	{
		call.fail(toString("the text is %u UTF-16 units long, and the shard takes at most %u: split it", length, (uint)CChatGroup::MaxClientTextLength));
		return;
	}
	// the chat window's own sending (CChatTargetFilter::msgEntered) leaves the chat mode on the channel
	static const char *const s_ModeNote = "the chat mode of the client is now this channel, as the chat window would leave it; the line comes back as a chat event once the shard sends it";
	const char *note = s_ModeNote;
	CJsonValue r = CJsonValue::object();
	r.set("channel", channel);
	if (channel == "tell")
	{
		const std::string &to = args.get("to").getString();
		if (to.empty())
		{
			call.fail("to is required for a tell");
			return;
		}
		// as the client cuts it
		if (to.size() > 255)
		{
			call.fail("to is longer than a player name can be");
			return;
		}
		ChatMngr.tell(to, text);
		r.set("to", to);
		note = "the shard does not send a tell back: no chat event follows it, and the client's tell window does not show it";
	}
	else if (channel == "team")
	{
		// without a team the client sends nothing (CClientChatManager::chat)
		CCDBNodeLeaf *present = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GROUP:0:PRESENT", false);
		if (present == nullptr || !present->getValueBool())
		{
			call.fail("the character is in no team: the client sends no team line then");
			return;
		}
		ChatMngr.chat(text, true);
		note = "the line comes back as a chat event of the team once the shard sends it; the chat mode of the client stays as it was";
	}
	else if (channel == "dyn")
	{
		sint64 index = args.get("index").getInt(-1);
		TChanID id = index >= 0 && index < CChatGroup::MaxDynChanPerPlayer ? ChatMngr.getDynamicChannelIdFromDbIndex((uint32)index) : CEntityId::Unknown;
		if (id == CEntityId::Unknown)
		{
			call.fail("no dynamic channel at that index; chat.channels lists them");
			return;
		}
		ChatMngr.setChatMode(CChatGroup::dyn_chat, id);
		ChatMngr.chat(text);
		r.set("index", index);
	}
	else
	{
		// the channels the chat window writes to; the others of the chat groups (around, civilization, territory, the
		// dynamic ones by type) are not sent to by a player this way
		static const char *const s_Channels[] = { "say", "shout", "region", "universe", "guild" };
		bool known = false;
		for (uint i = 0; i < sizeof(s_Channels) / sizeof(s_Channels[0]) && !known; ++i)
			known = channel == s_Channels[i];
		if (!known)
		{
			call.fail("channel is say, shout, region, universe, guild, team, tell (with to) or dyn (with index)");
			return;
		}
		ChatMngr.setChatMode(CChatGroup::stringToGroupType(channel));
		ChatMngr.chat(text);
	}
	r.set("text", text);
	r.set("sent", getCtrlTimeMs());
	r.set("note", note);
	call.reply(r);
}

void methodChannels(CCtrlCall &call)
{
	CJsonValue dyn = CJsonValue::array();
	for (uint i = 0; i < CChatGroup::MaxDynChanPerPlayer; ++i)
	{
		TChanID id = ChatMngr.getDynamicChannelIdFromDbIndex(i);
		if (id == CEntityId::Unknown)
			continue;
		CJsonValue c = CJsonValue::object();
		c.set("index", i);
		c.set("id", id.toString());
		uint32 nameId = ChatMngr.getDynamicChannelNameFromDbIndex(i);
		std::string name;
		// a dynamic string: the client asks for its parts itself as it arrives, so this asks the shard for nothing
		bool arrived = nameId == 0 || STRING_MANAGER::CStringManagerClient::instance()->getDynString(nameId, name);
		c.set("name", name);
		if (!arrived)
			c.set("name_arrived", false);
		dyn.push(c);
	}
	CJsonValue r = CJsonValue::object();
	CJsonValue fixed = CJsonValue::array();
	static const char *const s_Fixed[] = { "say", "shout", "region", "universe", "guild", "team", "tell" };
	for (uint i = 0; i < sizeof(s_Fixed) / sizeof(s_Fixed[0]); ++i)
		fixed.push(CJsonValue(s_Fixed[i]));
	r.set("channels", fixed);
	r.set("dynamic", dyn);
	r.set("reading", "ctrl.events with kinds chat and system_info, from a cursor (get_events in nel_mcp)");
	call.reply(r);
}

} // anonymous namespace

void registerChatMethods(CCtrlEndpoint &endpoint)
{
	endpoint.addMethod(new CCtrlFunctionMethod("chat.send",
	    "Sends a line to a chat channel without the chat box: say (around), shout, region, universe, guild, team, tell (to a player), or dyn (a dynamic channel by index, such as a DM or Ring channel). "
	    "A text starting with '/' is refused: commands go through run_command; so is a text longer than the shard takes (255 UTF-16 units), a team line without a team, and anything in local "
	    "mode. A line to a channel comes back as a chat event; a tell does not, and the tell window does not show it.",
	    CCtrlSchema()
	        .choice("channel", "say|shout|region|universe|guild|team|tell|dyn", "The channel (say by default).")
	        .str("text", "The line.", true)
	        .str("to", "For tell: the player.")
	        .integer("index", "For dyn: the index of the dynamic channel (chat.channels)."),
	    false, methodSend));
	endpoint.addMethod(new CCtrlFunctionMethod("chat.channels", "The channels a line can go to, and the dynamic channels the character is in now (index, id, name).", CCtrlSchema(), true,
	    methodChannels));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_chat.cpp */
