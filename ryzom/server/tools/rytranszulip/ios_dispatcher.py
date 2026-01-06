#!/bin/env python3
#############################################
#  _______________________________
#  \______   \__    ___/\____    /
#   |       _/ |    |     /     /
#   |    |   \ |    |    /     /_
#   |____|_  / |____|   /_______ \\
#          \/                   \/
#
# RyTransZulip - with delicious M.A.R.G.U.E.Z
# M.A.R.G.U.E.Z (Make Awesome all Ryzom's Gossips with the Unreasonable Empowerment of Zulip... and a touch of Deepl :D)
# Copyright (C) 2025 Nuneo (ulukyn@gmail.com)
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# -== Ryzom Dispatcher ==-
#
# This script dispatche messages to Ryzom IOS service
# This dispatcher is used 2 times.
# 1) When a message comes from a Fetcher, the original is sent without delay to IOS
# 2) When a message are translated by Deepl, the translation is sent to IOS
#

import os
import sys
import re

from time import sleep, time
from pynel.admin_modules_itf import CAdminServiceWeb
from ryzom_service import RyzomService, RyzomMessage

class IosDispatcher(RyzomService):

	def __init__(self):
		super().__init__()
		self.name = "IosDispatcher"
		self.version = "1.2"
		self.scriptfile = __file__
		self.ryzomAS = CAdminServiceWeb()
		self.ryzomAS_ok = False
		self.log_sections["messages"] = ("db", "Ryzom-Chat-LastID", "Ryzom-Chat-{}", "")
		self.stats = {"messages": 0, "messages_to_ios": 0}
		self.shard = host = self.config["shard"]["name"]
		self.domain = "("+self.shard[0].upper()+self.shard[1:]+")"
		self.updateStats()

	def updateStats(self):
		messages = self.stats["messages"]
		messages_ios =  self.stats["messages_to_ios"]
		status = "✅" if self.ryzomAS_ok else "🛑"
		self.infos = f"[orange1]Connection to Ryzom AS:[/orange1] {status}\n"
		self.infos += f"[bright_green]Messages checked: [bright_yellow]{messages}  \t[bright_green]Messages to IOS: [bright_yellow]{messages_ios}\n"
		self.infos += f"[bright_green]Shard: [orange1]{self.shard}"
		self.updateInfos()

	def runIOSCommand(self, command):
		if self.ryzomAS.connect("127.0.0.1", 46700):
			out = command.split(" ", 3)
			out[3] = "".join([ s[0] for s in  out[3].split() ])
			print("▶️ ", " ".join(out), "[", self.ryzomAS.service_cmd("ios", command) ,"]")
			self.ryzomAS.close()
			return True
		else:
			print("🛑 Connextion failed")
		return False

	def runEGSCommand(self, command):
		if self.ryzomAS.connect("127.0.0.1", 46700):
			out = command.split(" ")
			out[3] = "".join([ s[0] for s in  out[3].split() ])
			print("▶️ ", " ".join(out), "[", self.ryzomAS.service_cmd("egs", command) ,"]")
			self.ryzomAS.close()
			return True
		else:
			print("🛑 Connextion failed")
		return False

	def sendToService(self, m):
		command = "chat" if m.source == "ios" else "farChat"
		sender = m.sender + (self.domain if m.source == "ios" else "")
		prefix = ">" if command == "chat" else ""
		source_lang = ":"+m.source_lang.lower()+":"
		translated_lang = ":"+m.translated_lang.lower()+":"

		# First, normalize potential Zulip upload markdown links to plain URLs
		clean_text = self.convert_zulip_upload_links(m.text)
		clean_translation = self.convert_zulip_upload_links(m.translation)

		# Then escape quotes as before
		text = clean_text.replace("\"", "''") if clean_text is not None else ""
		translation = clean_translation.replace("\"", "''") if clean_translation is not None else ""

		if command == "chat" and m.channel == "player":
			return True
		if m.translated_lang == "WK":
			if m.channel == "player":
				self.runIOSCommand(command+" "+sender.lower()+" "+m.channel_id+self.domain+" \""+text+"\"")
				self.runIOSCommand(command+" "+m.channel_id.split(":")[1].lower()+" tell:"+sender+self.domain+" \"\n@{FF0F}"+sender+": "+text+"\"")
			elif command == "farChat": # Messages from zulip
				self.runIOSCommand(command+" "+m.sender+" "+m.channel_id+" \""+source_lang+text+"\"")
			elif m.channel_id.split(":")[0] == "faction": # FIXME on IOS
				self.runIOSCommand(command+" "+sender+" dyn:"+m.channel_id.split(":")[1]+" \""+prefix+source_lang+text+"\"")
			elif m.channel_id.split(":")[0] == "dyn":
				self.runIOSCommand(command+" "+sender+" "+m.channel_id+" \">"+source_lang+text+"\"")

		else:
			if m.channel == "arround":
				self.runIOSCommand(command+" "+sender+" "+m.channel_id+" \""+prefix+translated_lang+"&EMT&{"+source_lang+text+"}@{ "+translation+"\"")
			elif m.channel_id.split(":")[0] == "dyn":
				self.runIOSCommand(command+" "+sender+" "+m.channel_id+" \""+prefix+translated_lang+"{"+source_lang+text+"}@{ "+translation+"\"")
			else:
				self.runIOSCommand(command+" "+sender+" "+m.channel_id+" \""+prefix+translated_lang+"{"+source_lang+text+"}@{ "+translation+"\"")
		return True

	def checkMessages(self):
		last_id = self.getLastChatID()
		for i in range(self.current_id+1, last_id+1, 1):
			message = self.getRyzomMessage(i)
			if message != None:
				print("New Message", message.output_zipped())
				self.updateActivity(False)
				self.stats["messages"] += 1
				status = self.sendToService(message)
				if status:
					self.stats["messages_to_ios"] += 1
				self.updateStats()
				status = "✅" if status else "🛑"
				self.next_id = i
		self.current_id = self.next_id

	def run(self):
		self.current_id = self.getLastChatID()
		self.next_id = self.current_id
		print("Sending messages to IOS...")
		while True:
			#self.preCheck()
			self.checkMessages()
			self.updateActivity()
			sleep(0.1)

if __name__ == "__main__":
	dispatcher = IosDispatcher()
	dispatcher.register()
	dispatcher.run()
