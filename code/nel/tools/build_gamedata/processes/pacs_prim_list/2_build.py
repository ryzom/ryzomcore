#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build pacs_prim_list
# \date 2011-09-28 7:22GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build pacs_prim_list
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
printLog(log, "--- Build pacs_prim_list")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

printLog(log, ">>> List pacs_prim <<<")
listPath = DataCommonDirectory + "/landscape_col_prim_pacs_list.txt"
if os.path.isfile(listPath):
	os.remove(listPath)
listFile = open(listPath, "w")
printLog(log, "WRITE " + listPath)
for dir in PacsPrimExportSourceDirectories:
	outDirPacsPrim = ExportBuildDirectory + "/" + dir
	mkPath(log, outDirPacsPrim)
	exportedPacsPrims = findFiles(log, outDirPacsPrim, "", ".pacs_prim")
	for exported in exportedPacsPrims:
		listFile.write(exported + "\n")
listFile.close()

log.close()


# end of file
