
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

def FindDockerGCC(image):
	# FoundVisualStudio += [ { "Name": "Visual Studio " + str(VSMajor[yearVersion]) + " " + yearVersion, "DisplayName": "Visual Studio " + yearVersion + " " + edition, "Path": editionPath, "Version": VSMajor[yearVersion], "Toolset": "v140", "HasMFC": os.path.isfile("C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\atlmfc\\include\\afx.h") } ]
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
	cmd = baseCmd + [ "gcc", "-dumpfullversion", "-dumpversion" ]
	# print("TRY: " + str(cmd))
	try:
		output = subprocess.check_output(cmd, text=True) #, stderr=subprocess.DEVNULL, text=True)
	except subprocess.CalledProcessError as e:
		# print(e.output)
		print("FAILED: " + str(cmd))
		return
	except FileNotFoundError as e:
		print("FAILED: " + str(cmd))
		return
	version = output.strip()
	cmd = baseCmd + [ "cat", "/etc/os-release" ]
	try:
		output = subprocess.check_output(cmd, text=True) #, stderr=subprocess.DEVNULL, text=True)
	except subprocess.CalledProcessError as e:
		# print(e.output)
		print("FAILED: " + str(cmd))
		return
	except FileNotFoundError as e:
		print("FAILED: " + str(cmd))
		return
	os = dict(csv.reader(io.StringIO(output.strip()), delimiter='='))
	cmd = baseCmd + [ "find", "/usr/lib", "-name", "libluabind.so" ]
	try:
		output = subprocess.check_output(cmd, text=True) #, stderr=subprocess.DEVNULL, text=True)
	except subprocess.CalledProcessError as e:
		output = None
	except FileNotFoundError as e:
		output = None
	luaVer = None
	if output:
		luaSO = output.strip()
		cmd = baseCmd + [ "ldd", luaSO ]
		try:
			output = subprocess.check_output(cmd, text=True) #, stderr=subprocess.DEVNULL, text=True)
		except subprocess.CalledProcessError as e:
			# print(e.output)
			print("FAILED: " + str(cmd))
			return
		except FileNotFoundError as e:
			print("FAILED: " + str(cmd))
			return
		ldd = output.strip()
		# print(ldd)
		# Get the Lua version that Luabind depends on
		# If we can find that, then we assume development libraries are available
		ldd = ldd[ldd.index("liblua"):]
		ldd = ldd[:ldd.index(".so")]
		if len(ldd) > 6:
			luaVer = ldd[6:]
	name = "GCC " + version + " (" + os["PRETTY_NAME"]
	if luaVer:
		name += ", Lua " + luaVer
	name += ")"
	print(image + ": " + name)

for image in FoundDocker:
	print("Detect GCC in Docker image \"" + image + "\"")
	FindDockerGCC(image)

# end of file
