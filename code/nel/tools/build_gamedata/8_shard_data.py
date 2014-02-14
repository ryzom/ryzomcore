#!/usr/bin/python
# 
# \file 8_shard_data.py
# \brief Install shard data
# \date 2009-02-18 16:19GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install shard data
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
sys.path.append("configuration")

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite import *
from tools import *

sys.path.append(WorkspaceDirectory)
from projects import *

# Log error
printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Install shard data")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

for dir in InstallShardDataDirectories:
	printLog(log, "SHARD DIRECTORY " + dir)
	mkPath(log, ShardInstallDirectory + "/" + dir)
	printLog(log, "FROM " + dir)
	mkPath(log, InstallDirectory + "/" + dir)
	copyFilesNoTreeIfNeeded(log, InstallDirectory + "/" + dir, ShardInstallDirectory + "/" + dir)
for multiDir in InstallShardDataMultiDirectories:
	dstDir = multiDir[0]
	mkPath(log, ShardInstallDirectory + "/" + dstDir)
	printLog(log, "SHARD DIRECTORY " + dstDir)
	for srcDir in multiDir[1]:
		printLog(log, "FROM " + srcDir)
		mkPath(log, InstallDirectory + "/" + srcDir)
		mkPath(log, ShardInstallDirectory + "/" + dstDir + "/" + srcDir)
		copyFilesNoTreeIfNeeded(log, InstallDirectory + "/" + srcDir, ShardInstallDirectory + "/" + dstDir + "/" + srcDir)
for multiDir in InstallShardDataPrimitivesDirectories:
	dstDir = multiDir[0]
	mkPath(log, ShardInstallDirectory + "/" + dstDir)
	printLog(log, "SHARD DIRECTORY " + dstDir)
	for srcDir in multiDir[1]:
		printLog(log, "FROM PRIMITIVES " + srcDir)
		mkPath(log, PrimitivesDirectory + "/" + srcDir)
		mkPath(log, ShardInstallDirectory + "/" + dstDir + "/" + srcDir)
		copyFilesNoTreeIfNeeded(log, PrimitivesDirectory + "/" + srcDir, ShardInstallDirectory + "/" + dstDir + "/" + srcDir)
for execDir in InstallShardDataExecutables:
	dstDir = execDir[0]
	mkPath(log, LinuxServiceExecutableDirectory)
	mkPath(log, PatchmanCfgDefaultDirectory)
	mkPath(log, InstallDirectory)
	mkPath(log, ShardInstallDirectory + "/" + dstDir)
	printLog(log, "SHARD DIRECTORY " + dstDir)
	copyFileIfNeeded(log, LinuxServiceExecutableDirectory + "/" + execDir[1][1], ShardInstallDirectory + "/" + dstDir + "/" + execDir[1][0])
	copyFileListNoTreeIfNeeded(log, PatchmanCfgDefaultDirectory, ShardInstallDirectory + "/" + dstDir, execDir[2])
	copyFileListNoTreeIfNeeded(log, InstallDirectory, ShardInstallDirectory + "/" + dstDir, execDir[3])
printLog(log, "")

log.close()
if os.path.isfile("8_shard_data.log"):
	os.remove("8_shard_data.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_shard_install.log")
shutil.move("log.log", "8_shard_data.log")
