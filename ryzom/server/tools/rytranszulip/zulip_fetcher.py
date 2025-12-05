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
# Copyright (C) 2025 Nuneo (ulukyn@gmail.com)
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
		self.guilds = {}

	def updateGuilds(self):
		with open("/tmp/dump_guilds.json") as file:
			try:
				self.guilds = json.load(file)
			except:
				print("dump guilds error")
				pass

	def checkMessages(self, event):
		if time() > self.last_update_guilds + 60:
			self.updateGuilds()
			self.last_update_guilds = time()
				
		msg = event["message"]
		print(msg)
		if "local_message_id" in event and event["local_message_id"] == "ryzom-ig":
			return

		message = msg["content"]
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
			print(stream)
			if stream["stream"]["creator_id"] != self.admin_id or msg["subject"] != "general chat":
				return
			channel = dest.lower()
			if channel[0] == u"🔰":
				gid = channel.split("(")[1][:-1]
				channel_id = "guild:(0x002f5"+gid+":09:00:00)"
			elif channel[0] == u"💠":
				channel_id = "FACTION_RF"
			elif channel[0] == "⚜":
				channel_id = "FACTION_"+channel[2:].strip().upper()
			elif channel[0] == "❇":
				channel_id = channel[2:].strip()
			else:
				channel_id = channel[1:].strip().lower()
			
			lang = msg["translation_lang"] if "translation_lang" in msg else "en"
			message = RyzomMessage("zulip", sender.lower(), channel, channel_id, lang, "*", message, source_message_id=msg["id"])
			print(message.pprint())
			self.addRyzomMessage(message)
			
	
	def run(self):
		print("Fetching Zulip messages")
		self.setZulipQueueId(self.zulip.registerMessages())
		self.zulip.manageMessages(self.checkMessages)

if __name__ == "__main__":
	fetcher = ZulipFetcher()
	fetcher.register()
	fetcher.run()


