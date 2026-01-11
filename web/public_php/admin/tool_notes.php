<?php

	require_once('common.php');
	require_once('functions_tool_notes.php');

	if (!tool_admin_applications_check('tool_notes'))	nt_common_redirect('index.php');

	nt_common_add_debug('-- Starting on \'tool_notes.php\'');

	$tpl->assign('tool_title', "Notes");

	if (tool_admin_applications_check('tool_notes_global'))	$tpl->assign('restriction_tool_notes_global', true);

	if (isset($NELTOOL['GET_VARS']['note_id']))
	{
		$note_id		= $NELTOOL['GET_VARS']['note_id'];
		$tool_note_data	= tool_notes_get_id($nel_user['user_id'], $note_id);

		$tpl->assign('tool_note_edit_data',	$tool_note_data);
	}

	if (isset($NELTOOL['POST_VARS']['toolaction']))
	{
		$tool_action = $NELTOOL['POST_VARS']['toolaction'];

		switch($tool_action)
		{
			case 'create':

				$note_title 		= $NELTOOL['POST_VARS']['tool_form_note_title'];
				$note_data 			= $NELTOOL['POST_VARS']['tool_form_note_data'];
				$note_active		= $NELTOOL['POST_VARS']['tool_form_note_active'];
				$note_global		= (isset($NELTOOL['POST_VARS']['tool_form_note_global']) ? $NELTOOL['POST_VARS']['tool_form_note_global'] : 0);

				$note_mode			= $NELTOOL['POST_VARS']['tool_form_note_mode'];
				$note_uri			= $NELTOOL['POST_VARS']['tool_form_note_popup_uri'];
				$note_restriction	= $NELTOOL['POST_VARS']['tool_form_note_popup_restriction'];

				$tool_error = tool_notes_add($nel_user['user_id'], $note_title, $note_data, $note_active, $note_global, $note_mode, $note_uri, $note_restriction);
				if ($tool_error != "")
				{
					$tpl->assign('tool_alert_message',	$tool_error);
				}

				break;

			case 'update':

				$note_id 			= $NELTOOL['POST_VARS']['tool_form_note_id'];
				$note_title 		= $NELTOOL['POST_VARS']['tool_form_note_title'];
				$note_data 			= $NELTOOL['POST_VARS']['tool_form_note_data'];
				$note_active		= $NELTOOL['POST_VARS']['tool_form_note_active'];
				$note_global		= (isset($NELTOOL['POST_VARS']['tool_form_note_global']) ? $NELTOOL['POST_VARS']['tool_form_note_global'] : 0);

				$note_mode			= $NELTOOL['POST_VARS']['tool_form_note_mode'];
				$note_uri			= $NELTOOL['POST_VARS']['tool_form_note_popup_uri'];
				$note_restriction	= $NELTOOL['POST_VARS']['tool_form_note_popup_restriction'];

				$tool_error = tool_notes_update($nel_user['user_id'], $note_id, $note_title, $note_data, $note_active, $note_global, $note_mode, $note_uri, $note_restriction);
				if ($tool_error != "")
				{
					$tpl->assign('tool_alert_message',	$tool_error);
				}


				break;

			case 'delete':

				$note_id = $NELTOOL['POST_VARS']['tool_form_note_id'];
				tool_notes_del($nel_user['user_id'], $note_id);

				break;
		}
	}

	$tool_note_list = tool_notes_get_list($nel_user['user_id']);
	$tpl->assign('tool_note_list',	$tool_note_list);


	$tpl->display('tool_notes.tpl');

?>
