#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build ig_light
# \date 2009-03-11-15-16-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build ig_light
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
printLog(log, "--- Build ig_light")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
IgLighter = findTool(log, ToolDirectories, IgLighterTool, ToolSuffix)
printLog(log, "")

# For each ig_light directory
printLog(log, ">>> Build ig_light <<<")
if IgLighter == "":
	toolLogFail(log, IgLighterTool, ToolSuffix)
else:
	srcDir = ExportBuildDirectory + "/" + IgOtherBuildDirectory
	mkPath(log, srcDir)
	destDir = ExportBuildDirectory + "/" + IgOtherLightedBuildDirectory
	mkPath(log, destDir)
	subprocess.call([ IgLighter, srcDir, destDir, ActiveProjectDirectory + "/generated/properties.cfg" ])
printLog(log, "")

log.close()


# end of file
