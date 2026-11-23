
# This script finds all Visual Studio installations on the native drive
# [ { DisplayName, Path, HasMFC } ]
# The scan itself is in vstudio_scan.py, shared with the Wine prefix probe (dump_wine.py)

import os

from .vstudio_scan import *

print("Find all Visual Studio installations")

FoundVisualStudio = []
if os.name == "nt":
	FoundVisualStudio = ScanVisualStudio("C:\\")

NeLVSToolsetMap = ToolsetMap(FoundVisualStudio)

def FindVCVars(path, toolset, platform):
	global NeLVSToolsetMap
	auxVars = os.path.join(path, "VC\\Auxiliary\\Build\\vcvarsall.bat")
	if os.path.isfile(auxVars):
		return [ auxVars, platform, "-vcvars_ver=" + NeLVSToolsetMap[toolset] ]
	allVars = os.path.join(path, "VC\\vcvarsall.bat")
	if os.path.isfile(allVars):
		return [ allVars, platform ]
	return

# FindDirectX SDK, optimized for compatibility per version. If prefered SDK not available, just return blank
# https://archive.org/details/directxsdks
def FindDirectXSDK(version):
	if os.name != "nt":
		return
	res = PickDirectXSDK(ScanDirectXSDKs("C:\\"), version)
	if res:
		return res["Path"]
	dxSdkDir = os.getenv('DXSDK_DIR')
	if dxSdkDir and os.path.isfile(os.path.join(dxSdkDir, "Include\\d3d9.h")):
		return dxSdkDir
	return

#print(FoundVisualStudio)
