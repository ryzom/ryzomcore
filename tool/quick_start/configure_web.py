
# This pre-configures the local web (currently it just generates the mysql password to use, along with a batch script to have it in env) (this does not setup anything)

import sys, os, pathlib, secrets
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *

def EscapeArg(arg):
	return '"' + arg.replace('"', r'\"') + '"'

shardDevDir = os.path.join(NeLRootDir, os.path.normcase("pipeline/shard_dev"))
pathlib.Path(shardDevDir).mkdir(parents=True, exist_ok=True)

# This is the master file for the password
passwordFile = os.path.join(shardDevDir, "rc_mysql_password.txt")
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

# Map of php.ini configuration lines to change
configLines = {
	"max_execution_time = 30": "max_execution_time = 90",
	"max_input_time = 60": "max_input_time = 90",
	"memory_limit = 128M": "memory_limit = 256M",
	"post_max_size = 8M": "post_max_size = 32M",
	"upload_max_filesize = 2M": "upload_max_filesize = 32M",
	";date.timezone =": "date.timezone = UTC",
	";extension_dir = \"ext\"": "extension_dir = \"ext\"",
	";extension=gd2": "extension=gd2",
	";extension=gd": "extension=gd",
	";extension=mbstring": "extension=mbstring",
	";extension=mysqli": "extension=mysqli",
	";extension=openssl": "extension=openssl",
	";extension=pdo_mysql": "extension=pdo_mysql",
}

# Generate php.ini from php.ini-development on Windows
# NeLPHPDir = external/php
phpDir = os.path.join(NeLRootDir, NeLPHPDir)
phpIni = os.path.join(phpDir, "php.ini")
phpIniDevelopment = os.path.join(phpDir, "php.ini-development")
if os.path.isfile(phpIniDevelopment):
	# Read file, replace any matching line in configLines, write to php.ini
	# Remove lines already replaced from configLines
	with open(phpIni, 'w') as fo:
		with open(phpIniDevelopment, 'r') as fi:
			for line in fi:
				line = line.strip()
				if line in configLines:
					fo.write(configLines[line] + '\n')
					del configLines[line]
				else:
					fo.write(line + '\n')
		# Remove duplicates
		if ";extension=gd2" in configLines and not ";extension=gd" in configLines:
			del configLines[";extension=gd2"]
		if ";extension=gd" in configLines and not ";extension=gd2" in configLines:
			del configLines[";extension=gd"]
		# Append any remaining lines to php.ini
		if len(configLines) > 0:
			fo.write('\n')
			fo.write('; The following lines were added by configure_web.py\n')
			print("WARNING: The following lines could not be found in php.ini-development and must be configured manually:")
			for line in configLines:
				fo.write(";" + configLines[line] + '\n')
				print("  - " + configLines[line])
			print("Please improve configure_web.py appropriately and submit a pull request.")
			print()

# end of file
