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


# *** ECOSYSTEM AND CONTINENT NAMES ***

EcosystemName = "desert"
EcosystemPath = "ecosystems/" + EcosystemName
ContinentName = "fyros"
ContinentPath = "continents/" + ContinentName
CommonName = ContinentName
CommonPath = ContinentPath


# *** SOURCE DIRECTORIES LEVELDESIGN/WORLD ***
ContinentLeveldesignWorldDirectory = ContinentName


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Shape directories
ShapeSourceDirectories = [ ]
ShapeSourceDirectories += [ "stuff/fyros/decors/constructions" ]
ShapeSourceDirectories += [ "stuff/fyros/city" ]
ShapeSourceDirectories += [ "stuff/fyros/sky" ]
ShapeSourceDirectories += [ "landscape/water/meshes/desert" ]
ShapeSourceDirectories += [ "stuff/fyros/decors/constructions/fy_cn_mairie" ]

# Maps directories
MapSourceDirectories = [ ]
MapSourceDirectories += [ "stuff/fyros/decors/_textures/batiments" ]
MapSourceDirectories += [ "stuff/fyros/city/_textures" ]
MapSourceDirectories += [ "stuff/fyros/sky" ]
MapSourceDirectories += [ "landscape/water/meshes/desert" ]
MapUncompressedSourceDirectories = [ ]


# *** LOOKUP DIRECTORIES WITHIN THE BUILD PIPELINE *** (TODO: use these instead of search_pathes in properties(_base).cfg)

# Shape lookup directories used by rbank
ShapeLookupDirectories = [ ]
ShapeLookupDirectories += [ "common/sfx/ps" ]
ShapeLookupDirectories += [ "common/sfx/shape_clodtex_build" ]
ShapeLookupDirectories += [ "common/sfx/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ "common/construction/shape_clodtex_build" ]
ShapeLookupDirectories += [ "common/construction/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ EcosystemPath + "/shape_clodtex_build" ]
ShapeLookupDirectories += [ EcosystemPath + "/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ ContinentPath + "/shape_clodtex_build" ]
ShapeLookupDirectories += [ ContinentPath + "/shape_with_coarse_mesh" ]
# ShapeLookupDirectories += [ ContinentName + "/zone_light/water_shapes_lighted" ] huh?

# Map lookup directories used by shape
MapLookupDirectories = [ ]
MapLookupDirectories += [ "common/sfx/map_export" ]
MapLookupDirectories += [ "common/sfx/map_uncompressed" ]
MapLookupDirectories += [ "common/construction/map_export" ]
MapLookupDirectories += [ "common/construction/map_uncompressed" ]
MapLookupDirectories += [ EcosystemPath + "/map_export" ]
MapLookupDirectories += [ EcosystemPath + "/map_uncompressed" ]
MapLookupDirectories += [ ContinentPath + "/map_export" ]
MapLookupDirectories += [ ContinentPath + "/map_uncompressed" ]


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


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Map directory
MapInstallDirectory = CommonName + "_maps"
BitmapInstallDirectory = MapInstallDirectory

# Shape directory
ShapeInstallDirectory = CommonName + "_shapes"

# Shape lightmaps directory
LightmapInstallDirectory = ShapeInstallDirectory
