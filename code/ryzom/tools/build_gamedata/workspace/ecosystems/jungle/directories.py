#!/usr/bin/python
# 
# \file directories.py
# \brief Directories configuration
# \date 2010-05-24 06:34GMT
# \author Jan Boon (Kaetemi)
# \date 2001-2005
# \author Nevrax
# Python port of game data build pipeline.
# Directories configuration.
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

# *** SOURCE DIRECTORIES IN THE DATABASE ***


# *** ECOSYSTEM NAME ***
EcosystemName = "jungle"
EcosystemPath = "ecosystems/" + EcosystemName

# Shape directories
ShapeSourceDirectories = [ ]
ShapeSourceDirectories += [ "stuff/" + EcosystemName + "/decors/vegetations" ]
ShapeSourceDirectories += [ "landscape/ligo/" + EcosystemName + "/max" ]

# Maps directories
MapSourceDirectories = [ ]
MapSourceDirectories += [ "stuff/" + EcosystemName + "/decors/_textures/Vegetations" ]
MapSourceDirectories += [ "landscape/microveget/" + EcosystemName + "" ]
MapSourceDirectories += [ "landscape/water/meshes/" + EcosystemName + "" ]

MapUncompressedSourceDirectories = [ ]

# Tiles directories
TilesSourceDirectories = [ ]

TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/1-junglemousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/2-junglefeuilles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/3-jungleherbesseche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/4-jungleherbevieille" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/5-jungleterreaux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/6-junglegoo" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/7-sciurejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/8-terrejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/9-falaisejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/10-crevassejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/11-paroisjungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/12-vasejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/Transitions" ]

TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/1-junglemousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/2-junglefeuilles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/3-jungleherbesseche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/4-jungleherbevieille" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/5-jungleterreaux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/6-junglegoo" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/7-sciurejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/8-terrejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/9-falaisejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/10-crevassejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/11-paroisjungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/12-vasejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/Transitions" ]

TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/1-junglemousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/2-junglefeuilles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/3-jungleherbesseche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/4-jungleherbevieille" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/5-jungleterreaux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/6-junglegoo" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/7-sciurejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/8-terrejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/9-falaisejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/10-crevassejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/11-paroisjungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/12-vasejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/Transitions" ]

TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/1-junglemousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/2-junglefeuilles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/3-jungleherbesseche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/4-jungleherbevieille" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/5-jungleterreaux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/6-junglegoo" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/7-sciurejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/8-terrejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/9-falaisejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/10-crevassejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/11-paroisjungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/12-vasejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/Transitions" ]

# Tiles root directory
TileRootSourceDirectory = "landscape/_texture_tiles/" + EcosystemName

# Displace directoriy
DisplaceSourceDirectories = [ ]
DisplaceSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "/displace" ]

# Bank directory
BankSourceDirectory = "landscape/_texture_tiles/" + EcosystemName

# Vegetable set directories
VegetSetSourceDirectories = [ ]
VegetSetSourceDirectories += [ "landscape/microveget/" + EcosystemName ]

# Veget directories
VegetSourceDirectories = [ ]
VegetSourceDirectories += [ "landscape/microveget/" + EcosystemName ]

# Ligo directories
LigoBaseSourceDirectory = "landscape/ligo/" + EcosystemName
LigoMaxSourceDirectory = LigoBaseSourceDirectory + "/max"

# Zone directories
ZoneSourceDirectory = "landscape/zones/" + EcosystemName

# Ig landscape directories
IgLandSourceDirectory = "_invalid"

# Ig other directories
IgOtherSourceDirectory = "_invalid"

# PACS primitives directories
PacsPrimSourceDirectories = [ ]
PacsPrimSourceDirectories += [ "stuff/" + EcosystemName + "/decors/vegetations" ]


# *** EXPORT DIRECTORIES FOR THE BUILD PIPELINE ***

# Shape directories
ShapeTagExportDirectory = "ecosystems/" + EcosystemName + "/shape_tag"
ShapeExportDirectory = "ecosystems/" + EcosystemName + "/shape"
ShapeWithCoarseMeshExportDirectory = "ecosystems/" + EcosystemName + "/shape_with_coarse_mesh"
ShapeLightmapNotOptimizedExportDirectory = "ecosystems/" + EcosystemName + "/shape_lightmap_not_optimized"
ShapeAnimExportDirectory = "ecosystems/" + EcosystemName + "/shape_anim"

# Smallbank directories
SmallbankExportDirectory = "ecosystems/" + EcosystemName + "/smallbank"

# Tiles directories
TilesExportDirectory = "ecosystems/" + EcosystemName + "/tiles"

# Tiles directories
DisplaceExportDirectory = "ecosystems/" + EcosystemName + "/diplace"

# Veget directories
VegetExportDirectory = "ecosystems/" + EcosystemName + "/veget"
VegetTagExportDirectory = "ecosystems/" + EcosystemName + "/veget_tag"

# Veget Set directories
VegetSetExportDirectory = "ecosystems/" + EcosystemName + "/veget_set"

# Ligo directories
LigoDatabaseExportDirectory = "landscape/ligo/" + EcosystemName
LigoDatabaseIgExportDirectory = LigoDatabaseExportDirectory + "/igs"
LigoDatabaseZoneExportDirectory = LigoDatabaseExportDirectory + "/zones"
LigoDatabaseZoneLigoExportDirectory = LigoDatabaseExportDirectory + "/zoneligos"
LigoDatabaseCmbExportDirectory = LigoDatabaseExportDirectory + "/cmb"
LigoTagExportDirectory = "ecosystems/" + EcosystemName + "/ligo_tag"

# Zone directories
ZoneExportDirectory = "ecosystems/" + EcosystemName + "/zone"


# *** BUILD DIRECTORIES FOR THE BUILD PIPELINE ***

# Map directories
MapBuildDirectory = "ecosystems/" + EcosystemName + "/map"
MapPanoplyBuildDirectory = "ecosystems/" + EcosystemName + "/map_panoply"

# Shape directories
ShapeWithCoarseMeshBuildDirectory = "ecosystems/" + EcosystemName + "/shape_with_coarse_mesh_builded"

# Farbank directories
FarbankBuildDirectory = "ecosystems/" + EcosystemName + "/farbank"

# Ig directories ************** TODO CONFIRM IN IG BUILD PROCESS ************ FIX RBANK IF NEEDED ***********
IgLandBuildDirectory = "_invalid"
IgVillageBuildDirectory = "_invalid"

# Rbank directories
RbankOutputBuildDirectory = "_invalid"
RbankMeshBuildDirectory = "_invalid"

# Ligo directories


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Client directories
ClientSetupDirectories = [ ]
ClientSetupDirectories += [ "jungle_shapes" ]
ClientSetupDirectories += [ "jungle_maps" ]
ClientSetupDirectories += [ "jungle_tiles" ]
ClientSetupDirectories += [ "jungle_displaces" ]
ClientSetupDirectories += [ "jungle_bank" ]
ClientSetupDirectories += [ "jungle_vegetables" ]
ClientSetupDirectories += [ "jungle_vegetable_sets" ]
ClientSetupDirectories += [ "jungle_pacs_prim" ]
ClientSetupDirectories += [ "jungle_lightmaps" ]

# Shape directory
MapClientDirectory = "jungle_maps"

# Shape directory
ShapeClientDirectory = "jungle_shapes"

# Map directory
BitmapClientDirectory = "jungle_maps"

# Lightmap directory
LightmapClientDirectory = "jungle_lightmaps"

# Tile directory
TilesClientDirectory = "jungle_tiles"

# Displace directory
DisplaceClientDirectory = "jungle_displaces"

# Bank directory
BankClientDirectory = "jungle_bank"

# Vegetable set directory
VegetSetClientDirectory = "jungle_vegetable_sets"

# Vegetable shape directory
VegetClientDirectory = "jungle_vegetables"

# PACS primitives directories
PacsPrimitiveClientDirectory = "jungle_pacs_prim"
