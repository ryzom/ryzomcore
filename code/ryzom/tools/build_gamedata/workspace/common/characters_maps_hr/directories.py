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
EcosystemName = "characters_maps_hr"
EcosystemPath = "common/" + EcosystemName
ContinentName = EcosystemName
ContinentPath = EcosystemPath
CommonName = ContinentName
CommonPath = ContinentPath


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Maps directories
MapSourceDirectories = [ ]
MapSourceDirectories += [ "stuff/caravan/agents/_textures/actors" ]
MapSourceDirectories += [ "stuff/caravan/agents/_textures/actors/visages" ]

MapUncompressedSourceDirectories = [ ]

MapPanoplySourceDirectories = [ ]
MapPanoplySourceDirectories += [ [ "panoply_common.cfg" ] + [ "panoply_matis.cfg" ] + [ "stuff/matis/agents/_textures/actors" ] + [ "stuff/matis/agents/_textures/actors/mask" ] ]
MapPanoplySourceDirectories += [ [ "panoply_common.cfg" ] + [ "panoply_tryker.cfg" ] + [ "stuff/tryker/agents/_textures/actors" ] + [ "stuff/tryker/agents/_textures/actors/mask" ] ]
MapPanoplySourceDirectories += [ [ "panoply_common.cfg" ] + [ "panoply_fyros.cfg" ] + [ "stuff/fyros/agents/_textures/actors" ] + [ "stuff/fyros/agents/_textures/actors/mask" ] ]
MapPanoplySourceDirectories += [ [ "panoply_common.cfg" ] + [ "panoply_zorai.cfg" ] + [ "stuff/zorai/agents/_textures/actors" ] + [ "stuff/zorai/agents/_textures/actors/mask" ] ]
MapPanoplySourceDirectories += [ [ "panoply_common.cfg" ] + [ "panoply_generique.cfg" ] + [ "stuff/generique/agents/_textures/actors" ] + [ "stuff/generique/agents/_textures/actors/mask" ] ]


# *** EXPORT DIRECTORIES FOR THE BUILD PIPELINE ***

# Map directories
MapExportDirectory = CommonPath + "/map_export"
MapUncompressedExportDirectory = CommonPath + "/map_uncompressed"


# *** BUILD DIRECTORIES FOR THE BUILD PIPELINE ***

# Map directories
MapBuildDirectory = CommonPath + "/map"
MapPanoplyBuildDirectory = CommonPath + "/map_panoply"
MapPanoplyHlsInfoBuildDirectory = CommonPath + "/map_panoply_hls_info"
MapPanoplyHlsBankBuildDirectory = CommonPath + "/map_panoply_hls_bank"
MapPanoplyCacheBuildDirectory = CommonPath + "/map_panoply_cache"
MapTagBuildDirectory = CommonPath + "/map_tag"


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Map directory
MapInstallDirectory = CommonName
BitmapInstallDirectory = MapInstallDirectory
