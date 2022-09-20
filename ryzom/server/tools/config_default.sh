#!/bin/bash
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
# Config file for ryzom shard tools
#

# Automatic restart of services
AUTO_RESTART=1

# Web url use to notify and manage apps who need know when a service is started
NOTIFY_URL_SERVICE_RESTARTED="https://app.ryzom.com/app_arcc/services_started.php"
NOTIFY_URL_KEY="C6tLpddu8NJjvhyzBLqjw4uQWRXbGRsQ"

# Use gdb
USE_GDB=1

# Notification command
NOTIFY_COMMAND=""
