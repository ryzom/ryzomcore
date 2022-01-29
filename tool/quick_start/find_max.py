
# Script finds latest max installation, user directory, and max SDKs

import os

def HasPluginMax(path):
	return os.path.isfile(os.path.join(path, "3dsmax.exe")) and (os.path.isfile(os.path.join(path, "plugins/nelexport_r.dlu")) or os.path.isfile(os.path.join(path, "plugins/nelexport_d.dlu")) or os.path.isfile(os.path.join(path, "plugins/nelexport.dlu")))

def FindPluginMax():
	for i in range(2038, 2007, -1) + range(9, 2, -1):
		path = os.getenv('ADSK_3DSMAX_x64_' + str(i))
		if path and HasPluginMax(path):
			return os.path.normpath(path)
		path = "C:/Program Files/Autodesk/3ds Max " + str(i)
		if HasPluginMax(path):
			return os.path.normpath(path)
		path = "C:/Program Files (x86)/Autodesk/3ds Max " + str(i)
		if HasPluginMax(path):
			return os.path.normpath(path)

def FindLatestMax():
	for i in range(2038, 2007, -1) + range(9, 2, -1):
		path = os.getenv('ADSK_3DSMAX_x64_' + str(i))
		if path and os.path.isfile(os.path.join(path, "3dsmax.exe")):
			return os.path.normpath(path)
		path = "C:/Program Files/Autodesk/3ds Max " + str(i)
		if os.path.isfile(os.path.join(path, "3dsmax.exe")):
			return os.path.normpath(path)
		path = "C:/Program Files (x86)/Autodesk/3ds Max " + str(i)
		if os.path.isfile(os.path.join(path, "3dsmax.exe")):
			return os.path.normpath(path)

def FindMaxLocal(maxPath):
	maxPathSplit = maxPath.split()
	version = maxPathSplit[len(maxPathSplit) - 1]
	if "x86" in maxPath:
		version += " - 32bit"
	else:
		version += " - 64bit"
	return os.path.normpath(os.path.expandvars("%LocalAppData%/Autodesk/3dsMax/" + version + "/ENU"))

def FindMaxSDK(version):
	path = os.getenv('ADSK_3DSMAX_SDK_' + str(version))
	if path:
		if os.path.isfile(os.path.join(path, "include/max.h")):
			return os.path.normpath(path)
		elif os.path.isfile(os.path.join(path, "maxsdk/include/max.h")):
			return os.path.normpath(os.path.join(path, "maxsdk"))
	path = "C:/Program Files/Autodesk/3ds Max " + str(version) + " SDK"
	if os.path.isfile(os.path.join(path, "include/max.h")):
		return os.path.normpath(path)
	elif os.path.isfile(os.path.join(path, "maxsdk/include/max.h")):
		return os.path.normpath(os.path.join(path, "maxsdk"))
	path = "C:/Program Files (x86)/Autodesk/3ds Max " + str(version) + " SDK"
	if os.path.isfile(os.path.join(path, "include/max.h")):
		return os.path.normpath(path)
	elif os.path.isfile(os.path.join(path, "maxsdk/include/max.h")):
		return os.path.normpath(os.path.join(path, "maxsdk"))
	path = "C:/Program Files (x86)/Autodesk/3dsMax" + str(version)
	if os.path.isfile(os.path.join(path, "include/max.h")):
		return os.path.normpath(path)
	elif os.path.isfile(os.path.join(path, "maxsdk/include/max.h")):
		return os.path.normpath(os.path.join(path, "maxsdk"))
	return

def FindMaxSDKs():
	toolset = {
		2022: "v141",
		2021: "v141",
		2020: "v141",
		2019: "v140",
		2018: "v140",
		2017: "v140",
		2016: "v110",
		2015: "v110",
		2014: "v100",
		2013: "v100",
		2012: "v90",
		2011: "v90",
		2010: "v90",
		2009: "v80",
		2008: "v80",
		9: "v80",
		8: "v70",
		7: "v70",
		6: "v70",
		5: "v60",
		4: "v60",
		3: "v60",
	}
	compatible = {
		2021: 2020,
		2016: 2015,
		2014: 2013,
		2011: 2010,
		2008: 9,
		8: 7,
		7: 6,
		5: 4,
	}
	res = []
	set = {}
	remap = {}
	for i in range(3, 10) + range(2008, 2039):
		found = FindMaxSDK(i)
		if found:
			set[i] = True
			c = None
			remap[i] = i
			if i in compatible:
				if compatible[i] in set:
					c = remap[compatible[i]]
					remap[i] = c
			t = "v142"
			if i in toolset:
				t = toolset[i]
			rv = { "Path": found, "Version": i, "Toolset": t }
			if c:
				rv["Compatible"] = c
			if i >= 2015:
				rv["Platform"] = "x64"
			else:
				rv["Platform"] = "x86"
			res += [ rv ]
	return res

FoundPluginMax = FindPluginMax()
FoundPluginMaxLocal = FindMaxLocal(FindPluginMax())
FoundLatestMax = FindLatestMax()
FoundMaxSDKs = FindMaxSDKs()

print FoundPluginMax
print FoundPluginMaxLocal
print FoundLatestMax
print FoundMaxSDKs

# end of file
