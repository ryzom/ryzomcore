#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export cegui
# \date 2009-03-14-17-46-GMT
# \author Jan Boon (Kaetemi)
# Export cegui
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
printLog(log, "--- Export cegui")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# For each cegui imageset directory
printLog(log, ">>> Export cegui imagesets <<<")
destDir = ExportBuildDirectory + "/" + CeguiImagesetExportDirectory
mkPath(log, destDir)
for dir in CeguiImagesetSourceDirectories:
	srcDir = DatabaseDirectory + "/" + dir
	mkPath(log, srcDir)
	imagesets = findFiles(log, srcDir, "", ".imageset")
	if (len(imagesets) != 1):
		printLog(log, "FAIL Cannot find *.imageset, folder must contain at least one and only one imageset xml file")
	else:
		niouname = dir.replace("/", "_")
		newpath = destDir + "/" + niouname
		mkPath(log, newpath)
		copyFileIfNeeded(log, srcDir + "/" + imagesets[0], newpath + ".imageset")
		copyFilesExtNoTreeIfNeeded(log, srcDir, newpath, ".tga")
		copyFilesExtNoTreeIfNeeded(log, srcDir, newpath, ".png")
printLog(log, "")

log.close()


# end of file
