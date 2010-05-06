<?php

	/*
	 * checks whether a password matches a users password
	 */
	function tool_pref_check_old_password($userinfo, $password)
	{
		if (!is_array($userinfo) && !isset($userinfo['user_id'])) return false;

		$encoded_password = md5($password);
		if ($encoded_password == $userinfo['user_password']) return true;

		return false;
	}

	/*
	 * update a user's password
	 */
	function tool_pref_update_user_password($userinfo, $password)
	{
		global $db;

		if (!is_array($userinfo) && !isset($userinfo['user_id'])) 	return false;
		if ($password == '')										return false;

		$encoded_password = md5(trim($password));

		$sql = "UPDATE ". NELDB_USER_TABLE ." SET user_password='". $encoded_password ."' WHERE user_id=". $userinfo['user_id'];
		$db->sql_query($sql);

		return true;
	}

	function tool_pref_update_menu_style($userinfo, $menu)
	{
		global $db;

		$sql = "UPDATE ". NELDB_USER_TABLE ." SET user_menu_style=". $menu ." WHERE user_id=". $userinfo['user_id'];
		$db->sql_query($sql);
	}

	function tool_pref_update_default_application($userinfo, $application_id)
	{
		global $db;

		$sql = "UPDATE ". NELDB_USER_TABLE ." SET user_default_application_id=". $application_id ." WHERE user_id=". $userinfo['user_id'];
		$db->sql_query($sql);
	}

?>