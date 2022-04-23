
# This generates the build scripts for all targets

import sys, os, math
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *
from quick_start.find_targets import *

NeLSpecClient = 1
NeLSpecServer = 2
NeLSpecTools = 3
NeLSpecSamples = 4
NeLSpecPluginMax = 5

NeLCPUCount = os.cpu_count()
NeLParallel = int(max(math.ceil((NeLCPUCount * 3) / 4.0), 1))
NeLParallelProjects = int(max(math.floor(math.sqrt(NeLParallel)), 1))
NeLParallelFiles = int(max(NeLParallel // NeLParallelProjects, 1))

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

def GenerateCMakeOptions(spec, generator, fv, target, buildDir):
	global NeLSpecClient
	global NeLSpecServer
	global NeLSpecTools
	global NeLSpecSamples
	global NeLSpecPluginMax
	tc = NeLToolchains[target["Toolchain"]]
	cfg = target["Config"]
	maxSdk = None
	if "MaxSDK" in target:
		maxSdk = target["MaxSDK"]
	isDocker = "Docker" in tc and tc["Docker"]
	isHunter = "Hunter" in tc and tc["Hunter"]
	opts = []
	
	# Generator
	gen = generator
	if not gen and "Generator" in tc:
		gen = tc["Generator"]
	if gen:
		opts += [ "-G", gen ]
	
	# Visual Studio Toolset and Platform
	if gen.startswith("Visual Studio") and "Toolset" in tc and tc["Toolset"].startswith("v"):
		if "Platform" in tc:
			if tc["Platform"] == "x86" or tc["Platform"] == "386":
				opts += [ "-A", "Win32" ]
			else:
				opts += [ "-A", "x64" ]
		if tc["Version"] > 9:
			opts += [ "-T", tc["Toolset"] ]
	
	# SSE2 and SSE3 of on 32-bit x86 platform
	if tc["Platform"] == "x86" or tc["Platform"] == "386":
		opts += [ "-DWITH_SSE2=OFF" ]
		opts += [ "-DWITH_SSE3=OFF" ]
	
	# This is annoying in VS. And we run CMake ahead anyway
	opts += [ "-DCMAKE_SUPPRESS_REGENERATION=ON" ]
	
	# Toolchain and target options
	if "CMake" in tc:
		opts += tc["CMake"]
	if "CMake" in cfg:
		opts += cfg["CMake"]
	
	# Common
	opts += [ "-DWITH_EXTERNAL=OFF" ]
	opts += [ "-DWITH_STLPORT=OFF" ]
	opts += [ "-DWITH_NEL_TESTS=OFF" ]
	opts += [ "-DWITH_STATIC=ON" ]
	opts += [ "-DWITH_QT4=OFF" ]
	opts += [ "-DWITH_QT5=OFF" ] # TODO
	opts += [ "-DWITH_FFMPEG=OFF" ]
	
	if tc["Compiler"] == "MSVC" and not isHunter:
		opts += [ "-DWITH_STATIC_LIBXML2=OFF" ]
		opts += [ "-DWITH_STATIC_CURL=OFF" ]
		opts += [ "-DCURL_NO_CURL_CMAKE=ON" ]
	
	if gen and gen.startswith("Visual Studio"):
		opts += [ "-DCUSTOM_FLAGS=/MP%RC_PARALLEL_FILES%" ]
	
	if isHunter:
		opts += [ "-DHUNTER_ENABLED=ON" ]
		opts += [ "-DHUNTER_JOBS_NUMBER=%RC_PARALLEL%" ]
	
	if tc["Compiler"] == "GCC" or isHunter:
		opts += [ "-DWITH_STATIC_DRIVERS=ON" ]
	
	# FV
	if fv:
		opts += [ "-DFINAL_VERSION=ON" ]
	else:
		opts += [ "-DFINAL_VERSION=OFF" ]
	
	# Must specify the lua version based on what luabind links to
	if "LuaVersion" in tc:
		luaVer = tc["LuaVersion"]
		if luaVer != 501:
			opts += [ "-DWITH_LUA51=OFF" ]
		if luaVer == 500:
			opts += [ "-DWITH_LUA50=ON" ]
		elif luaVer == 501:
			opts += [ "-DWITH_LUA51=ON" ]
		elif luaVer == 502:
			opts += [ "-DWITH_LUA52=ON" ]
		elif luaVer == 503:
			opts += [ "-DWITH_LUA53=ON" ]
		elif luaVer == 504:
			opts += [ "-DWITH_LUA54=ON" ]
	else:
		opts += [ "-DWITH_LUA51=OFF" ]
	
	# MFC
	if spec != NeLSpecClient and spec != NeLSpecServer and "HasMFC" in tc and tc["HasMFC"]:
		opts += [ "-DWITH_MFC=ON" ]
	else:
		opts += [ "-DWITH_MFC=OFF" ]
	
	# DirectX
	if spec != NeLSpecServer and "DirectXSDK" in tc and len(tc["DirectXSDK"]) > 0:
		opts += [ "-DWITH_DRIVER_DIRECT3D=ON" ]
		opts += [ "-DWITH_DRIVER_DSOUND=ON" ]
		opts += [ "-DDXSDK_DIR=" + tc["DirectXSDK"] ]
	else:
		opts += [ "-DWITH_DRIVER_DIRECT3D=OFF" ]
		opts += [ "-DWITH_DRIVER_DSOUND=OFF" ]
	
	# XAudio2
	if spec != NeLSpecServer and "HasXAudio2" in tc and tc["HasXAudio2"]:
		opts += [ "-DWITH_DRIVER_XAUDIO2=ON" ]
	else:
		opts += [ "-DWITH_DRIVER_XAUDIO2=OFF" ]
	
	# OpenGL / OpenAL
	if spec != NeLSpecServer:
		opts += [ "-DWITH_DRIVER_OPENGL=ON" ]
		opts += [ "-DWITH_DRIVER_OPENAL=ON" ]
	else:
		opts += [ "-DWITH_DRIVER_OPENGL=OFF" ]
		opts += [ "-DWITH_DRIVER_OPENAL=OFF" ]
	
	# CMake Prefix
	if "Prefix" in tc and len(tc["Prefix"]) > 0:
		opts += [ "-DCMAKE_PREFIX_PATH=%RC_PREFIX_PATH%" ]
	
	if spec == NeLSpecClient:
		opts += [ "-DWITH_RYZOM_CLIENT=ON" ]
		opts += [ "%RC_CLIENT_DEFAULTS%" ]
	else:
		opts += [ "-DWITH_RYZOM_CLIENT=OFF" ]
	
	if spec == NeLSpecServer:
		opts += [ "-DWITH_RYZOM_SERVER=ON" ]
	else:
		opts += [ "-DWITH_RYZOM_SERVER=OFF" ]
	
	# Tools
	if spec == NeLSpecTools:
		opts += [ "-DWITH_NEL_TOOLS=ON" ]
		opts += [ "-DWITH_RYZOM_TOOLS=ON" ]
		opts += [ "-DWITH_ASSIMP=ON" ]
	else:
		opts += [ "-DWITH_NEL_TOOLS=OFF" ]
		opts += [ "-DWITH_RYZOM_TOOLS=OFF" ]
		opts += [ "-DWITH_ASSIMP=OFF" ]
	
	# Samples
	if spec == NeLSpecSamples:
		opts += [ "-DWITH_NEL_SAMPLES=ON" ]
		opts += [ "-DWITH_SNOWBALLS=ON" ]
		opts += [ "-DWITH_NELNS=ON" ]
	else:
		opts += [ "-DWITH_NEL_SAMPLES=OFF" ]
		opts += [ "-DWITH_SNOWBALLS=OFF" ]
		opts += [ "-DWITH_NELNS=OFF" ]
	
	# Max
	if spec == NeLSpecPluginMax:
		opts += [ "-DWITH_RYZOM=OFF" ]
		opts += [ "-DWITH_NEL_MAXPLUGIN=ON" ]
		opts += [ "-DMAXSDK_DIR=" + maxSdk["Path"] ]
	else:
		opts += [ "-DWITH_RYZOM=ON" ]
		opts += [ "-DWITH_NEL_MAXPLUGIN=OFF" ]
	
	# Coalesce custom flags
	res = []
	flags = []
	for opt in opts:
		if opt.startswith("-DCUSTOM_FLAGS="):
			flags += [ opt[len("-DCUSTOM_FLAGS="):] ]
		else:
			res += [ opt ]
	if len(flags):
		res += [ "-DCUSTOM_FLAGS=" + (" ".join(flags)) ]
	
	# Code
	if isDocker:
		res += [ "/mnt/nel/code" ]
	else:
		res += [ os.path.relpath(NeLCodeDir, buildDir) ]
	return res

def GeneratePathScript():
	fo = open(NeLPathScript, 'w')
	fo.write("set " + EscapeArg("RC_CODE_DIR=" + NeLCodeDir) + "\n")
	fo.write("set " + EscapeArg("RC_PYTHON27_DIR=" + NeLPython27Dir) + "\n")
	fo.write("set " + EscapeArg("RC_PYTHON3_DIR=" + NeLPython3Dir) + "\n")
	fo.write("set " + EscapeArg("RC_PERL_DIRS=" + os.path.join(NeLPerlDir, os.path.normcase("perl/site/bin")) + os.pathsep + os.path.join(NeLPerlDir, os.path.normcase("perl/bin")) + os.pathsep + NeLPerlDir) + "\n")
	envPath = ""
	for path in NeLEnvPaths:
		envPath += path + os.pathsep
	fo.write("set " + EscapeArg("RC_PATH=" + envPath[:-1]) + "\n")
	fo.write("set RC_CLIENT_DEFAULTS=")
	fo.write(EscapeArg("-DRYZOM_CLIENT_CREATE_ACCOUNT_URL=" + NeLConfig["Defaults"]["ClientCreateAccountUrl"]) + " ")
	fo.write(EscapeArg("-DRYZOM_CLIENT_EDIT_ACCOUNT_URL=" + NeLConfig["Defaults"]["ClientEditAccountUrl"]) + " ")
	fo.write(EscapeArg("-DRYZOM_CLIENT_FORGET_PASSWORD_URL=" + NeLConfig["Defaults"]["ClientForgetPasswordUrl"]) + " ")
	fo.write(EscapeArg("-DRYZOM_CLIENT_PATCH_URL=" + NeLConfig["Defaults"]["ClientPatchUrl"]) + " ")
	fo.write(EscapeArg("-DRYZOM_CLIENT_RELEASENOTES_URL=" + NeLConfig["Defaults"]["ClientReleaseNotesUrl"]) + " ")
	fo.write(EscapeArg("-DRYZOM_CLIENT_RELEASENOTES_RING_URL=" + NeLConfig["Defaults"]["ClientReleaseNotesRingUrl"]) + " ")
	fo.write(EscapeArg("-DRYZOM_CLIENT_APP_NAME=" + NeLConfig["Defaults"]["ClientDomain"]) + "\n")
	fo.write("set RC_PARALLEL=" + str(NeLParallel) + "\n")
	fo.write("set RC_PARALLEL_PROJECTS=" + str(NeLParallelProjects) + "\n")
	fo.write("set RC_PARALLEL_FILES=" + str(NeLParallelFiles) + "\n")
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
	if "Hunter" in tc and tc["Hunter"]:
		fo.write("set PATH=%RC_PERL_DIRS%;%PATH%\n")
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
	if "Prefix" in tc and len(tc["Prefix"]) > 0:
		prefixPath = ""
		for path in tc["Prefix"]:
			prefixPath += path + os.pathsep
		fo.write("set " + EscapeArg("RC_PREFIX_PATH=" + prefixPath[:-1]) + "\n")
	fo.close()

def GenerateMsvcCmd(file, envScript):
	fo = open(file, 'w')
	fo.write("call %~dp0" + envScript + "\n")
	fo.write("title %RC_GENERATOR% %RC_PLATFORM% %RC_TOOLSET%\n")
	#fo.write("cls\n")
	fo.write("cmd\n")
	fo.close()
	
def GenerateCMakeCreate(file, envScript, spec, generator, fv, target, buildDir):
	opts = GenerateCMakeOptions(spec, generator, fv, target, buildDir)
	fo = open(file, 'w')
	fo.write("call %~dp0" + envScript + "\n")
	fo.write("title %RC_GENERATOR% %RC_PLATFORM% %RC_TOOLSET%\n")
	fo.write("mkdir /s /q %RC_BUILD_DIR% > nul 2> nul\n")
	fo.write("powershell -Command \"Remove-Item '" + os.path.join("%RC_BUILD_DIR%", "*") + "' -Recurse -Force\"\n")
	fo.write("if %errorlevel% neq 0 pause\n")
	fo.write("cmake")
	for opt in opts:
		if opt.startswith("-") and ("%" in opt or "$" in opt):
			fo.write(" " + EscapeArg(opt))
		else:
			fo.write(" " + EscapeArgOpt(opt))
	fo.write("\n")
	fo.write("if %errorlevel% neq 0 pause\n")
	fo.close()

def ConfigureTarget(spec, name, fv, target):
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
	isDocker = "Docker" in tc and tc["Docker"]
	isHunter = "Hunter" in tc and tc["Hunter"]
	if isDocker:
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
	gen = None
	if (fv or spec == NeLSpecPluginMax) and not (isHunter and tc["Compiler"] == "MSVC"): # OpenSSL build fails with Hunter and Ninja using MSVC
		gen = "Ninja"
	if tc["Compiler"] == "MSVC":
		GenerateMsvcEnv(os.path.join(buildRootDir, safeName + "_env." + NeLScriptExt), buildDir, tc)
		GenerateMsvcCmd(os.path.join(buildRootDir, safeName + "_cmd." + NeLScriptExt), safeName + "_env." + NeLScriptExt)
		GenerateCMakeCreate(os.path.join(buildRootDir, safeName + "_create." + NeLScriptExt), safeName + "_env." + NeLScriptExt, spec, gen, fv, target, buildDir)
		pass
	elif tc["Compiler"] == "GCC":
		if "Docker" in tc and tc["Docker"]:
			pass
		else:
			pass
	return res

GeneratePathScript()

Targets["Native"]["client_dev"] = ConfigureTarget(NeLSpecClient, "client_dev", False, NeLTargetClientDev)
Targets["Native"]["server_dev"] = ConfigureTarget(NeLSpecServer, "server_dev", False, NeLTargetServerDev)
for client in NeLTargetClient:
	Targets["Client"][client] = ConfigureTarget(NeLSpecClient, "client/" + client, True, NeLTargetClient[client])
Targets["Server"] = ConfigureTarget(NeLSpecServer, "server", True, NeLTargetServer)
Targets["Native"]["tools"] = ConfigureTarget(NeLSpecTools, "tools", False, NeLTargetTools)
Targets["Native"]["samples"] = ConfigureTarget(NeLSpecSamples, "samples", False, NeLTargetSamples)
for pluginMax in NelTargetPluginMax:
	Targets["PluginMax"][pluginMax] = ConfigureTarget(NeLSpecPluginMax, "plugin_max/" + pluginMax, False, NelTargetPluginMax[pluginMax])

with open(os.path.join(NeLConfigDir, "targets_" + socket.gethostname().lower() + ".json"), 'w') as fo:
	json.dump(Targets, fo, indent=2)

# end of file
