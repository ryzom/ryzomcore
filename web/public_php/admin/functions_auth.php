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

	/*
	 * Turn a password into what goes in the user_password column. bcrypt is
	 * named explicitly rather than PASSWORD_DEFAULT so the result stays 60
	 * characters and keeps fitting the varchar(64) column.
	 */
	function nt_auth_hash_password($passwd)
	{
		return password_hash(trim($passwd), PASSWORD_BCRYPT);
	}

	/*
	 * Check a password against a stored value. Accounts created before the
	 * move to bcrypt still hold a plain md5 of the password; those are
	 * accepted, and the caller rehashes them.
	 */
	function nt_auth_verify_password($passwd, $stored)
	{
		$passwd = trim($passwd);
		$stored = (string)$stored;

		if ($stored === '') return false;

		if (strlen($stored) === 32 && ctype_xdigit($stored))
		{
			// legacy md5, compared without leaking where it stops matching
			return hash_equals($stored, md5($passwd));
		}

		return password_verify($passwd, $stored);
	}

	/*
	 * True when the stored value is one of the old md5 hashes and should be
	 * replaced the next time we see the password in clear.
	 */
	function nt_auth_password_needs_rehash($stored)
	{
		$stored = (string)$stored;

		if (strlen($stored) === 32 && ctype_xdigit($stored)) return true;

		return password_needs_rehash($stored, PASSWORD_BCRYPT);
	}

	function nt_auth_check_login($user, $passwd)
	{
		global $db;

		$data = null;

		$user = $db->sql_escape_string(trim($user));

		// look the account up by name and check the password in php: the
		// stored value is a hash we cannot reproduce inside the query
		$sql = "SELECT * FROM ". NELDB_USER_TABLE ." LEFT JOIN ". NELDB_GROUP_TABLE ." ON (user_group_id=group_id) WHERE user_name='". $user ."' AND user_active=1 AND group_active=1";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$row = $db->sql_fetchrow($result);
				if (nt_auth_verify_password($passwd, $row['user_password']))
				{
					if (nt_auth_password_needs_rehash($row['user_password']))
					{
						$sql = "UPDATE ". NELDB_USER_TABLE ." SET user_password='". $db->sql_escape_string(nt_auth_hash_password($passwd)) ."' WHERE user_id=". intval($row['user_id']);
						$db->sql_query($sql);
					}
					$data = $row;
				}
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

		// Without httponly the session id is readable by any script that
		// makes it onto a page; without a samesite policy the cookie rides
		// along on cross site form posts. Only ask for the secure flag when
		// the request itself arrived over tls, or a plain http install could
		// never log in.
		ini_set('session.cookie_httponly', '1');
		ini_set('session.use_only_cookies', '1');
		ini_set('session.cookie_samesite', 'Lax');
		if ((isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== '' && strtolower($_SERVER['HTTPS']) !== 'off')
			|| (isset($_SERVER['SERVER_PORT']) && $_SERVER['SERVER_PORT'] == 443))
		{
			ini_set('session.cookie_secure', '1');
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
