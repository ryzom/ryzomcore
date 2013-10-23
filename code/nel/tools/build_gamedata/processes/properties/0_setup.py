#!/usr/bin/python
# 
# \file 0_setup.py
# \brief setup properties
# \date 2010-05-24 13:42GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Setup properties
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
printLog(log, "--- Setup properties")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")



mkPath(log, ActiveProjectDirectory + "/generated")
zlp = open(ActiveProjectDirectory + "/generated/properties.cfg", "w")
ps = open(ActiveProjectDirectory + "/properties_base.cfg", "r")
for line in ps:
	try:
		SmallbankExportDirectory
	except NameError:
		SmallbankExportDirectory = "_invalid"
	try:
		FarbankBuildDirectory
	except NameError:
		FarbankBuildDirectory = "_invalid"
	try:
		EcosystemName
	except NameError:
		EcosystemName = "_invalid"
	try:
		EcosystemPath
	except NameError:
		EcosystemPath = "_invalid"
	try:
		ContinentName
	except NameError:
		ContinentName = "_invalid"
	try:
		ContinentPath
	except NameError:
		ContinentPath = "_invalid"
	try:
		BankTileBankName
	except NameError:
		BankTileBankName = "_invalid"
	try:
		IgLandBuildDirectory
	except NameError:
		IgLandBuildDirectory = "_invalid"
	try:
		IgOtherBuildDirectory
	except NameError:
		IgOtherBuildDirectory = "_invalid"
	try:
		RbankOutputBuildDirectory
	except NameError:
		RbankOutputBuildDirectory = "_invalid"
	try:
		RbankRbankName
	except NameError:
		RbankRbankName = "_invalid"
	newline = line.replace("%ExportBuildDirectory%", ExportBuildDirectory)
	newline = newline.replace("%LeveldesignDirectory%", LeveldesignDirectory)
	newline = newline.replace("%LeveldesignWorldDirectory%", LeveldesignWorldDirectory)
	newline = newline.replace("%LeveldesignDfnDirectory%", LeveldesignDfnDirectory)
	newline = newline.replace("%SmallbankExportDirectory%", SmallbankExportDirectory)
	newline = newline.replace("%FarbankBuildDirectory%", FarbankBuildDirectory)
	newline = newline.replace("%EcosystemName%", EcosystemName)
	newline = newline.replace("%EcosystemPath%", EcosystemPath)
	newline = newline.replace("%ContinentName%", ContinentName)
	newline = newline.replace("%ContinentPath%", ContinentPath)
	newline = newline.replace("%CommonName%", CommonName)
	newline = newline.replace("%CommonPath%", CommonPath)
	newline = newline.replace("%BankTileBankName%", BankTileBankName)
	newline = newline.replace("%IgLandBuildDirectory%", IgLandBuildDirectory)
	newline = newline.replace("%IgOtherBuildDirectory%", IgOtherBuildDirectory)
	newline = newline.replace("%RbankOutputBuildDirectory%", RbankOutputBuildDirectory)
	newline = newline.replace("%RbankRbankName%", RbankRbankName)
	newline = newline.replace("%BuildQuality%", str(BuildQuality))
	zlp.write(newline)
ps.close()
if (BuildQuality == 1):
	ps = open(ActiveProjectDirectory + "/properties_final.cfg", "r")
else:
	ps = open(ActiveProjectDirectory + "/properties_draft.cfg", "r")
for line in ps:
	zlp.write(line)
zlp.close()
printLog(log, "")

log.close()


# end of file
