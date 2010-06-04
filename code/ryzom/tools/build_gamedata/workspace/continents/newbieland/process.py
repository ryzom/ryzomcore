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
ProcessToComplete += [ "ligo" ] # not fully implemented, works for this process (not yet), but does not export max files
ProcessToComplete += [ "zone" ]
#ProcessToComplete += [ "ig" ] # not implemented
ProcessToComplete += [ "zone_light" ]
#ProcessToComplete += [ "rbank" ]
#ProcessToComplete += [ "ig_light" ]
#ProcessToComplete += [ "ps" ]


# *** ECOSYSTEM AND CONTINENT NAMES ***
EcosystemName = "jungle"
ContinentName = "newbieland"


# *** LANDSCAPE NAME ***
LandscapeName = ContinentName

# *** CONTINENT FILE ***
ContinentFile = ContinentName + "/" + ContinentName + ".continent"


# *** BANK EXPORT OPTIONS ***

# Name of the tilebank to use
BankTileBankName = EcosystemName


# *** LIGO OPTIONS ***
LigoExportLand = ContinentName + ".land"
LigoExportOnePass = 0
LigoExportColormap = "colormap_" + ContinentName + ".png"
LigoExportHeightmap1 = "big_" + ContinentName + ".png"
LigoExportZFactor1 = "1.0"
LigoExportHeightmap2 = "noise_" + ContinentName + ".png"
LigoExportZFactor2 = "0.5"
LigoTileBankFile = "landscape/_texture_tiles/" + EcosystemName + "/" + EcosystemName + ".bank"

# *** ZONE REGIONS ( up-left, down-right ) ***
ZoneRegion = [ "65_bz", "77_cs" ]

# *** RBANK OPTIONS ***

# Options
RbankReduceSurfaces = 1
RbankSmoothBorders = 1
RbankComputeElevation = 0
RbankComputeLevels = 1
RbankLinkElements = 1
RbankCutEdges = 1
RbankUseZoneSquare = 0

# Region to compute ( ALPHA UPPER CASE! )
RbankZoneUl = "65_BZ"
RbankZoneDr = "77_CS"

# Output names
RbankRbankName = LandscapeName

# Import ig pathes
RbankIgPaths = [ ] # ExportBuildDirectory/...
RbankIgPaths += [ "continents/" + ContinentName + "/ig_other" ]
RbankIgPaths += [ "continents/" + ContinentName + "/ig_land" ]

# Import shape pathes
RbankShapePaths = [ ] # ExportBuildDirectory/...
RbankShapePaths += [ "continents/" + ContinentName + "/shape" ]
RbankShapePaths += [ "continents/" + ContinentName + "/shape_with_coarse_mesh_builded" ]
RbankShapePaths += [ "ecosystems/" + EcosystemName + "/shape" ]
RbankShapePaths += [ "ecosystems/" + EcosystemName + "/shape_with_coarse_mesh_builded" ]
RbankShapePaths += [ "common/sfx/ps" ]
# RbankShapePaths += [ "l:/leveldesign/world_edit_files" ]

