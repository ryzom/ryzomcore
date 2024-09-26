#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build cartographer
# \date 2014-09-13 13:32GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build cartographer
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2014  Jan BOON
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
printLog(log, "--- Build cartographer")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
R2IslandsTextures = findTool(log, ToolDirectories, R2IslandsTexturesTool, ToolSuffix)
TgaToDds = findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)

if R2IslandsTextures == "":
	toolLogFail(log, R2IslandsTexturesTool, ToolSuffix)
else:
	printLog(log, ">>> Copy island_screenshots.cfg <<<")
	cfgPath = ActiveProjectDirectory + "/generated/island_screenshots.cfg"
	shutil.copy(cfgPath, "island_screenshots.cfg")
	printLog(log, ">>> Build cartographer <<<")
	mkPath(log, ExportBuildDirectory + "/" + CartographerBuildDirectory)
	subprocess.call([ R2IslandsTextures ])
printLog(log, "")

printLog(log, ">>> Compress cartographer maps to DDS <<<")
if TgaToDds == "":
	toolLogFail(log, TgaToDdsTool, ToolSuffix)
else:
	destPath = ExportBuildDirectory + "/" + CartographerMapBuildDirectory
	mkPath(log, destPath)
	sourcePath = ExportBuildDirectory + "/" + CartographerBuildDirectory
	mkPath(log, sourcePath)
	files = os.listdir(sourcePath)
	len_tga_png = len(".tga")
	len_dds = len(".dds")
	for fileName in files:
		if isLegalFileName(fileName):
			sourceFile = sourcePath + "/" + fileName
			if os.path.isfile(sourceFile):
				if (fileName[-len_tga_png:].lower() == ".tga") or (fileName[-len_tga_png:].lower() == ".png"):
					destFile = destPath + "/" + os.path.basename(fileName)[0:-len_tga_png] + ".dds"
					if needUpdateLogRemoveDest(log, sourceFile, destFile):
						subprocess.call([ TgaToDds, sourceFile, "-o", destFile, "-m" ])
			elif not os.path.isdir(sourceFile):
				printLog(log, "FAIL ?! file not dir or file ?! " + sourceFile)
printLog(log, "")

log.close()


# end of file
