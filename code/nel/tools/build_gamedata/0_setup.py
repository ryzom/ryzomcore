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
sys.path.append("configuration")

if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")
from scripts import *
from buildsite import *
from tools import *

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
ClientDataDirectory = askVar(log, "Client Data Directory", ClientDataDirectory).replace("\\", "/")
LeveldesignDirectory = askVar(log, "Leveldesign Directory", LeveldesignDirectory).replace("\\", "/")
LeveldesignDfnDirectory = askVar(log, "Leveldesign DFN Directory", LeveldesignDfnDirectory).replace("\\", "/")
LeveldesignWorldDirectory = askVar(log, "Leveldesign World Directory", LeveldesignWorldDirectory).replace("\\", "/")
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
sf.write("# Client data install directory (client/data)\n")
sf.write("ClientDataDirectory = \"" + str(ClientDataDirectory) + "\"\n")
sf.write("\n")
sf.write("# TODO: NETWORK RECONNECT NOT IMPLEMENTED :)\n")
sf.write("\n")
sf.write("# Leveldesign directories\n")
sf.write("LeveldesignDirectory = \"" + str(LeveldesignDirectory) + "\"\n")
sf.write("LeveldesignDfnDirectory = \"" + str(LeveldesignDfnDirectory) + "\"\n")
sf.write("LeveldesignWorldDirectory = \"" + str(LeveldesignWorldDirectory) + "\"\n")
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
	os.putenv("NELBUILDACTIVEPROJECT", os.path.abspath(WorkspaceDirectory + "/" + projectName))
	os.chdir("processes")
	try:
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
printLog(log, "")

log.close()
if os.path.isfile("0_setup.log"):
	os.remove("0_setup.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_setup.log")
shutil.move("log.log", "0_setup.log")
