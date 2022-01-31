
from .find_toolchain import *
from .find_max import *

NeLTargetClientDev = FindToolchainEx(NeLToolchainNative)
NeLTargetServerDev = FindToolchainEx(NeLToolchainNative)
NeLTargetClient = {}
for client in NeLConfig["Toolchain"]["Client"]:
	NeLTargetClient[client] = FindToolchainEx(NeLConfig["Toolchain"]["Client"][client])
NeLTargetServer = FindToolchainEx(NeLConfig["Toolchain"]["Server"])
NeLTargetTools = FindToolchainEx(NeLToolchainNative)
NeLTargetSamples = FindToolchainEx(NeLToolchainNative)
NelTargetPluginMax = {}
remapMaxCompatible = {}
foundMax = {}
for maxSdk in FoundMaxSDKs:
	if "Compatible" in maxSdk:
		# Skip unnecessary builds
		if maxSdk["Compatible"] in foundMax:
			continue
	filters = [ { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True, "Hunter": True }, { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True } ]
	foundTs = FindToolchainEx(filters)
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