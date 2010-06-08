#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export veget
# \date 2010-05-24 08:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export veget
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
printLog(log, "--- Export veget")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

if MaxAvailable:
	# Find tools
	Max = findMax(log, MaxDirectory, MaxExecutable)
	ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
	printLog(log, "")
	
	# Export veget 3dsmax	
	printLog(log, ">>> Export veget 3dsmax <<<")
	
	# Build paths
	scriptSrc = "maxscript/veget_export.ms"
	# scriptDst = MaxDirectory + "/scripts/veget_export.ms"
	scriptDst = MaxUserDirectory + "/scripts/veget_export.ms"
	logFile = ScriptDirectory + "/processes/veget/log.log"
	outputDirVeget = ExportBuildDirectory + "/" + VegetExportDirectory
	mkPath(log, outputDirVeget)
	outputDirTag = ExportBuildDirectory + "/" + VegetTagExportDirectory
	mkPath(log, outputDirTag)

	# For each directoy
	mkPath(log, ExportBuildDirectory + "/" + VegetExportDirectory)
	if os.path.isfile(scriptDst):
		os.remove(scriptDst)
	for dir in VegetSourceDirectories:
		vegetSourceDir = DatabaseDirectory + "/" + dir
		mkPath(log, vegetSourceDir)
		sSrc = open(scriptSrc, "r")
		sDst = open(scriptDst, "w")
		for line in sSrc:
			newline = line.replace("output_logfile", logFile)
			newline = newline.replace("veget_source_directory", vegetSourceDir)
			newline = newline.replace("output_directory_veget", outputDirVeget)
			newline = newline.replace("output_directory_tag", outputDirTag)
			sDst.write(newline)
		sSrc.close()
		sDst.close()
		printLog(log, "MAXSCRIPT " + scriptDst)
		subprocess.call([ Max, "-U", "MAXScript", "veget_export.ms", "-q", "-mi", "-vn" ])
		os.remove(scriptDst)

printLog(log, "")

log.close()


# end of file
