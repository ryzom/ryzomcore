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

from buildsite import *

# *** COMMON NAMES AND PATHS ***
CommonName = "data_common"
CommonPath = "common/" + CommonName


# *** DIRECT SOURCE DIRECTORIES ***

# Copy dir directories
CopyDirectSourceDirectories = [ ]
CopyDirectSourceDirectories += [ DataCommonDirectory ]
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


# *** SOURCE DIRECTORIES IN THE EXPORT/BUILD ***

# Pacs prim list source directories
PacsPrimExportSourceDirectories = [ ]
PacsPrimExportSourceDirectories += [ "ecosystems/desert/pacs_prim" ]
PacsPrimExportSourceDirectories += [ "ecosystems/jungle/pacs_prim" ]
PacsPrimExportSourceDirectories += [ "ecosystems/lacustre/pacs_prim" ]
PacsPrimExportSourceDirectories += [ "ecosystems/primes_racines/pacs_prim" ]


# *** INSTALL DIRECTORIES IN THE CLIENT DATA ***

# Common data install directory
CopyInstallDirectory = CommonName
