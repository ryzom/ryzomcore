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
ProcessToComplete += [ "properties" ]
ProcessToComplete += [ "shape" ]
ProcessToComplete += [ "ig" ]
ProcessToComplete += [ "rbank" ]
ProcessToComplete += [ "ig_light" ]
ProcessToComplete += [ "ps" ]
ProcessToComplete += [ "ai_wmap" ]


# *** ECOSYSTEM AND CONTINENT NAMES ***

EcosystemName = "desert"
EcosystemPath = "ecosystems/" + EcosystemName
ContinentName = "indoors"
ContinentPath = "continents/" + ContinentName
CommonName = ContinentName
CommonPath = ContinentPath


# *** LANDSCAPE NAME ***
LandscapeName = ContinentName

# *** CONTINENT FILE ***
ContinentFile = ContinentName + "/" + ContinentName + ".continent"



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
ShapeExportOptLightmapLog = "true"

# Coarse mesh texture mul size
TextureMulSizeValue = "1.5"

DoBuildShadowSkin = 0

ClodConfigFile = ""

# *** COARSE MESH TEXTURE NAME ***
CoarseMeshTextureNames = [ ]

# *** BANK EXPORT OPTIONS ***

# *** POSTFIX USED BY THE MULTIPLE TILES SYSTEM ***
MultipleTilesPostfix = [ ]
MultipleTilesPostfix += [ "_sp" ]
MultipleTilesPostfix += [ "_su" ]
MultipleTilesPostfix += [ "_au" ]
MultipleTilesPostfix += [ "_wi" ]

# Name of the tilebank to use
BankTileBankName = EcosystemName


# *** LIGO OPTIONS ***
LigoExportLand = ""
LigoExportOnePass = 0
LigoExportColormap = "colormap_" + ContinentName + ".png"
LigoExportHeightmap1 = "big_" + ContinentName + ".png"
LigoExportZFactor1 = "1.0"
LigoExportHeightmap2 = "noise_" + ContinentName + ".png"
LigoExportZFactor2 = "0.5"
LigoTileBankFile = "landscape/_texture_tiles/" + EcosystemName + "/" + EcosystemName + ".bank"

# *** ZONE REGIONS ( up-left, down-right ) ***
#ZoneRegions = [ ] 
#ZoneRegions += [ [ "1_aa" ] + [ "2_ai" ] ]

# *** RBANK OPTIONS ***

# Options
RBankVerbose = 0
RBankConsistencyCheck = 0
RbankReduceSurfaces = 1
RbankSmoothBorders = 1
RbankComputeElevation = 0
RbankComputeLevels = 1
RbankLinkElements = 1
RbankCutEdges = 1
RbankUseZoneSquare = 0

# Region to compute ( ALPHA UPPER CASE! )
RbankZoneUl = "1_AA"
RbankZoneDr = "2_AI"

# Output names
RbankRbankName = LandscapeName


# *** AI WMAP OPTIONS ***
AiWmapContinentName = ContinentName
AiWmapVerbose = 0
AiWmapStartPoints = [ ]

# matis
AiWmapStartPoints += [ ContinentName + " 20025 -432" ] # player room
AiWmapStartPoints += [ ContinentName + " 20120 -439" ] # hall room
AiWmapStartPoints += [ ContinentName + " 20190 -454" ] # guild room
AiWmapStartPoints += [ ContinentName + " 20281 -439" ] # role master/npc room

# zorai
AiWmapStartPoints += [ ContinentName + " 20363 -439" ] # player room
AiWmapStartPoints += [ ContinentName + " 20440 -444" ] # hall room
AiWmapStartPoints += [ ContinentName + " 20519 -466" ] # guild room
AiWmapStartPoints += [ ContinentName + " 20591 -442" ] # role master/npc room

# tryker
AiWmapStartPoints += [ ContinentName + " 20682 -439" ] # player room
AiWmapStartPoints += [ ContinentName + " 20755 -440" ] # hall room
# AiWmapStartPoints += [ ContinentName + " 20833 -439" ] # guild room
AiWmapStartPoints += [ ContinentName + " 20920 -439" ] # role master/npc room

# fyros
AiWmapStartPoints += [ ContinentName + " 20998 -441" ] # player room
AiWmapStartPoints += [ ContinentName + " 21079 -443" ] # hall room
AiWmapStartPoints += [ ContinentName + " 21162 -443" ] # guild room
AiWmapStartPoints += [ ContinentName + " 21239 -439" ] # role master/npc room
