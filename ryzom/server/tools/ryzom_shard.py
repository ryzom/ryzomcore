#!/bin/env python3
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
import time
import socket
import subprocess
import concurrent.futures

sys.stdout.reconfigure(line_buffering=True)

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))

services = (
	("AES", 0),
	("BMS_MASTER", 0),
	("RAS", 1),
	("RNS", 2),

	("LGS", 6),
	("LAS", 6),
	("MFS", 7),
	("MS", 7),
	("SBS", 8),
	("TS", 8),
	("FES", 9),
	("SU", 9),
	("RWS", 10),

#	("MOS", 5),
	("AIS_ARK", 11),
	("AIS_FYROS", 11),
	("AIS_MATIS", 11),
	("AIS_NEWBIELAND", 12),
	("AIS_ROOTS", 12),
	("AIS_TRYKER", 13),
	("AIS_ZORAI", 13),

	("EGS", 14),
	("GPMS", 15),
	("IOS", 16),
)

domain = socket.gethostname()
shard = domain.split(".")[0].lower()

def ribs_name(name):
	return "shard." + name.lower()


def start_service(name, retries=3, delay=2):
	print("=== Starting:", name)

	for attempt in range(retries):
		r = subprocess.run([os.path.join(TOOLS_DIR, "shard"), "start", ribs_name(name)])
		if r.returncode == 0:
			return
		print(f"\tRetry {attempt + 1}/{retries} for {name}...")
	
		time.sleep(delay)
	print(f"\tFailed to start {name} after {retries} attempts")



def stop_service(name, retries=3, delay=2):
	print("=== Stopping:", name)

	for attempt in range(retries):
		r = subprocess.run([os.path.join(TOOLS_DIR, "shard"), "stop", ribs_name(name)])
		if r.returncode == 0:
			return
		print(f"\tRetry {attempt + 1}/{retries} for {name}...")
		time.sleep(delay)
	print(f"\t{name} not running or failed to stop after {retries} attempts")


def stop_services_parallel(svcs, max_workers=5):
	with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
		executor.map(stop_service, [name for name, _ in svcs])


if __name__ == "__main__":
	command = sys.argv[1] if len(sys.argv) > 1 else ""
	service = sys.argv[2] if len(sys.argv) > 2 else ""
	print(f"Shard {command} {service} in {domain}")

	if command == "start":
		svcs = [(service.upper(), 0)] if service else list(services)
		stop_services_parallel(svcs)
		total = 0
		time.sleep(20)
		for i in range(20):
			time.sleep(1)
			for name, delay in svcs:
				if delay == i:
					total += 1
					start_service(name)
			if total >= len(svcs):
				break

	elif command == "stop":
		svcs = [(service.upper(), 0)] if service else list(services)
		stop_services_parallel(svcs)
