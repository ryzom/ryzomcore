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
# Base class for Ryzom <-> Deepl <-> Zulip system
#

import re
import configparser
from marguez.service import Service


class RyzomMessage():

	def __init__(self, source="", sender="", channel="", channel_id="", source_lang="", langs="", text="", translated_lang="WK", translation="", source_message_id=0):
		self.source = source
		self.sender = sender
		self.channel = channel
		self.channel_id = channel_id
		self.source_lang = source_lang
		self.langs = langs
		self.text = text
		self.translated_lang = translated_lang
		self.translation = ""
		self.source_message_id = source_message_id

	def set(self, message):
		self.source, self.sender, self.channel, self.channel_id, self.source_lang, self.langs, self.text, self.translated_lang, self.translation, self.source_message_id = message

	def get(self):
		return (self.source, self.sender, self.channel, self.channel_id, self.source_lang, self.langs, self.text, self.translated_lang, self.translation, self.source_message_id)

	def pprint(self):
		out = []
		for n,v in self.__dict__.items():
			out.append(f"[bright_green]{n}:[/bright_green]'{v}'")
		return " ".join(out)

	def output(self):
		return repr(self.get())

	def output_zipped(self):
		out = list(self.get())
		out[6] = "".join([ s[0] for s in  out[6].split() ])
		out[8] = "".join([ s[0] for s in  out[8].split() ])
		return repr(out)

class RyzomService(Service):
	def __init__(self):
		super().__init__()
		self.config = configparser.ConfigParser()
		self.config.read("/etc/ryzom/rtz.ini")
		self.base_url = self.config["zulip"]["site"]

	def getLastChatID(self):
		self.last_chat_id = self.client.get("Ryzom-Chat-LastID")
		if self.last_chat_id == None:
			self.client.set("Ryzom-Chat-LastID", 1)
			self.last_chat_id = 1
		return int(self.last_chat_id)

	def getLastCommandID(self):
		self.last_cmd_id = self.client.get("Shard-Command-Last")
		if self.last_cmd_id == None:
			self.client.set("Shard-Command-Last", 1)
			self.last_cmd_id = 1
		return int(self.last_cmd_id)

	def getLastManagedCommandID(self):
		self.last_cmd_id = self.client.get("Shard-Command-LastManaged")
		if self.last_cmd_id == None:
			self.client.set("Shard-Command-LastManaged", 1)
			self.last_cmd_id = 1
		return int(self.last_cmd_id)

	def setLastManagedCommandID(self, last_id):
		self.client.set("Shard-Command-LastManaged", last_id)

	def addRyzomMessage(self, message):
		self.last_chat_id = self.client.incr("Ryzom-Chat-LastID", 1)
		self.client.set("Ryzom-Chat-"+str(self.last_chat_id), message.get(), 24*60*60)

		print("Set", "Ryzom-Chat-"+str(self.last_chat_id), "=", message.output_zipped())
		return self.last_chat_id

	def getRyzomMessage(self, i):
		message = self.client.get("Ryzom-Chat-"+str(i))
		if message:
			m = RyzomMessage()
			m.set(message)
			return m
		return None

	def getRyzomCommand(self, i):
		cmd = self.client.get("Shard-Command-"+str(i))
		if cmd:
			return cmd.decode("utf-8", errors="ignore").split(":")
		return None

	def convert_zulip_upload_links(self, text):
		if not text:
			return text
		pattern = re.compile(r"\[[^\]]*]\((/user_uploads/[^)]+)\)")
		def repl(match):
			return f"{self.base_url}{match.group(1)}"
		return pattern.sub(repl, text)
