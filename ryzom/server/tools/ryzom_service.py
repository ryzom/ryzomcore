#!/usr/bin/env python
#############################################
# | ___ \|_   _|| ___ \/  ___|
# | |_/ /  | |  | |_/ /\ `--.
# |    /   | |  | ___ \ `--. \
# | |\ \ __| |__| |_/ //\__/ /
# \_| \_(_)___(_)____(_)____(_)
#
# Remote Interface to Bash Scripts
# - better with barcebue sauce -
# Copyright (C) 2019-2026 Nuneo (ulukyn@gmail.com)
#
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
import os
import sys
import signal
import socket
import psutil
import pexpect
import subprocess

from time import time, sleep

class RyzomService():
	FIFO_DIR = "/tmp/ribs_cmd"

	def __init__(self):
		super().__init__()
		self.name = "Ryzom_"+sys.argv[1].upper()
		self.last_update = time()
		self.start = time()
		self.log_sections = {}
		self.services_last_updates = {}
		self.service_name = sys.argv[1].lower()
		self.version = "0.1"
		self.shard = socket.gethostname()
		self.domain = "("+self.shard+")"
		self.fifo_path = os.environ.get("SERVICE_FIFO", os.path.join(self.FIFO_DIR, self.service_name))
		self.fifo_fd = None
		self.fifo_fd_write = None
		self.quitting = False

	def spawnService(self):
		os.chdir("/home/nevrax/shard/run")
		lock_file = "/tmp/updating_primitives.lock"
		if self.shard == "Gingo" and (self.service_name == "egs" or self.service_name[:4] == "ais_"):
			# Atomic lock creation — only the first service in wins the update,
			# the rest wait for the lock to be released instead of racing on it.
			try:
				lock_fd = os.open(lock_file, os.O_CREAT | os.O_EXCL)
			except FileExistsError:
				lock_fd = None

			if lock_fd is not None:
				os.close(lock_fd)
				print("Reset git primitives...")
				p = pexpect.spawn("git -C /home/nevrax/repos/ryzom-private-data/primitives/ checkout .")
				while p.isalive():
					print(p.readline().strip().decode("utf-8"))
				print("Get primitives from cloud...")
				p = pexpect.spawn("scp -r tools@cloud.ryzom.com:/home/data/nextcloud/ryzom/files/TEAMS/Level-Game-Design-Team/GINGO-PRIMITIVES-LiveUpdate/* /home/nevrax/shard/common/primitives/")
				while p.isalive():
					print(p.readline().strip().decode("utf-8"))
				if os.path.isfile(lock_file):
					os.unlink(lock_file)
			else:
				while True:
					if not os.path.isfile(lock_file):
						break
					sleep(1)
		print(os.getcwd())
		print(sys.argv[2:])
		self.p = pexpect.spawn(" ".join(sys.argv[2:]), encoding="utf-8", maxread=1)
		sleep(0.5)
		if not os.path.isdir(f"{self.service_name}"):
			os.makedirs(f"{self.service_name}")
		with open(f"{self.service_name}/{self.service_name}.state", "w") as f:
			f.write("RUNNING")
		self.service_pid = self.p.pid
		print(f"Service started with pid {self.service_pid}")
		sys.stdout.flush()

		os.makedirs(self.FIFO_DIR, exist_ok=True)
		if os.path.exists(self.fifo_path):
			os.unlink(self.fifo_path)
		os.mkfifo(self.fifo_path)
		self.fifo_fd = os.open(self.fifo_path, os.O_RDONLY | os.O_NONBLOCK)
		# Keep a write end open so reads never return EOF when no sender is connected
		self.fifo_fd_write = os.open(self.fifo_path, os.O_WRONLY | os.O_NONBLOCK)
		print(f"Command FIFO ready at {self.fifo_path}")
		sys.stdout.flush()

	def run(self):
		last_check = time()
		while True:
			if time() - last_check > 0.1:
				last_check = time()
				if not self.p.isalive():
					print("Service not alive...")
					subprocess.run(["schroot", "-e", "-c", self.schroot_session])
					sys_exit = 0 if self.quitting else -1
					print(f"Exit with {sys_exit}")
					sys.exit(sys_exit)
					break

				try:
					data = os.read(self.fifo_fd, 4096)
					if data:
						for line in data.decode("utf-8", "replace").splitlines():
							cmd = line.strip()
							if not cmd:
								continue
							print("Run cmd from FIFO:", cmd)
							sys.stdout.flush()
							try:
								self.p.sendline(cmd)
								if cmd == "quit":
									self.quitting = True
									sleep(5)
									if self.p.isalive():
										self.p.terminate()
									print("Service terminated by command. Ready to restart!")
							except:
								pass
				except BlockingIOError:
					pass

			try:
				data = self.p.read_nonblocking(size=4096, timeout=0.01)
				if data:
					sys.stdout.write(data)
					sys.stdout.flush()
			except:
				pass

		self.quit()

	def quit(self):
		if self.fifo_fd is not None:
			os.close(self.fifo_fd)
		if self.fifo_fd_write is not None:
			os.close(self.fifo_fd_write)
		if os.path.exists(self.fifo_path):
			os.unlink(self.fifo_path)
		if self.p.isalive():
			self.p.terminate()
		print("Bye!")

if __name__ == "__main__":
	service = RyzomService()
	service.spawnService()
	# RIBS itself already knows when a stop/restart was requested from the UI,
	# independently of this exit code. So a SIGTERM that isn't part of a
	# deliberate "quit" FIFO command means something killed this wrapper
	# unexpectedly — report a non-zero exit rather than masking it as clean.
	signal.signal(signal.SIGTERM, lambda s, f: (service.quit(), sys.exit(0 if service.quitting else 1)))
	try:
		service.run()
	except KeyboardInterrupt:
		service.quit()
