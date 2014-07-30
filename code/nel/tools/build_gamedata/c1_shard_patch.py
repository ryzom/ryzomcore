#!/usr/bin/python
# 
# \file c1_shard_patch.py
# \brief Create a new patch for the patchman bridge
# \date 2014-02-20 00:27GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Create a new patch for the patchman bridge
# 
# NeL - MMORPG Framework <http://www.ryzomcore.org/>
# Copyright (C) 2014  by authors
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

import time, sys, os, shutil, subprocess, distutils.dir_util, tarfile, argparse
sys.path.append("configuration")

parser = argparse.ArgumentParser(description='Ryzom Core - Build Gamedata - Shard Patch')
parser.add_argument('--admininstall', '-ai', action='store_true')
args = parser.parse_args()

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
printLog(log, "--- Create a new patch for the patchman bridge")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# List the directories that will be used
archiveDirectories = [ ]
for dir in InstallShardDataDirectories:
	if not dir in archiveDirectories:
		archiveDirectories += [ dir ]
for package in InstallShardDataFiles:
	dstDir = package[0]
	if not dstDir in archiveDirectories:
		archiveDirectories += [ dstDir ]
for multiDir in InstallShardDataMultiDirectories:
	dstDir = multiDir[0]
	if not dstDir in archiveDirectories:
		archiveDirectories += [ dstDir ]
for multiDir in InstallShardDataPrimitivesDirectories:
	dstDir = multiDir[0]
	if not dstDir in archiveDirectories:
		archiveDirectories += [ dstDir ]
for execDir in InstallShardDataExecutables:
	dstDir = execDir[0]
	if not dstDir in archiveDirectories:
		archiveDirectories += [ dstDir ]

printLog(log, ">>> Archive new admin_install.tgz <<<")
mkPath(log, PatchmanBridgeServerDirectory)
adminInstallTgz = PatchmanBridgeServerDirectory + "/admin_install.tgz"
patchmanExecutable = LinuxServiceExecutableDirectory + "/ryzom_patchman_service"
if needUpdateDirNoSubdirFile(log, PatchmanCfgAdminDirectory + "/bin", adminInstallTgz) or needUpdateDirNoSubdirFile(log, PatchmanCfgAdminDirectory + "/patchman", adminInstallTgz) or needUpdate(log, patchmanExecutable, adminInstallTgz):
	printLog(log, "WRITE " + adminInstallTgz)
	if os.path.isfile(adminInstallTgz):
		os.remove(adminInstallTgz)
	tar = tarfile.open(adminInstallTgz, "w:gz")
	tar.add(PatchmanCfgAdminDirectory + "/bin", arcname = "bin")
	tar.add(PatchmanCfgAdminDirectory + "/patchman", arcname = "patchman")
	tar.add(patchmanExecutable, arcname = "patchman/ryzom_patchman_service")
	tar.close()
else:
	printLog(log, "SKIP " + adminInstallTgz)
printLog(log, "")

if not args.admininstall:
	printLog(log, ">>> Create new version <<<")
	newVersion = 1
	vstr = str(newVersion).zfill(6)
	vpath = PatchmanBridgeServerDirectory + "/" + vstr
	while os.path.exists(vpath):
		newVersion = newVersion + 1
		vstr = str(newVersion).zfill(6)
		vpath = PatchmanBridgeServerDirectory + "/" + vstr
	mkPath(log, vpath)
	for dir in archiveDirectories:
		mkPath(log, ShardInstallDirectory + "/" + dir)
		tgzPath = vpath + "/" + dir + ".tgz"
		printLog(log, "WRITE " + tgzPath)
		tar = tarfile.open(tgzPath, "w:gz")
		tar.add(ShardInstallDirectory + "/" + dir, arcname = dir)
		tar.close()
	printLog(log, "")

log.close()
if os.path.isfile("c1_shard_patch.log"):
	os.remove("c1_shard_patch.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_shard_patch.log")
shutil.move("log.log", "c1_shard_patch.log")
