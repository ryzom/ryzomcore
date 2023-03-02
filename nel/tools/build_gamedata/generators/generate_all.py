#!/usr/bin/python
# 
# \file 0_setup.py
# \brief Run all setup processes
# \date 2009-02-18 15:28GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Run all setup processes
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2009-2014  by authors
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
if os.path.isfile("generate_all.log"):
	os.remove("generate_all.log")
log = open("generate_all.log", "w")
from scripts import *
from buildsite import *
from tools import *

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Generate all")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")


try:
	subprocess.call([ "python", "generate_simple_max_exporters.py" ])
except Exception, e:
	printLog(log, "<" + processName + "> " + str(e))
printLog(log, "")


try:
	subprocess.call([ "python", "generate_tagged_max_exporters.py" ])
except Exception, e:
	printLog(log, "<" + processName + "> " + str(e))
printLog(log, "")


try:
	subprocess.call([ "python", "generate_ecosystem_projects.py" ])
except Exception, e:
	printLog(log, "<" + processName + "> " + str(e))
printLog(log, "")


log.close()
