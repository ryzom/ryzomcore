#!/usr/bin/python
# 
# \file a1_worldedit_data.py
# \brief Install worldedit data
# \date 2014-09-10 14:01GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install worldedit data
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2014  by authors
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
sys.path.append("configuration")

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite import *
from tools import *

sys.path.append(WorkspaceDirectory)
from projects import *

# Log error
printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Install worldedit data")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

for ecosystem in WorldEditEcosystems:
	ecosystemName = ecosystem[0]
	srcZoneLigos = ExportBuildDirectory + "/ecosystems/" + ecosystemName + "/ligo_es/zoneligos/"
	dstZoneLigos = WorldEditInstallDirectory + "/" + ecosystemName + "/zoneligos/"
	mkPath(log, srcZoneLigos)
	mkPath(log, dstZoneLigos)
	copyFilesNoTreeIfNeeded(log, srcZoneLigos, dstZoneLigos)
	srcZoneBitmaps = DatabaseDirectory + "/landscape/ligo/" + ecosystemName + "/zonebitmaps/"
	dstZoneBitmaps = WorldEditInstallDirectory + "/" + ecosystemName + "/zonebitmaps/"
	mkPath(log, srcZoneBitmaps)
	mkPath(log, dstZoneBitmaps)
	copyFilesExtNoTreeIfNeeded(log, srcZoneBitmaps, dstZoneBitmaps, ".tga")
	copyFilesExtNoTreeIfNeeded(log, srcZoneBitmaps, dstZoneBitmaps, ".png")
	dstCollisionMap = WorldEditInstallDirectory + "/" + ecosystemName + "/collisionmap/"
	mkPath(log, dstCollisionMap)
	for continentName in ecosystem[1]:
		srcCollisionMap = ExportBuildDirectory + "/continents/" + continentName + "/ai_wmap/"
		mkPath(log, srcCollisionMap)
		copyFilesExtNoTreeIfNeeded(log, srcCollisionMap, dstCollisionMap, ".tga")
		copyFilesExtNoTreeIfNeeded(log, srcCollisionMap, dstCollisionMap, ".png")
printLog(log, "")

log.close()
if os.path.isfile("a1_worldedit_data.log"):
	os.remove("a1_worldedit_data.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_worldedit_data.log")
shutil.move("log.log", "a1_worldedit_data.log")
