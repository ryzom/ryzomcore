#!/usr/bin/env python3

import sys, os, time
import pyinotify
from subprocess import Popen, PIPE
from datetime import datetime

class OnWriteHandler(pyinotify.ProcessEvent):

	def process_IN_CREATE(self, event):
		print("Core Dump:", event.name)
		pid = event.name.split(".")[-3]
		is_bad = True
		while(is_bad):
			print("Getting backtrace...")
			p = Popen(["coredumpctl", "gdb" , pid], stdout=PIPE, stdin=PIPE, stderr=PIPE, text=True)
			stdout_data = p.communicate(input="bt\n")[0]
			for line in stdout_data.split("\n"):
				if "Command Line" in line:
					sline = line.split("../sbin/")
					if len(sline) > 1:
						line = line[1]
						sbin = line.split(" ")[0]
						if sbin == "ryzom_ai_service":
							sbin += "_"+line.split(" -N")[1].split(" ")[0]
						is_bad = False
					else:
						return
			time.sleep(1)
		with open("/home/nevrax/shard/crashs/"+sbin+"_"+datetime.today().strftime("%Y_%m_%d_%H_%M_%S.log"), "w") as f:
			f.write(stdout_data)
		Popen(["rm_coredump", event.name])

def monitor(path):
	wm = pyinotify.WatchManager()
	handler = OnWriteHandler()
	notifier = pyinotify.Notifier(wm, default_proc_fun=handler)
	wm.add_watch(path, pyinotify.ALL_EVENTS, rec=True, auto_add=True)
	print("==> Start monitoring %s (type c^c to exit)" % os.path.abspath(path))
	notifier.loop()

if __name__ == "__main__":
	path = sys.argv[1]
	monitor(path)
