<?php

	// Built for argv (CLI). Over http there is no $argv and NELTOOL_NO_USER_NEEDED
	// would open the admin stack with no session — refuse non-CLI.
	if (PHP_SAPI !== 'cli')
	{
		header('HTTP/1.1 403 Forbidden');
		echo "Access denied\n";
		return;
	}

	set_time_limit(900); // should not exceed 15 minutes

	define('NELTOOL_NO_USER_NEEDED',    true);

	require_once('../common.php');
	require_once('../functions_tool_main.php');

	$params = array();

	if (!isset($argv) || !is_array($argv))
		return;

	reset($argv);
	foreach($argv as $sValue)
	{
		if (strpos($sValue, '=') !== false)
		{
			$aData = explode('=', $sValue);
			$params[$aData[0]] = $aData[1];
		}
	}

	if (isset($params['restart_id']) && isset($params['services']))
	{
		while (true)
		{
			print_r($params);
			sleep(10);
		}
	}

?>