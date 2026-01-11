#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build zone_light
# \date 2009-03-11-13-45-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build zone_light
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
printLog(log, "--- Build zone_light")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
ZoneLighter = findTool(log, ToolDirectories, ZoneLighterTool, ToolSuffix)
ZoneIgLighter = findTool(log, ToolDirectories, ZoneIgLighterTool, ToolSuffix)
printLog(log, "")

# For each zone_light directory
printLog(log, ">>> Build zone_light <<<")
if ZoneLighter == "":
	toolLogFail(log, ZoneLighterTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogfail(log, ExecTimeoutTool, ToolSuffix)
else:
	srcDir = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory
	mkPath(log, srcDir)
	destDir = ExportBuildDirectory + "/" + ZoneLightBuildDirectory
	mkPath(log, destDir)
	dependDir = ExportBuildDirectory + "/" + ZoneDependBuildDirectory
	mkPath(log, dependDir)
	files = findFiles(log, srcDir, "", ".zonew")
	for file in files:
		srcFile = srcDir + "/" + file
		destFile = destDir + "/" + file[0:-len(".zonew")] + ".zonel"
		if (needUpdateLogRemoveDest(log, srcFile, destFile)):
			dependFile = dependDir + "/" + file[0:-len(".zonew")] + ".depend"
			callParallelProcess([ ExecTimeout, str(ZoneLightBuildTimeout), ZoneLighter, srcFile, destFile, ActiveProjectDirectory + "/generated/properties.cfg", dependFile ])
	flushParallelProcesses()
printLog(log, "")

# For each zone_light ig
printLog(log, ">>> Build zone_light ig <<<")
if ZoneIgLighter == "":
	toolLogFail(log,  ZoneIgLighterTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogfail(log, ExecTimeoutTool, ToolSuffix)
else:
	srcDir = ExportBuildDirectory + "/" + ZoneLightBuildDirectory
	mkPath(log, srcDir)
	igsrcDir = ExportBuildDirectory + "/" + IgLandBuildDirectory
	mkPath(log, igsrcDir)
	destDir = ExportBuildDirectory + "/" + ZoneLightIgLandBuildDirectory
	mkPath(log, destDir)
	dependDir = ExportBuildDirectory + "/" + ZoneDependBuildDirectory
	mkPath(log, dependDir)
	files = findFiles(log, srcDir, "", ".zonel")
	for file in files:
		igsrcFile = igsrcDir + "/" + os.path.basename(file)[0:-len(".zonel")] + ".ig"
		destFile = destDir + "/" + os.path.basename(file)[0:-len(".zonel")] + ".ig"
		if (os.path.isfile(igsrcFile)):
			if (needUpdateLogRemoveDest(log, igsrcFile, destFile)):
				srcFile = srcDir + "/" + file
				dependFile = dependDir + "/" + file[0:-len(".zonel")] + ".depend"
				callParallelProcess([ ExecTimeout, str(ZoneIgLightBuildTimeout), ZoneIgLighter, srcFile, destFile, ActiveProjectDirectory + "/generated/properties.cfg", dependFile ])
	flushParallelProcesses()
printLog(log, "")

log.close()


# end of file
