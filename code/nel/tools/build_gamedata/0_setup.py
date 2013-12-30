#!/usr/bin/python
# 
# \file 0_setup.py
# \brief Run all setup processes
# \date 2009-02-18 15:28GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Run all setup processes
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

import time, sys, os, shutil, subprocess, distutils.dir_util, argparse
sys.path.append("configuration")

parser = argparse.ArgumentParser(description='Ryzom Core - Build Gamedata - Setup')
parser.add_argument('--noconf', '-nc', action='store_true')
parser.add_argument('--noverify', '-nv', action='store_true')
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

if not args.noconf:
	try:
		BuildQuality
	except NameError:
		BuildQuality = 1
	try:
		ToolDirectories
	except NameError:
		ToolDirectories = [ 'R:/build/dev/bin/Release', 'D:/libraries/external/bin' ]
	try:
		ToolSuffix
	except NameError:
		ToolSuffix = ".exe"
	try:
		ScriptDirectory
	except NameError:
		ScriptDirectory = "R:/code/nel/tools/build_gamedata"
	try:
		WorkspaceDirectory
	except NameError:
		WorkspaceDirectory = "R:/code/ryzom/tools/build_gamedata/workspace"
	try:
		DatabaseDirectory
	except NameError:
		DatabaseDirectory = "W:/database"
	try:
		ExportBuildDirectory
	except NameError:
		ExportBuildDirectory = "W:/export"
	try:
		InstallDirectory
	except NameError:
		InstallDirectory = "W:/install"
	try:
		DataShardDirectory
	except NameError:
		DataShardDirectory = "R:/code/ryzom/server/data_shard"
	try:
		ClientDevDirectory
	except NameError:
		ClientDevDirectory = "W:/client_dev"
	try:
		ClientPatchDirectory
	except NameError:
		ClientPatchDirectory = "W:/client_patch"
	try:
		ClientInstallDirectory
	except NameError:
		ClientInstallDirectory = "W:/client_install"
	try:
		LeveldesignDirectory
	except NameError:
		LeveldesignDirectory = "L:/leveldesign"
	try:
		LeveldesignDfnDirectory
	except NameError:
		LeveldesignDfnDirectory = "L:/leveldesign/dfn"
	try:
		LeveldesignWorldDirectory
	except NameError:
		LeveldesignWorldDirectory = "L:/leveldesign/world"
	try:
		PrimitivesDirectory
	except NameError:
		PrimitivesDirectory = "L:/primitives"
	try:
		GamedevDirectory
	except NameError:
		GamedevDirectory = "R:/code/ryzom/client/data/gamedev"
	try:
		DataCommonDirectory
	except NameError:
		DataCommonDirectory = "R:/code/ryzom/common/data_common"
	try:
		WindowsExeDllCfgDirectories
	except NameError:
		WindowsExeDllCfgDirectories = [ 'C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/redist/x86', 'D:/libraries/external/bin', 'R:/build/dev/bin/Release', 'R:/code/ryzom/client', 'R:/code/nel/lib', 'R:/code/ryzom/bin', 'R:/code/ryzom/tools/client/client_config/bin' ]
	try:
		MaxAvailable
	except NameError:
		MaxAvailable = 1
	try:
		MaxDirectory
	except NameError:
		MaxDirectory = "C:/Program Files (x86)/Autodesk/3ds Max 2010"
	try:
		MaxUserDirectory
	except NameError:
		import os
		MaxUserDirectory = os.path.normpath(os.environ["LOCALAPPDATA"] + "/Autodesk/3dsMax/2010 - 32bit/enu")
	try:
		MaxExecutable
	except NameError:
		MaxExecutable = "3dsmax.exe"

	printLog(log, "")
	printLog(log, "-------")
	printLog(log, "--- Setup build site")
	printLog(log, "-------")
	printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
	printLog(log, "")
	printLog(log, "This script will set up the buildsite configuration, and create needed directories.")
	printLog(log, "To use the defaults, simply hit ENTER, else type in the new value.")
	printLog(log, "Use -- if you need to insert an empty value.")
	printLog(log, "")
	BuildQuality = int(askVar(log, "Build Quality", str(BuildQuality)))
	ToolDirectories[0] = askVar(log, "Primary Tool Directory", ToolDirectories[0]).replace("\\", "/")
	ToolDirectories[1] = askVar(log, "Secondary Tool Directory", ToolDirectories[1]).replace("\\", "/")
	ToolSuffix = askVar(log, "Tool Suffix", ToolSuffix)
	ScriptDirectory = askVar(log, "Script Directory", os.getcwd().replace("\\", "/")).replace("\\", "/")
	WorkspaceDirectory = askVar(log, "Workspace Directory", WorkspaceDirectory).replace("\\", "/")
	DatabaseDirectory = askVar(log, "Database Directory", DatabaseDirectory).replace("\\", "/")
	ExportBuildDirectory = askVar(log, "Export Build Directory", ExportBuildDirectory).replace("\\", "/")
	InstallDirectory = askVar(log, "Install Directory", InstallDirectory).replace("\\", "/")
	DataShardDirectory = askVar(log, "Data Shard Directory", DataShardDirectory).replace("\\", "/")
	ClientDevDirectory = askVar(log, "Client Dev Directory", ClientDevDirectory).replace("\\", "/")
	ClientPatchDirectory = askVar(log, "Client Patch Directory", ClientPatchDirectory).replace("\\", "/")
	ClientInstallDirectory = askVar(log, "Client Install Directory", ClientInstallDirectory).replace("\\", "/")
	LeveldesignDirectory = askVar(log, "Leveldesign Directory", LeveldesignDirectory).replace("\\", "/")
	LeveldesignDfnDirectory = askVar(log, "Leveldesign DFN Directory", LeveldesignDfnDirectory).replace("\\", "/")
	LeveldesignWorldDirectory = askVar(log, "Leveldesign World Directory", LeveldesignWorldDirectory).replace("\\", "/")
	PrimitivesDirectory = askVar(log, "Primitives Directory", PrimitivesDirectory).replace("\\", "/")
	GamedevDirectory = askVar(log, "Gamedev Directory", GamedevDirectory).replace("\\", "/")
	DataCommonDirectory = askVar(log, "Data Common Directory", DataCommonDirectory).replace("\\", "/")
	WindowsExeDllCfgDirectories[0] = askVar(log, "Primary Windows exe/dll/cfg Directory", WindowsExeDllCfgDirectories[0]).replace("\\", "/")
	WindowsExeDllCfgDirectories[1] = askVar(log, "Secondary Windows exe/dll/cfg Directory", WindowsExeDllCfgDirectories[1]).replace("\\", "/")
	WindowsExeDllCfgDirectories[2] = askVar(log, "Tertiary Windows exe/dll/cfg Directory", WindowsExeDllCfgDirectories[2]).replace("\\", "/")
	WindowsExeDllCfgDirectories[3] = askVar(log, "Quaternary Windows exe/dll/cfg Directory", WindowsExeDllCfgDirectories[3]).replace("\\", "/")
	WindowsExeDllCfgDirectories[4] = askVar(log, "Quinary Windows exe/dll/cfg Directory", WindowsExeDllCfgDirectories[4]).replace("\\", "/")
	WindowsExeDllCfgDirectories[5] = askVar(log, "Senary Windows exe/dll/cfg Directory", WindowsExeDllCfgDirectories[5]).replace("\\", "/")
	WindowsExeDllCfgDirectories[6] = askVar(log, "Septenary Windows exe/dll/cfg Directory", WindowsExeDllCfgDirectories[6]).replace("\\", "/")
	MaxAvailable = int(askVar(log, "3dsMax Available", str(MaxAvailable)))
	if MaxAvailable:
		MaxDirectory = askVar(log, "3dsMax Directory", MaxDirectory).replace("\\", "/")
		MaxUserDirectory = askVar(log, "3dsMax User Directory", MaxUserDirectory).replace("\\", "/")
		MaxExecutable = askVar(log, "3dsMax Executable", MaxExecutable)
	if os.path.isfile("configuration/buildsite.py"):
		os.remove("configuration/buildsite.py")
	sf = open("configuration/buildsite.py", "w")
	sf.write("#!/usr/bin/python\n")
	sf.write("# \n")
	sf.write("# \\file site.py\n")
	sf.write("# \\brief Site configuration\n")
	sf.write("# \\date " + time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "\n")
	sf.write("# \\author Jan Boon (Kaetemi)\n")
	sf.write("# Python port of game data build pipeline.\n")
	sf.write("# Site configuration.\n")
	sf.write("# \n")
	sf.write("# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>\n")
	sf.write("# Copyright (C) 2010  Winch Gate Property Limited\n")
	sf.write("# \n")
	sf.write("# This program is free software: you can redistribute it and/or modify\n")
	sf.write("# it under the terms of the GNU Affero General Public License as\n")
	sf.write("# published by the Free Software Foundation, either version 3 of the\n")
	sf.write("# License, or (at your option) any later version.\n")
	sf.write("# \n")
	sf.write("# This program is distributed in the hope that it will be useful,\n")
	sf.write("# but WITHOUT ANY WARRANTY; without even the implied warranty of\n")
	sf.write("# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n")
	sf.write("# GNU Affero General Public License for more details.\n")
	sf.write("# \n")
	sf.write("# You should have received a copy of the GNU Affero General Public License\n")
	sf.write("# along with this program.  If not, see <http://www.gnu.org/licenses/>.\n")
	sf.write("# \n")
	sf.write("\n")
	sf.write("\n")
	sf.write("# *** SITE INSTALLATION ***\n")
	sf.write("\n")
	sf.write("# Use '/' in path name, not '\'\n")
	sf.write("# Don't put '/' at the end of a directory name\n")
	sf.write("\n")
	sf.write("\n")
	sf.write("# Quality option for this site (1 for BEST, 0 for DRAFT)\n")
	sf.write("BuildQuality = " + str(BuildQuality) + "\n")
	sf.write("\n")
	sf.write("ToolDirectories = " + str(ToolDirectories) + "\n")
	sf.write("ToolSuffix = \"" + str(ToolSuffix) + "\"\n")
	sf.write("\n")
	sf.write("# Build script directory\n")
	sf.write("ScriptDirectory = \"" + str(ScriptDirectory) + "\"\n")
	sf.write("WorkspaceDirectory = \"" + str(WorkspaceDirectory) + "\"\n")
	sf.write("\n")
	sf.write("# Data build directories\n")
	sf.write("DatabaseDirectory = \"" + str(DatabaseDirectory) + "\"\n")
	sf.write("ExportBuildDirectory = \"" + str(ExportBuildDirectory) + "\"\n")
	sf.write("\n")
	sf.write("# Install directories\n")
	sf.write("InstallDirectory = \"" + str(InstallDirectory) + "\"\n")
	sf.write("DataShardDirectory = \"" + str(DataShardDirectory) + "\"\n")
	sf.write("ClientDevDirectory = \"" + str(ClientDevDirectory) + "\"\n")
	sf.write("ClientPatchDirectory = \"" + str(ClientPatchDirectory) + "\"\n")
	sf.write("ClientInstallDirectory = \"" + str(ClientInstallDirectory) + "\"\n")
	sf.write("\n")
	sf.write("# TODO: NETWORK RECONNECT NOT IMPLEMENTED :)\n")
	sf.write("\n")
	sf.write("# Leveldesign directories\n")
	sf.write("LeveldesignDirectory = \"" + str(LeveldesignDirectory) + "\"\n")
	sf.write("LeveldesignDfnDirectory = \"" + str(LeveldesignDfnDirectory) + "\"\n")
	sf.write("LeveldesignWorldDirectory = \"" + str(LeveldesignWorldDirectory) + "\"\n")
	sf.write("PrimitivesDirectory = \"" + str(PrimitivesDirectory) + "\"\n")
	sf.write("\n")
	sf.write("# Misc data directories\n")
	sf.write("GamedevDirectory = \"" + str(GamedevDirectory) + "\"\n")
	sf.write("DataCommonDirectory = \"" + str(DataCommonDirectory) + "\"\n")
	sf.write("WindowsExeDllCfgDirectories = " + str(WindowsExeDllCfgDirectories) + "\n")
	sf.write("\n")
	sf.write("# 3dsMax directives\n")
	sf.write("MaxAvailable = " + str(MaxAvailable) + "\n")
	sf.write("MaxDirectory = \"" + str(MaxDirectory) + "\"\n")
	sf.write("MaxUserDirectory = \"" + str(MaxUserDirectory) + "\"\n")
	sf.write("MaxExecutable = \"" + str(MaxExecutable) + "\"\n")
	sf.write("\n")
	sf.write("\n")
	sf.write("# end of file\n")
	sf.close()

sys.path.append(WorkspaceDirectory)
from projects import *

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

log.close()
if os.path.isfile("0_setup.log"):
	os.remove("0_setup.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_setup.log")
shutil.move("log.log", "0_setup.log")
