#!/usr/bin/python
# 
# \file 0_setup.py
# \brief setup pz
# \date 2014-09-13 13:32GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Setup pz
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
printLog(log, "--- Setup pz")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Setup build directories
printLog(log, ">>> Setup build directories <<<")
mkPath(log, ExportBuildDirectory + "/" + PackedZoneCacheBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + PackedZoneCWMapCacheBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + PackedZoneBuildDirectory)

# Setup lookup directories
printLog(log, ">>> Setup lookup directories <<<")
mkPath(log, ExportBuildDirectory + "/" + AiWmapBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + ZoneLightBuildDirectory)
mkPath(log, LeveldesignDataCommonDirectory)

# Setup client directories
printLog(log, ">>> Setup install directories <<<")
mkPath(log, InstallDirectory + "/" + PackedZoneInstallDirectory)

# Setup client directories
printLog(log, ">>> Setup configuration <<<")
mkPath(log, ActiveProjectDirectory + "/generated")
cfg = open(ActiveProjectDirectory + "/generated/build_world_packed_col.cfg", "w")
cfg.write("\n")
cfg.write("// BUILD WORLD PACKED COL CONFIGURATION\n")
cfg.write("\n")
cfg.write("SearchPaths = {\n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + AiWmapBuildDirectory + "\", \n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + ZoneLightBuildDirectory + "\", \n")
cfg.write("\t\"" + LeveldesignDataCommonDirectory + "\", \n")
cfg.write("};\n")
cfg.write("\n")
cfg.write("CachePath = \"" + ExportBuildDirectory + "/" + PackedZoneCacheBuildDirectory + "\";\n")
cfg.write("CWMapCachePath = \"" + ExportBuildDirectory + "/" + PackedZoneCWMapCacheBuildDirectory + "\";\n")
cfg.write("OutputPath = \"" + ExportBuildDirectory + "/" + PackedZoneBuildDirectory + "\";\n")
cfg.write("\n")
cfg.write("EntryPointsFile = \"r2_islands.xml\";\n")
cfg.write("\n")
cfg.write("CWMapList = {\n")
cfg.write("\t\"" + PackedZoneCWMap + "\", \n")
cfg.write("};\n")
cfg.write("\n")
cfg.write("Fly = 0;\n")
cfg.write("\n")
cfg.write("HeightMapsAsTga = 1;\n")
cfg.write("PixelPerMeter = 1;\n")
cfg.write("\n")
cfg.write("RefineThreshold = 32;\n")
cfg.write("\n")
cfg.close()

log.close()


# end of file
