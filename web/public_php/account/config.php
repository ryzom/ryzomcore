<?php

// Account Management Tool - Configuration
// Reuses the shared config from the setup directory

require_once(dirname(dirname(__FILE__)) . '/config.php');

// NEL shard database (contains `user`, `domain`, `shard`, `permission` tables)
$nel_db_host = $cfg['db']['shard']['host'];
$nel_db_port = $cfg['db']['shard']['port'];
$nel_db_user = $cfg['db']['shard']['user'];
$nel_db_pass = $cfg['db']['shard']['pass'];
$nel_db_name = $cfg['db']['shard']['name'];

// Ring domain database (contains `ring_users`, `characters`, `sessions`, `session_participant` tables)
$ring_db_host = $cfg['db']['ring']['host'];
$ring_db_port = $cfg['db']['ring']['port'];
$ring_db_user = $cfg['db']['ring']['user'];
$ring_db_pass = $cfg['db']['ring']['pass'];
$ring_db_name = $cfg['db']['ring']['name'];

/**
 * Get a PDO connection to a database.
 */
function getDatabase($host, $port, $user, $pass, $name)
{
	static $connections = array();
	$key = $host . ':' . $port . '/' . $name;
	if (!isset($connections[$key])) {
		$dsn = "mysql:host=$host;port=$port;dbname=$name;charset=utf8mb4";
		$connections[$key] = new PDO($dsn, $user, $pass, array(
			PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
			PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
			PDO::ATTR_EMULATE_PREPARES => false,
		));
	}
	return $connections[$key];
}

/**
 * Get a PDO connection to the NEL shard database.
 */
function getNelDatabase()
{
	global $nel_db_host, $nel_db_port, $nel_db_user, $nel_db_pass, $nel_db_name;
	return getDatabase($nel_db_host, $nel_db_port, $nel_db_user, $nel_db_pass, $nel_db_name);
}

/**
 * Get a PDO connection to a ring domain database by name.
 */
function getRingDatabase($ringDbName = null)
{
	global $ring_db_host, $ring_db_port, $ring_db_user, $ring_db_pass, $ring_db_name;
	$name = $ringDbName ? $ringDbName : $ring_db_name;
	return getDatabase($ring_db_host, $ring_db_port, $ring_db_user, $ring_db_pass, $name);
}

/**
 * Generate a crypt-compatible salt.
 */
function generateSalt()
{
	$chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./';
	$salt = '$6$';
	for ($i = 0; $i < 16; $i++) {
		$salt .= $chars[random_int(0, strlen($chars) - 1)];
	}
	$salt .= '$';
	return $salt;
}

/**
 * Hash a password using crypt().
 */
function hashPassword($password)
{
	return crypt($password, generateSalt());
}

/**
 * Verify a password against a crypt hash.
 */
function verifyPassword($password, $hash)
{
	if ($hash[0] === '$') {
		// Modern crypt hash (SHA-256 or SHA-512)
		$salt = substr($hash, 0, strrpos($hash, '$') + 1);
	} else {
		// Legacy DES-based crypt
		$salt = substr($hash, 0, 2);
	}
	return crypt($password, $salt) === $hash;
}

/**
 * Redirect to a page within the account tool.
 */
function redirect($page)
{
	header('Location: index.php?page=' . urlencode($page));
	exit;
}

/**
 * Escape HTML output.
 */
function h($str)
{
	return htmlspecialchars($str, ENT_QUOTES, 'UTF-8');
}

/**
 * Generate or retrieve a CSRF token for the current session.
 */
function csrfToken()
{
	if (empty($_SESSION['csrf_token'])) {
		$_SESSION['csrf_token'] = bin2hex(random_bytes(32));
	}
	return $_SESSION['csrf_token'];
}

/**
 * Return a hidden input field with the CSRF token.
 */
function csrfField()
{
	return '<input type="hidden" name="csrf_token" value="' . h(csrfToken()) . '">';
}

/**
 * Validate the CSRF token from the POST request.
 */
function csrfValidate()
{
	if (empty($_POST['csrf_token']) || empty($_SESSION['csrf_token'])) {
		return false;
	}
	return hash_equals($_SESSION['csrf_token'], $_POST['csrf_token']);
}

/* end of file */
