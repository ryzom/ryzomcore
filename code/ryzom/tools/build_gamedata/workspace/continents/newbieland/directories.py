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

EcosystemName = "jungle"
EcosystemPath = "ecosystems/" + EcosystemName
ContinentName = "newbieland"
ContinentPath = "continents/" + ContinentName
CommonName = ContinentName
CommonPath = ContinentPath


# *** SOURCE DIRECTORIES LEVELDESIGN/WORLD ***
ContinentLeveldesignWorldDirectory = ContinentName


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Shape directories
ShapeSourceDirectories = [ ]
ShapeSourceDirectories += [ "stuff/" + ContinentName + "/sky" ]

# Maps directories
MapSourceDirectories = [ ]
MapSourceDirectories += [ "stuff/" + ContinentName + "/sky" ]

MapUncompressedSourceDirectories = [ ]

# Ligo directories
LigoBaseSourceDirectory = "landscape/ligo/" + EcosystemName
LigoMaxSourceDirectory = LigoBaseSourceDirectory + "/max"

# Zone directories
ZoneSourceDirectory = [ "landscape/zones/" + ContinentName ] # For old snowballs style landscape when not using ligo

# RBank directories
RBankCmbSourceDirectories = [ ]

# Ig directories
IgLandSourceDirectories = [ ]
# IgLandSourceDirectories += [ "landscape/zones/" + ContinentName ] # For old snowballs style landscape when not using ligo
IgOtherSourceDirectories = [ ]
IgOtherSourceDirectories += [ "stuff/" + ContinentName + "/sky" ] # The canopee in the sky
IgPrimitiveSourceDirectories = [ ]
IgPrimitiveSourceDirectories += [ "primitive/" + ContinentName ] # Contains plants (trees, etc) primitive made with world editor

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
ShapeLookupDirectories += [ EcosystemPath + "/shape_clodtex_build" ]
ShapeLookupDirectories += [ EcosystemPath + "/shape_with_coarse_mesh" ]
ShapeLookupDirectories += [ ContinentPath + "/shape_clodtex_build" ]
ShapeLookupDirectories += [ ContinentPath + "/shape_with_coarse_mesh" ]
# ShapeLookupDirectories += [ ContinentName + "/zone_light/water_shapes_lighted" ] huh?

# Map lookup directories not yet used
MapLookupDirectories = [ ]
ShapeLookupDirectories += [ "common/sfx/map_export" ]
ShapeLookupDirectories += [ "common/sfx/map_uncompressed" ]
ShapeLookupDirectories += [ "common/construction/map_export" ]
ShapeLookupDirectories += [ "common/construction/map_uncompressed" ]
ShapeLookupDirectories += [ EcosystemPath + "/map_export" ]
ShapeLookupDirectories += [ EcosystemPath + "/map_uncompressed" ]
ShapeLookupDirectories += [ ContinentPath + "/map_export" ]
ShapeLookupDirectories += [ ContinentPath + "/map_uncompressed" ]



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

# Map directories
MapBuildDirectory = CommonPath + "/map"
MapPanoplyBuildDirectory = CommonPath + "/map_panoply"

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


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Map directory
MapClientDirectory = CommonName + "_maps"
BitmapClientDirectory = MapClientDirectory

# Shape directory
ShapeClientDirectory = CommonName + "_shapes"

# Shape lightmaps directory
LightmapClientDirectory = ShapeClientDirectory

# Ig directory
IgClientDirectory = CommonName + "_ig"

# Zone directory
ZoneClientDirectory = CommonName + "_zones"
WaterMapsClientDirectory = ZoneClientDirectory

# PACS directory
PacsClientDirectory = CommonName + "_pacs"

# PS directory
PsClientDirectory = CommonName + "_ig"
