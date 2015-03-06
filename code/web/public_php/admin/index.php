<?php

// Enw4k.XHXuULw

	require_once('common.php');
	require_once('functions_tool_main.php');
	require_once('functions_tool_notes.php');

	nt_common_add_debug('-- Starting on \'index.php\'');

	$tpl->assign('tool_title', "Main");

	$view_domain_id = nt_auth_get_session_var('view_domain_id');
	$view_shard_id 	= nt_auth_get_session_var('view_shard_id');

	if (!$view_domain_id)
	{
		$view_domain_id	= $nel_user['group_default_domain_id'];
		$view_shard_id	= $nel_user['group_default_shard_id'];
		nt_auth_set_session_var('view_domain_id', $view_domain_id);
		nt_auth_set_session_var('view_shard_id', $view_shard_id);
	}

	if (isset($_GET['domain']))
	{
		if ($view_domain_id != $_GET['domain'])
		{
			$view_domain_id = $_GET['domain'];
			nt_auth_set_session_var('view_domain_id', $view_domain_id);

			$view_shard_id = null;
			nt_auth_unset_session_var('view_shard_id');
		}
	}

	if (isset($_GET['shard']))
	{
		$view_shard_id = $_GET['shard'];
		nt_auth_set_session_var('view_shard_id', $view_shard_id);
	}

	if (!tool_main_check_user_domain($view_domain_id))	$view_domain_id = null;
	if (!tool_main_check_user_shard($view_shard_id))	$view_shard_id	= null;

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

	$AS_Host 				= tool_main_get_domain_host($view_domain_id);
	$AS_Port 				= tool_main_get_domain_port($view_domain_id);
	$AS_Name 				= tool_main_get_domain_name($view_domain_id);
	$AS_Application			= tool_main_get_domain_data($view_domain_id, 'domain_application');
	$AS_RingSQL				= tool_main_get_domain_data($view_domain_id, 'domain_sql_string');
	$AS_CsSQL				= tool_main_get_domain_data($view_domain_id, 'domain_cs_sql_string');
	$AS_ShardName			= tool_main_get_shard_name($view_shard_id);
	$AS_InternalName		= tool_main_get_shard_as_id($view_shard_id);
	$AS_ShardRestart		= tool_main_get_shard_data($view_shard_id, 'shard_restart');
	$AS_ShardDomainRestart	= tool_main_get_domain_shard_restart($view_domain_id);
	$AS_ShardLang			= tool_main_get_shard_data($view_shard_id, 'shard_lang');

	$tpl->assign('tool_shard_restart_status',		$AS_ShardRestart);
	$tpl->assign('tool_domain_has_shard_restart',	$AS_ShardDomainRestart);

	$tool_no_domain_lock = false;
	if (($AS_InternalName == '*') && ($AS_ShardDomainRestart > 0))
	{
		$tool_no_domain_lock = true;
		$tpl->assign('tool_no_domain_lock', $tool_no_domain_lock);
	}

	$tpl->assign('tool_page_title', $AS_Name . ($AS_ShardName != '' ? ' / '. $AS_ShardName : ''));

	$tool_annotation_info	= tool_main_get_annotation($view_domain_id, $view_shard_id);
	$tool_lock_info			= tool_main_get_lock($view_domain_id, $view_shard_id);

	// we check if the last activity on the lock is too old, in which case we unlock
	// and only if there are no restart sequence going on (prevent loosing lock when inactive for too long during restart sequence - patch ...)
	if ((LOCK_TIMEOUT > 0) && is_array($tool_lock_info) && ($AS_ShardRestart == 0) && !$tool_no_domain_lock)
	{
		$now = time();
		if ($tool_lock_info['lock_update'] + LOCK_TIMEOUT < $now)
		{
			if ($tool_lock_info['lock_domain_id'])	tool_main_delete_lock_domain($view_domain_id);
			if ($tool_lock_info['lock_shard_id'])	tool_main_delete_lock_shard($view_shard_id);

			$tool_lock_info	= tool_main_get_lock($view_domain_id, $view_shard_id);
		}
	}

	// we update the lock data for the last activity time
	if (is_array($tool_lock_info) && $tool_lock_info['lock_user_name'] == $nel_user['user_name'])
	{
		if ($tool_lock_info['lock_domain_id'])	tool_main_set_lock_domain($view_domain_id, false);
		if ($tool_lock_info['lock_shard_id'])	tool_main_set_lock_shard($view_domain_id, $view_shard_id, false);

		$tool_lock_info	= tool_main_get_lock($view_domain_id, $view_shard_id);
	}

	if (tool_admin_applications_check('tool_main_lock_shard') || tool_admin_applications_check('tool_main_lock_domain'))
	{
		if (isset($NELTOOL['POST_VARS']['lock']))
		{
			$tool_lock_mode = $NELTOOL['POST_VARS']['lock'];
			switch ($tool_lock_mode)
			{
				case 'lock shard':

					if (tool_admin_applications_check('tool_main_lock_shard'))
					{
						tool_main_set_lock_shard($view_domain_id, $view_shard_id);
						$tool_lock_info	= tool_main_get_lock($view_domain_id, $view_shard_id);

						if ($AS_ShardRestart > 0)
						{
							tool_main_set_restart_sequence_user($AS_ShardRestart);
						}
					}

					break;

				case 'lock domain':

					if (tool_admin_applications_check('tool_main_lock_domain'))
					{
						tool_main_set_lock_domain($view_domain_id);
						$tool_lock_info	= tool_main_get_lock($view_domain_id, $view_shard_id);
					}

					break;

				case 'unlock shard':

					if (is_array($tool_lock_info) && $tool_lock_info['lock_user_name'] == $nel_user['user_name'])
					{
						tool_main_delete_lock_shard($view_shard_id);
						$tool_lock_info	= tool_main_get_lock($view_domain_id, $view_shard_id);
					}

					break;

				case 'unlock domain':

					if (is_array($tool_lock_info) && $tool_lock_info['lock_user_name'] == $nel_user['user_name'])
					{
						tool_main_delete_lock_domain($view_domain_id);
						$tool_lock_info	= tool_main_get_lock($view_domain_id, $view_shard_id);
					}

					break;

				case 'update annotation':

					if (is_array($tool_lock_info) && $tool_lock_info['lock_user_name'] == $nel_user['user_name'])
					{
						$tool_annotation	= $NELTOOL['POST_VARS']['annotation'];
						tool_main_set_annotation($view_domain_id, $view_shard_id, $tool_annotation);
						$tool_annotation_info = tool_main_get_annotation($view_domain_id, $view_shard_id);
					}

					break;

				case 'restart sequence':

					if (is_array($tool_lock_info) && $tool_lock_info['lock_user_name'] == $nel_user['user_name'] && tool_admin_applications_check('tool_main_easy_restart'))
					{
						$restart_ws_state  = $NELTOOL['POST_VARS']['restart_ws_state'];

						$restart_first_step = 0;

						// if restart_ws_state == '' means that i didn't find a shard_id
						// which means the shard has been shutdown completely including AES and RAS
						if ($restart_ws_state == 'close' || $restart_ws_state == '') $restart_first_step = 3;

						$tool_restart_info = tool_main_set_restart_mode($view_domain_id, $view_shard_id, $restart_first_step);
					}

					break;
			}
		}

		if (is_array($tool_lock_info) && $tool_lock_info['lock_user_name'] == $nel_user['user_name'])
		{
			if (tool_admin_applications_check('tool_main_start'))				$tpl->assign('restriction_tool_main_start',				true);
			if (tool_admin_applications_check('tool_main_stop'))        		$tpl->assign('restriction_tool_main_stop',				true);
			if (tool_admin_applications_check('tool_main_restart'))     		$tpl->assign('restriction_tool_main_restart',			true);
			if (tool_admin_applications_check('tool_main_kill'))        		$tpl->assign('restriction_tool_main_kill',				true);
			if (tool_admin_applications_check('tool_main_abort'))       		$tpl->assign('restriction_tool_main_abort',				true);
			if (tool_admin_applications_check('tool_main_execute'))     		$tpl->assign('restriction_tool_main_execute',			true);
			if (tool_admin_applications_check('tool_main_ws'))     				$tpl->assign('restriction_tool_main_ws',				true);
			if (tool_admin_applications_check('tool_main_ws_old'))     			$tpl->assign('restriction_tool_main_ws_old',			true);
			if (tool_admin_applications_check('tool_main_reset_counters'))  	$tpl->assign('restriction_tool_main_reset_counters',	true);
			if (tool_admin_applications_check('tool_main_service_autostart'))	$tpl->assign('restriction_tool_main_service_autostart',	true);
			if (tool_admin_applications_check('tool_main_shard_autostart'))		$tpl->assign('restriction_tool_main_shard_autostart',	true);
			if (tool_admin_applications_check('tool_main_easy_restart'))		$tpl->assign('restriction_tool_main_easy_restart',		true);

			$nel_user['has_lock'] = true;
			if ($tool_lock_info['lock_shard_id'] > 0)		$tpl->assign('tool_has_shard_lock',		true);
			elseif ($tool_lock_info['lock_domain_id'] > 0)	$tpl->assign('tool_has_domain_lock',	true);

			if ($AS_ShardRestart > 0)
			{
				// define the shards language
				$tpl->assign('tool_shard_language',			$AS_ShardLang);
				$tpl->assign('tool_language_list',			$tool_language_list);

				$tool_restart_message_reboot_list = tool_admin_restart_messages_get_list_from_name('reboot');
				$tpl->assign('tool_restart_message_reboot_list', 	$tool_restart_message_reboot_list);

				$tool_restart_stop_list = tool_admin_restarts_get_list('DESC');
				$tool_restart_start_list = tool_admin_restarts_get_list('ASC');

				// they are assigned at the end of the script
				//$tpl->assign('tool_restart_stop_actions',	$tool_restart_stop_list);
				//$tpl->assign('tool_restart_start_actions',	$tool_restart_start_list);

				//$tool_restart_message_ready_list = tool_admin_restart_messages_get_list_from_name('ready');
				//$tpl->assign('tool_restart_message_ready_list', 	$tool_restart_message_ready_list);

				//$tool_restart_message_list = tool_admin_restart_messages_get_list_from_name('open');

				// restart information
				$tool_restart_info = tool_main_get_restart_sequence_by_id($AS_ShardRestart);
				$tpl->assign('tool_restart_info', $tool_restart_info);

			}


			$tpl->assign('tool_has_lock',			true);
		}
		else
		{
			$tpl->assign('tool_no_lock',			true);
			$tpl->assign('tool_no_annotation',		true);
		}
	}

	if (tool_admin_applications_check('tool_main_lock_shard'))  $tpl->assign('restriction_tool_main_lock_shard',true);
	if (tool_admin_applications_check('tool_main_lock_domain')) $tpl->assign('restriction_tool_main_lock_domain',true);
	elseif (!tool_admin_applications_check('tool_main_lock_domain')
	      && tool_admin_applications_check('tool_main_lock_shard')
	      && (tool_main_get_shard_as_id($view_shard_id) == "*"))
	{
		// you have shard lock access
		// but you don't have domain access
		// and you are viewing the * shard
		// locking the shard * will lock the domain
		// so you must not be able to lock this shard
		$tpl->assign('tool_cant_lock', true);
	}

	$tpl->assign('tool_lock_info',			$tool_lock_info);
	$tpl->assign('tool_annotation_info',	$tool_annotation_info);

	if (tool_admin_applications_check('tool_notes'))
	{
		$tool_note_list = tool_notes_get_list($nel_user['user_id'], 1);

		if (sizeof($tool_note_list))
		{
			$tpl->assign('restriction_tool_notes',	true);
			$tpl->assign('tool_note_list',			$tool_note_list);
		}
	}


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
			if ($nel_user['has_lock'])
			{
				//nt_common_add_debug("HTTP_POST_VARS");
				//nt_common_add_debug($HTTP_POST_VARS);
				//echo '<pre>'. print_r($HTTP_POST_VARS,true) .'</pre>';
				//die();

				// make sure you are the one who has its name in the restart info
				// as you can take over the lock and the restart sequence
				// we don't want more than 1 client executing the restart commands if they think they still have the lock
				if (isset($tool_restart_info) && ($tool_restart_info['restart_sequence_user_name'] == $nel_user['user_name']) && tool_admin_applications_check('tool_main_easy_restart'))
				{
					$tool_seq_id			= $NELTOOL['POST_VARS']['restart_sequence_id'];
					$tool_seq_step			= $NELTOOL['POST_VARS']['restart_sequence_step'];

					$restart_shard_id 		= $NELTOOL['POST_VARS']['restart_shard_id'];
					$service_su				= $NELTOOL['POST_VARS']['restart_su'];
					$service_egs			= $NELTOOL['POST_VARS']['restart_egs'];

					$restart_stop_services	= $NELTOOL['POST_VARS']['restart_stop_services'];

					if (isset($NELTOOL['POST_VARS']['restart_check_ws']))
					{
						// we are starting the restart sequence
						$restart_ws_state		= $NELTOOL['POST_VARS']['restart_ws_state'];

						if ($restart_ws_state == 'open')
						{
							// shard needs a regular restart

							// - broadcast a message on the shard

							$restart_reboot_message_id 		= $NELTOOL['POST_VARS']['restart_message_reboot_id'];
							$restart_reboot_message_data	= tool_admin_restart_messages_get_id($restart_reboot_message_id);
							$restart_reboot_message			= $restart_reboot_message_data['restart_message_value'];

							if ($restart_reboot_message != '')
							{
								$service_command = "broadcast repeat=10 every=60 ". $restart_reboot_message;

								nt_log("Domain '$AS_Name' : '$service_command' on ". $service_egs);
								nt_common_add_debug("about to run command '$service_command' on '$service_egs' ...");
								$adminService->serviceCmd($service_egs, $service_command);
								if (!$adminService->waitCallback())
								{
									nt_common_add_debug('Error while waiting for callback on service \''. $service_egs .'\' for command : '. $service_command);
								}

								$tpl->clear_assign('tool_execute_result');
							}

							// - prepare next step (timer countdown to 10 minutes)

							if($restart_shard_id == 301)
							{
                                // Fast restart for yubo shard (1mn)
								tool_main_set_restart_sequence_timer($tool_seq_id, 60);
								nt_common_add_debug("fast restart for yubo shard 301");
							}
							else
							{
								tool_main_set_restart_sequence_timer($tool_seq_id, 600);
								nt_common_add_debug("normal restart for live shard");
							}

							// - lets start by locking the WS

							$service_command = 'rsm.setWSState '. $restart_shard_id .' RESTRICTED ""';
							nt_common_add_debug("about to run command '$service_command' on '$service_su' ...");

							$adminService->serviceCmd($service_su, $service_command);
							if (!$adminService->waitCallback())
							{
								nt_common_add_debug('Error while waiting for callback on service \''. $service_su .'\' for command : '. $service_command);
							}

							$tpl->clear_assign('tool_execute_result');
							nt_sleep(VIEW_DELAY);
						}
						else
						{
							// shard needs an immediate restart

							// - prepare next step (timer countdown to a few seconds)

							tool_main_set_restart_sequence_timer($tool_seq_id, 30);
						}

						// - inform CS about the restart
						tool_main_add_restart_notification($tool_seq_id, $service_su, $service_egs, $restart_shard_id, 4101, $AS_CsSQL, $AS_ShardLang);

						// - resend the services to stop for the next step

						$tpl->assign('tool_restart_stop_actions', $restart_stop_services);

						// - move on to the next step

						$tool_restart_info = tool_main_set_next_restart_sequence_step($tool_seq_id, $tool_seq_step + 1);
						$tpl->assign('tool_restart_info', $tool_restart_info);

					}
					elseif (isset($NELTOOL['POST_VARS']['restart_back']))
					{
						// this makes you go to the next step
						if ($tool_seq_id == $tool_restart_info['restart_sequence_id'])
						{
							$tool_restart_info = tool_main_set_next_restart_sequence_step($tool_seq_id, $tool_seq_step - 1);
							$tpl->assign('tool_restart_info', $tool_restart_info);
						}
					}
					elseif (isset($NELTOOL['POST_VARS']['restart_end']))
					{
						tool_main_set_end_restart_sequence($tool_seq_id);
						nt_common_redirect('index.php');
						exit();
					}
					elseif (isset($NELTOOL['POST_VARS']['restart_continue']))
					{
						// this makes you go to the next step
						if ($tool_seq_id == $tool_restart_info['restart_sequence_id'])
						{
							$tool_restart_info = tool_main_set_next_restart_sequence_step($tool_seq_id, $tool_seq_step + 1);
							$tpl->assign('tool_restart_info', $tool_restart_info);
						}
					}
					elseif (isset($NELTOOL['POST_VARS']['restart_giveup']))
					{
						// update klients events to giveup mode

						//$klients_events = explode(',',$tool_restart_info['restart_sequence_events']);
						//tool_main_change_restart_notification($klients_events, 4105, $AS_CsSQL);
						tool_main_add_restart_notification($tool_seq_id, $service_su, $service_egs, $restart_shard_id, 4105, $AS_CsSQL, $AS_ShardLang);

						tool_main_set_end_restart_sequence($tool_seq_id);
						nt_common_redirect('index.php');
						exit();
					}
					elseif (isset($NELTOOL['POST_VARS']['restart_cancel']))
					{
						if ($restart_ws_state != 'open')
						{
							// - broadcast a message on the shard

							$restart_reboot_message_data	= tool_admin_restart_messages_get_list_from_name('cancel', $AS_ShardLang);

							if (sizeof($restart_reboot_message_data))
							{
								$restart_reboot_message = $restart_reboot_message_data[0]['restart_message_value'];

								if ($restart_reboot_message != '')
								{
									$service_command = "broadcast ". $restart_reboot_message;

									nt_log("Domain '$AS_Name' : '$service_command' on ". $service_egs);
									nt_common_add_debug("about to run command '$service_command' on '$service_egs' ...");
									$adminService->serviceCmd($service_egs, $service_command);
									if (!$adminService->waitCallback())
									{
										nt_common_add_debug('Error while waiting for callback on service \''. $service_egs .'\' for command : '. $service_command);
									}

									$tpl->clear_assign('tool_execute_result');
								}

							}

							// update the klients events to cancelled mode

							$klients_events = explode(',',$tool_restart_info['restart_sequence_events']);
							tool_main_change_restart_notification($klients_events, 4104, $AS_CsSQL);

							// - open the WS again

							$service_command = 'rsm.setWSState '. $restart_shard_id .' OPEN ""';
							nt_common_add_debug("about to run command '$service_command' on '$service_su' ...");

							$adminService->serviceCmd($service_su, $service_command);
							if (!$adminService->waitCallback())
							{
								nt_common_add_debug('Error while waiting for callback on service \''. $service_su .'\' for command : '. $service_command);
							}

							$tpl->clear_assign('tool_execute_result');
							nt_sleep(VIEW_DELAY);

							// end the sequence and refresh

							tool_main_set_end_restart_sequence($tool_seq_id);
							nt_common_redirect('index.php');
							exit();
						}
					}
					elseif (isset($NELTOOL['POST_VARS']['restart_wait_timer']))
					{
						// step 1, waited for the timer, lets shutdown the shard

						$service_list = explode(',', $restart_stop_services);

						if (sizeof($service_list))
						{
							// comment out to prevent stopping services for testing purposes
							nt_log("Domain '$AS_Name' : 'stopService' on ". implode(', ',array_values($service_list)));
							reset($service_list);
							foreach($service_list as $service)
							{
								nt_common_add_debug("about to run 'stopService' on '$service' ...");
								$adminService->controlCmd($service, 'stopService');
								if (!$adminService->waitCallback())
								{
									nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for stopService');
								}
							}
							nt_sleep(VIEW_DELAY);
						}

						// - prepare next step (timer countdown to a few seconds)

						tool_main_set_restart_sequence_timer($tool_seq_id, 30);

						// - move on to the next step

						$tool_restart_info = tool_main_set_next_restart_sequence_step($tool_seq_id, $tool_seq_step + 1);
						$tpl->assign('tool_restart_info', $tool_restart_info);
					}
					elseif (isset($NELTOOL['POST_VARS']['restart_shutdown_timer']))
					{
						// step 2, waited for the shutdown timer, lets move on

						// - move on to the next step

						$tool_restart_info = tool_main_set_next_restart_sequence_step($tool_seq_id, $tool_seq_step + 1);
						$tpl->assign('tool_restart_info', $tool_restart_info);
					}
					elseif (isset($NELTOOL['POST_VARS']['restart_start_group']))
					{
						// step 4, start a group of services
						$tool_seq_start_id 		= $NELTOOL['POST_VARS']['restart_start_group_id'];
						$tool_seq_start_list	= $NELTOOL['POST_VARS']['restart_start_group_list'];

						if (isset($tool_restart_start_list))
						{
							foreach($tool_restart_start_list as $restart_start_group)
							{
								if ($restart_start_group['restart_group_id'] == $tool_seq_start_id)
								{
									$service_list = explode(',', $tool_seq_start_list);

									if (sizeof($service_list))
									{
										nt_log("Domain '$AS_Name' : 'startService' on ". implode(', ',array_values($service_list)));
										reset($service_list);
										foreach($service_list as $service)
										{
											nt_common_add_debug("about to run 'startService' on '$service' ...");
											$adminService->controlCmd($service, 'startService');
											if (!$adminService->waitCallback())
											{
												nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for startService');
											}
										}

										nt_sleep(VIEW_DELAY);
									}

									break;
								}
							}
						}
					}
					elseif (isset($NELTOOL['POST_VARS']['restart_stop_group']))
					{
						// step 3, stop a group of services
						$tool_seq_stop_id 	= $NELTOOL['POST_VARS']['restart_stop_group_id'];
						$tool_seq_stop_list	= $NELTOOL['POST_VARS']['restart_stop_group_list'];

						if (isset($tool_restart_stop_list))
						{
							$service_list = explode(',', $tool_seq_stop_list);

							if (sizeof($service_list))
							{
								nt_log("Domain '$AS_Name' : 'stopService' on ". implode(', ',array_values($service_list)));
								reset($service_list);
								foreach($service_list as $service)
								{
									nt_common_add_debug("about to run 'stopService' on '$service' ...");
									$adminService->controlCmd($service, 'stopService');
									if (!$adminService->waitCallback())
									{
										nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for stopService');
									}
								}
								nt_sleep(VIEW_DELAY);
							}
						}
					}
					elseif (isset($NELTOOL['POST_VARS']['restart_over']))
					{
						// this makes you go to the next step
						if ($tool_seq_id == $tool_restart_info['restart_sequence_id'])
						{
							// - inform CS that the shard is ready and locked
							tool_main_add_restart_notification($tool_seq_id, $service_su, $service_egs, $restart_shard_id, 4102, $AS_CsSQL, $AS_ShardLang);

							$tool_restart_info = tool_main_set_next_restart_sequence_step($tool_seq_id, $tool_seq_step + 1);
							$tpl->assign('tool_restart_info', $tool_restart_info);
						}
					}
					//elseif (isset($NELTOOL['POST_VARS']['restart_broadcast_open']))
					//{
					//	// step 6, open ws, end restart sequence
                    //
					//	// - open WS
					//	$service 			= $NELTOOL['POST_VARS']['restart_su'];
					//	$restart_shard_id 	= $NELTOOL['POST_VARS']['restart_shard_id'];
                    //
					//	$service_command = 'rsm.setWSState '. $restart_shard_id .' OPEN ""';
					//	nt_common_add_debug("about to run command '$service_command' on '$service' ...");
                    //
					//	$adminService->serviceCmd($service, $service_command);
					//	if (!$adminService->waitCallback())
					//	{
					//		nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
					//	}
                    //
					//	$tpl->clear_assign('tool_execute_result');
					//	nt_sleep(VIEW_DELAY);
                    //
					//	// - end restart sequence
					//	tool_main_set_end_restart_sequence($tool_seq_id);
					//	nt_common_redirect('index.php');
					//	exit();
					//}
				}
				elseif (isset($_POST['shards_update']) && tool_admin_applications_check('tool_main_shard_autostart'))
				{
					$shard_update_mode	= $_POST['shards_update'];
					$shard_update_name	= $_POST['shards_update_name'];

					switch ($shard_update_mode)
					{
						case 'auto restart on':

							$as_command = "setShardStartMode ". $shard_update_name ." on";

							nt_log("Domain '$AS_Name' : $as_command");
							nt_common_add_debug("about to run '$as_command' ...");
							$adminService->globalCmd($as_command);
							if (!$adminService->waitCallback())
							{
								nt_common_add_debug('Error while waiting for callback for \''. $as_command .'\'');
							}

							nt_sleep(VIEW_DELAY);

							break;

						case 'auto restart off':

							$as_command = "setShardStartMode ". $shard_update_name ." off";

							nt_log("Domain '$AS_Name' : $as_command");
							nt_common_add_debug("about to run '$as_command' ...");
							$adminService->globalCmd($as_command);
							if (!$adminService->waitCallback())
							{
								nt_common_add_debug('Error while waiting for callback for \''. $as_command .'\'');
							}

							nt_sleep(VIEW_DELAY);

							break;
					}

				}
				elseif (isset($_POST['ws_update']) && tool_admin_applications_check('tool_main_ws'))
				{
					$shard_ws_su			= $_POST['ws_su'];
					$shard_ws_shard_name	= $_POST['ws_shard_name'];
					$shard_ws_shard_id		= $_POST['ws_shard_id'];

					$shard_ws_state			= $_POST['ws_state_'. $shard_ws_shard_name];
					$shard_ws_motd			= $_POST['ws_motd_'. $shard_ws_shard_name];

					// coders don't know how to write it seems
					// ace: now they know if ($shard_ws_state == 'close') $shard_ws_state = 'closed';

					//nt_common_add_debug("request for ". $shard_ws_su ."/". $shard_ws_shard_name ." to set STATE:". $shard_ws_state ." (". $shard_ws_motd .")");

					$service = $shard_ws_su;
					$service_command = 'rsm.setWSState '. $shard_ws_shard_id .' '. strtoupper($shard_ws_state) .' "'. $shard_ws_motd .'"';
					nt_common_add_debug("about to run command '$service_command' on '$service' ...");

					$adminService->serviceCmd($service, $service_command);
					if (!$adminService->waitCallback())
					{
						nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
					}

					$tpl->clear_assign('tool_execute_result');
					nt_sleep(VIEW_DELAY);
				}
				elseif (isset($_POST['services_update']))
				{
					$service_update_mode	= $_POST['services_update'];

					switch ($service_update_mode)
					{
						case 'open ws': // 2
							if (tool_admin_applications_check('tool_main_ws_old'))
							{
								$service_command = "ShardOpen 2";
								$service_list = tool_main_get_checked_services();

								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : '$service_command' on ". implode(', ',array_values($service_list)));
									nt_common_add_debug(array_combine($service_list, $service_list));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									$tpl->assign('tool_execute_result', '');
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run command '$service_command' on '$service' ...");
										$adminService->serviceCmd($service, $service_command);
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
										}
									}

									$tpl->clear_assign('tool_execute_result');
									nt_sleep(VIEW_DELAY);
								}
							}

							break;

						case 'lock ws': // 1
							if (tool_admin_applications_check('tool_main_ws_old'))
							{
								$service_command = "ShardOpen 1";
								$service_list = tool_main_get_checked_services();

								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : '$service_command' on ". implode(', ',array_values($service_list)));
									nt_common_add_debug(array_combine($service_list, $service_list));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									$tpl->assign('tool_execute_result', '');
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run command '$service_command' on '$service' ...");
										$adminService->serviceCmd($service, $service_command);
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
										}
									}

									$tpl->clear_assign('tool_execute_result');
									nt_sleep(VIEW_DELAY);
								}
							}

							break;

						case 'close ws':
							if (tool_admin_applications_check('tool_main_ws_old'))
							{
								$service_command = "ShardOpen 0";
								$service_list = tool_main_get_checked_services();

								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : '$service_command' on ". implode(', ',array_values($service_list)));
									nt_common_add_debug(array_combine($service_list, $service_list));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									$tpl->assign('tool_execute_result', '');
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run command '$service_command' on '$service' ...");
										$adminService->serviceCmd($service, $service_command);
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
										}
									}

									$tpl->clear_assign('tool_execute_result');
									nt_sleep(VIEW_DELAY);
								}
							}

							break;

						case 'activate':
							if (tool_admin_applications_check('tool_main_service_autostart'))
							{
								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : 'activateService' on ". implode(', ',array_values($service_list)));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run 'activateService' on '$service' ...");
										$adminService->controlCmd($service, 'activateService');
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback for \''. $service .'\'');
										}
									}

									nt_sleep(VIEW_DELAY);
								}

							}

							break;

						case 'deactivate':
							if (tool_admin_applications_check('tool_main_service_autostart'))
							{
								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : 'deactivateService' on ". implode(', ',array_values($service_list)));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run 'deactivateService' on '$service' ...");
										$adminService->controlCmd($service, 'deactivateService');
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback for \''. $service .'\'');
										}
									}

									nt_sleep(VIEW_DELAY);
								}

							}

							break;

						case 'reset counters':
							if (tool_admin_applications_check('tool_main_reset_counters'))
							{
								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : 'resetStartCounter' on ". implode(', ',array_values($service_list)));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run 'resetStartCounter' on '$service' ...");
										$adminService->serviceCmd($service, 'aes.resetStartCounter');
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback for \''. $service .'\'');
										}
									}

									nt_sleep(VIEW_DELAY);
								}
							}

							break;

						case 'start':
							if (tool_admin_applications_check('tool_main_start'))
							{
								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : 'startService' on ". implode(', ',array_values($service_list)));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run 'startService' on '$service' ...");
										$adminService->controlCmd($service, 'startService');
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for startService');
										}
									}
									nt_sleep(VIEW_DELAY);
								}
							}

							break;

						case 'stop':
							if (tool_admin_applications_check('tool_main_stop'))
							{
								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : 'stopService' on ". implode(', ',array_values($service_list)));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run 'stopService' on '$service' ...");
										$adminService->controlCmd($service, 'stopService');
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for stopService');
										}
									}
									nt_sleep(VIEW_DELAY);
								}
							}
							break;

						case 'restart':
							if (tool_admin_applications_check('tool_main_restart'))
							{
								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : 'restartService' on ". implode(', ',array_values($service_list)));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run 'restartService' on '$service' ...");
										$adminService->controlCmd($service, 'restartService');
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for restartService');
										}
									}
									nt_sleep(VIEW_DELAY);
								}
							}
							break;

						case 'kill':
							if (tool_admin_applications_check('tool_main_kill'))
							{
								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : 'killService' on ". implode(', ',array_values($service_list)));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run 'killService' on '$service' ...");
										$adminService->controlCmd($service, 'killService');
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for killService');
										}
									}
									nt_sleep(VIEW_DELAY);
								}
							}
							break;

						case 'abort':
							if (tool_admin_applications_check('tool_main_abort'))
							{
								$service_list = tool_main_get_checked_services();
								if (sizeof($service_list))
								{
									nt_log("Domain '$AS_Name' : 'abortService' on ". implode(', ',array_values($service_list)));
									$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
									reset($service_list);
									foreach($service_list as $service)
									{
										nt_common_add_debug("about to run 'abortService' on '$service' ...");
										$adminService->controlCmd($service, 'abortService');
										if (!$adminService->waitCallback())
										{
											nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for abortService');
										}
									}
									nt_sleep(VIEW_DELAY);
								}
							}
							break;

						case 'execute':
							if (tool_admin_applications_check('tool_main_execute'))
							{
								if (isset($_POST['service_command']))
								{
									$service_command = trim(stripslashes(html_entity_decode($_POST['service_command'], ENT_QUOTES)));
									$service_list = tool_main_get_checked_services();
									if (sizeof($service_list))
									{
										nt_log("Domain '$AS_Name' : '$service_command' on ". implode(', ',array_values($service_list)));
										nt_common_add_debug(array_combine($service_list, $service_list));
										$tpl->assign('tool_service_select_list', array_combine($service_list, $service_list));
										$tpl->assign('tool_execute_result', '');
										reset($service_list);
										foreach($service_list as $service)
										{
											nt_common_add_debug("about to run command '$service_command' on '$service' ...");
											$adminService->serviceCmd($service, $service_command);
											if (!$adminService->waitCallback())
											{
												nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
											}
											else
											{
												$tpl->assign('tool_execute_command', 	htmlentities($service_command, ENT_QUOTES));
											}
										}
									}
								}
							}

							break;
					}
				}
			} // if ($nel_user['has_lock'])

			$status_orders	= $adminService->getShardOrders();
			$status 		= $adminService->getStates();
			nt_common_add_debug($status_orders);
			nt_common_add_debug($status);

			$domainServices = tool_main_parse_status($status);
			$shardRunList	= tool_main_get_shards_from_status($domainServices, $tool_shard_filters);
			$shardRunOrders	= tool_main_get_shards_orders($status_orders);
			nt_common_add_debug($shardRunList);
			nt_common_add_debug($shardRunOrders);

			$shardInfos		= tool_main_get_shards_info_from_db($AS_Application, $domainServices, $tool_shard_filters, $AS_RingSQL);
			nt_common_add_debug("shard infos :");
			nt_common_add_debug($shardInfos);

			$tpl->assign('tool_services_list',	$domainServices);
			$tpl->assign('tool_shard_run_list',	$shardRunList);
			$tpl->assign('tool_shard_orders',	$shardRunOrders);

			$tpl->assign('tool_shard_su_name',	tool_main_get_su_from_status($domainServices));
			$tpl->assign('tool_shard_infos',	$shardInfos);
			$tpl->assign('tool_shard_ws_states',array('close','dev','restricted','open'));

			if (isset($shardInfos[$AS_InternalName]))
			{
				$tpl->assign('tool_restart_ws_state', $shardInfos[$AS_InternalName]['state']);
			}

			if (isset($tool_restart_stop_list) && isset($tool_restart_info) && tool_admin_applications_check('tool_main_easy_restart'))
			{
				// lets get a list of services for each group
				$tool_restart_start_group_list	= tool_main_get_restart_services($AS_InternalName, $domainServices, $tool_restart_start_list);
				$tpl->assign('tool_restart_start_actions',	$tool_restart_start_group_list);

				$tool_restart_stop_group_list	= tool_main_get_all_restart_services($tool_restart_start_group_list);
				$tpl->assign('tool_restart_stop_actions',	$tool_restart_stop_group_list);

				// get the shard id
				$tool_restart_shard_id	= tool_main_get_shardid_from_status($domainServices, $AS_InternalName);
				$tpl->assign('tool_restart_shard_id',		$tool_restart_shard_id);

				// find the shard egs for broadcasts
				$tool_restart_egs_name	= tool_main_get_egs_from_status($domainServices, $AS_InternalName);
				$tpl->assign('tool_restart_egs_name',		$tool_restart_egs_name);
			}

		}

		$tool_hd_list = tool_main_get_hd_data_for_domain($view_domain_id);
		nt_common_add_debug($tool_hd_list);
		$tpl->assign('tool_hd_list',		$tool_hd_list);
		$tpl->assign('tool_hd_time',		tool_main_get_last_hd_time_for_domain($view_domain_id));
	}
	//else
	//{
	//	nt_common_add_debug('invalid host/port!');
	//}

	$tpl->display('index.tpl');

?>