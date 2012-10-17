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
ContinentName = "indoors"
ContinentPath = "continents/" + ContinentName
CommonName = ContinentName
CommonPath = ContinentPath


# *** SOURCE DIRECTORIES LEVELDESIGN/WORLD ***
ContinentLeveldesignWorldDirectory = ContinentName


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Shape directories
ShapeSourceDirectories = [ ]
ShapeSourceDirectories += [ "stuff/fyros/decors/constructions/fy_cn_appart_joueur" ]
ShapeSourceDirectories += [ "stuff/fyros/decors/constructions/fy_cn_hall_conseil" ]
ShapeSourceDirectories += [ "stuff/fyros/decors/constructions/fy_cn_hall_reunion_vitrine" ]
ShapeSourceDirectories += [ "stuff/fyros/decors/constructions/fy_cn_salle_npc" ]
ShapeSourceDirectories += [ "stuff/matis/decors/constructions/appart_joueur" ]
ShapeSourceDirectories += [ "stuff/matis/decors/constructions/hall_du_conseil" ]
ShapeSourceDirectories += [ "stuff/matis/decors/constructions/hall_vitrine_hall_reunion" ]
ShapeSourceDirectories += [ "stuff/matis/decors/constructions/salle_npc" ]
ShapeSourceDirectories += [ "stuff/tryker/decors/constructions/hall_conseil" ]
ShapeSourceDirectories += [ "stuff/tryker/decors/constructions/hall_vitrine_reunion" ]
ShapeSourceDirectories += [ "stuff/tryker/decors/constructions/piece_npc" ]
ShapeSourceDirectories += [ "stuff/tryker/decors/constructions/tr_appart" ]
ShapeSourceDirectories += [ "stuff/zorai/decors/constructions/Appart_joueur" ]
ShapeSourceDirectories += [ "stuff/zorai/decors/constructions/hall_conseil" ]
ShapeSourceDirectories += [ "stuff/zorai/decors/constructions/hall_reunion_vitrine" ]
ShapeSourceDirectories += [ "stuff/zorai/decors/constructions/salle_npc" ]

# Ligo directories
LigoBaseSourceDirectory = "landscape/ligo/" + EcosystemName
LigoMaxSourceDirectory = LigoBaseSourceDirectory + "/max"

# Zone directories
ZoneSourceDirectory = [ "landscape/zones/" + ContinentName ] # For old snowballs style landscape when not using ligo

# Ig directories
IgLandSourceDirectories = [ ]
IgOtherSourceDirectories = [ ]
for dir in ShapeSourceDirectories:
	IgOtherSourceDirectories += [ dir ]
IgPrimitiveSourceDirectories = [ ]

# RBank directories
RBankCmbSourceDirectories = [ ]
for dir in ShapeSourceDirectories:
	RBankCmbSourceDirectories += [ dir ]

# Tiles root directory
TileRootSourceDirectory = "landscape/_texture_tiles/" + EcosystemName

# Displace directory
DisplaceSourceDirectory = "landscape/_texture_tiles/" + EcosystemName + "/displace"

# Ligo primitive directory used in the client
PsSourceDirectories = [ ]
PsSourceDirectories += [ "primitive_microlife/" + ContinentName ]


# *** LOOKUP DIRECTORIES WITHIN THE BUILD PIPELINE *** (TODO: use these instead of search_pathes in properties(_base).cfg)

# Ig lookup directories used by rbank
IgLookupDirectories = [ ]
IgLookupDirectories += [ ContinentPath + "/ig_land" ]
IgLookupDirectories += [ ContinentPath + "/ig_other" ]

# Shape lookup directories used by rbank
ShapeLookupDirectories = [ ]
ShapeLookupDirectories += [ "common/sfx/ps" ]
ShapeLookupDirectories += [ "common/sfx/shape_clodtex_build" ]
ShapeLookupDirectories += [ "common/sfx/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ "common/construction/shape_clodtex_build" ]
ShapeLookupDirectories += [ "common/construction/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ "ecosystems/desert/shape_clodtex_build" ]
ShapeLookupDirectories += [ "ecosystems/desert/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ "ecosystems/jungle/shape_clodtex_build" ]
ShapeLookupDirectories += [ "ecosystems/jungle/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ "ecosystems/lacustre/shape_clodtex_build" ]
ShapeLookupDirectories += [ "ecosystems/lacustre/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ ContinentPath + "/shape_clodtex_build" ]
ShapeLookupDirectories += [ ContinentPath + "/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ "continents/fyros/shape_clodtex_build" ]
ShapeLookupDirectories += [ "continents/fyros/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ "continents/matis/shape_clodtex_build" ]
ShapeLookupDirectories += [ "continents/matis/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ "continents/zorai/shape_clodtex_build" ]
ShapeLookupDirectories += [ "continents/zorai/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ "continents/tryker/shape_clodtex_build" ]
ShapeLookupDirectories += [ "continents/tryker/shape_with_coarse_mesh" ]

# Map lookup directories not yet used
MapLookupDirectories = [ ]
MapLookupDirectories += [ "common/sfx/map_export" ]
MapLookupDirectories += [ "common/sfx/map_uncompressed" ]
MapLookupDirectories += [ "common/construction/map_export" ]
MapLookupDirectories += [ "common/construction/map_uncompressed" ]
MapLookupDirectories += [ "ecosystems/desert/map_export" ]
MapLookupDirectories += [ "ecosystems/desert/map_uncompressed" ]
MapLookupDirectories += [ "ecosystems/jungle/map_export" ]
MapLookupDirectories += [ "ecosystems/jungle/map_uncompressed" ]
MapLookupDirectories += [ "ecosystems/lacustre/map_export" ]
MapLookupDirectories += [ "ecosystems/lacustre/map_uncompressed" ]
MapLookupDirectories += [ ContinentPath + "/map_export" ]
MapLookupDirectories += [ ContinentPath + "/map_uncompressed" ]
MapLookupDirectories += [ "continents/fyros/map_export" ]
MapLookupDirectories += [ "continents/fyros/map_uncompressed" ]
MapLookupDirectories += [ "continents/matis/map_export" ]
MapLookupDirectories += [ "continents/matis/map_uncompressed" ]
MapLookupDirectories += [ "continents/zorai/map_export" ]
MapLookupDirectories += [ "continents/zorai/map_uncompressed" ]
MapLookupDirectories += [ "continents/tryker/map_export" ]
MapLookupDirectories += [ "continents/tryker/map_uncompressed" ]

# PacsPrim lookup directories used by ai_wmap
PacsPrimLookupDirectories = [ ]
PacsPrimLookupDirectories += [ "ecosystems/desert/pacs_prim" ]
PacsPrimLookupDirectories += [ "ecosystems/jungle/pacs_prim" ]
PacsPrimLookupDirectories += [ "ecosystems/lacustre/pacs_prim" ]


# *** EXPORT DIRECTORIES FOR THE BUILD PIPELINE ***

# Shape directories
ShapeTagExportDirectory = CommonPath + "/shape_tag"
ShapeNotOptimizedExportDirectory = CommonPath + "/shape_not_optimized"
ShapeWithCoarseMeshExportDirectory = CommonPath + "/shape_with_coarse_mesh"
ShapeLightmapNotOptimizedExportDirectory = CommonPath + "/shape_lightmap_not_optimized"
ShapeAnimExportDirectory = CommonPath + "/shape_anim"

# Ligo directories
LigoDatabaseExportDirectory = "landscape/ligo/" + EcosystemName
LigoDatabaseIgExportDirectory = LigoDatabaseExportDirectory + "/igs"
LigoDatabaseZoneExportDirectory = LigoDatabaseExportDirectory + "/zones"
LigoDatabaseZoneLigoExportDirectory = LigoDatabaseExportDirectory + "/zoneligos"
LigoDatabaseCmbExportDirectory = LigoDatabaseExportDirectory + "/cmb"
LigoTagExportDirectory = "ecosystems/" + EcosystemName + "/ligo_tag"

# Zone directories
ZoneExportDirectory = ContinentPath + "/zone"
WaterMapSourceDirectories = [ ]

# RBank directories
RBankCmbExportDirectory = CommonPath + "/rbank_cmb_export"
RBankCmbTagExportDirectory = CommonPath + "/rbank_cmb_tag_export"

# Smallbank directories
SmallbankExportDirectory = EcosystemPath + "/smallbank"

# Tiles directories
DisplaceExportDirectory = EcosystemPath + "/diplace"

# Ig directories
IgStaticLandExportDirectory = ContinentPath + "/ig_static_land" # Landscape IG eported from 3dsmax not elevated by the heightmap
IgStaticOtherExportDirectory = ContinentPath + "/ig_static_other" # Village or construction IGs exported from 3dsmax
IgStaticTagExportDirectory = ContinentPath + "/ig_static_tag" # Tag for exported 3dsmax files

# PS directories
PsExportDirectory = CommonPath + "/ps"


# *** BUILD DIRECTORIES FOR THE BUILD PIPELINE ***

# Shape directories
ShapeClodtexBuildDirectory = CommonPath + "/shape_clodtex_build"
ShapeWithCoarseMeshBuildDirectory = CommonPath + "/shape_with_coarse_mesh_builded"
ShapeLightmapBuildDirectory = CommonPath + "/shape_lightmap"
ShapeLightmap16BitsBuildDirectory = CommonPath + "/shape_lightmap_16_bits"

# Ligo directories
LigoZoneBuildDirectory = CommonPath + "/ligo_zones"
LigoIgLandBuildDirectory = CommonPath + "/ligo_ig_land" # Landscape IG found in ligo bricks not elevated by the heightmap
LigoIgOtherBuildDirectory = CommonPath + "/ligo_ig_other" # Village or construction IGs exported from ligo landscape

# Zone directories
ZoneWeldBuildDirectory = CommonPath + "/zone_weld"
ZoneDependBuildDirectory = CommonPath + "/zone_depend"
ZoneLightWaterShapesLightedExportDirectory = CommonPath + "/zone_lwsl_temp" #fixme
ZoneLightBuildDirectory = CommonPath + "/zone_lighted" #fixme
ZoneLightDependBuildDirectory = CommonPath + "/zone_lighted_depend" #fixme
ZoneLightIgLandBuildDirectory = CommonPath + "/zone_lighted_ig_land" #fixme

# Farbank directories
FarbankBuildDirectory = EcosystemPath + "/farbank"

# Ig directories 
IgElevLandPrimBuildDirectory = CommonPath + "/ig_elev_land_prim" # landscape IG generated by the prim exporter (already elevated by the land exporter)
IgElevLandLigoBuildDirectory = CommonPath + "/ig_elev_land_ligo" # Landscape IG found in ligo bricks from 3dsmax elevated by the heightmap
IgElevLandStaticBuildDirectory = CommonPath + "/ig_elev_land_static" # Landscape IG eported from 3dsmax elevated by the heightmap
IgTempLandMergeBuildDirectory = CommonPath + "/ig_temp_land_merge"
IgTempLandCompareBuildDirectory = CommonPath + "/ig_temp_land_compare" # Tmp final IG directory for landscape IGs before comparison
IgLandBuildDirectory = CommonPath + "/ig_land" # Final IG directory for landscape IGs
IgOtherBuildDirectory = CommonPath + "/ig_other" # Final IG directory for village or construction IGs
IgOtherLightedBuildDirectory = CommonPath + "/ig_other_lighted"

# Rbank directories
RbankBboxBuildDirectory = CommonPath + "/rbank_bbox"
RbankTessellationBuildDirectory = CommonPath + "/rbank_tessellation"
RbankSmoothBuildDirectory = CommonPath + "/rbank_smooth"
RbankRawBuildDirectory = CommonPath + "/rbank_raw"
RbankPreprocBuildDirectory = CommonPath + "/rbank_preproc"
RbankRetrieversBuildDirectory = CommonPath + "/rbank_retrievers"
RbankOutputBuildDirectory = CommonPath + "/rbank_output"

# AI Wmap directories
AiWmapBuildDirectory = CommonPath + "/ai_wmap"
AiWmapBuildTagDirectory = CommonPath + "/ai_wmap_tag"


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Shape directory
ShapeInstallDirectory = CommonName + "_shapes"

# Shape lightmaps directory
LightmapInstallDirectory = CommonName + "_lightmaps"

# Ig directory
IgInstallDirectory = CommonName + "_ig"

# PACS directory
PacsInstallDirectory = CommonName + "_pacs"

# PS directory
PsInstallDirectory = CommonName + "_ig"

# AI Wmap directory
AiWmapInstallDirectory = CommonName + "_ai"
