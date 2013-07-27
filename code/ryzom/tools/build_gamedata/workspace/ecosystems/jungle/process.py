#!/usr/bin/python
# 
# #################################################################
# ## WARNING : this is a generated file, don't change it !
# #################################################################
# 
# \file config.py
# \brief Process configuration
# \date 2013-07-27-02-01-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Process configuration for 'jungle' ecosystem.
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

# *** PROCESS CONFIGURATION ***

# *** PROCESS CONFIG ***
ProcessToComplete = [ ]
ProcessToComplete += [ "shape" ]
ProcessToComplete += [ "map" ]
ProcessToComplete += [ "smallbank" ]
ProcessToComplete += [ "farbank" ]
ProcessToComplete += [ "tiles" ]
ProcessToComplete += [ "displace" ]
ProcessToComplete += [ "veget" ]
ProcessToComplete += [ "vegetset" ]
ProcessToComplete += [ "ligo" ]
ProcessToComplete += [ "pacs_prim" ]


# *** ECOSYSTEM AND CONTINENT NAMES ***

EcosystemName = "jungle"
EcosystemPath = "ecosystems/" + EcosystemName
CommonName = EcosystemName
CommonPath = EcosystemPath


# *** MAP EXPORT OPTIONS ***
PanoplyFileList = [ ]
HlsBankFileName = ""

# *** SHAPE EXPORT OPTIONS ***

# Compute lightmaps ?
ShapeExportOptExportLighting = "true"

# Cast shadow in lightmap ?
ShapeExportOptShadow = "true"

# Lighting limits. 0 : normal, 1 : soft shadows
ShapeExportOptLightingLimit = 1

# Lightmap lumel size
ShapeExportOptLumelSize = "0.25"

# Oversampling value. Can be 1, 2, 4 or 8
ShapeExportOptOversampling = 1

# Does the lightmap must be generated in 8 bits format ?
ShapeExportOpt8BitsLightmap = "false"

# Does the lightmaps export must generate logs ?
ShapeExportOptLightmapLog = "true"

# Coarse mesh texture mul size
TextureMulSizeValue = "1.5"

DoBuildShadowSkin = 0

ClodConfigFile = ""

# *** COARSE MESH TEXTURE NAME ***
CoarseMeshTextureNames = [ ]
CoarseMeshTextureNames += [ "nel_coarse_mesh_" + EcosystemName + "_sp" ]
CoarseMeshTextureNames += [ "nel_coarse_mesh_" + EcosystemName + "_su" ]
CoarseMeshTextureNames += [ "nel_coarse_mesh_" + EcosystemName + "_au" ]
CoarseMeshTextureNames += [ "nel_coarse_mesh_" + EcosystemName + "_wi" ]

# *** POSTFIX USED BY THE MULTIPLE TILES SYSTEM ***
MultipleTilesPostfix = [ ]
MultipleTilesPostfix += [ "_sp" ]
MultipleTilesPostfix += [ "_su" ]
MultipleTilesPostfix += [ "_au" ]
MultipleTilesPostfix += [ "_wi" ]

# *** BANK EXPORT OPTIONS ***

# Name of the tilebank to use
BankTileBankName = EcosystemName

# *** RBANK EXPORT OPTIONS ***

# Output names
RbankRbankName = "_invalid"

# *** LIGO OPTIONS ***

LigoExportLand = ""
LigoExportOnePass = 0

# *** MAPS OPTIONS ***

ReduceBitmapFactor = 0
# list all panoply files
MapPanoplyFileList = None
# name of the .hlsbank to build.
MapHlsBankFileName = None

# *** SHAPE BUILD OPTIONS *

DoBuildShadowSkin = False
ClodConfigFile = ""

# *** PACS_PRIM OPTIONS ***
WantLandscapeColPrimPacsList = True
