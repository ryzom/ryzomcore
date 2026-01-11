<?php

	set_time_limit(900); // should not exceed 15 minutes

	define('NELTOOL_NO_USER_NEEDED',    true);

	require_once('../common.php');
	require_once('../functions_tool_main.php');

	$params = array();

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