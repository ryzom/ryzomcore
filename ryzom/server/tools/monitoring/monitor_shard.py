#!/bin/env python3
#############################################
# ______                           _____ _                   _   _____           _
# | ___ \                         /  ___| |                 | | |_   _|         | |
# | |_/ /   _ _______  _ __ ___   \ `--.| |__   __ _ _ __ __| |   | | ___   ___ | |___
# |    / | | |_  / _ \| '_ ` _ \   `--. \ '_ \ / _` | '__/ _` |   | |/ _ \ / _ \| / __|
# | |\ \ |_| |/ / (_) | | | | | | /\__/ / | | | (_| | | | (_| |   | | (_) | (_) | \__ \
# \_| \_\__, /___\___/|_| |_| |_| \____/|_| |_|\__,_|_|  \__,_|   \_/\___/ \___/|_|___/
#        __/ |
#       |___/
#
# Ryzom Shard Tools - with delicious M.A.R.G.U.E.Z
# Copyright (C) 2025 Winch Gate Property Limited - Ulukyn (<nuno@troispetits.net>)
# This program is free software: read https://ryzom.com/dev/copying.html for more details
#
# Start a Ryzom Service and monitor it
# All Services can be managed by shard.py
#


import os
import sys
import glob
import json
import math
import time

from pymemcache.client.base import Client
from pynel.admin_modules_itf import CAdminServiceWeb

class MonitorServices():

	def __init__(self):
		self.client = Client("localhost")
		self.shard = sys.argv[1].lower()
		self.ryzomAS = CAdminServiceWeb()
		self.ryzomAsStatus = self.ryzomAS.connect("127.0.0.1", 46700)

	def parseState(self, state):
		values = {}
		for var in state.split("\t"):
			svar = var.split("=")
			if len(svar) == 2:
				values[svar[0].lower()] = svar[1]
		return values

	def run(self):
		last_ras_update = time.time()
		updates = 0
		noReportSince = 0
		states = {}
		icons = {"?": "⛔️", "ds_closed" : "🚷", "ds_dev" : "♻️", "ds_restricted" : "🕵🏽", "ds_open" : "✅", "started" : "✅", "ds_start" : "🚀", "starting" : "🚀", "shard_stoppped" : "😵", "slow" : "🥵", "WaitingMirrorReady" : "🚀", "Online" : "✅", "ReadWrite" : "✅", "Mapping names": "🚀", "Open" : "✅", "stopped" : "😵️"}

		name = ""
		server_open_status = ""
		last_status = {}
		first_error = True
		while(True):
			if time.time() - last_ras_update > 1:
				updates += 1

				with open("/home/nevrax/www/login/server_open_status", "r") as f:
					new_open_status = f.read().strip()
					if server_open_status != new_open_status:
						print("Server Open Status =", new_open_status)
						self.client.set("Dag-MORS-Services", json.dumps([new_open_status]))
					server_open_status = new_open_status

				if self.ryzomAsStatus:
					try:
						states = self.ryzomAS.get_states()
					except Exception as e:
						print("Error getting states", e)
						self.ryzomAS.close()
						self.ryzomAsStatus = False
						time.sleep(1)
						continue

					if not states:
						last_status = {}
						self.ryzomAS.close()
						self.ryzomAsStatus = False
						for file in glob.glob(f"/tmp/ribs_status/shard.*"):
							status = ""
							with open(file) as f:
								status = f.readlines()[0]
							if status != "ERROR":
								with open(file, "w") as f:
									f.write(f"INFO\n❓")
						time.sleep(1)
						continue

					first_error = True

					services = [server_open_status]
					for state in states:
						vals = self.parseState(state)
						name = vals["aliasname"]
						if "LAS" in name:
							name = "las"
						services.append(name)
						noReportSince = int(vals["noreportsince"]) if "noreportsince" in vals else -1
						infos = {}
						infos["start"] = (time.time() - int(vals["uptime"])) if "uptime" in vals else 0
						infos["speed"] = vals["tickspeedloop"] if "tickspeedloop" in vals else "-"
						infos["state"] = vals["state"] if "state" in vals else "?"
						infos["lastupdate"] = time.time()-noReportSince

						if infos["state"] == "Online":
							if noReportSince > 10:
								status = icons["stopped"]
							elif noReportSince > 1:
								status = icons["slow"]
							else:
								status = icons["ds_open"]
						elif infos["state"] == "":
							status = icons["ds_start"]
						else:
							if infos["state"] in icons:
								status = icons[infos["state"]]
							else:
								status = icons["starting"]


						infos["noreportsince"] = noReportSince
						uptime = vals["uptime"] if "uptime" in vals else "-"
						infos_out = "[orange1]State:[/orange1] "+infos["state"]+"\t[orange1]Uptime:[/orange1] "+uptime+\
							"\n[orange1]Tick SL:[/orange1] "+infos["speed"]+"\t[orange1]Last Update:[/orange1] "+str(noReportSince)

						self.client.set(f"Dag-{name}-RawInfos", json.dumps(infos))
						self.client.set(f"Dag-{name}-Infos", infos_out)
						self.client.set(f"Dag-{name}-Status", status.encode("utf-8"))
						self.client.set(f"Dag-{name}-LastUpdate", time.time())
						if not name in last_status:
							last_status[name] = ""
						if status != last_status[name]:
							print(f"write {name}")
							with open(f"/tmp/ribs_status/shard.{name}", "w") as f:
								f.write(f"INFO\n{status}")
							last_status[name] = status
					self.client.set("Dag-MORS-Services", json.dumps(services))
					updates = 0
				else:
					last_status = {}
					if first_error:
						print("RAS not connected")
						for file in glob.glob(f"/tmp/ribs_status/shard.*"):
							status = ""
							with open(file) as f:
								status = f.readlines()[0]
							if status != "ERROR":
								with open(file, "w") as f:
									f.write(f"INFO\n❓")
						first_error = False
					time.sleep(1)
					self.ryzomAsStatus = self.ryzomAS.connect("127.0.0.1", 46700)
				last_ras_update = time.time()
			time.sleep(0.01)

if __name__ == "__main__":
	print(f"Start Monitoring Ryzom Services for {sys.argv[1]}...")
	service = MonitorServices()
	service.run()
