
import subprocess, os, json, csv, io

from .common_root import *
from .find_docker import *

FoundGCC = []

def DockerBaseCommand(image, arch, workdir, hunter):
	res = [ "docker", "run" ]
	if arch == "386":
		res += [ "--platform", "linux/386" ]
	res += [ "--rm" ]
	if hunter:
		res += [ "-v", image + "_hunter:/root/.hunter" ]
	res += [ "--mount", "type=bind,source=" + NeLRootDir + ",target=/mnt/nel" ]
	if workdir:
		res += [ "--workdir", "/mnt/nel/" + workdir ]
	else:
		res += [ "--workdir", "/mnt/nel/.nel/temp" ]
	res += [ image ]
	if arch == "386":
		res += [ "setarch", "i686" ]
	return res

def DockerRootPath(path):
	return "/mnt/nel/" + path

def FindDockerGCC(image):
	global FoundGCC
	
	output = None
	try:
		output = subprocess.check_output(['docker', 'image', 'inspect', image], stderr=subprocess.DEVNULL, text=True)
	except subprocess.CalledProcessError as e:
		# print(e.output)
		print("FAILED: " + image)
		return
	except FileNotFoundError as e:
		print("FAILED: " + image)
		return
	inspect = json.loads(output)
	arch = inspect[0]["Architecture"]
	baseCmd = DockerBaseCommand(image, arch, None, None)
	
	cmd = baseCmd + [ "python3", DockerRootPath("code/tool/quick_start/dump_gcc.py") ]
	try:
		output = subprocess.check_output(cmd, stderr=subprocess.DEVNULL, text=True)
	except subprocess.CalledProcessError as e:
		# print(e.output)
		if "Failed" in e.output:
			print("FAILED: PYTHON: " + json.loads(e.output)["Failed"])
		else:
			print("FAILED: " + str(cmd))
		return
	except FileNotFoundError as e:
		print("FAILED: " + str(cmd))
		return
	
	res = json.loads(output.strip())
	res["Architecture"] = arch
	res["Image"] = image
	
	name = "GCC " + res["GCCVersion"] + " " + arch + " (" + res["OSRelease"]["PRETTY_NAME"]
	if "LuaVersion" in res:
		name += ", Lua " + res["LuaVersion"]
	name += ")"
	# print(name)
	
	res["DisplayName"] = name
	FoundGCC += [ res ]

for image in FoundDocker:
	print("Detect GCC installation in Docker image \"" + image + "\"")
	FindDockerGCC(image)

# end of file
