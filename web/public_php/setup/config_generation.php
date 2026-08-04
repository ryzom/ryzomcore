<?php

/**
 * Shared config.php generation logic.
 *
 * Used by install.php, upgrade.php, and setup_cli.php to avoid
 * duplicating the config template replacement code.
 */

/**
 * Generate a random key suitable for use as a crypt key.
 */
function generate_crypt_key($length = 16) {
	return substr(str_replace(array('+', '/', '='), '', base64_encode(random_bytes($length * 2))), 0, $length);
}

/**
 * Apply a set of values to the config.php template using str_replace.
 *
 * @param string $template   The raw config.php template content.
 * @param array  $values     Associative array of placeholder => value.
 * @return string            The config content with all placeholders replaced.
 */
function apply_config_values($template, $values) {
	// Single pass: replacing one placeholder at a time meant a value that
	// happened to contain %someOtherPlaceholder% (a password, a path) was
	// itself substituted by a later round.
	$pairs = array();
	foreach ($values as $placeholder => $value) {
		$pairs['%' . $placeholder . '%'] = addslashes((string)$value);
	}
	return strtr($template, $pairs);
}

/**
 * Generate config.php content for a fresh install.
 *
 * @param string $templatePath     Path to the config.php template file.
 * @param array  $params           Associative array with install parameters:
 *   'privatePhpDirectory', 'publicPhpDirectory', 'nelSqlHostname', 'nelSqlPort',
 *   'nelSqlUsername', 'nelSqlPassword', 'nelDatabase', 'toolDatabase',
 *   'amsDatabase', 'amsLibDatabase', 'nelSetupPassword', 'domainDatabase',
 *   'domainUsersDir', 'nelDomainName', 'nelSetupVersion'
 * @return string|false            The generated config content, or false on failure.
 */
function generate_install_config($templatePath, $params) {
	$template = file_get_contents($templatePath);
	if (!$template) {
		return false;
	}

	$values = array(
		'privatePhpDirectory' => $params['privatePhpDirectory'],
		'publicPhpDirectory'  => $params['publicPhpDirectory'],
		'nelSqlHostname'      => $params['nelSqlHostname'],
		'nelSqlPort'          => $params['nelSqlPort'],
		'nelSqlUsername'      => $params['nelSqlUsername'],
		'nelSqlPassword'      => $params['nelSqlPassword'],
		'nelDatabase'         => $params['nelDatabase'],
		'toolDatabase'        => $params['toolDatabase'],
		'amsDatabase'         => $params['amsDatabase'],
		'amsLibDatabase'      => $params['amsLibDatabase'],
		'nelSetupPassword'    => $params['nelSetupPassword'],
		'domainDatabase'      => $params['domainDatabase'],
		'domainUsersDir'      => $params['domainUsersDir'],
		'nelDomainName'       => $params['nelDomainName'],
		'nelSetupVersion'     => $params['nelSetupVersion'],
		'cryptKey'            => isset($params['cryptKey']) ? $params['cryptKey'] : generate_crypt_key(),
		'cryptKeyIMAP'        => isset($params['cryptKeyIMAP']) ? $params['cryptKeyIMAP'] : generate_crypt_key(),
	);

	return apply_config_values($template, $values);
}

/**
 * Generate config.php content for an upgrade (preserving existing values).
 *
 * @param string $templatePath                Path to the config.php template file.
 * @param array  $cfg                         The existing $cfg array from the loaded config.
 * @param string $privatePhpPath              The $PRIVATE_PHP_PATH value.
 * @param string $publicPhpPath               The $PUBLIC_PHP_PATH value.
 * @param string $setupPassword               The $NEL_SETUP_PASSWORD value.
 * @param string $domainName                  The $NEL_DOMAIN_NAME value.
 * @param string $setupVersion                The new $NEL_SETUP_VERSION value.
 * @param string $cryptKey                    The existing crypt key.
 * @param string $cryptKeyIMAP                The existing IMAP crypt key.
 * @param string $domainDatabase              The ring database name.
 * @param string $usersDir                    The users directory.
 * @param int    $previousVersion             The previously configured version (for migration logic).
 * @return string|false                       The generated config content, or false on failure.
 */
function generate_upgrade_config($templatePath, $cfg, $privatePhpPath, $publicPhpPath,
	$setupPassword, $domainName, $setupVersion, $cryptKey, $cryptKeyIMAP,
	$domainDatabase, $usersDir, $previousVersion) {

	$template = file_get_contents($templatePath);
	if (!$template) {
		return false;
	}

	// Handle version-specific migration logic for domain database name
	if ($previousVersion < 2) {
		$domainDatabase = $domainName . "_ring";
	}

	// Handle version-specific migration logic for users directory
	if ($previousVersion < 9) {
		$usersDir = "/home/nevrax/" . $domainName . "/www";
	}

	$values = array(
		'privatePhpDirectory' => $privatePhpPath,
		'publicPhpDirectory'  => $publicPhpPath,
		'nelSqlHostname'      => $cfg['db']['shard']['host'],
		'nelSqlPort'          => $cfg['db']['shard']['port'],
		'nelSqlUsername'      => $cfg['db']['shard']['user'],
		'nelSqlPassword'      => $cfg['db']['shard']['pass'],
		'nelDatabase'         => $cfg['db']['shard']['name'],
		'toolDatabase'        => $cfg['db']['tool']['name'],
		'amsDatabase'         => $cfg['db']['web']['name'],
		'amsLibDatabase'      => $cfg['db']['lib']['name'],
		'nelSetupPassword'    => $setupPassword,
		'domainDatabase'      => $domainDatabase,
		'domainUsersDir'      => $usersDir,
		'nelDomainName'       => $domainName,
		'nelSetupVersion'     => $setupVersion,
		'cryptKey'            => $cryptKey,
		'cryptKeyIMAP'        => $cryptKeyIMAP,
	);

	return apply_config_values($template, $values);
}

?>
