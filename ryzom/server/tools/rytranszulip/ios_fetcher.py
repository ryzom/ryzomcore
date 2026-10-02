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
import base64
import binascii
import mysql.connector

from time import sleep, time
from ryzom_service import RyzomService, RyzomMessage

class IosFetcher(RyzomService):

	def __init__(self):
		super().__init__()
		self.name = "IosFetcher"
		self.version = "1.2"
		self.scriptfile = __file__
		self.logname = "/home/nevrax/shard/logs/chat/chat.log"
		self.log_sections["logs"] = ("file", self.logname, "", "")
		self.log_sections["messages"] = ("db", "Ryzom-Chat-LastID", "Ryzom-Chat-{}", "")
		self.stats = {"filesize": 0, "seek": 0, "lines": 0}
		self.logfile = open(self.logname, "r", encoding="utf-8", errors="replace")
		self.shard = host = self.config["shard"]["name"]
		self.domain = "("+self.shard[0].upper()+self.shard[1:]+")"
		self.updateStats()
		self.guilds = {}

		self.db = None
		self.connectDb()

	def connectDb(self):
		try:
			self.db = mysql.connector.connect(
				host = self.config["db_webig"]["host"],
				user = self.config["db_webig"]["user"],
				passwd = self.config["db_webig"]["pass"],
				database = "webig",
			)
			print("MySQL Database connection successful")
		except mysql.connector.Error as err:
			print(f"Error: '{err}'")

	def ensureDbConnection(self):
		try:
			if self.db is None:
				raise mysql.connector.Error("No database connection")
			self.db.ping(reconnect=True, attempts=3, delay=5)
		except mysql.connector.Error as err:
			print(f"Error: '{err}', reconnecting...")
			self.connectDb()

	def follow(self, thefile):
		thefile.seek(0, os.SEEK_END)
		file_size = os.stat(self.logname).st_size
		last_update_guilds = 0
		while True:
			line = thefile.readline()
			if line:
				self.stats["lines"] += 1
				self.updateStats()
				self.status_color = "🟩"
				self.status = "New line in log... "
				self.updateActivity(False)
				yield line
			else:
				self.status_color = ""
				self.status = "Waiting..."
				size = os.stat(self.logname).st_size
				if file_size > size:
					self.stats["filesize"] = file_size
					self.updateStats()
					break
				file_size = size
				self.stats["filesize"] = file_size
				self.updateStats()
				self.updateActivity()
				sleep(0.01)

	def updateGuilds(self):
		with open("/tmp/dump_guilds.json") as file:
			try:
				self.guilds = json.load(file)
			except:
				pass

	def getGuildName(self, gid):
		self.ensureDbConnection()
		cursor = self.db.cursor()
		try:
			cursor.execute("SELECT * FROM guilds WHERE guild_id='"+gid+"' AND deleted = 0")
		except mysql.connector.Error as err:
			print("Error", err)
		else:
			guilds = cursor.fetchall()
			if guilds:
				return guilds[0][2]
		return ""

	def updateStats(self):
		lines = self.stats["lines"]
		filesize =  self.stats["filesize"]
		self.infos = f"[orange1]Reading log:[/orange1] {self.logname}\n"
		self.infos += f"[orange1]Size of logfile:[/orange1] {filesize}  \t[orange1]Lines parsed:[/orange1] {lines}\n"
		self.infos += f"[orange1]Shard: [/orange1]{self.shard}"
		self.updateInfos()

	def updateLogs(self, line):
		log_parts = line.strip().split(" ", 6)
		if len(log_parts) != 7:
			return
		payload = log_parts[6]
		if payload.startswith("chat_bridge|"):
			fields = payload.split("|")
			if len(fields) == 4 and fields[1] == "1":
				try:
					external_id = base64.b64decode(fields[2], validate=True).decode("utf-8")
					prefix = "zulip:"+self.base_url.rstrip("/")+":"
					if external_id.startswith(prefix):
						zulip_id = int(external_id[len(prefix):].split(":", 1)[0])
						self.addChatMessageId(fields[3], zulip_id)
						self.client.set("Chat-External-"+external_id, fields[3], 24*60*60)
				except (ValueError, UnicodeError, binascii.Error):
					print("Invalid bridge chat acknowledgement")
			return

		if payload.startswith("chat_meta|"):
			fields = payload.split("|")
			if len(fields) != 14 or fields[1] != "1":
				print("Unsupported structured chat log entry")
				return
			try:
				def decode(index):
					return base64.b64decode(fields[index], validate=True).decode("utf-8")
				channel, sender = decode(2), decode(3)
				source_lang, langs = fields[4:6]
				chat = {"message_id": fields[6],
					"quote_id": fields[9], "channel": channel,
					"quote_author": decode(10), "quote_text": decode(11)}
				message = decode(12)
				for label in decode(13).splitlines():
					fields = label.split("|", 3)
					if (len(fields) != 4 or fields[0] != "part" or
						int(fields[1]) != len(chat.setdefault("parts", [])) or
						fields[2] not in ("text", "reference")):
						raise ValueError("Invalid chat part")
					part_text = base64.b64decode(fields[3], validate=True).decode("utf-8")
					chat["parts"].append((fields[2], part_text))
				if "parts" in chat and (not chat["parts"] or
					"".join(part[1] for part in chat["parts"]) != message):
					raise ValueError("Invalid chat parts")
			except (ValueError, UnicodeError, binascii.Error, KeyError, TypeError):
				print("Invalid structured chat log entry")
				return
		else:
			parts = payload.split("|", 4)
			if len(parts) != 5:
				return
			channel, sender, source_lang, langs, message = parts
			chat = {}

		schannel = channel.split(":", 1)
		channel_id = schannel[1] if len(schannel) == 2 else ""
		channel = schannel[0]
		if channel not in ("say", "shout", "arround", "universe", "tell", "region", "guild", "team", "dyn"):
			return
		if channel == "guild":
			gid = str(int(channel_id[8:-10], 16)+0x6500000)
			guild_name = self.getGuildName(gid)
			channel = "🔰 "+guild_name+" ("+channel_id[8:-10]+")"
			channel_id = "guild:"+channel_id
		elif channel == "universe":
			channel = "🌐 Universe"
			channel_id = "universe"
		elif channel == "dyn":
			if channel_id[:8] == "FACTION_":
				langs = "*"
				channel = "💠 Forge" if channel_id == "FACTION_RF" else "⚜️  "+channel_id[8:]
				channel_id = "dyn:"+channel_id
			elif channel_id[:7] == "League_":
				return
			else:
				channel = "❇️  "+channel_id
				channel_id = "dyn:"+channel_id
		elif channel == "tell":
			channel = "player"
			if channel_id.startswith("~"):
				channel_id = channel_id[1:]
		elif channel == "arround":
			if not chat:
				message = message[5:]
			channel_id = channel
		elif channel == "region":
			channel_id = "region:"+channel_id
		elif channel == "team":
			channel_id = "team:"+channel_id
		else:
			channel_id = channel
		ssender = sender.split("@")
		if len(ssender) > 1:
			sender = ssender[1]
		self.addRyzomMessage(RyzomMessage("ios", sender, channel, channel_id, source_lang, langs, message, chat=chat))

	def run(self):
		loglines = self.follow(self.logfile)
		print("Fetching IOS log file")
		for line in loglines:
			self.updateLogs(line)

	def close(self):
		self.db.close()

if __name__ == "__main__":
	iosFetcher = IosFetcher()
	iosFetcher.register()
	iosFetcher.run()
	iosFetcher.close()
