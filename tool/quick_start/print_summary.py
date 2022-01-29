
from find_toolchain import *

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
# plugin_max

# end of file
