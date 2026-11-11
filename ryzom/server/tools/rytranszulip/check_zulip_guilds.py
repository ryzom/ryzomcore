#!/usr/bin/env python3

import json
import zulip
import shlex
import urllib
import mysql.connector
import requests
import subprocess
import configparser
import xml.etree.ElementTree as ET

cfg = configparser.ConfigParser()
cfg.read("/etc/ryzom/shard.ini")
c = cfg["db_ring"]

CUSTOM_PROFILE_GUILD="3"

DRY_RUN = False
REMOTE = "zulip@chat.ryzom.com"
MANAGE = "/home/zulip/deployments/current/manage.py"

root = ET.fromstring(requests.get("https://api.ryzom.com/guilds.php", timeout=30).content)
gid_by_name = {g.findtext("name").lower(): int(g.findtext("gid")) for g in root.iter("guild")}

def getGuildChannel(guild_id, name):
	return "🔰 "+name+f" ({guild_id-0x6500000:0>5X})"

# Pass the path to your zuliprc file here.
#client = zulip.Client(config_file="/etc/ryzom/csr_bot.rc")
client = zulip.Client(config_file="~/.zuliprc")
users = client.get_users({"include_custom_profile_fields": True})["members"]

declared = {}
for u in users:
	value = (u.get("profile_data") or {}).get(CUSTOM_PROFILE_GUILD, {}).get("value")
	g_id = 0
	if not "delivery_email" in u or not u["delivery_email"]:
		name = u["full_name"].lower()
	else:
		name = u["delivery_email"].split("@")[0].lower()
	if value:
		guild = value[2:-2].split(" (")
		g_name = guild[0]
		if g_name.lower() in gid_by_name:
			g_id = gid_by_name[g_name.lower()]
	if not "@" in u["full_name"] and u["is_active"] and not u["is_guest"] and not u["is_bot"] and u["delivery_email"] and not "(" in u["delivery_email"] and not " " in u["delivery_email"]:
		declared[name] = (u["user_id"], u["delivery_email"].lower(), g_id, g_name.lower())

db = mysql.connector.connect(
	host=c["host"],
	user=c["user"],
	password=c["pass"],
	database=c["name"],
)
cur = db.cursor()

names = list(declared)
in_db = {}
for i in range(0, len(names), 1000):
	chunk = names[i:i + 1000]
	cur.execute(
		f"SELECT char_name, guild_id FROM characters WHERE char_name IN ({','.join(['%s'] * len(chunk))})",
		chunk,
	)
	for char_name, guild_id in cur.fetchall():
		in_db[char_name.lower()] = guild_id

to_delete = []
for name, (zid, email, guild, guild_name) in declared.items():
	actual = in_db.get(name)
	if actual is None:
		#print(f"PERSO ABSENT     {zid} {email}")
		to_delete.append(email)

for i in range(0, len(to_delete), 5):
	chunk = to_delete[i:i + 5]
	emails = ",".join(chunk)
	remote_cmd = f"{MANAGE} delete_user -u {emails}"
	if not DRY_RUN:
		remote_cmd += " -f"
	print("Delete...")
	print(remote_cmd)
	subprocess.run(["ssh", REMOTE, remote_cmd], check=False)

for name, (zid, email, guild, guild_name) in declared.items():
	actual = in_db.get(name)
	if actual != guild and guild != 0:
		g_name = getGuildChannel(guild, guild_name)
		print(f"BAD GUILD  {g_name} for {name}: déclarée '{guild}', en base {actual}")
		print(client.call_endpoint(
			url="/users/"+email+"?profile_data="+urllib.parse.quote_plus("[{\"id\":"+CUSTOM_PROFILE_GUILD+", \"value\": \"""\"}]"),
			method="PATCH",
		))
		print(client.remove_subscriptions([g_name], principals=[email]))

