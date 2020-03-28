#!/usr/bin/python
# 
# \file 3_install.py
# \brief Install zone_light
# \date 2009-03-11-13-45-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install zone_light
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
printLog(log, "--- Install zone_light")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

printLog(log, ">>> Install zone_light zones <<<")
srcDir = ExportBuildDirectory + "/" + ZoneLightBuildDirectory
mkPath(log, srcDir)
destDir = InstallDirectory + "/" + ZoneInstallDirectory
mkPath(log, destDir)
copyFilesNoTreeIfNeeded(log, srcDir, destDir)

printLog(log, ">>> Install zone_light water maps <<<")
srcDir = ExportBuildDirectory + "/" + ZoneLightWaterShapesLightedExportDirectory
mkPath(log, srcDir)
destDir = InstallDirectory + "/" + WaterMapsInstallDirectory
mkPath(log, destDir)
copyFilesNoTreeIfNeeded(log, srcDir, destDir)

printLog(log, ">>> Install zone_light igs <<<")
srcDir = ExportBuildDirectory + "/" + ZoneLightIgLandBuildDirectory
mkPath(log, srcDir)
destDir = InstallDirectory + "/" + IgInstallDirectory
mkPath(log, destDir)
copyFilesNoTreeIfNeeded(log, srcDir, destDir)

printLog(log, "")
log.close()


# end of file
