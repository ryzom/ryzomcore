#!/usr/bin/python
# 
# \file b1_client_dev.py
# \brief Install to client dev
# \date 2009-02-18 16:19GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install to client dev
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

import time, sys, os, shutil, subprocess, distutils.dir_util, socket
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
printLog(log, "--- Install to client dev")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

mkPath(log, ClientDevLiveDirectory)
if not os.path.isfile(ClientDevLiveDirectory + "/client.cfg"):
	printLog(log, ">>> Generate live dev client.cfg <<<")
	cfg = open(ClientDevLiveDirectory + "/client.cfg", "w")
	cfg.write("RootConfigFilename   = \"client_default.cfg\";\n")
	cfg.write("PreDataPath          = {\n")
	cfg.write("\t\"user\", \"patch\", \"" + DataCommonDirectory + "\", \"" + GamedevDirectory + "\", \"" + LeveldesignDirectory + "/translation/translated\", \"" + InstallDirectory + "\", \"data\", \"examples\" \n")
	cfg.write("};\n")
	cfg.write("PreLoadPath          = \"" + InstallDirectory + "\";\n")
	cfg.write("PatchWanted          = 0;\n")
	cfg.write("DisplayLuaDebugInfo  = 1;\n")
	cfg.write("AllowDebugLua        = 1;\n")
	cfg.write("FullScreen           = 0;\n")
	cfg.flush()
	cfg.close()
	printLog(log, "")

mkPath(log, ClientDevDirectory)
if not os.path.isfile(ClientDevDirectory + "/client.cfg"):
	printLog(log, ">>> Generate local dev client.cfg <<<")
	cfg = open(ClientDevDirectory + "/client.cfg", "w")
	cfgr = open(ClientDevLiveDirectory + "/client.cfg", "r")
	for l in cfgr:
		cfg.write(l)
	cfgr.close()
	cfg.write("StartupHost          = \"http://" + socket.gethostname() + ":9042\";\n")
	cfg.write("Application          = {\n")
	cfg.write("	\"dev\", \"./client_ryzom_r.exe\", \"./\" \n")
	cfg.write("};\n")
	cfg.flush()
	cfg.close()
	printLog(log, "")

printLog(log, ">>> Install data <<<")
for category in InstallClientData:
	if (category["UnpackTo"] != None):
		printLog(log, "CATEGORY " + category["Name"])
		targetPath = ClientDevDirectory
		targetPathLive = ClientDevLiveDirectory
		if (category["UnpackTo"] != ""):
			targetPath += "/" + category["UnpackTo"]
			targetPathLive += "/" + category["UnpackTo"]
		mkPath(log, targetPath)
		mkPath(log, targetPathLive)
		for package in category["Packages"]:
			printLog(log, "PACKAGE " + package[0])
			mkPath(log, InstallDirectory + "/" + package[0])
			if "exedll" in package[0]:
				if package[0] == "exedll": # or package[0] == platformExeDll # TODO: 64-bit and Linux separation of exedll, only include one
					copyFileIfNeeded(log, InstallDirectory + "/" + package[0] + "/client_default.cfg", targetPath)
					copyFileIfNeeded(log, InstallDirectory + "/" + package[0] + "/client_default.cfg", targetPathLive)
			else:
				copyFilesNoTreeIfNeeded(log, InstallDirectory + "/" + package[0], targetPath)
				copyFilesNoTreeIfNeeded(log, InstallDirectory + "/" + package[0], targetPathLive)
printLog(log, "")

log.close()
if os.path.isfile("b1_client_dev.log"):
	os.remove("b1_client_dev.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_client_dev.log")
shutil.move("log.log", "b1_client_dev.log")
