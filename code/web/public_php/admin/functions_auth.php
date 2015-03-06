<?php

	/*
	 * THIS FILE SHOULD ONLY INCLUDE AUTHENTIFICATION RELATED FUNCTIONS
	 */

	function nt_auth_set_logging_count($user_id)
	{
		global $db;

		$sql = "UPDATE ". NELDB_USER_TABLE ." SET user_logged_count=user_logged_count+1,user_logged_last=". time() ." WHERE user_id=". (int)$user_id;
		$db->sql_query($sql);
	}

	function nt_auth_load_user($nelid)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_USER_TABLE ." LEFT JOIN ". NELDB_GROUP_TABLE ." ON (user_group_id=group_id) WHERE user_id=". (int)$nelid;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function nt_auth_get_group_name($group_id)
	{
		global $db;

		$sql = "SELECT user_name FROM ". NELDB_USER_TABLE ." WHERE user_id=". (int)$group_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$row = $db->sql_fetchrow($result);
				return $row['user_name'];
			}
		}

		return null;
	}

	function nt_auth_check_login($user, $passwd)
	{
		global $db;

		$data = null;

		$user = $db->sql_escape_string(trim($user));
		$passwd = md5(trim($passwd));

		$sql = "SELECT * FROM ". NELDB_USER_TABLE ." LEFT JOIN ". NELDB_GROUP_TABLE ." ON (user_group_id=group_id) WHERE user_name='". $user ."' AND user_password='". $passwd ."' AND user_active=1 AND group_active=1";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}
		return $data;
	}

	function nt_auth_load_login()
	{
		global $tpl;

		$tpl->assign('tool_login_title', 'Login');
		$tpl->display('index_login.tpl');
	}

	function nt_auth_start_session()
	{
		global $NEL_SETUP_SESSION;
		if (isset($NEL_SETUP_SESSION) && ($NEL_SETUP_SESSION))
		{
			return;
		}

		session_name(NELTOOL_SESSIONID);
		session_cache_limiter('nocache');
		session_start();

		header("Expires: Mon, 01 May 2000 06:00:00 GMT");
		header("Last-Modified: ". gmdate("D, d M Y H:i:s") ." GMT");
		header("Cache-Control: no-store, no-cache, must-revalidate");
		header("Cache-Control: post-check=0, pre-check=0", false);
		header("Pragma: no-cache");
	}

	function nt_auth_stop_session()
	{
		global $NEL_SETUP_SESSION;
		if (isset($NEL_SETUP_SESSION) && ($NEL_SETUP_SESSION))
		{
			return;
		}

		global $NELTOOL;

		foreach($NELTOOL['SESSION_VARS'] as $key => $val)
		{
			unset($NELTOOL['SESSION_VARS'][$key]);
		}
	}

	function nt_auth_set_session_var($name, $value)
	{
		global $NELTOOL;

		$NELTOOL['SESSION_VARS'][$name] = $value;
	}

	function nt_auth_get_session_var($name)
	{
		global $NELTOOL;

		if (isset($NELTOOL['SESSION_VARS'][$name])) return $NELTOOL['SESSION_VARS'][$name];
		return null;
	}

	function nt_auth_unset_session_var($name)
	{
		global $NELTOOL;

		unset($NELTOOL['SESSION_VARS'][$name]);
	}

?>
