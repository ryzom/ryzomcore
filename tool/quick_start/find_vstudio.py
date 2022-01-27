
# This script finds all Visual Studio installations
# [ { DisplayName, Path, HasMFC } ]

import os

FoundVisualStudio = []

# C:\Program Files (x86)\Microsoft Visual Studio\VC98
# "C:\Program Files (x86)\Microsoft Visual Studio\VC98\MFC\Include\AFX.H"

VSVersions = {}
VSVersions[8] = 2005
VSVersions[9] = 2008
VSVersions[10] = 2010
VSVersions[11] = 2012
VSVersions[12] = 2013
#VSVersions[13] = 2014
VSVersions[14] = 2015
#VSVersions[15] = 2017
#VSVersions[16] = 2019
#VSVersions[17] = 2022

VSMajor = {}
#VSMajor[2005] = 8
#VSMajor[2008] = 9
#VSMajor[2010] = 10
#VSMajor[2012] = 11
#VSMajor[2013] = 12
#VSMajor[2014] = 13
#VSMajor[2015] = 14
VSMajor["2017"] = 15
VSMajor["2019"] = 16
VSMajor["2022"] = 17

# "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\atlmfc\include\afx.h"
# "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.16.27023\atlmfc\include\afx.h"
# "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\*\atlmfc\include\afx.h"
def HasMFC(dir):
	if os.path.isfile(os.path.join(dir, "VC\\atlmfc\\include\\afx.h")):
		return True
	msvcTools = os.path.join(dir, "VC\\Tools\\MSVC")
	res = []
	if os.path.isdir(msvcTools):
		for version in os.listdir(msvcTools):
			if version.startswith("."):
				continue
			versionPath = os.path.join(msvcTools, version)
			if os.path.isdir(versionPath):
				headerPath = os.path.join(versionPath, "atlmfc\\include\\afx.h")
				if os.path.isfile(headerPath):
					res += [ version ]
	if len(res):
		return res
	return False

# "C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\VCExpress.exe" # Visual C++ 2005 Express
# C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.exe" # Visual Studio 2022 Professional
# C:\Program Files\Microsoft Visual Studio\*\*\Common7\IDE\devenv.exe"
# "C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" # Visual Studio 2008 Professional Edition
# Community, Professional, Enterprise
# Scan all folders 8 to 14, 8.0 to 14.0
for majorVersion in range(8, 15):
	folderA = "C:\\Program Files (x86)\\Microsoft Visual Studio " + str(majorVersion)
	folderB = folderA + ".0"
	if os.path.isfile(os.path.join(folderA, "Common7\\IDE\\VCExpress.exe")):
		FoundVisualStudio += [ { "Name": "Visual Studio " + str(majorVersion) + " " + str(VSVersions[majorVersion]), "DisplayName": "Visual C++ " + str(VSVersions[majorVersion]) + " Express", "Path": folderA, "Version": majorVersion, "Toolset": "v" + str(majorVersion) + "0", "HasMFC": HasMFC(folderB) } ]
	if os.path.isfile(os.path.join(folderB, "Common7\\IDE\VCExpress.exe")):
		FoundVisualStudio += [ { "Name": "Visual Studio " + str(majorVersion) + " " + str(VSVersions[majorVersion]), "DisplayName": "Visual C++ " + str(VSVersions[majorVersion]) + " Express", "Path": folderB, "Version": majorVersion, "Toolset": "v" + str(majorVersion) + "0", "HasMFC": HasMFC(folderB) } ]
	if os.path.isfile(os.path.join(folderA, "Common7\\IDE\\devenv.exe")):
		FoundVisualStudio += [ { "Name": "Visual Studio " + str(majorVersion) + " " + str(VSVersions[majorVersion]), "DisplayName": "Visual Studio " + str(VSVersions[majorVersion]), "Path": folderA, "Version": majorVersion, "Toolset": "v" + str(majorVersion) + "0", "HasMFC": HasMFC(folderB) } ]
	if os.path.isfile(os.path.join(folderB, "Common7\\IDE\\devenv.exe")):
		FoundVisualStudio += [ { "Name": "Visual Studio " + str(majorVersion) + " " + str(VSVersions[majorVersion]), "DisplayName": "Visual Studio " + str(VSVersions[majorVersion]), "Path": folderB, "Version": majorVersion, "Toolset": "v" + str(majorVersion) + "0", "HasMFC": HasMFC(folderB) } ]
	del folderA
	del folderB

# Scan all folders 2017 to 2022
def ProcessYearPath(yearVersion, yearPath):
	global FoundVisualStudio
	if os.path.isdir(yearPath):
		for edition in os.listdir(yearPath):
			if edition.startswith("."):
				continue
			editionPath = os.path.join(yearPath, edition)
			if os.path.isdir(editionPath) and os.path.isfile(os.path.join(editionPath, "Common7\\IDE\\devenv.exe")):
				auxBuildPath = os.path.join(editionPath, "VC\\Auxiliary\\Build")
				for buildProps in os.listdir(auxBuildPath):
					if buildProps.startswith("Microsoft.VCToolsVersion.") and buildProps.endswith(".default.txt"):
						toolchain = buildProps.split(".")[2]
						if toolchain != "default":
							fi = open(os.path.join(auxBuildPath, buildProps))
							toolsVersion = fi.read().strip()
							fi.close()
							del fi
							mfcPath = os.path.join(editionPath, "VC\\Tools\\MSVC\\" + toolsVersion + "\\atlmfc\\include\\afx.h")
							hasMFC = os.path.isfile(mfcPath)
							FoundVisualStudio += [ { "Name": "Visual Studio " + str(VSMajor[yearVersion]) + " " + yearVersion, "DisplayName": "Visual Studio " + yearVersion + " " + edition, "Path": editionPath, "Version": VSMajor[yearVersion], "Toolset": toolchain, "HasMFC": hasMFC } ]
for yearVersion in os.listdir("C:\\Program Files (x86)\\Microsoft Visual Studio"):
	if yearVersion.startswith("."):
		continue
	yearPath = os.path.join("C:\\Program Files (x86)\\Microsoft Visual Studio", yearVersion)
	ProcessYearPath(yearVersion, yearPath)
	del yearPath
for yearVersion in os.listdir("C:\\Program Files\\Microsoft Visual Studio"):
	if yearVersion.startswith("."):
		continue
	yearPath = os.path.join("C:\\Program Files\\Microsoft Visual Studio", yearVersion)
	ProcessYearPath(yearVersion, yearPath)
	del yearPath

def FindVCVars32(path):
	auxVars = os.path.join(path, "VC\\Auxiliary\\Build\\vcvars32.bat")
	if os.path.isfile(auxVars):
		return [ auxVars ]
	allVars = os.path.join(path, "VC\\vcvarsall.bat")
	if os.path.isfile(allVars):
		return [ allVars, "x86" ]
	return

def FindVCVars64(path):
	auxVars = os.path.join(path, "VC\\Auxiliary\\Build\\vcvars64.bat")
	if os.path.isfile(auxVars):
		return [ auxVars ]
	allVars = os.path.join(path, "VC\\vcvarsall.bat")
	if os.path.isfile(allVars):
		return [ allVars, "x64" ]
	return

del ProcessYearPath
del VSVersions
del VSMajor

#print(FoundVisualStudio)

