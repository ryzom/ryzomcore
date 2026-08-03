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
 *   --ams-db=NAME             AMS database name (default: nel_ams)
 *   --ams-lib-db=NAME         AMS Library database name (default: nel_ams_lib)
 *   --domain-db=NAME          Ring domain database name (default: ring_dev or ring_HOSTNAME)
 *   --domain-name=NAME        Domain name (default: dev or HOSTNAME)
 *   --users-dir=PATH          Users directory for MFS (auto-detected)
 *   --setup-password=PASS     Setup password (default: admin)
 *   --admin-user=USER         Shard admin username (default: admin)
 *   --admin-pass=PASS         Shard admin password (default: admin)
 *   --private-php=PATH        Private PHP directory (default: ../private_php/)
 *   --role-service            Enable service role (default: enabled)
 *   --role-domain             Enable domain role (default: enabled)
 *   --no-role-service         Disable service role
 *   --no-role-support         Disable support role
 *   --no-role-domain          Disable domain role
 */

if (PHP_SAPI !== 'cli') {
	echo "This script must be run from the command line.\n";
	exit(1);
}

error_reporting(E_ALL);
ini_set('display_errors', 'on');

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

if ($command === 'help' || $command === '--help' || $command === '-h') {
	print_usage();
	exit(0);
}

if ($command !== 'install' && $command !== 'upgrade') {
	cli_err("Unknown command: $command");
	print_usage();
	exit(1);
}

// Parse options
$opts = array();
foreach ($args as $arg) {
	if (substr($arg, 0, 2) === '--') {
		$parts = explode('=', substr($arg, 2), 2);
		$opts[$parts[0]] = isset($parts[1]) ? $parts[1] : true;
	}
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
	$message = strip_tags($message);
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
  --ams-db=NAME             AMS database name (default: nel_ams)
  --ams-lib-db=NAME         AMS Library database name (default: nel_ams_lib)
  --domain-db=NAME          Ring domain database name
  --domain-name=NAME        Domain name
  --users-dir=PATH          Users directory for MFS
  --setup-password=PASS     Setup password (default: admin)
  --admin-user=USER         Shard admin username (default: admin)
  --admin-pass=PASS         Shard admin password (default: admin)
  --private-php=PATH        Private PHP directory (default: ../private_php/)
  --no-role-service         Disable service role
  --no-role-support         Disable support role
  --no-role-domain          Disable domain role

USAGE;
}

function run_install($opts)
{
	$isDev = isset($opts['dev']);

	cli_info("Starting Ryzom Core installation" . ($isDev ? " (development mode)" : ""));

	// Resolve settings
	$sqlHost = isset($opts['sql-host']) ? $opts['sql-host'] : 'localhost';
	$sqlPort = isset($opts['sql-port']) ? $opts['sql-port'] : ($isDev ? '9040' : '3306');
	$sqlUser = isset($opts['sql-user']) ? $opts['sql-user'] : 'root';
	$sqlPass = isset($opts['sql-pass']) ? $opts['sql-pass'] : '';
	$nelDb = isset($opts['nel-db']) ? $opts['nel-db'] : 'nel';
	$toolDb = isset($opts['tool-db']) ? $opts['tool-db'] : 'nel_tool';
	$amsDb = isset($opts['ams-db']) ? $opts['ams-db'] : 'nel_ams';
	$amsLibDb = isset($opts['ams-lib-db']) ? $opts['ams-lib-db'] : 'nel_ams_lib';
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
		$usersDir = str_replace('/code/web/public_php/setup', '/pipeline/shard_dev/www', dirname(__FILE__));
		if ($shardWin) {
			$usersDir = str_replace('\\', '/', $usersDir);
		}
	} else {
		$usersDir = ($shardWin ? 'C:/nevrax/' : '/home/nevrax/') . $domainName . '/www';
	}

	$roleService = !isset($opts['no-role-service']);
	$roleSupport = !isset($opts['no-role-support']);
	$roleDomain = !isset($opts['no-role-domain']);
	$configureShardDev = $isDev && $roleDomain;

	if (!$roleService && !$roleSupport && !$roleDomain) {
		cli_err("No server roles enabled. At least one role must be active.");
		exit(1);
	}

	// Check already installed
	if (file_exists('config.php')) {
		cli_err("Already installed. Use 'upgrade' to update, or remove config.php to reinstall.");
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
	if ($roleSupport) {
		$continue = cli_validate_writable('ams/cache/') && $continue;
		$continue = cli_validate_writable('ams/templates_c/') && $continue;
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
		$roleSupport ? 'Support' : null,
		$roleDomain ? 'Domain' : null,
	))) . "\n";
	if ($roleService) echo "  NeL DB:        $nelDb\n  Tool DB:       $toolDb\n";
	if ($roleSupport) echo "  AMS DB:        $amsDb\n  AMS Lib DB:    $amsLibDb\n";
	if ($roleDomain) echo "  Domain:        $domainName\n  Domain DB:     $domainDb\n";
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

	// Create databases
	if ($roleService) {
		$continue = cli_create_database($con, $nelDb) && $continue;
		$continue = cli_create_database($con, $toolDb) && $continue;
	}
	if ($roleDomain) {
		$continue = cli_create_database($con, $domainDb) && $continue;
	}
	if ($roleSupport) {
		$continue = cli_create_database($con, $amsDb) && $continue;
		$continue = cli_create_database($con, $amsLibDb) && $continue;
	}

	mysqli_close($con);

	if (!$continue) {
		cli_err("Database creation failed.");
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
			'amsDatabase'         => $amsDb,
			'amsLibDatabase'      => $amsLibDb,
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
	if ($roleSupport) {
		$continue = upgrade_support_databases($continue);
	}
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

	// Write role flags
	if ($continue && $roleService) file_put_contents('role_service', '1');
	if ($continue && $roleSupport) file_put_contents('role_support', '1');
	if ($continue && $roleDomain) file_put_contents('role_domain', '1');

	echo "\n";
	if ($continue) {
		cli_ok("Installation complete!");
	} else {
		cli_err("Installation completed with errors.");
		exit(1);
	}
}

function run_upgrade()
{
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

	if (file_exists('role_support')) {
		$continue = upgrade_support_databases($continue);
	}
	if (file_exists('role_service')) {
		$continue = upgrade_service_databases($continue);
	}
	if (file_exists('role_domain')) {
		$continue = upgrade_domain_databases($continue);
	}

	// Rewrite config.php with updated version
	if ($continue) {
		require_once('setup/config_generation.php');
		$config = generate_upgrade_config(
			$PRIVATE_PHP_PATH . '/setup/config/config.php',
			$cfg, $PRIVATE_PHP_PATH, $PUBLIC_PHP_PATH,
			$NEL_SETUP_PASSWORD, $NEL_DOMAIN_NAME, $NEL_SETUP_VERSION,
			$cfg['crypt']['key'], $SUPPORT_GROUP_IMAP_CRYPTKEY,
			$cfg['db']['ring']['name'], $USERS_DIR,
			$NEL_SETUP_VERSION_CONFIGURED
		);
		if (!$config) {
			cli_err("Cannot read config template");
			$continue = false;
		} else {
			if (file_put_contents('config.php', $config)) {
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

function cli_create_database($con, $name)
{
	$escaped = mysqli_real_escape_string($con, $name);
	// Try to create, ignore if already exists
	$sql = "CREATE DATABASE IF NOT EXISTS `$escaped` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci";
	if (mysqli_query($con, $sql)) {
		cli_ok("Database '$name' ready");
		return true;
	} else {
		cli_err("Error creating database '$name': " . mysqli_error($con));
		return false;
	}
}

/* end of file */
