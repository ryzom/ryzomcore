#!/usr/bin/python
# 
# \file b2_shard_data.py
# \brief Install shard data
# \date 2009-02-18 16:19GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install shard data
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
sys.path.append("configuration")

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite_local import *
from tools import *

sys.path.append(WorkspaceDirectory)
from projects import *

# Log error
printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Install to shard dev")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

mkPath(log, ShardDevDirectory)
mkPath(log, ShardDevDirectory + "/local")
printLog(log, ">>> Generate shard dev local.cfg <<<")
cfg = open(ShardDevDirectory + "/local.cfg", "w")
cfg.write("WindowStyle = \"WIN\";")
cfg.write("Paths = {")
cfg.write("	\"" + ShardDevDirectory.replace('/', '\\') + "\\local\",")
cfg.write("	\"" + DataCommonDirectory.replace('/', '\\') + "\",")
cfg.write("	\"" + DataShardDirectory.replace('/', '\\') + "\",")
cfg.write("	\"" + LeveldesignDirectory.replace('/', '\\') + "\",")
cfg.write("	\"" + WorldEditorFilesDirectory.replace('/', '\\') + "\",")
for multiDir in InstallShardDataMultiDirectories:
	dstDir = multiDir[0]
	mkPath(log, ShardInstallDirectory + "/" + dstDir)
	cfg.write("	\"" + ShardInstallDirectory + "\\" + dstDir + "\",")
cfg.write("};")
cfg.write("StartCommands += {")
cfg.write("	// Don't launch AES on development shard for now")
cfg.write("	\"gw_aes.transportRemove aes_l3c\",")
cfg.write("};")
cfg.write("DontNeedBackend = 1;")
cfg.flush()
cfg.close()
printLog(log, "")

for execDir in InstallShardDataExecutables:
	dstDir = execDir[0]
	mkPath(log, PatchmanCfgDefaultDirectory)
	mkPath(log, InstallDirectory)
	mkPath(log, ShardDevDirectory + "/live/" + dstDir)
	printLog(log, "SHARD PACKAGE " + dstDir)
	copyFileListNoTreeIfNeeded(log, PatchmanCfgDefaultDirectory, ShardDevDirectory + "/live/" + dstDir, execDir[2])
	copyFileListNoTreeIfNeeded(log, InstallDirectory, ShardDevDirectory + "/live/" + dstDir, execDir[3])
printLog(log, "")

log.close()
if os.path.isfile("b3_shard_dev.log"):
	os.remove("b3_shard_dev.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_shard_dev.log")
shutil.move("log.log", "b3_shard_dev.log")
