#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build zone
# \date 2009-03-10-22-23-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build zone
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
printLog(log, "--- Build zone")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
ZoneDependencies = findTool(log, ToolDirectories, ZoneDependenciesTool, ToolSuffix)
ZoneWelder = findTool(log, ToolDirectories, ZoneWelderTool, ToolSuffix)
ZoneElevation = findTool(log, ToolDirectories, ZoneElevationTool, ToolSuffix)
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
printLog(log, "")

# We are in BEST mode
if BuildQuality == 1:
	printLog(log, ">>> Build zone dependencies <<<")
	if ZoneDependencies == "":
		toolLogFail(log, ZoneDependenciesTool, ToolSuffix)
	elif ExecTimeout == "":
		toolLogFail(log, ExecTimeoutTool, ToolSuffix)
	else:
		mkPath(log, ExportBuildDirectory + "/" + ZoneExportDirectory)
		mkPath(log, ExportBuildDirectory + "/" + ZoneDependBuildDirectory)
		needUpdateZoneDepend = needUpdateDirByLowercaseTagLog(log, ExportBuildDirectory + "/" + ZoneExportDirectory, ".zone", ExportBuildDirectory + "/" + ZoneDependBuildDirectory, ".depend")
		if needUpdateZoneDepend:
			printLog(log, "DETECT UPDATE Zone->Depend")
		else:
			printLog(log, "DETECT SKIP Zone->Depend")
		needUpdateContinentDepend = needUpdateFileDirNoSubdir(log, LeveldesignWorldDirectory + "/" + ContinentFile, ExportBuildDirectory + "/" + ZoneDependBuildDirectory)
		if needUpdateContinentDepend:
			printLog(log, "DETECT UPDATE Continent->Depend")
		else:
			printLog(log, "DETECT SKIP Continent->Depend")
		needUpdateSearchPaths = needUpdateMultiDirNoSubdir(log, ExportBuildDirectory, PropertiesExportBuildSearchPaths, ExportBuildDirectory + "/" + ZoneDependBuildDirectory)
		if needUpdateSearchPaths:
			printLog(log, "DETECT UPDATE SearchPaths->Depend")
		else:
			printLog(log, "DETECT SKIP SearchPaths->Depend")
		if needUpdateZoneDepend or needUpdateContinentDepend or needUpdateSearchPaths:
			printLog(log, "DETECT DECIDE UPDATE")
			mkPath(log, ActiveProjectDirectory + "/generated")
			configFile = ActiveProjectDirectory + "/generated/zone_dependencies.cfg"
			templateCf = open(ActiveProjectDirectory + "/generated/properties.cfg", "r")
			cf = open(configFile, "w")
			for line in templateCf:
				cf.write(line)
			cf.write("\n");
			cf.write("level_design_directory = \"" + LeveldesignDirectory + "\";\n");
			cf.write("level_design_world_directory = \"" + LeveldesignWorldDirectory + "\";\n");
			cf.write("level_design_dfn_directory = \"" + LeveldesignDfnDirectory + "\";\n");
			cf.write("continent_name = \"" + ContinentName + "\";\n");
			cf.write("\n");
			cf.close()
			
			for zoneRegion in ZoneRegions:
				# zone_dependencies [properties.cfg] [firstZone.zone] [lastzone.zone] [output_dependencies.cfg]
				subprocess.call([ ExecTimeout, str(ZoneBuildDependTimeout), ZoneDependencies, configFile, ExportBuildDirectory + "/" + ZoneExportDirectory + "/" + zoneRegion[0] + ".zone", ExportBuildDirectory + "/" + ZoneExportDirectory + "/" + zoneRegion[1] + ".zone", ExportBuildDirectory + "/" + ZoneDependBuildDirectory + "/doomy.depend" ])
		else:
			printLog(log, "DETECT DECIDE SKIP")
			printLog(log, "SKIP *")
	printLog(log, "")

# For each zone directory
#printLog(log, ">>> Build zone weld <<<")
#if ZoneWelder == "":
#	toolLogFail(log, ZoneWelderTool, ToolSuffix)
#elif ExecTimeout == "":
#	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
#else:
#	mkPath(log, ExportBuildDirectory + "/" + ZoneExportDirectory)
#	mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
#	files = findFiles(log, ExportBuildDirectory + "/" + ZoneExportDirectory, "", ".zone")
#	for file in files:
#		sourceFile = ExportBuildDirectory + "/" + ZoneExportDirectory + "/" + file
#		destFile = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + os.path.basename(file)[0:-len(".zone")] + ".zonew"
#		if needUpdateLogRemoveDest(log, sourceFile, destFile):
#			subprocess.call([ ExecTimeout, str(ZoneBuildWeldTimeout), ZoneWelder, sourceFile, destFile ])
#printLog(log, "")

# For each zone directory
printLog(log, ">>> Weld zone without heightmap <<<")
if ZoneWelder == "":
	toolLogFail(log, ZoneWelderTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + ZoneExportDirectory)
	mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
	files = findFiles(log, ExportBuildDirectory + "/" + ZoneExportDirectory, "", ".zonenh")
	for file in files:
		sourceFile = ExportBuildDirectory + "/" + ZoneExportDirectory + "/" + file
		destFile = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + os.path.basename(file)[0:-len(".zonenh")] + ".zonenhw"
		if needUpdateLogRemoveDest(log, sourceFile, destFile):
			subprocess.call([ ExecTimeout, str(ZoneBuildWeldTimeout), ZoneWelder, sourceFile, destFile ])
printLog(log, "")

printLog(log, ">>> Apply zone heightmap to welded zone <<<")
if ZoneElevation == "":
	toolLogFail(log, ZoneElevationTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
	mkPath(log, DatabaseDirectory + "/" + LigoBaseSourceDirectory);
	land = DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportLand
	heightMap1 = DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportHeightmap1
	heightMap2 = DatabaseDirectory + "/" + LigoBaseSourceDirectory + "/" + LigoExportHeightmap2
	files = findFiles(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory, "", ".zonenhw")
	for file in files:
		sourceFile = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + file
		destFile = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + os.path.basename(file)[0:-len(".zonenhw")] + ".zone"
		if needUpdateLogRemoveDest(log, sourceFile, destFile):
			command = [ ZoneElevation, sourceFile, destFile, "--land", land, "--heightmap", heightMap1, "--zfactor", LigoExportZFactor1, "--heightmap2", heightMap2, "--zfactor2", LigoExportZFactor2 ]
			if LigoExportExtendCoords != 0:
				command.append("--extendcoords")
			callParallelProcess(command)
	flushParallelProcesses()
printLog(log, "")

printLog(log, ">>> Re-weld zone with heightmap <<<")
if ZoneWelder == "":
	toolLogFail(log, ZoneWelderTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
	files = findFiles(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory, "", ".zone")
	for file in files:
		sourceFile = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + file
		destFile = ExportBuildDirectory + "/" + ZoneWeldBuildDirectory + "/" + os.path.basename(file)[0:-len(".zone")] + ".zonew"
		if needUpdateLogRemoveDest(log, sourceFile, destFile):
			subprocess.call([ ExecTimeout, str(ZoneBuildWeldTimeout), ZoneWelder, sourceFile, destFile ])
printLog(log, "")

log.close()


# end of file
