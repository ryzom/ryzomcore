
from common_config import *

import os

NeLCodeDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Code"])
if not os.path.isdir(NeLCodeDir):
	exit("NeL Code directory (" + NeLCodeDir + ") does not exist.")
NeLLeveldesignDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Leveldesign"])
if not os.path.isdir(NeLLeveldesignDir):
	exit("NeL Leveldesign directory (" + NeLLeveldesignDir + ") does not exist.")
NeLGraphicsDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Graphics"])
if not os.path.isdir(NeLGraphicsDir):
	exit("NeL Graphics directory (" + NeLGraphicsDir + ") does not exist.")
NeLSoundDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Sound"])
if not os.path.isdir(NeLSoundDir):
	exit("NeL Sound directory (" + NeLSoundDir + ") does not exist.")
NeLBuildDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Build"])
if not os.path.isdir(NeLBuildDir):
	exit("NeL Build directory (" + NeLBuildDir + ") does not exist.")
NeLPipelineDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Pipeline"])
if not os.path.isdir(NeLPipelineDir):
	exit("NeL Pipeline directory (" + NeLPipelineDir + ") does not exist.")

NeLPython27Dir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Tools"]["Python27"])
NeLRRDtoolDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Tools"]["RRDtool"])
NeLMariaDBDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Tools"]["MariaDB"])
NeLNginxDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Tools"]["Nginx"])
NeLPHPDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Tools"]["PHP"])
NeLphpMyAdminDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Tools"]["phpMyAdmin"])
NeL3dsMaxDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["Tools"]["3dsMax"])

#print(NeLRootDir)
#print(NeLConfigDir)
#print(NeLConfig["Domain"])
#print(NeLConfig["Paths"]["Code"])

#print(NeLExternalDir)
