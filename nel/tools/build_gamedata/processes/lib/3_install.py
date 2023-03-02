#!/usr/bin/python
# 
# \file 3_install.py
# \brief Install lib
# \date 2022-04-29 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install lib
# 
# NeL - MMORPG Framework <https://wiki.ryzom.dev/>
# Copyright (C) 2009-2022  by authors
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
printLog(log, "--- Install lib")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

srcDir = ExportBuildDirectory + "/" + LibExeDllDirectory
mkPath(log, srcDir)
destDir = InstallDirectory + "/" + LibInstallDirectory
mkPath(log, destDir)
copyFilesNoTreeIfNeeded(log, srcDir, destDir)

printLog(log, "")
log.close()


# end of file
