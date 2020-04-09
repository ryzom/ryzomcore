<?php


// SoniX: yop
// SoniX: pour récup les info de view, il faut utiliser la commande getView
// SoniX: par ex, sur un AIS :  "getView (0x0000000001:15:83:83).NamedEntityState" récupère le state de l'entité specifier
// SoniX: "getView *.NamedEntityName" récupère toutes les entité nommé de l'IA sur laquelle tu balance la commande
// SoniX: et pour setter une valuer :
// SoniX: "getView (0x0000000001:15:83:83).NamedEntityState=1" met le truc à 1
// SoniX: En gros, tu récupères les info dans la table variables, et tu vire les 3 premier morceaux (par ex, *.*.AIS.*.NamedEntityName deviens *.NamedEntityName).
// SoniX: Par contre, c'est a toi de faire le dispatch sur chaque AIS si besoin.
// YoGiN: hum, fun fun fun :D
// YoGiN: oki merci beaucoup, je vais voir ca :)
// SoniX: j'ai tester sur linuxshard8, d'jon mark a réactivé un morceau d'époside 2 dessus avec 1 variable
// YoGiN: d'accord

	require_once('common.php');
	require_once('functions_tool_main.php');
	require_once('functions_tool_event_entities.php');

	if (!tool_admin_applications_check('tool_event_entities'))	nt_common_redirect('index.php');

	nt_common_add_debug('-- Starting on \'tool_event_entities.php\'');

	$tpl->assign('tool_title', "Event Entities");

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

	$tpl->assign('tool_domain_list',		$nel_user['access']['domains']);
	$tpl->assign('tool_domain_selected',	$view_domain_id);

	$tpl->assign('tool_shard_list',			$nel_user['access']['shards']);
	$tpl->assign('tool_shard_selected',		$view_shard_id);

	$tool_shard_filters	= tool_main_get_shard_ids($view_shard_id);
	$tpl->assign('tool_shard_filters',		$tool_shard_filters);

	//if (tool_admin_applications_check('tool_player_locator_display_players'))		$tpl->assign('restriction_tool_player_locator_display_players',	true);
	//if (tool_admin_applications_check('tool_player_locator_locate'))				$tpl->assign('restriction_tool_player_locator_locate',			true);

	if ($view_domain_id)
	{
		$tool_as_error = null;

		$AS_Name = tool_main_get_domain_name($view_domain_id);
		$AS_Host = tool_main_get_domain_host($view_domain_id);
		$AS_Port = tool_main_get_domain_port($view_domain_id);
		$AS_ShardName	= tool_main_get_shard_name($view_shard_id);

		$tpl->assign('tool_page_title', 'Event Entities - '. $AS_Name . ($AS_ShardName != '' ? ' / '. $AS_ShardName : ''));

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
				$tool_services_ee = null;
				if		(isset($NELTOOL['POST_VARS']['services_ee']))	$tool_services_ee = $NELTOOL['POST_VARS']['services_ee'];
				elseif	(isset($NELTOOL['GET_VARS']['services_ee']))	$tool_services_ee = $NELTOOL['GET_VARS']['services_ee'];

				if ($tool_services_ee)
				{
					$tpl->assign('tool_post_data',	base64_encode(serialize($NELTOOL['POST_VARS'])));

					switch ($tool_services_ee)
					{
						case 'update entities':

							$requested_service_list	= $NELTOOL['POST_VARS']['requested_service_list'];
							$service_list = unserialize(base64_decode($requested_service_list));

							//nt_common_add_debug($NELTOOL['POST_VARS']);
							$update_entities = tool_ee_get_entities($NELTOOL['POST_VARS']);
							nt_common_add_debug('update_entities');
							nt_common_add_debug($update_entities);

							reset($update_entities);
							foreach($update_entities as $entity_data)
							{
								$service_command = '';
								$_commands = array();

								if ($entity_data['entity_state']	!= $entity_data['source_entity_state'])		$_commands[] = 'NamedEntityState='. $entity_data['entity_state'];
								if ($entity_data['entity_param1']	!= $entity_data['source_entity_param1'])	$_commands[] = 'NamedEntityParam1='. $entity_data['entity_param1'];
								if ($entity_data['entity_param2']	!= $entity_data['source_entity_param2'])	$_commands[] = 'NamedEntityParam2='. $entity_data['entity_param2'];

								if (sizeof($_commands) > 0)
								{
									nt_common_add_debug("something has been updated in entity : ". $entity_data['source_entity']);
									if (sizeof($_commands) == 1)
									{
										$service_command = 'getView '. $entity_data['source_entity'] .'.'. $_commands[0];
									}
									else
									{
										$service_command = 'getView '. $entity_data['source_entity'] .'.['. implode(',', $_commands) .']';
									}

									$service = strtolower($entity_data['source_service']);

									nt_log("Domain '$AS_Name' : '$service_command' on ". $service);

									$adminService->serviceCmd($service, $service_command);
									if (!$adminService->waitCallback())
									{
										nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
									}
								}
							}

							//$requested_service		= $NELTOOL['POST_VARS']['source_service'];
							//$requested_entity			= $NELTOOL['POST_VARS']['source_entity'];
							//$requested_entity_name	= $NELTOOL['POST_VARS']['source_entity_name'];
                            //
							//$new_entity_state 	= $NELTOOL['POST_VARS']['entity_state'];
							//$new_entity_param1	= $NELTOOL['POST_VARS']['entity_param1'];
							//$new_entity_param2	= $NELTOOL['POST_VARS']['entity_param2'];
                            //
							//$old_entity_state	= $NELTOOL['POST_VARS']['source_entity_state'];
							//$old_entity_param1	= $NELTOOL['POST_VARS']['source_entity_param1'];
							//$old_entity_param2	= $NELTOOL['POST_VARS']['source_entity_param2'];
                            //
							//$service_command = '';
                            //
							//$_commands = array();
                            //
							//if ($new_entity_state		!= $old_entity_state)	$_commands[] = 'NamedEntityState='. $new_entity_state;
							//if ($new_entity_param1	!= $old_entity_param1)	$_commands[] = 'NamedEntityParam1='. $new_entity_param1;
							//if ($new_entity_param2	!= $old_entity_param2)	$_commands[] = 'NamedEntityParam2='. $new_entity_param2;
                            //
							//if (sizeof($_commands) > 0)
							//{
							//	nt_common_add_debug("something has been updated in entity : ". $requested_entity);
							//	if (sizeof($_commands) == 1)
							//	{
							//		$service_command = 'getView '. $requested_entity .'.'. $_commands[0];
							//	}
							//	else
							//	{
							//		$service_command = 'getView '. $requested_entity .'.['. implode(',', $_commands) .']';
							//	}
                            //
							//	$service = strtolower($requested_service);
                            //
							//	nt_log("Domain '$AS_Name' : '$service_command' on ". $service);
                            //
							//	$adminService->serviceCmd($service, $service_command);
							//	if (!$adminService->waitCallback())
							//	{
							//		nt_common_add_debug('Error while waiting for callback on service \''. $service .'\' for command : '. $service_command);
							//	}
                            //
							//}

							//break;

						case 'display entities':

							if (!isset($service_list)) $service_list = tool_main_get_checked_services();

							if (sizeof($service_list))
							{
								$service_command = 'getView *.[NamedEntityName,NamedEntityState,NamedEntityParam1,NamedEntityParam2]';

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
									$entity_data = tool_ee_parse_getview($command_return_data);
									nt_common_add_debug($entity_data);
									$tpl->assign('tool_entity_data',	$entity_data);
									$tpl->assign('requested_service_list',	base64_encode(serialize($service_list)));
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
					if ($aService['ShortName'] == 'AIS')
					{
						$filteredServices[] = $aService;
					}
				}

				$tpl->assign('tool_services_list',	$filteredServices);
			}
		}
	}

	$tpl->display('tool_event_entities.tpl');

?>