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
CommonName = "leveldesign"
CommonPath = "common/" + CommonName


# *** DIRECT SOURCE DIRECTORIES ***

# Copy dir directories
CopyDirectSourceDirectories = [ ]
CopyDirectSourceFiles = [ ]


# *** SOURCE DIRECTORIES IN LEVELDESIGN ***
CopyLeveldesignSourceDirectories = [ ]
CopyLeveldesignSourceFiles = [ ]
CopyLeveldesignSourceFiles += [ "game_element/anim/mode2animset.string_array" ]
CopyLeveldesignSourceFiles += [ "game_elem/sheet_id.bin" ]
CopyLeveldesignWorldSourceDirectories = [ ]
CopyLeveldesignWorldSourceFiles = [ ]
CopyLeveldesignWorldSourceFiles += [ "static_fame.txt" ]
CopyLeveldesignDfnSourceDirectories = [ ]
CopyLeveldesignDfnSourceFiles = [ ]
CopyLeveldesignDfnSourceFiles += [ "game_elem/_creature/_creature_3d_eyes_color.typ" ]
CopyLeveldesignDfnSourceFiles += [ "game_elem/_creature/_creature_3d_hair_color.typ" ]
CopyLeveldesignDfnSourceFiles += [ "basics/string.typ" ]
CopyLeveldesignDfnSourceFiles += [ "game_elem/_anim/string_array.dfn" ]


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Copy dir directories
CopyDatabaseSourceDirectories = [ ]
CopyDatabaseSourceFiles = [ ]


# *** BUILD DIRECTORIES ***

# Sheets
SheetsBuildDirectory = CommonPath + "/sheets"


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Stuff
CopyInstallDirectory = CommonName

# Sheets
SheetsInstallDirectory = "packedsheets"
