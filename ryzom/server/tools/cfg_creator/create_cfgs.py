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



import os, sys

dstpath = sys.argv[1]

dir_path = os.path.dirname(os.path.realpath(__file__))

templatepath = dir_path+"/templates/"
finalpath = dstpath+"/cfgs/"

enc = "iso-8859-1"

with open(dstpath+"/globals.cfg", 'r', encoding=enc) as fd:
    if fd:
        lines = fd.read().split("\n")
        tmp = []
        for n in lines:
            if n:
                line = n.split(" = ")
                if line:
                    tmp.append(line)

        for f in os.listdir(templatepath):
            with open(templatepath+f, 'r', encoding=enc) as content:
                if content:
                    content = content.read()
                    for k, v in tmp:
                        content = content.replace(k, v)
                    with open(finalpath+f, 'w') as fd:
                        fd.write(content)
            print(f)
