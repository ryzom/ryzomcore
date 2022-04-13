
# Installs all the available docker images
# Manually run `docker system prune` afterwards to remove unused build caches

import sys, os, pathlib, subprocess
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *
from quick_start.find_docker import *

# Prepare temp directory
pathlib.Path(NeLTempDir).mkdir(parents=True, exist_ok=True)
prevCwd = os.getcwd()
dockerDir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "docker")

def NeedBuild(image):
	if image not in FoundDocker:
		return True
	created = FoundDocker[image]["CREATED"]
	if not created:
		return True
	if "second" in created or "minute" in created or "hour" in created:
		return False
	return True

# Prepare Steam Runtime
def BuildSteamRuntime(image, source, platform):
	if not NeedBuild(image):
		print("SKIP " + image)
		return
	print("DOCKER BUILD " + image)
	os.chdir(NeLTempDir)
	subprocess.run(["aria2c", "https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/" + source + ".Dockerfile"])
	subprocess.run(["aria2c", "https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-steam-client-general-availability/" + source + ".tar.gz"])
	subprocess.run(["docker", "build", "--platform", platform, "-f", source + ".Dockerfile", "-t", image + ":latest", "."])
	os.remove(source + ".Dockerfile")
	os.remove(source + ".tar.gz")

def BuildImage(image):
	if not NeedBuild(image):
		print("SKIP " + image)
		return
	print("DOCKER BUILD " + image)
	dockerFile = os.path.join(os.path.join(dockerDir, image), "Dockerfile")
	subprocess.run(["docker", "build",  "-t", image, "-f", dockerFile, NeLCodeDir])
	subprocess.run(["docker", "volume",  "rm", image + "_hunter"], stderr=subprocess.DEVNULL)

def BuildImages():
	for image in FoundDockerImages:
		BuildImage(image)

try:
	BuildSteamRuntime("steamrt_scout_amd64", "com.valvesoftware.SteamRuntime.Sdk-amd64,i386-scout-sysroot", "linux/amd64")
	BuildSteamRuntime("steamrt_scout_i386", "com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot", "linux/386")
	BuildImages()
finally:
	os.chdir(prevCwd)

# end of file
