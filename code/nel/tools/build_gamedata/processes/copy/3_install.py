#!/usr/bin/python
# 
# \file 3_install.py
# \brief Install copy
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install copy
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
printLog(log, "--- Install copy")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

installPath = InstallDirectory + "/" + CopyInstallDirectory
mkPath(log, installPath)

printLog(log, ">>> Install copy <<<")
for dir in CopyDirectSourceDirectories:
	copyFilesRecursiveNoTreeIfNeeded(log, dir, installPath)
for file in CopyDirectSourceFiles:
	copyFileIfNeeded(log, file, installPath + "/" + os.path.basename(file))
for dir in CopyLeveldesignSourceDirectories:
	copyFilesRecursiveNoTreeIfNeeded(log, LeveldesignDirectory + "/" + dir, installPath)
for file in CopyLeveldesignSourceFiles:
	copyFileIfNeeded(log, LeveldesignDirectory + "/" + file, installPath + "/" + os.path.basename(file))
for dir in CopyLeveldesignWorldSourceDirectories:
	copyFilesRecursiveNoTreeIfNeeded(log, LeveldesignWorldDirectory + "/" + dir, installPath)
for file in CopyLeveldesignWorldSourceFiles:
	copyFileIfNeeded(log, LeveldesignWorldDirectory + "/" + file, installPath + "/" + os.path.basename(file))
for dir in CopyLeveldesignDfnSourceDirectories:
	copyFilesRecursiveNoTreeIfNeeded(log, LeveldesignDfnDirectory + "/" + dir, installPath)
for file in CopyLeveldesignDfnSourceFiles:
	copyFileIfNeeded(log, LeveldesignDfnDirectory + "/" + file, installPath + "/" + os.path.basename(file))
for dir in CopyDatabaseSourceDirectories:
	copyFilesRecursiveNoTreeIfNeeded(log, DatabaseDirectory + "/" + dir, installPath)
for file in CopyDatabaseSourceFiles:
	copyFileIfNeeded(log, DatabaseDirectory + "/" + file, installPath + "/" + os.path.basename(file))

try:
	CopyWindowsExeDllCfgSourceFiles
except NameError:
	CopyWindowsExeDllCfgSourceFiles = [ ]
for file in CopyWindowsExeDllCfgSourceFiles:
	filePath = findFileMultiDir(log, WindowsExeDllCfgDirectories, file)
	if (filePath != ""):
		copyFileIfNeeded(log, filePath, installPath + "/" + os.path.basename(file))

printLog(log, "")

log.close()


# end of file
