
# This pre-configures the local web (currently it just generates the mysql password to use, along with a batch script to have it in env) (this does not setup anything)

import sys, os, pathlib, secrets
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *

def EscapeArg(arg):
	return '"' + arg.replace('"', r'\"') + '"'

mysqlDir = os.path.join(NeLRootDir, os.path.normcase("pipeline/shard_dev/mysql"))
pathlib.Path(mysqlDir).mkdir(parents=True, exist_ok=True)

# This is the master file for the password
passwordFile = os.path.join(mysqlDir, "rc_password.txt")
password = os.getenv('RC_MYSQL_PASSWORD')
if password:
	password = password.strip()
if not os.path.isfile(passwordFile):
	if not password:
		password = secrets.token_urlsafe(16).strip()
	fo = open(passwordFile, 'w')
	fo.write(password + '\n')
	fo.close()
else:
	fi = open(passwordFile, 'r')
	password = fi.readline().strip()
	fi.close()

# Generate environment script for web setup
fo = open(os.path.join(NeLConfigDir, "web_config.bat"), 'w')
fo.write("set " + EscapeArg("RC_MYSQL_PASSWORD=" + password) + "\n")
fo.close()

# end of file
