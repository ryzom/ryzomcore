
# Check the toolchain configuration if any Docker toolchains were previously detected and break if docker is not running

import sys, os
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *

def HadDocker():
	for toolchain in NeLToolchains:
		if "Docker" in NeLToolchains[toolchain]:
			return True
	return False

if HadDocker():
	from quick_start.find_docker import *
	if not FoundDocker:
		print("ERROR: Docker Desktop is not running or has not been configured")
		sys.exit(1)

# end of file
