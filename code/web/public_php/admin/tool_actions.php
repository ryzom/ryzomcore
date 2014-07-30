<?php

	require_once('common.php');
	require_once('functions_tool_main.php');

	nt_common_add_debug('-- Starting on \'tool_actions.php\'');

	if (!tool_admin_applications_check('tool_actions'))	nt_common_redirect('index.php');

	$tpl->assign('tool_title', "Actions");


	$tpl->display('tool_actions.tpl');

?>