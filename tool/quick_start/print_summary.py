
import sys, os
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.find_targets import *

def printBuildTarget(name, tn):
	if tn:
		tc = NeLToolchains[tn]
		withHunter = ""
		if "Hunter" in tc and tc["Hunter"]:
			withHunter = ", Hunter"
		print("  " + name + ": " + tc["DisplayName"] + " (" + tc["Generator"] + ", " + tc["Toolset"] + ", " + tc["Platform"] + ")" + withHunter)
	else:
		print("  " + name + ": NOT FOUND")

print("Build targets:")
printBuildTarget("client_dev", NeLTargetClientDev)
printBuildTarget("server_dev", NeLTargetServerDev)
for client in NeLTargetClient:
	printBuildTarget(client, NeLTargetClient[client])
printBuildTarget("server", NeLTargetServer)
printBuildTarget("tools", NeLTargetTools)
printBuildTarget("samples", NeLTargetSamples)
for pluginMax in NelTargetPluginMax:
	printBuildTarget("plugin_max/" + pluginMax, NelTargetPluginMax[pluginMax])

# end of file
