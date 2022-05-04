
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

# Tools for Windows only
NeLPython27Dir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Python27"]))
NeLPython3Dir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Python3"]))
NeLPerlDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Perl"])) # Only need this to build OpenSSH with Hunter...
NeLRRDtoolDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["RRDtool"]))
NeLMariaDBDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["MariaDB"]))
NeLNginxDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Nginx"]))
NeLPHPDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["PHP"]))
NeLphpMyAdminDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["phpMyAdmin"]))
NeLDependenciesDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Dependencies"])) # Detect all dependencies of the build output and get them from externals

# Tools added to system path, for any platform
# If these paths are wrong, tools from the system path may be used, which is okay
NeLAria2Dir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Aria2"])) # Used for downloading SteamRT
NeLNinjaDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Ninja"])) # Used to build faster
NeLJomDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Jom"])) # Used to build faster if Ninja doesn't work

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
