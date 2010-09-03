#!/usr/bin/python
# 
# #################################################################
# ## WARNING : this is a generated file, don't change it !
# #################################################################
# 
# \file directories.py
# \brief Directories configuration
# \date 2010-09-03-10-06-GMT
# \author Jan Boon (Kaetemi)
# \date 2001-2005
# \author Nevrax
# Python port of game data build pipeline.
# Directories configuration for 'jungle' ecosystem.
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


# *** ECOSYSTEM AND CONTINENT NAMES ***

EcosystemName = "jungle"
EcosystemPath = "ecosystems/" + EcosystemName
CommonName = EcosystemName
CommonPath = EcosystemPath

DatabaseRootName = "jungle"
DatabaseRootPath = "stuff/" + DatabaseRootName


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Shape directories
ShapeSourceDirectories = [ ]
ShapeSourceDirectories += [ DatabaseRootPath + "/decors/vegetations" ]
ShapeSourceDirectories += [ "landscape/ligo/" + EcosystemName + "/max" ]

# Maps directories
MapSourceDirectories = [ ]
MapSourceDirectories += [ DatabaseRootPath + "/decors/_textures" ]
MapSourceDirectories += [ DatabaseRootPath + "/decors/_textures/vegetations" ]
MapSourceDirectories += [ "landscape/microveget/" + EcosystemName + "" ]
MapSourceDirectories += [ "landscape/water/meshes/" + EcosystemName + "" ]

MapUncompressedSourceDirectories = [ ]

# Tiles directories
TilesSourceDirectories = [ ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/10-crevassejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/11-paroisjungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/12-vasejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/1-junglemousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/2-junglefeuilles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/3-jungleherbesseche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/4-jungleherbevieille" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/5-jungleterreaux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/6-junglegoo" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/7-sciurejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/8-terrejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/9-falaisejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/Transitions" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/10-crevassejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/11-paroisjungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/12-vasejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/1-junglemousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/2-junglefeuilles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/3-jungleherbesseche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/4-jungleherbevieille" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/5-jungleterreaux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/6-junglegoo" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/7-sciurejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/8-terrejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/9-falaisejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/Transitions" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/10-crevassejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/11-paroisjungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/12-vasejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/1-junglemousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/2-junglefeuilles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/3-jungleherbesseche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/4-jungleherbevieille" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/5-jungleterreaux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/6-junglegoo" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/7-sciurejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/8-terrejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/9-falaisejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/Transitions" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/10-crevassejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/11-paroisjungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/12-vasejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/1-junglemousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/2-junglefeuilles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/3-jungleherbesseche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/4-jungleherbevieille" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/5-jungleterreaux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/6-junglegoo" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/7-sciurejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/8-terrejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/9-falaisejungle" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/Transitions" ]

# Tiles root directory
TileRootSourceDirectory = "landscape/_texture_tiles/" + EcosystemName

# Displace directory
DisplaceSourceDirectory = "landscape/_texture_tiles/" + EcosystemName + "/displace"

# Do not use, needs to be removed and fixed in processes
DisplaceSourceDirectories = [ ]
DisplaceSourceDirectories += [ DisplaceSourceDirectory ]

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
ZoneSourceDirectory = [ "landscape/zones/" + EcosystemName ] # For old snowballs style landscape when not using ligo

# Ig landscape directories
IgLandSourceDirectory = "_invalid"

# Ig other directories
IgOtherSourceDirectory = "_invalid"

# PACS primitives directories
PacsPrimSourceDirectories = [ ]
PacsPrimSourceDirectories += [ DatabaseRootPath + "/decors/vegetations" ]


# *** EXPORT DIRECTORIES FOR THE BUILD PIPELINE ***

# Shape directories
ShapeTagExportDirectory = CommonPath + "/shape_tag"
ShapeNotOptimizedExportDirectory = CommonPath + "/shape_not_optimized"
ShapeWithCoarseMeshExportDirectory = CommonPath + "/shape_with_coarse_mesh"
ShapeLightmapNotOptimizedExportDirectory = CommonPath + "/shape_lightmap_not_optimized"
ShapeAnimExportDirectory = CommonPath + "/shape_anim"

# Smallbank directories
SmallbankExportDirectory = CommonPath + "/smallbank"

# Tiles directories
TilesExportDirectory = CommonPath + "/tiles"

# Tiles directories
DisplaceExportDirectory = CommonPath + "/diplace"

# Veget directories
VegetExportDirectory = CommonPath + "/veget"
VegetTagExportDirectory = CommonPath + "/veget_tag"

# Veget Set directories
VegetSetExportDirectory = CommonPath + "/veget_set"

# Ligo directories
LigoDatabaseExportDirectory = "landscape/ligo/" + EcosystemName
LigoDatabaseIgExportDirectory = LigoDatabaseExportDirectory + "/igs"
LigoDatabaseZoneExportDirectory = LigoDatabaseExportDirectory + "/zones"
LigoDatabaseZoneLigoExportDirectory = LigoDatabaseExportDirectory + "/zoneligos"
LigoDatabaseCmbExportDirectory = LigoDatabaseExportDirectory + "/cmb"
LigoTagExportDirectory = CommonPath + "/ligo_tag"

# Zone directories
ZoneExportDirectory = CommonPath + "/zone"

# PACS primitives directories
PacsPrimExportDirectory = CommonPath + "/pacs_prim"


# *** BUILD DIRECTORIES FOR THE BUILD PIPELINE ***

# Map directories
MapBuildDirectory = CommonPath + "/map"
MapPanoplyBuildDirectory = CommonPath + "/map_panoply"

# Shape directories
ShapeClodtexBuildDirectory = CommonPath + "/shape_clodtex_build"
ShapeWithCoarseMeshBuildDirectory = CommonPath + "/shape_with_coarse_mesh_builded"
ShapeLightmapBuildDirectory = CommonPath + "/shape_lightmap"
ShapeLightmap16BitsBuildDirectory = CommonPath + "/shape_lightmap_16_bits"

# Farbank directories
FarbankBuildDirectory = CommonPath + "/farbank"

# Ig directories ************** TODO CONFIRM IN IG BUILD PROCESS ************ FIX RBANK IF NEEDED ***********
IgLandBuildDirectory = "_invalid"
IgOtherBuildDirectory = "_invalid"

# Rbank directories
RbankOutputBuildDirectory = "_invalid"
RbankMeshBuildDirectory = "_invalid"

# Ligo directories


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Map directory
MapClientDirectory = CommonName + "_maps"
BitmapClientDirectory = MapClientDirectory

# Shape directory
ShapeClientDirectory = CommonName + "_shapes"

# Lightmap directory
LightmapClientDirectory = CommonName + "_lightmaps"

# Tile directory
TilesClientDirectory = CommonName + "_tiles"

# Displace directory
DisplaceClientDirectory = CommonName + "_displaces"

# Bank directory
BankClientDirectory = CommonName + "_bank"

# Vegetable set directory
VegetSetClientDirectory = CommonName + "_vegetable_sets"

# Vegetable shape directory
VegetClientDirectory = CommonName + "_vegetables"

# PACS primitives directories
PacsPrimClientDirectory = CommonName + "_pacs_prim"
