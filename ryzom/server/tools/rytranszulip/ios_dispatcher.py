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
# Copyright (C) 2025 Nuneo (nuno@troispetits.net)
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

CHAT_ACK_WAIT_SECONDS = 60

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
		if not self.ryzomAS.connect("127.0.0.1", 46700):
			print("🛑 Connection failed")
			return False
		try:
			if not self.ryzomAS.service_cmd("ios", command) or not self.ryzomAS.wait_callback():
				return False
			result = self.ryzomAS.command_return_data.lstrip()
			if "chat_bridge_reject|1|" in result:
				return None
			if "not found, try 'help'" in result or "Bad command usage" in result:
				print("AS/IOS could not execute", command.split(" ", 1)[0])
				return None
			if result.startswith("ERROR"):
				print("AS/IOS could not execute", command.split(" ", 1)[0])
				return False
			print("▶️ ", command.split(" ", 1)[0])
			return True
		except OSError as e:
			print("AS/IOS connection failed", e)
			return False
		finally:
			self.ryzomAS.close()

	def runEGSCommand(self, command):
		if self.ryzomAS.connect("127.0.0.1", 46700):
			out = "".join([ s[0] for s in  command.split(" ")[3].split() ])
			print("▶️ ", out)
			self.ryzomAS.service_cmd("egs", command)
			self.ryzomAS.close()
			return True
		else:
			print("🛑 Connextion failed")
		return False

	def sendStructured(self, message):
		if message.source == "ios" and message.translated_lang == "WK":
			return True
		chat = message.chat
		message_id = chat.get("message_id", "")
		if message.translated_lang != "WK" and not message_id:
			external_id = chat.get("external_id", "")
			message_id = self.client.get("Chat-External-"+external_id) if external_id else ""
			wait_key = "Chat-External-Wait-"+external_id
			if not message_id:
				original = self.getRyzomMessage(message.source_message_id) if message.source == "zulip" else None
				if (not external_id or not original or original.source != "zulip" or
					original.translated_lang != "WK" or original.chat.get("external_id") != external_id or
					original.channel_id != message.channel_id or original.source_lang != message.source_lang):
					self.client.delete(wait_key)
					return None
				if self.client.add("Chat-External-Retry-"+external_id, 1, expire=2, noreply=False):
					status = self.sendStructured(original)
					if status is not True:
						self.client.delete(wait_key)
						return status
					self.client.add(wait_key, time()+CHAT_ACK_WAIT_SECONDS, expire=24*60*60, noreply=False)
				deadline = self.client.get(wait_key)
				if deadline is not None and time() >= deadline:
					print("No IOS acknowledgement for translation", message.source_message_id)
					self.client.delete(wait_key)
					return None
				return False
			self.client.delete(wait_key)
		text = message.text if message.translated_lang == "WK" else message.translation
		if not text:
			return None
		part_payload = "-"
		if message.translated_lang != "WK" and chat.get("translation_parts"):
			parts = chat["translation_parts"]
			part_payload = str(len(parts))+":"+"".join(
				str(len(part.encode("utf-8")))+":"+part for part in parts)
			part_payload = self.encodeChatText(part_payload)
		arguments = [
			self.encodeChatText(message.sender),
			self.encodeChatText(chat.get("channel", message.channel_id)),
			self.encodeChatText(chat["external_id"]) if chat.get("external_id") else "-",
			message_id or "-", chat.get("quote_id") or "-",
			message.source_lang or "wk",
			"-" if message.translated_lang == "WK" else message.translated_lang.lower(),
			self.encodeChatText(text)]
		if part_payload != "-":
			arguments.append(part_payload)
		command = "bridgeChat "+" ".join(arguments)
		return self.runIOSCommand(command)

	def sendReaction(self, m):
		reaction = m.chat["reaction"]
		return self.runIOSCommand("bridgeReaction "+" ".join([self.encodeChatText(m.sender),
			reaction["message_id"], self.encodeChatText(reaction["emoji"]),
			"remove" if reaction["remove"] else "add"]))

	def sendToService(self, m):
		if m.chat.get("reaction"):
			# Game reactions were already delivered in game.
			return self.sendReaction(m) if m.source == "zulip" else True
		if m.chat:
			return self.sendStructured(m)
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
			try:
				message = self.getRyzomMessage(i)
				if message != None:
					self.updateActivity(False)
					self.stats["messages"] += 1
					status = self.sendToService(message)
					if status is False:
						break
					if status:
						self.stats["messages_to_ios"] += 1
					else:
						print("Skipping IOS message", i)
					self.updateStats()
			except (ValueError, TypeError, KeyError, IndexError, AttributeError) as e:
				print("Invalid queued IOS message", i, e)
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
