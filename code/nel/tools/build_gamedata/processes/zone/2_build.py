#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build zone
# \date 2009-03-10-22-23-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build zone
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
sys.path.append("../../configuration")
sys.path.append("../../configuration/project")
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
printLog(log, "--- Build zone")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
ZoneDependencies = findTool(log, ToolDirectories, ZoneDependenciesTool, ToolSuffix)
ZoneWelder = findTool(log, ToolDirectories, ZoneWelderTool, ToolSuffix)
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
printLog(log, "")

# We are in BEST mode
# TODO if (high quality) blahblahblah
printLog(log, ">>> Build zone dependencies <<<")
if ZoneDependencies == "":
	toolLogFail(log, ZoneDependenciesTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	printLog(log, "********************************")
	printLog(log, "********      TODO      ********")
	printLog(log, "********************************")
printLog(log, "")

# For each zone directory
printLog(log, ">>> Build zone weld <<<")
if ZoneWelder == "":
	toolLogFail(log, ZoneWelderTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + ZoneExportDirectory)
	mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
	files = findFiles(log, ExportBuildDirectory + "/" + ZoneExportDirectory, "", ".zone")
	for file in files:
		sourceFile = ExportBuildDirectory + "/" + ZoneExportDirectory + "/" + file
		destFile = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + os.path.basename(file)[0:-len(".zone")] + ".zonew"
		if needUpdateLogRemoveDest(log, sourceFile, destFile):
			subprocess.call([ ExecTimeout, str(ZoneBuildWeldTimeout), ZoneWelder, sourceFile, destFile ])
printLog(log, "")

# For each zone directory
printLog(log, ">>> Build zone weld no heightmap <<<")
if ZoneWelder == "":
	toolLogFail(log, ZoneWelderTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + ZoneExportDirectory)
	mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
	files = findFiles(log, ExportBuildDirectory + "/" + ZoneExportDirectory, "", ".zonenh")
	for file in files:
		sourceFile = ExportBuildDirectory + "/" + ZoneExportDirectory + "/" + file
		destFile = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + os.path.basename(file)[0:-len(".zonenh")] + ".zonenhw"
		if needUpdateLogRemoveDest(log, sourceFile, destFile):
			subprocess.call([ ExecTimeout, str(ZoneBuildWeldTimeout), ZoneWelder, sourceFile, destFile ])
printLog(log, "")

log.close()


# end of file
