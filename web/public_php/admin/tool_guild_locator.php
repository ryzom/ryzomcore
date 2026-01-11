<?php

	require_once('common.php');
	require_once('functions_tool_main.php');
	require_once('functions_tool_guild_locator.php');

	if (!tool_admin_applications_check('tool_guild_locator'))	nt_common_redirect('index.php');

	nt_common_add_debug('-- Starting on \'tool_guild_locator.php\'');

	$tpl->assign('tool_title', "Guild Locator");

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

	if (tool_admin_applications_check('tool_guild_locator_manage_guild'))	$tpl->assign('restriction_tool_guild_locator_manage_guild',true);
	if (tool_admin_applications_check('tool_guild_locator_manage_members'))	$tpl->assign('restriction_tool_guild_locator_manage_members',true);
	if (tool_admin_applications_check('tool_guild_locator_manage_forums'))	$tpl->assign('restriction_tool_guild_locator_manage_forums',true);

	if ($view_domain_id)
	{
		$tool_as_error = null;

		$AS_Name = tool_main_get_domain_name($view_domain_id);
		$AS_Host = tool_main_get_domain_host($view_domain_id);
		$AS_Port = tool_main_get_domain_port($view_domain_id);
		$AS_ShardName	= tool_main_get_shard_name($view_shard_id);
		$MFS_Web		= tool_main_get_domain_data($view_domain_id, 'domain_mfs_web');

		$tpl->assign('tool_page_title', 'Guild Locator - '. $AS_Name . ($AS_ShardName != '' ? ' / '. $AS_ShardName : ''));

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
				$tool_services_gl = null;
				if		(isset($NELTOOL['POST_VARS']['services_gl']))	$tool_services_gl = $NELTOOL['POST_VARS']['services_gl'];
				elseif	(isset($NELTOOL['GET_VARS']['services_gl']))	$tool_services_gl = $NELTOOL['GET_VARS']['services_gl'];

				if ($tool_services_gl)
				{
					$tpl->assign('tool_post_data',	base64_encode(serialize($NELTOOL['POST_VARS'])));

					switch ($tool_services_gl)
					{
						case 'display guilds':

							$service_list = tool_main_get_checked_services();
							if (sizeof($service_list))
							{
								$service_command = 'dumpGuildList local';

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
									$guild_data = tool_gl_parse_dump_guild_list($command_return_data);
									$tpl->assign('tool_guild_data',	$guild_data);
								}
							}

							break;

						case 'update name':

							if (($tool_services_gl == 'update name') && tool_admin_applications_check('tool_guild_locator_manage_guild'))
							{
								$service		= $NELTOOL['POST_VARS']['servicealias'];
								$guild_shard_id = $NELTOOL['POST_VARS']['guildshardid'];
								$guild_id		= $NELTOOL['POST_VARS']['guildid'];
								$new_guild_name = $NELTOOL['POST_VARS']['new_guild_name'];

								$new_guild_name = trim($new_guild_name);
								if (ereg("^[a-zA-Z0-9\ ]+$", $new_guild_name))
								{
									// this is a small hack that was done by daniel so i could use the renameGuild command without an EID
									$service_command = 'renameGuild admin_tool '. $guild_shard_id .':'. $guild_id .' "'. $new_guild_name .'"';

									nt_log("Domain '$AS_Name' : '$service_command' on ". $service);

									$tpl->assign('tool_execute_result', '');
									$command_return_data = array();

									$adminService->serviceCmd($service, $service_command);
									if (!$adminService->waitCallback())
									{
										nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
									}
									else
									{
										// tool_guild_errors
									}
								}
								else
								{
									$tpl->assign('tool_guild_errors',	array('New name contains illegal characters.'));
								}

								$NELTOOL['GET_VARS']['servicealias']	= $service;
								$NELTOOL['GET_VARS']['guildshardid']	= $guild_shard_id;
								$NELTOOL['GET_VARS']['guildid']			= $guild_id;
							}

						case 'update description':

							if (($tool_services_gl == 'update description') && tool_admin_applications_check('tool_guild_locator_manage_guild'))
							{
								$service		= $NELTOOL['POST_VARS']['servicealias'];
								$guild_shard_id = $NELTOOL['POST_VARS']['guildshardid'];
								$guild_id		= $NELTOOL['POST_VARS']['guildid'];
								$new_guild_desc	= $NELTOOL['POST_VARS']['new_guild_description'];

								$new_guild_desc = trim($new_guild_desc);
								if (ereg("^[a-zA-Z0-9\ ]+$", $new_guild_desc))
								{
									$service_command = 'setGuildDescription '. $guild_shard_id .':'. $guild_id .' "'. $new_guild_desc .'"';

									nt_log("Domain '$AS_Name' : '$service_command' on ". $service);

									$tpl->assign('tool_execute_result', '');
									$command_return_data = array();

									$adminService->serviceCmd($service, $service_command);
									if (!$adminService->waitCallback())
									{
										nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
									}
									else
									{
										// tool_guild_errors
									}

								}
								else
								{
									$tpl->assign('tool_guild_errors',	array('New description contains illegal characters.'));
								}

								$NELTOOL['GET_VARS']['servicealias']	= $service;
								$NELTOOL['GET_VARS']['guildshardid']	= $guild_shard_id;
								$NELTOOL['GET_VARS']['guildid']			= $guild_id;
							}

						case 'setleader':

							if (($tool_services_gl == 'setleader') && tool_admin_applications_check('tool_guild_locator_manage_members'))
							{
								$service		= $NELTOOL['GET_VARS']['servicealias'];
								$guild_shard_id = $NELTOOL['GET_VARS']['guildshardid'];
								$guild_id		= $NELTOOL['GET_VARS']['guildid'];
								$member_eid		= $NELTOOL['GET_VARS']['eid'];

								// guildSetLeader <guildName|<shardId>:<guildId> <member eid>

								$service_command = 'guildSetLeader '. $guild_shard_id .':'. $guild_id .' '. $member_eid;

								nt_log("Domain '$AS_Name' : '$service_command' on ". $service);

								$tpl->assign('tool_execute_result', '');
								$command_return_data = array();

								$adminService->serviceCmd($service, $service_command);
								if (!$adminService->waitCallback())
								{
									nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
								}
								else
								{
									// the locator displays a nice output, no need for the raw one
									//$tpl->assign('tool_execute_command', 	$service_command);
									$tpl->assign('tool_guild_errors', tool_gl_parse_grade_change($command_return_data));
								}

							}


						case 'promote':

							if (($tool_services_gl == 'promote') && tool_admin_applications_check('tool_guild_locator_manage_members'))
							{
								$service		= $NELTOOL['GET_VARS']['servicealias'];
								$guild_shard_id = $NELTOOL['GET_VARS']['guildshardid'];
								$guild_id		= $NELTOOL['GET_VARS']['guildid'];
								$member_eid		= $NELTOOL['GET_VARS']['eid'];
								$member_grade	= $NELTOOL['GET_VARS']['grade'];

								$new_grade		= 'Member';
								if		($member_grade == 'Officer')		$new_grade = 'Officer';
								elseif	($member_grade == 'HighOfficer')	$new_grade = 'HighOfficer';

								// guildSetGrade <guildName|<shardId>:<guildId> <member eid> <grade = Member/Officer/HighOfficer/Leader>

								$service_command = 'guildSetGrade '. $guild_shard_id .':'. $guild_id .' '. $member_eid .' '. $new_grade;

								nt_log("Domain '$AS_Name' : '$service_command' on ". $service);

								$tpl->assign('tool_execute_result', '');
								$command_return_data = array();

								$adminService->serviceCmd($service, $service_command);
								if (!$adminService->waitCallback())
								{
									nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
								}
								else
								{
									// the locator displays a nice output, no need for the raw one
									//$tpl->assign('tool_execute_command', 	$service_command);
									$tpl->assign('tool_guild_errors', tool_gl_parse_grade_change($command_return_data));
								}


							}

						case 'demote':

							if (($tool_services_gl == 'demote') && tool_admin_applications_check('tool_guild_locator_manage_members'))
							{
								$service		= $NELTOOL['GET_VARS']['servicealias'];
								$guild_shard_id = $NELTOOL['GET_VARS']['guildshardid'];
								$guild_id		= $NELTOOL['GET_VARS']['guildid'];
								$member_eid		= $NELTOOL['GET_VARS']['eid'];
								$member_grade	= $NELTOOL['GET_VARS']['grade'];

								$new_grade		= 'Member';
								if 		($member_grade == 'Officer')	$new_grade = 'Officer';
								elseif	($member_grade == 'Member')		$new_grade = 'Member';

								// guildSetGrade <guildName|<shardId>:<guildId> <member eid> <grade = Member/Officer/HighOfficer/Leader>

								$service_command = 'guildSetGrade '. $guild_shard_id .':'. $guild_id .' '. $member_eid .' '. $new_grade;

								nt_log("Domain '$AS_Name' : '$service_command' on ". $service);

								$tpl->assign('tool_execute_result', '');
								$command_return_data = array();

								$adminService->serviceCmd($service, $service_command);
								if (!$adminService->waitCallback())
								{
									nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
								}
								else
								{
									// the locator displays a nice output, no need for the raw one
									//$tpl->assign('tool_execute_command', 	$service_command);
									$tpl->assign('tool_guild_errors', tool_gl_parse_grade_change($command_return_data));
								}

							}

						case 'dumpguild':

							$service		= $NELTOOL['GET_VARS']['servicealias'];
							$guild_shard_id	= $NELTOOL['GET_VARS']['guildshardid'];
							$guild_id		= $NELTOOL['GET_VARS']['guildid'];

							if (($guild_shard_id > 0) && ($guild_id > 0) && ($service != ''))
							{
								$service_command = 'dumpGuild '. $guild_shard_id .':'. $guild_id;

								nt_log("Domain '$AS_Name' : '$service_command' on ". $service);

								$tpl->assign('tool_execute_result', '');
								$command_return_data = array();

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

								if (sizeof($command_return_data))
								{
									$tool_sub_services_gl = null;
									if		(isset($NELTOOL['POST_VARS']['subservices_gl']))	$tool_sub_services_gl = $NELTOOL['POST_VARS']['subservices_gl'];
									elseif	(isset($NELTOOL['GET_VARS']['subservices_gl']))		$tool_sub_services_gl = $NELTOOL['GET_VARS']['subservices_gl'];

									$guild_dump_data = tool_gl_parse_dump_guild($command_return_data);
									$tpl->assign('tool_guild_dump_data',	$guild_dump_data);
									$tpl->assign('tool_service',			$service);

									// view ingame guild forums
									if (tool_admin_applications_check('tool_guild_locator_manage_forums'))
									{
										if ($tool_sub_services_gl)
										{
											switch ($tool_sub_services_gl)
											{
												case 'viewthread':

													$view_forum_threadid		= $NELTOOL['GET_VARS']['threadid'];
													$view_forum_recoverable		= $NELTOOL['GET_VARS']['recoverable'];

													$thread_name = ($view_forum_recoverable == 1 ? '_':'') .'thread_'. $view_forum_threadid .'.index';

													$view_thread_data_raw 		= tool_gl_view_forum($MFS_Web, $guild_shard_id, $guild_dump_data['guild_name'], $thread_name);
													$view_thread_data			= tool_gl_parse_thread_view($view_thread_data_raw);
													$tpl->assign('tool_guild_thread',	$view_thread_data);

													break;

												case 'recoverthread':

													$recover_forum_threadid		= $NELTOOL['GET_VARS']['threadid'];

													$thread_name = '_thread_'. $recover_forum_threadid .'.index';
													tool_gl_view_forum($MFS_Web, $guild_shard_id, $guild_dump_data['guild_name'], $recover_forum_threadid, true);

													break;
											}
										}

										$view_forum_data_raw 	= tool_gl_view_forum($MFS_Web, $guild_shard_id, $guild_dump_data['guild_name']);
										$view_forum_data		= tool_gl_parse_forum_view($view_forum_data_raw);

										if (is_array($view_forum_data))	$tpl->assign('tool_guild_forums', 		$view_forum_data);
										else							$tpl->assign('tool_guild_forums_error', $view_forum_data);
									}
								}

							}

							break;

					}
				}

				if (isset($NELTOOL['GET_VARS']['eid']))
				{
					$locate_eid = $NELTOOL['GET_VARS']['eid'];

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
			}
		}
	}

	$tpl->display('tool_guild_locator.tpl');

?>