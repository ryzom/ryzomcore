<?php
/**
* This file contains all variables needed by other php scripts.
* @author Daan Janssens, mentored by Matthew Lagoe
*/

// WARNING: Do not modify this manually on your web server, it will be
// overwritten by the upgrade process. Instead, add overrides to the
// config_user.php file.

// Variables for database access to the shard database
$cfg['db']['shard']['host']  = '%nelSqlHostname%';
$cfg['db']['shard']['port']  = '%nelSqlPort%';
$cfg['db']['shard']['user']  = '%nelSqlUsername%';
$cfg['db']['shard']['pass']  = '%nelSqlPassword%';
$cfg['db']['shard']['name']  = '%nelDatabase%';

// Variables for database access to the ring_open database (only if domain role)
$cfg['db']['ring']['host']   = '%nelSqlHostname%';
$cfg['db']['ring']['port']   = '%nelSqlPort%';
$cfg['db']['ring']['user']   = '%nelSqlUsername%';
$cfg['db']['ring']['pass']   = '%nelSqlPassword%';
$cfg['db']['ring']['name']   = '%domainDatabase%';

// Variables for database access to the nel_tool database (only if service role)
$cfg['db']['tool']['host']   = '%nelSqlHostname%';
$cfg['db']['tool']['port']   = '%nelSqlPort%';
$cfg['db']['tool']['user']   = '%nelSqlUsername%';
$cfg['db']['tool']['pass']   = '%nelSqlPassword%';
$cfg['db']['tool']['name']   = '%toolDatabase%';

//-----------------------------------------------------------------------------------------
// If true, the server will add automatically unknown user in the database
// (in nel.user, nel.permission, ring.ring_user and ring.characters
$ALLOW_UNKNOWN = false;
// If true, the login service automaticaly create a ring user and a editor character if needed
$CREATE_RING = true;

// PHP server paths
$PUBLIC_PHP_PATH = "%publicPhpDirectory%";
$PRIVATE_PHP_PATH = "%privatePhpDirectory%";

// Shard web storage (without final /)
$USERS_DIR = '%domainUsersDir%';

// Setup password
$NEL_SETUP_PASSWORD = '%nelSetupPassword%';

// Name of current domain (only if domain role)
$NEL_DOMAIN_NAME = '%nelDomainName%';

// Currently configured setup version
$NEL_SETUP_VERSION_CONFIGURED = (int)'%nelSetupVersion%';

// Get installed version
require_once('setup/version.php');

$isWebRequest = PHP_SAPI !== 'cli';
$setupDir = realpath(__DIR__ . '/setup');
$scriptFilename = isset($_SERVER['SCRIPT_FILENAME']) ? realpath($_SERVER['SCRIPT_FILENAME']) : false;
$isSetupScript = $setupDir !== false && $scriptFilename !== false
	&& strpos(str_replace('\\', '/', $scriptFilename), str_replace('\\', '/', $setupDir) . '/') === 0;
$requiresUpgrade = $NEL_SETUP_VERSION_CONFIGURED < $NEL_SETUP_VERSION;
if ($isWebRequest && !$isSetupScript && $requiresUpgrade) {
	$scriptName = isset($_SERVER['SCRIPT_NAME']) ? str_replace('\\', '/', $_SERVER['SCRIPT_NAME']) : '';
	$scriptDirectoryName = rtrim(trim(str_replace('\\', '/', dirname($scriptName)), '/'), '/');
	$scriptDirectory = ($scriptDirectoryName === '' || $scriptDirectoryName === '.') ? '/' : '/' . $scriptDirectoryName;
	$loginDirectorySuffix = '/login';
	$loginResponsePrefix = '0:';
	$isLoginScript = $scriptName !== ''
		&& basename($scriptName) === 'r2_login.php'
		&& strlen($scriptDirectory) >= strlen($loginDirectorySuffix)
		&& substr($scriptDirectory, -strlen($loginDirectorySuffix)) === $loginDirectorySuffix;
	$loginMaintenanceMessage = $loginResponsePrefix . 'Service temporarily unavailable while a database upgrade is pending. Please try again later.';
	header('HTTP/1.1 503 Service Unavailable');
	header('Retry-After: 3600');
	if ($isLoginScript) {
		header('Content-Type: text/plain; charset=utf-8');
		die($loginMaintenanceMessage);
	} else {
		header('Content-Type: text/html; charset=utf-8');
		$maintenancePage = <<<'HTML'
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Maintenance</title>
</head>
<body style="font-family:sans-serif;max-width:48rem;margin:3rem auto;padding:0 1rem;">
<h1>Maintenance pending</h1>
<p>The website is temporarily unavailable while a database upgrade is pending.</p>
<p>Please try again later or contact the server administrator for upgrade progress.</p>
</body>
</html>
HTML;
		die($maintenancePage);
	}
}

// Override user parameters
require_once('config_user.php');

/* end of file */
