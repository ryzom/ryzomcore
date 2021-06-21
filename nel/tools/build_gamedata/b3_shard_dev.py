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

# Find tools
PatchmanService = findTool(log, ToolDirectories, PatchmanServiceTool, ToolSuffix)
printLog(log, "")

mkPath(log, ShardDevDirectory)
mkPath(log, ShardDevDirectory + "/local")
printLog(log, ">>> Generate shard dev local.cfg <<<")
cfg = open(ShardDevDirectory + "/local.cfg", "w")
cfg.write("WindowStyle = \"WIN\";\n")
cfg.write("Paths += {\n")
cfg.write("	\"" + ShardDevDirectory + "/local\",\n")
cfg.write("	\"" + DataCommonDirectory + "\",\n")
cfg.write("	\"" + DataShardDirectory + "\",\n")
cfg.write("	\"" + LeveldesignDirectory + "\",\n")
cfg.write("	\"" + WorldEditorFilesDirectory + "\",\n")
for dir in InstallShardDataDirectories:
	mkPath(log, ShardInstallDirectory + "/" + dir)
	cfg.write("	\"" + ShardInstallDirectory + "/" + dir + "\",\n")
for multiDir in InstallShardDataMultiDirectories:
	dstDir = multiDir[0]
	mkPath(log, ShardInstallDirectory + "/" + dstDir)
	cfg.write("	\"" + ShardInstallDirectory + "/" + dstDir + "\",\n")
cfg.write("};\n")
cfg.write("RRDToolPath = \"..\\..\\..\\external\\rrdtool\\rrdtool.exe\";\n")
cfg.write("StartCommands += {\n")
cfg.write("	// \"gw_aes.transportRemove aes_l3c\",\n")
cfg.write("};\n")
cfg.write("NegFiltersWarning += {\n")
cfg.write("	\"already inserted from\",\n")
cfg.write("};\n")
cfg.write("// Allow player to stay connected to FS when services go down\n")
cfg.write("DontNeedBackend = 1;\n")
cfg.flush()
cfg.close()
printLog(log, "")

if not os.path.exists(ShardDevDirectory  + "/aes_state.txt"):
	printLog(log, ">>> Generate shard dev aes_state.txt <<<")
	f = open(ShardDevDirectory + "/aes_state.txt", "w")
	f.write("ShardOrders unifier so_autostart_off\n")
	f.write("ShardOrders mainland so_autostart_off\n")
	f.write("ShardOrders ring so_autostart_off\n")
	f.flush()
	f.close()
	printLog(log, "")

mkPath(log, ShardDevDirectory + "/ras")
if not os.path.exists(ShardDevDirectory  + "/ras/as_state.txt"):
	printLog(log, ">>> Generate shard dev as_state.txt <<<")
	f = open(ShardDevDirectory + "/ras/as_state.txt", "w")
	f.write("ShardOrders unifier so_autostart_off\n")
	f.write("ShardOrders mainland so_autostart_off\n")
	f.write("ShardOrders ring so_autostart_off\n")
	f.flush()
	f.close()
	printLog(log, "")

for execDir in InstallShardDataExecutables:
	dstDir = execDir[0]
	mkPath(log, PatchmanCfgDefaultDirectory)
	mkPath(log, InstallDirectory)
	mkPath(log, ShardDevDirectory + "/live/" + dstDir)
	printLog(log, "SHARD PACKAGE " + dstDir)
	copyFileListNoTreeIfNeeded(log, PatchmanCfgDefaultDirectory, ShardDevDirectory + "/live/" + dstDir, execDir[2])
	copyFileListNoTreeIfNeeded(log, InstallDirectory, ShardDevDirectory + "/live/" + dstDir, execDir[3])
	for cfgName in execDir[2]:
		cfgPath = ShardDevDirectory + "/live/" + dstDir + "/" + cfgName
		found = False
		with open(cfgPath, "r") as f:
			for l in f:
				if "Paths += {" in l:
					found = True
		if not found:
			with open(cfgPath, "a") as cfg:
				cfg.write("\n")
				cfg.write("Paths += {\n")
				cfg.write("	\"" + ShardDevDirectory + "/live/" + dstDir + "\",\n")
				cfg.write("};\n")
				cfg.write("\n")
				cfg.flush()
printLog(log, "")


if PatchmanService == "":
	toolLogFail(log, PatchmanServiceTool, ToolSuffix)
else:
	mkPath(log, PatchmanDevDirectory)
	cwDir = os.getcwd().replace("\\", "/")
	os.chdir(PatchmanDevDirectory)
	if os.path.isfile("log.log"):
		os.remove("log.log")
	subprocess.call([ PatchmanService, "-C.", "-L." ])
	if os.path.isfile("log.log"):
		f = open("log.log", "r")
		for l in f:
			printLog(log, l.rstrip())
		f.close()
	os.chdir(cwDir)
	printLog(log, "")

log.close()
if os.path.isfile("b3_shard_dev.log"):
	os.remove("b3_shard_dev.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_shard_dev.log")
shutil.move("log.log", "b3_shard_dev.log")
