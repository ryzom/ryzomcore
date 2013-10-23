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
EcosystemName = "fauna"
EcosystemPath = "common/" + EcosystemName
ContinentName = EcosystemName
ContinentPath = EcosystemPath
CommonName = ContinentName
CommonPath = ContinentPath


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Skeleton directories
SkelSourceDirectories = [ ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/aquatique/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/aquatique_monture/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/cheval/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/chien/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/coureur/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/crustace/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/familier/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/grand_ryzomien/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kamiforet/animations/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kamiguard/animations/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kitin_volant/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/oiseau/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/pachyderme/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/ryzomien/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/carnitree/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/electroalg/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/endrobouchea/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/phytopsy/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/sapenslaver/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/swarmplant/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_guide_2/animations/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_guide_3/animations/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_guide_4/animations/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_preacher_2/animations/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_preacher_3/animations/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_preacher_4/animations/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/homins_degeneres/cute/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/homins_degeneres/frahar/animation/skeletons" ]
SkelSourceDirectories += [ "stuff/tryker/agents/monsters/homins_degeneres/gibbai/animation/skeletons" ]

# Skeleton template weight directories
SwtSourceDirectories = [ ]

# Shape directories
ShapeSourceDirectories = [ ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/aquatique/clapclap" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/aquatique/ryzetacee" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/aquatique_monture/sagass_selle" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/cheval/mektoub" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/cheval/mektoubselle" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/cheval/mektoubpack" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/chien/chorani" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/chien/jungler" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/chien/regus" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/chien/varinx" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/coureur/capryni" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/coureur/filin" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/crustace/cococlaw" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/crustace/estrasson" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/crustace/hachtaha" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/crustace/diranak" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/familier/dag" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/grand_ryzomien/ryzerb" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/grand_ryzomien/ryzoholok" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kamiforet" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kamiguard" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_guide_2" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_guide_3" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_guide_4" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_preacher_2" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_preacher_3" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_preacher_4" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/kitihank" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/kitinagan" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/kitinarak" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/kitinega" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/kitinokto" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/kitimandib" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/pucetron" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin_volant/kitifly" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin_volant/kitikil" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/oiseau/kazoar" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/oiseau/lightbird" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/oiseau/yber" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/pachyderme/arma" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/pachyderme/bul" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/pachyderme/vampignon" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/ryzomien/kakty" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/ryzomien/ryzoholo" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/ryzomien/zerx" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/carnitree" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/electroalg" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/endrobouchea" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/phytopsy" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/sapenslaver" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/swarmplant" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/homins_degeneres/cute" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/homins_degeneres/frahar" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/homins_degeneres/gibbai" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/aquatique" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/chiens" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/coureur" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/crustaces" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/grand_ryzomien" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/kitin" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/kitin_volant" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/oiseau" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/pachyderme/vampignon" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/ryzomien/kakty" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/ryzomien/ryzoholo" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/cheval/c03" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/cheval/h05" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/cheval/h12" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/chien/c02" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/chien/c07" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/coureur/h01" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/coureur/h04" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/crustace/c05" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/grand_ryzomien/c06" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/grand_ryzomien/h07" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/grand_ryzomien/h11" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/oiseau/c01" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/pachyderme/h08" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/pachyderme/h10" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/ryzomien/c04" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/ryzomien/h02" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/ryzomien/h06" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/ryzomien/h09" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/familier/h03" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/ryzomien/c04" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/familier/h03" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/homins_degeneres/cute" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/homins_degeneres/frahar" ]
ShapeSourceDirectories += [ "stuff/goo/agents/monsters/homins_degeneres/gibbai" ]
ShapeSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/kitin_queen" ]

# Maps directories
MapSourceDirectories = [ ]
MapSourceDirectories += [ "stuff/fyros/agents/_textures/monster" ]
MapSourceDirectories += [ "stuff/tryker/agents/_textures/monster" ]
MapSourceDirectories += [ "stuff/jungle/agents/_textures/monster" ]
MapSourceDirectories += [ "stuff/primes_racines/agents/_textures/monster" ]
MapSourceDirectories += [ "stuff/goo/agents/_textures/monster" ]

MapUncompressedSourceDirectories = [ ]

# Animation directories
AnimSourceDirectories = [ ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/aquatique/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/aquatique_monture/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/cheval/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/chien/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/coureur/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/crustace/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/familier/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/grand_ryzomien/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kamiforet/animations/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kamiguard/animations/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kitin/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kitin_volant/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/oiseau/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/pachyderme/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/ryzomien/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/carnitree/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/electroalg/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/endrobouchea/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/phytopsy/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/sapenslaver/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/plante_carnivore/swarmplant/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_guide_2/animations/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_guide_3/animations/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_guide_4/animations/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_preacher_2/animations/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_preacher_3/animations/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/kami/kami_preacher_4/animations/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/homins_degeneres/cute/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/homins_degeneres/frahar/animation/anims" ]
AnimSourceDirectories += [ "stuff/tryker/agents/monsters/homins_degeneres/gibbai/animation/anims" ]

# cLoD shape directories
ClodSourceDirectories = [ ]
ClodSourceDirectories += [ "stuff/lod_actors/lod_fauna" ]


# *** LOOKUP DIRECTORIES WITHIN THE BUILD PIPELINE *** (TODO: use these instead of search_pathes in properties(_base).cfg)

# Ig lookup directories used by rbank
IgLookupDirectories = [ ]

# Shape lookup directories used by rbank
ShapeLookupDirectories = [ ]
# ShapeLookupDirectories += [ CommonPath + "/ps" ]
ShapeLookupDirectories += [ CommonPath + "/shape_clodtex_build" ]
ShapeLookupDirectories += [ CommonPath + "/shape_with_coarse_mesh" ]

# Map lookup directories not yet used
MapLookupDirectories = [ ]
MapLookupDirectories += [ CommonPath + "/map_export" ]
MapLookupDirectories += [ CommonPath + "/map_uncompressed" ]


# *** EXPORT DIRECTORIES FOR THE BUILD PIPELINE ***

# Map directories
MapExportDirectory = CommonPath + "/map_export"
MapUncompressedExportDirectory = CommonPath + "/map_uncompressed"

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

# Map directory
MapInstallDirectory = CommonName + "_maps"
BitmapInstallDirectory = MapInstallDirectory

# Shape directory
ShapeInstallDirectory = CommonName + "_shapes"
LightmapInstallDirectory = ShapeInstallDirectory

# Animation directory
AnimInstallDirectory = CommonName + "_animations"

# Skeleton directory
SkelInstallDirectory = CommonName + "_skeletons"

# Skeleton directory
SwtInstallDirectory = CommonName + "_swt"
