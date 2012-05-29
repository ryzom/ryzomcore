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
EcosystemName = "characters"
EcosystemPath = "common/" + EcosystemName
ContinentName = EcosystemName
ContinentPath = EcosystemPath
CommonName = ContinentName
CommonPath = ContinentPath


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Skeleton directories
SkelSourceDirectories = [ ]
SkelSourceDirectories += [ "stuff/fyros/agents/actors/male/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/fyros/agents/actors/female/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/actors/male/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/actors/female/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/zorai/agents/actors/male/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/zorai/agents/actors/female/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/matis/agents/actors/male/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/matis/agents/actors/female/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/matis/agents/actors/roadsign/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/caravan/agents/actors/male/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/caravan/agents/actors/female/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/caravan/agents/actors/ship/animation/skeletons" ]

# Skeleton template weight directories
SwtSourceDirectories = [ ]
SwtSourceDirectories += [ "stuff/fyros/agents/actors/male/animation/swt" ]
SwtSourceDirectories += [ "stuff/fyros/agents/actors/female/animation/swt" ]
SwtSourceDirectories += [ "stuff/tryker/agents/actors/male/animation/swt" ]
SwtSourceDirectories += [ "stuff/tryker/agents/actors/female/animation/swt" ]
SwtSourceDirectories += [ "stuff/matis/agents/actors/male/animation/swt" ]
SwtSourceDirectories += [ "stuff/matis/agents/actors/female/animation/swt" ]
SwtSourceDirectories += [ "stuff/zorai/agents/actors/male/animation/swt" ]
SwtSourceDirectories += [ "stuff/zorai/agents/actors/female/animation/swt" ]
SwtSourceDirectories += [ "stuff/caravan/agents/actors/male/animation/swt" ]
SwtSourceDirectories += [ "stuff/caravan/agents/actors/female/animation/swt" ]

# Shape directories
ShapeSourceDirectories = [ ]
ShapeSourceDirectories += [ "stuff/fyros/agents/actors/male" ]
ShapeSourceDirectories += [ "stuff/fyros/agents/actors/female" ]
ShapeSourceDirectories += [ "stuff/fyros/agents/actors/visages" ]
ShapeSourceDirectories += [ "stuff/fyros/agents/actors/bots" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/actors/male" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/actors/female" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/actors/visages" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/actors/bots" ]
ShapeSourceDirectories += [ "stuff/matis/agents/actors/male" ]
ShapeSourceDirectories += [ "stuff/matis/agents/actors/female" ]
ShapeSourceDirectories += [ "stuff/matis/agents/actors/visages" ]
ShapeSourceDirectories += [ "stuff/matis/agents/actors/bots" ]
ShapeSourceDirectories += [ "stuff/matis/agents/actors/roadsign" ]
ShapeSourceDirectories += [ "stuff/zorai/agents/actors/male" ]
ShapeSourceDirectories += [ "stuff/zorai/agents/actors/female" ]
ShapeSourceDirectories += [ "stuff/zorai/agents/actors/visages" ]
ShapeSourceDirectories += [ "stuff/zorai/agents/actors/bots" ]
ShapeSourceDirectories += [ "stuff/caravan/agents/actors/female" ]
ShapeSourceDirectories += [ "stuff/caravan/agents/actors/male" ]
ShapeSourceDirectories += [ "stuff/caravan/agents/actors/visages" ]
ShapeSourceDirectories += [ "stuff/caravan/agents/actors/ship" ]
ShapeSourceDirectories += [ "stuff/generique/agents/actors/female" ]
ShapeSourceDirectories += [ "stuff/generique/agents/actors/male" ]
ShapeSourceDirectories += [ "stuff/generique/agents/actors/visages" ]

# Animation directories
AnimSourceDirectories = [ ]
AnimSourceDirectories += [ "stuff/fyros/agents/actors/male/animation/anims" ]
AnimSourceDirectories += [ "stuff/fyros/agents/actors/female/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/actors/male/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/actors/female/animation/anims" ]
AnimSourceDirectories += [ "stuff/matis/agents/actors/male/animation/anims" ]
AnimSourceDirectories += [ "stuff/matis/agents/actors/female/animation/anims" ]
AnimSourceDirectories += [ "stuff/matis/agents/actors/roadsign/animation/anims" ]
AnimSourceDirectories += [ "stuff/zorai/agents/actors/male/animation/anims" ]
AnimSourceDirectories += [ "stuff/zorai/agents/actors/female/animation/anims" ]
AnimSourceDirectories += [ "stuff/caravan/agents/actors/male/animation/anims" ]
AnimSourceDirectories += [ "stuff/caravan/agents/actors/female/animation/anims" ]
AnimSourceDirectories += [ "stuff/caravan/agents/actors/ship/animation/anims" ]

# cLoD shape directories
ClodSourceDirectories = [ ]
ClodSourceDirectories += [ "stuff/lod_actors/lod_" + CommonName ]


# *** LOOKUP DIRECTORIES WITHIN THE BUILD PIPELINE *** (TODO: use these instead of search_pathes in properties(_base).cfg)

# Ig lookup directories used by rbank
IgLookupDirectories = [ ]

# Shape lookup directories used by rbank
ShapeLookupDirectories = [ ]
ShapeLookupDirectories += [ CommonPath + "/shape_clodtex_build" ]
ShapeLookupDirectories += [ CommonPath + "/shape_with_coarse_mesh" ]

# Map lookup directories used by shape
MapLookupDirectories = [ ]


# *** EXPORT DIRECTORIES FOR THE BUILD PIPELINE ***

# Skeleton directories
SkelExportDirectory = CommonPath + "/skel"

# Skeleton template weight directories
SwtExportDirectory = CommonPath + "/swt"

# Shape directories
ShapeTagExportDirectory = CommonPath + "/shape_tag"
ShapeNotOptimizedExportDirectory = CommonPath + "/shape_not_optimized"
ShapeWithCoarseMeshExportDirectory = CommonPath + "/shape_with_coarse_mesh"
ShapeLightmapNotOptimizedExportDirectory = CommonPath + "/shape_lightmap_not_optimized"
ShapeAnimExportDirectory = CommonPath + "/shape_anim"

# Animation directories
AnimExportDirectory = CommonPath + "/anim_export"
AnimTagExportDirectory = CommonPath + "/anim_tag"

# cLoD directories
ClodExportDirectory = CommonPath + "/clod_export"
ClodTagExportDirectory = CommonPath + "/clod_tag_export"


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

# Animation directories
AnimBuildDirectory = CommonPath + "/anim"

# cLoD directories
ClodBankBuildDirectory = CommonPath + "/clod_bank"


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Shape directory
ShapeInstallDirectory = CommonName + "_shapes"
LightmapInstallDirectory = ShapeInstallDirectory

# Animation directory
AnimInstallDirectory = CommonName + "_animations"

# Skeleton directory
SkelInstallDirectory = CommonName + "_skeletons"

# Skeleton directory
SwtInstallDirectory = CommonName + "_swt"
