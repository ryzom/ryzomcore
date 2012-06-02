#!/usr/bin/python
# 
# \file 0_setup.py
# \brief setup copy
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Setup copy
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
printLog(log, "--- Setup copy")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Setup source directories
printLog(log, ">>> Setup source directories <<<")
for dir in CopyDirectSourceDirectories:
	mkPath(log, dir)
for file in CopyDirectSourceFiles:
	mkPath(log, os.path.dirname(file))
for dir in CopyLeveldesignSourceDirectories:
	mkPath(log, LeveldesignDirectory + "/" + dir)
for file in CopyLeveldesignSourceFiles:
	mkPath(log, os.path.dirname(LeveldesignDirectory + "/" + file))
for dir in CopyLeveldesignWorldSourceDirectories:
	mkPath(log, LeveldesignWorldDirectory + "/" + dir)
for file in CopyLeveldesignWorldSourceFiles:
	mkPath(log, os.path.dirname(LeveldesignWorldDirectory + "/" + file))
for dir in CopyLeveldesignDfnSourceDirectories:
	mkPath(log, LeveldesignDfnDirectory + "/" + dir)
for file in CopyLeveldesignDfnSourceFiles:
	mkPath(log, os.path.dirname(LeveldesignDfnDirectory + "/" + file))
for dir in CopyDatabaseSourceDirectories:
	mkPath(log, DatabaseDirectory + "/" + dir)
for file in CopyDatabaseSourceFiles:
	mkPath(log, os.path.dirname(DatabaseDirectory + "/" + file))

# Setup client directories
printLog(log, ">>> Setup client directories <<<")
mkPath(log, InstallDirectory + "/" + CopyInstallDirectory)

log.close()


# end of file
