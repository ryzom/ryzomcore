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
	mkPath(log, InstallDirectory + "/" + dir)
	mkPath(log, ShardInstallDirectory + "/" + dir)
	copyFilesNoTreeIfNeeded(log, InstallDirectory + "/" + dir, ShardInstallDirectory + "/" + dir)
for dir in InstallShardDataCollisionsDirectories:
	printLog(log, "SHARD COLLISIONS " + dir)
	mkPath(log, InstallDirectory + "/" + dir)
	mkPath(log, ShardInstallDirectory + "/" + InstallShardDataCollisionsDirectory + "/" + dir)
	copyFilesNoTreeIfNeeded(log, InstallDirectory + "/" + dir, ShardInstallDirectory + "/" + InstallShardDataCollisionsDirectory + "/" + dir)
printLog(log, "")

log.close()
if os.path.isfile("8_shard_data.log"):
	os.remove("8_shard_data.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_shard_install.log")
shutil.move("log.log", "8_shard_data.log")
