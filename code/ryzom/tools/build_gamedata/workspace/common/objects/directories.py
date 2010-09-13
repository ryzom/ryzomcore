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


# *** COMMON PATH ***

CommonPath = "common/objects"


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Shape directories
ShapeSourceDirectories = [ ]
ShapeSourceDirectories += [ "stuff/fyros/agents/accessories" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/accessories" ]
ShapeSourceDirectories += [ "stuff/matis/agents/accessories" ]
ShapeSourceDirectories += [ "stuff/zorai/agents/accessories" ]
ShapeSourceDirectories += [ "stuff/generique/agents/accessories" ]
ShapeSourceDirectories += [ "stuff/caravan/agents/accessories" ]
ShapeSourceDirectories += [ "stuff/animated_light" ]

# Maps directories
MapSourceDirectories = [ ]
MapSourceDirectories += [ "stuff/fyros/agents/_textures/accessories" ]
MapSourceDirectories += [ "stuff/tryker/agents/_textures/accessories" ]
MapSourceDirectories += [ "stuff/matis/agents/_textures/accessories" ]
MapSourceDirectories += [ "stuff/zorai/agents/_textures/accessories" ]
MapSourceDirectories += [ "stuff/generique/agents/_textures/accessories" ]
MapSourceDirectories += [ "stuff/caravan/agents/_textures/accessories" ]

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

# Map directory
MapClientDirectory = "objects"

# Map directory
BitmapClientDirectory = MapClientDirectory

# Shape directory
ShapeClientDirectory = "objects"

# Lightmap directory
LightmapClientDirectory = "objects"
