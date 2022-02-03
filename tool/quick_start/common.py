
from . common_config import *

import os

NeLCodeDir = os.path.join(NeLRootDir, "code")
NeLLeveldesignDir = os.path.join(NeLRootDir, "leveldesign")
NeLGraphicsDir = os.path.join(NeLRootDir, "graphics")
NeLSoundDir = os.path.join(NeLRootDir, "sound")
NeLBuildDirName = "build_" + NeLHostId
NeLBuildDir = os.path.join(NeLRootDir, NeLBuildDirName)
NeLPipelineDir = os.path.join(NeLRootDir, "pipeline")
NeLDistributionDir = os.path.join(NeLRootDir, "distribution")

# TODO: OS
#NeLPython27Dir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Win64"]["Python27"])
#NeLRRDtoolDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Win64"]["RRDtool"])
#NeLMariaDBDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Win64"]["MariaDB"])
#NeLNginxDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Win64"]["Nginx"])
#NeLPHPDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Win64"]["PHP"])
#NeLphpMyAdminDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Win64"]["phpMyAdmin"])
#NeL3dsMaxDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Win64"]["3dsMax"])

#print(NeLRootDir)
#print(NeLConfigDir)
#print(NeLConfig["Domain"])
#print(NeLConfig["Paths"]["Code"])

#print(NeLExternalDir)

print(NeLBuildDir)