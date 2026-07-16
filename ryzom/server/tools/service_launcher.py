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
# Base classe for a Ryzom Shard Service monitor
#

import os
import sys
import time
import socket
import psutil
import pexpect
import subprocess

from pathlib import Path
from marguez.service import Service

class ShardService(Service):

	def __init__(self):
		super().__init__()
		self.service_name = sys.argv[1].lower()
		self.name = "Ryzom_"+sys.argv[1].upper()
		self.version = "0.1"
		self.shard = socket.gethostname()
		self.domain = "("+self.shard+")"
		self.updateStats()

	def updateStats(self):
		self.infos = " \n[orange1]Booting up...[/orange1]"
		self.updateInfos()

	def spawnService(self):
		os.chdir("/home/nevrax/shard/run")
		lock_file = "/tmp/updating_primitives.lock"
		if self.shard == "Gingo" and (self.service_name == "egs" or self.service_name[:4] == "ais_"):
			if self.service_name == "egs" or not os.path.isfile(lock_file):
				Path(lock_file).touch()
				print("Reset git primitives...")
				p = pexpect.spawn("git -C /home/nevrax/repos/ryzom-private-data/primitives/ checkout .")
				while p.isalive():
					print(p.readline().strip().decode("utf-8"))
				print("Get primitives from cloud...")
				p = pexpect.spawn("scp -r tools@cloud.ryzom.com:/home/data/nextcloud/ryzom/files/TEAMS/Level-Game-Design-Team/GINGO-PRIMITIVES-LiveUpdate/* /home/nevrax/shard/common/primitives/")
				while p.isalive():
					print(p.readline().strip().decode("utf-8"))
				os.unlink(lock_file)
			else:
				while True:
					if not os.path.isfile(lock_file):
						break
					time.sleep(1)

		cmd = " ".join(["schroot", "-p", "-c", "atys", "-b"])
		print(cmd)
		self.schroot = pexpect.spawn(cmd, encoding="utf-8")
		time.sleep(0.5)
		self.schroot_session = self.schroot.readline().strip()
		cmd = " ".join(["schroot", "-p", "-r", "-c", str(self.schroot_session), "--"]+sys.argv[2:])
		print(cmd)
		self.p = pexpect.spawn(" ".join(["schroot", "-p", "-r", "-c", str(self.schroot_session), "--"]+sys.argv[2:]), encoding="utf-8", logfile=sys.stdout, maxread=1)
		self.p.logfile = None
		time.sleep(0.5)
		if not os.path.isdir(f"{self.service_name}"):
			os.makedirs(f"{self.service_name}")
		with open(f"{self.service_name}/{self.service_name}.state", "w") as f:
			f.write("RUNNING")
		self.schroot_pid = self.p.pid
		# Check subprocessus
		pid = None
		while not pid:
			current_process = psutil.Process(self.schroot_pid)
			children = current_process.children(recursive=True)
			for child in children:
				pid = child.pid
			time.sleep(0.1)
		self.service_pid = pid
		self.setMessage(f"Dag-{self.name}-Pid", self.service_pid)
		print(f"Service started in chroot {self.schroot_session} with pid {self.service_pid}")
		sys.stdout.flush()
		self.register()

	def run(self):
		cid = self.getMessage(f"DagRun-{self.name}-Id")
		if cid:
			cid = int(cid)
		else:
			cid = 0
			self.setMessage(f"DagRun-{self.name}-Id", 0)
		
		last_check = time.time()
		while(True):
			
			if time.time() - last_check > 0.1:
				last_check = time.time()
				if not self.p.isalive():
					print("Service not alive...")
					subprocess.run(["schroot", "-e", "-c", self.schroot_session])
					sys.exit(-1)
					break

				
				new_cid = int(self.getMessage(f"DagRun-{self.name}-Id"))
				
				while cid < new_cid:
					cid += 1
					cmd = self.getMessage(f"DagRun-{self.name}-{cid}")
					print("Run cmd from Marguez:", cmd)
					sys.stdout.flush()
					try:
						self.p.sendline(cmd)
						if cmd == "quit":
							time.sleep(5)
							if self.p.isalive():
								self.p.terminate()
							print("Service terminated by command. Ready to restart!")
					except:
						pass

			try:
				line = self.p.read_nonblocking(timeout=0.01)
			except:
				pass

		self.quit()

	def quit(self):
		if self.p.isalive():
			self.p.terminate()
		subprocess.run(["schroot", "-e", "-c", self.schroot_session])
		print("Bye!")

if __name__ == "__main__":
	service = ShardService()
	service.spawnService()
	service.register()
	try:
		service.run()
	except KeyboardInterrupt:
		service.quit()
