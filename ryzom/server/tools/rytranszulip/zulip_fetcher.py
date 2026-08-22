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
# Copyright (C) 2025 Nuneo (nuno@troispetits.net)
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# -== Ryzom IOS Fetcher ==-
#
# This script wait in a loop for all lines send to chat.log, parse them and fill the memcached server
#
# Message in format:  2025/05/17 01:42:18 INF 4155664128 IOS-136 : player:~Ulukyn|Ulueta|en|*|hello my friend
# Message out format: (SENDER, CHANNEL, CHANNEL_ID, SOURCE_LANG, DST_LANGS, TEXT)
#     ex. ("Ulueta", "player", "~Ulukyn", "en", "*", "hello my friend")
#

import os
import sys
import json
import re
from time import time

from zulip_service import ZulipService, RyzomMessage

class ZulipFetcher(ZulipService):

	def __init__(self):
		super().__init__()
		self.name = "ZulipFetcher"
		self.version = "1.2"
		self.scriptfile = __file__
		#self.log_sections["messages"] = ("db", "Ryzom-Chat-LastID", "Ryzom-Chat-{}", "")
		self.stats = {"messages": 0}
		self.admin_id = self.zulip.get_profile()["user_id"]
		self.last_update_guilds = 0
		self.shard = host = self.config["shard"]["name"]
		self.guilds_prefixes = {"atys": "0x00165", "gingo": "0x002f5"}

	def ingestZulipMessage(self, msg, source_lang=None):
		raw_msg = msg["content"]
		message = self.convert_zulip_upload_links(raw_msg)
		# Drop the "!" from image markdown: Zulip auto-embeds a plain [alt](url)
		# link to an image just as well, and this sidesteps every issue caused
		# by the "!" downstream (DeepL typographic spacing, link conversion, etc.)
		message = re.sub(r"!(\[[^\]]*]\([^)]+\))", r"\1", message)
		sender = msg["sender_full_name"]
		dest = msg["display_recipient"]
		if msg["type"] == "private":
			if len(dest) == 2:
				channel = "player"
				if dest[0]["id"] == msg["sender_id"]:
					channel_id = dest[1]["full_name"].lower()
				else:
					channel_id = dest[0]["full_name"].lower()

				message = RyzomMessage("zulip", sender, channel, "tell:"+channel_id, "wk", "*", message)
				self.addRyzomMessage(message)
		else:
			stream_id = msg["stream_id"]
			stream = self.zulip.call_endpoint(url=f"streams/{stream_id}", method="GET")
			if stream["result"] == "error":
				print(f"Error stream {stream_id}", stream["msg"])
				return
			if stream["stream"]["creator_id"] != self.admin_id: # or msg["subject"] != "general chat":
				print(f"RYZOM_DEBUG ignoring message {msg.get('id')}: stream {stream_id} not created by the bot (creator_id={stream['stream']['creator_id']}, admin_id={self.admin_id})")
				return
			channel = dest.lower()
			if channel[0] == u"🔰":
				gid = channel.split("(")
				if len(gid) > 1:
					gid = gid[1][:-1]
				else:
					gid = ""
				channel_id = "guild:("+self.guilds_prefixes[self.shard]+gid+":09:00:00)"
			elif channel[0] == u"💠":
				channel_id = "FACTION_RF"
			elif channel[0] == "⚜":
				channel_id = "FACTION_"+channel[2:].strip().upper()
			elif channel[0] == "❇":
				channel_id = channel[2:].strip()
			else:
				channel_id = channel[1:].strip().lower()

			if source_lang is None:
				# Language of the original message, so it can be reused if this message gets edited later
				source_lang = msg["translation_lang"] if "translation_lang" in msg else "en"
			self.client.set(f"Zulip-Msg-Lang-{msg['id']}", source_lang, 24*60*60)

			ryzom_message = RyzomMessage("zulip", sender.lower(), channel, channel_id, source_lang, "*", message, source_message_id=msg["id"])
			ryzom_id = self.addRyzomMessage(ryzom_message)
			print(f"RYZOM_DEBUG ingested zulip message {msg['id']} as Ryzom-Chat-{ryzom_id}, source_lang={source_lang!r}, translated_lang=WK, content={message!r}")

	def checkMessages(self, event):
		msg = event["message"]
		if "local_message_id" in event and event["local_message_id"] == "ryzom-ig":
			return
		self.ingestZulipMessage(msg)

	def checkUpdatedMessage(self, event):
		print(f"RYZOM_DEBUG update_message event received: message_id={event.get('message_id')} user_id={event.get('user_id')} rendering_only={event.get('rendering_only')} has_content={'content' in event}")
		# Only react to actual content edits by a real user; ignore rendering-only
		# fixups (e.g. link preview refresh) and edits with no content change.
		if event.get("rendering_only") or "content" not in event:
			print(f"RYZOM_DEBUG ignoring update_message {event.get('message_id')}: rendering_only or no content change")
			return
		# Edits made by the Ryzom bot itself are how translations get added to a
		# message; reacting to them here would create a translation loop.
		if event.get("user_id") == self.admin_id:
			print(f"RYZOM_DEBUG ignoring update_message {event.get('message_id')}: edit made by the bot itself (admin_id={self.admin_id})")
			return

		message_id = event["message_id"]
		result = self.zulip.call_endpoint(url=f"messages/{message_id}", method="GET", request={"apply_markdown": False})
		if result["result"] == "error":
			print(f"Error fetching edited message {message_id}", result["msg"])
			return

		# Reuse the original message's language, so a re-translation isn't
		# mistakenly sourced from the requesting bot's own language.
		source_lang = self.client.get(f"Zulip-Msg-Lang-{message_id}")
		print(f"RYZOM_DEBUG re-ingesting edited message {message_id} for re-translation, source_lang={source_lang!r}")
		self.ingestZulipMessage(result["message"], source_lang=source_lang)

	def dispatchEvent(self, event):
		if event["type"] == "message":
			self.checkMessages(event)
		elif event["type"] == "update_message":
			self.checkUpdatedMessage(event)

	def run(self):
		print("Fetching Zulip messages")
		self.setZulipQueueId(self.zulip.registerMessages())
		self.zulip.manageMessages(self.dispatchEvent)

if __name__ == "__main__":
	fetcher = ZulipFetcher()
	fetcher.register()
	fetcher.run()


