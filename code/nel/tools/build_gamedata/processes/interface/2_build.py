#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build interface
# \date 2009-03-10 13:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build interface
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
sys.path.append("../../configuration")

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite import *
from process import *
from tools import *
from directories import *

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Build interface")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
BuildInterface = findTool(log, ToolDirectories, BuildInterfaceTool, ToolSuffix)
printLog(log, "")

# For each interface directory
printLog(log, ">>> Build interface <<<")
if BuildInterface == "":
	toolLogFail(log, BuildInterfaceTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + InterfaceBuildDirectory)
	for dir in os.listdir(ExportBuildDirectory + "/" + InterfaceExportDirectory):
		dirPath = ExportBuildDirectory + "/" + InterfaceExportDirectory + "/" + dir
		if (os.path.isdir(dirPath)) and dir != ".svn" and dir != "*.*":
			texturePath = ExportBuildDirectory + "/" + InterfaceBuildDirectory + "/texture_" + dir + ".tga"
			if needUpdateDirNoSubdirFile(log, dirPath, texturePath):
				subprocess.call([ BuildInterface, texturePath, dirPath ])
			else:
				printLog(log, "SKIP " + texturePath)
printLog(log, "")

# For each interface directory to compress in one DXTC
printLog(log, ">>> Build interface dxtc <<<")
if BuildInterface == "":
	toolLogFail(log, BuildInterfaceTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + InterfaceDxtcBuildDirectory)
	dirPath = ExportBuildDirectory + "/" + InterfaceDxtcExportDirectory
	texturePath = ExportBuildDirectory + "/" + InterfaceDxtcBuildDirectory + "/texture_interfaces_dxtc.tga"
	if needUpdateDirNoSubdirFile(log, dirPath, texturePath):
		subprocess.call([ BuildInterface, texturePath, dirPath ])
	else:
		printLog(log, "SKIP " + texturePath)
printLog(log, "")

log.close()


# end of file
