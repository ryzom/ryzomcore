
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
		toolchain["Generator"] = vs["Name"]
		toolchain["Platform"] = platform
		toolchain["Toolset"] = ts
		toolchain["Prefix"] = FindVSPrefixPaths(ts, platform)
		if not len(toolchain["Prefix"]):
			toolchain["Hunter"] = True
		toolchain["Version"] = vs["Version"]
		if platform == "x64":
			toolchain["OS"] = "Win64"
			toolchain["VCVars"] = FindVCVars64(vs["Path"])
		else:
			toolchain["OS"] = "Win32"
			toolchain["VCVars"] = FindVCVars32(vs["Path"])
		Toolchains["MSVC/" + ts + "/" + platform] = toolchain

with open(os.path.join(NeLConfigDir, "toolchains_default.json"), 'w') as fo:
	json.dump(Toolchains, fo, indent=2)

# end of file
