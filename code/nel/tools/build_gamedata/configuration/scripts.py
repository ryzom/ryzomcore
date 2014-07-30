#!/usr/bin/python
# 
# \file export.py
# \brief Useful scripts
# \date 2009-02-18 09:22GMT
# \author Jan Boon (Kaetemi)
# Useful scripts
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

ActiveProjectDirectory = os.getenv("NELBUILDACTIVEPROJECT", "configuration/project")
sys.path.append(ActiveProjectDirectory)

def printLog(log, text):
	log.write(text + "\n")
	print text

def mkPath(log, path):
	printLog(log, "DIR " + path)
	distutils.dir_util.mkpath(path)

def needUpdate(log, source, dest):
	if (os.path.isfile(source)):
		if (os.path.isfile(dest)):
			if (os.stat(source).st_mtime > os.stat(dest).st_mtime):
				return 1
			else:
				return 0
		return 1
	printLog(log, "MISSING " + source)
	return 0

def needUpdateRemoveDest(log, source, dest):
	if (os.path.isfile(source)):
		if (os.path.isfile(dest)):
			if (os.stat(source).st_mtime > os.stat(dest).st_mtime):
				os.remove(dest)
				return 1
			else:
				return 0
		return 1
	printLog(log, "MISSING " + source)
	return 0

def needUpdateLogRemoveDest(log, source, dest):
	if (os.path.isfile(source)):
		if (os.path.isfile(dest)):
			if (os.stat(source).st_mtime > os.stat(dest).st_mtime):
				os.remove(dest)
				printLog(log, source + " -> " + dest)
				return 1
			else:
				printLog(log, "SKIP " + dest)
				return 0
		printLog(log, source + " -> " + dest)
		return 1
	printLog(log, "MISSING " + source)
	printLog(log, "SKIP " + dest)
	return 0

def copyFileList(log, dir_source, dir_target, files):
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			if (os.path.isfile(dir_source + "/" + fileName)):
				if needUpdateLogRemoveDest(log, dir_source + "/" + fileName, dir_target + "/" + fileName):
					shutil.copy(dir_source + "/" + fileName, dir_target + "/" + fileName)

def copyFileListLogless(log, dir_source, dir_target, files):
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			if (os.path.isfile(dir_source + "/" + fileName)):
				if needUpdateRemoveDest(log, dir_source + "/" + fileName, dir_target + "/" + fileName):
					shutil.copy(dir_source + "/" + fileName, dir_target + "/" + fileName)

def copyFileListNoTree(log, dir_source, dir_target, files):
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			if (os.path.isfile(dir_source + "/" + fileName)):
				printLog(log, dir_source + "/" + fileName + " -> " + dir_target + "/" + os.path.basename(fileName))
				shutil.copy(dir_source + "/" + fileName, dir_target + "/" + os.path.basename(fileName))

def copyFileListNoTreeIfNeeded(log, dir_source, dir_target, files):
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			if (os.path.isfile(dir_source + "/" + fileName)):
				srcFile = dir_source + "/" + fileName
				destFile = dir_target + "/" + os.path.basename(fileName)
				if needUpdateLogRemoveDest(log, srcFile, destFile):
					shutil.copy(srcFile, destFile)

def removeFilesRecursive(log, dir_files):
	files = os.listdir(dir_files)
	for fileName in files:
		if (fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*"):
			if os.path.isdir(dir_files + "/" + fileName):
				removeFilesRecursive(log, dir_files + "/" + fileName)
			else:
				printLog(log, "RM " + dir_files + "/" + fileName)
				os.remove(dir_files + "/" + fileName)

def removeFilesDirsRecursive(log, dir_files):
	files = os.listdir(dir_files)
	for fileName in files:
		if (fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*"):
			if os.path.isdir(dir_files + "/" + fileName):
				removeFilesRecursive(log, dir_files + "/" + fileName)
			else:
				printLog(log, "RM " + dir_files + "/" + fileName)
				os.remove(dir_files + "/" + fileName)
	printLog(log, "RMDIR " + dir_files)
	os.rmdir(dir_files)

def removeFilesRecursiveExt(log, dir_files, file_ext):
	files = os.listdir(dir_files)
	len_file_ext = len(file_ext)
	for fileName in files:
		if (fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*"):
			if os.path.isdir(dir_files + "/" + fileName):
				removeFilesRecursiveExt(log, dir_files + "/" + fileName, file_ext)
			elif (fileName[-len_file_ext:].lower() == file_ext.lower()):
				printLog(log, "RM " + dir_files + "/" + fileName)
				os.remove(dir_files + "/" + fileName)

def copyFilesRecursive(log, dir_source, dir_target):
	files = os.listdir(dir_source)
	mkPath(log, dir_target)
	for fileName in files:
		if (fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*"):
			if os.path.isdir(dir_source + "/" + fileName):
				copyFilesRecursive(log, dir_source + "/" + fileName, dir_target + "/" + fileName)
			else:
				printLog(log, dir_source + "/" + fileName + " -> " + dir_target + "/" + fileName)
				shutil.copy(dir_source + "/" + fileName, dir_target + "/" + fileName)

def copyFiles(log, dir_source, dir_target):
	copyFileList(log, dir_source, dir_target, os.listdir(dir_source))

def copyFilesLogless(log, dir_source, dir_target):
	copyFileListLogless(log, dir_source, dir_target, os.listdir(dir_source))

def copyFilesExt(log, dir_source, dir_target, file_ext):
	files = os.listdir(dir_source)
	len_file_ext = len(file_ext)
	for fileName in files:
		if (fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*") and (fileName[-len_file_ext:].lower() == file_ext.lower()):
			if (os.path.isfile(dir_source + "/" + fileName)):
				printLog(log, dir_source + "/" + fileName + " -> " + dir_target + "/" + fileName)
				shutil.copy(dir_source + "/" + fileName, dir_target + "/" + fileName)

def copyFilesRenamePrefixExt(log, dir_source, dir_target, old_prefix, new_prefix, file_ext):
	files = os.listdir(dir_source)
	len_file_ext = len(file_ext)
	len_prefix = len(old_prefix)
	for fileName in files:
		if (fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*") and (fileName[-len_file_ext:].lower() == file_ext.lower()) and ((fileName[:len_prefix].lower() == old_prefix.lower())):
			if (os.path.isfile(dir_source + "/" + fileName)):
				printLog(log, dir_source + "/" + fileName + " -> " + dir_target + "/" + new_prefix + fileName[-(len(fileName) - len_prefix):])
				shutil.copy(dir_source + "/" + fileName, dir_target + "/" + new_prefix + fileName[-(len(fileName) - len_prefix):])

def copyFilesExtNoSubdir(log, dir_source, dir_target, file_ext):
	files = findFilesNoSubdir(log, dir_source, file_ext)
	copyFileListNoTree(log, dir_source, dir_target, files)

def copyFilesExtNoTree(log, dir_source, dir_target, file_ext):
	files = findFiles(log, dir_source, "", file_ext)
	copyFileListNoTree(log, dir_source, dir_target, files)

def copyFilesExtNoTreeIfNeeded(log, dir_source, dir_target, file_ext):
	files = findFiles(log, dir_source, "", file_ext)
	copyFileListNoTreeIfNeeded(log, dir_source, dir_target, files)

def copyFilesExtNoSubdirIfNeeded(log, dir_source, dir_target, file_ext):
	files = findFilesNoSubdir(log, dir_source, file_ext)
	copyFileListNoTreeIfNeeded(log, dir_source, dir_target, files)

def copyFilesNoTreeIfNeeded(log, dir_source, dir_target):
	copyFileListNoTreeIfNeeded(log, dir_source, dir_target, os.listdir(dir_source))

def copyFilesRecursiveNoTreeIfNeeded(log, dir_source, dir_target):
	files = findFilesRecursive(log, dir_source, "")
	copyFileListNoTreeIfNeeded(log, dir_source, dir_target, files)

def copyFileListExtReplaceNoTreeIfNeeded(log, dir_source, dir_target, files, file_ext, target_ext):
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			if (os.path.isfile(dir_source + "/" + fileName)):
				srcFile = dir_source + "/" + fileName
				destFile = dir_target + "/" + os.path.basename(fileName)[0:-len(file_ext)] + target_ext
				if needUpdateLogRemoveDest(log, srcFile, destFile):
					shutil.copy(srcFile, destFile)
	
def copyFilesExtReplaceNoTreeIfNeeded(log, dir_source, dir_target, file_ext, target_ext):
	files = findFiles(log, dir_source, "", file_ext)
	copyFileListExtReplaceNoTreeIfNeeded(log, dir_source, dir_target, files, file_ext, target_ext)

def copyFileIfNeeded(log, srcFile, destFile):
	if needUpdateLogRemoveDest(log, srcFile, destFile):
		shutil.copy(srcFile, destFile)

def moveFileListNoTree(log, dir_source, dir_target, files):
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			if (os.path.isfile(dir_source + "/" + fileName)):
				printLog(log, "MOVE " + dir_source + "/" + fileName + " -> " + dir_target + "/" + os.path.basename(fileName))
				shutil.move(dir_source + "/" + fileName, dir_target + "/" + os.path.basename(fileName))

def moveFilesExtNoTree(log, dir_source, dir_target, file_ext):
	files = findFiles(log, dir_source, "", file_ext)
	moveFileListNoTree(log, dir_source, dir_target, files)

def moveFilesNoSubdir(log, dir_source, dir_target):
	files = os.listdir(dir_source)
	moveFileListNoTree(log, dir_source, dir_target, files)

def moveDir(log, dir_source, dir_target):
	printLog(log, "MOVE " + dir_source + " -> " + dir_target)
	shutil.move(dir_source, dir_target)

def findFilesRecursive(log, dir_where, dir_sub):
	result = [ ]
	files = os.listdir(dir_where + "/" + dir_sub)
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			filePath = dir_sub + fileName
			fileFull = dir_where + "/" + dir_sub + fileName
			if os.path.isfile(fileFull):
				result += [ filePath ]
			elif os.path.isdir(fileFull):
				result += findFilesRecursive(log, dir_where, filePath + "/")
			else:
				printLog(log, "findFilesRecursive: file not dir or file?!" + filePath)
	return result

def findFiles(log, dir_where, dir_sub, file_ext):
	result = [ ]
	files = os.listdir(dir_where + "/" + dir_sub)
	len_file_ext = len(file_ext)
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			filePath = dir_sub + fileName
			fileFull = dir_where + "/" + dir_sub + fileName
			if os.path.isfile(fileFull):
				if fileName[-len_file_ext:].lower() == file_ext.lower():
					result += [ filePath ]
			elif os.path.isdir(fileFull):
				result += findFiles(log, dir_where, filePath + "/", file_ext)
			else:
				printLog(log, "findFiles: file not dir or file?!" + filePath)
	return result

def isLegalFileName(fileName):
	return fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*"

def findFilesNoSubdir(log, dir_where, file_ext):
	result = [ ]
	files = os.listdir(dir_where)
	len_file_ext = len(file_ext)
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			fileFull = dir_where + "/" + fileName
			if os.path.isfile(fileFull):
				if fileName[-len_file_ext:].lower() == file_ext.lower():
					result += [ fileName ]
			elif not os.path.isdir(fileFull):
				printLog(log, "findFilesNoSubdir: file not dir or file?!" + fileFull)
	return result

def findFilesNoSubdirFiltered(log, dir_where, file_ext, filter):
	if len(filter) == 0:
		return findFilesNoSubdir(log, dir_where, file_ext)
	result = [ ]
	files = os.listdir(dir_where)
	len_file_ext = len(file_ext)
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			fileFull = dir_where + "/" + fileName
			if os.path.isfile(fileFull):
				if fileName[-len_file_ext:].lower() == file_ext.lower():
					fileNameLower = fileName.lower()
					for filterWord in filter:
						if filterWord in fileNameLower:
							result += [ fileName ]
							break
			elif not os.path.isdir(fileFull):
				printLog(log, "findFilesNoSubdir: file not dir or file?!" + fileFull)
	return result

def findFile(log, dir_where, file_name):
	files = os.listdir(dir_where)
	for fileName in files:
		if fileName != ".svn" and fileName != ".." and fileName != "." and fileName != "*.*":
			filePath = dir_where + "/" + fileName
			if os.path.isfile(filePath):
				if fileName == file_name:
					return filePath
			elif os.path.isdir(filePath):
				result = findFile(log, filePath, file_name)
				if result != "":
					return result
			else:
				printLog(log, "findFile: file not dir or file?! " + filePath)
	return ""

def needUpdateDirByLowercaseTagLog(log, dir_source, ext_source, dir_dest, ext_dest):
	updateCount = 0
	skipCount = 0
	lenSrcExt = len(ext_source)
	sourceFiles = findFilesNoSubdir(log, dir_source, ext_source)
	destFiles = findFilesNoSubdir(log, dir_dest, ext_dest)
	for file in sourceFiles:
		sourceFile = dir_source + "/" + file
		tagFile = dir_dest + "/" + file[0:-lenSrcExt].lower() + ext_dest
		if os.path.isfile(tagFile):
			sourceTime = os.stat(sourceFile).st_mtime
			tagTime = os.stat(tagFile).st_mtime
			if (sourceTime > tagTime):
				updateCount = updateCount + 1
			else:
				skipCount = skipCount + 1
		else:
			updateCount = updateCount + 1
	if updateCount > 0:
		printLog(log, "UPDATE " + str(updateCount) + " / " + str(len(sourceFiles)) + "; SKIP " + str(skipCount) + " / " + str(len(sourceFiles)) + "; DEST " + str(len(destFiles)))
		return 1
	else:
		printLog(log, "SKIP " + str(skipCount) + " / " + str(len(sourceFiles)) + "; DEST " + str(len(destFiles)))
		return 0

def needUpdateDirByTagLogFiltered(log, dir_source, ext_source, dir_dest, ext_dest, filter):
	updateCount = 0
	skipCount = 0
	lenSrcExt = len(ext_source)
	sourceFiles = findFilesNoSubdirFiltered(log, dir_source, ext_source, filter)
	destFiles = findFilesNoSubdir(log, dir_dest, ext_dest)
	for file in sourceFiles:
		sourceFile = dir_source + "/" + file
		tagFile = dir_dest + "/" + file[0:-lenSrcExt] + ext_dest
		if os.path.isfile(tagFile):
			sourceTime = os.stat(sourceFile).st_mtime
			tagTime = os.stat(tagFile).st_mtime
			if (sourceTime > tagTime):
				updateCount = updateCount + 1
			else:
				skipCount = skipCount + 1
		else:
			updateCount = updateCount + 1
	if updateCount > 0:
		printLog(log, "UPDATE " + str(updateCount) + " / " + str(len(sourceFiles)) + "; SKIP " + str(skipCount) + " / " + str(len(sourceFiles)) + "; DEST " + str(len(destFiles)))
		return 1
	else:
		printLog(log, "SKIP " + str(skipCount) + " / " + str(len(sourceFiles)) + "; DEST " + str(len(destFiles)))
		return 0

def needUpdateDirByTagLog(log, dir_source, ext_source, dir_dest, ext_dest):
	needUpdateDirByTagLogFiltered(log, dir_source, ext_source, dir_dest, ext_dest, [ ])

def needUpdateDirNoSubdirFile(log, dir_source, file_dest):
	if not os.path.isfile(file_dest):
		return 1
	destTime = os.stat(file_dest).st_mtime
	sourceFiles = os.listdir(dir_source)
	for file in sourceFiles:
		filePath = dir_source + "/" + file
		if os.path.isfile(filePath):
			fileTime = os.stat(filePath).st_mtime
			if fileTime > destTime:
				return 1
	else:
		return 0

def needUpdateFileDirNoSubdir(log, file_source, dir_dest):
	if not os.path.isfile(file_source):
		printLog(log, "WARNING MISSING " + file_source)
		return 0
	sourceTime = os.stat(file_source).st_mtime
	destFiles = os.listdir(dir_dest)
	for file in destFiles:
		filePath = dir_dest + "/" + file
		if os.path.isfile(filePath):
			fileTime = os.stat(filePath).st_mtime
			if sourceTime > fileTime:
				return 1
	else:
		return 0

def needUpdateDirNoSubdirMultiFile(log, dir_source, root_file, files_dest):
	for file_dest in files_dest:
		if needUpdateDirNoSubdirFile(log, dir_source, root_file + "/" + file_dest):
			return 1
	return 0

def needUpdateDirNoSubdirMultiFileExt(log, dir_source, root_file, files_dest, file_ext):
	for file_dest in files_dest:
		if needUpdateDirNoSubdirFile(log, dir_source, root_file + "/" + file_dest + file_ext):
			return 1
	return 0

def needUpdateMultiDirNoSubdirFile(log, root_dir, dirs_source, file_dest):
	for dir_source in dirs_source:
		if needUpdateDirNoSubdirFile(log, root_dir + "/" + dir_source, file_dest):
			return 1
	return 0

def needUpdateMultiDirNoSubdirMultiFileExt(log, root_dir, dirs_source, root_file, files_dest, file_ext):
	for file_dest in files_dest:
		if needUpdateMultiDirNoSubdirFile(log, root_dir, dirs_source, root_file + "/" + file_dest + file_ext):
			return 1
	return 0

def needUpdateMultiDirNoSubdir(log, root_dir, dirs_source, dir_dest):
	for dir_source in dirs_source:
		if needUpdateDirNoSubdir(log, root_dir + "/" + dir_source, dir_dest):
			return 1
	return 0

def needUpdateDirNoSubdirExtFile(log, dir_source, dir_ext, file_dest):
	if not os.path.isfile(file_dest):
		return 1
	destTime = os.stat(file_dest).st_mtime
	sourceFiles = os.listdir(dir_source)
	for file in sourceFiles:
		if file.endswith(dir_ext):
			filePath = dir_source + "/" + file
			if os.path.isfile(filePath):
				fileTime = os.stat(filePath).st_mtime
				if fileTime > destTime:
					return 1
	else:
		return 0

def needUpdateDirNoSubdirExtMultiFileExt(log, dir_source, dir_ext, root_file, files_dest, file_ext):
	for file_dest in files_dest:
		if needUpdateDirNoSubdirExtFile(log, dir_source, dir_ext, root_file + "/" + file_dest + file_ext):
			return 1
	return 0

def needUpdateDirNoSubdir(log, dir_source, dir_dest):
	latestSourceFile = 0
	oldestDestFile = 0
	sourceFiles = os.listdir(dir_source)
	destFiles = os.listdir(dir_dest)
	for file in sourceFiles:
		filePath = dir_source + "/" + file
		if os.path.isfile(filePath):
			fileTime = os.stat(filePath).st_mtime
			if fileTime > latestSourceFile:
				latestSourceFile = fileTime
	for file in destFiles:
		filePath = dir_dest + "/" + file
		if os.path.isfile(filePath):
			fileTime = os.stat(filePath).st_mtime
			if oldestDestFile == 0 or fileTime < oldestDestFile:
				oldestDestFile = fileTime
	if latestSourceFile > oldestDestFile:
		return 1
	else:
		return 0

def needUpdateDirNoSubdirLogExt(log, dir_source, ext_source, dir_dest, ext_dest):
	latestSourceFile = 0
	latestDestFile = 0
	sourceFiles = findFilesNoSubdir(log, dir_source, ext_source)
	destFiles = findFilesNoSubdir(log, dir_dest, ext_dest)
	for file in sourceFiles:
		fileTime = os.stat(dir_source + "/" + file).st_mtime
		if (fileTime > latestSourceFile):
			latestSourceFile = fileTime
	for file in destFiles:
		fileTime = os.stat(dir_dest + "/" + file).st_mtime
		if (fileTime > latestDestFile):
			latestDestFile = fileTime
	if latestSourceFile > latestDestFile or len(sourceFiles) > len(destFiles):
		printLog(log, "UPDATE; Source: " + str(latestSourceFile) + ", " + str(len(sourceFiles)) + " files; Dest: " + str(latestDestFile) + ", " + str(len(destFiles)) + " files")
		return 1
	else:
		printLog(log, "SKIP *")
		return 0

def needUpdateDirNoSubdirLogExtMultidir(log, all_dir_base, all_dir_source, dir_source, ext_source, dir_dest, ext_dest):
	latestSourceFile = 0
	latestDestFile = 0
	sourceFilesAll = [ ]
	for dir in all_dir_source:
		sourceFilesAll += findFilesNoSubdir(log, all_dir_base + "/" + dir, ext_source)
	sourceFiles = findFilesNoSubdir(log, dir_source, ext_source)
	destFiles = findFilesNoSubdir(log, dir_dest, ext_dest)
	for file in sourceFiles:
		fileTime = os.stat(dir_source + "/" + file).st_mtime
		if (fileTime > latestSourceFile):
			latestSourceFile = fileTime
	for file in destFiles:
		fileTime = os.stat(dir_dest + "/" + file).st_mtime
		if (fileTime > latestDestFile):
			latestDestFile = fileTime
	if latestSourceFile > latestDestFile or len(sourceFilesAll) > len(destFiles):
		printLog(log, "UPDATE; Source: " + str(latestSourceFile) + ", " + str(len(sourceFilesAll)) + " files; Dest: " + str(latestDestFile) + ", " + str(len(destFiles)) + " files")
		return 1
	else:
		printLog(log, "SKIP *")
		return 0

def findFileMultiDir(log, dirs_where, file_name):
	try:
		for dir in dirs_where:
			file = findFile(log, dir, file_name)
			if file != "":
				return file
	except Exception, e:
		printLog(log, "EXCEPTION " + str(e))
	printLog(log, "FILE NOT FOUND " + file_name)
	return ""

def findTool(log, dirs_where, file_name, suffix):
	try:
		for dir in dirs_where:
			tool = findFile(log, dir, file_name + suffix)
			if tool != "":
				printLog(log, "TOOL " + tool)
				return tool
	except Exception, e:
		printLog(log, "EXCEPTION " + str(e))
	printLog(log, "TOOL NOT FOUND " + file_name + suffix)
	return ""

def findMax(log, dir, file):
	tool = dir + "/" + file
	if os.path.isfile(tool):
		printLog(log, "3DSMAX " + tool)
		return tool
	printLog(log, "3DSMAX NOT FOUND " + file)
	return ""

def toolLogFail(log, tool, suffix):
	printLog(log, "FAIL " + tool + suffix + " is not found")

def askVar(log, name, default):
	sys.stdout.write(name + " (" + default + "): ")
	line = sys.stdin.readline()
	linestrip = line.strip()
	if linestrip == "--":
		log.write(name + " (" + default + "): ''\n")
		return ""
	elif linestrip == "":
		log.write(name + " (" + default + "): '" + default + "'\n")
		return default
	else:
		log.write(name + " (" + default + "): '" + linestrip + "'\n")
		return linestrip
