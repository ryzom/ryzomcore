<?php

    ob_start();
    set_time_limit(180); // 3 min time out

    define('NELTOOL_NO_USER_NEEDED',    true);
    define('NELTOOL_CRON_DEBUG',        false);

	if ($_GET['dbg']) define('NELTOOL_CRON_DEBUG', true);

	if (defined('NELTOOL_CRON_DEBUG')) echo "Checking HDs ... \n";

	require_once('../common.php');
	require_once('../functions_tool_main.php');

	$domainList = tool_admin_domains_get_list();
	$service_command = "aes.execScript df";
	if (defined('NELTOOL_CRON_DEBUG')) echo "domainList\n". print_r($domainList,true) ."\n";

	$aesList = array();

	if (is_array($domainList))
	{
		reset($domainList);
		foreach($domainList as $domain_data)
		{
			if ($domain_data['domain_hd_check'] == 1)
			{
				//echo '<pre>'. print_r($domain_data, true) .'</pre>';

				$adminService = new MyAdminService;
				if (@$adminService->connect($domain_data['domain_as_host'], $domain_data['domain_as_port'], $res) !== false)
				{
					$status			= $adminService->getStates();
					$domainServices	= tool_main_parse_status($status);
					$aesList		= tool_main_get_aes_from_status($domainServices);

					//echo '<pre>'. print_r($aesList, true) .'</pre>';

					if (sizeof($aesList))
					{
						reset($aesList);
						foreach($aesList as $service)
						{
							$adminService->serviceCmd($service, $service_command);
							if (!$adminService->waitCallback())
							{
								// error
							}
						}

						$aes_df_result = $tpl->get_template_vars('tool_execute_result');
						$tpl->clear_assign('tool_execute_result');

						if (defined('NELTOOL_NO_USER_NEEDED') && defined('NELTOOL_CRON_DEBUG')) echo '<pre>'. print_r($aes_df_result, true) .'</pre>';

						tool_main_update_hd_data_for_domain($domain_data['domain_id'], $aes_df_result);
					}

					unset($adminService);
				}

			}
		}
	}

	if (defined('NELTOOL_CRON_DEBUG')) echo "checked ". sizeof($aesList) ." servers!\n";

    if (defined('NELTOOL_CRON_DEBUG'))
    {
        if ($fp = fopen("./logs/checkdisk_". date("YmdHis", time()) .".log", "w"))
        {
            fputs($fp, ob_get_contents());
            fclose($fp);
        }
    }
    
    ob_end_clean();

?>
