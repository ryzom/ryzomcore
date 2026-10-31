
import subprocess, os, json, csv, io, sys, glob
from find_external import LuaHeaderVersion

output = None
cmd = [ "gcc", "-dumpfullversion", "-dumpversion" ]
try:
	output = bytes.decode(subprocess.check_output(cmd))
except subprocess.CalledProcessError as e:
	# print(e.output)
	print(json.dumps({"Failed": cmd}))
	sys.exit(1)
except OSError as e:
	print(json.dumps({"Failed": cmd}))
	sys.exit(1)
version = output.strip()

cmd = [ "cat", "/etc/os-release" ]
try:
	output = bytes.decode(subprocess.check_output(cmd))
except subprocess.CalledProcessError as e:
	# print(e.output)
	print(json.dumps({"Failed": cmd}))
	sys.exit(1)
except OSError as e:
	print(json.dumps({"Failed": cmd}))
	sys.exit(1)
osRelease = dict(csv.reader(io.StringIO(output.strip()), delimiter='='))

# The ryzom/luabind fork, which the build images install in /usr/local as a static library (the luabind package of
# Ubuntu does not work). Its Lua is the newest one installed, which luabind's own find_package(Lua) and the FindLua
# of the NeL build both take; when Lua headers are found, development libraries are assumed to be available.
luaVer = None
if glob.glob("/usr/local/lib/libluabind09.*") + glob.glob("/usr/lib/libluabind09.*") + glob.glob("/usr/lib/*/libluabind09.*"):
	luaNums = []
	for luaHeader in glob.glob("/usr/include/lua5.*/lua.h") + glob.glob("/usr/include/lua.h") + glob.glob("/usr/local/include/lua.h"):
		luaNum = LuaHeaderVersion(luaHeader)
		if luaNum:
			luaNums += [ luaNum ]
	if luaNums:
		luaVer = str(max(luaNums) // 100) + "." + str(max(luaNums) % 100)

# name = "GCC " + version + " (" + osRelease["PRETTY_NAME"]
# if luaVer:
# 	name += ", Lua " + luaVer
# name += ")"

# Check if MsQuic header is available
hasMsQuic = os.path.isfile("/usr/include/msquic.h")

res = { "GCCVersion": version, "OSRelease": osRelease }
if luaVer:
	res["LuaVersion"] = luaVer
if hasMsQuic:
	res["HasMsQuic"] = True
print(json.dumps(res))

# end of file
