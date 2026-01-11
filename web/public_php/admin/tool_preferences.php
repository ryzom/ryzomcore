<?php

	require_once('common.php');
	require_once('functions_tool_preferences.php');

	if (!tool_admin_applications_check('tool_preferences')) nt_common_redirect('index.php');

	$tpl->assign("tool_title",			"My Preferences");
	$tpl->assign("tool_v_login",		$nel_user['user_name']);
	$tpl->assign("tool_v_user_id",		$nel_user['user_id']);
	$tpl->assign("tool_v_menu",			$nel_user['user_menu_style']);
	$tpl->assign("tool_v_application",	isset($nel_user['user_default_application_id']) ? $nel_user['user_default_application_id']:'') ;

	if (isset($NELTOOL['POST_VARS']['tool_form_user_id']))
	{
		$post_user_id 	= $NELTOOL['POST_VARS']['tool_form_user_id'];
		$tool_action	= $NELTOOL['POST_VARS']['toolaction'];

		switch ($tool_action)
		{
			/*
			 * update main preferences
			 */
			case 'update':

				$post_old_pwd 	= $NELTOOL['POST_VARS']['tool_form_password_old'];
				$post_new_pwd 	= $NELTOOL['POST_VARS']['tool_form_password_new'];
				$post_menu		= $NELTOOL['POST_VARS']['tool_form_menu_style'];

				// update menu style
				if ($nel_user['user_menu_style'] != $post_menu)
				{
					tool_pref_update_menu_style($nel_user, $post_menu);
					$tpl->assign("tool_v_menu",	$post_menu);
				}

				// update password
				if (($post_old_pwd != '') && ($post_new_pwd != ''))
				{
					if (tool_pref_check_old_password($nel_user, $post_old_pwd))
					{
						if (tool_pref_update_user_password($nel_user, $post_new_pwd))
						{
							$tpl->assign("tool_error", "Password has been updated!");
						}
						else
						{
							$tpl->assign("tool_error", "Invalid new password!");
						}
					}
					else
					{
						$tpl->assign("tool_error", "Old password does not match!");
					}
				}
				elseif (($post_old_pwd != '') || ($post_new_pwd != ''))
				{
					$tpl->assign("tool_error", "You need to type your current and new passwords!");
				}

				break;

			/*
			 * update default application
			 */
			case 'update default application':

				$post_new_application = $NELTOOL['POST_VARS']['tool_form_application_default'];

				if ($nel_user['user_default_application_id'] != $post_new_application)
				{
					tool_pref_update_default_application($nel_user, $post_new_application);
					$tpl->assign("tool_v_application",	$post_new_application);
				}

				break;
		}

	}

	$tpl->display('tool_preferences.tpl');

?>