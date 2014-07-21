<?php

	require_once('common.php');
	require_once('functions_tool_administration.php');

	//if (!tool_admin_applications_check('tool_admin')) nt_common_redirect('index.php');

	nt_common_add_debug('-- Starting on \'tool_administration.php\'');

	if (!isset($NELTOOL['GET_VARS']['toolmode']))	$NELTOOL['GET_VARS']['toolmode'] = 'help';
	$tool_menu_item = tool_admin_menu_get_item_from_key($NELTOOL['GET_VARS']['toolmode']);

	$IE_CHECK = strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE');

	$tpl->assign('tool_title',		'Administration&nbsp;/&nbsp;'. $tool_menu_item['title']);
	$tpl->assign('tool_menu',		tool_admin_menu_get_list($IE_CHECK)); //$tool_admin_menu); // defined in 'functions_tool_administration.php'

	switch($NELTOOL['GET_VARS']['toolmode'])
	{
		case 'help':
			/*
			 * ###################################################################################################
			 *  Help Admin
			 * ###################################################################################################
			 */

			if ($IE_CHECK)	$tpl->assign('ie_check', true);
			else			$tpl->assign('ie_check', false);

			break;

		case 'logs':
			/*
			 * ###################################################################################################
			 *  Logs Admin
			 * ###################################################################################################
			 */

			if (!tool_admin_applications_check('tool_admin_logs'))	nt_common_redirect('index.php');

			$log_start	= 0;
			$log_step 	= 30;
			$num_logs	= tool_admin_logs_get_count();

			if (isset($_GET['page']))	$log_start = $_GET['page'];

			$tool_log_list	= tool_admin_logs_get_list($log_start * $log_step, $log_step);

			$log_page_first		= 0;
			$log_page_last		= ceil($num_logs / $log_step);

			$log_page_previous	= $log_start - 1;
			$log_page_next		= $log_start + 1;

			if ($log_page_previous < 0)				$log_page_previous = 0;
			if ($log_page_next >= $log_page_last)	$log_page_next = $log_page_last - 1;

			$tpl->assign('tool_log_page_first',		$log_page_first);
			$tpl->assign('tool_log_page_last',		$log_page_last - 1);
			$tpl->assign('tool_log_page_previous',	$log_page_previous);
			$tpl->assign('tool_log_page_next',		$log_page_next);

			$tpl->assign('tool_log_page_current',	$log_start + 1);
			$tpl->assign('tool_log_page_total',		$log_page_last);
			$tpl->assign('tool_log_list',			$tool_log_list);

			break;

		case 'users':
			/*
			 * ###################################################################################################
			 *  User Admin
			 * ###################################################################################################
			 */

			if (!tool_admin_applications_check('tool_admin_user'))	nt_common_redirect('index.php');

			$tool_action = null;
			if (isset($_POST['toolaction']))		$tool_action = $_POST['toolaction'];
			elseif (isset($_GET['toolaction']))	$tool_action = $_GET['toolaction'];

			switch ($tool_action)
			{
				case 'update applications':

					if ($tool_action == 'update applications')
					{
						$tool_user_update_id			= $_POST['tool_form_user_id'];
						$tool_user_update_appl_ids		= $_POST['tool_form_application_ids'];

						tool_admin_users_applications_update($tool_user_update_id, $tool_user_update_appl_ids);

						$_GET['user_id'] = $tool_user_update_id;
					}

					// break;

				case 'update domains':

					if ($tool_action == 'update domains')
					{
						$tool_user_update_id			= $_POST['tool_form_user_id'];
						$tool_user_update_domain_ids	= $_POST['tool_form_domain_ids'];

						$tool_user_data 				= tool_admin_users_get_id($tool_user_update_id);
						$tool_user_group_id				= $tool_user_data['user_group_id'];

						tool_admin_users_domains_update($tool_user_update_id, $tool_user_group_id, $tool_user_update_domain_ids);

						$_GET['user_id']		= $tool_user_update_id;
					}

					//break;

				case 'update shards':

					if ($tool_action == 'update shards')
					{
						$tool_user_update_id			= $_POST['tool_form_user_id'];
						$tool_user_update_shard_ids		= $_POST['tool_form_shard_ids'];

						$tool_user_data 				= tool_admin_users_get_id($tool_user_update_id);
						$tool_user_group_id				= $tool_user_data['user_group_id'];

						tool_admin_users_shards_update($tool_user_update_id, $tool_user_group_id, $tool_user_update_shard_ids);

						$_GET['user_id']		= $tool_user_update_id;
					}

					//break;

				case 'update':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update an existing User
					 * -------------------------------------------------------------------------------------------
					 */

					if ($tool_action == 'update')
					{
						$tool_user_update_id		= $_POST['tool_form_user_id'];
						$tool_user_update_name		= $_POST['tool_form_user_name'];
						$tool_user_update_password	= $_POST['tool_form_user_password'];
						$tool_user_update_group		= $_POST['tool_form_user_group'];
						$tool_user_update_active	= $_POST['tool_form_user_active'];

						$tool_error = tool_admin_users_update($tool_user_update_id, $tool_user_update_name, $tool_user_update_password, $tool_user_update_group, $tool_user_update_active);
						if ($tool_error != "")
						{
							$tpl->assign('tool_alert_message',	$tool_error);
						}

						$_GET['user_id'] = $tool_user_update_id;
					}

					//break;

				case 'edit':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Edit an existing User
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_user_edit_id		= $_GET['user_id'];
					$tool_user_edit_data 	= tool_admin_users_get_id($tool_user_edit_id);
					$tool_user_group_id		= $tool_user_edit_data['user_group_id'];

					$tpl->assign('tool_user_edit_data',		$tool_user_edit_data);

					$tool_domain_list		= tool_admin_domains_get_list();
					$tool_user_domain_list	= tool_admin_users_domains_get_list($tool_user_edit_id, true);
					$tool_group_domain_list	= tool_admin_groups_domains_get_list($tool_user_group_id, true);
					$tool_domain_list		= tool_admin_users_domains_merge($tool_domain_list, $tool_user_domain_list, $tool_group_domain_list);

					$tpl->assign('tool_domain_list',		$tool_domain_list);

					$tool_shard_list		= tool_admin_shards_get_list();
					$tool_user_shard_list	= tool_admin_users_shards_get_list($tool_user_edit_id, true);
					$tool_group_shard_list	= tool_admin_groups_shards_get_list($tool_user_group_id, true);
					$tool_shard_list		= tool_admin_users_shards_merge($tool_domain_list, $tool_shard_list, $tool_user_shard_list, $tool_group_shard_list);

					$tpl->assign('tool_shard_list',			$tool_shard_list);

					$tool_appl_list			= tool_admin_applications_get_list();
					$tool_user_appl_list	= tool_admin_users_applications_get_list($tool_user_edit_id, true);
					$tool_group_appl_list	= tool_admin_groups_applications_get_list($tool_user_group_id, true);
					$tool_appl_list			= tool_admin_users_applications_merge($tool_appl_list, $tool_user_appl_list, $tool_group_appl_list);

					$tpl->assign('tool_application_list',	$tool_appl_list);


					break;

				case 'delete':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Delete an existing User
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_user_delete_id		= $_POST['tool_form_user_id'];
					if (!($tool_user_delete_id > 0))
					{
						$tpl->assign('tool_alert_message',	"/!\ Error: invalid user!");
					}
					elseif ($tool_user_delete_id == $nel_user['user_id'])
					{
						$tpl->assign('tool_alert_message',	"/!\ Error: did you just try to delete yourself ?!");
					}
					else
					{
						tool_admin_users_del($tool_user_delete_id);
					}

					break;

				case 'create':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Create a new User
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_user_create_name		= $_POST['tool_form_user_name'];
					$tool_user_create_password	= $_POST['tool_form_user_password'];
					$tool_user_create_group		= $_POST['tool_form_user_group'];
					$tool_user_create_active	= $_POST['tool_form_user_active'];

					$tool_error = tool_admin_users_add($tool_user_create_name, $tool_user_create_password, $tool_user_create_group, $tool_user_create_active);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					break;

			}

			$tool_group_list	= tool_admin_groups_get_list();
			$tool_user_list		= tool_admin_users_get_list($tool_group_list);

			$tpl->assign('tool_user_list',	$tool_user_list);
			$tpl->assign('tool_group_list',	$tool_group_list);

			break;

		case 'groups':
			/*
			 * ###################################################################################################
			 *  Group Admin
			 * ###################################################################################################
			 */

			if (!tool_admin_applications_check('tool_admin_group'))	nt_common_redirect('index.php');

			$tool_action = null;
			if (isset($_POST['toolaction']))		$tool_action = $_POST['toolaction'];
			elseif (isset($_GET['toolaction']))	$tool_action = $_GET['toolaction'];

			switch ($tool_action)
			{
				case 'update applications':

					if ($tool_action == 'update applications')
					{
						$tool_group_update_id			= $_POST['tool_form_group_id'];
						$tool_group_update_appl_ids		= $_POST['tool_form_application_ids'];

						tool_admin_groups_applications_update($tool_group_update_id, $tool_group_update_appl_ids);

						$_GET['group_id'] = $tool_group_update_id;
					}

					// break;

				case 'update domains':

					if ($tool_action == 'update domains')
					{
						$tool_group_update_id			= $_POST['tool_form_group_id'];
						$tool_group_update_domain_ids	= $_POST['tool_form_domain_ids'];

						tool_admin_groups_domains_update($tool_group_update_id, $tool_group_update_domain_ids);

						$_GET['group_id'] = $tool_group_update_id;
					}

					//break;

				case 'update shards':

					if ($tool_action == 'update shards')
					{
						$tool_group_update_id			= $_POST['tool_form_group_id'];
						$tool_group_update_shard_ids	= $_POST['tool_form_shard_ids'];

						tool_admin_groups_shards_update($tool_group_update_id, $tool_group_update_shard_ids);

						$_GET['group_id']		= $tool_group_update_id;
					}

					//break;

				case 'update':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update an existing Group
					 * -------------------------------------------------------------------------------------------
					 */

					if ($tool_action == 'update')
					{
						$tool_group_update_id		= $_POST['tool_form_group_id'];
						$tool_group_update_name		= $_POST['tool_form_group_name'];
						$tool_group_update_level	= $_POST['tool_form_group_level'];
						$tool_group_update_default	= $_POST['tool_form_group_default'];
						$tool_group_update_active	= $_POST['tool_form_group_active'];

						$tool_error = tool_admin_groups_update($tool_group_update_id, $tool_group_update_name, $tool_group_update_level, $tool_group_update_default, $tool_group_update_active);
						if ($tool_error != "")
						{
							$tpl->assign('tool_alert_message',	$tool_error);
						}

						$_GET['group_id'] = $tool_group_update_id;
					}

					//break;

				case 'update default domain':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update group default domain
					 * -------------------------------------------------------------------------------------------
					 */

					if ($tool_action == 'update default domain')
					{
						$tool_group_update_id		= $_POST['tool_form_group_id'];
						$tool_group_default_domain	= $_POST['tool_form_domain_default'];

						$tool_error = tool_admin_groups_update_default_domain($tool_group_update_id, $tool_group_default_domain);
						if ($tool_error != "")
						{
							$tpl->assign('tool_alert_message',	$tool_error);
						}

						$_GET['group_id'] = $tool_group_update_id;
					}

					//break;

				case 'update default shard':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update group default shard
					 * -------------------------------------------------------------------------------------------
					 */

					if ($tool_action == 'update default shard')
					{
						$tool_group_update_id		= $_POST['tool_form_group_id'];
						$tool_group_default_shard	= $_POST['tool_form_shard_default'];

						$tool_error = tool_admin_groups_update_default_shard($tool_group_update_id, $tool_group_default_shard);
						if ($tool_error != "")
						{
							$tpl->assign('tool_alert_message',	$tool_error);
						}

						$_GET['group_id'] = $tool_group_update_id;
					}

					//break;

				case 'update default application':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update group default application
					 * -------------------------------------------------------------------------------------------
					 */

					if ($tool_action == 'update default application')
					{
						$tool_group_update_id			= $_POST['tool_form_group_id'];
						$tool_group_default_application	= $_POST['tool_form_application_default'];

						$tool_error = tool_admin_groups_update_default_application($tool_group_update_id, $tool_group_default_application);
						if ($tool_error != "")
						{
							$tpl->assign('tool_alert_message',	$tool_error);
						}

						$_GET['group_id'] = $tool_group_update_id;
					}

					//break;

				case 'edit':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Edit an existing Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_group_edit_id		= $_GET['group_id'];
					$tool_group_edit_data 	= tool_admin_groups_get_id($tool_group_edit_id);
					$tpl->assign('tool_group_edit_data',	$tool_group_edit_data);

					$tool_domain_list		= tool_admin_domains_get_list();
					$tool_group_domain_list	= tool_admin_groups_domains_get_list($tool_group_edit_id, true);
					$tool_domain_list		= tool_admin_groups_domains_merge($tool_domain_list, $tool_group_domain_list);

					$tpl->assign('tool_domain_list',		$tool_domain_list);

					$tool_shard_list		= tool_admin_shards_get_list();
					$tool_group_shard_list	= tool_admin_groups_shards_get_list($tool_group_edit_id, true);
					$tool_shard_list		= tool_admin_groups_shards_merge($tool_domain_list, $tool_shard_list, $tool_group_shard_list);

					$tpl->assign('tool_shard_list',			$tool_shard_list);

					$tool_appl_list			= tool_admin_applications_get_list();
					$tool_group_appl_list	= tool_admin_groups_applications_get_list($tool_group_edit_id, true);
					$tool_appl_list			= tool_admin_groups_applications_merge($tool_appl_list, $tool_group_appl_list);

					$tpl->assign('tool_application_list',	$tool_appl_list);

					$tool_group_user_list	= tool_admin_groups_get_user_list($tool_group_edit_id);

					$tpl->assign('tool_group_user_list',	$tool_group_user_list);

					break;

				case 'delete':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Delete an existing Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_group_delete_id		= $_POST['tool_form_group_id'];
					if (!($tool_group_delete_id > 0))
					{
						$tpl->assign('tool_alert_message',	"/!\ Error: invalid group!");
					}
					elseif ($tool_group_delete_id == $nel_user['user_group_id'])
					{
						$tpl->assign('tool_alert_message',	"/!\ Error: did you just try to delete your own group ?!");
					}
					else
					{
						tool_admin_groups_del($tool_group_delete_id);
					}

					break;

				case 'create':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Create a new Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_group_create_name		= $_POST['tool_form_group_name'];
					$tool_group_create_level	= $_POST['tool_form_group_level'];
					$tool_group_create_default	= $_POST['tool_form_group_default'];
					$tool_group_create_active	= $_POST['tool_form_group_active'];

					$tool_error = tool_admin_groups_add($tool_group_create_name, $tool_group_create_level, $tool_group_create_default, $tool_group_create_active);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					break;
			}

			$tool_group_list	= tool_admin_groups_get_list();
			$tpl->assign('tool_group_list',	$tool_group_list);
			$tpl->assign('tool_group_level_list',	$nel_user_group_levels);

			break;

		case 'applications':
			/*
			 * ###################################################################################################
			 *  Application Admin
			 * ###################################################################################################
			 */

			if (!tool_admin_applications_check('tool_admin_application'))	nt_common_redirect('index.php');

			$tool_action = null;
			if (isset($_POST['toolaction']))		$tool_action = $_POST['toolaction'];
			elseif (isset($_GET['toolaction']))	$tool_action = $_GET['toolaction'];

			switch ($tool_action)
			{
				case 'update':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update an existing Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_application_update_id				= $_POST['tool_form_application_id'];
					$tool_application_update_name			= $_POST['tool_form_application_name'];
					$tool_application_update_uri			= $_POST['tool_form_application_uri'];
					$tool_application_update_restriction	= $_POST['tool_form_application_restriction'];
					$tool_application_update_icon			= $_POST['tool_form_application_icon'];
					$tool_application_update_order			= $_POST['tool_form_application_order'];
					$tool_application_update_visible		= $_POST['tool_form_application_visible'];

					$tool_error = tool_admin_applications_update($tool_application_update_id, $tool_application_update_name, $tool_application_update_uri, $tool_application_update_restriction, $tool_application_update_icon, $tool_application_update_order, $tool_application_update_visible);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					$_GET['application_id'] = $tool_application_update_id;

					//break;

				case 'edit':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Edit an existing Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_application_edit_id		= $_GET['application_id'];
					$tool_application_edit_data 	= tool_admin_applications_get_id($tool_application_edit_id);
					$tpl->assign('tool_application_edit_data',	$tool_application_edit_data);

					break;

				case 'delete':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Delete an existing Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_application_delete_id		= $_POST['tool_form_application_id'];
					if (!($tool_application_delete_id > 0))
					{
						$tpl->assign('tool_alert_message',	"/!\ Error: invalid application!");
					}
					else
					{
						tool_admin_applications_del($tool_application_delete_id);
					}

					break;

				case 'create':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Create a new Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_application_create_name			= $_POST['tool_form_application_name'];
					$tool_application_create_uri			= $_POST['tool_form_application_uri'];
					$tool_application_create_restriction	= $_POST['tool_form_application_restriction'];
					$tool_application_create_icon			= $_POST['tool_form_application_icon'];
					$tool_application_create_order			= $_POST['tool_form_application_order'];
					$tool_application_create_visible		= $_POST['tool_form_application_visible'];

					$tool_error = tool_admin_applications_add($tool_application_create_name, $tool_application_create_uri, $tool_application_create_restriction, $tool_application_create_icon, $tool_application_create_order, $tool_application_create_visible);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					break;
			}

			$tool_application_list	= tool_admin_applications_get_list();
			$tpl->assign('tool_application_list',	$tool_application_list);

			break;

		case 'domains':
			/*
			 * ###################################################################################################
			 *  Domain Admin
			 * ###################################################################################################
			 */

			if (!tool_admin_applications_check('tool_admin_domain'))	nt_common_redirect('index.php');

			$tool_action = null;
			if (isset($_POST['toolaction']))		$tool_action = $_POST['toolaction'];
			elseif (isset($_GET['toolaction']))	$tool_action = $_GET['toolaction'];

			switch ($tool_action)
			{
				case 'update':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update an existing Domain
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_domain_update_id				= $_POST['tool_form_domain_id'];
					$tool_domain_update_name			= $_POST['tool_form_domain_name'];
					$tool_domain_update_application		= $_POST['tool_form_domain_application'];
					$tool_domain_update_as_host			= $_POST['tool_form_domain_as_host'];
					$tool_domain_update_as_port			= $_POST['tool_form_domain_as_port'];
					$tool_domain_update_mfs_web			= $_POST['tool_form_domain_mfs_web'];
					$tool_domain_update_rrd_path		= $_POST['tool_form_domain_rrd_path'];
					$tool_domain_update_las_admin_path	= $_POST['tool_form_domain_las_admin_path'];
					$tool_domain_update_las_local_path	= $_POST['tool_form_domain_las_local_path'];
					$tool_domain_update_sql_string		= $_POST['tool_form_domain_sql_string'];
					$tool_domain_update_cs_sql_string	= $_POST['tool_form_domain_cs_sql_string'];
					$tool_domain_update_hd_check		= $_POST['tool_form_domain_hd_check'];

					$tool_error = tool_admin_domains_update($tool_domain_update_id, $tool_domain_update_name, $tool_domain_update_application,
															$tool_domain_update_as_host, $tool_domain_update_as_port, $tool_domain_update_rrd_path,
															$tool_domain_update_las_admin_path, $tool_domain_update_las_local_path,
															$tool_domain_update_sql_string, $tool_domain_update_cs_sql_string,
															$tool_domain_update_hd_check, $tool_domain_update_mfs_web);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					$_GET['domain_id'] = $tool_domain_update_id;

					//break;

				case 'update_nel':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update an existing Domain (in the nel.domain table)
					 * -------------------------------------------------------------------------------------------
					 */

					if (isset($_POST['tool_form_domain_nel_id']))
					{
						$tool_domain_nel_update_id		= $_POST['tool_form_domain_nel_id'];
						$tool_domain_nel_update_name	= $_POST['tool_form_domain_nel_name'];
						$tool_domain_nel_update_status	= $_POST['tool_form_domain_nel_status'];
						//$tool_domain_nel_update_version	= $_POST['tool_form_domain_nel_version'];

						//tool_admin_domains_update_nel($tool_domain_nel_update_id, $tool_domain_nel_update_name, $tool_domain_nel_update_version, $tool_domain_nel_update_status);
						tool_admin_domains_update_nel($tool_domain_nel_update_id, $tool_domain_nel_update_name, $tool_domain_nel_update_status);

						$_GET['domain_id'] = $_POST['tool_form_domain_id'];
					}

					// break;

				case 'edit':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Edit an existing Domain
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_domain_edit_id	= $_GET['domain_id'];
					$tool_domain_edit_data 	= tool_admin_domains_get_id($tool_domain_edit_id);
					$tpl->assign('tool_domain_edit_data',	$tool_domain_edit_data);

					if ($tool_domain_edit_data['domain_application'] != '')
					{
						$domain_nel_status = array('ds_close','ds_dev','ds_restricted','ds_open');
						$tpl->assign('tool_domain_nel_status',	$domain_nel_status);

						$tool_domain_nel_data	= tool_admin_domains_get_nel($tool_domain_edit_data['domain_application']);
						$tpl->assign('tool_domain_nel_data',	$tool_domain_nel_data);
					}

					break;

				case 'delete':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Delete an existing Domain
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_domain_delete_id	= $_POST['tool_form_domain_id'];
					if (!($tool_domain_delete_id > 0))
					{
						$tpl->assign('tool_alert_message',	"/!\ Error: invalid domain!");
					}
					else
					{
						tool_admin_domains_del($tool_domain_delete_id);
					}

					break;

				case 'create':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Create a new Domain
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_domain_create_name			= $_POST['tool_form_domain_name'];
					$tool_domain_create_application		= $_POST['tool_form_domain_application'];
					$tool_domain_create_as_host			= $_POST['tool_form_domain_as_host'];
					$tool_domain_create_as_port			= $_POST['tool_form_domain_as_port'];
					$tool_domain_create_mfs_web			= $_POST['tool_form_domain_mfs_web'];
					$tool_domain_create_rrd_path		= $_POST['tool_form_domain_rrd_path'];
					$tool_domain_create_las_admin_path	= $_POST['tool_form_domain_las_admin_path'];
					$tool_domain_create_las_local_path	= $_POST['tool_form_domain_las_local_path'];
					$tool_domain_create_sql_string		= $_POST['tool_form_domain_sql_string'];
					$tool_domain_create_cs_sql_string	= $_POST['tool_form_domain_cs_sql_string'];
					$tool_domain_create_hd_check		= $_POST['tool_form_domain_hd_check'];

					$tool_error = tool_admin_domains_add(	$tool_domain_create_name, $tool_domain_create_application, $tool_domain_create_as_host,
															$tool_domain_create_as_port, $tool_domain_create_rrd_path,
															$tool_domain_create_las_admin_path, $tool_domain_create_las_local_path,
															$tool_domain_create_sql_string, $tool_domain_create_cs_sql_string,
															$tool_domain_create_hd_check, $tool_domain_create_mfs_web);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					break;
			}

			$tool_domain_list	= tool_admin_domains_get_list();
			$tpl->assign('tool_domain_list',	$tool_domain_list);

			break;

		case 'shards':
			/*
			 * ###################################################################################################
			 *  Shard Admin
			 * ###################################################################################################
			 */

			if (!tool_admin_applications_check('tool_admin_shard'))	nt_common_redirect('index.php');

			$tool_action = null;
			if (isset($_POST['toolaction']))		$tool_action = $_POST['toolaction'];
			elseif (isset($_GET['toolaction']))	$tool_action = $_GET['toolaction'];

			switch ($tool_action)
			{
				case 'update':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update an existing Shard
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_shard_update_id			= $_POST['tool_form_shard_id'];
					$tool_shard_update_name			= $_POST['tool_form_shard_name'];
					$tool_shard_update_as_id		= $_POST['tool_form_shard_as_id'];
					$tool_shard_update_domain_id	= $_POST['tool_form_shard_domain_id'];
					$tool_shard_update_language		= $_POST['tool_form_shard_language'];

					$tool_error = tool_admin_shards_update($tool_shard_update_id, $tool_shard_update_name, $tool_shard_update_as_id, $tool_shard_update_domain_id, $tool_shard_update_language);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					$_GET['shard_id'] = $tool_shard_update_id;

					//break;

				case 'edit':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Edit an existing Shard
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_shard_edit_id		= $_GET['shard_id'];
					$tool_shard_edit_data 	= tool_admin_shards_get_id($tool_shard_edit_id);
					$tpl->assign('tool_shard_edit_data',	$tool_shard_edit_data);

					break;

				case 'delete':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Delete an existing Shard
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_shard_delete_id	= $_POST['tool_form_shard_id'];
					if (!($tool_shard_delete_id > 0))
					{
						$tpl->assign('tool_alert_message',	"/!\ Error: invalid shard!");
					}
					else
					{
						tool_admin_shards_del($tool_shard_delete_id);
					}

					break;

				case 'create':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Create a new Shard
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_shard_create_name			= $_POST['tool_form_shard_name'];
					$tool_shard_create_as_id		= $_POST['tool_form_shard_as_id'];
					$tool_shard_create_domain_id	= $_POST['tool_form_shard_domain_id'];
					$tool_shard_create_language		= $_POST['tool_form_shard_language'];

					$tool_error = tool_admin_shards_add($tool_shard_create_name, $tool_shard_create_as_id, $tool_shard_create_domain_id, $tool_shard_create_language);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					break;
			}

			$tool_shard_list	= tool_admin_shards_get_list();
			$tool_domain_list	= tool_admin_domains_get_list();

			$tpl->assign('tool_shard_list',		$tool_shard_list);
			$tpl->assign('tool_domain_list',	$tool_domain_list);
			$tpl->assign('tool_language_list',	$tool_language_list);


			break;

		case 'restarts':
			/*
			 * ###################################################################################################
			 *  Restart Admin
			 * ###################################################################################################
			 */

			if (!tool_admin_applications_check('tool_admin_restart'))	nt_common_redirect('index.php');

			$tool_action = null;
			if (isset($_POST['toolaction']))		$tool_action = $_POST['toolaction'];
			elseif (isset($_GET['toolaction']))	$tool_action = $_GET['toolaction'];

			switch ($tool_action)
			{
				case 'update':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update an existing Restart Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_restart_update_id			= $_POST['tool_form_restart_id'];
					$tool_restart_update_name		= $_POST['tool_form_restart_name'];
					$tool_restart_update_services	= $_POST['tool_form_restart_services'];
					$tool_restart_update_order		= $_POST['tool_form_restart_order'];

					$tool_error = tool_admin_restarts_update($tool_restart_update_id, $tool_restart_update_name, $tool_restart_update_services, $tool_restart_update_order);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					$_GET['restart_id'] = $tool_restart_update_id;

					//break;

				case 'edit':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Edit an existing Restart Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_restart_edit_id		= $_GET['restart_id'];
					$tool_restart_edit_data 	= tool_admin_restarts_get_id($tool_restart_edit_id);
					$tpl->assign('tool_restart_edit_data',	$tool_restart_edit_data);

					break;

				case 'delete':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Delete an existing Restart Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_restart_delete_id	= $_POST['tool_form_restart_id'];
					if (!($tool_restart_delete_id > 0))
					{
						$tpl->assign('tool_alert_message',	"/!\ Error: invalid restart group!");
					}
					else
					{
						tool_admin_restarts_del($tool_restart_delete_id);
					}

					break;

				case 'create':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Create a new Restart Group
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_restart_create_name		= $_POST['tool_form_restart_name'];
					$tool_restart_create_services	= $_POST['tool_form_restart_services'];
					$tool_restart_create_order		= $_POST['tool_form_restart_order'];

					$tool_error = tool_admin_restarts_add($tool_restart_create_name, $tool_restart_create_services, $tool_restart_create_order);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					break;

				case 'update message':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Update an existing Restart Message
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_message_update_id			= $_POST['tool_form_message_id'];
					$tool_message_update_name		= $_POST['tool_form_message_name'];
					$tool_message_update_value		= $_POST['tool_form_message_value'];
					$tool_message_update_lang		= $_POST['tool_form_message_lang'];

					$tool_error = tool_admin_restart_messages_update($tool_message_update_id, $tool_message_update_name, $tool_message_update_value, $tool_message_update_lang);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					$_GET['msg_id'] = $tool_message_update_id;

					//break;

				case 'editmsg':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Edit an existing Restart Message
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_message_edit_id		= $_GET['msg_id'];
					$tool_message_edit_data 	= tool_admin_restart_messages_get_id($tool_message_edit_id);
					$tpl->assign('tool_message_edit_data',	$tool_message_edit_data);

					break;

				case 'delete message':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Delete an existing Restart Message
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_message_delete_id	= $_POST['tool_form_message_id'];
					if (!($tool_message_delete_id > 0))
					{
						$tpl->assign('tool_alert_message',	"/!\ Error: invalid restart message!");
					}
					else
					{
						tool_admin_restart_messages_del($tool_message_delete_id);
					}

					break;


				case 'create message':
					/*
					 * -------------------------------------------------------------------------------------------
					 *  Create a new Restart Message
					 * -------------------------------------------------------------------------------------------
					 */

					$tool_message_create_name		= $_POST['tool_form_message_name'];
					$tool_message_create_value		= $_POST['tool_form_message_value'];
					$tool_message_create_lang		= $_POST['tool_form_message_lang'];

					$tool_error = tool_admin_restart_messages_add($tool_message_create_name, $tool_message_create_value, $tool_message_create_lang);
					if ($tool_error != "")
					{
						$tpl->assign('tool_alert_message',	$tool_error);
					}

					break;
			}

			$tpl->assign('tool_language_list',		$tool_language_list);

			$tool_restart_list	= tool_admin_restarts_get_list();
			$tpl->assign('tool_restart_list',		$tool_restart_list);

			$tool_message_list	= tool_admin_restart_messages_get_list();
			$tpl->assign('tool_message_list',		$tool_message_list);

			break;

	}

	$tpl->display($tool_menu_item['tpl']);

?>