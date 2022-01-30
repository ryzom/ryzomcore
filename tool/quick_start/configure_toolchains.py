
# This script generates a configuration file listing all the available toolchains

from common import *
from find_vstudio import *
from find_external import *

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
		if vs["Version"] < 14:
			toolchain["CMake"] += [ "-DWINSDK_VERSION=6.0A" ]
		toolchain["EnvPath"] = FindBinPaths(toolchain["Prefix"])
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
		if (vs["Toolset"] == "v140" or vs["Toolset"] == "v140_xp") and vs["Version"] > 14:
			toolchain["EnvSet"] += [ "VS140COMNTOOLS=" + os.path.normpath(os.path.join(vs["Path"], "Common7/Tools")) ]
		elif (vs["Toolset"] == "v141" or vs["Toolset"] == "v141_xp") and vs["Version"] > 15:
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
				copyToolchain["EnvPath"] = []
				copyToolchain["EnvSet"] += [ "CL=/DLIBXML_STATIC;%CL%" ]
				Toolchains[toolchain["OS"] + "/VS/" + ts + "/" + platform + "/H"] = copyToolchain

with open(os.path.join(NeLConfigDir, "toolchains_" + socket.gethostname().lower() + "_default.json"), 'w') as fo:
	json.dump(Toolchains, fo, indent=2)

if not os.path.isfile("toolchains_" + socket.gethostname().lower() + ".json"):
	with open(os.path.join(NeLConfigDir, "toolchains_" + socket.gethostname().lower() + ".json"), 'w') as fo:
		json.dump({}, fo, indent=2)

# end of file
