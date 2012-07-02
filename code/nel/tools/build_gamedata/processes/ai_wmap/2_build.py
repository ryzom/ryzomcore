#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build ai_wmap
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build ai_wmap
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
printLog(log, "--- Build ai_wmap")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
AiBuildWmap = findTool(log, ToolDirectories, AiBuildWmapTool, ToolSuffix)
TgaCut = findTool(log, ToolDirectories, TgaCutTool, ToolSuffix)

if AiBuildWmap == "":
	toolLogFail(log, AiBuildWmapTool, ToolSuffix)
if TgaCut == "":
	toolLogFail(log, TgaCutTool, ToolSuffix)
else:
	printLog(log, ">>> Copy ai_build_wmap.cfg <<<")
	cfgPath = ActiveProjectDirectory + "/generated/ai_build_wmap.cfg"
	tagPath = ExportBuildDirectory + "/" + AiWmapBuildTagDirectory + "/ai_wmap_build.tag"
	shutil.copy(cfgPath, "ai_build_wmap.cfg")
	printLog(log, ">>> Check up packed sheets <<<")
	subprocess.call([ AiBuildWmap, "checkPackedSheets" ])
	printLog(log, ">>> Build ai_wmap <<<")
	mkPath(log, ExportBuildDirectory + "/" + RbankOutputBuildDirectory)
	mkPath(log, ExportBuildDirectory + "/" + AiWmapBuildDirectory)
	mkPath(log, ExportBuildDirectory + "/" + AiWmapBuildTagDirectory)
	if (needUpdate(log, "continents.packed_sheets", tagPath) or needUpdateMultiDirNoSubdirFile(log, ExportBuildDirectory, [ RbankOutputBuildDirectory ] + IgLookupDirectories + PacsPrimLookupDirectories, tagPath)):
		printLog(log, ">>> Generate wmap <<<")
		subprocess.call([ AiBuildWmap, "pacsCrunch " + AiWmapContinentName ])
		printLog(log, ">>> Generate sized wmap <<<")
		subprocess.call([ AiBuildWmap, "pacsBuildGabarit " + AiWmapContinentName ])
		printLog(log, ">>> Generate cwmaps for each size <<<")
		subprocess.call([ AiBuildWmap, "pacsBuildWmap " + AiWmapContinentName + "_0" ])
		subprocess.call([ AiBuildWmap, "pacsBuildWmap " + AiWmapContinentName + "_1" ])
		subprocess.call([ AiBuildWmap, "pacsBuildWmap " + AiWmapContinentName + "_2" ])
		printLog(log, ">>> Generate bitmap for each size <<<")
		subprocess.call([ AiBuildWmap, "pacsBuildBitmap " + AiWmapContinentName + "_0" ])
		subprocess.call([ AiBuildWmap, "pacsBuildBitmap " + AiWmapContinentName + "_1" ])
		subprocess.call([ AiBuildWmap, "pacsBuildBitmap " + AiWmapContinentName + "_2" ])
		printLog(log, ">>> Clear height maps for size 1 and 2 <<<")
		subprocess.call([ AiBuildWmap, "pacsClearHeightmap " + AiWmapContinentName ])
		printLog(log, ">>> Cut tga for world editor <<<")
		subprocess.call([ TgaCut, ExportBuildDirectory + "/" + AiWmapBuildDirectory + "/" + AiWmapContinentName + "_0.tga" ])
		moveFilesExtNoTree(log, ".", ExportBuildDirectory + "/" + AiWmapBuildDirectory, ".tga")
		printLog(log, ">>> Remove wmap <<<")
		removeFilesRecursiveExt(log, ExportBuildDirectory + "/" + AiWmapBuildDirectory, ".wmap")
		tagFile = open(tagPath, "w")
		tagFile.write(time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())) + "\n")
		tagFile.close()
	else:
		printLog("SKIP *")
printLog(log, "")

log.close()


# end of file
