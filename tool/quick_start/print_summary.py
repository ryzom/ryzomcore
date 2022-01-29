
from find_toolchain import *
from find_max import *

def printBuildTarget(name, filters):
	tn = FindToolchainEx(filters)
	if tn:
		tc = NeLToolchains[tn]
		withHunter = ""
		if "Hunter" in tc and tc["Hunter"]:
			withHunter = ", Hunter"
		print("  " + name + ": " + tc["DisplayName"] + " (" + tc["Generator"] + ", " + tc["Toolset"] + ", " + tc["Platform"] + ")" + withHunter)
	else:
		print("  " + name + ": NOT FOUND")

print("Build targets:")
printBuildTarget("client_dev", NeLToolchainNative)
printBuildTarget("server_dev", NeLToolchainNative)
for client in NeLConfig["Toolchain"]["Client"]:
	printBuildTarget(client, NeLConfig["Toolchain"]["Client"][client])
printBuildTarget("server", NeLConfig["Toolchain"]["Server"])
printBuildTarget("tools", NeLToolchainNative)
printBuildTarget("samples", NeLToolchainNative)
remapMaxCompatible = {}
foundMax = {}
for maxSdk in FoundMaxSDKs:
	if "Compatible" in maxSdk:
		# Skip unnecessary builds
		if maxSdk["Compatible"] in foundMax:
			continue
	filters = [ { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True, "Hunter": True }, { "Toolset": maxSdk["Toolset"], "Platform": maxSdk["Platform"], "HasMFC": True } ]
	if FindToolchainEx(filters):
		foundMax[maxSdk["Version"]] = True
		if "Compatible" in maxSdk:
			foundMax[maxSdk["Compatible"]] = True
	printBuildTarget("plugin_max/" + str(maxSdk["Version"]) + "_" + maxSdk["Platform"], filters)
# plugin_max

# end of file
