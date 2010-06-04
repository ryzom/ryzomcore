#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export rbank
# \date 2009-03-10-22-43-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export rbank
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

# ################### result = subprocess.Popen([ dfdsklfjslk ], stdout = PIPE).communicate()[0]  ######################################

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Export rbank")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
TgaToDds = findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)
printLog(log, "")

# For each rbank directory
printLog(log, ">>> Export rbank test 1 <<<")
#mkPath(log, ExportBuildDirectory + "/" + rbankExportDirectory)
#for dir in rbankSourceDirectories:
#	mkPath(log, DatabaseDirectory + "/" + dir)
#	niouname = dir.replace("/", "_")
#	newpath = ExportBuildDirectory + "/" + rbankExportDirectory + "/" + niouname
#	mkPath(log, newpath)
#	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, newpath, ".tga")
printLog(log, "")

# For each rbank directory to compress in one DXTC
printLog(log, ">>> Export rbank test 2 <<<")
#mkPath(log, ExportBuildDirectory + "/" + rbankDxtcExportDirectory)
#for dir in rbankDxtcSourceDirectories:
#	mkPath(log, DatabaseDirectory + "/" + dir)
#	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, ExportBuildDirectory + "/" + rbankDxtcExportDirectory, ".tga")
printLog(log, "")

# For each rbank fullscreen directory compress independently all in dds
printLog(log, ">>> Export rbank test 3 <<<")
#if TgaToDds == "":
#	toolLogFail(log, TgaToDdsTool, ToolSuffix)
#else:
#	mkPath(log, ExportBuildDirectory + "/" + rbankFullscreenExportDirectory)
#	for dir in rbankFullscreenSourceDirectories:
#		mkPath(log, DatabaseDirectory + "/" + dir)
#		files = findFiles(log, DatabaseDirectory + "/" + dir, "", ".tga")
#		for file in files:
#			sourceFile = DatabaseDirectory + "/" + dir + "/" + file
#			destFile = ExportBuildDirectory + "/" + rbankFullscreenExportDirectory + "/" + os.path.basename(file)[0:-len(".tga")] + ".dds"
#			if needUpdateLogRemoveDest(log, sourceFile, destFile):
#				subprocess.call([ TgaToDds, sourceFile, "-o", destFile, "-a", "5" ])
printLog(log, "")

# For each rbank 3d directory
printLog(log, ">>> Export rbank test 4 <<<")
#mkPath(log, ExportBuildDirectory + "/" + rbank3DExportDirectory)
#for dir in rbank3DSourceDirectories:
#	mkPath(log, DatabaseDirectory + "/" + dir)
#	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, ExportBuildDirectory + "/" + rbank3DExportDirectory, ".tga")
printLog(log, "")

log.close()


# end of file
