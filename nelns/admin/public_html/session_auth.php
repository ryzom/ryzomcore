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
		global	$admlogin, $admpassword, $uid, $gid, $useCookie, $group, $HTTP_POST_VARS;
		unset($error);

		switch($HTTP_POST_VARS["command"])
		{
		case "logout":
			addToLog("Logout!");

			$uid = $sessionAuth["uid"];
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
			
			htmlProlog($_SERVER['PHP_SELF'], "Logout", false);
			
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

			if (!($uid = validateId($admlogin, $admpassword, $useCookie, $gid, $group)))
			{
				$error = "Invalid login '$admlogin'";
				eraseCookies();
				return 0;
			}

			if (crypt($chOldPass, "NL") == $admpassword && $chNewPass == $chConfirmNewPass)
			{
				sqlquery("UPDATE user SET password='".crypt($chNewPass, "NL")."' WHERE uid='$uid'");
				$admpassword = $chNewPass;
				
				addToLog("Changed password to '$chNewPass':'".crypt($chNewPass, "NL")."'");

				//session_unregister("sessionAuth"); 
				unset($_SESSION["sessionAuth"]);
				session_destroy();
			}

		case "login":
			$admpassword = crypt($admpassword, "NL");

			addToLog("Login! -- admlogin='$admlogin', admpassword='$admpassword'");

			if (!($uid = validateId($admlogin, $admpassword, $useCookie, $gid, $group)))
			{
				$error = "Invalid login '$admlogin'";
				print $error;
				eraseCookies();
				return 0;
			}

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
				print "no sessionauth or admlogin is blank";
				if (!isset($admcookielogin))
				{
					addToLog("cookie not set");
					return false;
				}
				else
				{
					$admlogin = $admcookielogin;
					$admpassword = $admcookiepassword;
				}
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


	// validate id
	function validateId($admlogin, $admpassword, &$useCookies, &$gid, &$group)
	{
		global	$REMOTE_ADDR;

		if (!ereg('^[a-zA-Z0-9]+$', $admlogin))
		{
			//echo "DETECTED potential hacking login='$admlogin'<br>\n";
			return false;
		}

		addToLog("Validate login: '$admlogin'/'$admpassword'...");
		$res = mysql_query("SELECT auth.password AS password, auth.uid AS uid, auth.useCookie AS useCookie, auth.gid AS gid, ugroup.login AS gname, auth.allowed_ip AS allowed_ip FROM user AS auth, user AS ugroup WHERE BINARY auth.login='$admlogin' AND auth.gid=ugroup.uid");
		if (!$res || !($arr=mysql_fetch_array($res)) || !($arr["uid"]) || $admpassword != $arr["password"])
		{
			addToLog("failed !!");
			return false;
		}
		$allowed_ip = $arr["allowed_ip"];
		if ($allowed_ip != "" && strstr($REMOTE_ADDR, $allowed_ip) == FALSE)
			return false;

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
		addToLog("cookies set to admlogin=$admlogin admpassword=$admpassword");
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

		$result = sqlquery("SELECT login FROM user WHERE uid='$uid'");
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
		if (!$result || !($arr=mysql_fetch_array($result)) || $arr["http_agent"]!=$HTTP_USER_AGENT || $arr["remote_address"]!=$REMOTE_ADDR || $arr["act"]!=$act)
		{
			sqlquery("INSERT INTO user_log SET uid='$uid', http_agent='$HTTP_USER_AGENT', remote_address='$REMOTE_ADDR', log_date=NOW(), act='$act'");
		}
*/
	}
?>
