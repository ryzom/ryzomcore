<?php

	/*
	 * checks whether a password matches a users password
	 */
	function tool_pref_check_old_password($userinfo, $password)
	{
		if (!is_array($userinfo) && !isset($userinfo['user_id'])) return false;

		return nt_auth_verify_password($password, $userinfo['user_password']);
	}

	/*
	 * update a user's password
	 */
	function tool_pref_update_user_password($userinfo, $password)
	{
		global $db;

		if (!is_array($userinfo) && !isset($userinfo['user_id'])) 	return false;
		if ($password == '')										return false;

		$encoded_password = nt_auth_hash_password($password);

		$sql = "UPDATE ". NELDB_USER_TABLE ." SET user_password='". $db->sql_escape_string($encoded_password) ."' WHERE user_id=". intval($userinfo['user_id']);
		$db->sql_query($sql);

		return true;
	}

	function tool_pref_update_menu_style($userinfo, $menu)
	{
		global $db;

		$sql = "UPDATE ". NELDB_USER_TABLE ." SET user_menu_style=". intval($menu) ." WHERE user_id=". intval($userinfo['user_id']);
		$db->sql_query($sql);
	}

	function tool_pref_update_default_application($userinfo, $application_id)
	{
		global $db;

		$sql = "UPDATE ". NELDB_USER_TABLE ." SET user_default_application_id=". intval($application_id) ." WHERE user_id=". intval($userinfo['user_id']);
		$db->sql_query($sql);
	}

?>