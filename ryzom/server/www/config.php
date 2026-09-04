<?php
function ryzom_load_ini($file_path) {
	if (!file_exists($file_path))
		return false;

	$config_data = parse_ini_file($file_path, true);

	if ($config_data === false)
		return false;

	foreach ($config_data as $section_name => $section_content) {
		$section_prefix = strtoupper($section_name);
		if (is_array($section_content)) {
			foreach ($section_content as $key => $value) {
				$constant_name = $section_prefix . '_' . strtoupper($key);
				if (is_scalar($value))
					define($constant_name, $value);
			}
		} else {
			$constant_name = strtoupper($section_name);
			$value = $section_content;

			if (is_scalar($value))
					 define($constant_name, $value);
		}
	}
	return true;
}

ryzom_load_ini('/etc/ryzom/shard.ini');

if (file_exists(__DIR__.'/myconfig.php'))
	include_once(__DIR__.'/myconfig.php');

if (!defined('ACCEPT_UNKNOWN_USER'))
	define('ACCEPT_UNKNOWN_USER', false);
if (!defined('OAUTH_ACCESS_URL'))
	define('OAUTH_ACCESS_URL', 'https://me.ryzom.com/api/oauth/gameaccess');
if (!defined('GAME_SUBSCRIPTION_URL'))
	define('GAME_SUBSCRIPTION_URL', 'https://me.ryzom.com/api/account/subscription');
if (!defined('AUTO_CREATE_RING_INFO'))
	define('AUTO_CREATE_RING_INFO', true);
