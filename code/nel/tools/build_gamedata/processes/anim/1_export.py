#!/usr/bin/python
# 
# #################################################################
# ## WARNING : this is a generated file, don't change it !
# #################################################################
# 
# \file 1_export.py
# \brief Export anim
# \date 2011-09-21-20-51-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export anim
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
if os.path.isfile("temp_log.log"):
	os.remove("temp_log.log")
log = open("temp_log.log", "w")
from scripts import *
from buildsite import *
from process import *
from tools import *
from directories import *

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Export anim")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")


# Find tools
# ...

# Export anim 3dsmax
if MaxAvailable:
	# Find tools
	Max = findMax(log, MaxDirectory, MaxExecutable)
	printLog(log, "")
	
	printLog(log, ">>> Export anim 3dsmax <<<")
	mkPath(log, ExportBuildDirectory + "/" + AnimExportDirectory)
	mkPath(log, ExportBuildDirectory + "/" + AnimTagExportDirectory)
	for dir in AnimSourceDirectories:
		mkPath(log, DatabaseDirectory + "/" + dir)
		if (needUpdateDirByTagLog(log, DatabaseDirectory + "/" + dir, ".max", ExportBuildDirectory + "/" + AnimTagExportDirectory, ".max.tag")):
			scriptSrc = "maxscript/anim_export.ms"
			scriptDst = MaxUserDirectory + "/scripts/anim_export.ms"
			outputLogfile = ScriptDirectory + "/processes/anim/log.log"
			outputDirectory =  ExportBuildDirectory + "/" + AnimExportDirectory
			tagDirectory =  ExportBuildDirectory + "/" + AnimTagExportDirectory
			maxSourceDir = DatabaseDirectory + "/" + dir
			maxRunningTagFile = tagDirectory + "/max_running.tag"
			tagList = findFiles(log, tagDirectory, "", ".max.tag")
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
				newline = newline.replace("%TagDirectory%", tagDirectory)
				sDst.write(newline)
			sSrc.close()
			sDst.close()
			zeroRetryLimit = 3
			while tagDiff > 0:
				mrt = open(maxRunningTagFile, "w")
				mrt.write("moe-moe-kyun")
				mrt.close()
				printLog(log, "MAXSCRIPT " + scriptDst)
				subprocess.call([ Max, "-U", "MAXScript", "anim_export.ms", "-q", "-mi", "-mip" ])
				if os.path.exists(outputLogfile):
					try:
						lSrc = open(outputLogfile, "r")
						for line in lSrc:
							lineStrip = line.strip()
							if (len(lineStrip) > 0):
								printLog(log, lineStrip)
						lSrc.close()
						os.remove(outputLogfile)
					except Exception:
						printLog(log, "ERROR Failed to read 3dsmax log")
				else:
					printLog(log, "WARNING No 3dsmax log")
				tagList = findFiles(log, tagDirectory, "", ".max.tag")
				newTagLen = len(tagList)
				tagDiff = newTagLen - tagLen
				tagLen = newTagLen
				addTagDiff = 0
				if os.path.exists(maxRunningTagFile):
					printLog(log, "FAIL 3ds Max crashed and/or file export failed!")
					if tagDiff == 0:
						if zeroRetryLimit > 0:
							zeroRetryLimit = zeroRetryLimit - 1
							addTagDiff = 1
						else:
							printLog(log, "FAIL Retry limit reached!")
					else:
						addTagDiff = 1
					os.remove(maxRunningTagFile)
				printLog(log, "Exported " + str(tagDiff) + " .max files!")
				tagDiff += addTagDiff
			os.remove(scriptDst)
	printLog(log, "")



log.close()
if os.path.isfile("log.log"):
	os.remove("log.log")
shutil.move("temp_log.log", "log.log")


# end of file
