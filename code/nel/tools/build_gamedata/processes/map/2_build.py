#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build map
# \date 2009-03-10 13:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build map
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
printLog(log, "--- Build map")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
TgaToDds = findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
PanoplyMaker = findTool(log, ToolDirectories, PanoplyMakerTool, ToolSuffix)
HlsBankMaker = findTool(log, ToolDirectories, HlsBankMakerTool, ToolSuffix)
printLog(log, "")

# For each map directory
printLog(log, ">>> Build map compressed: compress tga and png to dds <<<")
if TgaToDds == "":
	toolLogFail(log, TgaToDdsTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + MapBuildDirectory)
	for dir in MapSourceDirectories:
		sourcePath = DatabaseDirectory + "/" + dir
		mkPath(log, sourcePath)
		destPath = ExportBuildDirectory + "/" + MapBuildDirectory
		mkPath(log, destPath)
		files = findFiles(log, sourcePath, "", ".tga")
		for file in files:
			sourceFile = sourcePath + "/" + file
			destFile = destPath + "/" + os.path.basename(file)[0:-len(".tga")] + ".dds"
			if needUpdateLogRemoveDest(log, sourceFile, destFile):
				subprocess.call([ ExecTimeout, str(MapsBuildTimeout), TgaToDds, sourceFile, "-o", destFile, "-m", "-r" + str(ReduceBitmapFactor) ])
		files = findFiles(log, sourcePath, "", ".png")
		for file in files:
			sourceFile = sourcePath + "/" + file
			destFile = destPath + "/" + os.path.basename(file)[0:-len(".png")] + ".dds"
			if needUpdateLogRemoveDest(log, sourceFile, destFile):
				subprocess.call([ ExecTimeout, str(MapsBuildTimeout), TgaToDds, sourceFile, "-o", destFile, "-m", "-r" + str(ReduceBitmapFactor) ])
printLog(log, "")

printLog(log, ">>> Build map uncompressed: copy tga, png, dds <<<")
for dir in MapSourceDirectories:
	sourcePath = DatabaseDirectory + "/" + dir
	mkPath(log, sourcePath)
	destPath = ExportBuildDirectory + "/" + MapBuildDirectory
	mkPath(log, destPath)
	copyFilesExtNoTreeIfNeeded(log, sourcePath, destPath, ".dds")
	copyFilesExtNoTreeIfNeeded(log, sourcePath, destPath, ".png")
	copyFilesExtNoTreeIfNeeded(log, sourcePath, destPath, ".tga")

printLog(log, ">>> Build panoply <<<")
printLog(log, "********************************")
printLog(log, "********      TODO      ********")
printLog(log, "********************************")

printLog(log, ">>> Build panoply dds <<<")
printLog(log, "********************************")
printLog(log, "********      TODO      ********")
printLog(log, "********************************")

printLog(log, ">>> Build hls map <<<")
printLog(log, "********************************")
printLog(log, "********      TODO      ********")
printLog(log, "********************************")

log.close()


# end of file
