
from .common_config import *
from .common_docker import *

import os

# Known folders
NeLCodeDir = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))) # os.path.join(NeLRootDir, "code")
NeLLeveldesignDir = os.path.join(NeLRootDir, "leveldesign")
NeLGraphicsDir = os.path.join(NeLRootDir, "graphics")
NeLSoundDir = os.path.join(NeLRootDir, "sound")
NeLPipelineDir = os.path.join(NeLRootDir, "pipeline")
NeLDistributionDir = os.path.join(NeLRootDir, "distribution")

# Separate builds by host platform (not target platform) for quick switching in case of dual boot
NeLBuildDirName = "build_" + NeLPlatformId
NeLBuildDir = os.path.join(NeLRootDir, NeLBuildDirName)
NeLBuildDockerDirName = "build_docker"
NeLBuildDockerDir = os.path.join(NeLRootDir, NeLBuildDockerDirName)
NeLBuildRemoteDirName = "build_remote"
NeLBuildRemoteDir = os.path.join(NeLRootDir, NeLBuildRemoteDirName)

# Special folders
NeLTempDir = os.path.join(NeLRootDir, os.path.normcase(".nel/temp"))

# Tools for Windows only, from the externals package (Paths/External); a tool path set as an absolute path in
# .nel/config.json replaces the one in the package
NeLExternalToolsDir = os.path.join(NeLRootDir, os.path.normpath(NeLConfig["Paths"]["External"]))
NeLPython3Dir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["Python3"])) # Runs the quick start and the build pipeline
NeLPerlDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["Perl"])) # Only need this to build OpenSSH with Hunter...
NeLRRDtoolDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["RRDtool"]))
NeLMariaDBDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["MariaDB"]))
NeLNginxDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["Nginx"]))
NeLPHPDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["PHP"]))
NeLphpMyAdminDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["phpMyAdmin"]))
NeLDependenciesDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["Dependencies"])) # Detect all dependencies of the build output and get them from externals
NeLUtilsDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["Utils"])) # nircmd, msxsl, 7za, xdelta

# The configuration and state of the development web stack of this root (nginx prefix, php.ini, phpMyAdmin), the
# packaged programs stay as they are
NeLWebDir = os.path.join(NeLRootDir, os.path.normpath(NeLConfig["Paths"]["Web"]))

# Cross-platform tools that are not C++ (Rust crates), built in Docker for a generic Linux and for Windows
NeLXplatToolsImage = "ryzombuild_xplat_tools"
NeLXplatTools = [ "tool/servdash_cli" ]
NeLXplatToolsTargets = { "linux": "x86_64-unknown-linux-musl", "win": "x86_64-pc-windows-msvc" }

# Tools added to system path, for any platform
# If these paths are wrong, tools from the system path may be used, which is okay
NeLAria2Dir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["Aria2"])) # Used for downloading SteamRT
NeLNinjaDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["Ninja"])) # Used to build faster
NeLJomDir = os.path.join(NeLExternalToolsDir, os.path.normpath(NeLConfig["Paths"]["Jom"])) # Used to build faster if Ninja doesn't work

# Deduplicate any paths, and add them to the system path variable
NeLEnvPaths = {}
NeLEnvPaths[NeLAria2Dir] = True
NeLEnvPaths[NeLNinjaDir] = True
NeLEnvPaths[NeLJomDir] = True
for path in NeLEnvPaths:
	os.environ["PATH"] = path + os.pathsep + os.environ["PATH"]
# print(os.environ["PATH"])

NeLScriptExt = 'sh'
if os.name == 'nt':
	NeLScriptExt = 'bat'

NeLPathScript = os.path.join(NeLRootDir, os.path.normcase(".nel/path_config." + NeLScriptExt))
NeLPatchVersionScript = os.path.join(NeLRootDir, os.path.normcase(".nel/patch_version." + NeLScriptExt))
NeLPatchVersionSetScript = os.path.join(NeLRootDir, os.path.normcase(".nel/patch_version_set." + NeLScriptExt))

#print(NeLRootDir)
#print(NeLConfigDir)
#print(NeLConfig["Domain"])
#print(NeLConfig["Paths"]["Code"])

#print(NeLExternalDir)

#print(NeLBuildDir)
