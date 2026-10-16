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
import json
import traceback
import urllib.parse
import urllib.request

from zulip_service import ZulipService

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

	def getLang(self, lang):
		return ":"+lang+":"

	def getRealChannel(self, channel):
		channels = {
			"FACTION_RF" : "Forge"
		}

		if channel in channels:
			return channels[channel]
		return channel

	def sendMessage(self, m):
		message_type = "stream"
		user = m.sender
		if not user:
			print("Message without sender, skipped")
			return -1
		# Normalize potential Zulip-style upload markdown to plain URLs before sending
		clean_text = self.convert_zulip_upload_links(m.text)
		content = user[0].upper()+user[1:]+":"+(clean_text if clean_text is not None else "")
		if m.channel == "player":
			message_type = "private"
			if not m.channel_id:
				print(f"Private message from {user} without recipient, skipped")
				return -1
			channel = m.channel_id.lower()+"@ig.ryzom.com"
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
		request = {
			"type": message_type,
			"to": channel,
			"topic": "",
			"content": content,
			"local_id": "ryzom-ig",
			"queue_id": self.getZulipQueueId(),
		}

		try:
			result = self.zulip.send_message(request)
		except Exception as e:
			print("Error sending message", e)
			return None
		if not isinstance(result, dict):
			print("Error sending message, invalid response", result)
			return None
		if result.get("result") == "success" and "id" in result:
			log_channel =  channel.split(" ")[0]
			log_content =  "".join([ s[0] for s in  content.split() ])
			print(f"💬 {message_type} to {channel} with {log_content} = {result['id']}")
			return result["id"]
		if result.get("result") == "error":
			print("Error sending message", result.get("msg", ""))
			return -1
		return None

	def sendTranslation(self, message_id, m):
		if message_id > 0 and m.translation and m.source_lang and m.translated_lang:
			# Normalize Zulip-style upload markdown in translation as well
			clean_translation = self.convert_zulip_upload_links(m.translation)
			request = {
				"message_id": message_id,
				"content": "<["+m.translated_lang+"]>"+flags.get(m.source_lang.lower(), "")+" "+(clean_translation if clean_translation is not None else ""),
			}
			try:
				result = self.zulip.update_message(request)
			except Exception as e:
				print("Error update message", e)
				return False
			if not isinstance(result, dict) or result.get("result") != "success":
				print(f"Error update message {message_id}", result.get("msg", "") if isinstance(result, dict) else result)
			else:
				print(f"{emojis.get(m.translated_lang.lower(), m.translated_lang)} {message_id}")
		return True

	def run(self):
		last_ids = {}
		messages = {}
		for lang in ALL_LANGS:
			last_ids[lang] = self.getLastChatLangID(lang)

		chat_id = self.getLastChatID()
		while True:
			chat_id = self.getLastChatID()
			if last_ids[lang]+1 < chat_id+1:
				for lang in ALL_LANGS:
					#print(lang, "{} -> {}".format(last_ids[lang]+1, chat_id+1))
					for i in range(last_ids[lang]+1, chat_id+1):
						if not i in messages:
							messages[i] = self.getRyzomMessage(i)
						message = messages[i]

						result = False
						try:
							if message and message.channel not in ("say", "shout", "arround", "region", "dyn", "team") and (message.translated_lang or "").lower() == lang:
								if lang == "wk":
									if message.source != "zulip":
										result = self.sendMessage(message)
										if result != None:
											self.addZulipMessageId(i, result)
								else:
									message_id = None
									tries = 50
									while not message_id:
										if message.source == "zulip":
											source_message = self.getRyzomMessage(message.source_message_id)
											if source_message is None:
												print(f"Source message {message.source_message_id} of {i} not found")
												break
											message_id = source_message.source_message_id
										else:
											message_id = self.getZulipMessageId(message.source_message_id)
										time.sleep(0.1)
										tries -= 1
										if tries <= 0:
											break

									if message_id:
										result = self.sendTranslation(int(message_id), message)
									else:
										result = True
							else:
								result = True
						except Exception:
							# Skip the message instead of crashing or retrying it forever
							print(f"Error dispatching message {i} ({lang}):\n{traceback.format_exc()}")
							result = True

						if result:
							last_ids[lang] = i
							self.setLastChatLangID(lang, i)
			time.sleep(0.1)


if __name__ == "__main__":
	zulip = ZulipDispatcher()
	zulip.register()
	zulip.run()
