#!/usr/bin/python
# 
# \file 3_install.py
# \brief Install map
# \date 2009-03-10 13:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install map
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
printLog(log, "--- Install map")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

installPath = InstallDirectory + "/" + MapInstallDirectory
mkPath(log, installPath)

printLog(log, ">>> Install map <<<")
sourcePaths = [ ExportBuildDirectory + "/" + MapBuildDirectory ] + [ ExportBuildDirectory + "/" + MapUncompressedExportDirectory ]
for sourcePath in sourcePaths:
	mkPath(log, sourcePath)
	files = os.listdir(sourcePath)
	len_ext = 4
	for fileName in files:
		if isLegalFileName(fileName):
			sourceFile = sourcePath + "/" + fileName
			if os.path.isfile(sourceFile):
				if (fileName[-len_ext:].lower() == ".tga") or (fileName[-len_ext:].lower() == ".png") or (fileName[-len_ext:].lower() == ".dds"):
					copyFileIfNeeded(log, sourceFile, installPath + "/" + os.path.basename(fileName))
			elif not os.path.isdir(sourceFile):
				printLog(log, "FAIL ?! file not dir or file ?! " + sourceFile)

if MapPanoplyFileList != None:
	printLog(log, ">>> Install panoply file list <<<")
	buildPanoplyTagPath = ExportBuildDirectory + "/" + MapTagBuildDirectory + "/build_panoply.tag"
	mkPath(log, ExportBuildDirectory + "/" + MapTagBuildDirectory)
	if needUpdate(log, buildPanoplyTagPath, installPath + "/" + MapPanoplyFileList):
		sourcePath = ExportBuildDirectory + "/" + MapPanoplyBuildDirectory
		mkPath(log, sourcePath)
		printLog(log, "WRITE " + installPath + "/" + MapPanoplyFileList)
		lf = open(installPath + "/" + MapPanoplyFileList, "w")
		files = os.listdir(sourcePath)
		for file in files:
			if isLegalFileName(file):
				lf.write(file + "\n")
		lf.close()
	else:
		printLog(log, "SKIP " + installPath + "/" + MapPanoplyBuildDirectory)

if MapHlsBankFileName != None:
	printLog(log, ">>> Install map hlsbank <<<")
	sourcePath = ExportBuildDirectory + "/" + MapPanoplyHlsBankBuildDirectory
	mkPath(log, sourcePath)
	copyFilesExtNoSubdirIfNeeded(log, sourcePath, installPath, ".hlsbank")

printLog(log, "")
log.close()


# end of file
