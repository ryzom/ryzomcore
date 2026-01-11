<?php

	require_once('common.php');
	require_once('functions_tool_main.php');
	require_once('functions_tool_log_analyser.php');

	if (!tool_admin_applications_check('tool_las'))	nt_common_redirect('index.php');

	nt_common_add_debug('-- Starting on \'tool_log_analyser.php\'');

	$tpl->assign('tool_title', "Log Analyser");

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

	$tpl->assign('tool_domain_list',		$nel_user['access']['domains']);
	$tpl->assign('tool_domain_selected',	$view_domain_id);

	$tpl->assign('tool_shard_list',			$nel_user['access']['shards']);
	$tpl->assign('tool_shard_selected',		$view_shard_id);

	$tool_shard_filters	= tool_main_get_shard_ids($view_shard_id);
	$tpl->assign('tool_shard_filters',		$tool_shard_filters);

	//$nel_tool_notes_meta  = "<script type=\"text/javascript\" src=\"overlib/overlib_mini.js\" ></script>\n";
	//$nel_tool_notes_meta .= "<script type=\"text/javascript\" src=\"overlib/overlib_anchor_mini.js\" ></script>\n";
	//$nel_tool_notes_meta .= "<script type=\"text/javascript\" src=\"overlib/overlib_draggable_mini.js\" ></script>\n";
	//$tpl->assign('nel_tool_notes_meta',	$nel_tool_notes_meta);

	$template_file	= 'tool_log_analyser.tpl';

	if ($view_domain_id)
	{
		$tool_as_error = null;

		$AS_Name = tool_main_get_domain_name($view_domain_id);
		$AS_Host = tool_main_get_domain_host($view_domain_id);
		$AS_Port = tool_main_get_domain_port($view_domain_id);
		$AS_ShardName	= tool_main_get_shard_name($view_shard_id);

		$tpl->assign('tool_page_title', 'Log Analyser - '. $AS_Name . ($AS_ShardName != '' ? ' / '. $AS_ShardName : ''));

		$tool_as_error = null;

		$AS_LAS_AdminPath	= tool_main_get_domain_data($view_domain_id, 'domain_las_admin_path');
		$AS_LAS_LocalPath	= tool_main_get_domain_data($view_domain_id, 'domain_las_local_path');
		$tool_las_file_list	= tool_las_get_file_list($AS_LAS_AdminPath);

		if (isset($NELTOOL['GET_VARS']['fileview']))
		{
			// FILE VIEWER

			$template_file	= 'tool_log_analyser_file_view.tpl';
			$view_file_name = base64_decode($NELTOOL['GET_VARS']['fileview']);
			$tpl->assign('tool_file_list',	$tool_las_file_list);

			$view_file_data = tool_las_check_for_file($tool_las_file_list, $view_file_name);

			if (isset($NELTOOL['GET_VARS']['downloadraw']))
			{
				if ($fp = fopen($view_file_data['path'] . $view_file_data['name'], 'r'))
				{
					header("Content-type: text/plain");
					header("Content-Disposition: attachment; filename=las_raw_". $view_file_data['name']);
					header("Pragma: no-cache");
					header("Expires: 0");
					fpassthru($fp);
					fclose($fp);
					exit();
				}
			}
			elseif (isset($NELTOOL['GET_VARS']['downloadparsed']))
			{
				$char_eid_data = tool_las_parse_file($view_file_data['path'] . $view_file_data['name']);

				// NOTE: 'ring_live' needs to be replace with the ringdb field from the domain table
				$db_char_data = tool_las_get_character_names('ring_live', $char_eid_data);

				if (sizeof($db_char_data))
				{
					$search_eid_ary		= array();
					$search_char_ary	= array();

					reset($char_eid_data);
					foreach($char_eid_data as $char_id => $char_eid)
					{
						if (isset($db_char_data[$char_id]))
						{
							$search_eid_ary[]	= $char_eid;
							$search_char_ary[]	= $db_char_data[$char_id];
						}
					}

					tool_las_fpassthru_replace($view_file_data['path'],$view_file_data['name'], $search_eid_ary, $search_char_ary);
					exit();
				}
			}
			//elseif (isset($NELTOOL['GET_VARS']['delete']))
			//{
			//	nt_common_add_debug('unlinking file : '. $view_file_data['path'] . $view_file_data['name']);
			//	@unlink($view_file_data['path'] . $view_file_data['name']);
			//	nt_common_redirect('tool_log_analyser.php');
			//	exit();
			//}
			elseif (is_array($view_file_data))
			{
				$tpl->assign('tool_view_file_data', $view_file_data);

				$file_line_start = 0;
				if (isset($NELTOOL['GET_VARS']['viewstart']))
				{
					$file_line_start = $NELTOOL['GET_VARS']['viewstart'];
				}

				$file_line_read_max = 200;

				$view_file_output_data = tool_las_read_file($view_file_data['path'] . $view_file_data['name'], $file_line_read_max, $file_line_start, $file_line_start_previous, $file_line_start_next);
				$tpl->assign('tool_file_output', $view_file_output_data);

				$tpl->assign('tool_view_line_start_previous',	$file_line_start_previous);
				$tpl->assign('tool_view_line_start_next',		$file_line_start_next);


			}
			else
			{
				$tpl->assign('tool_file_error', 'File not found !');
			}
		}
		elseif (($AS_LAS_AdminPath != '') && ($AS_LAS_LocalPath != ''))
		{
			// REGULAR SERVICE VIEW WITH COMMANDS

			$tpl->assign('tool_file_list',	$tool_las_file_list);

			if (substr($AS_LAS_AdminPath,-1) != '/') $AS_LAS_AdminPath .= '/';
			if (substr($AS_LAS_LocalPath,-1) != '/') $AS_LAS_LocalPath .= '/';

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
					if (isset($NELTOOL['POST_VARS']['services_las']))
					{
						$tool_services_las = $NELTOOL['POST_VARS']['services_las'];
						$tpl->assign('tool_post_data',	base64_encode(serialize($NELTOOL['POST_VARS'])));

						$service_search_database	= $NELTOOL['POST_VARS']['service_search_database'];
						$service_search_file_name	= $NELTOOL['POST_VARS']['service_search_file_name'];
						$service_search_start_date	= $NELTOOL['POST_VARS']['service_search_start_date'];
						$service_search_end_date	= $NELTOOL['POST_VARS']['service_search_end_date'];

						$tpl->assign('tool_form_service_search_database',	$service_search_database);
						$tpl->assign('tool_form_service_search_file_name',	$service_search_file_name);
						$tpl->assign('tool_form_service_search_start_date',	$service_search_start_date);
						$tpl->assign('tool_form_service_search_end_date',	$service_search_end_date);

						$file_name_error_msg 	= null;
						$start_date_error_msg	= null;

						switch ($tool_services_las)
						{
							case 'search eids':

								if ($service_search_file_name == '')	$file_name_error_msg	= "Need to specify a filename !";
								if ($service_search_start_date == '')	$start_date_error_msg	= "Need to specify a start date !";

								$tpl->assign('tool_file_name_error_msg',			$file_name_error_msg);
								$tpl->assign('tool_start_date_error_msg',			$start_date_error_msg);

								if (isset($NELTOOL['POST_VARS']['service_eids']) && !$file_name_error_msg && !$start_date_error_msg)
								{
									$service_eids = trim(stripslashes($NELTOOL['POST_VARS']['service_eids']));
									$tpl->assign('tool_form_service_eids', $service_eids);

									$service_eids_ary = tool_las_parse_eids_to_array($service_eids);
									nt_common_add_debug($service_eids_ary);

									if (sizeof($service_eids_ary) > 0)
									{
										$service_command = 'executeToFile '. $AS_LAS_LocalPath . $service_search_file_name ;

										if (sizeof($service_eids_ary) == 1)	$service_command .= ' searchEId ';
										else								$service_command .= ' searchEIds ';

										$service_command .= $service_search_database .' ';
										$service_command .= implode(' ', $service_eids_ary) .' ';

										if (sizeof($service_eids_ary) > 1)	$service_command .= '- ';
										$service_command .= $service_search_start_date;

										if ($service_search_end_date != '')	$service_command .= ' '. $service_search_end_date;

										nt_common_add_debug($service_command);

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
										}

									}
								}

								break;

							case 'search text':

								if ($service_search_file_name == '')	$file_name_error_msg	= "Need to specify a filename !";
								if ($service_search_start_date == '')	$start_date_error_msg	= "Need to specify a start date !";

								$tpl->assign('tool_file_name_error_msg',			$file_name_error_msg);
								$tpl->assign('tool_start_date_error_msg',			$start_date_error_msg);

								if (isset($NELTOOL['POST_VARS']['service_text']) && !$file_name_error_msg && !$start_date_error_msg)
								{
									$service_text = trim(stripslashes(html_entity_decode($NELTOOL['POST_VARS']['service_text'], ENT_QUOTES)));
									$tpl->assign('tool_form_service_text', htmlentities($service_text,ENT_QUOTES));

									if ($service_text != '')
									{
										$service_command = 'executeToFile '. $AS_LAS_LocalPath . $service_search_file_name ;
										$service_command .= ' searchString '. $service_search_database .' "'. addslashes($service_text) .'" ';
										$service_command .= $service_search_start_date;
										if ($service_search_end_date != '')	$service_command .= ' '. $service_search_end_date;

										nt_common_add_debug($service_command);

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
										}

									}
								}

								break;

							case 'execute':

								if (isset($NELTOOL['POST_VARS']['service_command']))
								{
									$service_command = trim(stripslashes(html_entity_decode($NELTOOL['POST_VARS']['service_command'], ENT_QUOTES)));
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

								break;

						}
					}

					$status = $adminService->getStates();
					nt_common_add_debug($status);

					$domainServices		= tool_main_parse_status($status);

					$filteredServices	= array();
					reset($domainServices);
					foreach($domainServices as $aKey => $aService)
					{
						// we are only interested in EGS
						if ($aService['ShortName'] == 'LAS')
						{
							$filteredServices[] = $aService;
						}
					}
					$tpl->assign('tool_services_list',	$filteredServices);
				}
			}
		}
	}

	$tpl->display($template_file);

?>