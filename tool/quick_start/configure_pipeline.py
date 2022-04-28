
# This generates the pipeline exedll projects

import sys, os, pathlib
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *

workspaceDir = os.path.join(NeLConfigDir, "workspace")
commonDir = os.path.join(workspaceDir, "common")
pathlib.Path(commonDir).mkdir(parents=True, exist_ok=True)

def isWindows(client):
	tcn = NeLTargets["Client"][client]["Toolchain"]
	tc = NeLToolchains[tcn]
	return "OS" in tc and tc["OS"].startswith("Win")

def isMSVC(client):
	tcn = NeLTargets["Client"][client]["Toolchain"]
	tc = NeLToolchains[tcn]
	return "Compiler" in tc and tc["Compiler"] == "MSVC"

# Generate the projects file, this is imported after the user's projects.py
fo = open(os.path.join(workspaceDir, "projects_exedll.py"), "w")
fo.write("#!/usr/bin/python\n")
fo.write("\n")
fo.write("from projects import *\n")
fo.write("\n")
for client in NeLTargets["Client"]:
	clientWindows = isWindows(client)
	fo.write("ProjectsToProcess += [ \"common/exedll_" + client + "\" ]\n")
	fo.write("\n")
	fo.write("ICMainExedllPlatform = { }\n")
	fo.write("ICMainExedllPlatform[\"Name\"] = \"main_exedll_" + client + "\"\n")
	fo.write("ICMainExedllPlatform[\"UnpackTo\"] = \"\"\n")
	fo.write("ICMainExedllPlatform[\"IsOptional\"] = 0\n")
	fo.write("ICMainExedllPlatform[\"IsIncremental\"] = 1\n")
	fo.write("ICMainExedllPlatform[\"StreamedPackages\"] = False\n")
	fo.write("ICMainExedllPlatform[\"Packages\"] = [ ]\n")
	fo.write("ICMainExedllPlatform[\"Packages\"] += [ [ \"exedll_" + client + "\", [ ] ] ]\n")
	if clientWindows:
		fo.write("ICMainExedllPlatform[\"Packages\"] += [ [ \"exedll_" + client + "_lib\", [ ] ] ]\n")
	fo.write("ICMainExedllPlatform[\"Refs\"] = [ ]\n")
	fo.write("InstallClientData += [ ICMainExedllPlatform ]\n")
	fo.write("del ICMainExedllPlatform\n")
	fo.write("\n")
fo.write("# end of file\n")
fo.close()

# Generate the individual projects
for client in NeLTargets["Client"]:
	clientWindows = isWindows(client)
	clientMSVC = isMSVC(client)
	projectDir = os.path.join(commonDir, "exedll_" + client)
	pathlib.Path(projectDir).mkdir(parents=True, exist_ok=True)
	fo = open(os.path.join(projectDir, "process.py"), "w")
	fo.write("#!/usr/bin/python\n")
	fo.write("\n")
	fo.write("ProcessToComplete = [ ]\n")
	if clientWindows:
		fo.write("ProcessToComplete += [ \"sign\", \"lib\", \"copy\" ]\n")
	else:
		fo.write("ProcessToComplete += [ \"copy\" ]\n")
	fo.write("CommonName = \"exedll_" + client + "\"\n")
	fo.write("CommonPath = \"common/\" + CommonName\n")
	fo.write("\n")
	fo.write("# end of file\n")
	fo.close()
	fo = open(os.path.join(projectDir, "directories.py"), "w")
	fo.write("#!/usr/bin/python\n")
	fo.write("\n")
	fo.write("# COMMON\n")
	fo.write("CommonName = \"exedll_" + client + "\"\n")
	fo.write("CommonPath = \"common/\" + CommonName\n")
	fo.write("# OVERRIDE\n")
	buildDir = NeLTargets["Client"][client]["BuildDir"]
	absBuildDir = os.path.join(NeLRootDir, buildDir)
	buildDirs = [ os.path.join(absBuildDir, "bin").replace("\\", "/"), os.path.join(NeLCodeDir, "ryzom/client").replace("\\", "/") ]
	if clientMSVC:
		buildDirs = [ os.path.join(absBuildDir, "bin/Release").replace("\\", "/") ] + buildDirs
	fo.write("ExeDllCfgDirectories = " + str(buildDirs) + "\n")
	fo.write("# COPY\n")
	fo.write("CopyDirectSourceDirectories = [ ]\n")
	fo.write("CopyDirectSourceFiles = [ ]\n")
	fo.write("CopyLeveldesignSourceDirectories = [ ]\n")
	fo.write("CopyLeveldesignSourceFiles = [ ]\n")
	fo.write("CopyLeveldesignWorldSourceDirectories = [ ]\n")
	fo.write("CopyLeveldesignWorldSourceFiles = [ ]\n")
	fo.write("CopyLeveldesignDfnSourceDirectories = [ ]\n")
	fo.write("CopyLeveldesignDfnSourceFiles = [ ]\n")
	fo.write("CopyDatabaseSourceDirectories = [ ]\n")
	fo.write("CopyDatabaseSourceFiles = [ ]\n")
	fo.write("CopyExeDllCfgSourceFiles = [ ]\n")
	fo.write("CopyExeDllCfgSourceFiles += [ \"client_default.cfg\" ]\n")
	if not clientWindows:
		fo.write("CopyExeDllCfgSourceFiles += [ \"ryzom_client\" ]\n")
		fo.write("CopyExeDllCfgSourceFiles += [ \"ryzom_client_patcher\" ]\n")
	if clientWindows:
		fo.write("# SIGN, LIB\n")
		fo.write("ExeDllFiles = [ ]\n")
		fo.write("ExeDllFiles += [ \"nel_drv_direct3d_win_r.dll\" ]\n")
		fo.write("ExeDllFiles += [ \"nel_drv_dsound_win_r.dll\" ]\n")
		fo.write("ExeDllFiles += [ \"nel_drv_fmod_win_r.dll\" ]\n")
		fo.write("ExeDllFiles += [ \"nel_drv_openal_win_r.dll\" ]\n")
		fo.write("ExeDllFiles += [ \"nel_drv_opengl_win_r.dll\" ]\n")
		fo.write("ExeDllFiles += [ \"nel_drv_opengl3_win_r.dll\" ]\n")
		fo.write("ExeDllFiles += [ \"nel_drv_xaudio2_win_r.dll\" ]\n")
		fo.write("ExeDllFiles += [ \"ryzom_client_r.exe\" ]\n")
		fo.write("ExeDllFiles += [ \"ryzom_client_patcher.exe\" ]\n")
		fo.write("ExeDllFiles += [ \"ryzom_configuration_qt_r.exe\" ]\n")
		fo.write("ExeDllFiles += [ \"crash_report.exe\" ]\n")
		fo.write("# EXPORT\n")
		fo.write("UnsignedExeDllDirectory = CommonPath + \"/unsigned_exe_dll\"\n")
		fo.write("LibExeDllDirectory = CommonPath + \"/lib_exe_dll\"\n")
		fo.write("# BUILD\n")
		fo.write("SignedExeDllDirectory = CommonPath + \"/signed_exe_dll\"\n")
	fo.write("# INSTALL\n")
	fo.write("CopyInstallDirectory = CommonName\n")
	if clientWindows:
		fo.write("LibInstallDirectory = CommonName + \"_lib\"\n")
		fo.write("SignInstallDirectory = CommonName\n")
	fo.write("\n")
	fo.write("# end of file\n")
	fo.close()

# end of file
