
# This generates the build scripts for all targets

import sys, os
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *
from quick_start.find_targets import *

NeLSpecClient = 1
NeLSpecServer = 2
NeLSpecTools = 3
NeLSpecSamples = 4
NeLSpecPluginMax = 5

Targets = {
	"Native": { },
	"Client": { },
	"PluginMax": { }
}

def EscapeArg(arg):
	return '"' + arg.replace('"', r'\"') + '"'

def EscapeArgOpt(arg):
	if " " in arg:
		return '"' + arg.replace('"', r'\"') + '"'
	return arg

def GeneratePathScript():
	fo = open(NeLPathScript, 'w')
	fo.write("set " + EscapeArg("RC_CODE_DIR=" + NeLCodeDir) + "\n")
	envPath = ""
	for path in NeLEnvPaths:
		envPath += path + os.pathsep
	fo.write("set " + EscapeArg("RC_PATH=" + envPath[:-1]) + "\n")
	fo.close()

def GenerateMsvcEnv(file, buildDir, tc):
	fo = open(file, 'w')
	fo.write("rem " + tc["DisplayName"] + "\n")
	fo.write("cd /d " + EscapeArg(tc["VSPath"]) + "\n")
	fo.write("call")
	for v in tc["VCVars"]:
		fo.write(" " + EscapeArgOpt(v))
	fo.write("\n")
	fo.write("cd /d " + EscapeArg(buildDir) + "\n")
	fo.write("call " + EscapeArg(NeLPathScript) + "\n")
	fo.write("set PATH=%RC_PATH%;%PATH%\n")
	for envSet in tc["EnvSet"]:
		varName = envSet.split("=")[0]
		varExt = "%" + varName + "%"
		if varExt in envSet:
			envSetClean = envSet.replace(os.pathsep + varExt, "").replace(varExt + os.pathsep, "")
			fo.write("if defined " + varName + " (\n")
			fo.write("set " + EscapeArg(envSet) + "\n")
			fo.write(") else (\n")
			fo.write("set " + EscapeArg(envSetClean) + "\n")
			fo.write(")\n")
		else:
			fo.write("set " + EscapeArg(envSet) + "\n")
	fo.write("set " + EscapeArg("RC_GENERATOR=" + tc["Generator"]) + "\n")
	fo.write("set " + EscapeArg("RC_PLATFORM=" + tc["Platform"]) + "\n")
	fo.write("set " + EscapeArg("RC_TOOLSET=" + tc["Toolset"]) + "\n")
	fo.write("set " + EscapeArg("RC_BUILD_DIR=" + buildDir) + "\n")
	fo.close()

def GenerateMsvcCmd(file, envScript):
	fo = open(file, 'w')
	fo.write("call %~dp0" + envScript + "\n")
	fo.write("title %RC_GENERATOR% %RC_PLATFORM% %RC_TOOLSET%\n")
	#fo.write("cls\n")
	fo.write("cmd\n")
	fo.close()

def ConfigureTarget(spec, name, target):
	global NeLSpecClient
	global NeLSpecServer
	global NeLSpecTools
	global NeLSpecSamples
	global NeLSpecPluginMax
	global NeLScriptExt
	if not "Toolchain" in target or not target["Toolchain"]:
		return None
	res = { }
	tc = NeLToolchains[target["Toolchain"]]
	cfg = target["Config"]
	safeName = name.replace("/", "_")
	relPath = os.path.normcase(name)
	buildRootDir = None
	if "Docker" in tc and tc["Docker"]:
		relPath = os.path.join(NeLBuildDockerDirName, relPath)
		buildRootDir = NeLBuildDockerDir
	elif "Remote" in tc and tc["Remote"]:
		relPath = os.path.join(NeLBuildRemoteDirName, relPath)
		buildRootDir = NeLBuildRemoteDir
	else:
		relPath = os.path.join(NeLBuildDirName, relPath)
		buildRootDir = NeLBuildDir
	buildDir = os.path.join(NeLRootDir, relPath)
	os.makedirs(buildDir, exist_ok=True)
	res["RelPath"] = relPath.replace("\\", "/")
	res["Toolchain"] = target["Toolchain"]
	if tc["Compiler"] == "MSVC":
		GenerateMsvcEnv(os.path.join(buildRootDir, safeName + "_env." + NeLScriptExt), buildDir, tc)
		GenerateMsvcCmd(os.path.join(buildRootDir, safeName + "_cmd." + NeLScriptExt), safeName + "_env." + NeLScriptExt)
		pass
	elif tc["Compiler"] == "GCC":
		if "Docker" in tc and tc["Docker"]:
			pass
		else:
			pass
	return res

GeneratePathScript()

Targets["Native"]["client_dev"] = ConfigureTarget(NeLSpecClient, "client_dev", NeLTargetClientDev)
Targets["Native"]["server_dev"] = ConfigureTarget(NeLSpecServer, "server_dev", NeLTargetServerDev)
for client in NeLTargetClient:
	Targets["Client"][client] = ConfigureTarget(NeLSpecClient, "client/" + client, NeLTargetClient[client])
Targets["Server"] = ConfigureTarget(NeLSpecServer, "server", NeLTargetServer)
Targets["Native"]["tools"] = ConfigureTarget(NeLSpecTools, "tools", NeLTargetTools)
Targets["Native"]["samples"] = ConfigureTarget(NeLSpecSamples, "samples", NeLTargetSamples)
for pluginMax in NelTargetPluginMax:
	Targets["PluginMax"][pluginMax] = ConfigureTarget(NeLSpecPluginMax, "plugin_max/" + pluginMax, NelTargetPluginMax[pluginMax])

with open(os.path.join(NeLConfigDir, "targets_" + socket.gethostname().lower() + ".json"), 'w') as fo:
	json.dump(Targets, fo, indent=2)

# end of file
