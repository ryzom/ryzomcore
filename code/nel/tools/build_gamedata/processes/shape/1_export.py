#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export shape
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export shape
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

if os.path.isfile("temp_log.log"):
	os.remove("temp_log.log")
log = open("temp_log.log", "w")
from scripts import *
from buildsite import *
from process import *
from tools import *
from directories import *

# Todo: make a tool to verify the files :)

def getTagFileName(filePath):
	return os.path.split(filePath)[1] + ".tag"

def hackBigTree():
	return 0
	if EcosystemName == "jungle":
		# FO_S2_big_tree is corrupt on first export...
		outDirTag = ExportBuildDirectory + "/" + ShapeTagExportDirectory
		outDirWithCoarse = ExportBuildDirectory + "/" + ShapeWithCoarseMeshExportDirectory
		shapeName = "FO_S2_big_tree.shape"
		tagName = "FO_S2_big_tree.max.tag"
		hackName = "FO_S2_big_tree_hack.tag"
		if os.path.exists(outDirWithCoarse + "/" + shapeName) and os.path.exists(outDirTag + "/" + tagName) and not os.path.exists(outDirTag + "/" + hackName):
			printLog(log, "Removing bad export of FO_S2_big_tree")
			printLog(log, "RM " + outDirWithCoarse + "/" + shapeName)
			os.remove(outDirWithCoarse + "/" + shapeName)
			printLog(log, "RM " + outDirTag + "/" + tagName)
			os.remove(outDirTag + "/" + tagName)
			printLog(log, "TAG " + outDirTag + "/" + hackName)
			hackTagFile = open(outDirTag + "/" + hackName, "w")
			hackTagFile.write("FO_S2_big_tree")
			hackTagFile.close()
			return 1
		elif os.path.exists(outDirTag + "/" + hackName) and not os.path.exists(outDirWithCoarse + "/" + shapeName) and not os.path.exists(outDirTag + "/" + tagName):
			printLog(log, "Missing export of FO_S2_big_tree")
			return 0
		else:
			return 0


printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Export shape")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Override config values for draft mode
if BuildQuality == 0:
	ShapeExportOptExportLighting = "false"
	ShapeExportOptShadow = "false"
	ShapeExportOptLightingLimit = 0
	ShapeExportOptLumelSize = "0.25"
	ShapeExportOptOversampling = 1

if MaxAvailable:
	# Find tools
	Max = findMax(log, MaxDirectory, MaxExecutable)
	ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
	printLog(log, "")
	
	# Export shape 3dsmax
	printLog(log, ">>> Export shape 3dsmax <<<")
	
	# Build paths
	scriptSrc = "maxscript/shape_export.ms"
	# scriptDst = MaxDirectory + "/scripts/shape_export.ms"
	scriptDst = MaxUserDirectory + "/scripts/shape_export.ms"
	logFile = ScriptDirectory + "/processes/shape/log.log"
	outDirTag = ExportBuildDirectory + "/" + ShapeTagExportDirectory
	mkPath(log, outDirTag)
	outDirWithoutCoarse = ExportBuildDirectory + "/" + ShapeExportDirectory
	mkPath(log, outDirWithoutCoarse)
	outDirWithCoarse = ExportBuildDirectory + "/" + ShapeWithCoarseMeshExportDirectory
	mkPath(log, outDirWithCoarse)
	outDirLightmap = ExportBuildDirectory + "/" + ShapeLightmapNotOptimizedExportDirectory
	mkPath(log, outDirLightmap)
	outDirAnim = ExportBuildDirectory + "/" + ShapeAnimExportDirectory
	mkPath(log, outDirAnim)
	
	tagList = findFiles(log, outDirTag, "", ".tag")
	tagLen = len(tagList)
	
	# For each directoy
	if os.path.isfile(scriptDst):
		os.remove(scriptDst)
	for dir in ShapeSourceDirectories:
		tagDiff = 1
		secondTry = 1
		shapeSourceDir = DatabaseDirectory + "/" + dir
		mkPath(log, shapeSourceDir)
		maxFiles = findFilesNoSubdir(log, shapeSourceDir, ".max")
		for maxFile in maxFiles:
			maxFilePath = shapeSourceDir + "/" + maxFile
			tagFilePath = outDirTag + "/" + getTagFileName(maxFilePath)
			if (needUpdate(log, maxFilePath, tagFilePath)):
				sSrc = open(scriptSrc, "r")
				sDst = open(scriptDst, "w")
				for line in sSrc:
					newline = line.replace("output_logfile", logFile)
					# newline = newline.replace("shape_source_directory", shapeSourceDir)
					newline = newline.replace("shape_max_file_path", maxFilePath)
					newline = newline.replace("output_directory_tag", outDirTag)
					newline = newline.replace("output_directory_without_coarse_mesh", outDirWithoutCoarse)
					newline = newline.replace("output_directory_with_coarse_mesh", outDirWithCoarse)
					newline = newline.replace("shape_export_opt_export_lighting", ShapeExportOptExportLighting)
					newline = newline.replace("shape_export_opt_shadow", ShapeExportOptShadow)
					newline = newline.replace("shape_export_opt_lighting_limit", str(ShapeExportOptLightingLimit))
					newline = newline.replace("shape_export_opt_lumel_size", ShapeExportOptLumelSize)
					newline = newline.replace("shape_export_opt_oversampling", str(ShapeExportOptOversampling))
					newline = newline.replace("shape_export_opt_lightmap_log", ShapeExportOptLightmapLog)
					newline = newline.replace("shape_lightmap_path", outDirLightmap)
					newline = newline.replace("output_directory_anim", outDirAnim)
					sDst.write(newline)
				sSrc.close()
				sDst.close()
				retriesLeft = 5
				while retriesLeft > 0:
					printLog(log, "MAXSCRIPT " + scriptDst + "; " + maxFilePath)
					subprocess.call([ ExecTimeout, str(MaxShapeExportTimeout), Max, "-U", "MAXScript", "shape_export.ms", "-q", "-mi", "-vn" ])
					lSrc = open(logFile, "r")
					for line in lSrc:
						if (len(line) > 0):
							printLog(log, line.strip())
					lSrc.close()
					os.remove(logFile)
					if (os.path.exists(tagFilePath)):
						printLog(log, "OK " + maxFilePath)
						retriesLeft = 0
					else:
						printLog(log, "FAIL " + maxFilePath)
					retriesLeft = retriesLeft - 1
				os.remove(scriptDst)
			else:
				printLog(log, "SKIP " + maxFilePath)
#while tagDiff > 0:
#	printLog(log, "MAXSCRIPT " + scriptDst)
#	subprocess.call([ Max, "-U", "MAXScript", "shape_export.ms", "-q", "-mi", "-vn" ])
#	tagList = findFiles(log, outDirTag, "", ".tag")
#	newTagLen = len(tagList)
#	tagDiff = newTagLen - tagLen
#	tagLen = newTagLen
#	printLog(log, "Exported " + str(tagDiff) + " .max files!")
#	if not tagDiff > 0:
#		tagDiff += hackBigTree() # force rerun also when big tree deleted
#		if not tagDiff > 0:
#			tagDiff += secondTry
#			secondTry = 0
	
	# Export clod 3dsmax
	printLog(log, ">>> Export character lod shape files (.clod) from Max <<<")
	printLog(log, "********************************")
	printLog(log, "********      TODO      ********")
	printLog(log, "********************************")
	
	# cat ../clodbank/maxscript/clod_export.ms 
	#| sed -e "s&shape_source_directory&$database_directory/$i&g" 
	#| sed -e "s&output_directory_clod&$build_gamedata_directory/processes/shape/clod&g" 
	#| sed -e "s&output_directory_tag&$build_gamedata_directory/processes/shape/tag&g" 
	# > $max_directory/scripts/clod_export.ms


printLog(log, "")

log.close()
shutil.move("temp_log.log", "log.log")

# end of file
