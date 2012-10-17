#!/usr/bin/python
# 
# \file generate_ecosystem_projects.py
# \brief Run all setup processes
# \date 2010-09-02 10:36GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Generate ecosystem projects
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
sys.path.append("../configuration")
if os.path.isfile("generate_ecosystem_projects.log"):
	os.remove("generate_ecosystem_projects.log")
log = open("generate_ecosystem_projects.log", "w")
from scripts import *
from buildsite import *
from tools import *

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Generate ecosystem projects")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

templateDir = os.getcwd().replace("\\", "/") + "/ecosystem_project_template"
mkPath(log, templateDir)

os.chdir("..")


# Scroll down to add an ecosystem.



DefaultShapeExportOptExportLighting = "true"
DefaultShapeExportOptShadow = "true"
DefaultShapeExportOptLightingLimit = "1"
DefaultShapeExportOptLumelSize = "0.25"
DefaultShapeExportOptOversampling = "1"
DefaultShapeExportOpt8BitsLightmap = "true"
DefaultShapeExportOptLightmapLog = "true"
DefaultTextureMulSizeValue = "1.5"
DefaultSeasonSuffixes = [ "sp" ] + [ "su" ] + [ "au" ] + [ "wi" ]
DefaultMapSubdirectories = [ ]
DefaultTileDirectories = [ ]


ShapeExportOptExportLighting = DefaultShapeExportOptExportLighting
ShapeExportOptShadow = DefaultShapeExportOptShadow
ShapeExportOptLightingLimit = DefaultShapeExportOptLightingLimit
ShapeExportOptLumelSize = DefaultShapeExportOptLumelSize
ShapeExportOptOversampling = DefaultShapeExportOptOversampling
ShapeExportOpt8BitsLightmap = DefaultShapeExportOpt8BitsLightmap
ShapeExportOptLightmapLog = DefaultShapeExportOptLightmapLog
TextureMulSizeValue = DefaultTextureMulSizeValue
SeasonSuffixes = DefaultSeasonSuffixes
MapSubdirectories = DefaultMapSubdirectories
TileDirectories = DefaultTileDirectories


PreGenDateTimeStamp = None
PreGenEcosystemName = None
PreGenDatabaseRootName = None
PreGenCoarseMeshTextureNames = None
PreGenMultipleTilesPostfix = None
PreGenMapSubdirectories = None
PreGenTileSourceDirectories = None


def transformLine(line):
	newline = line.replace("%PreGenWarning%", "WARNING : this is a generated file, don't change it !")
	newline = newline.replace("%PreGenDateTimeStamp%", PreGenDateTimeStamp)
	
	newline = newline.replace("%PreGenEcosystemName%", PreGenEcosystemName)
	newline = newline.replace("%PreGenDatabaseRootName%", PreGenDatabaseRootName)
	
	newline = newline.replace("%PreGenCoarseMeshTextureNames%", PreGenCoarseMeshTextureNames)
	newline = newline.replace("%PreGenMultipleTilesPostfix%", PreGenMultipleTilesPostfix)
	newline = newline.replace("%PreGenMapSubdirectories%", PreGenMapSubdirectories)
	newline = newline.replace("%PreGenTileSourceDirectories%", PreGenTileSourceDirectories)
	
	newline = newline.replace("%PreGenShapeExportOptExportLighting%", ShapeExportOptExportLighting)
	newline = newline.replace("%PreGenShapeExportOptShadow%", ShapeExportOptShadow)
	newline = newline.replace("%PreGenShapeExportOptLightingLimit%", ShapeExportOptLightingLimit)
	newline = newline.replace("%PreGenShapeExportOptLumelSize%", ShapeExportOptLumelSize)
	newline = newline.replace("%PreGenShapeExportOptOversampling%", ShapeExportOptOversampling)
	newline = newline.replace("%PreGenShapeExportOpt8BitsLightmap%", ShapeExportOpt8BitsLightmap)
	newline = newline.replace("%PreGenShapeExportOptLightmapLog%", ShapeExportOptLightmapLog)
	newline = newline.replace("%PreGenTextureMulSizeValue%", TextureMulSizeValue)
	newline = newline.replace("%PreGenTileSourceDirectories%", PreGenTileSourceDirectories)
	
	return newline

def generateFile(sourceFile, destFile):
	srcf = open(sourceFile, "r")
	dstf = open(destFile, "w")
	printLog(log, "WRITE " + destFile)
	for line in srcf:
		dstf.write(transformLine(line))
	dstf.close()
	srcf.close()

def generateEcosystem(ecosystemName, databaseRootName):
	global PreGenEcosystemName
	PreGenEcosystemName = ecosystemName
	global PreGenDatabaseRootName
	PreGenDatabaseRootName = databaseRootName
	global PreGenDateTimeStamp
	PreGenDateTimeStamp = time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time()))
	
	global PreGenMultipleTilesPostfix
	PreGenMultipleTilesPostfix = ""
	global PreGenCoarseMeshTextureNames
	PreGenCoarseMeshTextureNames = ""
	global PreGenTileSourceDirectories
	PreGenTileSourceDirectories = ""
	for suffix in SeasonSuffixes:
		PreGenMultipleTilesPostfix += "MultipleTilesPostfix += [ \"_" + suffix + "\" ]\n"
		PreGenCoarseMeshTextureNames += "CoarseMeshTextureNames += [ \"nel_coarse_mesh_\" + EcosystemName + \"_" + suffix + "\" ]\n"
		for tiledir in TileDirectories:
			PreGenTileSourceDirectories += "TilesSourceDirectories += [ \"landscape/_texture_tiles/\" + EcosystemName + \"_" + suffix + "/" + tiledir + "\" ]\n"
	global PreGenMapSubdirectories
	PreGenMapSubdirectories = ""
	for subdir in MapSubdirectories:
		PreGenMapSubdirectories += "MapSourceDirectories += [ DatabaseRootPath + \"/decors/_textures/" + subdir + "\" ]\n"
	
	destDir = WorkspaceDirectory + "/ecosystems/" + ecosystemName
	mkPath(log, destDir)
	
	generateFile(templateDir + "/process.py", destDir + "/process.py")
	generateFile(templateDir + "/directories.py", destDir + "/directories.py")
	
	return



# Add new ecosystems below this line.



# DESERT
ShapeExportOptExportLighting = DefaultShapeExportOptExportLighting
ShapeExportOptShadow = DefaultShapeExportOptShadow
ShapeExportOptLightingLimit = DefaultShapeExportOptLightingLimit
ShapeExportOptLumelSize = DefaultShapeExportOptLumelSize
ShapeExportOptOversampling = DefaultShapeExportOptOversampling
ShapeExportOpt8BitsLightmap = DefaultShapeExportOpt8BitsLightmap
ShapeExportOptLightmapLog = DefaultShapeExportOptLightmapLog
TextureMulSizeValue = DefaultTextureMulSizeValue
SeasonSuffixes = DefaultSeasonSuffixes
MapSubdirectories = [ ]
MapSubdirectories += [ "vegetations" ]
TileDirectories = [ ]
TileDirectories += [ "1.5-marecage_profond" ]
TileDirectories += [ "1-marecages" ]
TileDirectories += [ "2-citees" ]
TileDirectories += [ "3-fond_canyon" ]
TileDirectories += [ "4.2-boisbandeclair" ]
TileDirectories += [ "4.5-desert2boisbande" ]
TileDirectories += [ "4-falaise_bois_bande" ]
TileDirectories += [ "5-falaise_normales" ]
TileDirectories += [ "6.5-desertalternatif" ]
TileDirectories += [ "6-desert" ]
TileDirectories += [ "7-routes" ]
TileDirectories += [ "8-foretbrule" ]
generateEcosystem("desert", "fyros")


# JUNGLE
ShapeExportOptExportLighting = DefaultShapeExportOptExportLighting
ShapeExportOptShadow = DefaultShapeExportOptShadow
ShapeExportOptLightingLimit = DefaultShapeExportOptLightingLimit
ShapeExportOptLumelSize = DefaultShapeExportOptLumelSize
ShapeExportOptOversampling = DefaultShapeExportOptOversampling
ShapeExportOpt8BitsLightmap = "false"
ShapeExportOptLightmapLog = DefaultShapeExportOptLightmapLog
TextureMulSizeValue = DefaultTextureMulSizeValue
SeasonSuffixes = DefaultSeasonSuffixes
MapSubdirectories = [ ]
MapSubdirectories += [ "vegetations" ]
TileDirectories = [ ]
TileDirectories += [ "10-crevassejungle" ]
TileDirectories += [ "11-paroisjungle" ]
TileDirectories += [ "12-vasejungle" ]
TileDirectories += [ "1-junglemousse" ]
TileDirectories += [ "2-junglefeuilles" ]
TileDirectories += [ "3-jungleherbesseche" ]
TileDirectories += [ "4-jungleherbevieille" ]
TileDirectories += [ "5-jungleterreaux" ]
TileDirectories += [ "6-junglegoo" ]
TileDirectories += [ "7-sciurejungle" ]
TileDirectories += [ "8-terrejungle" ]
TileDirectories += [ "9-falaisejungle" ]
TileDirectories += [ "Transitions" ]
generateEcosystem("jungle", "jungle")


# PRIMES RACINES
ShapeExportOptExportLighting = DefaultShapeExportOptExportLighting
ShapeExportOptShadow = DefaultShapeExportOptShadow
ShapeExportOptLightingLimit = DefaultShapeExportOptLightingLimit
ShapeExportOptLumelSize = DefaultShapeExportOptLumelSize
ShapeExportOptOversampling = DefaultShapeExportOptOversampling
ShapeExportOpt8BitsLightmap = "false"
ShapeExportOptLightmapLog = DefaultShapeExportOptLightmapLog
TextureMulSizeValue = DefaultTextureMulSizeValue
SeasonSuffixes = DefaultSeasonSuffixes
MapSubdirectories = [ ]
MapSubdirectories += [ "vegetations" ]
MapSubdirectories += [ "batiments" ]
TileDirectories = [ ]
TileDirectories += [ "PR-creux" ]
TileDirectories += [ "PR-dome-moussu" ]
TileDirectories += [ "PR-kitiniere" ]
TileDirectories += [ "PR-mousse-licken" ]
TileDirectories += [ "PR-mousse-spongieus" ]
TileDirectories += [ "PR-parois" ]
TileDirectories += [ "PR-sol-mousse" ]
TileDirectories += [ "PR-souche" ]
TileDirectories += [ "PR-stalagmite" ]
TileDirectories += [ "PR-terre" ]
TileDirectories += [ "aditif" ]
generateEcosystem("primes_racines", "primes_racines")


# LACUSTRE
ShapeExportOptExportLighting = DefaultShapeExportOptExportLighting
ShapeExportOptShadow = DefaultShapeExportOptShadow
ShapeExportOptLightingLimit = "0"
ShapeExportOptLumelSize = DefaultShapeExportOptLumelSize
ShapeExportOptOversampling = "8"
ShapeExportOpt8BitsLightmap = "false"
ShapeExportOptLightmapLog = DefaultShapeExportOptLightmapLog
TextureMulSizeValue = DefaultTextureMulSizeValue
SeasonSuffixes = DefaultSeasonSuffixes
MapSubdirectories = [ ]
MapSubdirectories += [ "vegetations" ]
TileDirectories = [ ]
TileDirectories += [ "1a-sable-marin" ]
TileDirectories += [ "1-plages" ]
TileDirectories += [ "2-iles" ]
TileDirectories += [ "2-ilesa" ]
TileDirectories += [ "2-iles-marines" ]
TileDirectories += [ "3-fondmarin2plage" ]
TileDirectories += [ "4-marecages" ]
TileDirectories += [ "5-marecages" ]
TileDirectories += [ "5-parois-marine" ]
TileDirectories += [ "6-fond_marin" ]
TileDirectories += [ "7-bassesiles" ]
TileDirectories += [ "7-mousseter" ]
TileDirectories += [ "7-racines" ]
TileDirectories += [ "8-mousse_marine" ]
TileDirectories += [ "constructible" ]
generateEcosystem("lacustre", "tryker")




printLog(log, "")
log.close()
