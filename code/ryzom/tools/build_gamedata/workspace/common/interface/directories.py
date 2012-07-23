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
EcosystemName = "interface"
EcosystemPath = "common/" + EcosystemName
ContinentName = EcosystemName
ContinentPath = EcosystemPath
CommonName = ContinentName
CommonPath = ContinentPath


# *** SOURCE DIRECTORIES IN THE DATABASE ***

#Interface directories
InterfaceSourceDirectories = [ ]
InterfaceSourceDirectories += [ [ "interfaces/v3" ] + [ "interfaces/r2_interface" ] ]
InterfaceSourceDirectories += [ [ "interfaces/v3_outgame/ui" ] ]
InterfaceSourceDirectories += [ [ "interfaces/v3_login" ] ]

InterfaceDxtcSourceDirectories = [ ]
InterfaceDxtcSourceDirectories += [ "interfaces/v3_bricks" ]
InterfaceDxtcSourceDirectories += [ "interfaces/v3_items" ]
InterfaceDxtcSourceDirectories += [ "interfaces/v3_dxtc_misc" ]

InterfaceFullscreenSourceDirectories = [ ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_fullscreen" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_outgame/fullscreen" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_doc/graph/abilities_items" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_doc/graph/buy_sell" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_doc/graph/camera_character" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_doc/graph/create_perso" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_doc/graph/fight" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_doc/graph/MatisTown" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_doc/graph/spell" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_doc/graph/talk_bot" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_doc/graph/abilities_items" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/v3_quick_help/graph" ]
InterfaceFullscreenSourceDirectories += [ "interfaces/r2_decals" ]

Interface3DSourceDirectories = [ ]
Interface3DSourceDirectories += [ "interfaces/v3_outgame/3d" ]
Interface3DSourceDirectories += [ "interfaces/v3_doc/htm" ]
Interface3DSourceDirectories += [ "interfaces/v3_doc" ]
Interface3DSourceDirectories += [ "interfaces/v3_quick_help" ]
Interface3DSourceDirectories += [ "interfaces/r2_3d" ]


# *** EXPORT DIRECTORIES FOR THE BUILD PIPELINE ***

# Interface directories
InterfaceExportDirectory = CommonPath + "/interface_export"
InterfaceDxtcExportDirectory = CommonPath + "/interface_dxtc_export"
InterfaceFullscreenExportDirectory = CommonPath + "/interface_fullscreen_export"
Interface3DExportDirectory = CommonPath + "/interface_3d_export"


# *** BUILD DIRECTORIES FOR THE BUILD PIPELINE ***

# Interface directories
InterfaceBuildDirectory = CommonPath + "/interface_build"
InterfaceDxtcBuildDirectory = CommonPath + "/interface_dxtc_build"


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Lightmap directory
InterfaceInstallDirectory = "interfaces"
