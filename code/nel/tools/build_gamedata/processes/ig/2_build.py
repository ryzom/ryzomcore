#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build ig
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build ig
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
printLog(log, "--- Build ig")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
PrimExport = findTool(log, ToolDirectories, PrimExportTool , ToolSuffix)
IgElevation = findTool(log, ToolDirectories, IgElevationTool, ToolSuffix)
IgAdd = findTool(log, ToolDirectories, IgAddTool, ToolSuffix)

configDir = ActiveProjectDirectory + "/generated"
mkPath(log, configDir)

def igElevation(inputIgDir, outputIgDir):
	printLog(log, ">>> IG Elevation <<<")
	needUpdateIg = needUpdateDirByTagLog(log, inputIgDir, ".ig", outputIgDir, ".ig")
	if needUpdateIg:
		printLog(log, "DETECT UPDATE IG->Elevated")
	else:
		printLog(log, "DETECT SKIP IG->Elevated")
	needUpdateHeightMap = needUpdateFileDirNoSubdir(log, DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportHeightmap1, outputIgDir) or needUpdateFileDirNoSubdir(log, DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportHeightmap2, outputIgDir)
	if needUpdateHeightMap:
		printLog(log, "DETECT UPDATE HeightMap->Elevated")
	else:
		printLog(log, "DETECT SKIP HeightMap->Elevated")
	needUpdateLand = needUpdateFileDirNoSubdir(log, DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportLand, outputIgDir)
	if needUpdateLand:
		printLog(log, "DETECT UPDATE Land->Elevated")
	else:
		printLog(log, "DETECT SKIP Land->Elevated")
	if needUpdateIg or needUpdateHeightMap or needUpdateLand:
		printLog(log, "DETECT DECIDE UPDATE")
		mkPath(log, inputIgDir)
		mkPath(log, outputIgDir)
		mkPath(log, DatabaseDirectory + "/" + LigoBaseSourceDirectory)
		
		configFile = configDir + "/ig_elevation.cfg"
		if os.path.isfile(configFile):
			os.remove(configFile)
		
		printLog(log, "CONFIG " + configFile)
		cf = open(configFile, "w")
		cf.write("// ig_elevation.cfg\n")
		cf.write("\n")
		cf.write("InputIGDir = \"" + inputIgDir + "\";\n")
		cf.write("OutputIGDir = \"" + outputIgDir + "\";\n")
		cf.write("\n")
		cf.write("CellSize = 160.0;")
		cf.write("\n")
		cf.write("HeightMapFile1 = \"" + DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportHeightmap1 + "\";\n")
		cf.write("ZFactor1 = " + LigoExportZFactor1 + ";\n")
		cf.write("HeightMapFile2 = \"" + DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportHeightmap2 + "\";\n")
		cf.write("ZFactor2 = " + LigoExportZFactor2 + ";\n")
		cf.write("\n")
		cf.write("LandFile = \"" + DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportLand + "\";\n")
		cf.write("\n")
		cf.close()
		subprocess.call([ IgElevation, configFile ])
		os.remove(configFile)
		
		# Copy remaining IG files
		copyFilesLogless(log, inputIgDir, outputIgDir)
	else:
		printLog(log, "DETECT DECIDE SKIP")
		printLog(log, "SKIP *")

# Build process
if (ContinentLeveldesignWorldDirectory != "") or (len(IgOtherSourceDirectories) > 0):
	printLog(log, ">>> Prim IG: ON <<<")
	configFile = configDir + "/prim_export.cfg"
	if os.path.isfile(configFile):
		os.remove(configFile)
	
	outIgDir = ExportBuildDirectory + "/" + IgElevLandPrimBuildDirectory
	mkPath(log, outIgDir)
	zoneWDir = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory
	mkPath(log, zoneWDir)
	smallBank = DatabaseDirectory + "/" + TileRootSourceDirectory + "/" + BankTileBankName + ".bank"
	farBank = ExportBuildDirectory + "/" + FarbankBuildDirectory + "/" + BankTileBankName + MultipleTilesPostfix[0] + ".farbank"
	displaceDir = DatabaseDirectory + "/" + DisplaceSourceDirectory
	continentDir = LeveldesignWorldDirectory + "/" + ContinentLeveldesignWorldDirectory
	mkPath(log, continentDir)
	formDir = LeveldesignDirectory
	mkPath(log, LeveldesignDirectory)
	worldEditorFiles = WorldEditorFilesDirectory
	mkPath(log, WorldEditorFilesDirectory)
	
	printLog(log, "CONFIG " + configFile)
	cf = open(configFile, "w")
	cf.write("// prim_export.cfg\n")
	cf.write("\n")
	cf.write("OutIGDir = \"" + outIgDir + "\";\n")
	cf.write("ZoneWDir = \"" + zoneWDir + "\";\n")
	cf.write("\n")
	cf.write("SmallBank = \"" + smallBank + "\";\n")
	cf.write("FarBank = \"" + farBank + "\";\n")
	cf.write("DisplaceDir = \"" + displaceDir + "\";\n")
	cf.write("\n")
	cf.write("CellSize = 160.0;")
	cf.write("\n")
	cf.write("PrimDirs = {\n")
	cf.write("\t\"" + continentDir + "\", \n")
	for dir in IgPrimitiveSourceDirectories:
		mkPath(log, DatabaseDirectory + "/" + dir)
		cf.write("\t\"" + DatabaseDirectory + "/" + dir + "\", \n")
	cf.write("};\n")
	cf.write("\n")
	cf.write("FormDir = \"" + formDir + "\";\n")
	cf.write("WorldEditorFiles = \"" + worldEditorFiles + "\";\n")
	cf.write("\n")
	cf.close()
	subprocess.call([ PrimExport, configFile ])
	os.remove(configFile)
	
	igElevation(ExportBuildDirectory + "/" + LigoIgLandBuildDirectory, ExportBuildDirectory + "/" + IgElevLandLigoBuildDirectory)
	
	igElevation(ExportBuildDirectory + "/" + IgStaticLandExportDirectory, ExportBuildDirectory + "/" + IgElevLandStaticBuildDirectory)

printLog(log, ">>> Merge land IGs <<<")
mkPath(log, ExportBuildDirectory + "/" + IgTempLandMergeBuildDirectory)
removeFilesRecursive(log, ExportBuildDirectory + "/" + IgTempLandMergeBuildDirectory)

mkPath(log, ExportBuildDirectory + "/" + IgLandBuildDirectory)

mkPath(log, ExportBuildDirectory + "/" + IgElevLandPrimBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + IgElevLandLigoBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + IgElevLandStaticBuildDirectory)
igFilesPrim = findFiles(log, ExportBuildDirectory + "/" + IgElevLandPrimBuildDirectory, "", ".ig")
igFilesLigo = findFiles(log, ExportBuildDirectory + "/" + IgElevLandLigoBuildDirectory, "", ".ig")
igFilesStatic = findFiles(log, ExportBuildDirectory + "/" + IgElevLandStaticBuildDirectory, "", ".ig")
igFilesAll = [ ]
for igFile in igFilesPrim:
	if not igFile in igFilesAll:
		igFilesAll += [ igFile ]
for igFile in igFilesLigo:
	if not igFile in igFilesAll:
		igFilesAll += [ igFile ]
for igFile in igFilesStatic:
	if not igFile in igFilesAll:
		igFilesAll += [ igFile ]
igFilesAll.sort()
for igFile in igFilesAll:
	primIgFile = ExportBuildDirectory + "/" + IgElevLandPrimBuildDirectory + "/" + igFile
	ligoIgFile = ExportBuildDirectory + "/" + IgElevLandLigoBuildDirectory + "/" + igFile
	staticIgFile = ExportBuildDirectory + "/" + IgElevLandStaticBuildDirectory + "/" + igFile
	tempIgFile = ExportBuildDirectory + "/" + IgTempLandMergeBuildDirectory + "/" + igFile
	outIgFile = ExportBuildDirectory + "/" + IgLandBuildDirectory + "/" + igFile
	activeFile = ""
	needsUpdate = 0
	sourceTools = ""
	if igFile in igFilesPrim:
		if needUpdate(log, primIgFile, outIgFile):
			needsUpdate = 1
	if not needsUpdate == 1 and igFile in igFilesLigo:
		if needUpdate(log, ligoIgFile, outIgFile):
			needsUpdate = 1
	if not needsUpdate == 1 and igFile in igFilesStatic:
		if needUpdate(log, staticIgFile, outIgFile):
			needsUpdate = 1
	if needsUpdate == 1:
		if os.path.isfile(outIgFile):
			os.remove(outIgFile)
		if igFile in igFilesPrim:
			sourceTools += " [Prim]"
			activeFile = primIgFile
		if igFile in igFilesLigo:
			if activeFile == "":
				activeFile = ligoIgFile
			else:
				sourceTools += " [Ligo]"
				subprocess.call([ IgAdd, tempIgFile, ligoIgFile, activeFile ])
				activeFile = tempIgFile
		if igFile in igFilesStatic:
			if activeFile == "":
				activeFile = staticIgFile
			else:
				sourceTools += " [Static]"
				subprocess.call([ IgAdd, outIgFile, staticIgFile, activeFile ])
				activeFile = outIgFile
		else:
			shutil.copy(activeFile, outIgFile)
		printLog(log, "MERGE " + igFile + sourceTools)
	else:
		printLog(log, "SKIP " + igFile)
		
# Remove temporary land IGs
printLog(log, ">>> Remove temporary land IGs <<<")
mkPath(log, ExportBuildDirectory + "/" + IgTempLandMergeBuildDirectory)
removeFilesRecursive(log, ExportBuildDirectory + "/" + IgTempLandMergeBuildDirectory)

# Remove outdated land IGs
printLog(log, ">>> Remove outdated land IGs <<<")
igFilesOut = findFiles(log, ExportBuildDirectory + "/" + IgLandBuildDirectory, "", ".ig")
for igFile in igFilesOut:
	if not igFile in igFilesAll:
		printLog(log, "RM " + ExportBuildDirectory + "/" + IgLandBuildDirectory + "/" + igFile)
		os.remove(ExportBuildDirectory + "/" + IgLandBuildDirectory + "/" + igFile)
		
# Verify land IGs
printLog(log, ">>> Verify land IGs <<<")
for igFile in igFilesAll:
	if not igFile in igFilesOut:
		printLog(log, "MISSING " + igFile)

# Write land IGs TXT
printLog(log, ">>> Write land IGs TXT <<<")
igTxtFile = ExportBuildDirectory + "/" + IgLandBuildDirectory + "/" + LandscapeName + "_ig.txt"
if needUpdateDirNoSubdirFile(log, ExportBuildDirectory + "/" + IgLandBuildDirectory, igTxtFile):
	printLog(log, "WRITE " + ExportBuildDirectory + "/" + IgLandBuildDirectory + "/" + LandscapeName + "_ig.txt")
	if os.path.isfile(igTxtFile):
		os.remove(igTxtFile)
	igTxt = open(igTxtFile, "w")
	for igFile in igFilesAll:
		igTxt.write(igFile + "\n")
	igTxt.close()
else:
	printLog(log, "SKIP *")

# Merge other IGs
printLog(log, ">>> Merge other IGs <<<") # (not true merge, since not necesserary)
mkPath(log, ExportBuildDirectory + "/" + IgStaticOtherExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + LigoIgOtherBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + IgOtherBuildDirectory)
copyFilesExtNoTreeIfNeeded(log, ExportBuildDirectory + "/" + IgStaticOtherExportDirectory, ExportBuildDirectory + "/" + IgOtherBuildDirectory, ".ig")
copyFilesExtNoTreeIfNeeded(log, ExportBuildDirectory + "/" + LigoIgOtherBuildDirectory, ExportBuildDirectory + "/" + IgOtherBuildDirectory, ".ig")

log.close()


# end of file
