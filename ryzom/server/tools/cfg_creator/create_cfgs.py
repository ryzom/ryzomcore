#!/usr/bin/python3
# ______                           _____ _                   _   _____           _
# | ___ \                         /  ___| |                 | | |_   _|         | |
# | |_/ /   _ _______  _ __ ___   \ `--.| |__   __ _ _ __ __| |   | | ___   ___ | |___
# |    / | | |_  / _ \| '_ ` _ \   `--. \ '_ \ / _` | '__/ _` |   | |/ _ \ / _ \| / __|
# | |\ \ |_| |/ / (_) | | | | | | /\__/ / | | | (_| | | | (_| |   | | (_) | (_) | \__ \
# \_| \_\__, /___\___/|_| |_| |_| \____/|_| |_|\__,_|_|  \__,_|   \_/\___/ \___/|_|___/
#        __/ |
#       |___/
#
# Ryzom - MMORPG Framework <https://ryzom.com/dev/>
# Copyright (C) 2019  Winch Gate Property Limited
# This program is free software: read https://ryzom.com/dev/copying.html for more details
#
# This script is a helper to generate the configurations files of a ryzom shard
# just reading fields in a globals.cfg file and replacing it in all final cfgs files
#
# Usage are ./create_cfgs.py SHARD_PATH
#



import os
import sys
import configparser

dir_path = os.path.dirname(os.path.realpath(__file__))
templatepath = dir_path+"/templates/"

config = configparser.ConfigParser()
config.read("/etc/ryzom/shard.ini")
config = {s:dict(config.items(s)) for s in config.sections()}
finalpath = config["shard"]["path"]+"/cfgs/"
config_vars = {}
for ns,vs in config.items():
	for ni, vi in vs.items():
		config_vars[ns.upper()+"_"+ni.upper()] = vi

config_vars["UC_SHARD_NAME"] = config_vars["SHARD_NAME"].title()
config_vars["SHARD_WEB"] = "http://"+config_vars["SHARD_HOSTNAME"]+":55555/"
config_vars["CHAT"] = "- deprecated -"
config_vars["CHAT_DB"] = "- deprecated -"

for f in os.listdir(templatepath):
	with open(templatepath+f, "r", encoding="iso-8859-1") as content:
		if content:
			content = content.read()
			for k, v in config_vars.items():
				content = content.replace("#"+k+"#", v)
			with open(finalpath+f, "w") as fd:
				fd.write(content)
			print(f)
