
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
# -== Ryzom Shard Commands ==-
#
# This script wait in a loop for all commands put by shard in memecached
#


import os
import sys
import json
import traceback
import mysql.connector
import urllib.parse

from time import sleep, time

from zulip_service import ZulipClient
from ryzom_service import RyzomService


OWNER_GROUP=10
PLAYER_GROUP=73
CUSTOM_PROFILE_TRANSLATIONS="1"
CUSTOM_PROFILE_TITLE="2"
CUSTOM_PROFILE_GUILD="3"
INGAME_FOLDER_ID=2

ALLOWED_COMMANDS = ("playerConnects", "addFactionChannelToCharacter", "removeFactionChannelForCharacter", "deleteMember")

class ShardCommands(RyzomService):

	def __init__(self):
		super().__init__()
		while True:
			try:
				self.zulip = ZulipClient(config_file=".zuliprc")
			except:
				print("Zulip server not available. Retrying...")
				sleep(1)
			else:
				break
		self.name = "ShardCommands"
		self.version = "1.2"
		self.scriptfile = __file__
		self.log_sections["messages"] = ("db", "Shard-Command-Last", "Shard-Command-{}", "")
		self.stats = {"commands": 0}
		self.shard = host = self.config["shard"]["name"]
		self.domain = "("+self.shard[0].upper()+self.shard[1:]+")"
		self.updateStats()
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

	def updateStats(self):
		self.infos = f"[orange1]Shard: [/orange1]{self.shard}"
		self.updateInfos()

	def getUser(self, user):
		ret = self.zulip.call_endpoint(
			url=f"/users/{user}?include_custom_profile_fields=true",
			method="GET",
		)

		if isinstance(ret, dict) and ret.get("result") == "success":
			return ret.get("user")
		return None

	def getGuildId(self, name):
		self.ensureDbConnection()
		if self.db is None:
			print(f"No database connection, guild {name} id unknown")
			return 0
		try:
			cursor = self.db.cursor()
			cursor.execute("SELECT * FROM guilds WHERE name=%s AND deleted = 0", (name,))
			print("Database created successfully")
			guilds = cursor.fetchall()
		except mysql.connector.Error as err:
			print("Error", err)
		else:
			print(guilds)
			if guilds and len(guilds[0]) > 1:
				try:
					return int(guilds[0][1])
				except (TypeError, ValueError):
					print(f"Invalid guild id for {name}: {guilds[0][1]!r}")
		return 0

	def getGuildChannel(self, name):
		guild_id = self.getGuildId(name)
		if guild_id < 0x6500000:
			print(f"Guild {name} not found")
			return ""
		return "🔰 "+name+f" ({guild_id-0x6500000:0>5X})"

	def getProfileValue(self, user, field):
		value = user["profile_data"].get(field)
		if isinstance(value, dict):
			return value.get("value") or ""
		return ""

	def addSubscription(self, user, sub):
		return self.zulip.add_subscriptions(
			streams = [{"name": sub}],
			principals = [user],
			invite_only = True,
			history_public_to_subscribers = False,
			topics_policy = json.dumps("empty_topic_only"),
			message_retention_days = "7",
			can_add_subscribers_group = OWNER_GROUP,
			can_remove_subscribers_group = OWNER_GROUP,
			can_send_message_group = PLAYER_GROUP,
			send_new_subscription_messages = False,
			folder_id = INGAME_FOLDER_ID,
		)

	def playerConnects(self, command):
		if len(command) >= 3:
			user_email = command[1].lower()+"@ig.ryzom.com"
			old_guild = ""
			new_guild = ""
			title=""

			user = self.getUser(user_email)

			if isinstance(user, dict) and isinstance(user.get("profile_data"), dict):
				old_guild = self.getProfileValue(user, CUSTOM_PROFILE_GUILD)
				title = self.getProfileValue(user, CUSTOM_PROFILE_TITLE)


			if command[2]:
				if not title:
					# First ingame login of player : subscribe to default INGAME channels
					self.addSubscription(user_email, "🌐 Universe") # Universe
					self.addSubscription(user_email, "💠 Forge") # Forge
				#TODO: Manage translations
				if command[2] != title:
					self.zulip.call_endpoint(
						url="/users/"+user_email+"?profile_data="+urllib.parse.quote_plus("[{\"id\":"+CUSTOM_PROFILE_TITLE+", \"value\": \""+command[2]+"\"}]"),
						method="PATCH",
					)

			if len(command) >= 4 and command[3]:
				new_guild = self.getGuildChannel(command[3])
				if not new_guild:
					# unknown guild: keep current subscriptions untouched
					return

			if new_guild:
				self.addSubscription(user_email, new_guild)

			if old_guild and new_guild != old_guild:
				self.zulip.remove_subscriptions([old_guild], principals = [user_email])

			if new_guild != old_guild:
				self.zulip.call_endpoint(
					url="/users/"+user_email+"?profile_data="+urllib.parse.quote_plus("[{\"id\":"+CUSTOM_PROFILE_GUILD+", \"value\": \""+new_guild+"\"}]"),
					method="PATCH",
				)


	def addFactionChannelToCharacter(self, command):
		if len(command) >= 3 and command[2] != "FACTION_RF":
			if command[2][:8] == "FACTION_":
				name = "⚜️  "+command[2][8:].title()
			else:
				name = "❇️  "+command[2]
			self.addSubscription(command[1].lower()+"@ig.ryzom.com", name)
			return True
		return False

	def removeFactionChannelForCharacter(self, command):
		if len(command) >= 3 and command[2] != "FACTION_RF":
			if command[2][:8] == "FACTION_":
				name = "⚜️  "+command[2][8:].title()
			else:
				name = "❇️  "+command[2]
			self.zulip.remove_subscriptions([name], principals = [command[1].lower()+"@ig.ryzom.com"])
			return True
		return False

	def deleteMember(self, command):
		if len(command) >= 3:
			user_email = command[2].lower()+"@ig.ryzom.com"
			guild = self.getGuildChannel(command[1])
			if not guild:
				return False
			self.zulip.remove_subscriptions([guild], principals = [user_email])
			self.zulip.call_endpoint(
					url="/users/"+user_email+"?profile_data="+urllib.parse.quote_plus("[{\"id\":"+CUSTOM_PROFILE_GUILD+", \"value\": \"""\"}]"),
					method="PATCH",
				)
			return True
		return False

	def manageMessage(self, i):
		command = self.getRyzomCommand(i)
		if command:
			self.updateActivity(False)
			if command[0] not in ALLOWED_COMMANDS:
				print(f"Unknown shard command {i} {command!r}, skipped")
				self.current_id = i
				return
			try:
				if getattr(self, command[0])(command):
					self.stats["commands"] += 1
					self.updateStats()
			except Exception:
				print(f"Error running shard command {i} {command!r}:\n{traceback.format_exc()}")
		self.current_id = i
			
	def checkMessages(self):
		last_id = self.getLastCommandID()
		if self.current_id > last_id:
			self.manageMessage(last_id)

		for i in range(self.current_id+1, last_id, 1):
			self.manageMessage(i)
		self.setLastManagedCommandID(self.current_id)

	def run(self):
		self.current_id = self.getLastManagedCommandID()
		while True:
			self.checkMessages()
			self.updateActivity()
			sleep(0.1)

if __name__ == "__main__":
	shardCommands = ShardCommands()
	shardCommands.register()
	if len(sys.argv) > 1:
		if sys.argv[1] == "reset":
			shardCommands.setLastManagedCommandID(0)
			sys.exit(0)
	shardCommands.run()
