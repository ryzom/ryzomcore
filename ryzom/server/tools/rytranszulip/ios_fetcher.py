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
		sline = line.strip().split(" ", 6)
		if len(sline) >= 5:
			message = sline[6].split("|", 4)
			channel, sender, source_lang, langs, message = message
			schannel = channel.split(":", 1)
			if len(schannel) == 2:
				channel = schannel[0]
				channel_id = schannel[1]
			else:
				channel_id = ""

			if not channel in ("say", "shout", "arround", "universe", "tell", "region", "guild", "team", "dyn"):
				print(channel+" 🛑 "+sline[0]+" "+sline[1]+" ", " ".join(sline[2:5])+" ", sline[6])
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
					if channel_id == "FACTION_RF":
						channel = "💠 Forge"
					else:
						channel = "⚜️  "+channel_id[8:]
					channel_id = "dyn:"+channel_id
				else:
					channel = "❇️  "+channel_id
					channel_id = "dyn:"+channel_id
			elif channel == "tell":
				channel = "player"
				channel_id = channel_id[1:]
			elif channel == "arround":
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

			sline[6] = "".join([ s[0] for s in  sline[6].split() ])
			print("ios logs", "✅ "+sline[0]+" "+sline[1]+" ", " ".join(sline[2:5])+" ", sline[6])
			message = RyzomMessage("ios", sender, channel, channel_id, source_lang, langs, message)
			last_id = self.addRyzomMessage(message)
			print("last_id", last_id)

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
