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

import io
import os
import re
import sys
import logging
import builtins
import configparser

from time import time
from datetime import datetime
from inspect import currentframe, getframeinfo
from pymemcache.client.base import Client
from pymemcache import serde


class RyzomPrint():

	def __init__(self):
		self.print_buffer = io.StringIO()
		self.builtins_print = builtins.print
		self.out_file = "."+sys.argv[0].split("/")[-1].split(".")[0]+".out"
		self.err_file = "."+sys.argv[0].split("/")[-1].split(".")[0]+".err"
		self.stdout = None
		self.stderr = None
		self.last_out_file = ""

	def setStdOut(self):
		if self.stdout:
			self.stdout.close()
			if os.path.isfile(self.last_out_file):
				os.remove(self.last_out_file)
		if os.path.isfile(self.out_file):
			os.remove(self.out_file)
		self.stdout = open(self.out_file, "a")
		self.last_out_file = self.out_file

	def setStdErr(self):
		if self.stderr:
			self.stderr.close()
			os.remove(self.last_err_file)
		if os.path.isfile(self.err_file):
			self.last_run_errors = open(self.err_file, "r").read()
			os.remove(self.err_file)
		self.stderr = open(self.err_file, "a")
		sys.stderr = self.stderr
		self.last_err_file = self.err_file

	def print(self, *args, **kwargs):
		x = datetime.now()
		time = x.strftime("%Y-%m-%d %H:%M:%S")
		cf = currentframe()
		cf = cf.f_back
		filename = getframeinfo(cf).filename.split("/")[-1]

		line = cf.f_lineno
		try:
			self.builtins_print(*args, file=self.print_buffer, **kwargs)
		except:
			return

		out = self.print_buffer.getvalue()
		log = f"[bright_yellow]{time}[/bright_yellow] [bright_cyan]{filename}:{line}[/bright_cyan] {out}"
		self.builtins_print(log)
		self.stdout.write(log)
		self.stdout.flush()
		sys.stdout.flush()
		self.print_buffer.seek(0)
		self.print_buffer.truncate(0)


logging.getLogger("urllib3").propagate = False
logging.getLogger("asyncio").propagate = False

printer = RyzomPrint()
printer.setStdOut()
builtins.print = printer.print


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

class RyzomService():
	def __init__(self):
		self.client = Client("localhost", serde=serde.pickle_serde)
		self.last_update = time()
		self.start = time()
		self.log_sections = {}
		self.services_last_updates = {}
		self.config = configparser.ConfigParser()
		self.config.read("/etc/ryzom/rtz.ini")
		self.base_url = self.config["zulip"]["site"]

	def register(self):
		self.client.set(f"Dag-{self.name}-LastUpdate", time())

	def updateActivity(self, is_ping=True):
		"""Update activity of the service. If it's a simple ping, the update is old of 0.5s.
		So supervisor can check if an update is do in less than 0.5s => it's a real activity, not just a ping"""
		if not is_ping or time() >= self.last_update + 0.5:
			# When a ping, only update each 0.5 seconds
			self.last_update = update = time()
			if is_ping:
				# A ping is old of 0.5s
				update -= 0.5
			self.client.set(f"Dag-{self.name}-LastUpdate", update)

	def getMessage(self, name):
		return self.client.get(name)

	def setMessage(self, name, value, expire=0):
		return self.client.set(name, value, expire=expire)

	def updateInfos(self):
		self.client.set(f"Dag-{self.name}-Infos", self.infos)

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
