#!/usr/bin/python
# 
# \file 8_upload.py
# \brief Upload data to servers
# \date 2009-02-18 16:19GMT
# \author Jan Boon (Kaetemi)
# Game data build pipeline.
# Upload data to servers
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2011  Kaetemi
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

try:
	from upload import *
except ImportError:
	# Not documenting this. Because we can.
	printLog(log, "ERROR Upload not configured, bye.")
	exit()

sys.path.append(WorkspaceDirectory)
from projects import *

# Log error
printLog(log, "")
printLog(log, "-------")
printLog(log, "--- Upload data to servers")
printLog(log, "-------")
printLog(log, time.strftime("%Y-%m-%d %H:%MGMT", time.gmtime(time.time())))
printLog(log, "")

# Find tools
# Not documenting this. Because we can.
Psftp = findFileMultiDir(log, ToolDirectories + WindowsExeDllCfgDirectories, UploadPsftpTool)
printLog(log, "PSFTP " + Psftp)

def downloadVersionTag(server, user, sshkey, dir):
	if os.path.isfile("upload.tag"):
		os.remove("upload.tag")
	if os.path.isfile("upload.batch"):
		os.remove("upload.batch")
	ub = open("upload.batch", "w")
	ub.write("cd " + dir + "\n")
	ub.write("get upload.tag upload.tag\n")
	ub.write("quit\n")
	ub.close()
	subprocess.call([ Psftp, "-b", "upload.batch", "-i", sshkey, user + "@" + server ])
	os.remove("upload.batch")
	if os.path.isfile("upload.tag"):
		ft = open("upload.tag")
		result = float(ft.read()) # float, really
		ft.close()
		os.remove("upload.tag")
		printLog(log, "INFO Upload tag is " + str(result))
		return result
	else:
		printLog(log, "WARNING Upload tag not found, uploading everything")
		return 0

def isDirectoryNeeded(ft, dir):
	files = os.listdir(dir)
	for fileName in files:
		if isLegalFileName(fileName):
			fileFull = dir + "/" + fileName
			if os.path.isfile(fileFull):
				nftf = os.stat(fileFull).st_mtime
				if nftf > ft:
					return True
			elif os.path.isdir(fileFull):
				if isDirectoryNeeded(ft, fileFull):
					return True
			elif not os.path.isdir(fileFull):
				printLog(log, "isDirectoryNeeded: file not dir or file?!" + fileFull)
	return False

def listDirectoryUpload(ft, ub, udb, dir):
	nft = 0
	files = os.listdir(dir)
	for fileName in files:
		if isLegalFileName(fileName):
			fileFull = dir + "/" + fileName
			if os.path.isfile(fileFull):
				nftf = os.stat(fileFull).st_mtime
				if nftf > ft:
					ub.write("put " + fileFull + " " + fileName + "\n")
				if nftf > nft:
					nft = nftf
			elif os.path.isdir(fileFull):
				if isDirectoryNeeded(ft, fileFull):
					udb.write("mkdir " + fileName + "\n")
					ub.write("cd " + fileName + "\n")
					udb.write("cd " + fileName + "\n")
					nft2 = listDirectoryUpload(ft, ub, udb, fileFull)
					if (nft2 > nft):
						nft = nft2
					ub.write("cd ..\n")
					udb.write("cd ..\n")
			elif not os.path.isdir(fileFull):
				printLog(log, "listDirectoryUpload: file not dir or file?!" + fileFull)
	return nft

def uploadSftp(server, user, sshkey, dir_to, dir_from, addcmd):
	ft = downloadVersionTag(server, user, sshkey, dir_to)
	if isDirectoryNeeded(ft, dir_from):
		if os.path.isfile("upload_dir.batch"):
			os.remove("upload_dir.batch")
		if os.path.isfile("upload.batch"):
			os.remove("upload.batch")
		udb = open("upload_dir.batch", "w")
		udb.write("cd " + dir_to + "\n")
		ub = open("upload.batch", "w")
		ub.write("cd " + dir_to + "\n")
		for ac in addcmd:
			ub.write(ac + "\n")
		ftn = listDirectoryUpload(ft, ub, udb, dir_from)
		if (ft > ftn):
			ftn = ft
		nft = open("upload.tag", "w")
		nft.write(str(ftn))
		nft.close()
		ub.write("put upload.tag upload.tag\n")
		ub.write("quit\n")
		ub.close()
		udb.write("quit\n")
		udb.close()
		subprocess.call([ Psftp, "-be", "-b", "upload_dir.batch", "-i", sshkey, user + "@" + server ])
		subprocess.call([ Psftp, "-b", "upload.batch", "-i", sshkey, user + "@" + server ])
		os.remove("upload_dir.batch")
		os.remove("upload.batch")
		os.remove("upload.tag")
	else:
		printLog(log, "SKIP " + dir_to)

printLog(log, ">>> Upload patch <<<")
for target in UploadPatch:
	uploadSftp(target[0], target[1], target[2], target[3], ClientPatchDirectory + "/patch", [ ])

printLog(log, ">>> Upload data_shard <<<")
for target in UploadShard:
	uploadSftp(target[0], target[1], target[2], target[3], DataShardDirectory, [ "rm *.packed_sheets", "rm primitive_cache/*.binprim" ])

printLog(log, ">>> Upload data_common <<<")
for target in UploadCommon:
	uploadSftp(target[0], target[1], target[2], target[3], DataCommonDirectory, [ ])

printLog(log, ">>> Upload data_leveldesign/leveldesign <<<")
for target in UploadLeveldesign:
	uploadSftp(target[0], target[1], target[2], target[3], LeveldesignDirectory, [ ])

printLog(log, ">>> Upload data_leveldesign/primitives <<<")
for target in UploadPrimitives:
	uploadSftp(target[0], target[1], target[2], target[3], PrimitivesDirectory, [ ])

log.close()
if os.path.isfile("8_upload.log"):
	os.remove("8_upload.log")
shutil.copy("log.log", time.strftime("%Y-%m-%d-%H-%M-GMT", time.gmtime(time.time())) + "_upload.log")
shutil.move("log.log", "8_upload.log")
