#!/usr/bin/python
# 
# \file 0_setup.py
# \brief Setup rbank
# \date 2009-03-10-22-43-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Setup rbank
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

import time, sys, os, shutil, subprocess, distutils.dir_util
sys.path.append("../../configuration")
sys.path.append("../../configuration/project")
if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite import *
from process import *
from tools import *
from directories import *

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Setup rbank")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Setup source directories
printLog(log, ">>> Setup source directories <<<")
mkPath(log, LeveldesignWorldDirectory)

# Setup export directories
printLog(log, ">>> Setup export directories <<<")
mkPath(log, ExportBuildDirectory + "/" + SmallbankExportDirectory)

# Setup build directories
printLog(log, ">>> Setup build directories <<<")
mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
for dir in RbankIgPaths:
	mkPath(log, ExportBuildDirectory + "/" + dir)
for dir in RbankShapePaths:
	mkPath(log, ExportBuildDirectory + "/" + dir)
mkPath(log, ExportBuildDirectory + "/" + RbankBboxBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + IgLandBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + IgVillageBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + RbankTessellationBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + RbankSmoothBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + RbankRawBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + RbankPreprocBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + RbankMeshBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + RbankRetrieversBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + RbankOutputBuildDirectory)

# Setup client directories
printLog(log, ">>> Setup client directories <<<")
mkPath(log, ClientDataDirectory + "/" + PacsClientDirectory)

log.close()


# end of file
