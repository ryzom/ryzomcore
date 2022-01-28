
# This script finds the best toolchains for each purpose

from common import *

def FindToolchain(filter):
	toolchains = {}
	for tsn in NeLToolchains:
		ts = NeLToolchains[tsn]
		okay = True
		for k in filter:
			if ts[k] != filter[k]:
				okay = False
				break
		if okay:
			toolchains[tsn] = ts
	preference = NeLConfig["Toolchain"]["Preference"]
	for pn in preference:
		if pn in toolchains:
			return pn
	res = None
	bestVersion = 0
	for tsn in toolchains:
		ts = NeLToolchains[tsn]
		if ts["Version"] > bestVersion:
			res = tsn
	return res

NeLToolchainWin32 = FindToolchain({ "OS": "Win98", "Platform": "x86" })
if not NeLToolchainWin32:
	NeLToolchainWin32 = FindToolchain({ "OS": "Win2k", "Platform": "x86" })
if not NeLToolchainWin32:
	NeLToolchainWin32 = FindToolchain({ "OS": "WinXP", "Platform": "x86" })
if not NeLToolchainWin32:
	NeLToolchainWin32 = FindToolchain({ "OS": "Win7", "Platform": "x86" })
NeLToolchainWin64 = FindToolchain({ "OS": "WinXP", "Platform": "x64" })
if not NeLToolchainWin64:
	NeLToolchainWin64 = FindToolchain({ "OS": "Win7", "Platform": "x64" })
NeLToolchainServer = FindToolchain(NeLConfig["Toolchain"]["Server"])

print("Win32:")
print(NeLToolchainWin32)
print("Win64:")
print(NeLToolchainWin64)
print("Server:")
print(NeLToolchainServer)

# end of file
