#!/usr/bin/python
# 
# \file 3_install.py
# \brief Install cartographer
# \date 2014-09-13 13:32GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install cartographer
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2014  Jan BOON
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
from buildsite_local import *
from process import *
from tools import *
from directories import *

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Install cartographer")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")


installPath = InstallDirectory + "/" + CartographerInstallDirectory
islandsInstallPath = InstallDirectory + "/" + IslandsInstallDirectory


printLog(log, ">>> Install cartographer <<<")

mkPath(log, ExportBuildDirectory + "/" + CartographerMapBuildDirectory)
mkPath(log, installPath)
copyFilesExtNoTreeIfNeeded(log, ExportBuildDirectory + "/" + CartographerMapBuildDirectory, installPath, ".dds")

mkPath(log, ExportBuildDirectory + "/" + CartographerBuildDirectory)
mkPath(log, islandsInstallPath)
copyFilesExtNoTreeIfNeeded(log, ExportBuildDirectory + "/" + CartographerBuildDirectory, islandsInstallPath, ".xml")


printLog(log, "")
log.close()


# end of file
