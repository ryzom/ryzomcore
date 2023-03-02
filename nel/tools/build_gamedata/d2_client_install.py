#!/usr/bin/python
# 
# \file d2_client_install.py
# \brief Install to client install
# \date 2009-02-18 16:19GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install to client install
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

import time, sys, os, shutil, subprocess, distutils.dir_util
sys.path.append("configuration")

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite_local import *
from tools import *

sys.path.append(WorkspaceDirectory)
from projects import *

# Log error
printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Install to client install")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

ExeDllCategories = set()

try:
	ClientInstalls
	ExeDllCategories.add("main_exedll")
except NameError:
	ClientInstalls = []
	ClientInstalls += [ { "Category": "main_exedll" } ]

for clientInstall in ClientInstalls:
	ExeDllCategories.add(clientInstall["Category"])

for clientInstall in ClientInstalls:
	if "Directory" in clientInstall:
		printLog(log, "CLIENT INSTALL " + clientInstall["Directory"])
	for category in InstallClientData:
		if category["Name"] in ExeDllCategories and category["Name"] != clientInstall["Category"]:
			printLog(log, "SKIP CATEGORY " + category["Name"])
			continue
		printLog(log, "CATEGORY " + category["Name"])
		packExt = ".bnp"
		if (category["StreamedPackages"]):
			packExt = ".snp"
		targetPath = ClientInstallDirectory
		if "Directory" in clientInstall:
			targetPath = targetPath + "/" + clientInstall["Directory"]
		if (category["UnpackTo"] != None):
			if (category["UnpackTo"] != ""):
				targetPath += "/" + category["UnpackTo"]
			mkPath(log, targetPath)
			for package in category["Packages"]:
				printLog(log, "PACKAGE " + package[0])
				mkPath(log, InstallDirectory + "/" + package[0])
				copyFilesNoTreeIfNeeded(log, InstallDirectory + "/" + package[0], targetPath)
		else:
			sourcePath = ClientPatchDirectory + "/bnp"
			targetPath = targetPath + "/data"
			mkPath(log, targetPath)
			for package in category["Packages"]:
				printLog(log, "PACKAGE " + package[0])
				sourceBnp = sourcePath + "/" + package[0] + packExt
				targetBnp = targetPath + "/" + package[0] + packExt
				if (len(package[1]) > 0):
					sourceBnp = sourcePath + "/" + package[1][0]
					targetBnp = targetPath + "/" + package[1][0]
					printLog(log, "TARGET " + package[1][0])
				copyFileIfNeeded(log, sourceBnp, targetBnp)
			for ref in category["Refs"]:
				printLog(log, "REFERENCE " + ref)
				sourceRef = sourcePath + "/" + ref + "_.ref"
				targetRef = targetPath + "/" + ref + "_.ref"
				copyFileIfNeeded(log, sourceRef, targetRef)
	printLog(log, "")

log.close()
if os.path.isfile("d2_client_install.log"):
	os.remove("d2_client_install.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_client_install.log")
shutil.move("log.log", "d2_client_install.log")
