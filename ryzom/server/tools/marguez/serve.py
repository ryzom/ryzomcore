#############################################
#   __   __  _______  ______    _______  __   __  _______  _______ 
#  |  |_|  ||   _   ||    _ |  |       ||  | |  ||       ||       |
#  |       ||  |_|  ||   | ||  |    ___||  | |  ||    ___||____   |
#  |       ||       ||   |_||_ |   | __ |  |_|  ||   |___  ____|  |
#  |       ||       ||    __  ||   ||  ||       ||    ___|| ______|
#  | ||_|| ||   _   ||   |  | ||   |_| ||       ||   |___ | |_____ 
#  |_|   |_||__| |__||___|  |_||_______||_______||_______||_______|
# 
# M.A.R.G.U.E.Z (The Dyslexic Merguez)
# Copyright (C) 2025 Nuneo (nuno@troispetits.net)
#
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# Textual-Ganglion client to serve a web version of app
#

import os
import sys
import asyncio
import logging
import socket

from textual_web.ganglion_client import GanglionClient
from textual_web.config import load_config, default_config
from textual_web.environment import get_environment

args = sys.argv[1:]
if not args or ".py" in args[0]:
	print("Syntax: {} UniqID".format(sys.argv[0]))
	sys.exit(1)

logname = "servs/"+args[0]+".log"
pidname = "servs/"+args[0]+".pid"
args = sys.argv[2:]

class SimpleHandler(logging.Handler):
	def emit(self, record):
		global ganglion_client
		print(record)
		if "<ganglion>" in str(record):
			infos = str(record).split(" ")
			if infos[6] == "Serving":
				with open(logname, "a") as f:
					f.write(infos[7][:-2]+"\n")
				print(infos[7][:-2])
			else:
				try:
					socket.inet_aton(infos[6])
				except socket.error:
					pass
				else:
					# Force recreate of pid if delete by clean cron
					with open(pidname, "w") as f:
						f.write("")
					if len(infos) > 13:
						with open(logname, "a") as f:
							f.write("> "+infos[6]+"\n")
					else:
						print("bye")
						#os.remove(logname)
						#ganglion_client.force_exit()
						#sys.exit(0)



FORMAT = "%(message)s"
logging.basicConfig(
	level="INFO",
	format=FORMAT,
	datefmt="",
	handlers=[SimpleHandler()],
)

def app(appname, args):
	print("Start", appname)
	_environment = get_environment("local")
	_config = default_config()

	ganglion_client = GanglionClient(
		"./",
		_config,
		_environment,
		api_key="SASMSSD88A47",
		devtools=True,
		exit_on_idle=True,
		web_interface=False,
	)
	if args:
		args = " ".join(args)
	else:
		args = ""
	app_command = "textual run --dev "+appname+" --web "+args
	app_command = "textual-web -r "+appname+" -e local"+args
	ganglion_client.add_app(appname, app_command, "")

	import uvloop
	if sys.version_info >= (3, 11):
		with asyncio.Runner(loop_factory=uvloop.new_event_loop) as runner:
			runner.run(ganglion_client.run())
	else:
		uvloop.install()
		asyncio.run(ganglion_client.run())


if __name__ == "__main__":
	if args:
		appname = args[0]
		app(appname, args[1:])
	else:
		app("app.py", args)
