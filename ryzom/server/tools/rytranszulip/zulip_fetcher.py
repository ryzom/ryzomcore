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
import requests
from time import time

from zulip_service import ZulipService, RyzomMessage
from quote_bridge import QuoteNotForwarded, extract_quote

QUOTE_ACK_WAIT_SECONDS = 60

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
		self.pending_quote_key = "Chat-Zulip-Quote-Pending-"+self.name

	def ingestZulipMessage(self, msg, source_lang=None):
		def get_original(message_id):
			result = self.zulip.call_endpoint(url=f"messages/{message_id}", method="GET",
				request={"apply_markdown": False})
			if result.get("result") != "success" and self.retryError(result):
				raise requests.exceptions.ConnectionError("Cannot fetch quoted message "+str(message_id))
			return result.get("message") if result.get("result") == "success" else None

		try:
			raw_msg, quoted_zulip_id = extract_quote(msg, self.base_url, get_original,
				lambda message_id: bool(self.getChatMessageId(message_id)))
		except ValueError as error:
			print(f"Cannot forward quoted message {msg['id']}: {error}")
			return
		message = self.convert_zulip_upload_links(raw_msg)
		# Drop the "!" from image markdown: Zulip auto-embeds a plain [alt](url)
		# link to an image just as well, and this sidesteps every issue caused
		# by the "!" downstream (DeepL typographic spacing, link conversion, etc.)
		message = re.sub(r"!(\[[^\]]*]\([^)]+\))", r"\1", message)
		sender = msg["sender_full_name"]
		chat = {"zulip_id": msg["id"]}
		if quoted_zulip_id is not None:
			chat["quote_id"] = self.getChatMessageId(quoted_zulip_id)
		chat["external_id"] = "zulip:"+self.base_url.rstrip("/")+":"+str(msg["id"])
		if msg.get("last_edit_timestamp"):
			chat["external_id"] += ":edit:"+str(msg["last_edit_timestamp"])
		dest = msg["display_recipient"]
		if msg["type"] == "private":
			if len(dest) == 2:
				channel = "player"
				if dest[0]["id"] == msg["sender_id"]:
					channel_id = dest[1]["full_name"].lower()
				else:
					channel_id = dest[0]["full_name"].lower()

				message = RyzomMessage("zulip", sender, channel, "tell:"+channel_id, "wk", "*", message, source_message_id=msg["id"], chat=chat)
				return self.addRyzomMessage(message)
		else:
			stream_id = msg["stream_id"]
			stream = self.zulip.call_endpoint(url=f"streams/{stream_id}", method="GET")
			if stream["result"] == "error":
				print(f"Error stream {stream_id}", stream["msg"])
				return
			if stream["stream"]["creator_id"] != self.admin_id: # or msg["subject"] != "general chat":
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
				channel_id = "dyn:FACTION_RF"
			elif channel[0] == "⚜":
				channel_id = "dyn:FACTION_"+channel[2:].strip().upper()
			elif channel[0] == "❇":
				channel_id = "dyn:"+channel[2:].strip()
			else:
				channel_id = channel[1:].strip().lower()

			if source_lang is None:
				# Language of the original message, so it can be reused if this message gets edited later
				source_lang = msg["translation_lang"] if "translation_lang" in msg else "en"
			self.client.set(f"Zulip-Msg-Lang-{msg['id']}", source_lang, 24*60*60)

			ryzom_message = RyzomMessage("zulip", sender, channel, channel_id, source_lang, "*", message, source_message_id=msg["id"], chat=chat)
			self.addRyzomMessage(ryzom_message)

	def deferQuotedMessage(self, message_id, quoted_id, event_type):
		key = self.pending_quote_key
		pending = self.client.get(key) or {}
		deadline = pending[message_id][1] if message_id in pending else time()+QUOTE_ACK_WAIT_SECONDS
		if deadline > time():
			pending[message_id] = (quoted_id, deadline, event_type)
			self.client.set(key, pending, expire=QUOTE_ACK_WAIT_SECONDS)

	def clearPendingQuote(self, message_id):
		key = self.pending_quote_key
		pending = self.client.get(key) or {}
		if message_id in pending:
			del pending[message_id]
			if pending:
				self.client.set(key, pending, expire=QUOTE_ACK_WAIT_SECONDS)
			else:
				self.client.delete(key)

	def retryPendingQuotes(self):
		key = self.pending_quote_key
		for message_id, (quoted_id, deadline, event_type) in (self.client.get(key) or {}).items():
			if time() >= deadline:
				print(f"Cannot forward quoted message {message_id}: acknowledgement not received")
				self.clearPendingQuote(message_id)
				continue
			if not self.getChatMessageId(quoted_id):
				continue
			result = self.zulip.call_endpoint(url=f"messages/{message_id}", method="GET",
				request={"apply_markdown": False})
			if result.get("result") != "success":
				continue
			message = result["message"]
			if event_type == "update_message":
				event = {"type": event_type, "message_id": message_id, "message": message,
					"content": True, "user_id": message["sender_id"]}
			else:
				event = {"type": "message", "message": message}
			if self.dispatchEvent(event) is not False:
				self.clearPendingQuote(message_id)
		return bool(self.client.get(key))

	def checkMessages(self, event):
		msg = event["message"]
		if "local_message_id" in event and event["local_message_id"] == "ryzom-ig":
			return False
		return self.ingestZulipMessage(msg)

	def checkUpdatedMessage(self, event):
		# Only react to actual content edits by a real user; ignore rendering-only
		# fixups (e.g. link preview refresh) and edits with no content change.
		if event.get("rendering_only") or "content" not in event:
			return False
		# Edits made by the Ryzom bot itself are how translations get added to a
		# message; reacting to them here would create a translation loop.
		if event.get("user_id") == self.admin_id:
			return False

		message_id = event["message_id"]
		if "message" in event:
			result = {"result": "success", "message": event["message"]}
		else:
			result = self.zulip.call_endpoint(url=f"messages/{message_id}", method="GET",
				request={"apply_markdown": False})
		if result["result"] == "error":
			print(f"Error fetching edited message {message_id}", result["msg"])
			return False

		# Reuse the original message's language, so a re-translation isn't
		# mistakenly sourced from the requesting bot's own language.
		source_lang = self.client.get(f"Zulip-Msg-Lang-{message_id}")
		return self.ingestZulipMessage(result["message"], source_lang=source_lang)

	def checkReaction(self, event):
		# The bot's own reactions are the ones forwarded from the game.
		if event.get("user_id") == self.admin_id or event.get("reaction_type") != "unicode_emoji":
			return
		message_id = self.getChatMessageId(event["message_id"])
		if not message_id:
			return
		sender = event.get("user", {}).get("full_name")
		if not sender:
			result = self.zulip.get_user_by_id(event["user_id"])
			if result.get("result") != "success":
				print(f"Error fetching reacting user {event['user_id']}", result.get("msg", ""))
				return
			sender = result["user"]["full_name"]
		reaction = {"message_id": message_id,
			"emoji": event["emoji_name"], "remove": event["op"] == "remove"}
		self.addRyzomMessage(RyzomMessage("zulip", sender, "", "", "wk", "", ":"+event["emoji_name"]+":",
			source_message_id=event["message_id"], chat={"reaction": reaction}))

	def dispatchEvent(self, event):
		event_type = event["type"]
		if event_type == "reaction":
			return self.checkReaction(event)
		if event_type == "message":
			message_id = event["message"]["id"]
		elif event_type == "update_message":
			message_id = event["message_id"]
		else:
			return
		try:
			processed = self.checkMessages(event) if event_type == "message" else self.checkUpdatedMessage(event)
		except QuoteNotForwarded as error:
			self.deferQuotedMessage(message_id, error.original_id, event_type)
			return False
		if processed is not False:
			self.clearPendingQuote(message_id)

	def run(self):
		print("Fetching Zulip messages")
		self.setZulipQueueId(self.zulip.registerMessages(apply_markdown=False))
		self.zulip.manageMessages(self.dispatchEvent, on_register=self.setZulipQueueId,
			on_poll=self.retryPendingQuotes, apply_markdown=False)

if __name__ == "__main__":
	fetcher = ZulipFetcher()
	fetcher.register()
	fetcher.run()


