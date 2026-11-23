#!/bin/env python3
#############################################
#  _______________________________
#  \______   \__    ___/\____    /
#   |       _/ |    |     /     /
#   |    |   \ |    |    /     /_
#   |____|_  / |____|   /_______ \
#          \/                   \/
#
# RyTransZulip - with delicious M.A.R.G.U.E.Z
# M.A.R.G.U.E.Z (Make Awesome all Ryzom's Gossips with the Unreasonable Empowerment of Zulip... and a touch of Deepl :D)
# Copyright (C) 2025 Nuneo (nuno@troispetits.net)
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# -== Zulip Dispatcher ==-
#
# This script dispatche messages to Zulip chat
#


import os
import sys
import time
import html
import re
import traceback
import urllib.parse
import urllib.request

from pymemcache.exceptions import MemcacheIllegalInputError
from zulip_service import ZulipService
from quote_bridge import render_quote

ALL_LANGS = ["wk", "de", "en", "es", "fr", "ru"]
flags = {
	"en": ":flag_united_kingdom:",
	"es": ":flag_spain:",
	"de": ":flag_germany:",
	"fr": ":flag_france:",
	"ru": ":flag_russia:",
}
emojis = {
	"en": "🇬🇧",
	"es": "🇪🇸",
	"de": "🇩🇪",
	"fr": "🇫🇷",
	"ru": "🇷🇺",
}

class ZulipDispatcher(ZulipService):
	def __init__(self):
		super().__init__()
		self.name = "ZulipDispatcher"
		self.version = "1.2"
		self.scriptfile = __file__
		self.stats = {"messages": 0}
		self.player_subscriptions = set()

	def getLang(self, lang):
		return ":"+lang+":"

	def getRealChannel(self, channel):
		channels = {
			"FACTION_RF" : "Forge"
		}

		if channel in channels:
			return channels[channel]
		return channel

	def quotePrefix(self, message):
		chat = message.chat
		if not chat or not chat.get("quote_id") or not chat.get("quote_text"):
			return ""
		zulip_id = self.getChatZulipId(chat["quote_id"])
		stream_id = recipients = None
		if zulip_id:
			result = self.zulip.call_endpoint(url=f"messages/{zulip_id}", method="GET",
				request={"apply_markdown": False})
			if result.get("result") == "success":
				original = result["message"]
				if original["type"] == "stream":
					stream_id = original["stream_id"]
				else:
					recipients = [user["id"] for user in original["display_recipient"]]
		author = chat.get("quote_author", "")
		if author.startswith("~"):
			author = author[1:]
		return render_quote(author, chat["quote_text"], self.base_url,
			zulip_id=zulip_id, stream_id=stream_id, recipients=recipients)

	def sendMessage(self, m):
		message_type = "stream"
		user = m.sender
		if not user:
			print("Message without sender, skipped")
			return None
		# Normalize potential Zulip-style upload markdown to plain URLs before sending
		clean_text = self.convert_zulip_upload_links(m.text)
		content = clean_text if clean_text is not None else ""
		if m.channel == "player":
			message_type = "private"
			if not m.channel_id:
				print(f"Private message from {user} without recipient, skipped")
				return -1
			# A player of another shard is named Name(Shard) in the game
			channel = re.sub(r"\([^)]*\)$", "", m.channel_id).lower()+"@ig.ryzom.com"
		elif m.channel == "dyn":
			channel = self.getRealChannel(m.channel_id)
			if channel.startswith(u"❇️ League_"):
				print(f"Bad channel {channel}")
				return None
		else:
			channel = self.getRealChannel(m.channel)
		if not channel:
			print(f"Message from {user} without channel, skipped")
			return -1
		sender = self.zulip.ensurePlayer(user)
		if not sender:
			print(f"No Zulip account for {user}, message skipped")
			return -1
		request = {
			"type": message_type,
			"to": channel,
			"topic": "",
			"content": content,
			"sender": sender,
			"local_id": "ryzom-ig",
			"queue_id": self.getZulipQueueId(),
		}
		if message_type == "private":
			# Zulip only lets the bot write a private message in the name of
			# another user when the message is flagged as forged
			request["forged"] = "true"

		try:
			if message_type == "stream" and (channel, sender) not in self.player_subscriptions:
				subscription = self.zulip.addIngameSubscription(channel, [sender])
				if isinstance(subscription, dict) and subscription.get("result") == "success":
					self.player_subscriptions.add((channel, sender))
				else:
					print("Error subscribing", sender, "to", channel, subscription)
			result = self.zulip.send_message(request)
			if message_type == "stream" and isinstance(result, dict) and result.get("code") == "STREAM_DOES_NOT_EXIST":
				self.zulip.addIngameSubscription(channel)
				result = self.zulip.send_message(request)
		except (ValueError, TypeError, KeyError, IndexError, AttributeError) as e:
			print("Invalid message", e)
			return None
		except Exception as e:
			print("Error sending message", e)
			return -1
		if not isinstance(result, dict):
			print("Error sending message, invalid response", result)
			return None
		if result.get("result") == "success" and "id" in result:
			log_channel =  channel.split(" ")[0]
			log_content =  "".join([ s[0] for s in  content.split() ])
			print(f"💬 {message_type} to {log_channel} with {log_content} = {result['id']}")
			return result["id"]
		print("Error sending message", result.get("code", ""), result.get("msg", ""))
		return -1 if self.retryError(result) else None

	def sendReaction(self, m):
		reaction = m.chat["reaction"]
		# The bot reacts once for all players who chose the same emoji.
		key = "Chat-Reaction-"+reaction["message_id"]+"-"+reaction["emoji"]
		try:
			zulip_id = self.getChatZulipId(reaction["message_id"])
			players = set(self.client.get(key) or ()) if zulip_id else None
		except MemcacheIllegalInputError as e:
			# An emoji name memcached takes no key from (blanks, non-ASCII) is no Zulip emoji either
			print("Unusable reaction emoji", e)
			return None
		except Exception as e:
			# memcached not reachable: tried again like a failed write
			print("Error reading reaction", e)
			return False
		if not zulip_id:
			return None
		shown = bool(players)
		if reaction["remove"]:
			players.discard(m.sender)
		else:
			players.add(m.sender)
		if shown != bool(players):
			request = {"message_id": int(zulip_id), "emoji_name": reaction["emoji"]}
			try:
				result = self.zulip.add_reaction(request) if players else self.zulip.remove_reaction(request)
			except Exception as e:
				print("Error sending reaction", e)
				return False
			if not isinstance(result, dict):
				print("Error sending reaction, invalid response", result)
				return None
			# The answer when Zulip had applied the call and its reply was lost
			applied = result.get("code") == ("REACTION_ALREADY_EXISTS" if players else "REACTION_DOES_NOT_EXIST")
			if result.get("result") != "success" and not applied:
				print("Error sending reaction", result.get("code", ""), result.get("msg", ""))
				return False if self.retryError(result) else None
			print(f"{'➕' if players else '➖'} {reaction['emoji']} on {zulip_id}")
		# Stored last and confirmed, so a retry after a failed call or write sends the reaction again
		try:
			return self.client.set(key, list(players), 24*60*60, noreply=False)
		except Exception as e:
			print("Error storing reaction", e)
			return False

	def sendTranslation(self, message_id, m):
		if message_id > 0 and m.translation and m.source_lang and m.translated_lang:
			# Normalize Zulip-style upload markdown in translation as well
			clean_translation = self.convert_zulip_upload_links(m.translation)
			request = {
				"message_id": message_id,
			}
			try:
				request["content"] = ("<["+m.translated_lang+"]>"+flags.get(m.source_lang.lower(), "")+" "+self.quotePrefix(m)+
					(clean_translation if clean_translation is not None else ""))
				result = self.zulip.update_message(request)
			except (ValueError, TypeError, KeyError, IndexError, AttributeError) as e:
				print("Invalid translation", e)
				return None
			except Exception as e:
				print("Error update message", e)
				return False
			if not isinstance(result, dict):
				print(f"Error update message {message_id}, invalid response", result)
				return None
			if result.get("result") != "success":
				print("Error updating translation", result.get("code", ""), result.get("msg", ""))
				return False if self.retryError(result) else None
			print(f"{emojis.get(m.translated_lang.lower(), m.translated_lang)} {message_id}")
		return True

	def run(self):
		last_ids = {}
		for lang in ALL_LANGS:
			last_ids[lang] = self.getLastChatLangID(lang)
		while True:
			chat_id = self.getLastChatID()
			for lang in ALL_LANGS:
				for i in range(last_ids[lang]+1, chat_id+1):
					result = True
					try:
						message = self.getRyzomMessage(i)
						if message and message.chat.get("reaction"):
							if lang == "wk" and message.source != "zulip":
								result = self.sendReaction(message)
						elif message and message.channel not in ("say", "shout", "arround", "region", "dyn", "team") and (message.translated_lang or "").lower() == lang:
							if lang == "wk":
								if message.source != "zulip":
									message_id = self.sendMessage(message)
									if message_id == -1:
										result = False
									elif message_id is None:
										result = None
									else:
										self.addZulipMessageId(i, message_id)
										if message.chat.get("message_id"):
											self.addChatMessageId(message.chat["message_id"], message_id)
							else:
								if message.source == "zulip":
									message_id = message.chat.get("zulip_id")
								else:
									message_id = (self.getChatZulipId(message.chat["message_id"])
										if message.chat.get("message_id") else self.getZulipMessageId(message.source_message_id))
								if message_id:
									result = self.sendTranslation(int(message_id), message)
								elif message.source != "zulip" and last_ids["wk"] < message.source_message_id:
									result = False
								else:
									print("No Zulip ID for translation", i, lang)
									result = None
					except Exception:
						# Skip the message instead of crashing or retrying it forever
						print(f"Error dispatching message {i} ({lang}):\n{traceback.format_exc()}")
						result = None
					if result is False:
						break
					if result is None:
						print("Skipping Zulip message", i, lang)
					last_ids[lang] = i
					self.setLastChatLangID(lang, i)
			time.sleep(0.1)


if __name__ == "__main__":
	zulip = ZulipDispatcher()
	zulip.register()
	zulip.run()
