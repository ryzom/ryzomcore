#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export dummy
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export dummy
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
printLog(log, "--- Export dummy")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

if MaxAvailable:
	# Find tools
	Max = findMax(log, MaxDirectory, MaxExecutable)
	# ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
	printLog(log, "")
	
	# Export dummy 3dsmax
	printLog(log, ">>> Export dummy 3dsmax <<<")
	
	# Build paths
	#scriptSrc = "maxscript/dummy_export.ms"
	# scriptDst = MaxDirectory + "/scripts/dummy_export.ms"
	#scriptDst = MaxUserDirectory + "/scripts/dummy_export.ms"
	#logFile = ScriptDirectory + "/processes/dummy/log.log"
	#outDirTag = ExportBuildDirectory + "/" + DummyTagExportDirectory
	#mkPath(log, outDirTag)
	#outDirWithoutCoarse = ExportBuildDirectory + "/" + DummyExportDirectory
	#mkPath(log, outDirWithoutCoarse)
	#outDirWithCoarse = ExportBuildDirectory + "/" + DummyWithCoarseMeshExportDirectory
	#mkPath(log, outDirWithCoarse)
	#outDirLightmap = ExportBuildDirectory + "/" + DummyLightmapNotOptimizedExportDirectory
	#mkPath(log, outDirLightmap)
	#outDirAnim = ExportBuildDirectory + "/" + DummyAnimExportDirectory
	#mkPath(log, outDirAnim)
	
	#tagList = findFiles(log, outDirTag, "", ".tag")
	#tagLen = len(tagList)
	
	# For each directoy
	#if os.path.isfile(scriptDst):
	#	os.remove(scriptDst)
	#for dir in DummySourceDirectories:
	#	tagDiff = 1
	#	dummySourceDir = DatabaseDirectory + "/" + dir
	#	mkPath(log, dummySourceDir)
	#	sSrc = open(scriptSrc, "r")
	#	sDst = open(scriptDst, "w")
	#	for line in sSrc:
	#		newline = line.replace("output_logfile", logFile)
	#		newline = newline.replace("dummy_source_directory", dummySourceDir)
	#		newline = newline.replace("output_directory_tag", outDirTag)
	#		newline = newline.replace("output_directory_without_coarse_mesh", outDirWithoutCoarse)
	#		newline = newline.replace("output_directory_with_coarse_mesh", outDirWithCoarse)
	#		newline = newline.replace("dummy_export_opt_export_lighting", DummyExportOptExportLighting)
	#		newline = newline.replace("dummy_export_opt_shadow", DummyExportOptShadow)
	#		newline = newline.replace("dummy_export_opt_lighting_limit", str(DummyExportOptLightingLimit))
	#		newline = newline.replace("dummy_export_opt_lumel_size", DummyExportOptLumelSize)
	#		newline = newline.replace("dummy_export_opt_oversampling", str(DummyExportOptOversampling))
	#		newline = newline.replace("dummy_export_opt_lightmap_log", DummyExportOptLightmapLog)
	#		newline = newline.replace("dummy_lightmap_path", outDirLightmap)
	#		newline = newline.replace("output_directory_anim", outDirAnim)
	#		sDst.write(newline)
	#	sSrc.close()
	#	sDst.close()
	#	while tagDiff > 0:
	#		printLog(log, "MAXSCRIPT " + scriptDst)
	#		subprocess.call([ Max, "-U", "MAXScript", "dummy_export.ms", "-q", "-mi", "-mip" ])
	#		tagList = findFiles(log, outDirTag, "", ".tag")
	#		newTagLen = len(tagList)
	#		tagDiff = newTagLen - tagLen
	#		tagLen = newTagLen
	#		printLog(log, "Exported " + str(tagDiff) + " .max files!")
	#		tagDiff += hackBdummyTree() # force rerun also when bdummy tree deleted
	#	os.remove(scriptDst)


printLog(log, "")

log.close()


# end of file
