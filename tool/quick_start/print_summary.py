
import sys, os
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

# Docker images

from quick_start.find_docker import *

print("Docker images:")
def printDockerImage(image):
	if not FoundDocker or not image in FoundDocker:
		print("  " + image + ": NOT FOUND")
	else:
		print("  " + image + ": " + FoundDocker[image]["IMAGE"] + " (" + FoundDocker[image]["CREATED"] + ", " + FoundDocker[image]["SIZE"] + ")")
printDockerImage("steamrt_scout_amd64")
printDockerImage("steamrt_scout_i386")
for image in FoundDockerImages:
	printDockerImage(image)

# Build targets

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

print()
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

# Notifications

if not FoundDocker:
	print()
	print("NOTIFICATION: Docker Desktop is not running. It is recommended to install Docker Desktop, which is required to build server and client binaries for Linux targets. (It is not required if you only plan to develop locally on Windows.)")
elif NeedConfigureDocker():
	print()
	print("NOTIFICATION: There are Docker build environment images available for Ryzom Core which have not yet been built. Run the `configure_docker` script to build all available Docker images.")

# end of file
