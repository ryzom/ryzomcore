
# This script finds the Windows toolchains under Wine: every Wine image of the quick start is asked to
# probe the prefixes of its volume (dump_wine.py), which reports the Visual Studio installations, SDKs
# and externals in each prefix. One entry per prefix:
# [ { Image, WineVersion, Path, Name, ProgramFiles, VisualStudio, WindowsSDK, DirectXSDK, MaxSDK, Externals } ]

import subprocess, os, json, sys

from .common import *
from .find_docker import *

FoundWine = []

def FindDockerWine(image):
	global FoundWine

	cmd = DockerWineCommand(image, None) + [ "python3", DockerRootPath("code/tool/quick_start/dump_wine.py"), NeLWinePrefixesDir ]
	try:
		output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, text=True)
	except subprocess.CalledProcessError as e:
		print("FAILED: " + str(cmd))
		if e.output:
			print(e.output.strip())
		return
	except FileNotFoundError as e:
		print("FAILED: " + str(cmd))
		return

	try:
		res = json.loads(output.strip())
	except ValueError:
		print("FAILED: " + str(cmd))
		print(output.strip())
		return
	for prefix in res["Prefixes"]:
		prefix["Image"] = image
		if "WineVersion" in res:
			prefix["WineVersion"] = res["WineVersion"]
		FoundWine += [ prefix ]

for image in WineDockerImages():
	print("Find Wine prefixes in Docker image \"" + image + "\"")
	FindDockerWine(image)

# end of file
