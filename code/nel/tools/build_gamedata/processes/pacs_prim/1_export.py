#!/usr/bin/python
# 
# #################################################################
# ## WARNING : this is a generated file, don't change it !
# #################################################################
# 
# \file 1_export.py
# \brief Export pacs_prim
# \date 2011-09-28-07-42-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export pacs_prim
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
printLog(log, "--- Export pacs_prim")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
# ...

# Export pacs_prim 3dsmax
if MaxAvailable:
	# Find tools
	Max = findMax(log, MaxDirectory, MaxExecutable)
	printLog(log, "")
	
	printLog(log, ">>> Export pacs_prim 3dsmax <<<")
	mkPath(log, ExportBuildDirectory + "/" + PacsPrimExportDirectory)
	for dir in PacsPrimSourceDirectories:
		mkPath(log, DatabaseDirectory + "/" + dir)
		if (needUpdateDirByTagLog(log, DatabaseDirectory + "/" + dir, ".max", ExportBuildDirectory + "/" + PacsPrimExportDirectory, ".pacs_prim")):
			scriptSrc = "maxscript/pacs_prim_export.ms"
			scriptDst = MaxUserDirectory + "/scripts/pacs_prim_export.ms"
			outputLogfile = ScriptDirectory + "/processes/pacs_prim/log.log"
			outputDirectory =  ExportBuildDirectory + "/" + PacsPrimExportDirectory
			maxSourceDir = DatabaseDirectory + "/" + dir
			tagList = findFiles(log, outputDirectory, "", ".pacs_prim")
			tagLen = len(tagList)
			if os.path.isfile(scriptDst):
				os.remove(scriptDst)
			tagDiff = 1
			sSrc = open(scriptSrc, "r")
			sDst = open(scriptDst, "w")
			for line in sSrc:
				newline = line.replace("%OutputLogfile%", outputLogfile)
				newline = newline.replace("%MaxSourceDirectory%", maxSourceDir)
				newline = newline.replace("%OutputDirectory%", outputDirectory)
				sDst.write(newline)
			sSrc.close()
			sDst.close()
			while tagDiff > 0:
				printLog(log, "MAXSCRIPT " + scriptDst)
				subprocess.call([ Max, "-U", "MAXScript", "pacs_prim_export.ms", "-q", "-mi", "-vn" ])
				tagList = findFiles(log, outputDirectory, "", ".pacs_prim")
				newTagLen = len(tagList)
				tagDiff = newTagLen - tagLen
				tagLen = newTagLen
				printLog(log, "Exported " + str(tagDiff) + " .pacs_prim files!")
			os.remove(scriptDst)



# Remove bad file from previous script version
listPath = ExportBuildDirectory + "/" + PacsPrimExportDirectory + "/landscape_col_prim_pacs_list.txt"
if os.path.isfile(listPath):
	os.remove(listPath)



printLog(log, "")
log.close()


# end of file
