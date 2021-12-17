<?php

	require_once('common.php');
	require_once('functions_tool_main.php');
	require_once('functions_tool_mfs.php');

	if (!tool_admin_applications_check('tool_mfs'))	nt_common_redirect('index.php');

	nt_common_add_debug('-- Starting on \'tool_mfs.php\'');

	$tpl->assign('tool_title', "Mails & Forums");

	$view_domain_id = nt_auth_get_session_var('view_domain_id');
	$view_shard_id 	= nt_auth_get_session_var('view_shard_id');

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

	$template_file	= 'tool_mfs.tpl';

	if ($view_domain_id && $view_shard_id)
	{
		$tpl->assign('tool_page_title', 'Mails & Forums');

		$tpl->assign('tool_curl_output',tool_mfs_HTTPOpen("http://"));

	}


	$tpl->display($template_file);

?>