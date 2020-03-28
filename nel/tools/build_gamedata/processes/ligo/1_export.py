#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export ligo
# \date 2010-05-24 08:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export ligo
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
printLog(log, "--- Export ligo")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

if LigoExportLand == "" or LigoExportOnePass == 1:
	# Find tools
	Max = findMax(log, MaxDirectory, MaxExecutable)
	ExecTimeout = findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
	printLog(log, "")
	
	# For each directory
	printLog(log, ">>> Export ligo 3dsmax <<<")
	
	ligoIniPath = MaxUserDirectory + "/plugcfg/nelligo.ini"
	mkPath(log, ExportBuildDirectory + "/" + SmallbankExportDirectory)
	mkPath(log, DatabaseDirectory + "/" + LigoMaxSourceDirectory)
	mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemExportDirectory)
	mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemIgExportDirectory)
	mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemZoneExportDirectory)
	mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemZoneLigoExportDirectory)
	mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemCmbExportDirectory)
	mkPath(log, DatabaseDirectory + "/" + ZoneSourceDirectory[0])
	tagDirectory = ExportBuildDirectory + "/" + LigoEcosystemTagExportDirectory
	mkPath(log, tagDirectory)
	if (needUpdateDirByTagLogFiltered(log, DatabaseDirectory + "/" + LigoMaxSourceDirectory, ".max", tagDirectory, ".max.tag", [ "zonematerial", "zonetransition", "zonespecial" ])):
		printLog(log, "WRITE " + ligoIniPath)
		ligoIni = open(ligoIniPath, "w")
		ligoIni.write("[LigoConfig]\n")
		ligoIni.write("LigoPath=" + DatabaseDirectory + "/" + LigoMaxSourceDirectory + "/\n")
		ligoIni.write("LigoExportPath=" + ExportBuildDirectory + "/" + LigoEcosystemExportDirectory + "/\n")
		ligoIni.write("LigoOldZonePath=" + DatabaseDirectory + "/" + ZoneSourceDirectory[0] + "/\n")
		ligoIni.close()
		
		outputLogfile = ScriptDirectory + "/processes/ligo/log.log"
		smallBank = ExportBuildDirectory + "/" + SmallbankExportDirectory + "/" + BankTileBankName + ".smallbank"
		maxRunningTagFile = tagDirectory + "/max_running.tag"
		
		scriptSrc = "maxscript/nel_ligo_export.ms"
		scriptDst = MaxUserDirectory + "/scripts/nel_ligo_export.ms"
		
		tagList = findFiles(log, tagDirectory, "", ".max.tag")
		tagLen = len(tagList)
		
		if os.path.isfile(scriptDst):
			os.remove(scriptDst)
		
		tagDiff = 1
		printLog(log, "WRITE " + scriptDst)
		sSrc = open(scriptSrc, "r")
		sDst = open(scriptDst, "w")
		for line in sSrc:
			newline = line.replace("%OutputLogfile%", outputLogfile)
			newline = newline.replace("%TagDirectory%", tagDirectory)
			newline = newline.replace("%SmallBankFilename%", smallBank)
			sDst.write(newline)
		sSrc.close()
		sDst.close()
		
		zeroRetryLimit = 3
		while tagDiff > 0:
			mrt = open(maxRunningTagFile, "w")
			mrt.write("moe-moe-kyun")
			mrt.close()
			printLog(log, "MAXSCRIPT " + scriptDst)
			subprocess.call([ Max, "-U", "MAXScript", "nel_ligo_export.ms", "-q", "-mi", "-mip" ])
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
