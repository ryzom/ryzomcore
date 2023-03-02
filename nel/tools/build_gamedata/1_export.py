#!/usr/bin/python
# 
# \file 1_export.py
# \brief Run all export processes
# \date 2009-02-18 09:22GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Run all export processes
# 
# NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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

import time, sys, os, shutil, subprocess, distutils.dir_util, argparse
sys.path.append("configuration")

parser = argparse.ArgumentParser(description='Ryzom Core - Build Gamedata - Export')
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
from buildsite_local import *
from tools import *

sys.path.append(WorkspaceDirectory)
from projects import *

NeLWorkspaceDir = None
if NeLConfigDir:
	NeLWorkspaceDir = os.path.join(NeLConfigDir, "workspace")

# Log error
printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Run the export processes")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")
# For each project
for projectName in ProjectsToProcess:
	if ((args.includeproject == None or projectName in args.includeproject) and (args.excludeproject == None or not projectName in args.excludeproject)):
		printLog(log, "PROJECT " + projectName)
		if os.path.isfile(os.path.join(os.path.join(NeLWorkspaceDir, projectName), "process.py")):
			os.putenv("NELBUILDACTIVEPROJECT", os.path.abspath(os.path.join(NeLWorkspaceDir, projectName).replace("\\", "/")))
		else:
			os.putenv("NELBUILDACTIVEPROJECT", os.path.abspath(WorkspaceDirectory + "/" + projectName))
		os.chdir("processes")
		try:
			if not args.includeprocess == None:
				subprocess.call([ "python", "1_export.py", "--includeprocess" ] + args.includeprocess)
			elif not args.excludeprocess == None:
				subprocess.call([ "python", "1_export.py", "--excludeprocess" ] + args.excludeprocess)
			else:
				subprocess.call([ "python", "1_export.py" ])
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
if os.path.isfile("1_export.log"):
	os.remove("1_export.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_export.log")
shutil.move("log.log", "1_export.log")
