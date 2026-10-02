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
import urllib.parse
import urllib.request

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
		if isinstance(zulip_id, bytes):
			zulip_id = zulip_id.decode("ascii")
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
		# Normalize potential Zulip-style upload markdown to plain URLs before sending
		clean_text = self.convert_zulip_upload_links(m.text)
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
			"local_id": "ryzom-ig",
			"queue_id": self.getZulipQueueId(),
		}

		try:
			content = self.quotePrefix(m)+user[0].upper()+user[1:]+":"+(clean_text if clean_text is not None else "")
			request["content"] = content
			result = self.zulip.send_message(request)
		except (ValueError, TypeError, KeyError, IndexError, AttributeError) as e:
			print("Invalid message", e)
			return None
		except Exception as e:
			print("Error sending message", e)
			return -1
		if result["result"] == "success":
			log_channel =  channel.split(" ")[0]
			log_content =  "".join([ s[0] for s in  content.split() ])
			log_queueid = self.getZulipQueueId()
			print(f"💬 {message_type} to {log_channel} with {log_content} = {result['id']}")
			return result["id"]
		print("Error sending message", result.get("code", ""), result.get("msg", ""))
		return -1 if self.retryError(result) else None

	def sendTranslation(self, message_id, m):
		if message_id > 0 and m.translation and m.source_lang:
			# Normalize Zulip-style upload markdown in translation as well
			clean_translation = self.convert_zulip_upload_links(m.translation)
			request = {
				"message_id": message_id,
			}
			try:
				request["content"] = (self.quotePrefix(m)+"<["+m.translated_lang+"]>"+flags[m.source_lang]+" "+
					(clean_translation if clean_translation is not None else ""))
				result = self.zulip.update_message(request)
			except (ValueError, TypeError, KeyError, IndexError, AttributeError) as e:
				print("Invalid translation", e)
				return None
			except Exception as e:
				print("Error update message", e)
				return False
			else:
				if result.get("result") != "success":
					print("Error updating translation", result.get("code", ""), result.get("msg", ""))
					return False if self.retryError(result) else None
				request["content"] = "".join([ s[0] for s in  request["content"].split() ])
				print(f"{emojis[m.translated_lang.lower()]} {message_id}")
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
						if message and message.channel not in ("say", "shout", "arround", "region", "dyn", "team") and message.translated_lang.lower() == lang:
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
									if not message_id:
										original = self.getRyzomMessage(message.source_message_id)
										message_id = original.source_message_id if original else None
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
					except (ValueError, TypeError, KeyError, IndexError, AttributeError) as e:
						print("Invalid queued message", i, lang, e)
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
