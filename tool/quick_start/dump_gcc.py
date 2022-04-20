
import subprocess, os, json, csv, io

output = None
cmd = [ "gcc", "-dumpfullversion", "-dumpversion" ]
try:
	output = bytes.decode(subprocess.check_output(cmd))
except subprocess.CalledProcessError as e:
	# print(e.output)
	print(json.dumps({"Failed": cmd}))
	exit(1)
except OSError as e:
	print(json.dumps({"Failed": cmd}))
	exit(1)
version = output.strip()

cmd = [ "cat", "/etc/os-release" ]
try:
	output = bytes.decode(subprocess.check_output(cmd))
except subprocess.CalledProcessError as e:
	# print(e.output)
	print(json.dumps({"Failed": cmd}))
	exit(1)
except OSError as e:
	print(json.dumps({"Failed": cmd}))
	exit(1)
os = dict(csv.reader(io.StringIO(output.strip()), delimiter='='))

cmd = [ "find", "/usr/lib", "-name", "libluabind.so" ]
try:
	output = bytes.decode(subprocess.check_output(cmd))
except subprocess.CalledProcessError as e:
	output = None
except OSError as e:
	output = None
luaVer = None
if output:
	luaSO = output.strip()
	cmd = [ "ldd", luaSO ]
	try:
		output = bytes.decode(subprocess.check_output(cmd))
	except subprocess.CalledProcessError as e:
		output = None
	except OSError as e:
		output = None
	if output:
		ldd = output.strip()
		# print(ldd)
		# Get the Lua version that Luabind depends on
		# If we can find that, then we assume development libraries are available
		ldd = ldd[ldd.index("liblua"):]
		ldd = ldd[:ldd.index(".so")]
		if len(ldd) > 6:
			luaVer = ldd[6:]

# name = "GCC " + version + " (" + os["PRETTY_NAME"]
# if luaVer:
# 	name += ", Lua " + luaVer
# name += ")"

res = { "GCCVersion": version, "OSRelease": os }
if luaVer:
	res["LuaVersion"] = luaVer
print(json.dumps(res))

# end of file
