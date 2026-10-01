#!/usr/bin/env python3

import os
import sys
import urllib.request
import urllib.parse
import configparser
from functools import partial

from pynel.log_follower import LogFollower

p = partial(print, flush=True)

logfilename=sys.argv[1]

if not os.path.isfile(logfilename):
	p("No log file")
	sys.exit(0)


config = configparser.ConfigParser(interpolation=None)
config.read("/etc/ryzom/api.ini")
RYAPI_SALT=config["ryapi"]["salt"].strip("\"")
p("Follow errors in {}".format(logfilename))

for line in LogFollower(logfilename):
	if " : Outpost " in line or "setOutpostLevel" in line:
		p(line)
		data = urllib.parse.urlencode({"token": RYAPI_SALT, "line": line})
		req = urllib.request.Request("https://app.ryzom.com/app_arcc/check_nexus_outposts.php?"+data)
		with urllib.request.urlopen(req) as response:
			p(response.read())
