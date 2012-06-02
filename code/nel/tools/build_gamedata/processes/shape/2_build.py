#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build shape
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build shape
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
printLog(log, "--- Build shape")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
BuildShadowSkin = findTool(log, ToolDirectories, BuildShadowSkinTool, ToolSuffix)
BuildClodtex = findTool(log, ToolDirectories, BuildClodtexTool, ToolSuffix)
LightmapOptimizer = findTool(log, ToolDirectories, LightmapOptimizerTool, ToolSuffix)
TgaToDds = findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)
BuildCoarseMesh = findTool(log, ToolDirectories, BuildCoarseMeshTool, ToolSuffix)

if DoBuildShadowSkin:
	printLog(log, ">>> BuildShadowSkin <<<")
	printLog(log, "********************************")
	printLog(log, "********      TODO      ********")
	printLog(log, "********************************")

mkPath(log, ExportBuildDirectory + "/" + ShapeNotOptimizedExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + ShapeClodtexBuildDirectory)
if ClodConfigFile != "":
	mkPath(log, ExportBuildDirectory + "/" + ClodExportDirectory)
	printLog(log, ">>> Build CLodTex <<<")
	subprocess.call([ BuildClodtex, "-d", DatabaseDirectory + "/" + ClodConfigFile, ExportBuildDirectory + "/" + ClodExportDirectory, ExportBuildDirectory + "/" + ShapeNotOptimizedExportDirectory, ExportBuildDirectory + "/" + ShapeClodtexBuildDirectory ])
else:
	printLog(log, ">>> Copy Shape <<<")
	copyFilesExtNoTreeIfNeeded(log, ExportBuildDirectory + "/" + ShapeNotOptimizedExportDirectory, ExportBuildDirectory + "/" + ShapeClodtexBuildDirectory, ".shape")

# copy lightmap_not_optimized to lightmap
printLog(log, ">>> Optimize lightmaps <<<")
mkPath(log, ExportBuildDirectory + "/" + ShapeLightmapNotOptimizedExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + ShapeLightmapBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + ShapeTagExportDirectory)
mkPath(log, ExportBuildDirectory + "/" + ShapeClodtexBuildDirectory)
removeFilesRecursive(log, ExportBuildDirectory + "/" + ShapeLightmapBuildDirectory)
copyFiles(log, ExportBuildDirectory + "/" + ShapeLightmapNotOptimizedExportDirectory, ExportBuildDirectory + "/" + ShapeLightmapBuildDirectory)
# Optimize lightmaps if any. Additionnaly, output a file indicating which lightmaps are 8 bits
subprocess.call([ LightmapOptimizer, ExportBuildDirectory + "/" + ShapeLightmapBuildDirectory, ExportBuildDirectory + "/" + ShapeClodtexBuildDirectory, ExportBuildDirectory + "/" + ShapeTagExportDirectory, ExportBuildDirectory + "/" + ShapeLightmapBuildDirectory + "/list_lm_8bit.txt" ])

# Convert lightmap in 16 bits mode if they are not 8 bits lightmap
printLog(log, ">>> Convert lightmaps in 16 or 8 bits <<<")
mkPath(log, ExportBuildDirectory + "/" + ShapeLightmapBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + ShapeLightmap16BitsBuildDirectory)
lightMapTgas = findFilesNoSubdir(log, ExportBuildDirectory + "/" + ShapeLightmapBuildDirectory, ".tga")
listLm8Bit = [ ]
listLm8BitFile = open(ExportBuildDirectory + "/" + ShapeLightmapBuildDirectory + "/list_lm_8bit.txt", "r")
for line in listLm8BitFile:
	lineStrip = line.strip()
	if (len(lineStrip) > 0):
		listLm8Bit += [ lineStrip ]
for lightMapTga in lightMapTgas:
	srcTga = ExportBuildDirectory + "/" + ShapeLightmapBuildDirectory + "/" + lightMapTga
	dstTga = ExportBuildDirectory + "/" + ShapeLightmap16BitsBuildDirectory + "/" + lightMapTga
	if needUpdateLogRemoveDest(log, srcTga, dstTga):
		if lightMapTga in listLm8Bit: # THIS MAY NOT WORK, PLEASE VERIFY CONTENTS OF list_lm_8bit.txt!!!
			subprocess.call([ TgaToDds, srcTga, "-o", dstTga, "-a", "tga8" ])
		else:
			subprocess.call([ TgaToDds, srcTga, "-o", dstTga, "-a", "tga16" ])

# Corse meshes for this process ?
if len(CoarseMeshTextureNames) > 0:
	printLog(log, ">>> Build coarse meshes <<<")
	shapeWithCoarseMesh = ExportBuildDirectory + "/" + ShapeWithCoarseMeshExportDirectory
	mkPath(log, shapeWithCoarseMesh)
	shapeWithCoarseMeshBuilded = ExportBuildDirectory + "/" + ShapeWithCoarseMeshBuildDirectory
	mkPath(log, shapeWithCoarseMeshBuilded)
	cf = open("config_generated.cfg", "w")
	cf.write("texture_mul_size = " + TextureMulSizeValue + ";\n")
	cf.write("\n")
	cf.write("search_path = \n")
	cf.write("{\n")
	cf.write("\t\"" + shapeWithCoarseMesh + "\", \n")
	for dir in MapLookupDirectories:
		cf.write("\t\"" + ExportBuildDirectory + "/" + dir + "\", \n")
	cf.write("};\n")
	cf.write("\n")
	cf.write("list_mesh = \n")
	cf.write("{\n")
	# For each shape with coarse mesh
	files = findFiles(log, shapeWithCoarseMesh, "", ".shape")
	for file in files:
		sourceFile = shapeWithCoarseMesh + "/" + file
		if os.path.isfile(sourceFile):
			destFile = shapeWithCoarseMeshBuilded + "/" + file
			cf.write("\t\"" + file + "\", \"" + destFile + "\", \n")
	cf.write("};\n")
	cf.write("\n")
	cf.write("output_textures = \n")
	cf.write("{\n")
	# For each shape with coarse mesh
	for tn in CoarseMeshTextureNames:
		cf.write("\t\"" + shapeWithCoarseMesh + "/" + tn + ".tga\", \n")
	cf.write("};\n")
	cf.close()
	subprocess.call([ BuildCoarseMesh, "config_generated.cfg" ])
	os.remove("config_generated.cfg")
	# Convert the coarse texture to dds
	for tn in CoarseMeshTextureNames:
		subprocess.call([ TgaToDds, shapeWithCoarseMesh + "/" + tn + ".tga", "-o", shapeWithCoarseMeshBuilded + "/" + tn + ".dds", "-a", "5" ])
else:
	printLog(log, ">>> No coarse meshes <<<")

log.close()


# end of file
