#!/usr/bin/python
# 
# \file 0_setup.py
# \brief setup ai_wmap
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Setup ai_wmap
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
printLog(log, "--- Setup ai_wmap")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Setup build directories
printLog(log, ">>> Setup build directories <<<")
mkPath(log, ExportBuildDirectory + "/" + RbankOutputBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + AiWmapBuildDirectory)
mkPath(log, ExportBuildDirectory + "/" + AiWmapBuildTagDirectory)

# Setup lookup directories
printLog(log, ">>> Setup lookup directories <<<")
for dir in IgLookupDirectories:
	mkPath(log, ExportBuildDirectory + "/" + dir)
for dir in PacsPrimLookupDirectories:
	mkPath(log, ExportBuildDirectory + "/" + dir)

# Setup client directories
printLog(log, ">>> Setup install directories <<<")
mkPath(log, InstallDirectory + "/" + AiWmapInstallDirectory)

# Setup client directories
printLog(log, ">>> Setup configuration <<<")
mkPath(log, InstallDirectory + "/" + AiWmapInstallDirectory)
mkPath(log, ActiveProjectDirectory + "/generated")
cfg = open(ActiveProjectDirectory + "/generated/ai_build_wmap.cfg", "w")
cfg.write("\n")
cfg.write("// AI BUILD WMAP CONFIGURATION\n")
cfg.write("\n")
cfg.write("Paths = {\n")
for dir in IgLookupDirectories:
	cfg.write("\t\"" + ExportBuildDirectory + "/" + dir + "\", \n")
cfg.write("\t\"" + ExportBuildDirectory + "/" + RbankOutputBuildDirectory + "\", \n")
cfg.write("\t\"" + LeveldesignDirectory + "\", \n")
cfg.write("};\n")
cfg.write("\n")
cfg.write("NoRecursePaths = { };\n")
cfg.write("\n")
cfg.write("PacsPrimPaths = {\n")
for dir in PacsPrimLookupDirectories:
	cfg.write("\t\"" + ExportBuildDirectory + "/" + dir + "\", \n")
cfg.write("};\n")
cfg.write("\n")
cfg.write("OutputPath = \"" + ExportBuildDirectory + "/" + AiWmapBuildDirectory + "\";\n")
cfg.write("\n")
cfg.write("Commands = {\n")
cfg.write("\t\"Verbose " + str(AiWmapVerbose) + "\", \n")
for startPoint in AiWmapStartPoints:
	cfg.write("\t\"setStartPoint " + startPoint + "\", \n")
cfg.write("};\n")
cfg.write("\n")
cfg.close()

log.close()


# end of file
