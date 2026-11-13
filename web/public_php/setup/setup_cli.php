#!/usr/bin/env php
<?php

/**
 * CLI Setup Tool for Ryzom Core Web Services
 *
 * Usage:
 *   php setup_cli.php install [--dev] [options]
 *   php setup_cli.php upgrade
 *
 * Install options:
 *   --dev                     Configure for development shard (port 9040, domain 'dev')
 *   --sql-host=HOST           SQL hostname (default: localhost)
 *   --sql-port=PORT           SQL port (default: 3306, or 9040 with --dev)
 *   --sql-user=USER           SQL username (default: root)
 *   --sql-pass=PASS           SQL password (default: empty)
 *   --nel-db=NAME             NeL database name (default: nel)
 *   --tool-db=NAME            NeL Tool database name (default: nel_tool)
 *   --domain-db=NAME          Ring domain database name (default: ring_dev or ring_HOSTNAME)
 *   --domain-name=NAME        Domain name (default: dev or HOSTNAME)
 *   --users-dir=PATH          Users directory for MFS (auto-detected)
 *   --setup-password=PASS     Setup password (required unless --dev)
 *   --admin-user=USER         Shard admin username (default: admin)
 *   --admin-pass=PASS         Shard admin password (required unless --dev)
 *   --admin-login=LOGIN       Administrator account: the first game account, with the
 *   --admin-password=PASS       :DEV: privilege. All three or none; without them no
 *   --admin-email=ADDRESS       game account exists.
 *   --private-php=PATH        Private PHP directory (default: ../private_php/)
 *   --no-role-service         Disable service role
 *   --no-role-domain          Disable domain role
 */

if (PHP_SAPI !== 'cli') {
	echo "This script must be run from the command line.\n";
	exit(1);
}

error_reporting(E_ALL);
ini_set('display_errors', 'on');

// php 8.1 defaults mysqli to exception reporting; this script and the
// helpers it includes check their results by hand
if (function_exists('mysqli_report'))
	mysqli_report(MYSQLI_REPORT_OFF);

// ANSI colors
function cli_info($msg) { echo "\033[36m[INFO]\033[0m $msg\n"; }
function cli_ok($msg) { echo "\033[32m[ OK ]\033[0m $msg\n"; }
function cli_warn($msg) { echo "\033[33m[WARN]\033[0m $msg\n"; }
function cli_err($msg) { echo "\033[31m[FAIL]\033[0m $msg\n"; }

// Parse command line
$args = $argv;
array_shift($args); // Remove script name

if (empty($args)) {
	print_usage();
	exit(1);
}

$command = array_shift($args);

if ($command === '--help' || $command === '-h') {
	$command = 'help';
}

if ($command !== 'install' && $command !== 'upgrade' && $command !== 'help') {
	cli_err("Unknown command: $command");
	print_usage();
	exit(1);
}

// Parse options
$opts = cli_parse_options($command, $args);

if ($command === 'help') {
	print_usage();
	exit(0);
}

// Determine paths
$scriptDir = dirname(__FILE__);
$publicPhpDir = realpath($scriptDir . '/..');
if (!$publicPhpDir) {
	cli_err("Cannot determine public PHP directory.");
	exit(1);
}

chdir($publicPhpDir);

// Define printalert for CLI mode (used by database.php and header.php functions)
function printalert($type, $message)
{
	// the messages are written for the web page: take the markup out, then
	// turn the entities back into the characters they stand for
	$message = html_entity_decode(strip_tags($message), ENT_QUOTES, 'UTF-8');
	switch ($type) {
		case 'success': cli_ok($message); break;
		case 'danger': cli_err($message); break;
		case 'info': cli_info($message); break;
		default: cli_info($message); break;
	}
}

if ($command === 'upgrade') {
	run_upgrade();
} else {
	run_install($opts);
}

// The options each command accepts: name => true when the option takes a
// value (--name=VALUE), false when it is a switch.
function cli_known_options($command)
{
	if ($command !== 'install') {
		return array(); // upgrade and help take none
	}
	return array(
		'dev'              => false,
		'sql-host'         => true,
		'sql-port'         => true,
		'sql-user'         => true,
		'sql-pass'         => true,
		'nel-db'           => true,
		'tool-db'          => true,
		'domain-db'        => true,
		'domain-name'      => true,
		'users-dir'        => true,
		'setup-password'   => true,
		'admin-user'       => true,
		'admin-pass'       => true,
		'admin-login'      => true,
		'admin-password'   => true,
		'admin-email'      => true,
		'private-php'      => true,
		'no-role-service'  => false,
		'no-role-domain'   => false,
	);
}

// Anything on the command line that the command does not know is refused.
// It used to be ignored without a word: a mistyped option installed with the
// default in its place, and "--sql-pass secret" set the password to "1".
function cli_parse_options($command, $args)
{
	$known = cli_known_options($command);
	// these went away with the AMS account management system
	$removedWithAms = array('no-role-support', 'ams-db', 'ams-lib-db');
	// these were listed in this file's header and never did anything: both
	// roles are installed unless the --no-role- option says otherwise. They
	// get their own message on install, the command they were listed for.
	$neverDidAnything = array('role-service', 'role-domain');
	$opts = array();
	$errors = array();
	foreach ($args as $arg) {
		if (substr($arg, 0, 2) !== '--') {
			$errors[] = "Unexpected argument: $arg";
			continue;
		}
		$parts = explode('=', substr($arg, 2), 2);
		$name = $parts[0];
		if (in_array($name, $removedWithAms, true)) {
			$errors[] = "Option --$name no longer exists: the AMS account management system was removed, and the support role and the AMS databases with it. Drop the option.";
		} else if ($command === 'install' && in_array($name, $neverDidAnything, true)) {
			$errors[] = "Option --$name does not exist: the role is installed by default, and --no-$name leaves it out. Drop the option.";
		} else if (!array_key_exists($name, $known)) {
			$errors[] = "Unknown option: --$name";
		} else if ($known[$name] && !isset($parts[1])) {
			$errors[] = "Option --$name needs a value: --$name=VALUE";
		} else if (!$known[$name] && isset($parts[1])) {
			$errors[] = "Option --$name does not take a value";
		} else {
			$opts[$name] = isset($parts[1]) ? $parts[1] : true;
		}
	}
	if (!empty($errors)) {
		foreach ($errors as $error) {
			cli_err($error);
		}
		echo "Run 'php setup_cli.php help' for the options of each command.\n";
		exit(1);
	}
	return $opts;
}

function print_usage()
{
	echo <<<'USAGE'
Ryzom Core CLI Setup Tool

Usage:
  php setup_cli.php install [options]    Initial installation
  php setup_cli.php upgrade              Upgrade existing installation
  php setup_cli.php help                 Show this help

Install options:
  --dev                     Configure for development shard
  --sql-host=HOST           SQL hostname (default: localhost)
  --sql-port=PORT           SQL port (default: 3306, or 9040 with --dev)
  --sql-user=USER           SQL username (default: root)
  --sql-pass=PASS           SQL password (default: empty)
  --nel-db=NAME             NeL database name (default: nel)
  --tool-db=NAME            NeL Tool database name (default: nel_tool)
  --domain-db=NAME          Ring domain database name
  --domain-name=NAME        Domain name
  --users-dir=PATH          Users directory for MFS
  --setup-password=PASS     Setup password (required unless --dev)
  --admin-user=USER         Shard admin username (default: admin)
  --admin-pass=PASS         Shard admin password (required unless --dev)
  --admin-login=LOGIN       Administrator account: the first game account, with the
  --admin-password=PASS       :DEV: privilege. All three or none; without them no
  --admin-email=ADDRESS       game account exists.
  --private-php=PATH        Private PHP directory (default: ../private_php/)
  --no-role-service         Disable service role
  --no-role-domain          Disable domain role

USAGE;
}

function run_install($opts)
{
	// The web installer requires config.php / setup/version.php /
	// setup/database.php / admin/common.php at top level, so their
	// top-level assignments become globals -- which is what the functions
	// they define read back (connect_database: global $cfg, the migration
	// loops: global $db_nel..., tool_admin_users_add: global $db). In here
	// a bare require makes them locals: the connects got nulls and every
	// migration loop silently ran zero times.
	global $cfg, $NEL_SETUP_VERSION;
	global $db_nel, $db_nel_tool, $db_ring_domain;
	global $db, $tpl;
	// set at the top of this script; the config paths that config.php
	// assigns are read back by the migration helpers (global $PRIVATE_PHP_PATH)
	global $publicPhpDir, $PRIVATE_PHP_PATH, $PUBLIC_PHP_PATH, $USERS_DIR;

	$isDev = isset($opts['dev']);

	cli_info("Starting Ryzom Core installation" . ($isDev ? " (development mode)" : ""));

	// Resolve settings
	$sqlHost = isset($opts['sql-host']) ? $opts['sql-host'] : 'localhost';
	$sqlPort = isset($opts['sql-port']) ? $opts['sql-port'] : ($isDev ? '9040' : '3306');
	$sqlUser = isset($opts['sql-user']) ? $opts['sql-user'] : 'root';
	$sqlPass = isset($opts['sql-pass']) ? $opts['sql-pass'] : '';
	$nelDb = isset($opts['nel-db']) ? $opts['nel-db'] : 'nel';
	$toolDb = isset($opts['tool-db']) ? $opts['tool-db'] : 'nel_tool';
	$domainName = isset($opts['domain-name']) ? $opts['domain-name'] : ($isDev ? 'dev' : gethostname());
	$domainDb = isset($opts['domain-db']) ? $opts['domain-db'] : 'ring_' . $domainName;
	$setupPass = isset($opts['setup-password']) ? $opts['setup-password'] : 'admin';
	$adminUser = isset($opts['admin-user']) ? $opts['admin-user'] : 'admin';
	$adminPass = isset($opts['admin-pass']) ? $opts['admin-pass'] : 'admin';
	$privatePhp = isset($opts['private-php']) ? $opts['private-php'] : '../private_php/';

	$shardWin = strtoupper(substr(PHP_OS, 0, 3)) === 'WIN';
	if (isset($opts['users-dir'])) {
		$usersDir = $opts['users-dir'];
	} else if ($isDev) {
		// forward slashes first: on Windows the path of this file has backslashes, and the replace
		// below would not match, leaving the users directory on this setup folder
		$usersDir = dirname(__FILE__);
		if ($shardWin) {
			$usersDir = str_replace('\\', '/', $usersDir);
		}
		$usersDir = str_replace('/code/web/public_php/setup', '/pipeline/shard_dev/www', $usersDir);
	} else {
		$usersDir = ($shardWin ? 'C:/nevrax/' : '/home/nevrax/') . $domainName . '/www';
	}

	$roleService = !isset($opts['no-role-service']);
	$roleDomain = !isset($opts['no-role-domain']);
	$configureShardDev = $isDev && $roleDomain;

	if (!$roleService && !$roleDomain) {
		cli_err("No server roles enabled. At least one role must be active.");
		exit(1);
	}

	// 'admin' as the setup password and as the shard admin password is fine
	// for a throwaway --dev shard and is exactly what nobody should end up
	// running in front of players: the setup password is the gate on the
	// installer and upgrade pages, and the admin account drives services.
	// Outside --dev, ask for them.
	if (!$isDev) {
		$weak = array();
		if (!isset($opts['setup-password']) || $opts['setup-password'] === true || $opts['setup-password'] === 'admin')
			$weak[] = '--setup-password';
		if (!isset($opts['admin-pass']) || $opts['admin-pass'] === true || $opts['admin-pass'] === 'admin')
			$weak[] = '--admin-pass';
		if (!empty($weak)) {
			cli_err("Refusing to install with the built-in password for: " . implode(', ', $weak));
			cli_err("Pass a password of your own, or add --dev for a throwaway development shard.");
			exit(1);
		}
	} else if ($setupPass === 'admin' || $adminPass === 'admin') {
		cli_warn("Development install: setup and/or admin password left at 'admin'.");
	}

	// The development shard is configured by a script that names its
	// databases itself. Other names used to fail in that script, after
	// config.php was written; refuse them now.
	if ($configureShardDev) {
		require_once('setup/database.php');
		$shardDevProblems = shard_dev_database_problems($nelDb, $toolDb, $domainDb);
		if (!empty($shardDevProblems)) {
			cli_err("--dev configures the development shard, and for that " . implode(', ', $shardDevProblems) . ".");
			cli_err("Leave --nel-db, --tool-db, --domain-db and --domain-name (the default of --domain-db is made from it) at their defaults, or install without --dev or with --no-role-domain.");
			exit(1);
		}
	}

	// The administrator account, the first game account: all three options
	// or none of them. Checked here, before anything is created, since the
	// install cannot be run again once config.php is written. The password
	// is not printed anywhere below.
	$accountOptions = array('admin-login', 'admin-password', 'admin-email');
	$accountMissing = array();
	foreach ($accountOptions as $name) {
		if (!isset($opts[$name]))
			$accountMissing[] = '--' . $name;
	}
	$createAccount = empty($accountMissing);
	if (!$createAccount && count($accountMissing) < count($accountOptions)) {
		cli_err("The administrator account needs --admin-login, --admin-password and --admin-email together; missing: " . implode(', ', $accountMissing));
		exit(1);
	}
	$accountLogin = $createAccount ? trim($opts['admin-login']) : '';
	$accountEmail = $createAccount ? trim($opts['admin-email']) : '';
	$accountPassword = $createAccount ? $opts['admin-password'] : '';
	if ($createAccount) {
		if (!$roleService) {
			cli_err("The administrator account goes into the NeL database, which the service role installs: drop --no-role-service or the --admin-login, --admin-password and --admin-email options.");
			exit(1);
		}
		require_once('setup/database.php');
		$accountError = admin_account_error($accountLogin, $accountPassword, $accountEmail);
		if ($accountError !== '') {
			cli_err("Administrator account: " . $accountError);
			exit(1);
		}
	}

	// Check already installed
	if (file_exists('config.php')) {
		cli_err("Already installed. Use 'upgrade' to update.");
		echo "       To install again from scratch, first drop the databases of the installation and\n";
		echo "       delete config.php and the db_version_* and role_* files from the public PHP\n";
		echo "       directory: $publicPhpDir\n";
		exit(1);
	}

	// Validate private PHP directory
	if (!file_exists($privatePhp)) {
		cli_err("Private PHP directory not found: $privatePhp");
		exit(1);
	}
	cli_ok("Private PHP directory found: " . realpath($privatePhp));

	// Check PHP PDO mysql driver
	if (!in_array('mysql', PDO::getAvailableDrivers(), true)) {
		cli_err("PHP PDO mysql driver is not available.");
		exit(1);
	}
	cli_ok("PHP PDO mysql driver available");

	// Validate writable directories
	$continue = true;
	if ($roleService) {
		$continue = cli_validate_writable('login/logs/') && $continue;
		$continue = cli_validate_writable('admin/graphs_output/') && $continue;
		$continue = cli_validate_writable('admin/templates/default_c/') && $continue;
	}
	$continue = cli_validate_writable('./') && $continue;
	if (!$continue) {
		cli_err("Required directories are not writable. Fix permissions and retry.");
		exit(1);
	}
	cli_ok("Directory permissions verified");

	// Print configuration summary
	cli_info("Configuration:");
	echo "  SQL Server:    $sqlHost:$sqlPort\n";
	echo "  SQL User:      $sqlUser\n";
	echo "  Roles:         " . implode(', ', array_filter(array(
		$roleService ? 'Service' : null,
		$roleDomain ? 'Domain' : null,
	))) . "\n";
	if ($roleService) echo "  NeL DB:        $nelDb\n  Tool DB:       $toolDb\n";
	if ($roleDomain) echo "  Domain:        $domainName\n  Domain DB:     $domainDb\n";
	if ($roleService) echo "  Game account:  " . ($createAccount ? "$accountLogin <$accountEmail> (:DEV:)" : "none") . "\n";
	echo "\n";

	// Connect to MySQL
	try {
		$con = mysqli_connect($sqlHost, $sqlUser, $sqlPass, null, (int)$sqlPort);
	} catch (Exception $e) {
		cli_err("Failed to connect to MySQL: " . $e->getMessage());
		exit(1);
	}
	if (!$con) {
		cli_err("Failed to connect to MySQL: " . mysqli_connect_error());
		exit(1);
	}
	cli_ok("Connected to MySQL server");

	// Create databases. One that is there already is used as it is, and
	// the install keeps track of which ones it made itself: after a failure
	// only those may be named as safe to drop.
	$createdDatabases = array();
	$existingDatabases = array();
	if ($roleService) {
		$continue = cli_create_database($con, $nelDb, $createdDatabases, $existingDatabases) && $continue;
		$continue = cli_create_database($con, $toolDb, $createdDatabases, $existingDatabases) && $continue;
	}
	if ($roleDomain) {
		$continue = cli_create_database($con, $domainDb, $createdDatabases, $existingDatabases) && $continue;
	}

	mysqli_close($con);

	if (!$continue) {
		cli_err("Database creation failed.");
		if (!empty($createdDatabases)) {
			echo "       This run created " . implode(', ', $createdDatabases) . " before it stopped; the next run uses them as they are.\n";
		}
		exit(1);
	}

	// Write config.php
	require_once('setup/config_generation.php');
	require_once('setup/version.php');
	$config = generate_install_config(
		realpath($privatePhp) . '/setup/config/config.php',
		array(
			// $cwd was never set here: both paths resolved to nothing and the
			// generated config got an empty private and public php path. We
			// already changed into the public root above, so the private
			// directory resolves against it.
			'privatePhpDirectory' => realpath($privatePhp),
			'publicPhpDirectory'  => $publicPhpDir,
			'nelSqlHostname'      => $sqlHost,
			'nelSqlPort'          => $sqlPort,
			'nelSqlUsername'       => $sqlUser,
			'nelSqlPassword'      => $sqlPass,
			'nelDatabase'         => $nelDb,
			'toolDatabase'        => $toolDb,
			'nelSetupPassword'    => $setupPass,
			'domainDatabase'      => $domainDb,
			'domainUsersDir'      => $usersDir,
			'nelDomainName'       => $domainName,
			'nelSetupVersion'     => $NEL_SETUP_VERSION,
		)
	);
	if (!$config) {
		cli_err("Cannot read config template from private PHP directory.");
		exit(1);
	}

	if (!file_put_contents('config.php', $config)) {
		cli_err("Cannot write config.php");
		exit(1);
	}
	forget_cached_script('config.php');
	cli_ok("Generated config.php");

	// Copy config_user.php
	$configUser = file_get_contents(realpath($privatePhp) . '/setup/config/config_user.php');
	if ($configUser && !file_exists('config_user.php')) {
		file_put_contents('config_user.php', $configUser);
		cli_ok("Copied config_user.php");
	}

	// Load config to get $cfg and path variables
	require_once('config.php');

	require_once('setup/database.php');

	// Run database migrations
	if ($roleService) {
		$continue = upgrade_service_databases($continue);
	}
	if ($roleDomain) {
		$continue = upgrade_domain_databases($continue);
	}

	// Create shard admin user
	if ($continue && $roleService) {
		$origDir = getcwd();
		if (chdir('admin/')) {
			try {
				// setup/header.php sets this for the web installer: it tells
				// the admin tool bootstrap to skip session auth (which would
				// print the login page and start a session from the CLI)
				global $NEL_SETUP_SESSION;
				$NEL_SETUP_SESSION = true;
				require_once('common.php');
				require_once('functions_tool_administration.php');
				$result = tool_admin_users_add($adminUser, $adminPass, '1', '1');
				if ($result === '') {
					cli_ok("Shard admin user '$adminUser' created");
				} else {
					cli_err("Failed to create shard admin: $result");
					$continue = false;
				}
			} catch (Exception $e) {
				cli_err("Failed to create shard admin: " . $e->getMessage());
				$continue = false;
			}
			chdir($origDir);
		} else {
			cli_warn("Cannot change to admin/ directory, skipping shard admin creation");
		}
	}

	// Configure development shard
	if ($continue && $configureShardDev) {
		// Mock the POST variable that configure_shard_dev expects
		$_POST['domainUsersDir'] = $usersDir;
		$continue = configure_shard_dev($continue);
	}

	// Create the administrator account, after the development shard so that
	// its domain is there to get a permission row
	if ($continue && $createAccount) {
		$continue = create_admin_account($continue, $accountLogin, $accountPassword, $accountEmail);
	}

	// Write role flags
	if ($continue && $roleService) file_put_contents('role_service', '1');
	if ($continue && $roleDomain) file_put_contents('role_domain', '1');

	echo "\n";
	if ($continue) {
		cli_ok("Installation complete!");
		if ($createAccount) {
			cli_ok("Game account '$accountLogin' <$accountEmail> is the administrator, with the :DEV: privilege.");
			$accountNotes = admin_account_notes($accountLogin, $nelDb);
			$accountStatement = array_pop($accountNotes); // kept on one line, to copy
			foreach ($accountNotes as $line) {
				echo "       " . str_replace("\n", "\n       ", wordwrap($line, 88, "\n", false)) . "\n";
			}
			echo "         $accountStatement\n";
		} else if ($roleService) {
			cli_warn("No game account exists: --admin-login, --admin-password and --admin-email were not given.");
			echo "       Registration is closed by default, and only an account with the :DEV: privilege\n";
			echo "       can open it. An install that passes the three options creates that account. On\n";
			echo "       this one, make it by hand in the '$nelDb' database:\n";
			echo "         1. UPDATE setting SET value = '1' WHERE setting = 'registration_open';\n";
			echo "         2. register in the account tool (account/?page=register)\n";
			echo "         3. UPDATE user SET Privilege = ':DEV:' WHERE Login = '<the login you registered>';\n";
			echo "         4. close the registration again on the Dev page of the account tool, unless\n";
			echo "            it is meant to stay open\n";
		}
	} else {
		cli_err("Installation completed with errors.");
		// config.php is on disk by now: the install cannot simply be run again
		echo "       " . str_replace("\n", "\n       ", wordwrap(failed_install_notes($createdDatabases, $existingDatabases), 88, "\n", false)) . "\n";
		echo "       Public PHP directory: $publicPhpDir\n";
		exit(1);
	}
}

function run_upgrade()
{
	// Same scope story as run_install(): the required files' top-level
	// assignments must land in the globals their functions read back.
	global $cfg, $NEL_SETUP_VERSION, $NEL_SETUP_VERSION_CONFIGURED;
	global $db_nel, $db_nel_tool, $db_ring_domain;
	global $PRIVATE_PHP_PATH, $PUBLIC_PHP_PATH, $NEL_SETUP_PASSWORD;
	global $NEL_DOMAIN_NAME, $USERS_DIR;

	if (!file_exists('config.php')) {
		cli_err("Not installed. Run 'install' first.");
		exit(1);
	}

	cli_info("Starting Ryzom Core upgrade");

	require_once('config.php');
	require_once('setup/version.php');
	if (!isset($NEL_SETUP_VERSION_CONFIGURED)) {
		$NEL_SETUP_VERSION_CONFIGURED = 1;
	}

	if ($NEL_SETUP_VERSION_CONFIGURED >= $NEL_SETUP_VERSION) {
		cli_ok("Already up to date (version $NEL_SETUP_VERSION_CONFIGURED).");
		return;
	}

	cli_info("Upgrading from version $NEL_SETUP_VERSION_CONFIGURED to $NEL_SETUP_VERSION");

	require_once('setup/database.php');

	$continue = true;

	if (file_exists('role_service')) {
		$continue = upgrade_service_databases($continue);
	}
	if (file_exists('role_domain')) {
		$continue = upgrade_domain_databases($continue);
	}
	if (file_exists('role_service') && $NEL_SETUP_VERSION_CONFIGURED < 14 && $NEL_DOMAIN_NAME === 'dev') {
		$continue = upgrade_shard_dev_localhost($continue);
	}

	// A development install on Windows took this setup folder for the shard web storage (its detection replaced
	// the path before converting the backslashes); the storage is the www folder of the development shard.
	$setupTail = '/code/web/public_php/setup';
	$usersDirSlashes = str_replace('\\', '/', $USERS_DIR);
	if ($NEL_DOMAIN_NAME === 'dev' && substr($usersDirSlashes, -strlen($setupTail)) === $setupTail) {
		$fixedUsersDir = substr($usersDirSlashes, 0, -strlen($setupTail)) . '/pipeline/shard_dev/www';
		cli_info("Shard web storage moved from $USERS_DIR to $fixedUsersDir");
		$USERS_DIR = $fixedUsersDir;
	}

	// Rewrite config.php with updated version
	if ($continue) {
		require_once('setup/config_generation.php');
		$config = generate_upgrade_config(
			$PRIVATE_PHP_PATH . '/setup/config/config.php',
			$cfg, $PRIVATE_PHP_PATH, $PUBLIC_PHP_PATH,
			$NEL_SETUP_PASSWORD, $NEL_DOMAIN_NAME, $NEL_SETUP_VERSION,
			$cfg['db']['ring']['name'], $USERS_DIR,
			$NEL_SETUP_VERSION_CONFIGURED
		);
		if (!$config) {
			cli_err("Cannot read config template");
			$continue = false;
		} else {
			if (file_put_contents('config.php', $config)) {
				forget_cached_script('config.php');
				cli_ok("Updated config.php to version $NEL_SETUP_VERSION");
			} else {
				cli_err("Cannot write config.php");
				$continue = false;
			}
		}
	}

	// Create config_user.php if missing
	if ($continue && !file_exists('config_user.php')) {
		$configUser = file_get_contents($PRIVATE_PHP_PATH . '/setup/config/config_user.php');
		if ($configUser) {
			file_put_contents('config_user.php', $configUser);
			cli_ok("Copied config_user.php");
		}
	}

	echo "\n";
	if ($continue) {
		cli_ok("Upgrade complete!");
		// this process cannot reach the opcode cache of the web server
		echo "       If the site still answers that an upgrade is pending, the web server is serving\n";
		echo "       the previous config.php from its opcode cache: it picks the new one up after its\n";
		echo "       revalidation interval, or after a reload when it does not check timestamps.\n";
	} else {
		cli_err("Upgrade completed with errors.");
		exit(1);
	}
}

function cli_validate_writable($path)
{
	if (!file_exists($path)) {
		// Directory doesn't exist, try to create it
		if (substr($path, -1) === '/') {
			if (@mkdir($path, 0755, true)) {
				return true;
			}
		}
		cli_warn("Path does not exist: $path (non-fatal)");
		return true; // Non-fatal for CLI setup
	}
	if (!is_writable($path)) {
		cli_err("Not writable: $path");
		return false;
	}
	return true;
}

// Makes sure the database is there. The name goes into $createdDatabases
// when this call created it and into $existingDatabases when it was there
// already, in which case it is used as it is.
function cli_create_database($con, $name, &$createdDatabases, &$existingDatabases)
{
	$escaped = mysqli_real_escape_string($con, $name);
	$result = mysqli_query($con, "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '$escaped'");
	if (!$result) {
		cli_err("Error looking for database '$name': " . mysqli_error($con));
		return false;
	}
	$exists = mysqli_num_rows($result) > 0;
	mysqli_free_result($result);
	if ($exists) {
		$existingDatabases[] = $name;
		cli_ok("Database '$name' exists already, using it as it is");
		return true;
	}
	$sql = "CREATE DATABASE `$escaped` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci";
	if (mysqli_query($con, $sql)) {
		$createdDatabases[] = $name;
		cli_ok("Database '$name' created");
		return true;
	} else {
		cli_err("Error creating database '$name': " . mysqli_error($con));
		return false;
	}
}

/* end of file */
