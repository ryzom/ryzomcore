#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export smallbank
# \date 2009-03-10-20-54-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export smallbank
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
printLog(log, "--- Export smallbank")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
BuildSmallbank = findTool(log, ToolDirectories, BuildSmallbankTool, ToolSuffix)
printLog(log, "")

# For each bank export smallbank
printLog(log, ">>> Export smallbank <<<")
if ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
elif BuildSmallbank == "":
	toolLogFail(log, BuildSmallbankTool, ToolSuffix)
else:
	mkPath(log, DatabaseDirectory + "/" + BankSourceDirectory)
	mkPath(log, ExportBuildDirectory + "/" + SmallbankExportDirectory)
	files = findFiles(log, DatabaseDirectory + "/" + BankSourceDirectory, "", ".bank")
	for file in files:
		sourceFile = DatabaseDirectory + "/" + BankSourceDirectory + "/" + file
		if os.path.isfile(sourceFile):
			destFile =  ExportBuildDirectory + "/" + SmallbankExportDirectory + "/" + file[0:-len(".bank")] + ".smallbank"
			if (needUpdateLogRemoveDest(log, sourceFile, destFile)):
				subprocess.call([ ExecTimeout, str(SmallbankBuildTimeout), BuildSmallbank, sourceFile, destFile, DatabaseDirectory + "/" + TileRootSourceDirectory + "/" ])
printLog(log, "")

log.close()


# end of file
