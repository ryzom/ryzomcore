
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
NelTargetPluginMax = {}
remapMaxCompatible = {}
foundMax = {}
for maxSdk in FoundMaxSDKs:
	if "Compatible" in maxSdk:
		# Skip unnecessary builds
		if maxSdk["Compatible"] in foundMax:
			continue
	filters = [ { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True, "Hunter": True }, { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True } ]
	foundTs = { "Toolchain": FindToolchainEx(filters), "Config": NeLConfig["Toolchain"]["PluginMax"], "MaxSDK": maxSdk, "DisplayName": "NeL Plugin Max DEV: " + str(maxSdk["Version"]) }
	if foundTs:
		foundMax[maxSdk["Version"]] = True
		if "Compatible" in maxSdk:
			foundMax[maxSdk["Compatible"]] = True
	NelTargetPluginMax[str(maxSdk["Version"]) + "_" + maxSdk["Platform"]] = foundTs
	del filters
	del foundTs
del remapMaxCompatible
del foundMax

# end of file
