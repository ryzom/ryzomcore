
# Run the command `docker images` to check if docker is running, and list the images
# Docker must be in the system path
# Returns None if Docker is not found, otherwise a table of available images
# (Pre-check script will check the toolchain configuration if any Docker toolchains were previously detected and break if docker is not running)

import subprocess
import os

def FindDocker():
	try:
		output = subprocess.check_output(
			['docker', 'images', '--format', '{{.Repository}}\t{{.Tag}}\t{{.ID}}\t{{.CreatedSince}}\t{{.Size}}'],
			stderr=subprocess.DEVNULL, text=True)
	except subprocess.CalledProcessError as e:
		# print(e.output)
		return None
	except FileNotFoundError as e:
		return None
	entries = {}
	for line in output.splitlines():
		if not line.strip():
			continue
		parts = line.split('\t')
		if len(parts) < 5:
			continue
		repo = parts[0].strip()
		if not repo or repo == '<none>':
			continue
		entries[repo] = {
			"REPOSITORY": repo,
			"TAG": parts[1].strip(),
			"IMAGE": parts[2].strip(),
			"CREATED": parts[3].strip(),
			"SIZE": parts[4].strip(),
		}
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
		if not os.path.isfile(os.path.join(sdi, "Dockerfile")):
			continue
		res += [ di ]
	return res

FoundDocker = FindDocker()
FoundDockerImages = FindDockerImages()

def NeedConfigureDocker():
	if FoundDocker:
		for image in FoundDockerImages:
			if not image in FoundDocker:
				return True
		return False
	if not FoundDocker == None:
		return True
	return False
