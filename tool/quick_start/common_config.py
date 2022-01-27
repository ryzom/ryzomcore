
from common_root import *

import os, json

fi = open(os.path.join(NeLQuickStartDir, "config_default.json"), "r")
NeLConfig = json.load(fi)
fi.close()

fi = open(os.path.join(NeLConfigDir, "config.json"), "r")
NeLUserConfig = json.load(fi)
if not "Paths" in NeLUserConfig:
	NeLUserConfig["Paths"] = {}
if not "Tools" in NeLUserConfig["Paths"]:
	NeLUserConfig["Paths"]["Tools"] = {}
NeLConfig["Paths"]["Tools"].update(NeLUserConfig["Paths"]["Tools"])
NeLUserConfig["Paths"]["Tools"] = NeLConfig["Paths"]["Tools"]
NeLConfig["Paths"].update(NeLUserConfig["Paths"])
NeLUserConfig["Paths"] = NeLConfig["Paths"]
NeLConfig.update(NeLUserConfig)
fi.close()

del NeLUserConfig
del fi

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
NeLExternalDir = os.path.join(NeLRootDir, NeLConfig["Paths"]["External"])
if not os.path.isdir(NeLExternalDir):
	exit("NeL External directory (" + NeLExternalDir + ") does not exist.")

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
