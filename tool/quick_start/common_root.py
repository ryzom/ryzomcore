
import os

NeLQuickStartDir = os.path.dirname(os.path.realpath(__file__))

def findNeLRootRecurse(dirPath):
	configDir = os.path.join(dirPath, ".nel")
	if os.path.isdir(configDir):
		return dirPath
	parentPath = os.path.dirname(dirPath)
	if parentPath == dirPath:
		exit("NeL Root folder (.nel) not found in folder hierarchy.")
	return findNeLRootRecurse(parentPath)

def findNeLRoot():
	return findNeLRootRecurse(NeLQuickStartDir)

NeLRootDir = findNeLRoot()
NeLConfigDir = os.path.join(NeLRootDir, ".nel")

del findNeLRootRecurse
del findNeLRoot
