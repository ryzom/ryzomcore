
from .common_root import *

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

# end of file
