
# Visual Studio installation scan over one drive root. find_vstudio.py runs it on the native C: drive,
# dump_wine.py on the drive_c of every Wine prefix (where the path components are case sensitive, hence
# the case insensitive joins). Standard library only, no quick start imports, so it also runs inside the
# Wine Docker image.
# [ { Name, DisplayName, Path, Version, Toolset, HasMFC } ]

import os

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
VSMajor["2026"] = 18

VSPlatform = {}
VSPlatform["v110"] = "v110"
VSPlatform["v120"] = "v120"
VSPlatform["v140"] = "v140"
VSPlatform["v141"] = "v150"
VSPlatform["v142"] = "v160"
VSPlatform["v143"] = "v170"

def ListDir(path):
	try:
		res = []
		for entry in os.listdir(path):
			if not entry.startswith("."):
				res += [ entry ]
		res.sort()
		return res
	except OSError:
		return []

# Join path components onto a root, matching each component without regard to case when the exact
# spelling does not exist. Returns None when a component is missing.
def JoinCI(root, *parts):
	path = root
	for part in parts:
		for comp in part.replace("\\", "/").split("/"):
			if not comp:
				continue
			candidate = os.path.join(path, comp)
			if os.path.exists(candidate):
				path = candidate
				continue
			match = None
			for entry in ListDir(path):
				if entry.lower() == comp.lower():
					match = entry
					break
			if match is None:
				return None
			path = os.path.join(path, match)
	return path

def IsFileCI(root, *parts):
	path = JoinCI(root, *parts)
	return path is not None and os.path.isfile(path)

def IsDirCI(root, *parts):
	path = JoinCI(root, *parts)
	return path is not None and os.path.isdir(path)

# The program folders of a drive: both on a 64-bit Windows, only the plain one on a 32-bit install
# such as the XP based Wine prefixes
def ProgramFilesDirs(root):
	res = []
	for name in [ "Program Files (x86)", "Program Files" ]:
		path = JoinCI(root, name)
		if path is not None and os.path.isdir(path):
			res += [ path ]
	return res

# "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\atlmfc\include\afx.h"
# "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.16.27023\atlmfc\include\afx.h"
# "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\*\atlmfc\include\afx.h"
def HasMFC(dir):
	if IsFileCI(dir, "VC/atlmfc/include/afx.h"):
		return True
	msvcTools = JoinCI(dir, "VC/Tools/MSVC")
	res = []
	if msvcTools is not None and os.path.isdir(msvcTools):
		for version in ListDir(msvcTools):
			versionPath = os.path.join(msvcTools, version)
			if os.path.isdir(versionPath):
				if IsFileCI(versionPath, "atlmfc/include/afx.h"):
					res += [ version ]
	if len(res):
		return res
	return False

# "C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\VCExpress.exe" # Visual C++ 2005 Express
# C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.exe" # Visual Studio 2022 Professional
# C:\Program Files\Microsoft Visual Studio\*\*\Common7\IDE\devenv.exe"
# "C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" # Visual Studio 2008 Professional Edition
# Community, Professional, Enterprise
def ScanFolderEra(found, programFiles):
	# Scan all folders 8 to 14, 8.0 to 14.0
	for majorVersion in range(8, 15):
		for folderName in [ "Microsoft Visual Studio " + str(majorVersion), "Microsoft Visual Studio " + str(majorVersion) + ".0" ]:
			folder = JoinCI(programFiles, folderName)
			if folder is None or not os.path.isdir(folder):
				continue
			if IsFileCI(folder, "Common7/IDE/VCExpress.exe"):
				found += [ { "Name": "Visual Studio " + str(majorVersion) + " " + str(VSVersions[majorVersion]), "DisplayName": "Visual C++ " + str(VSVersions[majorVersion]) + " Express", "Path": folder, "Version": majorVersion, "Toolset": "v" + str(majorVersion) + "0", "HasMFC": HasMFC(folder) } ]
			if IsFileCI(folder, "Common7/IDE/devenv.exe"):
				found += [ { "Name": "Visual Studio " + str(majorVersion) + " " + str(VSVersions[majorVersion]), "DisplayName": "Visual Studio " + str(VSVersions[majorVersion]), "Path": folder, "Version": majorVersion, "Toolset": "v" + str(majorVersion) + "0", "HasMFC": HasMFC(folder) } ]

# Scan all folders 2017 and up
def ScanYearPath(found, root, yearVersion, yearPath):
	for edition in ListDir(yearPath):
		editionPath = os.path.join(yearPath, edition)
		if not os.path.isdir(editionPath) or not IsFileCI(editionPath, "Common7/IDE/devenv.exe"):
			continue
		if not yearVersion in VSMajor:
			# Installer and Shared live next to the release folders, without an IDE; a release this scan does not know yet has one
			print("WARNING: Unknown Visual Studio release \"" + yearVersion + "\" in " + editionPath + ", skipped")
			return
		auxBuildPath = JoinCI(editionPath, "VC/Auxiliary/Build")
		if auxBuildPath is None or not os.path.isdir(auxBuildPath):
			continue
		# v141 Toolset and up
		for buildProps in ListDir(auxBuildPath):
			if buildProps.startswith("Microsoft.VCToolsVersion.") and buildProps.endswith(".default.txt"):
				toolchain = buildProps.split(".")[2]
				if toolchain != "default":
					fi = open(os.path.join(auxBuildPath, buildProps))
					toolsVersion = fi.read().strip()
					fi.close()
					hasMFC = IsFileCI(editionPath, "VC/Tools/MSVC/" + toolsVersion + "/atlmfc/include/afx.h")
					found += [ { "Name": "Visual Studio " + str(VSMajor[yearVersion]) + " " + yearVersion, "DisplayName": "Visual Studio " + yearVersion + " " + edition, "Path": editionPath, "Version": VSMajor[yearVersion], "Toolset": toolchain, "HasMFC": hasMFC } ]
		# v140 Toolset
		for programFiles in ProgramFilesDirs(root):
			vs14 = JoinCI(programFiles, "Microsoft Visual Studio 14.0")
			if vs14 is not None and IsFileCI(vs14, "VC/vcvarsall.bat"):
				found += [ { "Name": "Visual Studio " + str(VSMajor[yearVersion]) + " " + yearVersion, "DisplayName": "Visual Studio " + yearVersion + " " + edition, "Path": editionPath, "Version": VSMajor[yearVersion], "Toolset": "v140", "HasMFC": IsFileCI(vs14, "VC/atlmfc/include/afx.h") } ]
				break

def ScanVisualStudio(root):
	found = []
	for programFiles in ProgramFilesDirs(root):
		ScanFolderEra(found, programFiles)
		yearRoot = JoinCI(programFiles, "Microsoft Visual Studio")
		if yearRoot is not None and os.path.isdir(yearRoot):
			for yearVersion in ListDir(yearRoot):
				yearPath = os.path.join(yearRoot, yearVersion)
				if os.path.isdir(yearPath):
					ScanYearPath(found, root, yearVersion, yearPath)
	# Check for any available XP toolsets
	for vs in list(found):
		if vs["Version"] >= 11 and vs["Toolset"] in VSPlatform:
			platform = VSPlatform[vs["Toolset"]]
			# C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Microsoft\VC\v150\Platforms\Win32\PlatformToolsets\v141_xp
			if vs["Toolset"] == "v140" or IsDirCI(vs["Path"], "MSBuild/Microsoft/VC/" + platform + "/Platforms/Win32/PlatformToolsets/" + vs["Toolset"] + "_xp"):
				found += [ { "Name": vs["Name"], "DisplayName": vs["DisplayName"], "Path": vs["Path"], "Version": vs["Version"], "Toolset": vs["Toolset"] + "_xp", "HasMFC": vs["HasMFC"] } ]
	return found

# The newest MSVC tools version installed for each toolset, as vcvarsall wants it
def ToolsetMap(found):
	res = {}
	for vs in found:
		versions = {}
		toolsPath = JoinCI(vs["Path"], "VC/Tools/MSVC")
		if toolsPath is not None and os.path.isdir(toolsPath):
			for dir in ListDir(toolsPath):
				if "." in dir:
					versions[dir] = True
		bvremap = [ 0, 1, 2, 3, 3 ]
		highest = [ 0, 0, 0, 0 ]
		foundVersions = [ "14.0", "14.16", "14.29", "14.30" ]
		for v in versions:
			spl = v.split(".")
			if spl[0] == "14" and len(spl) > 1 and spl[1].isdigit() and int(spl[1][0]) < len(bvremap):
				bv = bvremap[int(spl[1][0])]
				sub = int(spl[1])
				if sub > highest[bv]:
					highest[bv] = sub
					foundVersions[bv] = spl[0] + "." + spl[1]
		res["v140"] = foundVersions[0]
		res["v140_xp"] = foundVersions[0]
		res["v141"] = foundVersions[1]
		res["v141_xp"] = foundVersions[1]
		res["v142"] = foundVersions[2]
		res["v143"] = foundVersions[3]
	return res

# The DirectX SDKs of a drive, newest first: [ { Name, Path } ]
# https://archive.org/details/directxsdks
def ScanDirectXSDKs(root):
	res = []
	for programFiles in ProgramFilesDirs(root):
		for entry in ListDir(programFiles):
			if entry.lower().startswith("microsoft directx sdk (") and entry.endswith(")"):
				path = os.path.join(programFiles, entry)
				if IsFileCI(path, "Include/d3d9.h"):
					res += [ { "Name": entry[len("Microsoft DirectX SDK ("):-1], "Path": path, "HasXAudio2": HasXAudio2(path) } ]
	return res

# The DirectX SDK entry for a Visual Studio version, optimized for compatibility. If the prefered SDK
# is not available, the generic one, else None
def PickDirectXSDK(sdks, version):
	byName = {}
	for sdk in sdks:
		byName[sdk["Name"].lower()] = sdk
	preferred = []
	if version == 8: # 2005, Windows 98 compatibility, DirectX 9.0c (October 2006)
		preferred = [ "october 2006" ]
	elif version == 9: # 2008, Windows 2000 compatibility, DirectX 9.0c (June 2008) and unofficially (February 2010)
		preferred = [ "february 2010", "june 2008" ]
	preferred += [ "june 2010" ]
	for name in preferred:
		if name in byName:
			return byName[name]
	return None

def HasXAudio2(path):
	if not path:
		return False
	return IsFileCI(path, "Include/XAudio2.h")

# The Windows SDKs of a drive: { "v6.0A": path, ... }
# "C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include\Msi.h"
def ScanWindowsSDKs(root):
	res = {}
	for programFiles in ProgramFilesDirs(root):
		sdkRoot = JoinCI(programFiles, "Microsoft SDKs/Windows")
		if sdkRoot is None or not os.path.isdir(sdkRoot):
			continue
		for entry in ListDir(sdkRoot):
			path = os.path.join(sdkRoot, entry)
			if entry not in res and (IsFileCI(path, "Include/Msi.h") or IsFileCI(path, "Include/Windows.h")):
				res[entry] = path
	return res

# The 3ds Max SDKs of a drive: [ { Version, Path } ], Path being the maxsdk folder
# "C:\Program Files\Autodesk\3ds Max 2010 SDK\maxsdk\include\max.h"
def ScanMaxSDKs(root):
	res = []
	for programFiles in ProgramFilesDirs(root):
		autodesk = JoinCI(programFiles, "Autodesk")
		if autodesk is None or not os.path.isdir(autodesk):
			continue
		for entry in ListDir(autodesk):
			lower = entry.lower()
			if not lower.startswith("3ds max ") or not lower.endswith(" sdk"):
				continue
			version = entry[len("3ds max "):-len(" sdk")].strip()
			if not version.isdigit():
				continue
			path = os.path.join(autodesk, entry)
			maxsdk = JoinCI(path, "maxsdk")
			if maxsdk is not None and IsFileCI(maxsdk, "include/max.h"):
				res += [ { "Version": int(version), "Path": maxsdk } ]
			elif IsFileCI(path, "include/max.h"):
				res += [ { "Version": int(version), "Path": path } ]
	return res

# end of file
