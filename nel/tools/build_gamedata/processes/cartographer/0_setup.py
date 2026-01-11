#!/usr/bin/python
# 
# \file 0_setup.py
# \brief setup cartographer
# \date 2014-09-13 13:32GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Setup cartographer
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
printLog(log, "--- Setup cartographer")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Setup build directories
printLog(log, ">>> Setup build directories <<<")
mkPath(log, ExportBuildDirectory + "/" + CartographerBuildDirectory)

# Setup lookup directories
printLog(log, ">>> Setup lookup directories <<<")
mkPath(log, ExportBuildDirectory + "/" + AiWmapBuildDirectory) # IN
mkPath(log, ExportBuildDirectory + "/" + ZoneLightBuildDirectory) # IN (.zonel)
mkPath(log, ExportBuildDirectory + "/" + ZoneLightIgLandBuildDirectory) # IN (.ig)
mkPath(log, ExportBuildDirectory + "/" + SmallbankExportDirectory) # IN
mkPath(log, ExportBuildDirectory + "/" + FarbankBuildDirectory) # IN
mkPath(log, ExportBuildDirectory + "/" + DisplaceExportDirectory) # IN
mkPath(log, ExportBuildDirectory + "/" + TilesExportDirectory) # IN
mkPath(log, LeveldesignDataCommonDirectory) # IN
mkPath(log, LeveldesignDfnDirectory) # IN
mkPath(log, LeveldesignDirectory) # IN
for dir in PropertiesExportBuildSearchPaths:
	mkPath(log, ExportBuildDirectory + "/" + dir)

# Setup client directories
printLog(log, ">>> Setup install directories <<<")
mkPath(log, InstallDirectory + "/" + CartographerInstallDirectory)

# Setup client directories
printLog(log, ">>> Setup configuration <<<")
mkPath(log, ActiveProjectDirectory + "/generated")
cfg = open(ActiveProjectDirectory + "/generated/island_screenshots.cfg", "w")
cfg.write("\n")
cfg.write("// BUILD CARTOGRAPHER CONFIGURATION\n")
cfg.write("\n")
cfg.write("SearchPaths = {\n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + AiWmapBuildDirectory + "\", \n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + ZoneLightBuildDirectory + "\", \n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + ZoneLightIgLandBuildDirectory + "\", \n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + SmallbankExportDirectory + "\", \n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + FarbankBuildDirectory + "\", \n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + DisplaceExportDirectory + "\", \n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + TilesExportDirectory + "\", \n")
cfg.write("\t\"" + LeveldesignDataCommonDirectory + "\", \n")
cfg.write("\t\"" + LeveldesignDfnDirectory + "\", \n")
cfg.write("\t\"" + LeveldesignDirectory + "\", \n")
for dir in PropertiesExportBuildSearchPaths:
	cfg.write("\t\"" + ExportBuildDirectory + "/" + dir + "\", \n")
cfg.write("};\n")
cfg.write("\n")
cfg.write("OutDir = \"" + ExportBuildDirectory + "/" + CartographerBuildDirectory + "\";\n")
cfg.write("\n")
cfg.write("Continents = {\n")
cfg.write("\t\"" + CartographerContinent + "\", \n")
cfg.write("};\n")
cfg.write("\n")
cfg.write("SeasonSuffixes = {\n")
if CartographerSeasonSuffixes:
	for suffix in CartographerSeasonSuffixes:
		cfg.write("\t\"" + suffix + "\", \n")
else:
	for suffix in MultipleTilesPostfix:
		cfg.write("\t\"" + suffix + "\", \n")
cfg.write("};\n")
cfg.write("\n")
cfg.write("InverseZTest = true;\n")
cfg.write("Vegetation = true;\n")
cfg.write("MeterPixelSize = 2;\n")
cfg.write("\n")
cfg.write("CompleteIslandsFile = \"r2_islands.xml\";\n")
cfg.write("EntryPointsFile = \"r2_entry_points.txt\";\n")
cfg.write("\n")
cfg.close()

log.close()


# end of file
