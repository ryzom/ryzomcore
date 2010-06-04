#!/usr/bin/python
# 
# \file site.py
# \brief Site configuration
# \date 2010-06-04-21-25-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Site configuration.
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


# *** SITE INSTALLATION ***

# Use '/' in path name, not ''
# Don't put '/' at the end of a directory name


# Quality option for this site (1 for BEST, 0 for DRAFT)
BuildQuality = 1

ToolDirectories = ['R:/code/nel', 'R:/code/ryzom/tools']
ToolSuffix = "_r.exe"

# Build script directory
ScriptDirectory = "W:/build_gamedata"
WorkspaceDirectory = "R:/code/ryzom/tools/build_gamedata/workspace"

# Data build directories
DatabaseDirectory = "W:/database"
ExportBuildDirectory = "W:/export"

# Client data install directory (client/data)
ClientDataDirectory = "S:/ryzom_client_open/user"

# TODO: NETWORK RECONNECT NOT IMPLEMENTED :)

# Leveldesign directories
LeveldesignDirectory = "L:/leveldesign"
LeveldesignDfnDirectory = "L:/leveldesign/dfn"
LeveldesignWorldDirectory = "L:/leveldesign/world"

# 3dsMax directives
MaxAvailable = 1
MaxDirectory = "C:/Program Files (x86)/Autodesk/3ds Max 2010"
MaxUserDirectory = "C:/Users/Kaetemi/AppData/Local/Autodesk/3dsMax/2010 - 32bit/enu"
MaxExecutable = "3dsmax.exe"


# end of file
