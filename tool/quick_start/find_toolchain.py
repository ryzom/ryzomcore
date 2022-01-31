
# This script finds the best toolchains for each purpose

from .common import *

def FindToolchain(filter):
	toolchains = {}
	for tsn in NeLToolchains:
		ts = NeLToolchains[tsn]
		okay = True
		for k in filter:
			if not ((not k in ts and not filter[k]) or (k in ts and ts[k] == filter[k])):
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

def FindToolchainEx(filters):
	for filter in filters:
		res = FindToolchain(filter)
		if res:
			return res
	return

NeLToolchainNative = [
	{ "Native": True, "Hunter": False }, # G++ on Linux with system packages
	{ "OS": "Win7", "Platform": "x64", "Hunter": False }, # Fallback, VS on Windows
	{ "OS": "Win7", "Platform": "x64" }
]

# end of file
