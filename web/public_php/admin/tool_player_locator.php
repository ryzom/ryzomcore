<?php

	require_once('common.php');
	require_once('functions_tool_main.php');
	require_once('functions_tool_player_locator.php');

	if (!tool_admin_applications_check('tool_player_locator'))	nt_common_redirect('index.php');

	nt_common_add_debug('-- Starting on \'tool_player_locator.php\'');

	$tpl->assign('tool_title', "Player Locator");

	$view_domain_id = nt_auth_get_session_var('view_domain_id');
	$view_shard_id 	= nt_auth_get_session_var('view_shard_id');

	if (!$view_domain_id)
	{
		$view_domain_id	= $nel_user['group_default_domain_id'];
		$view_shard_id	= $nel_user['group_default_shard_id'];
		nt_auth_set_session_var('view_domain_id', $view_domain_id);
		nt_auth_set_session_var('view_shard_id', $view_shard_id);
	}

	if (isset($NELTOOL['GET_VARS']['domain']))
	{
		if ($view_domain_id != $NELTOOL['GET_VARS']['domain'])
		{
			$view_domain_id = $NELTOOL['GET_VARS']['domain'];
			nt_auth_set_session_var('view_domain_id', $view_domain_id);

			$view_shard_id = null;
			nt_auth_unset_session_var('view_shard_id');
		}
	}

	if (isset($NELTOOL['GET_VARS']['shard']))
	{
		$view_shard_id = $NELTOOL['GET_VARS']['shard'];
		nt_auth_set_session_var('view_shard_id', $view_shard_id);
	}

	if (isset($NELTOOL['GET_VARS']['refdata']))
	{
		$tmp_data = unserialize(base64_decode($NELTOOL['GET_VARS']['refdata']));
		if (is_array($tmp_data))
		{
			$NELTOOL['POST_VARS'] = $tmp_data;
		}
	}

	$current_refresh_rate = nt_auth_get_session_var('current_refresh_rate');

	if (isset($_POST['services_refresh']))
	{
		if ($current_refresh_rate != $_POST['services_refresh'])
		{
			$current_refresh_rate = $_POST['services_refresh'];
			nt_auth_set_session_var('current_refresh_rate',$current_refresh_rate);
		}
	}

	if ($current_refresh_rate == null)
	{
		$current_refresh_rate = 0;
	}
	elseif ($current_refresh_rate > 0)
	{
		$tpl->assign('nel_tool_refresh',	'<meta http-equiv=refresh content="'. $current_refresh_rate .'">');
	}

	$tpl->assign('tool_refresh_list',		$refresh_rates);
	$tpl->assign('tool_refresh_rate',		$current_refresh_rate);

	$tpl->assign('tool_domain_list',		$nel_user['access']['domains']);
	$tpl->assign('tool_domain_selected',	$view_domain_id);

	$tpl->assign('tool_shard_list',			$nel_user['access']['shards']);
	$tpl->assign('tool_shard_selected',		$view_shard_id);

	$tool_shard_filters	= tool_main_get_shard_ids($view_shard_id);
	$tpl->assign('tool_shard_filters',		$tool_shard_filters);

	if (tool_admin_applications_check('tool_player_locator_display_players'))		$tpl->assign('restriction_tool_player_locator_display_players',	true);
	if (tool_admin_applications_check('tool_player_locator_locate'))				$tpl->assign('restriction_tool_player_locator_locate',			true);
	if (tool_admin_applications_check('tool_player_locator_userid_check'))			$tpl->assign('restriction_tool_player_locator_userid_check', 	true);
	if (tool_admin_applications_check('tool_player_locator_csr_relocate'))			$tpl->assign('restriction_tool_player_locator_csr_relocate', 	true);

	if ($view_domain_id)
	{
		$tool_as_error = null;

		$AS_Name 		= tool_main_get_domain_name($view_domain_id);
		$AS_Host 		= tool_main_get_domain_host($view_domain_id);
		$AS_Port 		= tool_main_get_domain_port($view_domain_id);
		$AS_ShardName	= tool_main_get_shard_name($view_shard_id);
		$AS_Application	= tool_main_get_domain_data($view_domain_id, 'domain_application');
		$AS_RingSQL		= tool_main_get_domain_data($view_domain_id, 'domain_sql_string');

		$tpl->assign('tool_page_title', 'Player Locator - '. $AS_Name . ($AS_ShardName != '' ? ' / '. $AS_ShardName : ''));

		$tool_as_error = null;

		if ($AS_Host && $AS_Port)
		{
			$adminService = new MyAdminService;
			if (@$adminService->connect($AS_Host, $AS_Port, $res) === false)
			{
				nt_common_add_debug($res);
				$tpl->assign('tool_domain_error', $res );
			}
			else
			{
				if (isset($NELTOOL['POST_VARS']['services_pl']))
				{
					$tool_services_pl = $NELTOOL['POST_VARS']['services_pl'];
					$tpl->assign('tool_post_data',	base64_encode(serialize($NELTOOL['POST_VARS'])));

					switch ($tool_services_pl)
					{
						case 'display players':

							if (tool_admin_applications_check('tool_player_locator_display_players'))
							{
								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list))
								{
									$service_command = 'displayPlayers';

									nt_log("Domain '$AS_Name' : '$service_command' on ". implode(', ',array_values($service_list)));

									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									$tpl->assign('tool_execute_result', '');
									$command_return_data = array();

									reset($service_list);
									foreach($service_list as $service)
									{
										//nt_common_add_debug("about to run 'displayPlayers' on '$service' ...");

										$adminService->serviceCmd($service, $service_command);
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
										}
										else
										{
											// the locator displays a nice output, no need for the raw one
											//$tpl->assign('tool_execute_command', 	$service_command);
										}
									}

									if (sizeof($command_return_data))
									{
										$player_data = tool_pl_parse_display_players($command_return_data);
										$tpl->assign('tool_player_data',	$player_data);
									}
								}
							}

							break;

//						case 'csr relocate':
						case 'ani':
						case 'ari':
						case 'lea':

							if (tool_admin_applications_check('tool_player_locator_csr_relocate'))
							{
								$relocate_su	= $NELTOOL['POST_VARS']['pl_su'];
								$relocate_shardid	= $NELTOOL['POST_VARS']['relocate_shardid'];
								$relocate_eid	= $NELTOOL['POST_VARS']['relocate_eid'];

								if ($relocate_eid != 'na' && $relocate_shardid != 'na')
								{
									$service			= $relocate_su;
									$service_command	= 'cs.relocChar ' . $relocate_eid . ' ' . $relocate_shardid;

									nt_common_add_debug("about to run command '$service_command' on '$service' ...");

									$tpl->assign('tool_execute_result', '');
									$command_return_data = array();

									$adminService->serviceCmd($service, $service_command);
									if (!$adminService->waitCallback())
									{
										nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
									}
									else
									{
										//$tpl->assign('tool_execute_command', 	$service_command);
									}

									if (sizeof($command_return_data))
									{
										$relocate_data = tool_pl_parse_relocate($command_return_data);
										$tpl->assign('tool_relocate_data',		$relocate_data);
									}

								}
							}

							//break;

						case 'locate':

							if (tool_admin_applications_check('tool_player_locator_locate'))
							{
								$tool_locate_name	= trim($NELTOOL['POST_VARS']['services_pl_locate']);
								$tpl->assign('tool_locate_value', $tool_locate_name);

								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list) && ($tool_locate_name != ''))
								{
									$service_command = 'playerInfo '. $tool_locate_name;
									nt_log("Domain '$AS_Name' : '$service_command' on ". implode(', ',array_values($service_list)));

									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									//$tpl->assign('tool_execute_result', '');
									$command_return_data = array();

									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run 'playerInfo' on '$service' ...");

										$adminService->serviceCmd($service, $service_command);
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
										}
										else
										{
											// the locator displays a nice output, no need for the raw one
											//$tpl->assign('tool_execute_command', 	$service_command);
										}
									}

									if (sizeof($command_return_data))
									{
										$locate_data = tool_pl_parse_locate($command_return_data);
										$tpl->assign('tool_locate_data',		$locate_data);
									}
								}
							}

							break;

						case 'Compute User IDs and Clear SQL Cache':

							if (tool_admin_applications_check('tool_player_locator_userid_check'))
							{
								$check_su = $NELTOOL['POST_VARS']['pl_su'];

								tool_pl_fix_character_check_list($AS_Application);

								$service = $check_su;
								$service_command = 'sqlObjectCache.clearCache';
								nt_common_add_debug("about to run command '$service_command' on '$service' ...");

								$adminService->serviceCmd($service, $service_command);
								if (!$adminService->waitCallback())
								{
									nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
								}
							}

							break;

					}
				}

				if (isset($NELTOOL['GET_VARS']['eid']))
				{
					$locate_eid = $NELTOOL['GET_VARS']['eid'];

					// someday i'll do something here :)
				}

				$status = $adminService->getStates();
				nt_common_add_debug($status);

				$domainServices		= tool_main_parse_status($status);

				$filteredServices	= array();
				reset($domainServices);
				foreach($domainServices as $aKey => $aService)
				{
					// we are only interested in EGS
					if ($aService['ShortName'] == 'EGS')
					{
						$filteredServices[] = $aService;
					}
				}

				$tpl->assign('tool_services_list',	$filteredServices);

				$tpl->assign('shard_su_name',	tool_main_get_su_from_status($domainServices));

				// user_id == 0 check system
				if (tool_admin_applications_check('tool_player_locator_userid_check'))
				{
					$user_check_list = tool_pl_get_character_check_list($AS_Application, $AS_RingSQL);
					$tpl->assign('user_check_list', $user_check_list);
				}


			}
		}
	}

	$tpl->display('tool_player_locator.tpl');

?>