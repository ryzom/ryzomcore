
# This generates the build scripts for all targets

import sys, os, math
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *
from quick_start.find_external import *
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

def GenerateCMakeOptions(spec, generator, fv, target, buildDir, filteredPrefix):
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
		if "Generator" in tc and gen == tc["Generator"]:
			opts += [ "-G", "%RC_GENERATOR%" ]
		else:
			opts += [ "-G", gen ]
	
	# Visual Studio Toolset and Platform
	if gen and gen.startswith("Visual Studio") and "Toolset" in tc and tc["Toolset"].startswith("v"):
		if "Platform" in tc:
			opts += [ "-A", "%RC_PLATFORM%" ]
		if tc["Version"] > 9:
			opts += [ "-T", "%RC_TOOLSET%" ]
	
	# SSE2 and SSE3 off on 32-bit x86 platform
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
	
	if len(FilterExternalDirs([ "ffmpeg" ], filteredPrefix)) > 0:
		opts += [ "-DWITH_FFMPEG=ON" ]
	else:
		opts += [ "-DWITH_FFMPEG=OFF" ]
	
	if "HasMsQuic" in tc and tc["HasMsQuic"] and (len(filteredPrefix) == 0 or len(FilterExternalDirs([ "msquic" ], filteredPrefix)) > 0):
		opts += [ "-DWITH_MSQUIC=ON" ]
	else:
		opts += [ "-DWITH_MSQUIC=OFF" ]
	
	if len(FilterExternalDirs([ "qt4", "qt4_static" ], filteredPrefix)) > 0:
		opts += [ "-DWITH_QT=ON" ]
	else:
		opts += [ "-DWITH_QT=OFF" ]
	
	if len(FilterExternalDirs([ "qt5", "qt5_static" ], filteredPrefix)) > 0:
		opts += [ "-DWITH_QT5=ON" ]
	else:
		opts += [ "-DWITH_QT5=OFF" ]
	
	if len(FilterExternalDirs([ "qt6", "qt6_static" ], filteredPrefix)) > 0:
		opts += [ "-DWITH_QT6=ON" ]
	else:
		opts += [ "-DWITH_QT6=OFF" ]
	
	if tc["Compiler"] == "MSVC" and not isHunter:
		opts += [ "-DWITH_STATIC_LIBXML2=OFF" ]
		opts += [ "-DWITH_STATIC_CURL=OFF" ]
		opts += [ "-DCURL_NO_CURL_CMAKE=ON" ]
	
	if gen:
		if gen.startswith("Visual Studio"):
			if tc["Version"] > 16:
				opts += [ "-DCUSTOM_FLAGS=/MP%RC_PARALLEL%" ]
			else:
				opts += [ "-DCUSTOM_FLAGS=/MP%RC_PARALLEL_FILES%" ]
		elif gen == "NMake Makefiles":
			opts += [ "-DCUSTOM_FLAGS=/MP%RC_PARALLEL%" ]
	
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
		opts += [ "-DWITH_RYZOM_PATCH=ON" ]
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

def FilterPrefix(target, spec):
	global NeLSpecClient
	global NeLSpecServer
	global NeLSpecTools
	global NeLSpecSamples
	global NeLSpecPluginMax
	nelDirs = [ "zlib", "openssl", "curl", "libjpeg", "libpng", "iconv", "libxml2", "freetype2", "ogg", "vorbis", "openal", "gl", "gles", "lua", "boost", "luabind" , "mariadb" ]
	toolsDirs = [ "squish", "assimp" ]
	serverClientDirs = [ "msquic" ]
	tc = NeLToolchains[target["Toolchain"]]
	cfg = target["Config"]
	qt = None
	if "Qt" in cfg:
		qt = cfg["Qt"]
	dirs = [] + nelDirs
	if spec == NeLSpecTools or spec == NeLSpecSamples or spec == NeLSpecPluginMax:
		dirs = dirs + toolsDirs
	if spec == NeLSpecServer or spec == NeLSpecClient:
		dirs = dirs + serverClientDirs
	res = FilterExternalDirs(dirs, tc["Prefix"])
	if qt:
		for qtVer in qt:
			qtRes = FilterExternalDirs([ qtVer ], tc["Prefix"])
			if len(qtRes):
				res = res + qtRes
				break
	return res

def GeneratePathScript():
	fo = open(NeLPathScript, 'w')
	fo.write("set " + EscapeArg("RC_ROOT=" + NeLRootDir) + "\n")
	fo.write("set " + EscapeArg("RC_ROOT_FW=" + NeLRootDir.replace("\\", "/")) + "\n")
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
	if Targets["Native"]["client_dev"]:
		clientDir = os.path.join(NeLRootDir, os.path.normcase(Targets["Native"]["client_dev"]["BuildDir"]))
	else:
		clientDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Fallback"]["Client"]))
	fo.write("set " + EscapeArg("RC_CLIENT_DIRS_RELEASE=" + os.path.join(clientDir, os.path.normcase("bin/Release").replace("release", "Release")) + os.pathsep + os.path.join(clientDir, "bin")) + "\n")
	fo.write("set " + EscapeArg("RC_CLIENT_DIRS_DEBUG=" + os.path.join(clientDir, os.path.normcase("bin/Debug").replace("debug", "Debug")) + os.pathsep + os.path.join(clientDir, "bin")) + "\n")
	if Targets["Native"]["server_dev"]:
		serverDir = os.path.join(NeLRootDir, os.path.normcase(Targets["Native"]["server_dev"]["BuildDir"]))
	else:
		serverDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Fallback"]["Server"]))
	fo.write("set " + EscapeArg("RC_SERVER_DIRS_RELEASE=" + os.path.join(serverDir, os.path.normcase("bin/Release").replace("release", "Release")) + os.pathsep + os.path.join(serverDir, "bin")) + "\n")
	fo.write("set " + EscapeArg("RC_SERVER_DIRS_DEBUG=" + os.path.join(serverDir, os.path.normcase("bin/Debug").replace("debug", "Debug")) + os.pathsep + os.path.join(serverDir, "bin")) + "\n")
	if Targets["Server"]:
		serverFvDir = os.path.join(NeLRootDir, os.path.normcase(Targets["Server"]["BuildDir"]))
		fo.write("set " + EscapeArg("RC_SERVER_DIRS_FV=" + os.path.join(serverFvDir, os.path.normcase("bin/Release").replace("release", "Release")) + os.pathsep + os.path.join(serverFvDir, "bin")) + "\n")
	if Targets["Native"]["tools"]:
		toolsDir = os.path.join(NeLRootDir, os.path.normcase(Targets["Native"]["tools"]["BuildDir"]))
	else:
		toolsDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Fallback"]["Tools"]))
	fo.write("set " + EscapeArg("RC_TOOLS_DIRS_RELEASE=" + os.path.join(toolsDir, os.path.normcase("bin/Release").replace("release", "Release")) + os.pathsep + os.path.join(toolsDir, "bin") + os.pathsep + "%RC_SERVER_DIRS_RELEASE%" + os.pathsep + NeLDependenciesDir) + "\n")
	fo.write("set " + EscapeArg("RC_TOOLS_DIRS_DEBUG=" + os.path.join(toolsDir, os.path.normcase("bin/Debug").replace("debug", "Debug")) + os.pathsep + os.path.join(toolsDir, "bin") + os.pathsep + "%RC_SERVER_DIRS_RELEASE%" + os.pathsep + NeLDependenciesDir) + "\n")
	externalBinDirs = {}
	if Targets["Native"]["client_dev"]:
		if "PrefixBin" in Targets["Native"]["client_dev"]:
			for dir in Targets["Native"]["client_dev"]["PrefixBin"]:
				if not dir in externalBinDirs:
					externalBinDirs[dir] = True
	if Targets["Native"]["server_dev"]:
		if "PrefixBin" in Targets["Native"]["server_dev"]:
			for dir in Targets["Native"]["server_dev"]["PrefixBin"]:
				if not dir in externalBinDirs:
					externalBinDirs[dir] = True
	if Targets["Native"]["tools"]:
		if "PrefixBin" in Targets["Native"]["tools"]:
			for dir in Targets["Native"]["tools"]["PrefixBin"]:
				if not dir in externalBinDirs:
					externalBinDirs[dir] = True
	if Targets["Native"]["samples"]:
		if "PrefixBin" in Targets["Native"]["samples"]:
			for dir in Targets["Native"]["samples"]["PrefixBin"]:
				if not dir in externalBinDirs:
					externalBinDirs[dir] = True
	if not len(externalBinDirs.keys()) > 0:
		for dir in NeLConfig["Fallback"]["PrefixBin"]:
			if not dir in externalBinDirs:
				externalBinDirs[dir] = True
	fo.write("set " + EscapeArg("RC_EXTERNAL_BIN_DIRS=" + os.pathsep.join(externalBinDirs.keys())) + "\n")
	if Targets["Native"]["tools"] and "QtPluginBin" in Targets["Native"]["client_dev"]:
		fo.write("set " + EscapeArg("RC_QT_PLUGIN_DIR=" + Targets["Native"]["client_dev"]["QtPluginBin"]) + "\n")
	else:
		fo.write("set " + EscapeArg("RC_QT_PLUGIN_DIR=" + NeLConfig["Fallback"]["QtPluginBin"]) + "\n")
	exedll = []
	for client in NeLTargetClient:
		exedll += [ "common/exedll_" + client ]
	fo.write("set " + EscapeArg("RC_EXEDLL_PROJECTS=" + " ".join(exedll)) + "\n")
	fo.write("set " + EscapeArg("RC_3DSMAX_EXE=" + NeLConfig["3dsMax"]["Executable"]) + "\n")
	fo.write("set " + EscapeArg("RC_3DSMAX_USER_DIR=" + NeLConfig["3dsMax"]["UserDirectory"]) + "\n")
	fo.write("set " + EscapeArg("RC_SIGNTOOL_EXE=" + NeLConfig["SignTool"]["Executable"]) + "\n")
	fo.write("set " + EscapeArg("RC_SIGNTOOL_SHA1=" + NeLConfig["SignTool"]["Sha1"]) + "\n")
	fo.write("set " + EscapeArg("RC_SIGNTOOL_TIMESTAMP=" + NeLConfig["SignTool"]["Timestamp"]) + "\n")
	fo.write("call " + EscapeArg(os.path.join(NeLRootDir, os.path.normcase(".nel/web_config.bat"))))
	fo.close()

def GeneratePatchVersionScript():
	fo = open(NeLPatchVersionScript, 'w')
	fo.write("if /I \"%RC_PYTHON3_DIR%\"==\"\" call " + EscapeArg(NeLPathScript) + "\n")
	fo.write("if /I \"%RC_CLIENT_PATCH_VERSION%\"==\"\" goto :update\n")
	fo.write("goto :done\n")
	fo.write(":update\n")
	fo.write("\"%RC_PYTHON3_DIR%\\python\" \"%RC_CODE_DIR%\\tool\\quick_start\\patch_version.py\"\n")
	fo.write("call " + EscapeArg(NeLPatchVersionSetScript) + "\n")
	fo.write(":done\n")
	fo.close()

def GenerateDockerEnv(file, relBuildDir, buildDir, tc):
	fo = open(file, 'w')
	fo.write("@echo off\n")
	fo.write("rem " + tc["DisplayName"] + "\n")
	fo.write("cd /d " + EscapeArg(buildDir) + "\n")
	fo.write("call " + EscapeArg(NeLPathScript) + "\n")
	if "Hunter" in tc and tc["Hunter"]:
		fo.write("set PATH=%RC_PERL_DIRS%;%PATH%\n")
	fo.write("set PATH=%RC_PATH%;%PATH%\n")
	fo.write("set " + EscapeArg("RC_BUILD_DIR=" + buildDir) + "\n")
	fo.write("set RC_DOCKER=")
	dockerCmd = DockerBaseCommand(tc["Image"], tc["Platform"], relBuildDir, "Hunter" in tc and tc["Hunter"])
	dockerCmdStr = ""
	for cmd in dockerCmd:
		dockerCmdStr += EscapeArgOpt(cmd) + " "
	if len(dockerCmdStr) > 0:
		fo.write(dockerCmdStr[:-1])
	fo.write("\n")
	fo.write("set RC_DOCKER_IT=")
	dockerCmd = DockerBaseCommand(tc["Image"], tc["Platform"], relBuildDir, "Hunter" in tc and tc["Hunter"], interactive=True)
	dockerCmdStr = ""
	for cmd in dockerCmd:
		dockerCmdStr += EscapeArgOpt(cmd) + " "
	if len(dockerCmdStr) > 0:
		fo.write(dockerCmdStr[:-1])
	fo.write("\n")
	fo.close()

def WritePauseGoto(fo, go):
	fo.write("if %errorlevel% neq 0 (\n")
	fo.write("pause\n")
	fo.write("goto " + go + "\n")
	fo.write(")\n")

def WriteHeader(fo):
	fo.write("@echo off\n")
	fo.write("title Ryzom Core\n")
	fo.write("cd /d " + EscapeArg(NeLRootDir) + "\n")
	fo.write("goto :checktools\n")
	fo.write(":nogit\n")
	fo.write("echo ERROR: Git is not installed\n")
	fo.write("pause\n")
	fo.write("exit\n")
	fo.write(":nocmake\n")
	fo.write("echo ERROR: CMake is not installed\n")
	fo.write("pause\n")
	fo.write("exit\n")
	fo.write(":checktools\n")
	fo.write("where /q git\n")
	fo.write("if %errorlevel% neq 0 goto :nogit\n")
	fo.write("where /q cmake\n")
	fo.write("if %errorlevel% neq 0 goto :nocmake\n")
	fo.write(":patchversion\n")
	fo.write("call " + EscapeArg(NeLPatchVersionScript) + "\n")
	WritePauseGoto(fo, ":patchversion")
	fo.write(":checkdocker\n")
	fo.write("\"%RC_PYTHON3_DIR%\\python\" \"%RC_ROOT%\\code\\tool\\quick_start\\check_docker.py\"\n")
	WritePauseGoto(fo, ":checkdocker")

def WriteFooter(fo, title):
	fo.write("title " + title + ": Ready\n")
	fo.write("echo Ready\n")
	fo.write("echo OK > " + EscapeArg("%RC_ROOT%\.nel\code_build.tag") + "\n")
	fo.write("pause\n")

def GenerateMsvcEnv(file, buildDir, generator, tc, filteredPrefix):
	fo = open(file, 'w')
	fo.write("@echo off\n")
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
	platform = tc["Platform"]
	if platform == "x86" or platform == "386":
		platform = "Win32"
	else:
		platform = "x64"
	fo.write("set " + EscapeArg("RC_PLATFORM=" + platform) + "\n")
	fo.write("set " + EscapeArg("RC_TOOLSET=" + tc["Toolset"]) + "\n")
	fo.write("set " + EscapeArg("RC_BUILD_DIR=" + buildDir) + "\n")
	if "Prefix" in tc and len(tc["Prefix"]) > 0:
		prefixPath = ""
		for path in filteredPrefix:
			prefixPath += path + os.pathsep
		fo.write("set " + EscapeArg("RC_PREFIX_PATH=" + prefixPath[:-1]) + "\n")
		binPrefix = FindBinPaths(filteredPrefix)
		prefixBin = ""
		for path in binPrefix:
			prefixBin += path + os.pathsep
		fo.write("set " + EscapeArg("RC_PREFIX_BIN=" + prefixBin[:-1]) + "\n")
		qtPluginBin = FindQtPluginPath(filteredPrefix)
		if qtPluginBin:
			fo.write("set " + EscapeArg("RC_QT_PLUGIN_DIR=" + qtPluginBin) + "\n")
	gen = generator
	if not gen and "Generator" in tc:
		gen = tc["Generator"]
	if gen.startswith("Visual Studio"):
		fo.write("set CL_MP=true\n")
		if tc["Version"] > 16:
			fo.write("set UseMSBuildResourceManager=true\n")
			fo.write("set UseMultiToolTask=false\n")
			fo.write("set EnforceProcessCountAcrossBuilds=true\n")
			fo.write("set EnableClServerMode=true\n")
		elif tc["Version"] > 15:
			fo.write("set UseMultiToolTask=false\n")
			fo.write("set EnforceProcessCountAcrossBuilds=false\n")
	fo.close()

def GenerateMsvcCmd(file, envScript, target):
	fo = open(file, 'w')
	fo.write("@echo off\n")
	fo.write("call " + EscapeArg(envScript) + "\n")
	fo.write("title Terminal - " + target["DisplayName"] +" - %RC_GENERATOR% %RC_PLATFORM% %RC_TOOLSET%\n")
	#fo.write("cls\n")
	fo.write("cmd\n")
	fo.close()

def GenerateDockerCmd(file, envScript, target):
	tc = NeLToolchains[target["Toolchain"]]
	fo = open(file, 'w')
	fo.write("@echo off\n")
	fo.write("call " + EscapeArg(envScript) + "\n")
	fo.write("title Terminal - " + target["DisplayName"] +" - " + tc["DisplayName"] + "\n")
	#fo.write("cls\n")
	fo.write("%RC_DOCKER_IT% bash\n")
	fo.close()

def GenerateCMakeCreate(file, envScript, spec, generator, fv, target, buildDir, filteredPrefix):
	tc = NeLToolchains[target["Toolchain"]]
	isDocker = "Docker" in tc and tc["Docker"]
	opts = GenerateCMakeOptions(spec, generator, fv, target, buildDir, filteredPrefix)
	fo = open(file, 'w')
	fo.write("@echo off\n")
	fo.write("call " + EscapeArg(envScript) + "\n")
	tcTitle = "%RC_GENERATOR% %RC_PLATFORM% %RC_TOOLSET%"
	if tc["Compiler"] != "MSVC":
		tcTitle = tc["DisplayName"]
	fo.write("title Configure - " + target["DisplayName"] +" - " + tcTitle + "\n")
	fo.write("mkdir /s /q %RC_BUILD_DIR% > nul 2> nul\n")
	fo.write(":erasedir\n")
	fo.write("powershell -Command \"Remove-Item '" + os.path.join("%RC_BUILD_DIR%", "*") + "' -Recurse -Force\"\n")
	WritePauseGoto(fo, ":erasedir")
	fo.write(":configure\n")
	fo.write("@echo on\n")
	if isDocker:
		fo.write("%RC_DOCKER% ")
	fo.write("cmake")
	lastOpt = False
	for opt in opts:
		if lastOpt or (opt.startswith("-") and ("%" in opt or "$" in opt)):
			fo.write(" " + EscapeArg(opt))
		else:
			fo.write(" " + EscapeArgOpt(opt))
		lastOpt = (len(opt) == 2)
	fo.write("\n")
	fo.write("@echo off\n")
	WritePauseGoto(fo, ":configure")
	# If gen is VS, generate all the appropriate .vcxproj.user files for easy debugging (call a Python script for this)
	gen = generator
	if not gen and "Generator" in tc:
		gen = tc["Generator"]
	if gen and gen.startswith("Visual Studio"):
		fo.write(":vcxprojuser\n")
		fo.write("\"%RC_PYTHON3_DIR%\\python\" \"%RC_CODE_DIR%\\tool\\quick_start\\vcxproj_user.py\" " + EscapeArgOpt(buildDir) + " " + EscapeArgOpt(target["Toolchain"]) + "\n")
		WritePauseGoto(fo, ":vcxprojuser")
	fo.close()

def GenerateBuild(file, envScript, spec, generator, fv, target, buildDir):
	tc = NeLToolchains[target["Toolchain"]]
	cfg = target["Config"]
	isDocker = "Docker" in tc and tc["Docker"]
	
	# Generator
	gen = generator
	if not gen and "Generator" in tc:
		gen = tc["Generator"]
	
	fo = open(file, 'w')
	fo.write("@echo off\n")
	fo.write("call " + EscapeArg(envScript) + "\n")
	tcTitle = "%RC_GENERATOR% %RC_PLATFORM% %RC_TOOLSET%"
	if tc["Compiler"] != "MSVC":
		tcTitle = tc["DisplayName"]
	fo.write("title Build - " + target["DisplayName"] +" - " + tcTitle + "\n")
	
	# Update patch version, may be done ahead by calling build script as well
	fo.write(":patchversion\n")
	fo.write("call " + EscapeArg(NeLPatchVersionScript) + "\n")
	WritePauseGoto(fo, ":patchversion")
	
	# Ensure it's configured
	fo.write("if not exist CMakeCache.txt (\n")
	fo.write("echo ERROR: This build project has not been configured yet\n")
	fo.write("pause\n")
	fo.write("goto :done\n")
	fo.write(")\n")
	
	# Update patch version
	# TODO: Update build number (increment a meaningless number locally anytime the git sha1 `git rev-parse HEAD` changes, set the minimum to `git rev-list --count HEAD`)
	fo.write(":reconfigure\n")
	fo.write("@echo on\n")
	if isDocker:
		fo.write("%RC_DOCKER% ")
	fo.write("cmake -DNL_VERSION_PATCH=%RC_CLIENT_PATCH_VERSION% -DNL_VERSION_BUILD=%RC_BUILD_NUMBER% .\n")
	fo.write("@echo off\n")
	WritePauseGoto(fo, ":reconfigure")
	
	# Build
	fo.write(":build\n")
	fo.write("@echo on\n")
	if isDocker:
		fo.write("%RC_DOCKER% ")
	if gen and gen.startswith("Visual Studio"):
		if not fv:
			if tc["Version"] > 16:
				fo.write("msbuild RyzomCore.sln /t:ALL_BUILD /m:%RC_PARALLEL% /p:CL_MPCount=%RC_PARALLEL% /p:Configuration=Debug\n")
			else:
				fo.write("msbuild RyzomCore.sln /t:ALL_BUILD /m:%RC_PARALLEL_PROJECTS% /p:CL_MPCount=%RC_PARALLEL_FILES% /p:Configuration=Debug\n")
			WritePauseGoto(fo, ":build")
		if tc["Version"] > 16:
			fo.write("msbuild RyzomCore.sln /t:ALL_BUILD /m:%RC_PARALLEL% /p:CL_MPCount=%RC_PARALLEL% /p:Configuration=Release\n")
		else:
			fo.write("msbuild RyzomCore.sln /t:ALL_BUILD /m:%RC_PARALLEL_PROJECTS% /p:CL_MPCount=%RC_PARALLEL_FILES% /p:Configuration=Release\n")
	elif gen and "Ninja" in gen:
		fo.write("ninja -j%RC_PARALLEL%\n")
	elif gen and "JOM" in gen:
		fo.write("jom -j%RC_PARALLEL%\n")
	elif gen and "NMake" in gen:
		fo.write("nmake\n")
	else:
		fo.write("make -j%RC_PARALLEL%\n")
	fo.write("@echo off\n")
	WritePauseGoto(fo, ":build")
	
	fo.write(":done\n")
	fo.close()

fo_configure_clean_all = open(os.path.join(NeLRootDir, "code_configure_clean_all." + NeLScriptExt), "w")
fo_configure_rebuild_all = open(os.path.join(NeLRootDir, "code_configure_rebuild_all." + NeLScriptExt), "w")
fo_build_all = open(os.path.join(NeLRootDir, "code_build_all." + NeLScriptExt), "w")
fo_build_game = open(os.path.join(NeLRootDir, "code_build_game." + NeLScriptExt), "w")
fo_build_game_dev = open(os.path.join(NeLRootDir, "code_build_game_dev." + NeLScriptExt), "w")

WriteHeader(fo_configure_clean_all)
WriteHeader(fo_configure_rebuild_all)
WriteHeader(fo_build_all)
WriteHeader(fo_build_game)
WriteHeader(fo_build_game_dev)

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
	relBuildRootDir = None
	buildRootDir = None
	isDocker = "Docker" in tc and tc["Docker"]
	isHunter = "Hunter" in tc and tc["Hunter"]
	if isDocker:
		relPath = os.path.join(NeLBuildDockerDirName, relPath)
		relBuildRootDir = NeLBuildDockerDirName
		buildRootDir = NeLBuildDockerDir
	elif "Remote" in tc and tc["Remote"]:
		relPath = os.path.join(NeLBuildRemoteDirName, relPath)
		relBuildRootDir = NeLBuildRemoteDirName
		buildRootDir = NeLBuildRemoteDir
	else:
		relPath = os.path.join(NeLBuildDirName, relPath)
		relBuildRootDir = NeLBuildDirName
		buildRootDir = NeLBuildDir
	buildDir = os.path.join(NeLRootDir, relPath)
	os.makedirs(buildDir, exist_ok=True)
	res["BuildScriptDir"] = relBuildRootDir
	res["BuildDir"] = relPath.replace("\\", "/")
	res["Toolchain"] = target["Toolchain"]
	gen = None
	if (fv or spec == NeLSpecPluginMax or isDocker) and NeLConfig["UseNinja"]:
		if (isHunter and tc["Compiler"] == "MSVC"):
			 # OpenSSL build fails with Hunter and Ninja using MSVC, JOM works
			gen = "NMake Makefiles JOM"
		else:
			gen = "Ninja"
	if gen:
		res["Generator"] = gen
	elif "Generator" in tc:
		res["Generator"] = tc["Generator"]
	envScript = os.path.join(buildRootDir, safeName + "_env." + NeLScriptExt)
	configureScript = os.path.join(buildRootDir, safeName + "_configure." + NeLScriptExt)
	buildScript = os.path.join(buildRootDir, safeName + "_build." + NeLScriptExt)
	scriptOk = False
	if tc["Compiler"] == "MSVC":
		filteredPrefix = FilterPrefix(target, spec)
		res["PrefixPath"] = filteredPrefix
		res["PrefixBin"] = FindBinPaths(filteredPrefix)
		qtPluginBin = FindQtPluginPath(filteredPrefix)
		if qtPluginBin:
			res["QtPluginBin"] = qtPluginBin
		GenerateMsvcEnv(envScript, buildDir, gen, tc, filteredPrefix)
		GenerateMsvcCmd(os.path.join(buildRootDir, safeName + "_terminal." + NeLScriptExt), envScript, target)
		GenerateCMakeCreate(configureScript, envScript, spec, gen, fv, target, buildDir, filteredPrefix)
		GenerateBuild(buildScript, envScript, spec, gen, fv, target, buildDir)
		scriptOk = True
	elif tc["Compiler"] == "GCC":
		if "Docker" in tc and tc["Docker"]:
			GenerateDockerEnv(envScript, relPath, buildDir, tc)
			GenerateDockerCmd(os.path.join(buildRootDir, safeName + "_terminal." + NeLScriptExt), envScript, target)
			GenerateCMakeCreate(configureScript, envScript, spec, gen, fv, target, buildDir, [])
			GenerateBuild(buildScript, envScript, spec, gen, fv, target, buildDir)
			scriptOk = True
		else:
			pass
	if scriptOk:
		fo_configure_clean_all.write("cmd /C " + EscapeArg("call " + configureScript) + "\n")
		fo_configure_rebuild_all.write("cmd /C " + EscapeArg("call " + configureScript) + "\n")
		fo_configure_rebuild_all.write("cmd /C " + EscapeArg("call " + buildScript) + "\n")
		fo_build_all.write("cmd /C " + EscapeArg("call " + buildScript) + "\n")
		if spec == NeLSpecServer or spec == NeLSpecClient:
			fo_build_game.write("cmd /C " + EscapeArg("call " + buildScript) + "\n")
			if not fv:
				fo_build_game_dev.write("cmd /C " + EscapeArg("call " + buildScript) + "\n")
		if spec == NeLSpecTools:
			fo_build_game_dev.write("cmd /C " + EscapeArg("call " + buildScript) + "\n")
	return res

Targets["Native"]["client_dev"] = ConfigureTarget(NeLSpecClient, "client_dev", False, NeLTargetClientDev)
Targets["Native"]["server_dev"] = ConfigureTarget(NeLSpecServer, "server_dev", False, NeLTargetServerDev)
for client in NeLTargetClient:
	Targets["Client"][client] = ConfigureTarget(NeLSpecClient, "client/" + client, True, NeLTargetClient[client])
Targets["Server"] = ConfigureTarget(NeLSpecServer, "server", True, NeLTargetServer)
Targets["Native"]["tools"] = ConfigureTarget(NeLSpecTools, "tools", False, NeLTargetTools)
Targets["Native"]["samples"] = ConfigureTarget(NeLSpecSamples, "samples", False, NeLTargetSamples)
for pluginMax in NelTargetPluginMax:
	Targets["PluginMax"][pluginMax] = ConfigureTarget(NeLSpecPluginMax, "plugin_max/" + pluginMax, False, NelTargetPluginMax[pluginMax])

GeneratePathScript()
GeneratePatchVersionScript()

WriteFooter(fo_configure_clean_all, "Ryzom Core: Code Configure")
WriteFooter(fo_configure_rebuild_all, "Ryzom Core: Code Configure Rebuild All")
WriteFooter(fo_build_all, "Ryzom Core: Build All")
WriteFooter(fo_build_game, "Ryzom Core: Build Game")
WriteFooter(fo_build_game_dev, "Ryzom Core: Build Game Dev")

fo_configure_clean_all.close()
fo_configure_rebuild_all.close()
fo_build_all.close()
fo_build_game.close()
fo_build_game_dev.close()

with open(os.path.join(NeLConfigDir, "targets_" + NeLHostId + "_" + NeLPlatformId + ".json"), 'w') as fo:
	json.dump(Targets, fo, indent=2)

# end of file
