<?php

/*
 * Account code that the account tool (public_php/account) and the setup
 * (public_php/setup) share: the database connection, the password rules and
 * hashing, the e-mail rule, and the creation of an account.
 *
 * Functions only, and nothing else gets included from here. The installers
 * run the admin tool bootstrap in the same request, and the admin tool has
 * its own copy of the classes of tools/nel_message.php, so a file that pulls
 * those in (account/config.php does) cannot be loaded next to it.
 */

require_once(dirname(__FILE__) . '/account_name.php');

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

/** Minimum accepted password length for this tool. */
function accountPasswordMinLength()
{
	return 8;
}

/** Cap password length to bound crypt()/DoS cost. */
function accountPasswordMaxLength()
{
	return 256;
}

/**
 * True when a candidate password meets length policy (not complexity).
 */
function accountPasswordAcceptable($password)
{
	if (!is_string($password)) {
		return false;
	}
	$len = strlen($password);
	return $len >= accountPasswordMinLength() && $len <= accountPasswordMaxLength();
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
	$password = (string)$password;
	$hash = (string)$hash;
	// An account row with no password must never authenticate, and indexing
	// an empty string is an error on php 8.
	if (strlen($hash) < 2) {
		return false;
	}
	// Bound work even on failure paths with absurdly long inputs.
	if (strlen($password) > accountPasswordMaxLength()) {
		return false;
	}
	if ($hash[0] === '$') {
		// Modern crypt hash (SHA-256 or SHA-512)
		$last = strrpos($hash, '$');
		if ($last === false || $last < 1) {
			return false;
		}
		$salt = substr($hash, 0, $last + 1);
	} else {
		// Legacy DES-based crypt
		$salt = substr($hash, 0, 2);
	}
	// Constant time, like the login service does: a plain comparison leaks
	// where the two values stop matching.
	return hash_equals($hash, (string)crypt($password, $salt));
}

/**
 * The problem with an e-mail address for an account, as a message for the
 * person who entered it, or '' when the address can be used.
 */
function accountEmailError($email)
{
	if (!filter_var($email, FILTER_VALIDATE_EMAIL) || strlen($email) > 255) {
		return 'Please enter a valid email address.';
	}
	if (nel_is_account_placeholder_email($email)) {
		// reserved for the accounts the login scripts create on first login
		return 'This email address cannot be used.';
	}
	return '';
}

/**
 * The first problem with the fields of a new account, in the order the
 * registration form reports them, as a message for the person who entered
 * them, or '' when the fields are acceptable. Does not touch the database.
 */
function accountNewFieldsError($login, $email, $password, $confirm)
{
	if ($login === '' || $email === '' || $password === '' || $confirm === '') {
		return 'All fields are required.';
	}
	if (!nel_account_name_length_ok($login)) {
		return 'Username must be between ' . nel_account_name_min_length()
			. ' and ' . nel_account_name_max_length() . ' characters.';
	}
	if (!nel_is_valid_account_name($login)) {
		return 'Username may only contain letters, numbers, and underscores.';
	}
	$emailError = accountEmailError($email);
	if ($emailError !== '') {
		return $emailError;
	}
	if (!accountPasswordAcceptable($password)) {
		return 'Password must be between ' . accountPasswordMinLength()
			. ' and ' . accountPasswordMaxLength() . ' characters.';
	}
	if ($password !== $confirm) {
		return 'Passwords do not match.';
	}
	return '';
}

/**
 * Create an account in the nel database.
 *
 * Checks the fields (accountNewFieldsError), refuses a login or an e-mail
 * address that an account already has, inserts the user row and then, for
 * every domain whose status is listed in $accessStatuses, the permission row
 * that lets the account into that domain.
 *
 * @param PDO    $db              the nel database, as getDatabase() returns it
 * @param string $login
 * @param string $email
 * @param string $password        plain text, hashed here
 * @param string $confirm         the password a second time
 * @param string $privilege       stored as it is, '' or a string like ':DEV:'
 * @param array  $accessStatuses  domain statuses (ds_open, ds_dev,
 *                                ds_restricted) to give access for; empty
 *                                for no permission rows
 * @return array  'uid' is the id of the new account and 'error' is ''; on a
 *                refusal 'uid' is 0 and 'error' the message for the person
 * @throws PDOException on a database error; nothing is rolled back
 */
function accountCreate($db, $login, $email, $password, $confirm, $privilege, $accessStatuses)
{
	$error = accountNewFieldsError($login, $email, $password, $confirm);
	if ($error !== '') {
		return array('uid' => 0, 'error' => $error);
	}

	// Check if username already exists
	$stmt = $db->prepare('SELECT UId FROM user WHERE Login = :login');
	$stmt->execute(array(':login' => $login));
	if ($stmt->fetch()) {
		return array('uid' => 0, 'error' => 'This username is already taken.');
	}

	// Check if email already exists
	$stmt = $db->prepare('SELECT UId FROM user WHERE Email = :email');
	$stmt->execute(array(':email' => $email));
	if ($stmt->fetch()) {
		return array('uid' => 0, 'error' => 'This email is already registered.');
	}

	// Create the user directly in the nel user table
	$hashedPassword = hashPassword($password);

	$stmt = $db->prepare('INSERT INTO user (Login, Password, Email, Privilege) VALUES (:login, :pass, :email, :priv)');
	$stmt->execute(array(
		':login' => $login,
		':pass' => $hashedPassword,
		':email' => $email,
		':priv' => $privilege,
	));
	$uid = (int)$db->lastInsertId();

	// Create the permissions for the domains whose status is listed
	$allowedStatuses = array('ds_open', 'ds_dev', 'ds_restricted', 'ds_close');
	$statuses = array();
	foreach (array_map('trim', $accessStatuses) as $st) {
		if (in_array($st, $allowedStatuses, true)) {
			$statuses[] = $st;
		}
	}
	if (!empty($statuses)) {
		$domains = $db->query("SELECT domain_id, status FROM domain");
		foreach ($domains as $domain) {
			if (in_array($domain['status'], $statuses, true)) {
				// The login service turns the domain status into the
				// access privilege it then looks for (ds_dev => DEV),
				// so a row written as OPEN on a dev domain refuses
				// the very account it was meant to admit. It is also
				// a SET column: only its own members may be stored.
				$accessPriv = strtoupper(substr((string)$domain['status'], 3));
				if (!in_array($accessPriv, array('OPEN', 'DEV', 'RESTRICTED'), true)) {
					continue;
				}
				$pstmt = $db->prepare('INSERT INTO permission (UId, DomainId, AccessPrivilege) VALUES (:uid, :did, :priv)');
				$pstmt->execute(array(
					':uid' => $uid,
					':did' => (int)$domain['domain_id'],
					':priv' => $accessPriv,
				));
			}
		}
	}

	return array('uid' => $uid, 'error' => '');
}

/* end of file */
