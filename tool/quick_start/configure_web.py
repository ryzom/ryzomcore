
# This pre-configures the development web stack of this root: the MySQL password (with a batch script that has it
# in the environment), and the configuration of the packaged MariaDB, PHP, nginx and phpMyAdmin under Paths/Web
# (pipeline/shard_dev/web). The programs run from the externals package as they are, nothing is written there.
# The database itself is set up the first time the web is started (win32/web/mariadb_setup.bat).

import sys, os, pathlib, secrets, shutil
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from quick_start.common import *

def EscapeArg(arg):
	return '"' + arg.replace('"', r'\"') + '"'

def ForwardSlashes(path):
	return path.replace("\\", "/")

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

if os.name != 'nt':
	sys.exit()

def RequireDir(path, what):
	if not os.path.isdir(path):
		sys.exit("ERROR: " + what + " was not found in " + path + ". Extract the externals package to C:\\, or set Paths in .nel/config.json.")

RequireDir(NeLMariaDBDir, "MariaDB")
RequireDir(NeLPHPDir, "PHP")
RequireDir(NeLNginxDir, "nginx")
RequireDir(NeLphpMyAdminDir, "phpMyAdmin")

webPhpDir = os.path.join(NeLWebDir, "php")
webNginxDir = os.path.join(NeLWebDir, "nginx")
webPmaDir = os.path.join(NeLWebDir, "phpmyadmin")
mysqlDataDir = os.path.join(shardDevDir, "mysql")
for dir in [ webPhpDir, os.path.join(webNginxDir, "conf"), os.path.join(webNginxDir, "logs"), os.path.join(webNginxDir, "temp") ]:
	pathlib.Path(dir).mkdir(parents=True, exist_ok=True)

# Map of php.ini configuration lines to change
configLines = {
	"max_execution_time = 30": "max_execution_time = 90",
	"max_input_time = 60": "max_input_time = 90",
	"memory_limit = 128M": "memory_limit = 256M",
	"post_max_size = 8M": "post_max_size = 32M",
	"upload_max_filesize = 2M": "upload_max_filesize = 32M",
	";date.timezone =": "date.timezone = UTC",
	";extension_dir = \"ext\"": "extension_dir = \"" + os.path.join(NeLPHPDir, "ext") + "\"",
	";extension=gd2": "extension=gd2",
	";extension=gd": "extension=gd",
	";extension=mbstring": "extension=mbstring",
	";extension=mysqli": "extension=mysqli",
	";extension=openssl": "extension=openssl",
	";extension=pdo_mysql": "extension=pdo_mysql",
}

# Generate php.ini from php.ini-development of the packaged PHP, php-cgi reads it with -c
phpIni = os.path.join(webPhpDir, "php.ini")
phpIniDevelopment = os.path.join(NeLPHPDir, "php.ini-development")
if not os.path.isfile(phpIniDevelopment):
	sys.exit("ERROR: " + phpIniDevelopment + " was not found")
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

# phpMyAdmin reads its configuration from its own folder and writes its cache there, so this root serves a copy of
# the packaged one, refreshed when the package brings another release
def PmaRelease(dir):
	if not os.path.isdir(dir):
		return None
	for name in os.listdir(dir):
		if name.startswith("RELEASE-DATE-"):
			return name
	return None

pmaRelease = PmaRelease(NeLphpMyAdminDir)
if not pmaRelease:
	sys.exit("ERROR: " + NeLphpMyAdminDir + " has no RELEASE-DATE file, it does not look like phpMyAdmin")
if PmaRelease(webPmaDir) != pmaRelease:
	if os.path.isdir(webPmaDir):
		print("phpMyAdmin " + pmaRelease[len("RELEASE-DATE-"):] + " replaces the copy in " + webPmaDir)
		shutil.rmtree(webPmaDir)
	shutil.copytree(NeLphpMyAdminDir, webPmaDir)
pathlib.Path(os.path.join(webPmaDir, "tmp")).mkdir(parents=True, exist_ok=True)

# Generate the phpMyAdmin configuration with its own cookie secret, from the template in the web folder
pmaSecretFile = os.path.join(shardDevDir, "rc_pma_blowfish_secret.txt")
if not os.path.isfile(pmaSecretFile):
	pmaSecret = secrets.token_urlsafe(24) # 32 characters, the length phpMyAdmin expects
	fo = open(pmaSecretFile, 'w')
	fo.write(pmaSecret + '\n')
	fo.close()
else:
	fi = open(pmaSecretFile, 'r')
	pmaSecret = fi.readline().strip()
	fi.close()

pmaTemplate = os.path.join(os.path.dirname(os.path.realpath(__file__)), "win32", "web", "config.inc.php")
pmaPlaceholder = "$cfg['blowfish_secret'] = '';"
with open(pmaTemplate, 'r') as fi:
	pmaConfig = fi.read()
if not pmaPlaceholder in pmaConfig:
	sys.exit("ERROR: The blowfish_secret placeholder was not found in " + pmaTemplate)
pmaConfig = pmaConfig.replace(pmaPlaceholder, "$cfg['blowfish_secret'] = '" + pmaSecret + "';")
with open(os.path.join(webPmaDir, "config.inc.php"), 'w') as fo:
	fo.write(pmaConfig)

# nginx runs with this root's folder as prefix (-p), for its configuration, logs and temporary files
def FastCgi(indent):
	return [
		indent + "fastcgi_pass   127.0.0.1:9041;",
		indent + "fastcgi_index  index.php;",
		indent + "fastcgi_param  SCRIPT_FILENAME    $document_root$fastcgi_script_name;",
		indent + "include        \"" + ForwardSlashes(os.path.join(NeLNginxDir, "conf", "fastcgi_params")) + "\";",
	]

def LocalOnly(indent):
	return [
		indent + "allow 127.0.0.1;",
		indent + "allow ::1;",
		indent + "deny  all;",
	]

nginxConf = [
	"worker_processes  1;",
	"events {",
	"    worker_connections  1024;",
	"}",
	"http {",
	"    include       \"" + ForwardSlashes(os.path.join(NeLNginxDir, "conf", "mime.types")) + "\";",
	"    default_type  application/octet-stream;",
	"    sendfile        on;",
	"    keepalive_timeout  65;",
	"    server {",
	"        listen       9042;",
	"        server_name  localhost;",
	"        root         \"" + ForwardSlashes(os.path.join(NeLCodeDir, "web", "public_php")) + "\";",
	"        index        index.php index.html index.htm;",
	"        location ~ /\\.ht {",
	"            deny  all;",
	"        }",
	"        # The database console, the web admin and the installer are served to this machine only.",
	"        # The web root stays open on the network for the in-game browser of clients on other machines.",
	"        location ^~ /phpmyadmin {",
] + LocalOnly("            ") + [
	"            root  \"" + ForwardSlashes(NeLWebDir) + "\";",
	"            location ~ ^/phpmyadmin/(.+\\.php)$ {",
] + FastCgi("                ") + [
	"            }",
	"        }",
]
for location in [ "/admin", "/setup" ]:
	nginxConf += [ "        location ^~ " + location + " {" ] + LocalOnly("            ") + [ "            location ~ \\.php$ {" ] + FastCgi("                ") + [ "            }", "        }" ]
nginxConf += [ "        location ~ \\.php$ {" ] + FastCgi("            ") + [ "        }", "    }", "}" ]
with open(os.path.join(webNginxDir, "conf", "nginx.conf"), 'w') as fo:
	fo.write("\n".join(nginxConf) + "\n")

# The service dashboard of the web stack, opened by web_on.bat
nginxExe = os.path.join(NeLNginxDir, "nginx.exe")
nginxPrefix = "-p " + EscapeArg(ForwardSlashes(webNginxDir) + "/") + " -c conf/nginx.conf"
mysqldArgs = "--no-defaults --bind-address=127.0.0.1 --port=9040 --datadir=" + EscapeArg(mysqlDataDir) + " --character-set-server=utf8mb4 --collation-server=utf8mb4_general_ci --console --standalone"
def TailLogArgs(path):
	# one command string, so a path with spaces survives the argument parsing of powershell
	return "-NoProfile -Command \"Get-Content -LiteralPath '" + path.replace("'", "''") + "' -Wait\""
dashIni = [
	"[]",
	"Title=Web",
	"",
	"[access_log]",
	"Terminal=1",
	"Title=Access Log",
	"LaunchCmd=powershell",
	"LaunchArgs=" + TailLogArgs(os.path.join(webNginxDir, "logs", "access.log")),
	"",
	"[error_log]",
	"Terminal=1",
	"Title=Error Log",
	"LaunchCmd=powershell",
	"LaunchArgs=" + TailLogArgs(os.path.join(webNginxDir, "logs", "error.log")),
	"",
	"[mariadb]",
	"Terminal=1",
	"Title=MariaDB",
	"WorkingDirectory=" + NeLWebDir,
	"LaunchCmd=" + os.path.join(NeLMariaDBDir, "bin", "mysqld.exe"),
	"LaunchArgs=" + mysqldArgs,
	"ShutdownCmd=" + os.path.join(NeLMariaDBDir, "bin", "mysqladmin.exe"),
	"ShutdownArgs=-u root -P 9040 shutdown",
	"",
	"[php]",
	"Terminal=1",
	"Title=PHP-CGI",
	"WorkingDirectory=" + webPhpDir,
	"LaunchCmd=" + os.path.join(NeLPHPDir, "php-cgi.exe"),
	"LaunchArgs=-c " + EscapeArg(phpIni) + " -b 127.0.0.1:9041",
	"; php-cgi exits after PHP_FCGI_MAX_REQUESTS requests",
	"RestartDelay=1",
	"",
	"[nginx]",
	"Terminal=1",
	"Title=Nginx",
	"WorkingDirectory=" + webNginxDir,
	"LaunchCmd=" + nginxExe,
	"LaunchArgs=" + nginxPrefix,
	"ShutdownCmd=" + nginxExe,
	"ShutdownArgs=" + nginxPrefix + " -s stop",
	"",
]
with open(os.path.join(NeLWebDir, "web_dash.ini"), 'w') as fo:
	fo.write("\n".join(dashIni))

# end of file
