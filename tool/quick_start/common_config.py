
from common_root import *

import os, json, socket

fi = open(os.path.join(NeLQuickStartDir, "config_default.json"), "r")
NeLConfig = json.load(fi)
fi.close()

fi = open(os.path.join(NeLConfigDir, "config.json"), "r")
NeLUserConfig = json.load(fi)
if not "Paths" in NeLUserConfig:
	NeLUserConfig["Paths"] = {}
if not "Win64" in NeLUserConfig["Paths"]:
	NeLUserConfig["Paths"]["Win64"] = {}
if not "Toolchain" in NeLUserConfig:
	NeLUserConfig["Toolchain"] = {}
NeLConfig["Paths"]["Win64"].update(NeLUserConfig["Paths"]["Win64"])
NeLUserConfig["Paths"]["Win64"] = NeLConfig["Paths"]["Win64"]
NeLConfig["Paths"].update(NeLUserConfig["Paths"])
NeLUserConfig["Paths"] = NeLConfig["Paths"]
NeLConfig["Toolchain"].update(NeLUserConfig["Toolchain"])
NeLUserConfig["Toolchain"] = NeLConfig["Toolchain"]
NeLConfig.update(NeLUserConfig)
fi.close()

NeLHostName = socket.gethostname()

# Path-safe ID from hostname
# Used for separating build directories in multiboot development scenario
# Release builds should always be done from the same box, ideally
NeLHostId = NeLHostName.lower()
if "HostId" in NeLConfig:
	# Override if specified
	NeLHostId = NeLConfig["HostId"]

if os.path.isfile(os.path.join(NeLConfigDir, "toolchains_" + NeLHostId + "_default.json")):
	fi = open(os.path.join(NeLConfigDir, "toolchains_" + NeLHostId + "_default.json"), "r")
	NeLToolchains = json.load(fi)
	fi.close()
else:
	NeLToolchains = {}

if os.path.isfile(os.path.join(NeLConfigDir, "toolchains_" + NeLHostId + ".json")):
	fi = open(os.path.join(NeLConfigDir, "toolchains_" + NeLHostId + ".json"), "r")
	NeLToolchains.update(json.load(fi))
	fi.close()

del NeLUserConfig
del fi
