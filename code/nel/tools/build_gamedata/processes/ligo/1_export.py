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
log = open("log.log", "w")
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
	mkPath(log, ExportBuildDirectory + "/" + LigoEcosystemTagExportDirectory)
	if (needUpdateDirByTagLogFiltered(log, DatabaseDirectory + "/" + LigoMaxSourceDirectory, ".max", ExportBuildDirectory + "/" + LigoEcosystemTagExportDirectory, ".max.tag", [ "zonematerial", "zonetransition", "zonespecial" ])):
		printLog(log, "WRITE " + ligoIniPath)
		ligoIni = open(ligoIniPath, "w")
		ligoIni.write("[LigoConfig]\n")
		ligoIni.write("LigoPath=" + DatabaseDirectory + "/" + LigoMaxSourceDirectory + "/\n")
		ligoIni.write("LigoExportPath=" + ExportBuildDirectory + "/" + LigoEcosystemExportDirectory + "/\n")
		ligoIni.write("LigoOldZonePath=" + DatabaseDirectory + "/" + ZoneSourceDirectory[0] + "/\n")
		ligoIni.close()
		
		outDirTag = ExportBuildDirectory + "/" + LigoEcosystemTagExportDirectory
		logFile = ScriptDirectory + "/processes/ligo/log.log"
		smallBank = ExportBuildDirectory + "/" + SmallbankExportDirectory + "/" + BankTileBankName + ".smallbank"
		
		scriptSrc = "maxscript/nel_ligo_export.ms"
		scriptDst = MaxUserDirectory + "/scripts/nel_ligo_export.ms"
		
		if os.path.isfile(scriptDst):
			os.remove(scriptDst)
		
		printLog(log, "WRITE " + scriptDst)
		sSrc = open(scriptSrc, "r")
		sDst = open(scriptDst, "w")
		for line in sSrc:
			newline = line.replace("output_logfile", logFile)
			newline = newline.replace("output_directory_tag", outDirTag)
			newline = newline.replace("bankFilename", smallBank)
			sDst.write(newline)
		sSrc.close()
		sDst.close()
		
		printLog(log, "MAXSCRIPT " + scriptDst)
		subprocess.call([ Max, "-U", "MAXScript", "nel_ligo_export.ms", "-q", "-mi", "-mip" ])
		
		os.remove(scriptDst)
	printLog(log, "")

log.close()


# end of file
