
# Populates the Wine prefixes of the Wine toolchain images. The packaged prefixes and externals listed
# under "WinePrefixes" in the configuration are downloaded into the <image>_prefixes volume (the archive
# of a prefix unpacks to a folder of its own name, the externals archives are bare and unpack into a
# folder of that name under drive_c), then the setup script of the toolchain layer prepares the prefix.
# Idempotent; configure_docker.py runs it after the images are built.

import sys, os, pathlib, subprocess
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *
from quick_start.find_docker import *

def WinePrefixScript(name, prefix):
	path = NeLWinePrefixesDir + "/" + name
	lines = [ "set -e" ]
	lines += [ 'if [ ! -d "' + path + '/drive_c" ]; then' ]
	lines += [ '  echo "DOWNLOAD ' + name + '"' ]
	lines += [ '  curl -sSfL -o "' + path + '.7z" "' + prefix["Url"] + '"' ]
	lines += [ '  7z x -y -o"' + NeLWinePrefixesDir + '" "' + path + '.7z" > /dev/null' ]
	lines += [ '  rm -f "' + path + '.7z"' ]
	lines += [ 'fi' ]
	if "Externals" in prefix:
		for extName in prefix["Externals"]:
			extPath = path + "/drive_c/" + extName
			lines += [ 'if [ ! -d "' + extPath + '" ]; then' ]
			lines += [ '  echo "DOWNLOAD ' + extName + '"' ]
			lines += [ '  curl -sSfL -o "' + path + '.ext.7z" "' + prefix["Externals"][extName] + '"' ]
			lines += [ '  mkdir -p "' + extPath + '"' ]
			lines += [ '  7z x -y -o"' + extPath + '" "' + path + '.ext.7z" > /dev/null' ]
			lines += [ '  rm -f "' + path + '.ext.7z"' ]
			lines += [ 'fi' ]
	if "Setup" in prefix:
		lines += [ 'echo "SETUP ' + name + '"' ]
		lines += [ prefix["SetupEnv"] + '="' + path + '" bash "' + DockerRootPath("code/" + prefix["Setup"]) + '"' ]
	return "\n".join(lines) + "\n"

def ConfigureWinePrefixes():
	images = WineDockerImages(FindDocker())
	if not len(images):
		return
	if not "WinePrefixes" in NeLConfig:
		return
	pathlib.Path(NeLTempDir).mkdir(parents=True, exist_ok=True)
	for image in images:
		for name in NeLConfig["WinePrefixes"]:
			print("WINE PREFIX " + name + " (" + image + ")")
			scriptName = "wine_prefix_" + name + ".sh"
			with open(os.path.join(NeLTempDir, scriptName), "w", newline="\n") as fo:
				fo.write(WinePrefixScript(name, NeLConfig["WinePrefixes"][name]))
			cmd = DockerWineCommand(image, None) + [ "bash", DockerRootPath(".nel/temp/" + scriptName) ]
			subprocess.run(cmd, check=True)

if __name__ == "__main__":
	if FoundDocker == None:
		print("ERROR: Docker Desktop is not running.")
	else:
		ConfigureWinePrefixes()

# end of file
