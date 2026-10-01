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
		# Normalize potential Zulip-style upload markdown to plain URLs before sending
		clean_text = self.convert_zulip_upload_links(m.text)
		content = user[0].upper()+user[1:]+":"+(clean_text if clean_text is not None else "")
		if m.channel == "player":
			message_type = "private"
			channel = m.channel_id.lower()+"@ig.ryzom.com"
		elif m.channel == "dyn":
			channel = self.getRealChannel(m.channel_id)
		else:
			channel = self.getRealChannel(m.channel)
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
		if result["result"] == "success":
			log_channel =  channel.split(" ")[0]
			log_content =  "".join([ s[0] for s in  content.split() ])
			log_queueid = self.getZulipQueueId()
			print(f"💬 {message_type} to {log_channel} with {log_content} = {result["id"]}")
			return result["id"]
		if result["result"] == "error":
			print("Error sending message")
			return -1
		return None

	def sendTranslation(self, message_id, m):
		if message_id > 0 and m.translation and m.source_lang:
			# Normalize Zulip-style upload markdown in translation as well
			clean_translation = self.convert_zulip_upload_links(m.translation)
			request = {
				"message_id": message_id,
				"content": "<["+m.translated_lang+"]>"+flags[m.source_lang]+" "+(clean_translation if clean_translation is not None else ""),
			}
			try:
				result = self.zulip.update_message(request)
			except Exception as e:
				print("Error update message", e)
				return False
			else:
				request["content"] = "".join([ s[0] for s in  request["content"].split() ])
				print(f"{emojis[m.translated_lang.lower()]} {message_id}")
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
						if message and message.channel not in ("say", "shout", "arround", "region", "dyn", "team") and message.translated_lang.lower() == lang:
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

						if result:
							last_ids[lang] = i
							self.setLastChatLangID(lang, i)
			time.sleep(0.1)


if __name__ == "__main__":
	zulip = ZulipDispatcher()
	zulip.register()
	zulip.run()
