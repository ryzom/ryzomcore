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

import time, sys, os, shutil, subprocess, distutils.dir_util, argparse
sys.path.append("configuration")

parser = argparse.ArgumentParser(description='Ryzom Core - Build Gamedata - Build')
# parser.add_argument('--haltonerror', '-eh', action='store_true')
parser.add_argument('--includeproject', '-ipj', nargs='+')
parser.add_argument('--excludeproject', '-epj', nargs='+')
parser.add_argument('--includeprocess', '-ipc', nargs='+')
parser.add_argument('--excludeprocess', '-epc', nargs='+')
args = parser.parse_args()

if not args.includeproject == None and not args.excludeproject == None:
	print "ERROR --includeproject cannot be combined with --excludeproject, exit."
	exit()

if not args.includeprocess == None and not args.excludeprocess == None:
	print "ERROR --includeprocess cannot be combined with --excludeprocess, exit."
	exit()

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite import *
from tools import *

sys.path.append(WorkspaceDirectory)
from projects import *

# Log error
printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Run the build processes")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")
# For each project
for projectName in ProjectsToProcess:
	if ((args.includeproject == None or projectName in args.includeproject) and (args.excludeproject == None or not projectName in args.excludeproject)):
		printLog(log, "PROJECT " + projectName)
		os.putenv("NELBUILDACTIVEPROJECT", os.path.abspath(WorkspaceDirectory + "/" + projectName))
		os.chdir("processes")
		try:
			if not args.includeprocess == None:
				subprocess.call([ "python", "2_build.py", "--includeprocess" ] + args.includeprocess)
			elif not args.excludeprocess == None:
				subprocess.call([ "python", "2_build.py", "--excludeprocess" ] + args.excludeprocess)
			else:
				subprocess.call([ "python", "2_build.py" ])
		except Exception, e:
			printLog(log, "<" + projectName + "> " + str(e))
		os.chdir("..")
		try:
			projectLog = open("processes/log.log", "r")
			projectLogData = projectLog.read()
			projectLog.close()
			log.write(projectLogData)
		except Exception, e:
			printLog(log, "<" + projectName + "> " + str(e))
	else:
		printLog(log, "IGNORE PROJECT " + projectName)
printLog(log, "")

log.close()
if os.path.isfile("2_build.log"):
	os.remove("2_build.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_build.log")
shutil.move("log.log", "2_build.log")
