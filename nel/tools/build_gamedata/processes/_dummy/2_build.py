#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build dummy
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build dummy
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
printLog(log, "--- Build dummy")
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

#if 1: # todo: CoarseMeshTextureNames length > 0 ...
#	printLog(log, ">>> Build coarse meshes <<<")
#	dummyWithCoarseMesh = ExportBuildDirectory + "/" + DummyWithCoarseMeshExportDirectory
#	mkPath(log, dummyWithCoarseMesh)
#	dummyWithCoarseMeshBuilded = ExportBuildDirectory + "/" + DummyWithCoarseMeshBuildDirectory
#	mkPath(log, dummyWithCoarseMeshBuilded)
#	cf = open("confdummy_generated.cfg", "w")
#	cf.write("texture_mul_size = " + TextureMulSizeValue + ";\n")
#	cf.write("\n")
#	cf.write("search_path = \n")
#	cf.write("{\n")
#	cf.write("\t\"" + dummyWithCoarseMesh + "\", \n")
#	for dir in MapSourceDirectories:
#		cf.write("\t\"" + DatabaseDirectory + "/" + dir + "\", \n")
#	cf.write("};\n")
#	cf.write("\n")
#	cf.write("list_mesh = \n")
#	cf.write("{\n")
#	# For each dummy with coarse mesh
#	files = findFiles(log, dummyWithCoarseMesh, "", ".dummy")
#	for file in files:
#		sourceFile = dummyWithCoarseMesh + "/" + file
#		if os.path.isfile(sourceFile):
#			destFile = dummyWithCoarseMeshBuilded + "/" + file
#			cf.write("\t\"" + file + "\", \"" + destFile + "\", \n")
#	cf.write("};\n")
#	cf.write("\n")
#	cf.write("output_textures = \n")
#	cf.write("{\n")
#	# For each dummy with coarse mesh
#	for tn in CoarseMeshTextureNames:
#		cf.write("\t\"" + dummyWithCoarseMesh + "/" + tn + ".tga\", \n")
#	cf.write("};\n")
#	cf.close()
#	subprocess.call([ BuildCoarseMesh, "confdummy_generated.cfg" ])
#	os.remove("confdummy_generated.cfg")
#	for tn in CoarseMeshTextureNames:
#		subprocess.call([ TgaToDds, dummyWithCoarseMesh + "/" + tn + ".tga", "-o", dummyWithCoarseMeshBuilded + "/" + tn + ".dds", "-a", "5" ])

log.close()


# end of file
