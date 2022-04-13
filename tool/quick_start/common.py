
from .common_config import *

import os

# Known folders
NeLCodeDir = os.path.join(NeLRootDir, "code")
NeLLeveldesignDir = os.path.join(NeLRootDir, "leveldesign")
NeLGraphicsDir = os.path.join(NeLRootDir, "graphics")
NeLSoundDir = os.path.join(NeLRootDir, "sound")
NeLBuildDirName = "build_" + NeLHostId
NeLBuildDir = os.path.join(NeLRootDir, NeLBuildDirName)
NeLPipelineDir = os.path.join(NeLRootDir, "pipeline")
NeLDistributionDir = os.path.join(NeLRootDir, "distribution")

# Special folders
NeLTempDir = os.path.join(NeLRootDir, os.path.normcase(".nel/temp"))

# Tools
NeLPython27Dir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Python27"])
NeLRRDtoolDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["RRDtool"])
NeLMariaDBDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["MariaDB"])
NeLNginxDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Nginx"])
NeLPHPDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["PHP"])
NeLphpMyAdminDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["phpMyAdmin"])
NeL3dsMaxDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["3dsMax"])

# Tools added to system path
NeLAria2Dir = os.path.join(NeLRootDir, os.path.normcase(NeLConfig["Paths"]["Aria2"]))

NeLEnvPaths = {}
NeLEnvPaths[NeLAria2Dir] = True
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
