#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export interface
# \date 2009-03-10 13:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export interface
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
printLog(log, "--- Export interface")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
TgaToDds = findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)
printLog(log, "")

# For each interface directory
printLog(log, ">>> Export interface <<<")
mkPath(log, ExportBuildDirectory + "/" + InterfaceExportDirectory)
for dir in InterfaceSourceDirectories:
	mkPath(log, DatabaseDirectory + "/" + dir)
	niouname = dir.replace("/", "_")
	newpath = ExportBuildDirectory + "/" + InterfaceExportDirectory + "/" + niouname
	mkPath(log, newpath)
	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, newpath, ".tga")
printLog(log, "")

# For each interface directory to compress in one DXTC
printLog(log, ">>> Export interface dxtc <<<")
mkPath(log, ExportBuildDirectory + "/" + InterfaceDxtcExportDirectory)
for dir in InterfaceDxtcSourceDirectories:
	mkPath(log, DatabaseDirectory + "/" + dir)
	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, ExportBuildDirectory + "/" + InterfaceDxtcExportDirectory, ".tga")
printLog(log, "")

# For each interface fullscreen directory compress independently all in dds
printLog(log, ">>> Export interface fullscreen <<<")
if TgaToDds == "":
	toolLogFail(log, TgaToDdsTool, ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + InterfaceFullscreenExportDirectory)
	for dir in InterfaceFullscreenSourceDirectories:
		mkPath(log, DatabaseDirectory + "/" + dir)
		files = findFiles(log, DatabaseDirectory + "/" + dir, "", ".tga")
		for file in files:
			sourceFile = DatabaseDirectory + "/" + dir + "/" + file
			destFile = ExportBuildDirectory + "/" + InterfaceFullscreenExportDirectory + "/" + os.path.basename(file)[0:-len(".tga")] + ".dds"
			if needUpdateLogRemoveDest(log, sourceFile, destFile):
				subprocess.call([ TgaToDds, sourceFile, "-o", destFile, "-a", "5" ])
printLog(log, "")

# For each interface 3d directory
printLog(log, ">>> Export interface 3d <<<")
mkPath(log, ExportBuildDirectory + "/" + Interface3DExportDirectory)
for dir in Interface3DSourceDirectories:
	mkPath(log, DatabaseDirectory + "/" + dir)
	copyFilesExtNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, ExportBuildDirectory + "/" + Interface3DExportDirectory, ".tga")
printLog(log, "")

log.close()


# end of file
