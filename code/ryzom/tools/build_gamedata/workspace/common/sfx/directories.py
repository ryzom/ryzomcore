#!/usr/bin/python
# 
# \file directories.py
# \brief Directories configuration
# \date 2010-08-27 17:13GMT
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


# *** COMMON NAMES AND PATHS ***
EcosystemName = "sfx"
EcosystemPath = "common/" + EcosystemName
ContinentName = EcosystemName
ContinentPath = EcosystemPath
CommonName = ContinentName
CommonPath = ContinentPath


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# PS directories
PsSourceDirectories = [ ]
PsSourceDirectories += [ "sfx/buildings" ]
PsSourceDirectories += [ "sfx/environment" ]
PsSourceDirectories += [ "sfx/fighting" ]
PsSourceDirectories += [ "sfx/magic" ]
PsSourceDirectories += [ "sfx/moving" ]
PsSourceDirectories += [ "sfx/teaser" ]
PsSourceDirectories += [ "sfx/forage" ]
PsSourceDirectories += [ "sfx/monsters" ]

# Maps directories
MapSourceDirectories = [ ]
MapSourceDirectories += [ "sfx/maps" ]

# Shape directories
ShapeSourceDirectories = [ ]
ShapeSourceDirectories += [ "sfx/meshtoparticle" ]

MapUncompressedSourceDirectories = [ ]


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

# PS directories
PsExportDirectory = CommonPath + "/ps"


# *** BUILD DIRECTORIES FOR THE BUILD PIPELINE ***

# Map directories
MapBuildDirectory = CommonPath + "/map"
MapPanoplyBuildDirectory = CommonPath + "/map_panoply"

# Shape directories
ShapeClodtexBuildDirectory = CommonPath + "/shape_clodtex_build"
ShapeWithCoarseMeshBuildDirectory = CommonPath + "/shape_with_coarse_mesh_builded"
ShapeLightmapBuildDirectory = CommonPath + "/shape_lightmap"
ShapeLightmap16BitsBuildDirectory = CommonPath + "/shape_lightmap_16_bits"


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Particule system directory
PsClientDirectory = "sfx"

# Map directory
MapClientDirectory = "sfx"
BitmapClientDirectory = MapClientDirectory

# Shape directory
ShapeClientDirectory = "sfx"

# Lightmap directory
LightmapClientDirectory = "sfx"
