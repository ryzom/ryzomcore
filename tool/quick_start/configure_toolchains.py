
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
	if not vs["Toolset"] in ByToolset or ByToolset[vs["Toolset"]]["Version"] < vs["Version"]:
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
		if platform == "x64":
			toolchain["OS"] = "Win64"
			toolchain["VCVars"] = FindVCVars64(vs["Path"])
		else:
			toolchain["OS"] = "Win32"
			toolchain["VCVars"] = FindVCVars32(vs["Path"])
		if toolchain["VCVars"] and (len(toolchain["Prefix"]) or "Hunter" in toolchain):
			Toolchains["MSVC/" + ts + "/" + platform] = toolchain

with open(os.path.join(NeLConfigDir, "toolchains_default.json"), 'w') as fo:
	json.dump(Toolchains, fo, indent=2)

# end of file
