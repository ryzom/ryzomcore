
from .find_toolchain import *
from .find_max import *

NeLToolchainNative = NeLConfig["Toolchain"]["Native"]

NeLTargetClientDev = { "Toolchain": FindToolchainEx(NeLToolchainNative["Filter"]), "Config": NeLToolchainNative, "DisplayName": "Ryzom Core Client DEV" }
NeLTargetServerDev = { "Toolchain": FindToolchainEx(NeLToolchainNative["Filter"]), "Config": NeLToolchainNative, "DisplayName": "Ryzom Core Server DEV" }
NeLTargetClient = {}
for client in NeLConfig["Toolchain"]["Client"]:
	NeLTargetClient[client] = { "Toolchain": FindToolchainEx(NeLConfig["Toolchain"]["Client"][client]["Filter"]), "Config": NeLConfig["Toolchain"]["Client"][client], "DisplayName": "Ryzom Core Client FV: " + client }
NeLTargetServer = { "Toolchain": FindToolchainEx(NeLConfig["Toolchain"]["Server"]["Filter"]), "Config": NeLConfig["Toolchain"]["Server"], "DisplayName": "Ryzom Core Server FV" }
NeLTargetTools = { "Toolchain": FindToolchainEx(NeLToolchainNative["Filter"]), "Config": NeLToolchainNative, "DisplayName": "Ryzom Core Tools DEV" }
NeLTargetSamples = { "Toolchain": FindToolchainEx(NeLToolchainNative["Filter"]), "Config": NeLToolchainNative, "DisplayName": "Ryzom Core Samples DEV" }
# The Max SDKs: the native ones, and the ones in the Wine prefixes of the Windows toolchains under Wine (Max 2010
# with Visual Studio 2008). A Wine SDK is only reachable from the toolchain of its own prefix, a native SDK only from
# a native toolchain. The native SDKs come first, a version that gets a native toolchain is not built again under
# Wine (DescribeMaxSDKs keeps the order within a version).
def FindAllMaxSDKs():
	found = []
	for sdk in FoundMaxSDKs:
		found += [ { "Path": sdk["Path"], "Version": sdk["Version"] } ]
	for tsn in sorted(NeLToolchains):
		tc = NeLToolchains[tsn]
		if not ("Wine" in tc and tc["Wine"]) or not "MaxSDK" in tc:
			continue
		for sdk in tc["MaxSDK"]:
			found += [ { "Path": sdk["Path"], "Version": sdk["Version"], "WinePrefix": tc["WinePrefix"] } ]
	return DescribeMaxSDKs(found)

# Only the Max versions listed in Toolchain/PluginMax/Versions get a plugin build (the plugins for the later versions
# do not work at the moment); a version that is binary compatible with a listed one counts as listed
NeLPluginMaxVersions = None
if "Versions" in NeLConfig["Toolchain"]["PluginMax"]:
	NeLPluginMaxVersions = NeLConfig["Toolchain"]["PluginMax"]["Versions"]

NelTargetPluginMax = {}
foundMax = {}
for maxSdk in FindAllMaxSDKs():
	if NeLPluginMaxVersions != None and not maxSdk["Version"] in NeLPluginMaxVersions and not ("Compatible" in maxSdk and maxSdk["Compatible"] in NeLPluginMaxVersions):
		continue
	if "Compatible" in maxSdk:
		# Skip unnecessary builds
		if maxSdk["Compatible"] in foundMax:
			continue
	filters = [ { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True, "Hunter": True }, { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True } ]
	for filter in filters:
		if "WinePrefix" in maxSdk:
			filter["WinePrefix"] = maxSdk["WinePrefix"]
		else:
			filter["Wine"] = False
	targetName = str(maxSdk["Version"]) + "_" + maxSdk["Platform"]
	if targetName in NelTargetPluginMax and NelTargetPluginMax[targetName]["Toolchain"]:
		continue
	foundTs = { "Toolchain": FindToolchainEx(filters), "Config": NeLConfig["Toolchain"]["PluginMax"], "MaxSDK": maxSdk, "DisplayName": "NeL Plugin Max DEV: " + str(maxSdk["Version"]) }
	if foundTs["Toolchain"]:
		foundMax[maxSdk["Version"]] = True
		if "Compatible" in maxSdk:
			foundMax[maxSdk["Compatible"]] = True
	NelTargetPluginMax[targetName] = foundTs
	del filters
	del foundTs
del foundMax

# end of file
