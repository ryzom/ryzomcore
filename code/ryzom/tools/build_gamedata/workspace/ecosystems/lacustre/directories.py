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
# Directories configuration for 'lacustre' ecosystem.
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

EcosystemName = "lacustre"
EcosystemPath = "ecosystems/" + EcosystemName
CommonName = EcosystemName
CommonPath = EcosystemPath

DatabaseRootName = "tryker"
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
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/1a-sable-marin" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/1-plages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/2-iles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/2-ilesa" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/2-iles-marines" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/3-fondmarin2plage" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/4-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/5-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/5-parois-marine" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/6-fond_marin" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/7-bassesiles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/7-mousseter" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/7-racines" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/8-mousse_marine" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/constructible" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/1a-sable-marin" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/1-plages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/2-iles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/2-ilesa" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/2-iles-marines" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/3-fondmarin2plage" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/4-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/5-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/5-parois-marine" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/6-fond_marin" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/7-bassesiles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/7-mousseter" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/7-racines" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/8-mousse_marine" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/constructible" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/1a-sable-marin" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/1-plages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/2-iles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/2-ilesa" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/2-iles-marines" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/3-fondmarin2plage" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/4-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/5-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/5-parois-marine" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/6-fond_marin" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/7-bassesiles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/7-mousseter" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/7-racines" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/8-mousse_marine" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/constructible" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/1a-sable-marin" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/1-plages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/2-iles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/2-ilesa" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/2-iles-marines" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/3-fondmarin2plage" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/4-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/5-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/5-parois-marine" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/6-fond_marin" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/7-bassesiles" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/7-mousseter" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/7-racines" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/8-mousse_marine" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/constructible" ]

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
