#!/usr/bin/python
# 
# \file 2_build.py
# \brief Run all build processes
# \date 2009-02-18 09:22GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Run all build processes
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2010  Winch Gate Property Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 

import time, sys, os, shutil, subprocess, distutils.dir_util
sys.path.append("../configuration")

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite import *
from process import *
from tools import *
from directories import *

# Log error
printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Run the build processes")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")
# For each process
for processName in ProcessToComplete:
	os.chdir(processName)
	try:
		subprocess.call([ "python", "2_build.py" ])
	except Exception, e:
		printLog(log, "<" + processName + "> " + str(e))
	os.chdir("..")
	try:
		processLog = open(processName + "/log.log", "r")
		processLogData = processLog.read()
		processLog.close()
		log.write(processLogData)
	except Exception, e:
		printLog(log, "<" + processName + "> " + str(e))
	# subprocess.call("idle.bat")
printLog(log, "")

log.close()
