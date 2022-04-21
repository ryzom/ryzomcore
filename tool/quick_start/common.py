
from .common_config import *
from .common_docker import *

import os

# Known folders
NeLCodeDir = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))) # os.path.join(NeLRootDir, "code")
NeLLeveldesignDir = os.path.join(NeLRootDir, "leveldesign")
NeLGraphicsDir = os.path.join(NeLRootDir, "graphics")
NeLSoundDir = os.path.join(NeLRootDir, "sound")
NeLBuildDirName = "build_" + NeLHostId
NeLBuildDir = os.path.join(NeLRootDir, NeLBuildDirName)
NeLPipelineDir = os.path.join(NeLRootDir, "pipeline")
NeLDistributionDir = os.path.join(NeLRootDir, "distribution")

# Special folders
NeLTempDir = os.path.join(NeLRootDir, os.path.normcase(".nel/temp"))

# Tools for Windows only
NeLPython27Dir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Python27"])
NeLPython3Dir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Python3"])
NeLRRDtoolDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["RRDtool"])
NeLMariaDBDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["MariaDB"])
NeLNginxDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Nginx"])
NeLPHPDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["PHP"])
NeLphpMyAdminDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["phpMyAdmin"])
NeL3dsMaxDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["3dsMax"])
NeLDependenciesDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Dependencies"])) # Detect all dependencies of the build output and get them from externals

# Tools added to system path, for any platform
# If these paths are wrong, tools from the system path may be used, which is okay
NeLAria2Dir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Aria2"])) # Used for downloading SteamRT
NeLNinjaDir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Ninja"])) # Used to build faster

# Deduplicate any paths, and add them to the system path variable
NeLEnvPaths = {}
NeLEnvPaths[NeLAria2Dir] = True
NeLEnvPaths[NeLNinjaDir] = True
for path in NeLEnvPaths:
	os.environ["PATH"] = path + os.pathsep + os.environ["PATH"]
# print(os.environ["PATH"])
del NeLEnvPaths

#print(NeLRootDir)
#print(NeLConfigDir)
#print(NeLConfig["Domain"])
#print(NeLConfig["Paths"]["Code"])

#print(NeLExternalDir)

#print(NeLBuildDir)
