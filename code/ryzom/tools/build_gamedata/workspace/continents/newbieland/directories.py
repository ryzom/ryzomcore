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

# *** SOURCE DIRECTORIES IN THE DATABASE ***


# *** ECOSYSTEM AND CONTINENT NAMES ***
EcosystemName = "jungle"
EcosystemPath = "ecosystems/" + EcosystemName
ContinentName = "newbieland"
ContinentPath = "continents/" + ContinentName


# Ligo directories
LigoBaseSourceDirectory = "landscape/ligo/" + EcosystemName
LigoMaxSourceDirectory = LigoBaseSourceDirectory + "/max"

# Zone directories
ZoneSourceDirectory = "landscape/zones/" + EcosystemName


# *** EXPORT DIRECTORIES FOR THE BUILD PIPELINE ***

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

# Smallbank directories
SmallbankExportDirectory = EcosystemPath + "/smallbank"

# Tiles directories
DisplaceExportDirectory = EcosystemPath + "/diplace"


# *** BUILD DIRECTORIES FOR THE BUILD PIPELINE ***

# Ligo directories
LigoZoneBuildDirectory = ContinentPath + "/ligo_zones"
LigoIgLandBuildDirectory = ContinentPath + "/ligo_ig_land"
LigoIgOtherBuildDirectory = ContinentPath + "/ligo_ig_other"

# Zone directories
ZoneWeldBuildDirectory = ContinentPath + "/zone_weld"
ZoneLightWaterShapesLightedExportDirectory = ContinentPath + "/zone_lwsl_temp" #fixme
ZoneLightBuildDirectory = ContinentPath + "/zone_lighted" #fixme
ZoneLightDependBuildDirectory = ContinentPath + "/zone_lighted_depend" #fixme
ZoneLightIgLandBuildDirectory = ContinentPath + "/zone_lighted_ig_land" #fixme

# Farbank directories
FarbankBuildDirectory = EcosystemPath + "/farbank"

# Ig directories ************** TODO CONFIRM IN IG BUILD PROCESS ************ FIX RBANK IF NEEDED ***********
IgLandBuildDirectory = "_invalid"
IgVillageBuildDirectory = "_invalid"

# Rbank directories
RbankBboxBuildDirectory = ContinentPath + "/rbank_bbox"
RbankTessellationBuildDirectory = ContinentPath + "/rbank_tessellation"
RbankSmoothBuildDirectory = ContinentPath + "/rbank_smooth"
RbankRawBuildDirectory = ContinentPath + "/rbank_raw"
RbankPreprocBuildDirectory = ContinentPath + "/rbank_preproc"
RbankMeshBuildDirectory = ContinentPath + "/rbank_cmb"
RbankRetrieversBuildDirectory = ContinentPath + "/rbank_retrievers"
RbankOutputBuildDirectory = ContinentPath + "/rbank_output"


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Ig directory
IgClientDirectory = ContinentName + "_ig"

# Zone directory
ZoneClientDirectory = ContinentName + "_zones"
WaterMapsClientDirectory = ContinentName + "_zones"

# PACS directory
PacsClientDirectory = ContinentName + "_pacs"

# PS directory
IgClientDirectory = ContinentName + "_ig"
