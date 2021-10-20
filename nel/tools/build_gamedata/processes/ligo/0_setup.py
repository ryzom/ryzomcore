#!/usr/bin/python
# 
# \file 0_setup.py
# \brief setup ligo
# \date 2010-05-24 08:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Setup ligo
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
printLog(log, "--- Setup ligo")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Setup source directories
printLog(log, ">>> Setup source directories <<<")
mkPath(log, DatabaseDirectory + "/" + LigoBaseSourceDirectory)
mkPath(log, DatabaseDirectory + "/" + LigoMaxSourceDirectory)
mkPath(log, DatabaseDirectory + "/" + ZoneSourceDirectory[0])

# Setup export directories
printLog(log, ">>> Setup export directories <<<")
mkPath(log, ExportBuildDirectory + "/" + SmallbankExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemIgExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemZoneExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemZoneLigoExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemCmbExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemTagExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + ZoneExportDirectory)

# Setup build directories
printLog(log, ">>> Setup build directories <<<")
if LigoExportLand != "":
	mkPath(log, ExportBuildDirectory + "/" + LigoZoneBuildDirectory)
	mkPath(log, ExportBuildDirectory + "/" + LigoIgLandBuildDirectory)
	mkPath(log, ExportBuildDirectory + "/" + LigoIgOtherBuildDirectory)
	mkPath(log, ExportBuildDirectory + "/" + RBankCmbExportDirectory)

# Setup client directories
printLog(log, ">>> Setup client directories <<<")

# Setup land exporter cfg
if LigoExportLand != "":
	printLog(log, ">>> Setup land exporter cfg <<<")
	mkPath(log, ActiveProjectDirectory + "/generated")
	cf = open(ActiveProjectDirectory + "/generated/land_exporter.cfg", "w")
	cf.write("\n")
	cf.write("// Ligo settings\n")
	cf.write("\n")
	cf.write("OutZoneDir = \"" + ExportBuildDirectory + "/" + LigoZoneBuildDirectory + "\";\n")
	cf.write("OutIGDir  = \"" + ExportBuildDirectory + "/" + LigoIgLandBuildDirectory + "\";\n")
	cf.write("AdditionnalIGOutDir = \"" + ExportBuildDirectory + "/" + LigoIgOtherBuildDirectory + "\";\n")
	cf.write("\n")
	cf.write("RefZoneDir = \"" + ExportBuildDirectory + "/" + LigoEcosystemZoneExportDirectory+ "\";\n") # FIXME
	cf.write("RefIGDir  = \"" + ExportBuildDirectory + "/" + LigoEcosystemIgExportDirectory + "\";\n")
	cf.write("AdditionnalIGInDir = \"" + ExportBuildDirectory + "/" + LigoEcosystemIgExportDirectory + "\";\n") # FIXME
	cf.write("ContinentsDir = \"" + LeveldesignWorldDirectory + "\";\n")
	cf.write("LigoBankDir = \"" + ExportBuildDirectory + "/" + LigoEcosystemZoneLigoExportDirectory + "\";\n") # FIXME
	cf.write("\n")
	cf.write("TileBankFile = \"" + DatabaseDirectory + "/" + LigoTileBankFile + "\";\n")
	cf.write("\n")
	cf.write("ColorMapFile = \"" + DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportColormap + "\";\n")
	cf.write("HeightMapFile1 = \"" + DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportHeightmap1 + "\";\n")
	cf.write("ZFactor1 = " + LigoExportZFactor1 + ";\n")
	cf.write("HeightMapFile2 = \"" + DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportHeightmap2 + "\";\n")
	cf.write("ZFactor2 = " + LigoExportZFactor2 + ";\n")
	cf.write("ExtendCoords = " + str(LigoExportExtendCoords) + ";\n")
	cf.write("\n")
	cf.write("ZoneLight = 0;\n")
	cf.write("CellSize = 160;\n")
	cf.write("Threshold = 1;\n")
	cf.write("\n")
	cf.write("DFNDir = \"" + LeveldesignDfnDirectory + "\";\n")
	cf.write("RefCMBDir = \"" + ExportBuildDirectory + "/" + LigoEcosystemCmbExportDirectory + "\";\n") # FIXME
	cf.write("OutCMBDir = \"" + ExportBuildDirectory + "/" + RBankCmbExportDirectory + "\";\n")
	cf.write("\n")
	cf.write("ContinentFile = \"" + LeveldesignWorldDirectory + "/" + ContinentFile + "\";\n")
	cf.write("\n")
	cf.write("ExportCollisions = 1;\n")
	cf.write("ExportAdditionnalIGs = 1;\n")
	cf.write("\n")
	cf.write("ZoneRegionFile = \"" + DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportLand + "\";\n")
	cf.write("\n")
	cf.close()

log.close()


# end of file
