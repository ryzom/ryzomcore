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
printLog(log, "--- Make and merge work")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")


TranslationTools = findTool(log, ToolDirectories, TranslationToolsTool, ToolSuffix)
printLog(log, ">>> Override languages.txt <<<")
if not os.path.isfile("make_merge_wk_languages.txt"):
	shutil.move("languages.txt", "make_merge_wk_languages.txt")
languagesTxt = open("languages.txt", "w")
languagesTxt.write("wk\n")
languagesTxt.close()


printLog(log, ">>> Merge diff <<<") # This is necessary, because when we crop lines, we should only do this from untranslated files
try:
	subprocess.call([ TranslationTools, "merge_phrase_diff" ])
	subprocess.call([ TranslationTools, "merge_clause_diff" ])
	subprocess.call([ TranslationTools, "merge_words_diff" ])
	subprocess.call([ TranslationTools, "merge_string_diff" ])
	subprocess.call([ TranslationTools, "merge_worksheet_diff", "bot_names.txt" ])
except Exception, e:
	printLog(log, "<" + processName + "> " + str(e))
printLog(log, "")


printLog(log, ">>> Make diff <<<")
try:
	subprocess.call([ TranslationTools, "make_phrase_diff" ])
	subprocess.call([ TranslationTools, "make_clause_diff" ])
	subprocess.call([ TranslationTools, "make_words_diff" ])
	subprocess.call([ TranslationTools, "make_string_diff" ])
	subprocess.call([ TranslationTools, "make_worksheet_diff", "bot_names.txt" ])
except Exception, e:
	printLog(log, "<" + processName + "> " + str(e))


printLog(log, ">>> Mark diffs as translated <<<")
diffFiles = os.listdir("diff")
for diffFile in diffFiles:
	if "wk_diff_" in diffFile:
		printLog(log, "DIFF " + "diff/" + diffFile)
		subprocess.call([ TranslationTools, "crop_lines", "diff/" + diffFile, "3" ])


printLog(log, ">>> Merge diff <<<")
try:
	subprocess.call([ TranslationTools, "merge_phrase_diff" ])
	subprocess.call([ TranslationTools, "merge_clause_diff" ])
	subprocess.call([ TranslationTools, "merge_words_diff" ])
	subprocess.call([ TranslationTools, "merge_string_diff" ])
	subprocess.call([ TranslationTools, "merge_worksheet_diff", "bot_names.txt" ])
except Exception, e:
	printLog(log, "<" + processName + "> " + str(e))
printLog(log, "")


printLog(log, ">>> Restore languages.txt <<<")
os.remove("languages.txt")
shutil.move("make_merge_wk_languages.txt", "languages.txt")


log.close()
if os.path.isfile("make_merge_wk.log"):
	os.remove("make_merge_wk.log")
shutil.copy("log.log", "make_merge_wk_" + time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + ".log")
shutil.move("log.log", "make_merge_wk.log")

raw_input("PRESS ANY KEY TO EXIT")
