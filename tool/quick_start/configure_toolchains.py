
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
			toolchain["Hunter"] = vs["Version"] >= 14
		toolchain["CMake"] = []
		if vs["Version"] < 14:
			toolchain["CMake"] += [ "-DWINSDK_VERSION=6.0A" ]
		toolchain["EnvPath"] = FindBinPaths(toolchain["Prefix"])
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
		if platform == "x64":
			toolchain["VCVars"] = FindVCVars64(vs["Path"])
		else:
			toolchain["VCVars"] = FindVCVars32(vs["Path"])
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
				Toolchains[toolchain["OS"] + "/VS/" + ts + "/" + platform + "/H"] = copyToolchain

with open(os.path.join(NeLConfigDir, "toolchains_" + socket.gethostname().lower() + "_default.json"), 'w') as fo:
	json.dump(Toolchains, fo, indent=2)

if not os.path.isfile("toolchains_" + socket.gethostname().lower() + ".json"):
	with open(os.path.join(NeLConfigDir, "toolchains_" + socket.gethostname().lower() + ".json"), 'w') as fo:
		json.dump({}, fo, indent=2)

# end of file
