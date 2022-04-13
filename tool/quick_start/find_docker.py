
# Run the command `docker images` to check if docker is running, and list the images
# Docker must be in the system path
# Returns None if Docker is not found, otherwise a table of available images
# (Pre-check script will check the toolchain configuration if any Docker toolchains were previously detected and break if docker is not running)

import subprocess
import os

def FindDocker():
	output = None
	try:
		output = subprocess.check_output(['docker', 'images'], stderr = subprocess.DEVNULL, text = True)
	except subprocess.CalledProcessError as e:
		# print(e.output)
		return None
	except FileNotFoundError as e:
		return None
	lines = output.splitlines()
	headers = lines[0].split()
	entries = {}
	offsets = []
	for header in headers:
		offsets += [ lines[0].find(header) ]
	# print(headers)
	# print(offsets)
	# print(lines)
	# print(len(lines))
	for i in range(1, len(lines)):
		entry = {}
		for h in range(0, len(headers)):
			value = None
			offset = offsets[h]
			if h + 1 < len(headers):
				end = offsets[h + 1]
				value = lines[i][offset:end]
			else:
				value = lines[i][offset:]
			value = value.strip()
			entry[headers[h]] = value
		entries[entry["REPOSITORY"]] = entry
	# print(entries)
	# if not len(entries):
	# 	return None
	return entries

def FindDockerImages():
	res = []
	qsDir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "docker")
	for di in os.listdir(qsDir):
		if di.startswith("."):
			continue
		sdi = os.path.join(qsDir, di)
		if not os.path.isdir(sdi):
			continue
		res += [ di ]
	return res

FoundDocker = FindDocker()
FoundDockerImages = FindDockerImages()

# if (FoundDocker):
# 	print(FoundDocker)
