
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
# -== Ryzom Shard Commands ==-
#
# This script wait in a loop for all commands put by shard in memecached
#


import os
import sys
import json
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

	def updateStats(self):
		self.infos = f"[orange1]Shard: [/orange1]{self.shard}"
		self.updateInfos()

	def getUser(self, user):
		ret = self.zulip.call_endpoint(
			url=f"/users/{user}?include_custom_profile_fields=true",
			method="GET",
		)

		if ret["result"] == "success":
			return ret["user"]
		return None

	def getGuildId(self, name):
		cursor = self.db.cursor()
		try:
			cursor.execute("SELECT * FROM guilds WHERE name=%s AND deleted = 0", (name,))
			print("Database created successfully")
		except mysql.connector.Error as err:
			print("Error", err)
		else:
			guilds = cursor.fetchall()
			print(guilds)
			if guilds:
				return guilds[0][1]
		return 0

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

			print("User", user)
			if user and "profile_data" in user:
				if CUSTOM_PROFILE_GUILD in user["profile_data"]:
					old_guild = user["profile_data"][CUSTOM_PROFILE_GUILD]["value"]
				if CUSTOM_PROFILE_TITLE in user["profile_data"]:
					title = user["profile_data"][CUSTOM_PROFILE_TITLE]["value"]


			if command[2]:
				print("Current Title:", title, "New:", command[2])
				if not title:
					# First ingame login of player : subscribe to default INGAME channels
					print(self.addSubscription(user_email, "🌐 Universe")) # Universe
					print(self.addSubscription(user_email, "💠 Forge")) # Forge
				#TODO: Manage translations
				if command[2] != title:
					print(self.zulip.call_endpoint(
						url="/users/"+user_email+"?profile_data="+urllib.parse.quote_plus("[{\"id\":"+CUSTOM_PROFILE_TITLE+", \"value\": \""+command[2]+"\"}]"),
						method="PATCH",
					))

			if len(command) >= 4 and command[3]:
				gid = int(self.getGuildId(command[3]))-0x6500000
				new_guild = "🔰 "+command[3]+f" ({gid:0>5X})"

			print(old_guild, "vs", new_guild)
			if new_guild:
				print(self.addSubscription(user_email, new_guild))

			if old_guild and new_guild != old_guild:
				print("Remove", self.zulip.remove_subscriptions([old_guild], principals = [user_email]))

			if new_guild != old_guild:
				print(self.zulip.call_endpoint(
					url="/users/"+user_email+"?profile_data="+urllib.parse.quote_plus("[{\"id\":"+CUSTOM_PROFILE_GUILD+", \"value\": \""+new_guild+"\"}]"),
					method="PATCH",
				))


	def addFactionChannelToCharacter(self, command):
		if len(command) >= 3 and command[2] != "FACTION_RF":
			if command[2][:8] == "FACTION_":
				name = "⚜️  "+command[2][8:].title()
			else:
				name = "❇️  "+command[2]
			print("Add sub:", self.addSubscription(command[1].lower()+"@ig.ryzom.com", name))
			return True
		return False

	def removeFactionChannelForCharacter(self, command):
		if len(command) >= 3 and command[2] != "FACTION_RF":
			if command[2][:8] == "FACTION_":
				name = "⚜️  "+command[2][8:].title()
			else:
				name = "❇️  "+command[2]
			print(self.zulip.remove_subscriptions([name], principals = [command[1].lower()+"@ig.ryzom.com"]))
			return True
		return False

	def deleteMember(self, command):
		if len(command) >= 3:
			user_email = command[2].lower()+"@ig.ryzom.com"
			gid = int(self.getGuildId(command[1]))-0x6500000
			guild = "🔰 "+command[1]+f" ({gid:0>5X})"
			print(self.zulip.remove_subscriptions([guild], principals = [user_email]))
			print(self.zulip.call_endpoint(
					url="/users/"+user_email+"?profile_data="+urllib.parse.quote_plus("[{\"id\":"+CUSTOM_PROFILE_GUILD+", \"value\": \"""\"}]"),
					method="PATCH",
				))
			return True
		return False

	def manageMessage(self, i):
		command = self.getRyzomCommand(i)
		if command != None:
			print(f"New Command #{i} = {command}")
			self.updateActivity(False)
			if hasattr(self, command[0]) and callable(getattr(self, command[0])) and getattr(self, command[0])(command):
				self.stats["commands"] += 1
				self.updateStats()
		self.current_id = i
			
	def checkMessages(self):
		last_id = self.getLastCommandID()
		if self.current_id > last_id:
			print(f"Back from {self.current_id} to {last_id}")
			self.manageMessage(last_id)

		for i in range(self.current_id+1, last_id, 1):
			self.manageMessage(i)
		self.setLastManagedCommandID(self.current_id)

	def run(self):
		self.current_id = self.getLastManagedCommandID()
		print(f"Managing shard commands from {self.current_id}...")
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
