#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export zone_light
# \date 2009-03-11-13-45-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export zone_light
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
printLog(log, "--- Export zone_light")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
#TgaToDds = findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)
printLog(log, "")

# For each zone_light directory
printLog(log, ">>> Export zone_light water maps <<<")
srcDir = ExportBuildDirectory + "/" + ZoneLightWaterShapesLightedExportDirectory
mkPath(log, srcDir)
for dir in WaterMapSourceDirectories:
	destDir = DatabaseDirectory + "/" + dir
	mkPath(log, destDir)
	copyFilesExtNoTreeIfNeeded(log, srcDir, destDir, ".tga")
#mkPath(log, ExportBuildDirectory + "/" + zone_lightExportDirectory)
#for dir in zone_lightSourceDirectories:
#	mkPath(log, DatabaseDirectory + "/" + dir)
#	niouname = dir.replace("/", "_")
#	newpath = ExportBuildDirectory + "/" + zone_lightExportDirectory + "/" + niouname
#	mkPath(log, newpath)
#	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, newpath, ".tga")
printLog(log, "")

# For each zone_light directory to compress in one DXTC
#printLog(log, ">>> Export zone_light dxtc <<<")
#mkPath(log, ExportBuildDirectory + "/" + zone_lightDxtcExportDirectory)
#for dir in zone_lightDxtcSourceDirectories:
#	mkPath(log, DatabaseDirectory + "/" + dir)
#	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, ExportBuildDirectory + "/" + zone_lightDxtcExportDirectory, ".tga")
#printLog(log, "")

# For each zone_light fullscreen directory compress independently all in dds
#printLog(log, ">>> Export zone_light fullscreen <<<")
#if TgaToDds == "":
#	toolLogFail(log, TgaToDdsTool, ToolSuffix)
#else:
#	mkPath(log, ExportBuildDirectory + "/" + zone_lightFullscreenExportDirectory)
#	for dir in zone_lightFullscreenSourceDirectories:
#		mkPath(log, DatabaseDirectory + "/" + dir)
#		files = findFiles(log, DatabaseDirectory + "/" + dir, "", ".tga")
#		for file in files:
#			sourceFile = DatabaseDirectory + "/" + dir + "/" + file
#			destFile = ExportBuildDirectory + "/" + zone_lightFullscreenExportDirectory + "/" + os.path.basename(file)[0:-len(".tga")] + ".dds"
#			if needUpdateLogRemoveDest(log, sourceFile, destFile):
#				subprocess.call([ TgaToDds, sourceFile, "-o", destFile, "-a", "5" ])
#printLog(log, "")

# For each zone_light 3d directory
#printLog(log, ">>> Export zone_light 3d <<<")
#mkPath(log, ExportBuildDirectory + "/" + zone_light3DExportDirectory)
#for dir in zone_light3DSourceDirectories:
#	mkPath(log, DatabaseDirectory + "/" + dir)
#	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, ExportBuildDirectory + "/" + zone_light3DExportDirectory, ".tga")
#printLog(log, "")

log.close()


# end of file
