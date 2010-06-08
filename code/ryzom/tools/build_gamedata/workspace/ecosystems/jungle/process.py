#!/usr/bin/python
# 
# \file config.py
# \brief Process configuration
# \date 2010-05-24 06:30GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Process configuration.
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
ProcessToComplete += [ "shape" ] # not fully implemented, but works for this process
ProcessToComplete += [ "map" ] # not fully implemented, but works for this process
ProcessToComplete += [ "smallbank" ] # OK
ProcessToComplete += [ "farbank" ] # OK
ProcessToComplete += [ "tiles" ] # OK
ProcessToComplete += [ "displace" ] # OK
ProcessToComplete += [ "veget" ] # OK
ProcessToComplete += [ "vegetset" ] # OK
ProcessToComplete += [ "ligo" ] # not fully implemented, works for this process, but does not export max files
#ProcessToComplete += [ "pacs_prim" ]

# *** MAP EXPORT OPTIONS ***
PanoplyFileList = [ ]
HlsBankFileName = ""

# *** SHAPE EXPORT OPTIONS ***

# Compute lightmaps ?
ShapeExportOptExportLighting = "true"

# Cast shadow in lightmap ?
ShapeExportOptShadow = "true"

# Lighting limits. 0 : normal, 1 : soft shadows
ShapeExportOptLightingLimit = 0

# Lightmap lumel size
ShapeExportOptLumelSize = "0.25"

# Oversampling value. Can be 1, 2, 4 or 8
ShapeExportOptOversampling = 1

# Does the lightmap must be generated in 8 bits format ?
ShapeExportOpt8BitsLightmap = "false"

# Does the lightmaps export must generate logs ?
ShapeExportOptLightmapLog = "false"

# Coarse mesh texture mul size
TextureMulSizeValue = "1.5"

# *** COARSE MESH TEXTURE NAME ***
CoarseMeshTextureNames = [ ]
CoarseMeshTextureNames += [ "nel_coarse_mesh_jungle_sp" ]
CoarseMeshTextureNames += [ "nel_coarse_mesh_jungle_su" ]
CoarseMeshTextureNames += [ "nel_coarse_mesh_jungle_au" ]
CoarseMeshTextureNames += [ "nel_coarse_mesh_jungle_wi" ]

# *** POSTFIX USED BY THE MULTIPLE TILES SYSTEM ***
MultipleTilesPostfix = [ ]
MultipleTilesPostfix += [ "_sp" ]
MultipleTilesPostfix += [ "_su" ]
MultipleTilesPostfix += [ "_au" ]
MultipleTilesPostfix += [ "_wi" ]

# *** BANK EXPORT OPTIONS ***

# Name of the tilebank to use
BankTileBankName = "jungle"

# *** RBANK EXPORT OPTIONS ***

# Output names
RbankRbankName = "_invalid"

# *** LIGO OPTIONS ***

LigoExportLand = ""
LigoExportOnePass = 0

# *** MAPS OPTIONS ***

ReduceBitmapFactor = 0

# *** SHAPE BUILD OPTIONS *

DoBuildShadowSkin = False
ClodConfigFile = ""
