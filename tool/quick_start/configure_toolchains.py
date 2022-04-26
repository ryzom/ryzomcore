
# This script generates a configuration file listing all the available toolchains

import sys, os
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *
from quick_start.find_vstudio import *
from quick_start.find_external import *
from quick_start.find_gcc import *

Toolchains = {}


# Only include one VS version per toolset
ByToolset = {}
SortedToolsets = []
for vs in FoundVisualStudio:
	if not vs["Toolset"] in ByToolset:
		SortedToolsets += [ vs["Toolset"] ]
	if not vs["Toolset"] in ByToolset or (ByToolset[vs["Toolset"]]["Version"] < vs["Version"] and (not ByToolset[vs["Toolset"]]["HasMFC"] or vs["HasMFC"])):
		ByToolset[vs["Toolset"]] = vs;

VSPlatforms = [ "x86", "x64" ]
for ts in SortedToolsets:
	vs = ByToolset[ts]
	for platform in VSPlatforms:
		toolchain = {}
		toolchain["Compiler"] = "MSVC"
		toolchain["VSPath"] = vs["Path"]
		if vs["Version"] >= 9:
			toolchain["Generator"] = vs["Name"]
		else:
			# https://cmake.org/cmake/help/latest/generator/Visual%20Studio%208%202005.html
			# Must set VCVars ahead of compiling!
			toolchain["Generator"] = "NMake Makefiles"
		toolchain["DisplayName"] = vs["DisplayName"]
		toolchain["Platform"] = platform
		toolchain["Toolset"] = ts
		toolchain["Prefix"] = FindVSPrefixPaths(ts, platform)
		if not len(toolchain["Prefix"]) and vs["Version"] >= 14:
			toolchain["Hunter"] = True
		toolchain["CMake"] = []
		# Set the SDK version
		# https://en.wikipedia.org/wiki/Microsoft_Windows_SDK
		# C:\Program Files (x86)\Windows Kits\10
		if vs["Version"] < 14 and not ts.endswith("_xp"):
			if vs["Version"] >= 12: # 2013
				toolchain["CMake"] += [ "-DWINSDK_VERSION=8.1A" ]
			elif vs["Version"] >= 11:  # 2012
				toolchain["CMake"] += [ "-DWINSDK_VERSION=8.0A" ]
			elif vs["Version"] >= 10:  # 2010
				toolchain["CMake"] += [ "-DWINSDK_VERSION=7.0A" ]
			elif vs["Version"] >= 9:
				# C:\Program Files\Microsoft SDKs\Windows\v6.0A
				if os.path.isfile("C:\\Program Files\\Microsoft SDKs\\Windows\\v6.0A\\Include\\Msi.h"):
					toolchain["CMake"] += [ "-DWINSDK_DIR=C:\\Program Files\\Microsoft SDKs\\Windows\\v6.0A" ]
				toolchain["CMake"] += [ "-DWINSDK_VERSION=6.0A" ]
			else:
				# C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2
				if os.path.isfile("C:\\Program Files\\Microsoft Platform SDK for Windows Server 2003 R2\\Include\\Msi.h"):
					toolchain["CMake"] += [ "-DWINSDK_DIR=C:/Program Files/Microsoft Platform SDK for Windows Server 2003 R2" ]
				toolchain["CMake"] += [ "-DWINSDK_VERSION=5.2" ]
		#toolchain["EnvPath"] = FindBinPaths(toolchain["Prefix"])
		toolchain["EnvSet"] = []
		# For XP support, simply target SDK 7.1A
		if ts.endswith("_xp"):
			toolchain["CMake"] += [ "-DWINSDK_VERSION=7.1A" ]
			toolchain["EnvSet"] += [ "INCLUDE=%ProgramFiles(x86)%\\Microsoft SDKs\\Windows\\7.1A\\Include;%INCLUDE%" ]
			toolchain["EnvSet"] += [ "PATH=%ProgramFiles(x86)%\\Microsoft SDKs\\Windows\\7.1A\\Bin;%PATH%" ]
			toolchain["EnvSet"] += [ "LIB=%ProgramFiles(x86)%\\Microsoft SDKs\\Windows\\7.1A\\Lib;%LIB%" ]
			toolchain["EnvSet"] += [ "CL=/D_USING_V110_SDK71_;%CL%" ]
		# Hunter looks for this variable based on the toolset version, rather than the VS version
		# It must return the VS path, however, not the toolchain path
		if (vs["Toolset"] == "v141" or vs["Toolset"] == "v141_xp") and vs["Version"] > 15:
			toolchain["EnvSet"] += [ "VS150COMNTOOLS=" + os.path.normpath(os.path.join(vs["Path"], "Common7/Tools")) ]
		elif vs["Toolset"] == "v142" and vs["Version"] > 16:
			toolchain["EnvSet"] += [ "VS160COMNTOOLS=" + os.path.normpath(os.path.join(vs["Path"], "Common7/Tools")) ]
		elif vs["Toolset"] == "v143" and vs["Version"] > 17:
			toolchain["EnvSet"] += [ "VS170COMNTOOLS=" + os.path.normpath(os.path.join(vs["Path"], "Common7/Tools")) ]
		# Hunter doesn't build the libxml2 dependency correctly in static build mode
		if "Hunter" in toolchain:
			toolchain["EnvSet"] += [ "CL=/DLIBXML_STATIC;%CL%" ]
		toolchain["Version"] = vs["Version"]
		directXSdk = FindDirectXSDK(vs["Version"])
		if directXSdk:
			toolchain["DirectXSDK"] = directXSdk
		if HasXAudio2(directXSdk):
			toolchain["HasXAudio2"] = True
		if vs["HasMFC"]:
			toolchain["HasMFC"] = True
		luaVersion = FindLuaVersion(toolchain["Prefix"])
		if luaVersion:
			toolchain["LuaVersion"] = luaVersion
		toolchain["VCVars"] = FindVCVars(vs["Path"], vs["Toolset"], platform)
		if vs["Version"] >= 11:
			if toolchain["Toolset"].endswith("_xp"):
				toolchain["OS"] = "WinXP"
			else:
				toolchain["OS"] = "Win7"
		elif vs["Version"] >= 10:
			toolchain["OS"] = "WinXP"
		elif vs["Version"] >= 9:
			toolchain["OS"] = "Win2k"
		elif vs["Version"] >= 8:
			toolchain["OS"] = "Win98"
		else:
			continue
		if toolchain["VCVars"] and (len(toolchain["Prefix"]) or "Hunter" in toolchain):
			addHunter = ""
			if "Hunter" in toolchain:
				addHunter = "/H"
			Toolchains[toolchain["OS"] + "/VS/" + ts + "/" + platform + addHunter] = toolchain
			if not "Hunter" in toolchain and vs["Version"] >= 14:
				# Duplicate toolchain with Hunter externals for all newer VS versions
				copyToolchain = {}
				for k in toolchain:
					copyToolchain[k] = toolchain[k]
				copyToolchain["Hunter"] = True
				copyToolchain["Prefix"] = []
				#copyToolchain["EnvPath"] = []
				copyEnvSet = []
				copyEnvSet += copyToolchain["EnvSet"]
				copyEnvSet += [ "CL=/DLIBXML_STATIC;%CL%" ]
				copyToolchain["EnvSet"] = copyEnvSet
				if "LuaVersion" in copyToolchain:
					del copyToolchain["LuaVersion"]
				Toolchains[toolchain["OS"] + "/VS/" + ts + "/" + platform + "/H"] = copyToolchain

for gcc in FoundGCC:
	#print(gcc)
	toolchain = {}
	toolchain["Compiler"] = "GCC"
	toolchain["DisplayName"] = gcc["DisplayName"]
	toolchain["GCCVersion"] = gcc["GCCVersion"]
	toolchain["Version"] = int(gcc["GCCVersion"].split(".")[0])
	if "LuaVersion" in gcc:
		luaVer = gcc["LuaVersion"]
		if luaVer == "50":
			toolchain["LuaVersion"] = 500
		elif luaVer == "5.1":
			toolchain["LuaVersion"] = 501
		elif luaVer == "5.2":
			toolchain["LuaVersion"] = 502
		elif luaVer == "5.3":
			toolchain["LuaVersion"] = 503
		elif luaVer == "5.4":
			toolchain["LuaVersion"] = 504
	toolchain["CMake"] = []
	toolchain["OSId"] = gcc["OSRelease"]["ID"]
	toolchain["OSCodename"] = gcc["OSRelease"]["VERSION_CODENAME"]
	if "Native" in gcc and gcc["Native"]:
		toolchain["Native"] = True
	if "Docker" in gcc and gcc["Docker"]:
		toolchain["Docker"] = True
		toolchain["Image"] = gcc["Image"]
	toolchain["Platform"] = gcc["Architecture"]
	name = toolchain["OSId"] + "/" + toolchain["OSCodename"] + "/" + toolchain["Compiler"] + "/" + toolchain["Platform"]
	if "Docker" in toolchain:
		name += "/D"
	if "LuaVersion" in toolchain:
		Toolchains[name] = toolchain
	copyToolchain = {}
	for k in toolchain:
		copyToolchain[k] = toolchain[k]
	copyToolchain["Hunter"] = True
	if "LuaVersion" in copyToolchain:
		del copyToolchain["LuaVersion"]
	Toolchains[name + "/H"] = copyToolchain

with open(os.path.join(NeLConfigDir, "toolchains_" + NeLHostId + "_" + NeLPlatformId + "_default.json"), 'w') as fo:
	json.dump(Toolchains, fo, indent=2)

if not os.path.isfile("toolchains_" + NeLHostId + "_" + NeLPlatformId + ".json"):
	with open(os.path.join(NeLConfigDir, "toolchains_" + NeLHostId + "_" + NeLPlatformId + ".json"), 'w') as fo:
		json.dump({}, fo, indent=2)

print("Found " + str(len(Toolchains)) + " valid toolchain configurations")

# end of file
