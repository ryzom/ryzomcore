#!/usr/bin/python
# 
# \file 1_export.py
# \brief Export lib
# \date 2022-04-29 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Export lib
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

import time, sys, os, shutil, subprocess, json
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
printLog(log, "--- Export lib")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

def listModuleNames(res, deps):
	for dep in deps["Dependencies"]:
		moduleName = dep["ModuleName"]
		if not moduleName.startswith("api-ms") and not "system32" in dep["Filepath"].lower() and not "qt" in moduleName.lower():
			res[dep["ModuleName"]] = True
			listModuleNames(res, dep)

def findDependencies(moduleNames):
	res = []
	for name in moduleNames.copy():
		for dir in LibSourceDirectories:
			path = os.path.join(dir, name)
			if os.path.isfile(path):
				res += [ path ]
				moduleNames[name] = path
				break
	return res

ProcessedModules = {}

def processModule(file, filePath):
	global ProcessedModules
	# print(file)
	# print(filePath)
	if file in ProcessedModules:
		return
	ProcessedModules[file] = True
	tagPath = ExportBuildDirectory + "/" + LibTagDirectory + "/" + file + ".tag"
	if not needUpdate(log, filePath, tagPath):
		printLog(log, "SKIP " + filePath)
		return
	printLog(log, "DEPENDENCIES " + filePath)
	output = None
	cmd = [ "Dependencies.exe", "-json", "-chain", "-depth", "1", filePath ]
	try:
		output = bytes.decode(subprocess.check_output(cmd))
	except subprocess.CalledProcessError as e:
		printLog(log, "FAILED")
		printLog(log, str(e))
	except OSError as e:
		printLog(log, "FAILED")
		printLog(log, str(e))
	# print(output)
	deps = json.loads(output)
	# print(deps)
	moduleNames = {}
	listModuleNames(moduleNames, deps["Root"])
	# print(moduleNames)
	depPaths = findDependencies(moduleNames)
	# print(depPaths)
	for name in moduleNames:
		srcPath = moduleNames[name]
		if not srcPath in depPaths:
			continue
		dstPath = ExportBuildDirectory + "/" + LibExeDllDirectory + "/" + name
		# print(srcPath)
		# print(dstPath)
		copyFileIfNeeded(log, srcPath, dstPath)
		processModule(name, srcPath)
	tagFile = open(tagPath, "w")
	tagFile.write(time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())) + "\n")
	tagFile.close()

Dependencies = findFileMultiDir(log, ToolDirectories, "Dependencies.exe")
printLog(log, "DEPENDENCIES " + Dependencies)
if Dependencies == "":
	toolLogFail(log, "Dependencies", ToolSuffix)
else:
	mkPath(log, ExportBuildDirectory + "/" + LibExeDllDirectory)
	mkPath(log, ExportBuildDirectory + "/" + LibTagDirectory)
	origPath = os.getenv('PATH')
	os.putenv('PATH', os.pathsep.join(LibSourceDirectories) + os.pathsep + origPath)
	for file in ExeDllFiles:
		# printLog(log, file)
		filePath = findFileMultiDir(log, ExeDllCfgDirectories, file)
		if (filePath != ""):
			processModule(file, filePath)
	os.putenv('PATH', origPath)

log.close()


# end of file
