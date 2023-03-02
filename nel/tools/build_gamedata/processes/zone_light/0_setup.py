#!/usr/bin/python
# 
# \file 0_setup.py
# \brief Setup zone_light
# \date 2009-03-11-13-45-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Setup zone_light
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2009-2014  by authors
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
printLog(log, "--- Setup zone_light")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Setup source directories
printLog(log, ">>> Setup source directories <<<")
for dir in WaterMapSourceDirectories:
	mkPath(log, DatabaseDirectory + "/" + dir)

# Setup export directories
printLog(log, ">>> Setup export directories <<<")
mkPath(log, ExportBuildDirectory + "/" + ZoneLightWaterShapesLightedExportDirectory)

# Setup build directories
printLog(log, ">>> Setup build directories <<<")
mkPath(log, ExportBuildDirectory + "/" + ZoneWeldBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + ZoneDependBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + ZoneLightBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + ZoneLightDependBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + ZoneLightIgLandBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + IgLandBuildDirectory)

# Setup client directories
printLog(log, ">>> Setup client directories <<<")
mkPath(log, InstallDirectory + "/" + ZoneInstallDirectory)
mkPath(log, InstallDirectory + "/" + IgInstallDirectory)
mkPath(log, InstallDirectory + "/" + WaterMapsInstallDirectory)

log.close()


# end of file
