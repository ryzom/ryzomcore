#!/usr/bin/env python3

import sys, os, time
import pyinotify
import zulip
import configparser
from subprocess import Popen, PIPE
from datetime import datetime

config = configparser.ConfigParser()
config.read("/etc/ryzom/rtz.ini")
zulip_client = zulip.Client(email=config["zulip"]["email"], api_key=config["zulip"]["key"], site=config["zulip"]["site"])
config.read("/etc/ryzom/shard.ini")

def sendNotif(message):
	global config, zulip_client
	message_type = "stream"
	user = "Ry Notify"
	request = {
		"type": message_type,
		"to": "📈 MONITORING",
		"topic": config["shard"]["emoji"]+" "+config["shard"]["name"].title()+" - 💔 Crashs",
		"content": user[0].upper()+user[1:]+":"+message,
	}
	result = zulip_client.send_message(request)
	print("Result:", result)

def export_gdb(pid):
	date = datetime.today().strftime("%Y_%m_%d_%H_%M_%S")
	sbin = ""
	if os.path.isfile("/tmp/"+pid+".core"):
		p = Popen(["file", "/tmp/"+pid+".core"],  stdout=PIPE, stdin=PIPE, stderr=PIPE, text=True)
		stdout_data = p.communicate(input="\n")[0]
		print(stdout_data)
		sbin = stdout_data.split(", execfn: '../sbin/")[1].split("'")[0]
	else:
		print("Dumping core...")
		p = Popen(["coredumpctl", "dump" , "-q", "--output", "/tmp/"+pid+".core", pid],  stdout=PIPE, stdin=PIPE, stderr=PIPE, text=True)
		stdout_data = p.communicate(input="\n")[0]
		for line in stdout_data.split("\n"):
			if "Command Line:" in line:
				sline = line.split("../sbin/")
				if len(sline) > 1:
					line = sline[1]
					sbin = line.split(" ")[0]
				else:
					return
			elif "Timestamp:" in line:
				sline = line.split("Timestamp:")[1].strip().split(" ")
				date = sline[1]+"_"+sline[2]

	if not sbin:
		print("Invalid core")
		return
	print("Getting backtrace with gdb of", sbin, "...")
	p = Popen(["schroot", "-c", "atys", "--", "gdb", "/home/nevrax/shard/sbin/"+sbin, "/tmp/"+pid+".core"], stdout=PIPE, stdin=PIPE, stderr=PIPE, text=True)
	stdout_data = p.communicate(input="bt\n")[0]
	if sbin == "ryzom_ai_service":
		logname = sbin+"_"+line.split(" -N")[1].split(" ")[0]
	else:
		logname = sbin
	with open("/home/nevrax/shard/crashs/"+date+"_"+logname+".log", "w") as f:
		f.write(stdout_data)
	sendNotif("Crash of **"+sbin+"**\n```\n"+stdout_data+"\n```\n")
	sys.stdout.flush()

class OnWriteHandler(pyinotify.ProcessEvent):

	def process_IN_CREATE(self, event):
		print("Core Dump:", event.name)
		pid = event.name.split(".")[-3]
		export_gdb(pid)
		Popen(["sudo", "rm_coredump", corefile])

def monitor(path):
	print("Monitoring Core dumps...")
	sys.stdout.flush()
	for corefile in os.listdir(path):
		print("Core Dump:", corefile)
		pid = corefile.split(".")[-3]
		export_gdb(pid)
		Popen(["sudo", "rm_coredump", corefile])
	wm = pyinotify.WatchManager()
	handler = OnWriteHandler()
	notifier = pyinotify.Notifier(wm, default_proc_fun=handler)
	wm.add_watch(path, pyinotify.ALL_EVENTS, rec=True, auto_add=True)
	print("==> Start monitoring %s (type c^c to exit)" % os.path.abspath(path))
	notifier.loop()

if __name__ == "__main__":
	path = sys.argv[1]
	monitor(path)

