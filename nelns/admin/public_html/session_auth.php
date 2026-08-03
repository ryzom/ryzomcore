<?php
// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

	// authenticate
	function auth(&$error)
	{
		global	$command, $sessionAuth, $admcookielogin, $admcookiepassword, $sessionAuth;
		global	$admlogin, $admpassword, $uid, $gid, $useCookie, $group, $HTTP_POST_VARS, $HTTP_GET_VARS;
		unset($error);

		// Prefer POST for mutations; the logout link is still a GET, so accept
		// command from either after the request-to-global import in foo.php.
		$cmd = '';
		if (isset($HTTP_POST_VARS["command"]))
			$cmd = $HTTP_POST_VARS["command"];
		else if (isset($command))
			$cmd = $command;
		else if (isset($HTTP_GET_VARS["command"]))
			$cmd = $HTTP_GET_VARS["command"];

		switch($cmd)
		{
		case "logout":
			addToLog("Logout!");

			$uid = isset($sessionAuth["uid"]) ? $sessionAuth["uid"] : 0;
			if ($uid)
				logUser($uid, "LOGOUT");

			//session_unregister("sessionAuth"); 
			unset($_SESSION["sessionAuth"]);
			session_destroy();

			// erases cookies
			eraseCookies();

			unset($admlogin);
			unset($admpassword);
			unset($admcookielogin);
			unset($admcookiepassword);
			unset($uid);
			
			htmlProlog(htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES), "Logout", false);
			
			echo "<center>\n";
			echo "You are not logged any more<br>\n";
			echo "Click <a href='index.php'>here</a> to login<br>\n";
			echo "</center>\n";
			
			htmlEpilog();
			
			die();
			break;

		case "chPassword":
			addToLog("Change pass!");
			global	$chOldPass, $chNewPass, $chConfirmNewPass;

			// Identity is the existing session only. Do not take login or
			// stored hash from the form (that used to put the hash in HTML).
			if (!isset($sessionAuth) || empty($sessionAuth["admlogin"]) || empty($sessionAuth["admpassword"]))
			{
				$error = "Not logged in";
				eraseCookies();
				return 0;
			}

			$admlogin = $sessionAuth["admlogin"];
			$admpassword = $sessionAuth["admpassword"];
			$uid = isset($sessionAuth["uid"]) ? $sessionAuth["uid"] : 0;

			if (!($uid = validateId($admlogin, $admpassword, $useCookie, $gid, $group)))
			{
				$error = "Invalid session";
				eraseCookies();
				return 0;
			}

			if (!verifyPassword($chOldPass, $admpassword)
				|| (string)$chNewPass === ''
				|| (string)$chNewPass !== (string)$chConfirmNewPass)
			{
				// Stay logged in; do not fall through into login with a
				// half-updated password state.
				$error = "Password change failed";
				$sessionAuth = array("admlogin" => $admlogin, "admpassword" => $admpassword, "uid" => $uid);
				$_SESSION["sessionAuth"] = $sessionAuth;
				return 1;
			}

			$newHash = hashPassword($chNewPass);
			sqlquery("UPDATE user SET password='".sqlescape($newHash)."' WHERE uid='".intval($uid)."'");

			// never write the password, in clear or hashed, to the admin
			// log: htmlEpilog() prints that log back into the page
			addToLog("Password changed");

			if (function_exists('session_regenerate_id'))
				session_regenerate_id(true);

			$admpassword = $newHash;
			$sessionAuth = array("admlogin" => $admlogin, "admpassword" => $admpassword, "uid" => $uid);
			$_SESSION["sessionAuth"] = $sessionAuth;

			if ($useCookie)
				setupCookies($admlogin, $admpassword);

			logUser($uid, "CHPASSWORD");
			return 1;

		case "login":
			// turn the submitted password into the stored form; everything
			// downstream works with that, as it did before
			$admpassword = lookupStoredPassword($admlogin, $admpassword);

			addToLog("Login! -- admlogin='$admlogin'");

			if (!($uid = validateId($admlogin, $admpassword, $useCookie, $gid, $group)))
			{
				// Login is request-controlled; never reflect it raw into HTML.
				$error = "Invalid login '".htmlspecialchars((string)$admlogin, ENT_QUOTES)."'";
				print $error;
				eraseCookies();
				return 0;
			}

			// drop any session id the caller may have planted before auth
			if (function_exists('session_regenerate_id'))
				session_regenerate_id(true);

			$sessionAuth = array ("admlogin" => $admlogin, "admpassword" => $admpassword, "uid" => $uid);
			//session_register("sessionAuth");
			$_SESSION["sessionAuth"] = $sessionAuth;

			if ($useCookie)
				setupCookies($admlogin, $admpassword);

			logUser($uid, "LOGIN");

			return 1;
			break;

		default:

			if (!isset($sessionAuth) || $sessionAuth["admlogin"] == "")
			{
				// There used to be a fallback here that took the login and
				// the password out of the admcookielogin/admcookiepassword
				// cookies. What that password field holds is the stored hash,
				// which validateId() compares straight against the column, so
				// the cookie was the credential. setupCookies() has been
				// commented out for as long as this file has existed, so
				// nothing ever set those cookies -- only a caller supplying
				// their own would have reached it. No session, no login.
				addToLog("no session");
				return false;
			}
			else
			{
				$admlogin = $sessionAuth["admlogin"];
				$admpassword = $sessionAuth["admpassword"];
				$uid = $sessionAuth["uid"];
			}

			if (!($uid = validateId($admlogin, $admpassword, $useCookie, $gid, $group)))
			{
				if (!$uid)
				{
					$error = "Invalid login '$admlogin'";
					eraseCookies();
					return false;
				}
			}

			$sessionAuth = array ("admlogin" => $admlogin, "admpassword" => $admpassword, "uid" => $uid);
			//session_register("sessionAuth");
			$_SESSION["sessionAuth"] = $sessionAuth;

			if ($useCookie)
				setupCookies($admlogin, $admpassword);
			else
				eraseCookies();
				
			//logUser($uid, "BROWSE");

			return 1;
			break;
		}
	}


	/*
	 * Per-session token for the state changing links this tool builds as
	 * plain hrefs. The session cookie is SameSite=Lax, which keeps a cross
	 * site *post* from carrying it, but a top level navigation still does --
	 * and "delete this account" is one click on a GET url here.
	 */
	function nelnsCsrfToken()
	{
		if (empty($_SESSION['nelns_csrf']) || !is_string($_SESSION['nelns_csrf']))
		{
			$_SESSION['nelns_csrf'] = bin2hex(random_bytes(16));
		}
		return $_SESSION['nelns_csrf'];
	}

	function nelnsCsrfCheck($token)
	{
		return isset($_SESSION['nelns_csrf']) && is_string($_SESSION['nelns_csrf'])
			&& $_SESSION['nelns_csrf'] !== ''
			&& is_string($token) && hash_equals($_SESSION['nelns_csrf'], $token);
	}

	/*
	 * Produce the value stored in user.password. New passwords get a random
	 * per user salt; the old form was crypt($pass, "NL"), one fixed salt for
	 * every account and only the first eight characters of the password.
	 */
	function hashPassword($password)
	{
		return password_hash($password, PASSWORD_BCRYPT);
	}

	/*
	 * Check a plain password against a stored value, accepting accounts that
	 * still carry the old fixed salt form.
	 */
	function verifyPassword($password, $stored)
	{
		$stored = (string)$stored;

		if ($stored === '') return false;

		if (strncmp($stored, 'NL', 2) === 0 && strlen($stored) === 13)
		{
			return hash_equals($stored, crypt($password, "NL"));
		}

		return password_verify($password, $stored);
	}

	/*
	 * Verify a submitted password and hand back the stored hash, which is
	 * what the session and the cookies carry. Accounts still on the old fixed
	 * salt form are moved over while we have the password in clear.
	 * Returns an empty string when the password does not match.
	 */
	function lookupStoredPassword($admlogin, $password)
	{
		if (!preg_match('/^[a-zA-Z0-9]+$/', $admlogin)) return "";

		$res = sqlquery("SELECT uid, password FROM user WHERE BINARY login='".sqlescape($admlogin)."'");
		if (!$res || !($arr = sqlfetch($res))) return "";

		if (!verifyPassword($password, $arr["password"])) return "";

		if (strncmp($arr["password"], 'NL', 2) === 0 && strlen($arr["password"]) === 13)
		{
			$stored = hashPassword($password);
			sqlquery("UPDATE user SET password='".sqlescape($stored)."' WHERE uid='".intval($arr["uid"])."'");
			return $stored;
		}

		return $arr["password"];
	}

	// validate id
	function validateId($admlogin, $admpassword, &$useCookies, &$gid, &$group)
	{
		global	$REMOTE_ADDR;

		if (!preg_match('/^[a-zA-Z0-9]+$/', $admlogin))
		{
			//echo "DETECTED potential hacking login='$admlogin'<br>\n";
			return false;
		}

		addToLog("Validate login: '$admlogin'...");
		$res = sqlquery("SELECT auth.password AS password, auth.uid AS uid, auth.useCookie AS useCookie, auth.gid AS gid, ugroup.login AS gname, auth.allowed_ip AS allowed_ip FROM user AS auth, user AS ugroup WHERE BINARY auth.login='".sqlescape($admlogin)."' AND auth.gid=ugroup.uid");
		if (!$res || !($arr=sqlfetch($res)) || !($arr["uid"]) || !hash_equals((string)$arr["password"], (string)$admpassword))
		{
			addToLog("failed !!");
			return false;
		}
		// allowed_ip is an optional restriction set by an admin. A substring
		// match used to accept "1.2" against "11.22.x.x"; require a full IP
		// equality or a dotted prefix that ends on an octet boundary.
		$allowed_ip = trim((string)$arr["allowed_ip"]);
		if ($allowed_ip !== "")
		{
			$remote = (string)$REMOTE_ADDR;
			$ipOk = ($remote === $allowed_ip);
			if (!$ipOk && preg_match('/^[0-9A-Fa-f:.]+$/', $allowed_ip))
			{
				// Prefix: "192.168.1" matches "192.168.1.10" but not "192.168.10.1"
				if (strpos($remote, $allowed_ip) === 0)
				{
					$next = substr($remote, strlen($allowed_ip), 1);
					$ipOk = ($next === '' || $next === '.' || $next === ':');
				}
			}
			if (!$ipOk)
				return false;
		}

		addToLog("success");
		$useCookies = ($arr["useCookie"] == "yes");
		$gid = $arr["gid"];
		$group = $arr["gname"];
		return $arr["uid"];
	}


	// setup cookies
	function setupCookies($admlogin, $admpassword)
	{
/*
		setcookie("admcookielogin", $admlogin, time()+3600*24*15);
		setcookie("admcookiepassword", $admpassword, time()+3600*24*15);
*/
		addToLog("cookies set for admlogin=$admlogin");
	}

	// erase cookies
	function eraseCookies()
	{
		setcookie("admcookielogin");
		setcookie("admcookiepassword");
		
		addToLog("cookies reset");
	}

	// log user
	function logUser($uid, $act, $prefix="")
	{
		global	$HTTP_USER_AGENT, $REMOTE_ADDR, $userlogpath;

		$result = sqlquery("SELECT login FROM user WHERE uid='".sqlescape($uid)."'");
		if ($result && ($result=sqlfetch($result)))
		{
			$login = $result["login"];
			$filename = $userlogpath."/".$login.".log";
			$file = fopen($filename, "a");
			if ($file)
			{
				fwrite($file, ($prefix!="" ? $prefix." " : "").date("Y/m/d H:i:s")." $uid:$login:$HTTP_USER_AGENT:$REMOTE_ADDR $act\n");
				fclose($file);
			}
		}
		else
		{
			$filename = $userlogpath."/unreferenced_user.log";
			$file = fopen($filename, "a");
			if ($file)
			{
				fwrite($file, date("Y/m/d H:i:s")." $uid:<unknown login>:$HTTP_USER_AGENT:$REMOTE_ADDR $act\n");
				fclose($file);
			}
		}

/*
		$result = sqlquery("SELECT http_agent, remote_address, act FROM user_log WHERE uid='$uid' ORDER BY log_date DESC LIMIT 1");
		if (!$result || !($arr=sqlfetch($result)) || $arr["http_agent"]!=$HTTP_USER_AGENT || $arr["remote_address"]!=$REMOTE_ADDR || $arr["act"]!=$act)
		{
			sqlquery("INSERT INTO user_log SET uid='$uid', http_agent='$HTTP_USER_AGENT', remote_address='$REMOTE_ADDR', log_date=NOW(), act='$act'");
		}
*/
	}
?>
