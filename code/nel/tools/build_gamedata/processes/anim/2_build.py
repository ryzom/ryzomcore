#!/usr/bin/python
# 
# \file 2_build.py
# \brief Build anim
# \date 2009-03-10 13:13GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Build anim
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
printLog(log, "--- Build anim")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
AnimBuilder = findTool(log, ToolDirectories, AnimBuilderTool, ToolSuffix)
printLog(log, "")

# For each anim directory
printLog(log, ">>> Build anim <<<")
if AnimBuilder == "":
	toolLogFail(log, AnimBuilderTool, ToolSuffix)
else:
	srcDir = ExportBuildDirectory + "/" + AnimExportDirectory
	mkPath(log, srcDir)
	destDir = ExportBuildDirectory + "/" + AnimBuildDirectory
	mkPath(log, destDir)
	if DoOptimizeAnimations:
		printLog(log, ">>> Optimizing animations <<<")
		subprocess.call([ AnimBuilder, srcDir, destDir, ActiveProjectDirectory + "/anim_builder.cfg" ])
	else:
		printLog(log, ">>> Not optimizing animations <<<")
		copyFilesNoTreeIfNeeded(log, srcDir, destDir)
printLog(log, "")

log.close()


# end of file
