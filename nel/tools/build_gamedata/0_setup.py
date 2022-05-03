#!/usr/bin/python
# 
# \file 0_setup.py
# \brief Run all setup processes
# \date 2009-02-18 15:28GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Run all setup processes
# 
# NeL - MMORPG Framework <https://wiki.ryzom.dev/>
# Copyright (C) 2009-2022  by authors
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

import time, sys, os, shutil, subprocess, distutils.dir_util, argparse
sys.path.append("configuration")

parser = argparse.ArgumentParser(description='Ryzom Core - Build Gamedata - Setup')
parser.add_argument('--noconf', '-nc', action='store_true')
parser.add_argument('--noverify', '-nv', action='store_true')
parser.add_argument('--preset', '-p', action='store_true')
# parser.add_argument('--haltonerror', '-eh', action='store_true')
parser.add_argument('--includeproject', '-ipj', nargs='+')
parser.add_argument('--excludeproject', '-epj', nargs='+')
parser.add_argument('--includeprocess', '-ipc', nargs='+')
parser.add_argument('--excludeprocess', '-epc', nargs='+')
args = parser.parse_args()

if not args.includeproject == None and not args.excludeproject == None:
	print "ERROR --includeproject cannot be combined with --excludeproject, exit."
	exit()

if not args.includeprocess == None and not args.excludeprocess == None:
	print "ERROR --includeprocess cannot be combined with --excludeprocess, exit."
	exit()

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
try:
	from buildsite import *
except ImportError:
	printLog(log, "*** FIRST RUN ***")
	if args.noconf:
		printLog(log, "ERROR --noconf is invalid on first run, exit.")
		exit()
from tools import *

NeLToolsDirsStock = os.getenv("RC_TOOLS_DIRS_STOCK")
if NeLToolsDirsStock:
	NeLToolsDirsStock = NeLToolsDirsStock.split(os.pathsep)
NeLPath = os.getenv("RC_PATH")
if NeLPath:
	NeLToolsDirsStock = NeLToolsDirsStock + NeLPath.split(os.pathsep)

NeLServerDirsFv = os.getenv("RC_SERVER_DIRS_FV")
if NeLServerDirsFv:
	NeLServerDirsFv = NeLServerDirsFv.split(os.pathsep)

NeL3dsMaxExe = os.getenv('RC_3DSMAX_EXE')
NeL3dsMaxUserDir = os.getenv('RC_3DSMAX_USER_DIR')

if not args.noconf:
	try:
		if args.preset:
			DummyUnknownName
		BuildQuality
	except NameError:
		BuildQuality = 1
	try:
		if args.preset:
			DummyUnknownName
		RemapLocalFrom
	except NameError:
		RemapLocalFrom = 'R:'
	try:
		if args.preset:
			DummyUnknownName
		RemapLocalTo
	except NameError:
		RemapLocalTo = None
		if NeLRootDir:
			RemapLocalTo = NeLRootDir.replace('\\', '/')
		if (not RemapLocalTo) or (not ':' in RemapLocalTo):
			RemapLocalTo = 'R:'
	try:
		if args.preset:
			DummyUnknownName
		ToolDirectories
	except NameError:
		if NeLToolsDirsStock:
			ToolDirectories = [] + NeLToolsDirsStock
			for i in range(len(ToolDirectories)):
				ToolDirectories[i] = ToolDirectories[i].replace('\\', '/').replace(RemapLocalTo, RemapLocalFrom)
		else:
			ToolDirectories = [ 'R:/stock/nel_tools', 'R:/stock/ryzom_tools' ]
	try:
		ToolSuffix
	except NameError:
		ToolSuffix = ".exe"
	try:
		if args.preset:
			DummyUnknownName
		ScriptDirectory
	except NameError:
		ScriptDirectory = "R:/code/nel/tools/build_gamedata"
	try:
		if args.preset:
			DummyUnknownName
		WorkspaceDirectory
	except NameError:
		WorkspaceDirectory = "R:/leveldesign/workspace"
	try:
		if args.preset:
			DummyUnknownName
		DatabaseDirectory
	except NameError:
		DatabaseDirectory = "R:/graphics"
	try:
		if args.preset:
			DummyUnknownName
		SoundDirectory
	except NameError:
		SoundDirectory = "R:/sound"
	try:
		if args.preset:
			DummyUnknownName
		SoundDfnDirectory
	except NameError:
		SoundDfnDirectory = "R:/sound/DFN"
	try:
		if args.preset:
			DummyUnknownName
		ExportBuildDirectory
	except NameError:
		ExportBuildDirectory = "R:/pipeline/export"
	try:
		if args.preset:
			DummyUnknownName
		InstallDirectory
	except NameError:
		InstallDirectory = "R:/pipeline/install"
	try:
		if args.preset:
			DummyUnknownName
		ClientDevDirectory
	except NameError:
		ClientDevDirectory = "R:/pipeline/client_dev"
	try:
		if args.preset:
			DummyUnknownName
		ClientDevLiveDirectory
	except NameError:
		ClientDevLiveDirectory = "R:/pipeline/client_dev_live"
	try:
		if args.preset:
			DummyUnknownName
		ClientPatchDirectory
	except NameError:
		ClientPatchDirectory = "R:/pipeline/client_patch"
	try:
		if args.preset:
			DummyUnknownName
		ClientInstallDirectory
	except NameError:
		ClientInstallDirectory = "R:/pipeline/client_install"
	try:
		if args.preset:
			DummyUnknownName
		ShardInstallDirectory
	except NameError:
		ShardInstallDirectory = "R:/pipeline/shard"
	try:
		if args.preset:
			DummyUnknownName
		ShardDevDirectory
	except NameError:
		ShardDevDirectory = "R:/pipeline/shard_dev"
	try:
		if args.preset:
			DummyUnknownName
		WorldEditInstallDirectory
	except NameError:
		WorldEditInstallDirectory = "R:/pipeline/worldedit"
	try:
		if args.preset:
			DummyUnknownName
		WorldEditorFilesDirectory
	except NameError:
		WorldEditorFilesDirectory = "R:/code/ryzom/common/data_leveldesign/leveldesign/world_editor_files"
	try:
		if args.preset:
			DummyUnknownName
		LeveldesignDirectory
	except NameError:
		LeveldesignDirectory = "R:/leveldesign"
	try:
		if args.preset:
			DummyUnknownName
		LeveldesignDfnDirectory
	except NameError:
		LeveldesignDfnDirectory = "R:/leveldesign/DFN"
	try:
		if args.preset:
			DummyUnknownName
		LeveldesignWorldDirectory
	except NameError:
		LeveldesignWorldDirectory = "R:/leveldesign/world"
	try:
		if args.preset:
			DummyUnknownName
		PrimitivesDirectory
	except NameError:
		PrimitivesDirectory = "R:/leveldesign/primitives"
	try:
		if args.preset:
			DummyUnknownName
		LeveldesignDataCommonDirectory
	except NameError:
		LeveldesignDataCommonDirectory = "R:/leveldesign/common"
	try:
		if args.preset:
			DummyUnknownName
		LeveldesignDataShardDirectory
	except NameError:
		LeveldesignDataShardDirectory = "R:/leveldesign/shard"
	try:
		if args.preset:
			DummyUnknownName
		TranslationDirectory
	except NameError:
		TranslationDirectory = "R:/leveldesign/translation"
	try:
		if args.preset:
			DummyUnknownName
		GamedevDirectory
	except NameError:
		GamedevDirectory = "R:/code/ryzom/client/data/gamedev"
	try:
		if args.preset:
			DummyUnknownName
		DataCommonDirectory
	except NameError:
		DataCommonDirectory = "R:/code/ryzom/common/data_common"
	try:
		if args.preset:
			DummyUnknownName
		DataShardDirectory
	except NameError:
		DataShardDirectory = "R:/code/ryzom/server/data_shard"
	try:
		if args.preset:
			DummyUnknownName
		ExeDllCfgDirectories
	except NameError:
		# For legacy exedll bnp only
		ExeDllCfgDirectories = [ '', '', '', 'R:/code/ryzom/client', '', '', '' ]
	try:
		if args.preset:
			DummyUnknownName
		LinuxServiceExecutableDirectories
	except NameError:
		if NeLServerDirsFv:
			LinuxServiceExecutableDirectories = [] + NeLServerDirsFv
			for i in range(len(LinuxServiceExecutableDirectories)):
				LinuxServiceExecutableDirectories[i] = LinuxServiceExecutableDirectories[i].replace('\\', '/').replace(RemapLocalTo, RemapLocalFrom)
		else:
			LinuxServiceExecutableDirectories = [ "R:/build_docker/server/bin" ]
	try:
		if args.preset:
			DummyUnknownName
		PatchmanDevDirectory
	except NameError:
		PatchmanDevDirectory = "R:/patchman/terminal_dev"
	try:
		if args.preset:
			DummyUnknownName
		PatchmanCfgAdminDirectory
	except NameError:
		PatchmanCfgAdminDirectory = "R:/patchman/admin_install"
	try:
		if args.preset:
			DummyUnknownName
		PatchmanCfgDefaultDirectory
	except NameError:
		PatchmanCfgDefaultDirectory = "R:/patchman/default"
	try:
		if args.preset:
			DummyUnknownName
		PatchmanBridgeServerDirectory
	except NameError:
		PatchmanBridgeServerDirectory = "R:/pipeline/bridge_server"
	try:
		if args.preset:
			DummyUnknownName
		SignToolExecutable
	except NameError:
		SignToolExecutable = os.getenv('RC_SIGNTOOL_EXE')
		if SignToolExecutable:
			SignToolExecutable = SignToolExecutable.replace("\\", "/")
		else:
			SignToolExecutable = "C:/Program Files/Microsoft SDKs/Windows/v6.0A/Bin/signtool.exe"
	try:
		if args.preset:
			DummyUnknownName
		SignToolSha1
	except NameError:
		SignToolSha1 = os.getenv('RC_SIGNTOOL_SHA1')
		if not SignToolSha1:
			SignToolSha1 = ""
	try:
		if args.preset:
			DummyUnknownName
		SignToolTimestamp
	except NameError:
		SignToolTimestamp = os.getenv('RC_SIGNTOOL_TIMESTAMP')
		if not SignToolTimestamp:
			SignToolTimestamp = "http://timestamp.comodoca.com/authenticode"
	try:
		if args.preset:
			DummyUnknownName
		MaxAvailable
	except NameError:
		MaxAvailable = 0
		if NeL3dsMaxExe:
			MaxDirectory = os.path.dirname(NeL3dsMaxExe)
			if os.path.isfile(os.path.join(MaxDirectory, "3dsmax.exe")) and (os.path.isfile(os.path.join(MaxDirectory, "plugins/nelexport_r.dlu")) or os.path.isfile(os.path.join(MaxDirectory, "plugins/nelexport_d.dlu")) or os.path.isfile(os.path.join(MaxDirectory, "plugins/nelexport.dlu"))):
				MaxAvailable = 1
	try:
		if args.preset:
			DummyUnknownName
		MaxDirectory
	except NameError:
		MaxDirectory = "C:/Program Files (x86)/Autodesk/3ds Max 2010"
		if NeL3dsMaxExe:
			MaxDirectory = os.path.dirname(NeL3dsMaxExe).replace("\\", "/")
	try:
		if args.preset:
			DummyUnknownName
		MaxUserDirectory
	except NameError:
		if NeL3dsMaxUserDir:
			MaxUserDirectory = NeL3dsMaxUserDir.replace("\\", "/")
		else:
			try:
				MaxUserDirectory = os.path.normpath(os.environ["LOCALAPPDATA"] + "/Autodesk/3dsMax/2010 - 32bit/enu")
			except KeyError:
				MaxAvailable = 0
				MaxUserDirectory = "C:/Users/Kaetemi/AppData/Local/Autodesk/3dsMax/2010 - 32bit/enu"
	try:
		if args.preset:
			DummyUnknownName
		MaxExecutable
	except NameError:
		MaxExecutable = "3dsmax.exe"
		if NeL3dsMaxExe:
			MaxExecutable = os.path.basename(NeL3dsMaxExe)

	printLog(log, "")
	printLog(log, "-------")
	printLog(log, "--- Setup build site")
	printLog(log, "-------")
	printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
	printLog(log, "")
	if not args.preset:
		printLog(log, "This script will set up the buildsite configuration, and create needed directories.")
		printLog(log, "To use the defaults, simply hit ENTER, else type in the new value.")
		printLog(log, "Use -- if you need to insert an empty value.")
		printLog(log, "")
		BuildQuality = int(askVar(log, "Build Quality", str(BuildQuality)))
		ToolDirectories[0] = askVar(log, "[IN] Primary Tool Directory", ToolDirectories[0]).replace("\\", "/")
		ToolDirectories[1] = askVar(log, "[IN] Secondary Tool Directory", ToolDirectories[1]).replace("\\", "/")
		ToolSuffix = askVar(log, "Tool Suffix", ToolSuffix)
		ScriptDirectory = askVar(log, "[IN] Script Directory", os.getcwd().replace("\\", "/")).replace("\\", "/")
		WorkspaceDirectory = askVar(log, "[IN] Workspace Directory", WorkspaceDirectory).replace("\\", "/")
		DatabaseDirectory = askVar(log, "[IN] Database Directory", DatabaseDirectory).replace("\\", "/")
		SoundDirectory = askVar(log, "[IN] Sound Directory", SoundDirectory).replace("\\", "/")
		SoundDfnDirectory = askVar(log, "[IN] Sound DFN Directory", SoundDfnDirectory).replace("\\", "/")
		ExportBuildDirectory = askVar(log, "[OUT] Export Build Directory", ExportBuildDirectory).replace("\\", "/")
		InstallDirectory = askVar(log, "[OUT] Install Directory", InstallDirectory).replace("\\", "/")
		ClientDevDirectory = askVar(log, "[OUT] Client Dev Directory", ClientDevDirectory).replace("\\", "/")
		ClientDevLiveDirectory = askVar(log, "[OUT] Client Dev Live Directory", ClientDevLiveDirectory).replace("\\", "/")
		ClientPatchDirectory = askVar(log, "[OUT] Client Patch Directory", ClientPatchDirectory).replace("\\", "/")
		ClientInstallDirectory = askVar(log, "[OUT] Client Install Directory", ClientInstallDirectory).replace("\\", "/")
		ShardInstallDirectory = askVar(log, "[OUT] Shard Data Install Directory", ShardInstallDirectory).replace("\\", "/")
		ShardDevDirectory = askVar(log, "[OUT] Shard Dev Directory", ShardDevDirectory).replace("\\", "/")
		WorldEditInstallDirectory = askVar(log, "[OUT] World Edit Data Install Directory", WorldEditInstallDirectory).replace("\\", "/")
		LeveldesignDirectory = askVar(log, "[IN] Leveldesign Directory", LeveldesignDirectory).replace("\\", "/")
		LeveldesignDfnDirectory = askVar(log, "[IN] Leveldesign DFN Directory", LeveldesignDfnDirectory).replace("\\", "/")
		LeveldesignWorldDirectory = askVar(log, "[IN] Leveldesign World Directory", LeveldesignWorldDirectory).replace("\\", "/")
		PrimitivesDirectory = askVar(log, "[IN] Primitives Directory", PrimitivesDirectory).replace("\\", "/")
		GamedevDirectory = askVar(log, "[IN] Gamedev Directory", GamedevDirectory).replace("\\", "/")
		DataShardDirectory = askVar(log, "[IN] Data Shard Directory", DataShardDirectory).replace("\\", "/")
		DataCommonDirectory = askVar(log, "[IN] Data Common Directory", DataCommonDirectory).replace("\\", "/")
		TranslationDirectory = askVar(log, "[IN] Translation Directory", TranslationDirectory).replace("\\", "/")
		LeveldesignDataShardDirectory = askVar(log, "[IN] Leveldesign Data Shard Directory", LeveldesignDataShardDirectory).replace("\\", "/")
		LeveldesignDataCommonDirectory = askVar(log, "[IN] Leveldesign Data Common Directory", LeveldesignDataCommonDirectory).replace("\\", "/")
		WorldEditorFilesDirectory = askVar(log, "[IN] World Editor Files Directory", WorldEditorFilesDirectory).replace("\\", "/")
		ExeDllCfgDirectories[0] = askVar(log, "[IN] Primary Windows exe/dll/cfg Directory", ExeDllCfgDirectories[0]).replace("\\", "/")
		ExeDllCfgDirectories[1] = askVar(log, "[IN] Secondary Windows exe/dll/cfg Directory", ExeDllCfgDirectories[1]).replace("\\", "/")
		ExeDllCfgDirectories[2] = askVar(log, "[IN] Tertiary Windows exe/dll/cfg Directory", ExeDllCfgDirectories[2]).replace("\\", "/")
		ExeDllCfgDirectories[3] = askVar(log, "[IN] Quaternary Windows exe/dll/cfg Directory", ExeDllCfgDirectories[3]).replace("\\", "/")
		ExeDllCfgDirectories[4] = askVar(log, "[IN] Quinary Windows exe/dll/cfg Directory", ExeDllCfgDirectories[4]).replace("\\", "/")
		ExeDllCfgDirectories[5] = askVar(log, "[IN] Senary Windows exe/dll/cfg Directory", ExeDllCfgDirectories[5]).replace("\\", "/")
		ExeDllCfgDirectories[6] = askVar(log, "[IN] Septenary Windows exe/dll/cfg Directory", ExeDllCfgDirectories[6]).replace("\\", "/")
		LinuxServiceExecutableDirectories[0] = askVar(log, "[IN] Linux Service Executable Directory", LinuxServiceExecutableDirectories[0]).replace("\\", "/")
		PatchmanDevDirectory = askVar(log, "[IN] Patchman Directory", PatchmanDevDirectory).replace("\\", "/")
		PatchmanCfgAdminDirectory = askVar(log, "[IN] Patchman Cfg Admin Directory", PatchmanCfgAdminDirectory).replace("\\", "/")
		PatchmanCfgDefaultDirectory = askVar(log, "[IN] Patchman Cfg Default Directory", PatchmanCfgDefaultDirectory).replace("\\", "/")
		PatchmanBridgeServerDirectory = askVar(log, "[OUT] Patchman Bridge Server Patch Directory", PatchmanBridgeServerDirectory).replace("\\", "/")
		SignToolExecutable = askVar(log, "Sign Tool Executable", SignToolExecutable).replace("\\", "/")
		SignToolSha1 = askVar(log, "Sign Tool Signature SHA1", SignToolSha1)
		SignToolTimestamp = askVar(log, "Sign Tool Timestamp Authority", SignToolTimestamp)
		MaxAvailable = int(askVar(log, "3dsMax Available", str(MaxAvailable)))
		if MaxAvailable:
			MaxDirectory = askVar(log, "3dsMax Directory", MaxDirectory).replace("\\", "/")
			MaxUserDirectory = askVar(log, "3dsMax User Directory", MaxUserDirectory).replace("\\", "/")
			MaxExecutable = askVar(log, "3dsMax Executable", MaxExecutable)
	buildsiteFileName = "configuration/buildsite.py"
	localBuildsiteFileName = "configuration/buildsite_local.py"
	if NeLConfigDir:
		buildsiteFileName = os.path.join(NeLConfigDir, "buildsite_" + NeLHostId + "_" + NeLPlatformId + ".py")
		localBuildsiteFileName = os.path.join(NeLConfigDir, "buildsite_" + NeLHostId + "_" + NeLPlatformId + "_local.py")
		fo = open(os.path.join(NeLConfigDir, "buildsite.py"), "w")
		fo.write("import importlib, socket, sys\n")
		fo.write("NeLHostId = socket.gethostname().lower()\n")
		fo.write("NeLPlatformId = sys.platform.lower()\n")
		fo.write("globals().update(importlib.import_module(\"buildsite_\" + NeLHostId + \"_\" + NeLPlatformId).__dict__)\n")
		fo.close()
		fo = open(os.path.join(NeLConfigDir, "buildsite_local.py"), "w")
		fo.write("import importlib, socket, sys\n")
		fo.write("NeLHostId = socket.gethostname().lower()\n")
		fo.write("NeLPlatformId = sys.platform.lower()\n")
		fo.write("globals().update(importlib.import_module(\"buildsite_\" + NeLHostId + \"_\" + NeLPlatformId + \"_local\").__dict__)\n")
		fo.close()
	if os.path.isfile(buildsiteFileName):
		os.remove(buildsiteFileName)
	sf = open(buildsiteFileName, "w")
	sf.write("#!/usr/bin/python\n")
	sf.write("# \n")
	sf.write("# \\file buildsite.py\n")
	sf.write("# \\brief Site configuration\n")
	sf.write("# \\date " + time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "\n")
	sf.write("# \\author Jan Boon (Kaetemi)\n")
	sf.write("# Python port of game data build pipeline.\n")
	sf.write("# Site configuration.\n")
	sf.write("# \n")
	sf.write("# NeL - MMORPG Framework <https://wiki.ryzom.dev/>\n")
	sf.write("# Copyright (C) 2009-2022  by authors\n")
	sf.write("# \n")
	sf.write("# This is free and unencumbered software released into the public domain.\n")
	sf.write("# \n")
	sf.write("# Anyone is free to copy, modify, publish, use, compile, sell, or\n")
	sf.write("# distribute this software, either in source code form or as a compiled\n")
	sf.write("# binary, for any purpose, commercial or non-commercial, and by any\n")
	sf.write("# means.\n")
	sf.write("# \n")
	sf.write("# In jurisdictions that recognize copyright laws, the author or authors\n")
	sf.write("# of this software dedicate any and all copyright interest in the\n")
	sf.write("# software to the public domain. We make this dedication for the benefit\n")
	sf.write("# of the public at large and to the detriment of our heirs and\n")
	sf.write("# successors. We intend this dedication to be an overt act of\n")
	sf.write("# relinquishment in perpetuity of all present and future rights to this\n")
	sf.write("# software under copyright law.\n")
	sf.write("# \n")
	sf.write("# THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n")
	sf.write("# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n")
	sf.write("# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n")
	sf.write("# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR\n")
	sf.write("# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,\n")
	sf.write("# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n")
	sf.write("# OTHER DEALINGS IN THE SOFTWARE.\n")
	sf.write("# \n")
	sf.write("\n")
	sf.write("\n")
	sf.write("# *** SITE INSTALLATION ***\n")
	sf.write("\n")
	sf.write("# Use '/' in path name, not '\'\n")
	sf.write("# Don't put '/' at the end of a directory name\n")
	sf.write("\n")
	sf.write("import os\n")
	sf.write("\n")
	sf.write("NeLToolsDirs = os.getenv(\"RC_TOOLS_DIRS\")\n")
	sf.write("if NeLToolsDirs:\n")
	sf.write("	NeLToolsDirs = NeLToolsDirs.split(os.pathsep)\n")
	sf.write("NeLPath = os.getenv(\"RC_PATH\")\n")
	sf.write("if NeLPath:\n")
	sf.write("	NeLToolsDirs = NeLToolsDirs + NeLPath.split(os.pathsep)\n")
	sf.write("\n")
	sf.write("NeLServerDirsFv = os.getenv(\"RC_SERVER_DIRS_FV\")\n")
	sf.write("if NeLServerDirsFv:\n")
	sf.write("	NeLServerDirsFv = NeLServerDirsFv.split(os.pathsep)\n")
	sf.write("\n")
	sf.write("# Quality option for this site (1 for BEST, 0 for DRAFT)\n")
	sf.write("BuildQuality = " + str(BuildQuality) + "\n")
	sf.write("\n")
	sf.write("RemapLocalFrom = \"" + str(RemapLocalFrom) + "\"\n")
	sf.write("RemapLocalTo = \"" + str(RemapLocalTo) + "\"\n")
	sf.write("\n")
	sf.write("ToolDirectories = " + str(ToolDirectories) + "\n")
	sf.write("ToolSuffix = \"" + str(ToolSuffix) + "\"\n")
	sf.write("\n")
	sf.write("if NeLToolsDirs:\n")
	sf.write("	ToolDirectories = [] + NeLToolsDirs\n")
	sf.write("	for i in range(len(ToolDirectories)):\n")
	sf.write("		ToolDirectories[i] = ToolDirectories[i].replace('\\\\', '/')\n")
	sf.write("\n")
	sf.write("# Build script directory\n")
	sf.write("ScriptDirectory = \"" + str(ScriptDirectory) + "\"\n")
	sf.write("WorkspaceDirectory = \"" + str(WorkspaceDirectory) + "\"\n")
	sf.write("\n")
	sf.write("# Data build directories\n")
	sf.write("DatabaseDirectory = \"" + str(DatabaseDirectory) + "\"\n")
	sf.write("SoundDirectory = \"" + str(SoundDirectory) + "\"\n")
	sf.write("SoundDfnDirectory = \"" + str(SoundDfnDirectory) + "\"\n")
	sf.write("ExportBuildDirectory = \"" + str(ExportBuildDirectory) + "\"\n")
	sf.write("\n")
	sf.write("# Install directories\n")
	sf.write("InstallDirectory = \"" + str(InstallDirectory) + "\"\n")
	sf.write("ClientDevDirectory = \"" + str(ClientDevDirectory) + "\"\n")
	sf.write("ClientDevLiveDirectory = \"" + str(ClientDevLiveDirectory) + "\"\n")
	sf.write("ClientPatchDirectory = \"" + str(ClientPatchDirectory) + "\"\n")
	sf.write("ClientInstallDirectory = \"" + str(ClientInstallDirectory) + "\"\n")
	sf.write("ShardInstallDirectory = \"" + str(ShardInstallDirectory) + "\"\n")
	sf.write("ShardDevDirectory = \"" + str(ShardDevDirectory) + "\"\n")
	sf.write("WorldEditInstallDirectory = \"" + str(WorldEditInstallDirectory) + "\"\n")
	sf.write("\n")
	sf.write("# Utility directories\n")
	sf.write("WorldEditorFilesDirectory = \"" + str(WorldEditorFilesDirectory) + "\"\n")
	sf.write("\n")
	sf.write("# Leveldesign directories\n")
	sf.write("LeveldesignDirectory = \"" + str(LeveldesignDirectory) + "\"\n")
	sf.write("LeveldesignDfnDirectory = \"" + str(LeveldesignDfnDirectory) + "\"\n")
	sf.write("LeveldesignWorldDirectory = \"" + str(LeveldesignWorldDirectory) + "\"\n")
	sf.write("PrimitivesDirectory = \"" + str(PrimitivesDirectory) + "\"\n")
	sf.write("LeveldesignDataCommonDirectory = \"" + str(LeveldesignDataCommonDirectory) + "\"\n")
	sf.write("LeveldesignDataShardDirectory = \"" + str(LeveldesignDataShardDirectory) + "\"\n")
	sf.write("TranslationDirectory = \"" + str(TranslationDirectory) + "\"\n")
	sf.write("\n")
	sf.write("# Misc data directories\n")
	sf.write("GamedevDirectory = \"" + str(GamedevDirectory) + "\"\n")
	sf.write("DataCommonDirectory = \"" + str(DataCommonDirectory) + "\"\n")
	sf.write("DataShardDirectory = \"" + str(DataShardDirectory) + "\"\n")
	sf.write("ExeDllCfgDirectories = " + str(ExeDllCfgDirectories) + "\n")
	sf.write("LinuxServiceExecutableDirectories = " + str(LinuxServiceExecutableDirectories) + "\n")
	sf.write("PatchmanDevDirectory = \"" + str(PatchmanDevDirectory) + "\"\n")
	sf.write("PatchmanCfgAdminDirectory = \"" + str(PatchmanCfgAdminDirectory) + "\"\n")
	sf.write("PatchmanCfgDefaultDirectory = \"" + str(PatchmanCfgDefaultDirectory) + "\"\n")
	sf.write("PatchmanBridgeServerDirectory = \"" + str(PatchmanBridgeServerDirectory) + "\"\n")
	sf.write("\n")
	sf.write("if NeLServerDirsFv:\n")
	sf.write("	LinuxServiceExecutableDirectories = [] + NeLServerDirsFv\n")
	sf.write("	for i in range(len(LinuxServiceExecutableDirectories)):\n")
	sf.write("		LinuxServiceExecutableDirectories[i] = LinuxServiceExecutableDirectories[i].replace('\\\\', '/')\n")
	sf.write("\n")
	sf.write("# Sign tool\n")
	sf.write("SignToolExecutable = \"" + str(SignToolExecutable) + "\"\n")
	sf.write("SignToolSha1 = \"" + str(SignToolSha1) + "\"\n")
	sf.write("SignToolTimestamp = \"" + str(SignToolTimestamp) + "\"\n")
	sf.write("\n")
	sf.write("# 3dsMax directives\n")
	sf.write("MaxAvailable = " + str(MaxAvailable) + "\n")
	sf.write("MaxDirectory = \"" + str(MaxDirectory) + "\"\n")
	sf.write("MaxUserDirectory = \"" + str(MaxUserDirectory) + "\"\n")
	sf.write("MaxExecutable = \"" + str(MaxExecutable) + "\"\n")
	sf.write("\n")
	sf.write("\n")
	sf.write("# end of file\n")
	sf.flush()
	sf.close()
	sf = open(localBuildsiteFileName, "w")
	sfr = open(buildsiteFileName, "r")
	for l in sfr:
		sf.write(l.replace(RemapLocalFrom + '/', RemapLocalTo + '/'))
	sf.flush()
	sfr.close()
	sf.close()

from buildsite_local import *

sys.path.append(WorkspaceDirectory)
from projects import *

NeLWorkspaceDir = None
if NeLConfigDir:
	NeLWorkspaceDir = os.path.join(NeLConfigDir, "workspace")

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Run the setup projects")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")
# For each project
for projectName in ProjectsToProcess:
	if ((args.includeproject == None or projectName in args.includeproject) and (args.excludeproject == None or not projectName in args.excludeproject)):
		printLog(log, "PROJECT " + projectName)
		if os.path.isfile(os.path.join(os.path.join(NeLWorkspaceDir, projectName), "process.py")):
			os.putenv("NELBUILDACTIVEPROJECT", os.path.abspath(os.path.join(NeLWorkspaceDir, projectName).replace("\\", "/")))
		else:
			os.putenv("NELBUILDACTIVEPROJECT", os.path.abspath(WorkspaceDirectory + "/" + projectName))
		os.chdir("processes")
		try:
			if not args.includeprocess == None:
				subprocess.call([ "python", "0_setup.py", "--includeprocess" ] + args.includeprocess)
			elif not args.excludeprocess == None:
				subprocess.call([ "python", "0_setup.py", "--excludeprocess" ] + args.excludeprocess)
			else:
				subprocess.call([ "python", "0_setup.py" ])
		except Exception, e:
			printLog(log, "<" + projectName + "> " + str(e))
		os.chdir("..")
		try:
			projectLog = open("processes/log.log", "r")
			projectLogData = projectLog.read()
			projectLog.close()
			log.write(projectLogData)
		except Exception, e:
			printLog(log, "<" + projectName + "> " + str(e))
	else:
		printLog(log, "IGNORE PROJECT " + projectName)
printLog(log, "")

# Additional directories
printLog(log, ">>> Setup additional directories <<<")
mkPath(log, ClientDevDirectory)
mkPath(log, ClientDevLiveDirectory)
mkPath(log, ClientPatchDirectory)
mkPath(log, ClientInstallDirectory)

if not args.noverify:
	printLog(log, "")
	printLog(log, "-------")
	printLog(log, "--- Verify tool paths")
	printLog(log, "-------")
	printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
	printLog(log, "")
	if MaxAvailable:
		findMax(log, MaxDirectory, MaxExecutable)
	findTool(log, ToolDirectories, TgaToDdsTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildInterfaceTool, ToolSuffix)
	findTool(log, ToolDirectories, ExecTimeoutTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildSmallbankTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildFarbankTool, ToolSuffix)
	findTool(log, ToolDirectories, ZoneDependenciesTool, ToolSuffix)
	findTool(log, ToolDirectories, ZoneWelderTool, ToolSuffix)
	findTool(log, ToolDirectories, ZoneElevationTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildRbankTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildIndoorRbankTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildIgBoxesTool, ToolSuffix)
	findTool(log, ToolDirectories, GetNeighborsTool, ToolSuffix)
	findTool(log, ToolDirectories, ZoneLighterTool, ToolSuffix)
	findTool(log, ToolDirectories, ZoneIgLighterTool, ToolSuffix)
	findTool(log, ToolDirectories, IgLighterTool, ToolSuffix)
	findTool(log, ToolDirectories, AnimBuilderTool, ToolSuffix)
	findTool(log, ToolDirectories, TileEditTool, ToolSuffix)
	# findTool(log, ToolDirectories, BuildImagesetTool, ToolSuffix) # kaetemi stuff, ignore this
	findTool(log, ToolDirectories, MakeSheetIdTool, ToolSuffix)
	# findTool(log, ToolDirectories, BuildSheetsTool, ToolSuffix) # kaetemi stuff, ignore this
	# findTool(log, ToolDirectories, BuildSoundTool, ToolSuffix) # kaetemi stuff, ignore this
	# findTool(log, ToolDirectories, BuildSoundTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildSoundbankTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildSamplebankTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildCoarseMeshTool, ToolSuffix)
	findTool(log, ToolDirectories, LightmapOptimizerTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildClodtexTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildShadowSkinTool, ToolSuffix)
	findTool(log, ToolDirectories, PanoplyMakerTool, ToolSuffix)
	findTool(log, ToolDirectories, HlsBankMakerTool, ToolSuffix)
	findTool(log, ToolDirectories, LandExportTool, ToolSuffix)
	findTool(log, ToolDirectories, PrimExportTool, ToolSuffix)
	findTool(log, ToolDirectories, IgElevationTool, ToolSuffix)
	findTool(log, ToolDirectories, IgAddTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildClodBankTool, ToolSuffix)
	findTool(log, ToolDirectories, SheetsPackerTool, ToolSuffix)
	findTool(log, ToolDirectories, BnpMakeTool, ToolSuffix)
	findTool(log, ToolDirectories, AiBuildWmapTool, ToolSuffix)
	findTool(log, ToolDirectories, TgaCutTool, ToolSuffix)
	findTool(log, ToolDirectories, PatchGenTool, ToolSuffix)
	findTool(log, ToolDirectories, TranslationToolsTool, ToolSuffix)
	findTool(log, ToolDirectories, BuildWorldPackedColTool, ToolSuffix)
	findTool(log, ToolDirectories, R2IslandsTexturesTool, ToolSuffix)
	findTool(log, ToolDirectories, PatchmanServiceTool, ToolSuffix)

log.close()
if os.path.isfile("0_setup.log"):
	os.remove("0_setup.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_setup.log")
shutil.move("log.log", "0_setup.log")
