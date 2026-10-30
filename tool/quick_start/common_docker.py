from .common_root import *

def DockerBaseCommand(image, arch, workdir, hunter, interactive=False):
	res = [ "docker", "run" ]
	res += [ "--rm" ]
	if hunter:
		res += [ "-v", image + "_hunter:/root/.hunter" ]
	res += [ "-v", image + "_ccache:/root/.ccache" ]
	res += [ "--mount", "type=bind,source=" + NeLRootDir + ",target=/mnt/nel" ]
	if workdir:
		res += [ "--workdir", "/mnt/nel/" + workdir.replace("\\", "/") ]
	else:
		res += [ "--workdir", "/mnt/nel/.nel/temp" ]
	if interactive:
		res += [ "-it" ]
	res += [ image ]
	if arch == "386":
		res += [ "setarch", "i686" ]
	return res

# The cross-platform tools image: cargo keeps its registry and the Windows SDK of cargo-xwin in
# volumes, and leaves the build output under the build directory
def DockerXplatCommand(image, buildDir, interactive=False):
	res = [ "docker", "run" ]
	res += [ "--rm" ]
	res += [ "-v", image + "_cargo:/usr/local/cargo/registry" ]
	res += [ "-v", image + "_xwin:/root/.cache/cargo-xwin" ]
	res += [ "-e", "CARGO_TARGET_DIR=/mnt/nel/" + buildDir.replace("\\", "/") ]
	res += [ "--mount", "type=bind,source=" + NeLRootDir + ",target=/mnt/nel" ]
	res += [ "--workdir", "/mnt/nel/code" ]
	if interactive:
		res += [ "-it" ]
	res += [ image ]
	return res

# Where a Wine image keeps its prefixes: the <image>_prefixes volume, one prefix per folder
NeLWinePrefixesDir = "/opt/wineprefixes"

# A Wine toolchain image: the prefixes volume, plus the environment the toolchain layer reads
def DockerWineCommand(image, workdir, env={}, interactive=False):
	res = [ "docker", "run" ]
	res += [ "--rm" ]
	res += [ "-v", image + "_prefixes:" + NeLWinePrefixesDir ]
	for name in env:
		res += [ "-e", name + "=" + env[name] ]
	res += [ "--mount", "type=bind,source=" + NeLRootDir + ",target=/mnt/nel" ]
	if workdir:
		res += [ "--workdir", "/mnt/nel/" + workdir.replace("\\", "/") ]
	else:
		res += [ "--workdir", "/mnt/nel/.nel/temp" ]
	if interactive:
		res += [ "-it" ]
	res += [ image ]
	return res

def DockerRootPath(path):
	return "/mnt/nel/" + path

# end of file
