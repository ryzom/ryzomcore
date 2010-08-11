#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export ig
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export ig
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
printLog(log, "--- Export ig")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

Max = "" #find later


def igExport(sourceDir, targetDir):
	scriptSrc = "maxscript/ig_export.ms"
	scriptDst = MaxUserDirectory + "/scripts/ig_export.ms"
	logFile = ScriptDirectory + "/processes/ig/log.log"
	outDirTag = ExportBuildDirectory + "/" + IgStaticTagExportDirectory
	outDirIg =  ExportBuildDirectory + "/" + targetDir
	igSourceDir = DatabaseDirectory + "/" + sourceDir
	tagList = findFiles(log, outDirTag, "", ".tag")
	tagLen = len(tagList)
	if os.path.isfile(scriptDst):
		os.remove(scriptDst)
	tagDiff = 1
	sSrc = open(scriptSrc, "r")
	sDst = open(scriptDst, "w")
	for line in sSrc:
		newline = line.replace("output_logfile", logFile)
		newline = newline.replace("ig_source_directory", igSourceDir)
		newline = newline.replace("output_directory_tag", outDirTag)
		newline = newline.replace("output_directory_ig", outDirIg)
		sDst.write(newline)
	sSrc.close()
	sDst.close()
	while tagDiff > 0:
		printLog(log, "MAXSCRIPT " + scriptDst)
		subprocess.call([ Max, "-U", "MAXScript", "ig_export.ms", "-q", "-mi", "-vn" ])
		tagList = findFiles(log, outDirTag, "", ".tag")
		newTagLen = len(tagList)
		tagDiff = newTagLen - tagLen
		tagLen = newTagLen
		printLog(log, "Exported " + str(tagDiff) + " .max files!")
	os.remove(scriptDst)
	return


if MaxAvailable:
	# Find tools
	Max = findMax(log, MaxDirectory, MaxExecutable)
	# ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
	printLog(log, "")
	
	mkPath(log, ExportBuildDirectory + "/" + IgStaticTagExportDirectory)
	
	# Export ig land 3dsmax
	printLog(log, ">>> Export ig land 3dsmax <<<")
	mkPath(log, ExportBuildDirectory + "/" + IgStaticLandExportDirectory)
	for dir in IgLandSourceDirectories:
		mkPath(log, DatabaseDirectory + "/" + dir)
		igExport(dir, IgStaticLandExportDirectory)
	
	# Export ig other 3dsmax
	printLog(log, ">>> Export ig other 3dsmax <<<")
	mkPath(log, ExportBuildDirectory + "/" + IgStaticOtherExportDirectory)
	for dir in IgOtherSourceDirectories:
		mkPath(log, DatabaseDirectory + "/" + dir)
		igExport(dir, IgStaticOtherExportDirectory)


printLog(log, "")

log.close()


# end of file
