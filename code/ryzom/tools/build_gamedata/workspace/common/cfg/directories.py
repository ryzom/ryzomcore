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
CommonName = "cfg"
CommonPath = "common/" + CommonName


# *** DIRECT SOURCE DIRECTORIES ***

# Copy dir directories
CopyDirectSourceDirectories = [ ]
CopyDirectSourceFiles = [ ]


# *** SOURCE DIRECTORIES IN LEVELDESIGN ***
CopyLeveldesignSourceDirectories = [ ]
CopyLeveldesignSourceFiles = [ ]
CopyLeveldesignWorldSourceDirectories = [ ]
CopyLeveldesignWorldSourceFiles = [ ]
CopyLeveldesignDfnSourceDirectories = [ ]
CopyLeveldesignDfnSourceFiles = [ ]


# *** SOURCE DIRECTORIES IN THE DATABASE ***

# Copy dir directories
CopyDatabaseSourceDirectories = [ ]
CopyDatabaseSourceFiles = [ ]


# *** SPECIAL SOURCES ***

# Copy dir directories
CopyWindowsExeDllCfgSourceFiles = [ ]

CopyWindowsExeDllCfgSourceFiles += [ "client_cpu_1.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_cpu_2.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_cpu_3.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_cpu_4.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_gpu_1.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_gpu_2.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_gpu_3.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_gpu_4.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_ram_256.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_ram_512.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_sound_buffer.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_vram_32.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_vram_64.cfg" ]
CopyWindowsExeDllCfgSourceFiles += [ "client_vram_128.cfg" ]


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Copy
CopyInstallDirectory = CommonName
