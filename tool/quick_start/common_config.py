
from .common_root import *

import os, json, socket

NeLHostName = socket.gethostname()

# Path-safe ID from hostname
# Used for separating build directories in multiboot development scenario
# Release builds should always be done from the same box, ideally
NeLHostId = NeLHostName.lower()

fi = open(os.path.join(NeLQuickStartDir, "config_default.json"), "r")
NeLConfig = json.load(fi)
fi.close()

def MergeConfig(file):
	global NeLConfig
	if not os.path.isfile(file):
		return
	fi = open(os.path.join(NeLConfigDir, file), "r")
	config = json.load(fi)
	if not "Paths" in config:
		config["Paths"] = {}
	if not "Toolchain" in config:
		config["Toolchain"] = {}
	if not "Client" in config["Toolchain"]:
		config["Toolchain"]["Client"] = {}
	NeLConfig["Paths"].update(config["Paths"])
	config["Paths"] = NeLConfig["Paths"]
	NeLConfig["Toolchain"]["Client"].update(config["Toolchain"]["Client"])
	config["Toolchain"]["Client"] = NeLConfig["Toolchain"]["Client"]
	NeLConfig["Toolchain"].update(config["Toolchain"])
	config["Toolchain"] = NeLConfig["Toolchain"]
	NeLConfig.update(config)
	fi.close()

MergeConfig("config.json")

if "HostId" in NeLConfig:
	# Override if specified
	NeLHostId = NeLConfig["HostId"]

MergeConfig("config_" + NeLHostId + "_default.json")
MergeConfig("config_" + NeLHostId + ".json")

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

del MergeConfig
del fi