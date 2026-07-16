#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build sheets
# \date 2009-06-03 10:47GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build sheets
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
printLog(log, "--- Build sheets")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
SheetsPacker = findTool(log, ToolDirectories, SheetsPackerTool, ToolSuffix)
printLog(log, "")

# For each sheets directory
printLog(log, ">>> Build sheets <<<")
if SheetsPacker == "":
	toolLogFail(log, SheetsPackerTool, ToolSuffix)
else:
	mkPath(log, LeveldesignDirectory)
	mkPath(log, LeveldesignDfnDirectory)
	mkPath(log, DataCommonDirectory)
	mkPath(log, GamedevDirectory)
	mkPath(log, PrimitivesDirectory)
	mkPath(log, ExportBuildDirectory + "/" + SheetsBuildDirectory)
	cf = open("sheets_packer.cfg", "w")
	cf.write("\n")
	cf.write("// SHEETS PACKER CONFIG FILE\n")
	cf.write("\n")
	cf.write("DataPath = \n")
	cf.write("{\n")
	cf.write("\t\"" + LeveldesignDirectory + "\", \n")
	cf.write("\t\"" + LeveldesignDfnDirectory + "\", \n")
	cf.write("\t\"" + DataCommonDirectory + "\", \n")
	cf.write("\t\"" + GamedevDirectory + "\", \n")
	cf.write("\t\"" + PrimitivesDirectory + "\", \n")
	cf.write("};\n")
	cf.write("WorldSheet = \"" + WorldSheet + "\";\n")
	cf.write("PrimitivesPath = \"" + PrimitivesDirectory + "\";\n")
	cf.write("OutputDataPath = \"" + ExportBuildDirectory + "/" + SheetsBuildDirectory + "\";\n")
	cf.write("LigoPrimitiveClass = \"" + LigoPrimitiveClass + "\";\n")
	cf.write("\n")
	cf.close()
	subprocess.call([ SheetsPacker ])
	mkPath(log, ExportBuildDirectory + "/" + VisualSlotTabBuildDirectory)
	copyFileIfNeeded(log, "visual_slot.tab", ExportBuildDirectory + "/" + VisualSlotTabBuildDirectory + "/visual_slot.tab")
	os.remove("visual_slot.tab")
printLog(log, "")

log.close()


# end of file
