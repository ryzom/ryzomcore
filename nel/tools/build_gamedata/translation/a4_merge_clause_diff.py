#!/usr/bin/python
# 
# \author Jan Boon (Kaetemi)
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

import time, sys, os, shutil, subprocess, distutils.dir_util
sys.path.append("../configuration")
from scripts import *
from buildsite import *
from tools import *
os.chdir(TranslationDirectory)
if os.path.isfile("log.log"):
	os.remove("log.log")
log = open("log.log", "w")

printLog(log, "")
printLog(log, "-------")
printLog(log, "--- merge clause diff")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")


TranslationTools = findTool(log, ToolDirectories, TranslationToolsTool, ToolSuffix)
try:
	subprocess.call([ TranslationTools, "merge_clause_diff" ])
except Exception, e:
	printLog(log, "<" + processName + "> " + str(e))
printLog(log, "")


log.close()
if os.path.isfile("a4_merge_clause_diff.log"):
	os.remove("a4_merge_clause_diff.log")
shutil.copy("log.log", "a4_merge_clause_diff_" + time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + ".log")
shutil.move("log.log", "a4_merge_clause_diff.log")

raw_input("PRESS ANY KEY TO EXIT")
