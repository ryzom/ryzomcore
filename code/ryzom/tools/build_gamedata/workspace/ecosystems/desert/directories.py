#!/usr/bin/python
# 
# #################################################################
# ## WARNING : this is a generated file, don't change it !
# #################################################################
# 
# \file directories.py
# \brief Directories configuration
# \date 2010-09-19-14-19-GMT
# \author Jan Boon (Kaetemi)
# \date 2001-2005
# \author Nevrax
# Python port of game data build pipeline.
# Directories configuration for 'desert' ecosystem.
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

EcosystemName = "desert"
EcosystemPath = "ecosystems/" + EcosystemName
CommonName = EcosystemName
CommonPath = EcosystemPath

DatabaseRootName = "fyros"
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
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/1.5-marecage_profond" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/1-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/2-citees" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/3-fond_canyon" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/4.2-boisbandeclair" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/4.5-desert2boisbande" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/4-falaise_bois_bande" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/5-falaise_normales" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/6.5-desertalternatif" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/6-desert" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/7-routes" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_sp/8-foretbrule" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/1.5-marecage_profond" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/1-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/2-citees" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/3-fond_canyon" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/4.2-boisbandeclair" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/4.5-desert2boisbande" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/4-falaise_bois_bande" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/5-falaise_normales" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/6.5-desertalternatif" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/6-desert" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/7-routes" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_su/8-foretbrule" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/1.5-marecage_profond" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/1-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/2-citees" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/3-fond_canyon" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/4.2-boisbandeclair" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/4.5-desert2boisbande" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/4-falaise_bois_bande" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/5-falaise_normales" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/6.5-desertalternatif" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/6-desert" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/7-routes" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_au/8-foretbrule" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/1.5-marecage_profond" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/1-marecages" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/2-citees" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/3-fond_canyon" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/4.2-boisbandeclair" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/4.5-desert2boisbande" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/4-falaise_bois_bande" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/5-falaise_normales" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/6.5-desertalternatif" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/6-desert" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/7-routes" ]
TilesSourceDirectories += [ "landscape/_texture_tiles/" + EcosystemName + "_wi/8-foretbrule" ]

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
