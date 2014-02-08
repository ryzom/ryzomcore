#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build rbank
# \date 2009-03-10-22-43-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build rbank
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
printLog(log, "--- Build rbank")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
BuildIgBoxes = findTool(log, ToolDirectories, BuildIgBoxesTool, ToolSuffix)
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
BuildRbank = findTool(log, ToolDirectories, BuildRbankTool, ToolSuffix)
GetNeighbors = findTool(log, ToolDirectories, GetNeighborsTool, ToolSuffix)
BuildIndoorRbank = findTool(log, ToolDirectories, BuildIndoorRbankTool, ToolSuffix)
# AiBuildWmap = findTool(log, ToolDirectories, AiBuildWmapTool, ToolSuffix)
printLog(log, "")

# Build rbank bbox
printLog(log, ">>> Build rbank bbox <<<")
if BuildIgBoxes == "":
	toolLogFail(log, BuildIgBoxesTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + RbankBboxBuildDirectory)
	cf = open("build_ig_boxes.cfg", "w")
	cf.write("\n")
	cf.write("Pathes = {\n")
	for dir in IgLookupDirectories:
		mkPath(log, ExportBuildDirectory + "/" + dir)
		cf.write("\t\"" + ExportBuildDirectory + "/" + dir + "\", \n")
	for dir in ShapeLookupDirectories:
		mkPath(log, ExportBuildDirectory + "/" + dir)
		cf.write("\t\"" + ExportBuildDirectory + "/" + dir + "\", \n")
	cf.write("};\n")
	cf.write("\n")
	cf.write("IGs = {\n")
	for dir in IgLookupDirectories:
		files = findFiles(log, ExportBuildDirectory + "/" + dir, "", ".ig")
		for file in files:
			cf.write("\t\"" + os.path.basename(file)[0:-len(".ig")] + "\", \n")
	cf.write("};\n")
	cf.write("\n")
	cf.write("Output = \"" + ExportBuildDirectory + "/" + RbankBboxBuildDirectory + "/temp.bbox\";\n")
	cf.write("\n")
	cf.close()
	subprocess.call([ BuildIgBoxes ])
	os.remove("build_ig_boxes.cfg")
printLog(log, "")

printLog(log, ">>> Build rbank build config <<<")
cf = open("build_rbank.cfg", "w")
cf.write("\n")
cf.write("// Rbank settings\n")
cf.write("\n")
cf.write("Verbose = " + str(RBankVerbose) + ";\n")
cf.write("CheckConsistency = " + str(RBankConsistencyCheck) + ";\n")
mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
cf.write("ZonePath = \"" + ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/\";\n")
mkPath(log, ExportBuildDirectory + "/" + SmallbankExportDirectory)
cf.write("BanksPath = \"" + ExportBuildDirectory + "/" + SmallbankExportDirectory + "/\";\n")
cf.write("Bank = \"" + ExportBuildDirectory + "/" + SmallbankExportDirectory + "/" + BankTileBankName + ".smallbank\";\n")
cf.write("ZoneExt = \".zonew\";\n")
cf.write("ZoneNHExt = \".zonenhw\";\n")
cf.write("IGBoxes = \"" + ExportBuildDirectory + "/" + RbankBboxBuildDirectory + "/temp.bbox\";\n")
mkPath(log, LeveldesignWorldDirectory)
cf.write("LevelDesignWorldPath = \"" + LeveldesignWorldDirectory + "\";\n")
mkPath(log, ExportBuildDirectory + "/" + IgLandBuildDirectory)
cf.write("IgLandPath = \"" + ExportBuildDirectory + "/" + IgLandBuildDirectory + "\";\n")
mkPath(log, ExportBuildDirectory + "/" + IgOtherBuildDirectory)
cf.write("IgVillagePath = \"" + ExportBuildDirectory + "/" + IgOtherBuildDirectory + "\";\n")
cf.write("\n")
mkPath(log, ExportBuildDirectory + "/" + RbankTessellationBuildDirectory)
cf.write("TessellationPath = \"" + ExportBuildDirectory + "/" + RbankTessellationBuildDirectory + "/\";\n")
cf.write("TessellateLevel = " + str(BuildQuality) + ";\n") # BuildQuality
cf.write("\n")
cf.write("WaterThreshold = 1.0;\n")
cf.write("\n")
cf.write("OutputRootPath = \"" + ExportBuildDirectory + "/\";\n")
mkPath(log, ExportBuildDirectory + "/" + RbankSmoothBuildDirectory)
cf.write("SmoothDirectory = \"" + RbankSmoothBuildDirectory + "/\";\n")
mkPath(log, ExportBuildDirectory + "/" + RbankRawBuildDirectory)
cf.write("RawDirectory = \"" + RbankRawBuildDirectory + "/\";\n")
cf.write("\n")
cf.write("ReduceSurfaces = " + str(RbankReduceSurfaces) + ";\n")
cf.write("SmoothBorders = " + str(RbankSmoothBorders) + ";\n")
cf.write("\n")
cf.write("ComputeElevation = " + str(RbankComputeElevation) + ";\n")
cf.write("ComputeLevels = " + str(RbankComputeLevels) + ";\n")
cf.write("\n")
cf.write("LinkElements = " + str(RbankLinkElements) + ";\n")
cf.write("\n")
cf.write("CutEdges = " + str(RbankCutEdges) + ";\n")
cf.write("\n")
cf.write("UseZoneSquare = " + str(RbankUseZoneSquare) + ";\n")
cf.write("\n")
cf.write("// The whole landscape\n")
cf.write("ZoneUL = \"" + RbankZoneUl + "\";\n")
cf.write("ZoneDR = \"" + RbankZoneDr + "\";\n")
cf.write("\n")
mkPath(log, ExportBuildDirectory + "/" + RbankPreprocBuildDirectory)
cf.write("PreprocessDirectory = \"" + ExportBuildDirectory + "/" + RbankPreprocBuildDirectory + "/\";\n")
cf.write("\n")
cf.write("// The global retriever processing settings\n")
cf.write("GlobalRetriever = \"temp.gr\";\n")
cf.write("RetrieverBank = \"temp.rbank\";\n")
cf.write("\n")
cf.write("GlobalUL = \"" + RbankZoneUl + "\";\n")
cf.write("GlobalDR = \"" + RbankZoneDr + "\";\n")
cf.write("\n")
cf.write("// Which kind of stuff to do\n")
cf.write("TessellateZones = 0;\n")
cf.write("MoulineZones = 0;\n")
cf.write("ProcessRetrievers = 0;\n")
cf.write("ProcessGlobal = 0;\n")
cf.write("\n")
cf.write("Zones = {\n")
mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
files = findFiles(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory, "", ".zonew")
for file in files:
	cf.write("\t\"" + os.path.basename(file) + "\", \n")
cf.write("};\n")
cf.write("\n")
cf.write("Pathes = {\n")
mkPath(log, WorldEditorFilesDirectory);
cf.write("\t\"" + WorldEditorFilesDirectory + "\", \n");
for dir in IgLookupDirectories:
	mkPath(log, ExportBuildDirectory + "/" + dir)
	cf.write("\t\"" + ExportBuildDirectory + "/" + dir + "\", \n")
for dir in ShapeLookupDirectories:
	mkPath(log, ExportBuildDirectory + "/" + dir)
	cf.write("\t\"" + ExportBuildDirectory + "/" + dir + "\", \n")
cf.write("};\n")
cf.write("\n")
cf.close()
printLog(log, "")

printLog(log, ">>> Build rbank check prims <<<")
if BuildRbank == "":
	toolLogFail(log, BuildRbankTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	subprocess.call([ ExecTimeout, str(RbankBuildTesselTimeout), BuildRbank, "-C", "-p", "-g" ])
printLog(log, "")

printLog(log, ">>> Build rbank process all passes <<<")
if BuildRbank == "":
	toolLogFail(log, BuildRbankTool, ToolSuffix)
if GetNeighbors == "":
	toolLogFail(log, GetNeighborsTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	zonefiles = findFiles(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory, "", ".zonew")
	for zonefile in zonefiles:
		zone = os.path.basename(zonefile)[0:-len(".zonew")]
		lr1 = ExportBuildDirectory + "/" + RbankSmoothBuildDirectory + "/" + zone + ".lr"
		nearzones = subprocess.Popen([ GetNeighbors, zone ], stdout = subprocess.PIPE).communicate()[0].strip().split(" ")
		printLog(log, "ZONE " + zone + ": " + str(nearzones))
		zone_to_build = 0
		for nearzone in nearzones:
			sourcePath = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + nearzone + ".zonew"
			if (os.path.isfile(sourcePath)):
				if (needUpdate(log, sourcePath, lr1)):
					zone_to_build = 1
		sourcePath = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + zone + ".zonew"
		if zone_to_build:
			printLog(log, sourcePath + " -> " + lr1)
			subprocess.call([ ExecTimeout, str(RbankBuildTesselTimeout), BuildRbank, "-c", "-P", "-g", os.path.basename(zonefile) ])
		else:
			printLog(log, "SKIP " + lr1)
printLog(log, "")

printLog(log, ">>> Build rbank process global <<<") # TODO: Check if the LR changed?
if BuildRbank == "":
	toolLogFail(log, BuildRbankTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	subprocess.call([ ExecTimeout, str(RbankBuildProcglobalTimeout), BuildRbank, "-c", "-P", "-G" ])
printLog(log, "")
os.remove("build_rbank.cfg")

printLog(log, ">>> Build rbank indoor <<<")
if BuildIndoorRbank == "":
	toolLogFail(log, BuildIndoorRbankTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	retrieversDir = ExportBuildDirectory + "/" + RbankRetrieversBuildDirectory
	mkPath(log, retrieversDir)
	removeFilesRecursiveExt(log, retrieversDir, ".rbank")
	removeFilesRecursiveExt(log, retrieversDir, ".gr")
	removeFilesRecursiveExt(log, retrieversDir, ".lr")
	cf = open("build_indoor_rbank.cfg", "w")
	cf.write("\n")
	mkPath(log, ExportBuildDirectory + "/" + RBankCmbExportDirectory)
	cf.write("MeshPath = \"" + ExportBuildDirectory + "/" + RBankCmbExportDirectory + "/\";\n")
	# cf.write("Meshes = { };\n")
	cf.write("Meshes = \n")
	cf.write("{\n")
	meshFiles = findFilesNoSubdir(log, ExportBuildDirectory + "/" + RBankCmbExportDirectory, ".cmb")
	lenCmbExt = len(".cmb")
	for file in meshFiles:
		cf.write("\t\"" + file[0:-lenCmbExt] + "\", \n")
	cf.write("};\n")
	cf.write("OutputPath = \"" + retrieversDir + "/\";\n")
	# mkPath(log, ExportBuildDirectory + "/" + RbankOutputBuildDirectory)
	# cf.write("OutputPath = \"" + ExportBuildDirectory + "/" + RbankOutputBuildDirectory + "/\";\n")
	cf.write("OutputPrefix = \"unused\";\n")
	cf.write("Merge = 1;\n")
	mkPath(log, ExportBuildDirectory + "/" + RbankSmoothBuildDirectory)
	cf.write("MergePath = \"" + ExportBuildDirectory + "/" + RbankSmoothBuildDirectory + "/\";\n")
	cf.write("MergeInputPrefix = \"temp\";\n")
	cf.write("MergeOutputPrefix = \"tempMerged\";\n")
	# cf.write("MergeOutputPrefix = \"" + RbankRbankName + "\";\n")
	cf.write("AddToRetriever = 1;\n")
	cf.write("\n")
	cf.close()
	subprocess.call([ ExecTimeout, str(RbankBuildIndoorTimeout), BuildIndoorRbank ])
	os.remove("build_indoor_rbank.cfg")
printLog(log, "")

retrieversDir = ExportBuildDirectory + "/" + RbankRetrieversBuildDirectory
mkPath(log, retrieversDir)
outputDir = ExportBuildDirectory + "/" + RbankOutputBuildDirectory
mkPath(log, outputDir)
printLog(log, ">>> Move gr, rbank and lr <<<")
if needUpdateDirNoSubdir(log, retrieversDir, outputDir):
	removeFilesRecursiveExt(log, outputDir, ".rbank")
	removeFilesRecursiveExt(log, outputDir, ".gr")
	removeFilesRecursiveExt(log, outputDir, ".lr")
	copyFilesRenamePrefixExt(log, retrieversDir, outputDir, "tempMerged", RbankRbankName, ".rbank")
	copyFilesRenamePrefixExt(log, retrieversDir, outputDir, "tempMerged", RbankRbankName, ".gr")
	copyFilesRenamePrefixExt(log, retrieversDir, outputDir, "tempMerged_", RbankRbankName + "_", ".lr")
else:
	printLog(log, "SKIP *")

log.close()


# end of file
