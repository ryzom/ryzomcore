#!/usr/bin/env python3

import os
import sys
import json
import time
import subprocess
import urllib.request
import configparser
from datetime import datetime as date
from functools import partial
p = partial(print, flush=True)

logfilename=sys.argv[1]

def follow(thefile):
	thefile.seek(0,os.SEEK_END)
	file_size = os.stat(logfilename).st_size
	while True:
		line = thefile.readline()
		if line:
			yield line
		else:
			size = os.stat(logfilename).st_size
			if file_size > size:
				sys.exit()
			file_size = size
			time.sleep(0.01)
		continue


if not os.path.isfile(logfilename):
	p("No log file")
	sys.exit(0)


config = configparser.ConfigParser(interpolation=None)
config.read("/etc/ryzom/api.ini")
RYAPI_SALT=config["ryapi"]["salt"].strip("\"")
p("Follow errors in {}".format(logfilename))

logfile = open(logfilename, "r", encoding="utf-8", errors="replace")
loglines = follow(logfile)
for line in loglines:
	if " : Outpost " in line or "setOutpostLevel" in line:
		p(line)
		data = urllib.parse.urlencode({"token": RYAPI_SALT, "line": line})
		req = urllib.request.Request("https://app.ryzom.com/app_arcc/check_nexus_outposts.php?"+data)
		with urllib.request.urlopen(req) as response:
			p(response.read())



