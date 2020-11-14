#!/usr/bin/python
# 
# \file d1_client_patch.py
# \brief Install to client patch
# \date 2009-02-18 16:19GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Install to client patch
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

import time, sys, os, shutil, subprocess, distutils.dir_util, argparse
sys.path.append("configuration")

parser = argparse.ArgumentParser(description='Ryzom Core - Build Gamedata - Client Patch')
parser.add_argument('--bnponly', '-bo', action='store_true')
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
printLog(log, "--- Install to client patch")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
BnpMake = findTool(log, ToolDirectories, BnpMakeTool, ToolSuffix)
PatchGen = findTool(log, ToolDirectories, PatchGenTool, ToolSuffix)
printLog(log, "")

# Find **** HARDCODED **** WINDOWS **** tools ... TODO: fix patch_gen tool !!!
Lzma = findFileMultiDir(log, ToolDirectories + WindowsExeDllCfgDirectories, "lzma.exe")
printLog(log, "LZMA " + Lzma)
XDelta = findFileMultiDir(log, ToolDirectories + WindowsExeDllCfgDirectories, "xdelta.exe")
printLog(log, "XDELTA " + XDelta)
printLog(log, "")

if BnpMake == "":
	toolLogFail(log, BnpMakeTool, ToolSuffix)
elif PatchGen == "" and not args.bnponly:
	toolLogFail(log, PatchGenTool, ToolSuffix)
elif Lzma == "" and not args.bnponly:
	toolLogFail(log, "LZMA", ToolSuffix)
elif XDelta == "" and not args.bnponly:
	toolLogFail(log, "XDELTA", ToolSuffix)
elif os.path.dirname(Lzma) != os.path.dirname(XDelta):
	printLog(log, "FAIL lzma.exe and xdelta.exe must be in the same directory")
else:
	mkPath(log, ClientPatchDirectory)
	if not args.bnponly:
		productXml = ClientPatchDirectory + "/" + ProductName + ".xml"
		if not os.path.isfile(productXml):
			printLog(log, ">>> Create new product <<<")
			subprocess.call([ PatchGen, "createNewProduct", productXml ])
		printLog(log, "")
		printLog(log, ">>> Rewrite " + ProductName + ".xml <<<") # because we know better.
		shutil.move(productXml, productXml + ".old")
		oldCfg = open(productXml + ".old", "r")
		cfg = open(productXml, "w")
		inCategories = 0
		for line in oldCfg:
			if not inCategories:
				if line.strip() == "<_Categories>":
					inCategories = 1
					cfg.write("\t<_Categories>\n")
					for category in InstallClientData:
						cfg.write("\t\t<_Category>\n")
						cfg.write("\t\t\t<_Name type=\"STRING\" value=\"" + category["Name"] + "\"/>\n")
						if category["UnpackTo"] != None:
							if category["UnpackTo"] != "":
								cfg.write("\t\t\t<_UnpackTo type=\"STRING\" value=\"./" + category["UnpackTo"] + "/\"/>\n")
							else:
								cfg.write("\t\t\t<_UnpackTo type=\"SINT32\" value=\"./\"/>\n")
						cfg.write("\t\t\t<_IsOptional type=\"SINT32\" value=\"" + str(category["IsOptional"]) + "\"/>\n")
						cfg.write("\t\t\t<_IsIncremental type=\"SINT32\" value=\"" + str(category["IsIncremental"]) + "\"/>\n")
						for package in category["Packages"]:
							if (len(package[1]) > 0):
								cfg.write("\t\t\t<_Files type=\"STRING\" value=\"" + package[1][0] + "\"/>\n")
							else:
								cfg.write("\t\t\t<_Files type=\"STRING\" value=\"" + package[0] + ".bnp\"/>\n")
						for ref in category["Refs"]:
							cfg.write("\t\t\t<_Files type=\"STRING\" value=\"" + ref + "_.ref\"/>\n")
						cfg.write("\t\t</_Category>\n")
					cfg.write("\t</_Categories>\n")
				else:
					cfg.write(line)
			else:
				if line.strip() == "</_Categories>":
					inCategories = 0
		oldCfg.close()
		cfg.close()
		os.remove(productXml + ".old")
	printLog(log, "")
	printLog(log, ">>> Make bnp <<<")
	targetPath = ClientPatchDirectory + "/bnp"
	mkPath(log, targetPath)
	for category in InstallClientData:
		for package in category["Packages"]:
			printLog(log, "PACKAGE " + package[0])
			sourcePath = InstallDirectory + "/" + package[0]
			mkPath(log, sourcePath)
			targetBnp = targetPath + "/" + package[0] + ".bnp"
			if (len(package[1]) > 0):
				targetBnp = targetPath + "/" + package[1][0]
				printLog(log, "TARGET " + package[1][0])
			needUpdateBnp = 1
			if (len(package) > 2):
				needUpdateBnp = needUpdate(log, sourcePath + "/" + package[2], targetBnp)
			else:
				needUpdateBnp = needUpdateDirNoSubdirFile(log, sourcePath, targetBnp)
			if (needUpdateBnp):
				printLog(log, "BNP " + targetBnp)
				subprocess.call([ BnpMake, "-p", sourcePath, "-o", targetBnp ] + package[1][1:])
			else:
				printLog(log, "SKIP " + targetBnp)
	printLog(log, "")
	if not args.bnponly:
		printLog(log, ">>> Update product <<<")
		cwDir = os.getcwd().replace("\\", "/")
		toolDir = os.path.dirname(Lzma).replace("\\", "/")
		os.chdir(toolDir)
		subprocess.call([ PatchGen, "updateProduct", productXml ])
		os.chdir(cwDir)
		printLog(log, "")


log.close()
if os.path.isfile("d1_client_patch.log"):
	os.remove("d1_client_patch.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_client_patch.log")
shutil.move("log.log", "d1_client_patch.log")
