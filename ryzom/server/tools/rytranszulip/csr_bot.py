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
# Base class for Ryzom <-> Deepl <-> Zulip system
#

import configparser
import mysql.connector
import zulip
from zulip_service import ZulipClient

class DB:

	def __init__(self, configfile, db):
		self.config = configparser.ConfigParser()
		self.config.read(configfile)
		self.db_section = db
		self.db = None
		self.connect()

	def connect(self):
		try:
			self.db = mysql.connector.connect(
				host=self.config[self.db_section]["host"],
				user=self.config[self.db_section]["user"],
				passwd=self.config[self.db_section]["pass"],
				database=self.config[self.db_section]["name"])
			print("MySQL Database connection successful")
		except mysql.connector.Error as err:
			print(f"Connection error: '{err}'")

	def query(self, sql, values=()):
		try:
			self.db.ping(reconnect=True, attempts=3, delay=1)
		except mysql.connector.Error:
			self.connect()
		cursor = self.db.cursor()
		try:
			cursor.execute(sql, values)
		except mysql.connector.Error as err:
			print("Error", err)
		return cursor

	def exec(self, sql, values=()):
		try:
			self.db.ping(reconnect=True, attempts=3, delay=1)
		except mysql.connector.Error:
			self.connect()
		cursor = self.db.cursor()
		try:
			cursor.execute(sql, values)
		except mysql.connector.Error as err:
			print("Error", err)
		self.db.commit()

class CsrBot():

	def __init__(self):
		super().__init__()
		self.reply = ""
		self.db_ring = DB("/etc/ryzom/shard.ini", "db_ring")
		self.db_webig = DB("/etc/ryzom/rtz.ini", "db_webig")

		self.config = configparser.ConfigParser()
		self.config.read("/etc/ryzom/rtz.ini")
		self.base_url = self.config["zulip"]["site"]

		while True:
			try:
				self.zulip = ZulipClient(config_file="/etc/ryzom/csr_bot.rc")
			except Exception as e:
				print("Error Zulip server", self.config["zulip"]["site"])
				print(e)
				print("Retrying...")
				time.sleep(1)
			else:
				break
		print("Connected! to", self.base_url, "!")

	def send_message(self, channel, topic, content):
		channel_type = "stream"
		if channel[0] == "@":
			channel = channel[1:]
			channel_type = "private"

		request = {
			"type": channel_type,
			"to": channel,
			"topic": topic,
			"content": content,
			"local_id": "csr-bot",
			"queue_id": self.zulip.queue_id,
		}
		print("Send message to Zulip", request)
		try:
			result = self.zulip.send_message(request)
		except Exception as e:
			print("Error sending message", e)
		print("Result:", result)
		if result["result"] == "success":
			return result["id"]
		if result["result"] == "error":
			return -1
		return None

	def react_message(self, message_id, emoji):
		request = {
			"message_id": message_id,
			"emoji_name": emoji,
		}
		try:
			result = self.zulip.add_reaction(request)
		except Exception as e:
			print("Error sending message", e)
		print("Result:", result)
		if result["result"] == "success":
			return message_id
		if result["result"] == "error":
			return -1
		return None

	def call_rename(self, sender, args):
		"""**Rename a player:** /rename `player` `new_name`
:small_blue_diamond: **`player`** : the name of the char
:small_blue_diamond: **`new_name`** : the new name"""
		if len(args) < 2:
			return "warning"
		char_name = args[0]
		new_name = args[1]

		char = self.db_ring.query("SELECT last_played_date FROM characters WHERE char_name = %s LIMIT 1", (char_name,)).fetchall()
		if char:
			char2 = self.db_ring.query("SELECT last_played_date FROM characters WHERE char_name = %s LIMIT 1", (new_name,)).fetchall()
			if not char2:
				self.db_webig.exec("INSERT INTO player_login_commands (`agent`, `player`, `command`) VALUES (%s, %s, %s)", (sender, char_name, "rnm "+new_name))
				self.send_message("🚨 CSR Logs", "📯 Commands", "@**"+sender+"** ask for a rename of player **"+char_name+"** to "+new_name+" at connection!")
			else:
				self.reply += ":warning: The name **"+new_name+"** already used, player need choose another"
				return "wrong_way"
		else:
			self.reply += ":warning: The player **"+char_name+"** doesn\'t exists"
			return "wrong_way"

		return "hourglass"


	def checkMessages(self, event):
		message = event["message"]
		channel = message["display_recipient"]
		topic = message["subject"]
		print(event)
		content = message["content"].strip()
		sender = message["sender_email"].split("@")[0]
		if len(content) >= 2 and content[0] == "/":
			args = content[1:].split(" ")
			command = args[0]
			args = args[1:]
			emoji_name = ""
			if hasattr(self, "call_"+command):
				func = getattr(self, "call_"+command)
				emoji_name = func(sender, args)
				if emoji_name == "warning":
					self.reply += ":warning: Invalid usage of command **"+command+"**\n"
					args = [command]+args
					command = "help"

			if not emoji_name:
				if command == "help" or command == "?":
					skip = False
					if args:
						if hasattr(self, "call_"+args[0]):
							func = getattr(self, "call_"+args[0])
							self.reply += "```quote\n"+func.__doc__+"\n```"
							skip = True
						else:
							self.reply += ":warning: Command **"+args[0]+"** not found\n"
							emoji_name = "face_with_raised_eyebrow"
					if not skip:
						self.reply += """```quote
**:place_holder: Available commands:**
:small_blue_diamond: **`rename`** : Rename a player (/rename <player> <new_name>)
:small_blue_diamond: **`wait`** : Wait for connexion of a player (/wait <reason to wait>)
```"""

				else:
					emoji_name = "interrobang"

			if self.reply:
				self.send_message(channel, topic, self.reply)

			if emoji_name:
				self.react_message(message["id"], emoji_name)

	def run(self):
		print("Fetching Zulip messages")
		self.zulip.registerMessages()
		self.zulip.manageMessages(self.checkMessages)

if __name__ == "__main__":
	csrbot = CsrBot()
	csrbot.run()
