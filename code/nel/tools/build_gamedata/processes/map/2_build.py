#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build map
# \date 2009-03-10 13:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build map
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
printLog(log, "--- Build map")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
TgaToDds = findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
PanoplyMaker = findTool(log, ToolDirectories, PanoplyMakerTool, ToolSuffix)
HlsBankMaker = findTool(log, ToolDirectories, HlsBankMakerTool, ToolSuffix)
printLog(log, "")

buildPanoplyTagPath = ExportBuildDirectory + "/" + MapTagBuildDirectory + "/build_panoply.tag"
buildCompressTagPath = ExportBuildDirectory + "/" + MapTagBuildDirectory + "/build_compress.tag"

if MapPanoplyFileList != None:
	printLog(log, ">>> Panoply build <<<")
	mkPath(log, ExportBuildDirectory + "/" + MapTagBuildDirectory)
	directoriesCheck = [ ]
	for panoplyCfg in MapPanoplySourceDirectories:
		directoriesCheck += [ panoplyCfg[2] ]
		directoriesCheck += [ panoplyCfg[3] ]
	if (needUpdateMultiDirNoSubdirFile(log, DatabaseDirectory, directoriesCheck, buildPanoplyTagPath)):
		mkPath(log, ActiveProjectDirectory + "/generated")
		mkPath(log, ExportBuildDirectory + "/" + MapPanoplyBuildDirectory)
		mkPath(log, ExportBuildDirectory + "/" + MapPanoplyHlsInfoBuildDirectory)
		mkPath(log, ExportBuildDirectory + "/" + MapPanoplyCacheBuildDirectory)
		printLog(log, "")
		printLog(log, ">>> Move panoply and hls to cache <<<")
		removeFilesDirsRecursive(log, ExportBuildDirectory + "/" + MapPanoplyCacheBuildDirectory)
		moveDir(log, ExportBuildDirectory + "/" + MapPanoplyBuildDirectory, ExportBuildDirectory + "/" + MapPanoplyCacheBuildDirectory)
		mkPath(log, ExportBuildDirectory + "/" + MapPanoplyBuildDirectory)
		moveFilesNoSubdir(log, ExportBuildDirectory + "/" + MapPanoplyHlsInfoBuildDirectory, ExportBuildDirectory + "/" + MapPanoplyCacheBuildDirectory)
		printLog(log, "")
		for panoplyCfg in MapPanoplySourceDirectories:
			printLog(log, ">>> Panoply " + panoplyCfg[1] + " <<<")
			mkPath(log, DatabaseDirectory + "/" + panoplyCfg[2])
			mkPath(log, DatabaseDirectory + "/" + panoplyCfg[3])
			cfg = open(ActiveProjectDirectory + "/generated/current_panoply.cfg", "w")
			cfgCommon = open(ActiveProjectDirectory + "/" + panoplyCfg[0], "r")
			cfgRace = open(ActiveProjectDirectory + "/" + panoplyCfg[1], "r")
			cfg.write("\n")
			cfg.write("// CURRENT PANOPLY CONFIGURATION\n")
			cfg.write("\n")
			cfg.write("input_path = \"" + DatabaseDirectory + "/" + panoplyCfg[2] + "\";\n")
			cfg.write("additionnal_paths = \"" + DatabaseDirectory + "/" + panoplyCfg[3] + "\";\n")
			cfg.write("output_path = \"" + ExportBuildDirectory + "/" + MapPanoplyBuildDirectory + "\";\n")
			cfg.write("hls_info_path = \"" + ExportBuildDirectory + "/" + MapPanoplyHlsInfoBuildDirectory + "\";\n")
			cfg.write("cache_path = \"" + ExportBuildDirectory + "/" + MapPanoplyCacheBuildDirectory + "\";\n")
			cfg.write("\n")
			cfg.write("/////////////////////////////////////////////\n")
			cfg.write("\n")
			for line in cfgCommon:
				cfg.write(line)
			for line in cfgRace:
				cfg.write(line)
			cfg.close()
			cfgCommon.close()
			cfgRace.close()
			subprocess.call([ PanoplyMaker, ActiveProjectDirectory + "/generated/current_panoply.cfg" ])
			printLog(log, "")
		tagFile = open(buildPanoplyTagPath, "w")
		tagFile.write(time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())) + "\n")
		tagFile.close()
	else:
		printLog(log, "SKIP *.*")
		printLog(log, "")

printLog(log, ">>> Compress TGA and PNG maps to DDS <<<")
if TgaToDds == "":
	toolLogFail(log, TgaToDdsTool, ToolSuffix)
elif ExecTimeout == "":
	toolLogFail(log, ExecTimeoutTool, ToolSuffix)
else:
	destPath = ExportBuildDirectory + "/" + MapBuildDirectory
	mkPath(log, destPath)
	sourcePaths = [ ExportBuildDirectory + "/" + MapExportDirectory ]
	writeTag = 0
	if MapPanoplyFileList != None:
		if needUpdate(log, buildPanoplyTagPath, buildCompressTagPath):
			sourcePaths += [ ExportBuildDirectory + "/" + MapPanoplyBuildDirectory ]
		else:
			printLog(log, "SKIP " + ExportBuildDirectory + "/" + MapPanoplyBuildDirectory + "/*.*")
	for sourcePath in sourcePaths:
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
							subprocess.call([ ExecTimeout, str(MapsBuildTimeout), TgaToDds, sourceFile, "-o", destFile, "-m", "-r" + str(ReduceBitmapFactor) ])
							writeTag = 1
					elif fileName[-len_dds:].lower() == ".dds":
						copyFileIfNeeded(log, sourceFile, destPath + "/" + os.path.basename(fileName))
						writeTag = 1
				elif not os.path.isdir(sourceFile):
					printLog(log, "FAIL ?! file not dir or file ?! " + sourceFile)
	if writeTag:
		tagFile = open(buildCompressTagPath, "w")
		tagFile.write(time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())) + "\n")
		tagFile.close()
printLog(log, "")

if MapHlsBankFileName != None:
	printLog(log, ">>> Build the HLSBank <<<")
	if HlsBankMaker == "":
		toolLogFail(log, HlsBankMakerTool, ToolSuffix)
	else:
		mkPath(log, ExportBuildDirectory + "/" + MapPanoplyHlsInfoBuildDirectory)
		mkPath(log, ExportBuildDirectory + "/" + MapPanoplyHlsBankBuildDirectory)
		hlsBankPath = ExportBuildDirectory + "/" + MapPanoplyHlsBankBuildDirectory + "/" + MapHlsBankFileName
		if (needUpdate(log, buildPanoplyTagPath, hlsBankPath) or needUpdate(log, buildCompressTagPath, hlsBankPath)):
			if os.path.isfile(hlsBankPath):
				os.remove(hlsBankPath)
			subprocess.call([ HlsBankMaker, ExportBuildDirectory + "/" + MapPanoplyHlsInfoBuildDirectory, hlsBankPath ])
		else:
			printLog(log,"SKIP " + hlsBankPath)
	printLog(log, "")

log.close()


# end of file
