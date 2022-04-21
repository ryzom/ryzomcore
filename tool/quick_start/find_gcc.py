
import subprocess, os, json, csv, io, sys

from .common import *
from .find_docker import *

FoundGCC = []

def FindLocalGCC():
	global FoundGCC
	
	output = None
	cmd = [ sys.executable, os.path.join(NeLCodeDir, os.path.normcase("tool/quick_start/dump_gcc.py")) ]
	try:
		output = subprocess.check_output(cmd, stderr=subprocess.DEVNULL, text=True)
	except subprocess.CalledProcessError as e:
		# print(e.output)
		# if "Failed" in e.output:
		# 	print("FAILED: PYTHON: " + str(json.loads(e.output)["Failed"]))
		# else:
		# 	print("FAILED: " + str(cmd))
		return
	except FileNotFoundError as e:
		print("FAILED: " + str(cmd))
		return
	
	res = json.loads(output.strip())
	res["Architecture"] = "amd64"
	res["Native"] = True
	
	name = "GCC " + res["GCCVersion"] + " " + arch + " (" + res["OSRelease"]["PRETTY_NAME"]
	if "LuaVersion" in res:
		name += ", Lua " + res["LuaVersion"]
	name += ")"
	# print(name)
	
	res["DisplayName"] = name
	FoundGCC += [ res ]

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
			print("FAILED: PYTHON: " + str(json.loads(e.output)["Failed"]))
		else:
			print("FAILED: " + str(cmd))
		return
	except FileNotFoundError as e:
		print("FAILED: " + str(cmd))
		return
	
	res = json.loads(output.strip())
	res["Architecture"] = arch
	res["Image"] = image
	res["Docker"] = True
	
	name = "GCC " + res["GCCVersion"] + " " + arch + " (" + res["OSRelease"]["PRETTY_NAME"]
	if "LuaVersion" in res:
		name += ", Lua " + res["LuaVersion"]
	name += ")"
	# print(name)
	
	res["DisplayName"] = name
	FoundGCC += [ res ]

print("Find native GCC installation")
FindLocalGCC()

if FoundDocker:
	for image in FoundDocker:
		if image in FoundDockerImages:
			print("Find GCC installation in Docker image \"" + image + "\"")
			FindDockerGCC(image)

# end of file
