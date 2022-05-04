import os, subprocess
prevCwd = os.getcwd()
def FindHighest(path):
	highest = 0
	try:
		for di in os.listdir(path):
			if di.startswith("."):
				continue
			try:
				ver = int(di, 10)
			except:
				continue
			if ver > highest:
				highest = ver
	except:
		pass
	return highest
def GetCurrentGitSha1():
	try:
		output = bytes.decode(subprocess.check_output(["git", "rev-parse", "HEAD"]))
		return output.strip()
	except:
		return "0"
def GetCurrentGitCount():
	try:
		output = bytes.decode(subprocess.check_output(["git", "rev-list", "--count", "HEAD"]))
		return int(output.strip(), 10)
	except:
		return 0
def GetLastVersion():
	gitSha1 = "0"
	buildNumber = 0
	try:
		fi = open(os.path.join(rcRoot, os.path.normcase(".nel/patch_version_set.bat")), "r")
		try:
			line = fi.readline()
			while line:
				if line.startswith("set RC_GIT_SHA1="):
					gitSha1 = line[len("set RC_GIT_SHA1="):].strip()
				elif line.startswith("set RC_BUILD_NUMBER="):
					buildNumber = int(line[len("set RC_BUILD_NUMBER="):].strip(), 10)
				line = fi.readline()
		finally:
			fi.close()
	except:
		pass
	return { "RC_GIT_SHA1": gitSha1, "RC_BUILD_NUMBER": buildNumber }
try:
	rcRoot = os.environ['RC_ROOT']
	rcCodeDir = os.environ['RC_CODE_DIR']
	os.chdir(rcCodeDir)
	# Find next client patch version
	clientPatchVersion = FindHighest(os.path.join(rcRoot, os.path.normcase("pipeline/client_patch/patch"))) + 1
	# Find next server patch version
	serverPatchVersion = FindHighest(os.path.join(rcRoot, os.path.normcase("pipeline/bridge_server"))) + 1
	# Increment build number if necessary
	currentSha1 = GetCurrentGitSha1()
	currentGitCount = GetCurrentGitCount()
	lastVersion = GetLastVersion()
	buildNumber = lastVersion["RC_BUILD_NUMBER"]
	if currentSha1 != lastVersion["RC_GIT_SHA1"]:
		buildNumber = buildNumber + 1
	if currentGitCount > buildNumber:
		buildNumber = currentGitCount
	bf = open(os.path.join(rcRoot, os.path.normcase(".nel/patch_version_set.bat")), "w")
	bf.write("set RC_CLIENT_PATCH_VERSION=" + str(clientPatchVersion) + "\n")
	bf.write("set RC_SERVER_PATCH_VERSION=" + str(serverPatchVersion) + "\n")
	bf.write("set RC_GIT_SHA1=" + currentSha1 + "\n")
	bf.write("set RC_BUILD_NUMBER=" + str(buildNumber) + "\n")
	bf.close()
finally:
	os.chdir(prevCwd)
# end of file
