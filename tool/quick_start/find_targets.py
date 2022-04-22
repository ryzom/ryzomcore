
from .find_toolchain import *
from .find_max import *

NeLToolchainNative = NeLConfig["Toolchain"]["Native"]

NeLTargetClientDev = { "Toolchain": FindToolchainEx(NeLToolchainNative["Filter"]), "Config": NeLToolchainNative }
NeLTargetServerDev = { "Toolchain": FindToolchainEx(NeLToolchainNative["Filter"]), "Config": NeLToolchainNative }
NeLTargetClient = {}
for client in NeLConfig["Toolchain"]["Client"]:
	NeLTargetClient[client] = { "Toolchain": FindToolchainEx(NeLConfig["Toolchain"]["Client"][client]["Filter"]), "Config": NeLConfig["Toolchain"]["Client"][client] }
NeLTargetServer = { "Toolchain": FindToolchainEx(NeLConfig["Toolchain"]["Server"]["Filter"]), "Config": NeLConfig["Toolchain"]["Server"] }
NeLTargetTools = { "Toolchain": FindToolchainEx(NeLToolchainNative["Filter"]), "Config": NeLToolchainNative }
NeLTargetSamples = { "Toolchain": FindToolchainEx(NeLToolchainNative["Filter"]), "Config": NeLToolchainNative }
NelTargetPluginMax = {}
remapMaxCompatible = {}
foundMax = {}
for maxSdk in FoundMaxSDKs:
	if "Compatible" in maxSdk:
		# Skip unnecessary builds
		if maxSdk["Compatible"] in foundMax:
			continue
	filters = [ { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True, "Hunter": True }, { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True } ]
	foundTs = { "Toolchain": FindToolchainEx(filters), "Config": NeLConfig["Toolchain"]["PluginMax"], "MaxSDK": maxSdk }
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
