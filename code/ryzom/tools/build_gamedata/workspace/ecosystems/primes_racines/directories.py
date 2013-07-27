#!/usr/bin/python
# 
# #################################################################
# ## WARNING : this is a generated file, don't change it !
# #################################################################
# 
# \file directories.py
# \brief Directories configuration
# \date 2013-07-27-02-01-GMT
# \author Jan Boon (Kaetemi)
# \date 2001-2005
# \author Nevrax
# Python port of game data build pipeline.
# Directories configuration for 'primes_racines' ecosystem.
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

EcosystemName = "primes_racines"
EcosystemPath = "ecosystems/" + EcosystemName
CommonName = EcosystemName
CommonPath = EcosystemPath

DatabaseRootName = "primes_racines"
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
MapSourceDirectories += [ DatabaseRootPath + "/decors/_textures/batiments" ]
MapSourceDirectories += [ "landscape/microveget/" + EcosystemName + "" ]
MapSourceDirectories += [ "landscape/water/meshes/" + EcosystemName + "" ]

MapUncompressedSourceDirectories = [ ]

# Tiles directories
TilesSourceDirectories = [ ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-creux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-dome-moussu" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-kitiniere" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-mousse-licken" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-mousse-spongieus" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-parois" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-sol-mousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-souche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-stalagmite" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/PR-terre" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/aditif" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-creux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-dome-moussu" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-kitiniere" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-mousse-licken" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-mousse-spongieus" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-parois" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-sol-mousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-souche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-stalagmite" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/PR-terre" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/aditif" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-creux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-dome-moussu" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-kitiniere" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-mousse-licken" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-mousse-spongieus" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-parois" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-sol-mousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-souche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-stalagmite" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/PR-terre" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/aditif" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-creux" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-dome-moussu" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-kitiniere" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-mousse-licken" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-mousse-spongieus" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-parois" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-sol-mousse" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-souche" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-stalagmite" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/PR-terre" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/aditif" ]

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


# *** LOOKUP DIRECTORIES WITHIN THE BUILD PIPELINE *** (TODO: use these instead of search_pathes in properties(_base).cfg)

# Ig lookup directories used by rbank
IgLookupDirectories = [ ]

# Shape lookup directories used by rbank
ShapeLookupDirectories = [ ]
ShapeLookupDirectories += [ EcosystemPath + "/shape_clodtex_build" ]
ShapeLookupDirectories += [ EcosystemPath + "/shape_with_coarse_mesh" ]

# Map lookup directories not yet used
MapLookupDirectories = [ ]
MapLookupDirectories += [ EcosystemPath + "/map_export" ]
MapLookupDirectories += [ EcosystemPath + "/map_uncompressed" ]


# *** EXPORT DIRECTORIES FOR THE BUILD PIPELINE ***

# Map directories
MapExportDirectory = CommonPath + "/map_export"
MapUncompressedExportDirectory = CommonPath + "/map_uncompressed"

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
LigoEcosystemExportDirectory = CommonPath + "/ligo_es"
LigoEcosystemIgExportDirectory = LigoEcosystemExportDirectory + "/igs"
LigoEcosystemZoneExportDirectory = LigoEcosystemExportDirectory + "/zones"
LigoEcosystemZoneLigoExportDirectory = LigoEcosystemExportDirectory + "/zoneligos"
LigoEcosystemCmbExportDirectory = LigoEcosystemExportDirectory + "/cmb"
LigoEcosystemTagExportDirectory = CommonPath + "/ligo_es_tag"

# Zone directories
ZoneExportDirectory = CommonPath + "/zone"

# PACS primitives directories
PacsPrimExportDirectory = CommonPath + "/pacs_prim"
PacsPrimTagExportDirectory = CommonPath + "/pacs_prim_tag"


# *** BUILD DIRECTORIES FOR THE BUILD PIPELINE ***

# Map directories
MapBuildDirectory = CommonPath + "/map"
MapPanoplyBuildDirectory = CommonPath + "/map_panoply"
MapPanoplyHlsInfoBuildDirectory = CommonPath + "/map_panoply_hls_info"
MapPanoplyHlsBankBuildDirectory = CommonPath + "/map_panoply_hls_bank"
MapPanoplyCacheBuildDirectory = CommonPath + "/map_panoply_cache"
MapTagBuildDirectory = CommonPath + "/map_tag"

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

# Ligo directories


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Map directory
MapInstallDirectory = CommonName + "_maps"
BitmapInstallDirectory = MapInstallDirectory

# Shape directory
ShapeInstallDirectory = CommonName + "_shapes"

# Lightmap directory
LightmapInstallDirectory = CommonName + "_lightmaps"

# Tile directory
TilesInstallDirectory = CommonName + "_tiles"

# Displace directory
DisplaceInstallDirectory = CommonName + "_displaces"

# Bank directory
BankInstallDirectory = CommonName + "_bank"

# Vegetable set directory
VegetSetInstallDirectory = CommonName + "_vegetable_sets"

# Vegetable shape directory
VegetInstallDirectory = CommonName + "_vegetables"

# PACS primitives directories
PacsPrimInstallDirectory = CommonName + "_pacs_prim"
