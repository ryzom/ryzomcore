
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
if not "Toolchain" in NeLUserConfig:
	NeLUserConfig["Toolchain"] = {}
NeLConfig["Paths"]["Tools"].update(NeLUserConfig["Paths"]["Tools"])
NeLUserConfig["Paths"]["Tools"] = NeLConfig["Paths"]["Tools"]
NeLConfig["Paths"].update(NeLUserConfig["Paths"])
NeLUserConfig["Paths"] = NeLConfig["Paths"]
NeLConfig["Toolchain"].update(NeLUserConfig["Toolchain"])
NeLUserConfig["Toolchain"] = NeLConfig["Toolchain"]
NeLConfig.update(NeLUserConfig)
fi.close()

if os.path.isfile(os.path.join(NeLConfigDir, "toolchains_default.json")):
	fi = open(os.path.join(NeLConfigDir, "toolchains_default.json"), "r")
	NeLToolchains = json.load(fi)
	fi.close()
else:
	NeLToolchains = {}

if os.path.isfile(os.path.join(NeLConfigDir, "toolchains.json")):
	fi = open(os.path.join(NeLConfigDir, "toolchains.json"), "r")
	NeLToolchains.update(json.load(fi))
	fi.close()

del NeLUserConfig
del fi
