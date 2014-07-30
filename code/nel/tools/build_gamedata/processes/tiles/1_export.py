#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export tiles
# \date 2009-03-10-21-31-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export tiles
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
printLog(log, "--- Export tiles")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
TgaToDds = findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
printLog(log, "")

# For each tiles directory
printLog(log, ">>> Export tiles as DDS <<<")
if TgaToDds == "":
	toolLogFail(log, TgaToDdsTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + TilesExportDirectory)
	for dir in TilesSourceDirectories:
		mkPath(log, DatabaseDirectory + "/" + dir)
		files = findFiles(log, DatabaseDirectory + "/" + dir, "", ".tga")
		for file in files:
			sourceFile = DatabaseDirectory + "/" + dir + "/" + file
			destFile = ExportBuildDirectory + "/" + TilesExportDirectory + "/" + os.path.basename(file)[0:-len(".tga")] + ".dds"
			if needUpdateLogRemoveDest(log, sourceFile, destFile):
				subprocess.call([ ExecTimeout, str(MapsBuildTimeout), TgaToDds, sourceFile, "-o", destFile, "-a", "5", "-m" ])
		files = findFiles(log, DatabaseDirectory + "/" + dir, "", ".png")
		for file in files:
			sourceFile = DatabaseDirectory + "/" + dir + "/" + file
			destFile = ExportBuildDirectory + "/" + TilesExportDirectory + "/" + os.path.basename(file)[0:-len(".png")] + ".dds"
			if needUpdateLogRemoveDest(log, sourceFile, destFile):
				subprocess.call([ ExecTimeout, str(MapsBuildTimeout), TgaToDds, sourceFile, "-o", destFile, "-a", "5", "-m" ])

#printLog(log, ">>> Copy PNG tiles <<<")
#mkPath(log, ExportBuildDirectory + "/" + TilesExportDirectory)
#for dir in TilesSourceDirectories:
#	mkPath(log, DatabaseDirectory + "/" + dir)
#	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, ExportBuildDirectory + "/" + TilesExportDirectory, ".png")
#printLog(log, "")

log.close()


# end of file
