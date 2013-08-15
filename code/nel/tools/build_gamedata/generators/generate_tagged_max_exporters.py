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

import time, sys, os, shutil, subprocess, distutils.dir_util
sys.path.append("../configuration")
if os.path.isfile("generate_tagged_max_exporters.log"):
	os.remove("generate_tagged_max_exporters.log")
log = open("generate_tagged_max_exporters.log", "w")
from scripts import *
from buildsite import *
from tools import *

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Generate tagged max exporters")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

templateDir = os.getcwd().replace("\\", "/") + "/tagged_max_exporter_template"
mkPath(log, templateDir)
scriptDir = os.getcwd().replace("\\", "/") + "/max_exporter_scripts"
mkPath(log, scriptDir)

def processLine(line, processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp):
	newline = line.replace("%PreGenWarning%", "WARNING : this is a generated file, don't change it !")
	newline = newline.replace("%PreGenDateTimeStamp%", dateTimeStamp)
	newline = newline.replace("%PreGenProcessName%", processName)
	newline = newline.replace("%PreGenFileExtension%", fileExtension)
	newline = newline.replace("%PreGenSourceDirectoriesVariable%", sourceDirectoriesVariable)
	newline = newline.replace("%PreGenExportDirectoryVariable%", exportDirectoryVariable)
	newline = newline.replace("%PreGenTagExportDirectoryVariable%", tagExportDirectoryVariable)
	newline = newline.replace("%PreGenInstallDirectoryVariable%", clientDirectoryVariable)
	return newline

def generateTaggedMaxExporterFile(sourceFile, destFile, writeMode, processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp):
	srcf = open(sourceFile, "r")
	dstf = open(destFile, writeMode)
	printLog(log, "WRITE " + destFile + " " + writeMode)
	for line in srcf:
		dstf.write(processLine(line, processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp))
	dstf.close()
	srcf.close()

def generateTaggedMaxScriptFile(processDir, processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp):
	maxscriptDir = processDir + "/maxscript"
	mkPath(log, maxscriptDir)
	generateTaggedMaxExporterFile(templateDir + "/export_header.ms", maxscriptDir + "/" + fileExtension + "_export.ms", "w", processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)
	generateTaggedMaxExporterFile(scriptDir + "/" + fileExtension + ".ms", maxscriptDir + "/" + fileExtension + "_export.ms", "a", processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)
	generateTaggedMaxExporterFile(templateDir + "/export_footer.ms", maxscriptDir + "/" + fileExtension + "_export.ms", "a", processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)

def generateTaggedMaxScript(processName, fileExtension):
	dateTimeStamp = time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time()))
	processDir = ScriptDirectory + "/processes/" + processName
	mkPath(log, processDir)
	
	generateTaggedMaxScriptFile(processDir, processName, fileExtension, "_invalid", "_invalid", "_invalid", "_invalid", dateTimeStamp)

def generateTaggedMaxExporter(processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable):
	dateTimeStamp = time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time()))
	processDir = ScriptDirectory + "/processes/" + processName
	mkPath(log, processDir)
	
	if not os.path.isfile(processDir + "/0_setup.py"):
		generateTaggedMaxExporterFile(templateDir + "/0_setup.py", processDir + "/0_setup.py", "w", processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)
	
	generateTaggedMaxExporterFile(templateDir + "/1_export_header.py", processDir + "/1_export.py", "w", processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)
	generateTaggedMaxExporterFile(scriptDir + "/" + fileExtension + ".py", processDir + "/1_export.py", "a", processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)
	generateTaggedMaxExporterFile(templateDir + "/1_export_footer.py", processDir + "/1_export.py", "a", processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)
	
	if not os.path.isfile(processDir + "/2_build.py"):
		generateTaggedMaxExporterFile(templateDir + "/2_build.py", processDir + "/2_build.py", "w", processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)
	if not os.path.isfile(processDir + "/3_install.py"):
		generateTaggedMaxExporterFile(templateDir + "/3_install.py", processDir + "/3_install.py", "w", processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)
	
	generateTaggedMaxScriptFile(processDir, processName, fileExtension, sourceDirectoriesVariable, exportDirectoryVariable, tagExportDirectoryVariable, clientDirectoryVariable, dateTimeStamp)



generateTaggedMaxExporter("pacs_prim", "pacs_prim", "PacsPrimSourceDirectories", "PacsPrimExportDirectory", "PacsPrimTagExportDirectory", "PacsPrimInstallDirectory")

generateTaggedMaxExporter("clodbank", "clod", "ClodSourceDirectories", "ClodExportDirectory", "ClodTagExportDirectory", "ClodInstallDirectory")

generateTaggedMaxScript("ig", "ig")

generateTaggedMaxExporter("rbank", "cmb", "RBankCmbSourceDirectories", "RBankCmbExportDirectory", "RBankCmbTagExportDirectory", "PacsInstallDirectory")

generateTaggedMaxExporter("veget", "veget", "VegetSourceDirectories", "VegetExportDirectory", "VegetTagExportDirectory", "VegetInstallDirectory")

generateTaggedMaxScript("shape", "shape")

generateTaggedMaxExporter("anim", "anim", "AnimSourceDirectories", "AnimExportDirectory", "AnimTagExportDirectory", "AnimInstallDirectory")



printLog(log, "")
log.close()
