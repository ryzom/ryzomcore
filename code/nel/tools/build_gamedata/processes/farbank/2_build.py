#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build farbank
# \date 2009-03-10-21-12-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build farbank
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
printLog(log, "--- Build farbank")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
BuildFarbank = findTool(log, ToolDirectories, BuildFarbankTool, ToolSuffix)
printLog(log, "")

# For each bank export farbank
printLog(log, ">>> Build farbank <<<")
if ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
elif BuildFarbank == "":
	toolLogFail(log, BuildFarbankTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + SmallbankExportDirectory)
	mkPath(log, ExportBuildDirectory + "/" + FarbankBuildDirectory)
	files = findFiles(log, ExportBuildDirectory + "/" + SmallbankExportDirectory, "", ".smallbank")
	for file in files:
		sourceFile = ExportBuildDirectory + "/" + SmallbankExportDirectory + "/" + file
		if os.path.isfile(sourceFile):
			for postfix in MultipleTilesPostfix:
				destFile =  ExportBuildDirectory + "/" + FarbankBuildDirectory + "/" + file[0:-len(".smallbank")] + postfix + ".farbank"
				if (needUpdateLogRemoveDest(log, sourceFile, destFile)):
					mkPath(log, DatabaseDirectory + "/" + TileRootSourceDirectory + postfix)
					subprocess.call([ ExecTimeout, str(FarbankBuildTimeout), BuildFarbank, sourceFile, destFile, "-d" + DatabaseDirectory + "/" + TileRootSourceDirectory + postfix + "/", "-p" + postfix ])
printLog(log, "")

log.close()


# end of file
