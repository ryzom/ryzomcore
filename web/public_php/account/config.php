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
 * Include the Ring Session Manager PHP interface for RSM RPC calls.
 * These files provide CRingSessionManagerWeb which communicates with the
 * RSM service via a custom binary socket protocol.
 *
 * The ring scripts use relative includes, so we add the ring and tools
 * directories to the include path before loading them.
 */
$_accountIncludePath = get_include_path();
set_include_path(
	dirname(dirname(__FILE__)) . '/ring' . PATH_SEPARATOR .
	dirname(dirname(__FILE__)) . '/tools' . PATH_SEPARATOR .
	dirname(dirname(__FILE__)) . PATH_SEPARATOR .
	$_accountIncludePath
);
require_once('nel_message.php');
require_once('ring_session_manager_itf.php');
set_include_path($_accountIncludePath);
unset($_accountIncludePath);

/**
 * Callback handler for RSM session actions (close, invite, remove, kick).
 * Captures the result code and message from the RSM response.
 */
class AccountRSMCallback extends CRingSessionManagerWeb
{
	public $resultCode = -1;
	public $resultString = '';
	public $sessionId = 0;

	function invokeResult($userId, $resultCode, $resultString)
	{
		$this->resultCode = $resultCode;
		$this->resultString = $resultString;
	}

	function scheduleSessionResult($charId, $sessionId, $result, $resultString)
	{
		$this->resultCode = $result;
		$this->resultString = $resultString;
		$this->sessionId = $sessionId;
	}
}

/**
 * Connect to the Ring Session Manager for a given domain.
 * Returns an AccountRSMCallback object on success, or false on failure.
 *
 * @param string $sessionManagerAddress  "host:port" from the domain table
 * @return AccountRSMCallback|false
 */
function connectToRSM($sessionManagerAddress)
{
	if (empty($sessionManagerAddress)) {
		return false;
	}
	$addr = explode(':', $sessionManagerAddress);
	if (count($addr) < 2) {
		return false;
	}
	$rsm = new AccountRSMCallback();
	$res = '';
	$port = (int)$addr[1];
	if ($port < 1 || $port > 65535) {
		return false;
	}
	$rsm->connect($addr[0], $port, $res);
	if ($res !== '') {
		return false;
	}
	return $rsm;
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
	$hash = (string)$hash;
	// An account row with no password must never authenticate, and indexing
	// an empty string is an error on php 8.
	if (strlen($hash) < 2) {
		return false;
	}
	if ($hash[0] === '$') {
		// Modern crypt hash (SHA-256 or SHA-512)
		$salt = substr($hash, 0, strrpos($hash, '$') + 1);
	} else {
		// Legacy DES-based crypt
		$salt = substr($hash, 0, 2);
	}
	// Constant time, like the login service does: a plain comparison leaks
	// where the two values stop matching.
	return hash_equals($hash, (string)crypt($password, $salt));
}

/**
 * Check if a privilege string contains a specific privilege.
 * Matches the server-side havePriv() logic: privileges are stored as
 * colon-delimited strings like ":DEV:SGM:GM:" and checked via substring match.
 *
 * @param string $privileges  The user's privilege string (e.g. ":DEV:SGM:")
 * @param string $priv        The privilege to check for (e.g. ":DEV:")
 * @return bool
 */
function hasPriv($privileges, $priv)
{
	if (empty($privileges) || empty($priv)) {
		return false;
	}
	return strpos($privileges, $priv) !== false;
}

/**
 * Check if a privilege string contains any of the given privileges.
 *
 * @param string $privileges  The user's privilege string
 * @param array  $privList    Array of privilege strings to check
 * @return bool
 */
function hasAnyPriv($privileges, $privList)
{
	foreach ($privList as $priv) {
		if (hasPriv($privileges, $priv)) {
			return true;
		}
	}
	return false;
}

/**
 * Check if the current session user has admin privileges.
 * Uses the `admin_privileges` setting (default: :DEV:SGM:GM:).
 */
function isAdmin()
{
	$priv = isset($_SESSION['account_privilege']) ? $_SESSION['account_privilege'] : '';
	$adminPrivs = getSetting('admin_privileges', ':DEV:SGM:GM:');
	$adminCodes = parsePrivileges($adminPrivs);
	foreach ($adminCodes as $code) {
		if (hasPriv($priv, ':' . $code . ':')) {
			return true;
		}
	}
	return false;
}

/**
 * Check if the current session user has the settings privilege.
 * Uses the `settings_privilege` setting (default: :DEV:).
 */
function canEditSettings()
{
	$priv = isset($_SESSION['account_privilege']) ? $_SESSION['account_privilege'] : '';
	$settingsPrivs = getSetting('settings_privilege', ':DEV:');
	$settingsCodes = parsePrivileges($settingsPrivs);
	foreach ($settingsCodes as $code) {
		if (hasPriv($priv, ':' . $code . ':')) {
			return true;
		}
	}
	return false;
}

/**
 * Parse a privilege string into an array of individual privilege names.
 *
 * @param string $privileges  e.g. ":DEV:SGM:GM:"
 * @return array              e.g. array("DEV", "SGM", "GM")
 */
function parsePrivileges($privileges)
{
	if (empty($privileges)) {
		return array();
	}
	$parts = explode(':', $privileges);
	$result = array();
	foreach ($parts as $part) {
		$part = trim($part);
		if ($part !== '') {
			$result[] = $part;
		}
	}
	return $result;
}

/**
 * Privilege rank order, highest to lowest.
 * Matches server-side: DEV > SGM > EM > GM > EG > VG > SG > G > OBSERVER > PR
 */
function privRank($code)
{
	$ranks = array(
		'DEV' => 100,
		'SGM' => 90,
		'EM' => 80,
		'GM' => 70,
		'EG' => 60,
		'VG' => 50,
		'SG' => 40,
		'G' => 30,
		'CM' => 25,
		'OBSERVER' => 20,
		'PR' => 10,
	);
	return isset($ranks[$code]) ? $ranks[$code] : 0;
}

/**
 * Get the highest privilege rank from a privilege string.
 */
function highestPrivRank($privileges)
{
	$codes = parsePrivileges($privileges);
	$max = 0;
	foreach ($codes as $code) {
		$r = privRank($code);
		if ($r > $max) { $max = $r; }
	}
	return $max;
}

/**
 * Check if the current user can edit a target user based on privilege hierarchy.
 * A user can only edit users with strictly lower privilege rank.
 */
function canEditUser($targetPrivilege)
{
	$myPriv = isset($_SESSION['account_privilege']) ? $_SESSION['account_privilege'] : '';
	$myRank = highestPrivRank($myPriv);
	$targetRank = highestPrivRank($targetPrivilege);
	return $myRank > $targetRank;
}

/**
 * Get a human-readable label for a privilege code.
 */
function privilegeLabel($code)
{
	$labels = array(
		'DEV' => 'Developer',
		'SGM' => 'Senior GM',
		'GM' => 'Game Master',
		'VG' => 'Venue Guardian',
		'SG' => 'Senior Guide',
		'G' => 'Guide',
		'EM' => 'Event Manager',
		'EG' => 'Event Guide',
		'CM' => 'Community Manager',
		'OBSERVER' => 'Observer',
		'PR' => 'Press',
	);
	return isset($labels[$code]) ? $labels[$code] : $code;
}

/**
 * Get a setting value from the database, with a fallback default.
 */
function getSetting($key, $default = '')
{
	try {
		$db = getNelDatabase();
		$stmt = $db->prepare('SELECT value FROM setting WHERE setting = :key');
		$stmt->execute(array(':key' => $key));
		$row = $stmt->fetch();
		return $row ? $row['value'] : $default;
	} catch (PDOException $e) {
		return $default;
	}
}

/**
 * Set a setting value in the database.
 */
function setSetting($key, $value)
{
	try {
		$db = getNelDatabase();
		$stmt = $db->prepare('INSERT INTO setting (setting, value) VALUES (:key, :val) ON DUPLICATE KEY UPDATE value = :val2');
		$stmt->execute(array(':key' => $key, ':val' => $value, ':val2' => $value));
	} catch (PDOException $e) {
		// Setting save failed silently; caller should handle via getSetting fallback
	}
}

/**
 * Check if the current session is impersonating another user.
 */
function isImpersonating()
{
	return !empty($_SESSION['impersonate_admin_uid']);
}

/**
 * Start impersonating a target user. Saves the admin's session and
 * switches identity to the target. Requires the admin to outrank the target.
 *
 * @param array $targetUser  Row from the `user` table (UId, Login, Email, Privilege)
 * @return bool  True if impersonation started
 */
function startImpersonation($targetUser)
{
	if (isImpersonating()) {
		return false; // Already impersonating
	}
	if (!canEditUser($targetUser['Privilege'])) {
		return false; // Can't impersonate equal or higher rank
	}
	// Save admin identity
	$_SESSION['impersonate_admin_uid'] = $_SESSION['account_uid'];
	$_SESSION['impersonate_admin_login'] = $_SESSION['account_login'];
	$_SESSION['impersonate_admin_privilege'] = $_SESSION['account_privilege'];
	// Switch to target user
	$_SESSION['account_uid'] = (int)$targetUser['UId'];
	$_SESSION['account_login'] = $targetUser['Login'];
	$_SESSION['account_email'] = $targetUser['Email'];
	$_SESSION['account_privilege'] = $targetUser['Privilege'];
	return true;
}

/**
 * Stop impersonating and restore the admin's session.
 *
 * @return bool  True if impersonation was stopped
 */
function stopImpersonation()
{
	if (!isImpersonating()) {
		return false;
	}
	// Restore admin identity
	$_SESSION['account_uid'] = $_SESSION['impersonate_admin_uid'];
	$_SESSION['account_login'] = $_SESSION['impersonate_admin_login'];
	// Restore the admin's email from the database
	try {
		$db = getNelDatabase();
		$stmt = $db->prepare('SELECT Email FROM user WHERE UId = :uid');
		$stmt->execute(array(':uid' => $_SESSION['impersonate_admin_uid']));
		$row = $stmt->fetch();
		if ($row) {
			$_SESSION['account_email'] = $row['Email'];
		}
	} catch (PDOException $e) {
		// Non-fatal; email will be refreshed on next login
	}
	$_SESSION['account_privilege'] = $_SESSION['impersonate_admin_privilege'];
	// Clear impersonation state
	unset($_SESSION['impersonate_admin_uid']);
	unset($_SESSION['impersonate_admin_login']);
	unset($_SESSION['impersonate_admin_privilege']);
	return true;
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
