#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build cegui
# \date 2009-03-14-17-46-GMT
# \author Jan Boon (Kaetemi)
# Build cegui
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
printLog(log, "--- Build cegui")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
BuildImageset = findTool(log, ToolDirectories, BuildImagesetTool, ToolSuffix)
printLog(log, "")

# For each cegui imageset directory
printLog(log, ">>> Build cegui imagesets <<<")
if BuildImageset == "":
	toolLogFail(log, BuildImagesetTool, ToolSuffix)
else:
	srcDir = ExportBuildDirectory + "/" + CeguiImagesetExportDirectory
	mkPath(log, srcDir)
	destDir = ExportBuildDirectory + "/" + CeguiImagesetBuildDirectory
	mkPath(log, destDir)
	for dir in os.listdir(srcDir):
		if (os.path.isdir(srcDir + "/" + dir)) and dir != ".svn" and dir != "*.*":
			mkPath(log, srcDir + "/" + dir)
			subprocess.call([ BuildImageset, destDir + "/" + dir + ".tga", srcDir + "/" + dir ])
printLog(log, "")

log.close()


# end of file
