#!/bin/env python3

import os
import sys
import json
import yaml
import time
import glob
import psutil
import shutil

import subprocess
import requests_openapi
from marguez.app import Marguez

shard_path = os.getenv("SHARD_PATH")
if not shard_path:
	shard_path="/home/nevrax/shard/"

dagu_path = os.getenv("DAGU_PATH")
if not dagu_path:
	dagu_path="/home/data/dagu/"

services = (
("AES", 0),
("BMS_MASTER", 5),
("FES", 5),
("LAS", 5),
("LGS", 5),
("MFS", 5),
("MOS", 5),
("MS", 5),
("RAS", 5),
("RNS", 5),
("RWS", 5),
("SBS", 5),
("SU", 5),
("TS", 5),
("AIS_ARK", 10),
("AIS_FYROS", 10),
("AIS_MATIS", 10),
("AIS_NEWBIELAND", 10),
("AIS_ROOTS", 10),
("AIS_TRYKER", 10),
("AIS_ZORAI", 10),
("EGS", 15),
("GPMS", 15),
("IOS", 15),
)

client = None

def stopService(name):
	global client
	if not client:
		client = requests_openapi.Client().load_spec_from_file(shard_path+"/tools/dagu_api.yaml")
		client.set_server(requests_openapi.Server(url="http://gingo.ryzom.com:9888/api/v2"))

	infos = client.getDAGDetails(fileName="Ryzom_"+name).json()
	print("Terminate:", name)
	client.dequeueDAGRun(name=infos["dag"]["name"], dagRunId=infos["latestDAGRun"]["dagRunId"])
	client.terminateDAGRun(name=infos["dag"]["name"], dagRunId=infos["latestDAGRun"]["dagRunId"])

def cleanService(name):
	with open(dagu_path+"dags/Ryzom_"+name+".yaml", "r") as file:
		config = yaml.safe_load(file)
	cmd = config["steps"][0]["command"].split(" ", 3)[3]
	schroot = None
	python = None
	for proc in psutil.process_iter():
		n = proc.name()
		if cmd in " ".join(proc.cmdline()):
			if n == "schroot":
				schroot = proc
			elif n == "python3":
				python = proc
			else:
				print("KILL:", n, proc.pid)
				proc.terminate()
				sys.stdout.flush()

	if schroot:
		time.sleep(0.5)
		print("KILL: schroot", schroot.pid)
		try:
			schroot.terminate()
		except:
			pass
		sys.stdout.flush()

	if python:
		time.sleep(0.5)
		print("KILL: python", python.pid)
		try:
			python.terminate()
		except:
			pass
		sys.stdout.flush()

	shutil.rmtree(dagu_path+"history/queue/Ryzom "+name, True)
	shutil.rmtree(dagu_path+"history/proc/Ryzom "+name, True)
	import glob, os
	for f in glob.glob("/tmp/@dagu_Ryzom_"+name+"_*.sock"):
		os.remove(f)

def stopServices(services):
	for name, null in services:
		stopService(name)
	time.sleep(0.5)
	for name, null in services:
		cleanService(name)
	if len(services) > 1:
		shutil.rmtree(shard_path+"/run/", True)
		os.makedirs(shard_path+"/run/")

if __name__ == "__main__":
	command = sys.argv[1] if len(sys.argv) > 1 else ""
	service = sys.argv[2] if len(sys.argv) > 2 else ""

	if command == "start":
		if service:
			services = [(service.upper(), 0)]
		total = 0
		stopServices(services)
		for i in range(20):
			time.sleep(1)
			for (name, sleep) in services:
				if sleep == i:
					total += 1
					print("Starting:", name)
					subprocess.Popen(["dagu", "enqueue", "Ryzom_"+name, "-c", "/etc/dagu.yaml"])
			if total >= len(services):
				break

	elif command == "stop":
		if service:
			services = [(service.upper(), 0)]
		stopServices(services)
	else:
		app = Marguez()
		app.parseArgs()
		app.register(shard_path, dagu_path)
		app.run()


