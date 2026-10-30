
# Probe of the Wine prefixes reachable from here: the Visual Studio installations, SDKs and externals in
# each prefix, as JSON on stdout. The quick start runs it inside the Wine Docker image (find_wine.py);
# it also runs by hand on a Linux box with Wine. Standard library only.
#
#   dump_wine.py [<prefix or folder of prefixes> ...]
#
# Without arguments: $WINEPREFIX, /opt/wineprefixes, ~/.wine, ~/.local/share/wineprefixes and ~/*_prefix

import subprocess, os, json, sys, re, glob

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))
from vstudio_scan import *
from find_external import FindPrefixPaths, FindLuaVersion, FindMsQuic

def IsWinePrefix(path):
	return os.path.isdir(os.path.join(path, "drive_c")) and os.path.isfile(os.path.join(path, "system.reg"))

def FindPrefixes(roots):
	res = []
	for root in roots:
		if not root or not os.path.isdir(root):
			continue
		if IsWinePrefix(root):
			if not root in res:
				res += [ root ]
			continue
		for entry in ListDir(root):
			path = os.path.join(root, entry)
			if IsWinePrefix(path) and not path in res:
				res += [ path ]
	return res

def DefaultRoots():
	home = os.path.expanduser("~")
	res = [ os.environ.get("WINEPREFIX"), "/opt/wineprefixes", os.path.join(home, ".wine"), os.path.join(home, ".local/share/wineprefixes") ]
	res += sorted(glob.glob(os.path.join(home, "*_prefix")))
	return res

# 2019q4_external_v90_x86
ExternalDirPattern = re.compile(r"^(.+)_external_(v[0-9]+(?:_xp)?)_(x86|x64|arm64)$", re.IGNORECASE)

def ProbePrefix(prefix):
	drive = os.path.join(prefix, "drive_c")
	res = {}
	res["Path"] = prefix
	res["Name"] = os.path.basename(prefix.rstrip("/"))
	res["ProgramFiles"] = ProgramFilesDirs(drive)
	res["VisualStudio"] = ScanVisualStudio(drive)
	res["WindowsSDK"] = ScanWindowsSDKs(drive)
	res["DirectXSDK"] = ScanDirectXSDKs(drive)
	res["MaxSDK"] = ScanMaxSDKs(drive)
	externals = []
	for entry in ListDir(drive):
		match = ExternalDirPattern.match(entry)
		if not match:
			continue
		path = os.path.join(drive, entry)
		if not os.path.isdir(path):
			continue
		prefixPaths = FindPrefixPaths(path)
		if not len(prefixPaths):
			continue
		external = { "Dir": path, "Toolset": match.group(2).lower(), "Platform": match.group(3).lower(), "Prefix": prefixPaths }
		luaVersion = FindLuaVersion(prefixPaths)
		if luaVersion:
			external["LuaVersion"] = luaVersion
		if FindMsQuic(prefixPaths):
			external["HasMsQuic"] = True
		externals += [ external ]
	# Newest release first for each toolset and platform, like FindVSPrefixPaths
	externals.sort(key=lambda e: e["Dir"], reverse=True)
	res["Externals"] = externals
	return res

def WineVersion():
	try:
		return subprocess.check_output([ "wine", "--version" ], stderr=subprocess.DEVNULL, text=True).strip()
	except (subprocess.CalledProcessError, OSError):
		return None

roots = sys.argv[1:]
if not len(roots):
	roots = DefaultRoots()

res = { "Prefixes": [] }
wineVersion = WineVersion()
if wineVersion:
	res["WineVersion"] = wineVersion
for prefix in FindPrefixes(roots):
	res["Prefixes"] += [ ProbePrefix(prefix) ]
print(json.dumps(res))

# end of file
