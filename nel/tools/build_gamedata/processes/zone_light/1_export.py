#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export zone_light
# \date 2009-03-11-13-45-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export zone_light
# 
# NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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
printLog(log, "--- Export zone_light")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
#TgaToDds = findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)
printLog(log, "")

# Export zone_light water maps
printLog(log, ">>> Export zone_light water maps <<<")
srcDir = ExportBuildDirectory + "/" + ZoneLightWaterShapesLightedExportDirectory
mkPath(log, srcDir)
for dir in WaterMapSourceDirectories:
	destDir = DatabaseDirectory + "/" + dir
	mkPath(log, destDir)
	copyFilesExtNoTreeIfNeeded(log, srcDir, destDir, ".tga")
	copyFilesExtNoTreeIfNeeded(log, srcDir, destDir, ".png")
printLog(log, "")

log.close()


# end of file
