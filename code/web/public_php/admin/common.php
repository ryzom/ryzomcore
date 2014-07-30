<?php

	//error_reporting(E_ALL); // extreme debug info ;)
	set_time_limit(180); // 3 min time out

	// add the path for the nel class files
	ini_set('include_path',ini_get('include_path') .':./nel/');

	require_once('config.php');

	require_once(NELTOOL_SYSTEMBASE .'functions_common.php');
	require_once(NELTOOL_SYSTEMBASE .'functions_auth.php');

	//assert_options(ASSERT_ACTIVE, 1);
	//assert_options(ASSERT_WARNING, 0);
	//assert_options(ASSERT_QUIET_EVAL, 1);
	//assert_options(ASSERT_CALLBACK, 'nt_common_assert');

	require_once(NELTOOL_SYSTEMBASE .'functions_mysqli.php');
	require_once(NELTOOL_SYSTEMBASE .'smarty/Smarty.class.php');
	require_once(NELTOOL_SYSTEMBASE. 'functions_tool_administration.php');
	require_once(NELTOOL_SYSTEMBASE. 'nel/admin_modules_itf.php');

	// lets set up the database
	$db = new sql_db(NELTOOL_DBHOST, NELTOOL_DBUSER, NELTOOL_DBPASS, NELTOOL_DBNAME);
	if (!is_object($db)) die("error on db init");

	// lets set up the smarty template engine
	$tpl = new Smarty;
	if (!is_object($tpl)) die("error on smarty init");

    $iPhone = (strstr($_SERVER['HTTP_USER_AGENT'], "iPhone") !== FALSE);
	$tpl->assign('iPhone', $iPhone);

	$tpl->template_dir	= NELTOOL_SYSTEMBASE .'/templates/default/';
	$tpl->compile_dir	= NELTOOL_SYSTEMBASE .'/templates/default_c/';
	$tpl->config_dir	= NELTOOL_SYSTEMBASE .'/templates/config/';
	$tpl->cache_dir		= NELTOOL_SYSTEMBASE .'/templates/cache/';

	$tpl->caching = false;
	$tpl->clear_all_cache();
	if (NELTOOL_DEBUG) $tpl->debugging = false;

	if (defined('NELTOOL_NO_USER_NEEDED'))
	{
		// this is used for cron jobs that don't need authentifications when running
	}
	else
	{
		nt_auth_start_session();

		$NELTOOL = array(
						'POST_VARS'		=> &$_POST,
						'GET_VARS'		=> &$_GET,
						'COOKIE_VARS'	=> &$_COOKIE,
						'SESSION_VARS'	=> &$_SESSION,
						'SERVER_VARS'	=> &$_SERVER,
						);

		$nel_user = null;
		$nel_debug = array();

		nt_common_add_debug(date("Y-m-d H:i:s",time()));
		nt_common_add_debug('-- Basic init complete, time to get some work done.');


		// login and session process
		if (isset($NELTOOL['GET_VARS']['mode']) && ($NELTOOL['GET_VARS']['mode'] == 'logout'))
		{
			$nel_user = null;
			nt_auth_stop_session();
			nt_common_redirect('');
			exit();
		}
		elseif (isset($NELTOOL['SESSION_VARS']['nelid']) && !empty($NELTOOL['SESSION_VARS']['nelid']))
		{
			$nel_user = nt_auth_load_user($NELTOOL['SESSION_VARS']['nelid']);
		}
		elseif (isset($NELTOOL['POST_VARS']['nel_login']) && isset($NELTOOL['POST_VARS']['nel_passwd']) && ($NELTOOL['POST_VARS']['action'] == 'login'))
		{
			$nel_user = nt_auth_check_login($NELTOOL['POST_VARS']['nel_login'], $NELTOOL['POST_VARS']['nel_passwd']);
			if ($nel_user)
			{
				nt_auth_set_session_var('nelid',$nel_user['user_id']);
				nt_auth_set_logging_count($nel_user['user_id']);
				$nel_user['new_login'] = true;
			}
		}

		if (!$nel_user)
		{
			nt_auth_load_login();
			exit();
		}

		nt_common_add_debug('-- User authentification complete.');

		// some site settings

		if (NELTOOL_DEBUG && ($nel_user['group_level'] == 10)) // need to use the array instead of the value being hardcoded
		{
			$tpl->assign('NELTOOL_DEBUG', true); //$nel_debug);
		}

		$tpl->assign('nel_script',			$NELTOOL['SERVER_VARS']['SCRIPT_NAME']);
		$tpl->assign('nel_request_uri', 	basename($NELTOOL['SERVER_VARS']['REQUEST_URI']));
		$tpl->assign('nel_tool_title',		NELTOOL_SITETITLE);
		$tpl->assign('nel_web_base_uri',	NELTOOL_SITEBASE);

		$tpl->assign('tool_title',			"&lt;unknown&gt;");
		$tpl->assign('user_info',			$nel_user['user_name'] .' ('. $nel_user['group_name'] .')');

		// load user & group applications/domains/shards

		$nel_user['access'] = array(
									'applications'			=> tool_admin_applications_get_list(),
									'user_applications'		=> tool_admin_users_applications_get_list($nel_user['user_id']),
									'user_domains'			=> tool_admin_users_domains_get_list($nel_user['user_id']),
									'user_shards'			=> tool_admin_users_shards_get_list($nel_user['user_id']),
									'group_applications'	=> tool_admin_groups_applications_get_list($nel_user['user_group_id']),
									'group_domains'			=> tool_admin_groups_domains_get_list($nel_user['user_group_id']),
									'group_shards'			=> tool_admin_groups_shards_get_list($nel_user['user_group_id']),
									);

		$nel_user['access']['domains'] = tool_admin_users_groups_domains_merge();
		$nel_user['access']['shards'] = tool_admin_users_groups_shards_merge();
		$nel_user['has_lock'] = false;

		//nt_common_add_debug($nel_user);

		// load the user application menu

		$tool_application_list = tool_admin_applications_build_menu_list($nel_user['access']);
		$tpl->assign('nel_menu',			$tool_application_list);
		$tpl->assign('menu_style',			$nel_user['user_menu_style']);
		$tpl->assign('unknown_menu',		'imgs/icon_unknown.png');

		if (isset($nel_user['new_login']))
		{
			$default_user_application_id	= 0;
			if (isset( $nel_user['user_default_application_id']) &&($nel_user['user_default_application_id'] > 0)) {
                $default_user_application_id	= $nel_user['user_default_application_id']; 
			}elseif (isset( $nel_user['group_default_application_id']) &&($nel_user['group_default_application_id'] > 0)) {
                $default_user_application_id	= $nel_user['group_default_application_id'];
            }
            
			if ($default_user_application_id > 0)
			{
				nt_common_add_debug("default application : user:". $nel_user['user_default_application_id'] ." group:". $nel_user['group_default_application_id']);
				$default_user_application_data	= tool_admin_applications_get_default($tool_application_list, $default_user_application_id);
				nt_common_redirect($default_user_application_data['application_uri']);
				exit();
			}
		}

		$nel_tool_extra_css = '';
		if (BG_IMG !== null)
		{
			$nel_tool_extra_css .= "<style><!--\n";
			$nel_tool_extra_css .= "body { background: url(". BG_IMG ."); }\n";
			$nel_tool_extra_css .= "--></style>\n";
		}
		$tpl->assign('nel_tool_extra_css', $nel_tool_extra_css);
		$tpl->assign('system_time',	time());

		nt_common_add_debug('-- Common init. complete.');
	}

?>
