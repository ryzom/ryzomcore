
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
	if not "Win64" in config["Paths"]:
		config["Paths"]["Win64"] = {}
	if not "Toolchain" in config:
		config["Toolchain"] = {}
	if not "Client" in config["Toolchain"]:
		config["Toolchain"]["Client"] = {}
	NeLConfig["Paths"]["Win64"].update(config["Paths"]["Win64"])
	config["Paths"]["Win64"] = NeLConfig["Paths"]["Win64"]
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

fi = open(os.path.join(NeLConfigDir, "config.json"), "r")
NeLUserConfig = json.load(fi)
if not "Paths" in NeLUserConfig:
	NeLUserConfig["Paths"] = {}
if not "Win64" in NeLUserConfig["Paths"]:
	NeLUserConfig["Paths"]["Win64"] = {}
if not "Toolchain" in NeLUserConfig:
	NeLUserConfig["Toolchain"] = {}
if not "Client" in NeLUserConfig["Toolchain"]:
	NeLUserConfig["Toolchain"]["Client"] = {}
NeLConfig["Paths"]["Win64"].update(NeLUserConfig["Paths"]["Win64"])
NeLUserConfig["Paths"]["Win64"] = NeLConfig["Paths"]["Win64"]
NeLConfig["Paths"].update(NeLUserConfig["Paths"])
NeLUserConfig["Paths"] = NeLConfig["Paths"]
NeLConfig["Toolchain"]["Client"].update(NeLUserConfig["Toolchain"]["Client"])
NeLUserConfig["Toolchain"]["Client"] = NeLConfig["Toolchain"]["Client"]
NeLConfig["Toolchain"].update(NeLUserConfig["Toolchain"])
NeLUserConfig["Toolchain"] = NeLConfig["Toolchain"]
NeLConfig.update(NeLUserConfig)
fi.close()

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
