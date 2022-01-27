
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

NeLToolchainWin32 = FindToolchain({ "OS": "Win32" })
NeLToolchainWin64 = FindToolchain({ "OS": "Win64" })
NeLToolchainServer = FindToolchain(NeLConfig["Toolchain"]["Server"])

print("Win32:")
print(NeLToolchainWin32)
print("Win64:")
print(NeLToolchainWin64)
print("Server:")
print(NeLToolchainServer)

# end of file
