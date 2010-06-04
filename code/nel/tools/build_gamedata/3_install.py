#!/usr/bin/python
# 
# \file 3_install.py
# \brief Run all install processes
# \date 2009-02-18 16:19GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Run all install processes
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
sys.path.append("configuration")
if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite import *
from tools import *

sys.path.append(WorkspaceDirectory)
from projects import *

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Run the install processes")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")
mkPath(log, "configuration/project")
removeFilesRecursive(log, "configuration/project")
# For each project
for projectName in ProjectsToProcess:
	copyFilesRecursive(log, WorkspaceDirectory + "/" + projectName, "configuration/project")
	os.chdir("processes")
	try:
		subprocess.call([ "python", "3_install.py" ])
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
	removeFilesRecursive(log, WorkspaceDirectory + "/" + projectName)
	copyFilesRecursive(log, "configuration/project", WorkspaceDirectory + "/" + projectName)
	removeFilesRecursive(log, "configuration/project")
printLog(log, "")

log.close()
if os.path.isfile("3_install.log"):
	os.remove("3_install.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_install.log")
shutil.move("log.log", "3_install.log")
