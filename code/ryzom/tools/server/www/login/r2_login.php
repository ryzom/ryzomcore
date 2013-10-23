<?php

	error_reporting(E_ERROR | E_PARSE);
	set_error_handler('err_callback');

	// For error handling, buffer all output
	ob_start('ob_callback_r2login');

	include_once('config.php');
	include_once('login_translations.php');
	include_once('../tools/nel_message.php');
	include_once('../tools/domain_info.php');
	include_once('login_service_itf.php');
	include_once('../ring/join_shard.php');


	// see errorMsg
	function errorMsgBlock($errNum=GENERIC_ERROR_NUM) // $mixedArgs
	{
		$args = func_get_args();
		return '0:'.call_user_func_array('errorMsg', $args);
	}

	class LoginCb extends CLoginServiceWeb
	{
		// receive the login result sent back by the LS
		function loginResult($userId, $cookie, $resultCode, $errorString)
		{
			global $RingWebHost, $RingWebHostPHP;
			global $domainId;

			if ($resultCode == 0 && $cookie != "")
			{
				// gather the domain information (server version, patch urls and backup patch url
				global $DBHost, $DBUserName, $DBPassword, $DBName, $AutoInsertInRing;

				$link = mysql_connect($DBHost, $DBUserName, $DBPassword) or die (errorMsgBlock(3004, 'main', $DBHost, $DBUserName));
				mysql_select_db ($DBName) or die (errorMsgBlock(3005, 'main', $DBName, $DBHost, $DBUserName));
				$query = "SELECT * FROM domain WHERE domain_id=$domainId";
				$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

				if( mysql_num_rows($result) != 1)
				{
					die(errorMsgBlock(3001, $domainId));
				}
				$row = mysql_fetch_array($result);

				// set the cookie
				setcookie ( "ryzomId" , $cookie, 0, "/");
				$_COOKIE["ryzomId"] = $cookie; // make it available immediately

				// Auto-join an available mainland shard
				global $FSHostLuaMode, $FSHostResultStr;
				$FSHostLuaMode = false;
				$res = joinMainland($userId, $domainId, $row["domain_name"]);

				if ($res)
				{
					// return the cookie to the user, il will then be used as an auth to browse the site and to connect to the shard
					//echo "1#".$cookie."#http://".$RingWebHost."/ring/web_start.php\n";
// use this line to use woopra stats
//                  echo "1#".$cookie."#".$FSHostResultStr."#http://".$RingWebHost."/ring/web_start.php#http://".$RingWebHostPHP."/ring/#1\n";
					echo "1#".$cookie."#".$FSHostResultStr."#http://".$RingWebHost."/ring/web_start.php#http://".$RingWebHostPHP."/ring/\n";
					// return the ring domain information
					echo $row["patch_version"]."#".$row["backup_patch_url"]."#".$row["patch_urls"];
				}
				else
				{
					global $JoinSessionResultCode, $JoinSessionResultMsg;
					echo errorMsgBlock(BASE_TRANSLATED_RSM_ERROR_NUM + $JoinSessionResultCode, $JoinSessionResultCode, $JoinSessionResultMsg, $userId);
				}
			}
			else
			{
				// empty cookie, this mean the user id can't be validated by the LS
				echo errorMsgBlock(BASE_TRANSLATED_LS_ERROR_NUM + $resultCode, $resultCode, $errorString, $userId);
			}
		}
	}

	class CWwwLog
	{
		//function CWwwLog() {}

		/*
		 * Return the log directory. Create it if it does not exist, or return false if creation failed.
		 */
		function getSafeLogDir()
		{
			// Examples:
			// __FILE__ = r:\code\ryzom\www\login\config.php
			// $_SERVER['PATH_TRANSLATED'] = 'r:/code/ryzom/www/login//r2_login.php'
			// $_SERVER['SCRIPT_FILENAME'] = 'r:/code/ryzom/www/login//r2_login.php'
			global $LogRelativePath;
			$pathInfo = pathinfo(__FILE__);
			$logPath = $pathInfo['dirname'].'/'.$LogRelativePath;
			if (!is_dir($logPath))
			{
				$res = mkdir($LogPath, 0700);
				return $res ? $logPath : false;
			}
			return $logPath;
		}

		function logStr($str)
		{
			$logPath = $this->getSafeLogDir();
			if ($logPath !== false)
			{
				$fp = fopen($logPath.'/r2_login_'.date('Y-m-d').'.log', 'a');
				fwrite($fp, date('Y-m-d H:i:s').' ('.$_SERVER['REMOTE_ADDR'].':'.$_SERVER['REQUEST_URI']."): $str\n");
				fclose($fp);
			}
		}
	}

	// Callback called on end of output buffering
	function ob_callback_r2login($buffer)
	{
		// Log only in case of error or malformed result string
		$blockHd = substr($buffer, 0, 2);
		if ($blockHd != '1:')
		{
			$logFile = new CWwwLog();
			$logFile->logStr(str_replace("\n",'\n',$buffer));
		}
		return $buffer; // sent to output
	}

	// Callback called on error
	function err_callback($errno, $errmsg, $filename, $linenum, $vars)
	{
		$logFile = new CWwwLog();
		$logFile->logStr("PHP ERROR/$errno $errmsg ($filename:$linenum)");
		// Never die after an error
	}

	if (!isset($_GET['cmd']))
	{
		die (errorMsgBlock(3002));
	}

	// check for 'clear password' tag
	if (!isset($_GET['cp']))
	{
		$cp = 0;
	}
	else
	{
		$cp = $_GET['cp'];
	}

	$submittedLang = isset($_GET['lg']) ? $_GET['lg'] : 'unknown';
	if (isset($_GET['dbg']) && ($_GET['dbg'] == 1))
		$DisplayDbg = true;

	switch($_GET['cmd'])
	{
	case 'ask':
		// client ask for a login salt
		askSalt($_GET['login'], $submittedLang);
		die();
	case 'login':
		$domainId = -1;
		// client sent is login info
		if (!checkUserValidity($_GET['login'], $_GET['password'], $_GET['clientApplication'], $cp, $id, $reason, $priv, $extended, $domainId, $submittedLang))
		{
			echo '0:'.$reason;
		}
		else
		{

			// retreive the domain info
			$domainInfo = getDomainInfo($domainId);

			// if we need to create missing ring info
			if ($AutoCreateRingInfo)
			{
				////////////// Temporary code alpha 0 only /////////////////////////////////////
				// check if the ring user exist, and create it if not
				$ringDb = mysql_connect($DBHost, $RingDBUserName, $RingDBPassword) or die(errorMsgBlock(3004, 'Ring', $DBHost, $RingDBUserName));
				mysql_select_db ($domainInfo['ring_db_name'], $ringDb) or die(errorMsgBlock(3005, 'Ring', $domainInfo['ring_db_name'], $DBHost, $RingDBUserName));
				$query = "SELECT user_id FROM ring_users where user_id = '".$id."'";
				$result = mysql_query ($query) or die(errorMsgBlock(3006, $query, 'Ring', $domainInfo['ring_db_name'], $DBHost, $RingDBUserName, mysql_error()));

				if (mysql_num_rows($result) == 0)
				{
					// no ring user record, build one
					$query = "INSERT INTO ring_users SET user_id = '".$id."', user_name = '".$_GET["login"]."', user_type='ut_pioneer'";
					$result = mysql_query ($query) or die(errorMsgBlock(3006, $query, 'Ring', $domainInfo['ring_db_name'], $DBHost, $RingDBUserName, mysql_error()));
				}

//				// check that there is a character record (deprecated)
//				$query = "SELECT user_id FROM characters where user_id = '".$id."'";
//				$result = mysql_query ($query) or die("Query ".$query." failed");
//				if (mysql_num_rows($result) == 0)
//				{
//					// no characters record, build a default one
//					$charId = ($id * 16);
//					$query = "INSERT INTO characters SET char_id='".$charId."', char_name='".$_GET["login"]."_default', user_id = '".$id."'";
//					$result = mysql_query ($query) or die("Query ".$query." failed");
//				}
			}

//			// check domain status
//			if ($domainInfo['status'] == "ds_close")
//			{
//				// the domain is closed
//				echo "0:Server is currently closed";
//				die;
//			}
//			else if ($domainInfo['status'] == "ds_dev" && strstr($priv, ":DEV:") == false)
//			{
//				// the domain is open to dev only
//				echo "0:You are not allowed to connect now, retry later";
//				die;
//			}
//			else if ($domainInfo['status'] == "ds_restricted")
//			{
//				// check for one of the needed privilege
//				if (	strstr($priv, ":DEV:") == false
//					&& 	strstr($priv, ":SGM:") == false
//					&& 	strstr($priv, ":GM:") == false
//					&& 	strstr($priv, ":EG:") == false)
//				{
//					// the domain is open to privileged user only
//					echo "0:You are not allowed to connect now, retry later";
//					die;
//				}
//			}

			// store the web host for this domain
			global $RingWebHost, $RingWebHostPHP;
			$RingWebHost = $domainInfo['web_host'];
			$RingWebHostPHP = $domainInfo['web_host_php'];

			$LSaddr = split(":", $domainInfo['login_address']);

			// ask for a session cookie to the login service
			$login = new LoginCb;
			$res = "";
			$login->connect($LSaddr[0], $LSaddr[1], $res);
//			$lsProxy = new CLoginServiceWebProxy;
			$login->login($id, $_SERVER["REMOTE_ADDR"], $domainId);

			// wait for the return message
//			$lsSkel = new CLoginServiceWebSkel;
			if (!$login->waitCallback())
			{
				die(errorMsgBlock(3003));
			}

			//the rest of the process is done in the callback function
		}
	}

	// no more to do (other global statement are old garbage)
	die();

// ----------------------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------------------

	// $reason contains the reason why the check failed or success
	// return true if the check is ok
	function checkUserValidity ($login, $password, $clientApplication, $cp, &$id, &$reason, &$priv, &$extended, &$domainId, $lang)
	{
		global $DBHost, $DBUserName, $DBPassword, $DBName, $AcceptUnknownUser;

		setMsgLanguage($lang);

		// we map the client application to the domain name
		$domainName = $clientApplication;

		$link = mysql_connect($DBHost, $DBUserName, $DBPassword) or die (errorMsgBlock(3004, 'main', $DBHost, $DBUserName));
		mysql_select_db ($DBName) or die (errorMsgBlock(3005, 'main', $DBName, $DBHost, $DBUserName));
		// retreive the domain id
		$query = "SELECT domain_id FROM domain WHERE domain_name='$domainName'";
		$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

		if (mysql_num_rows($result) == 0)
		{
			// unrecoverable error, we must giveup
			$reason = errorMsg(3007, $domainName);
			mysql_close($link);
			return false;
		}

		$row = mysql_fetch_array($result);
		$domainId = $row[0];

		// retreive the domain info
		$domainInfo = getDomainInfo($domainId);

		// convert the domain status enum into the privilege access set
		$accessPriv = strtoupper(substr($domainInfo['status'], 3));

		// now, retrieve the user infos
		$query = "SELECT * FROM user where Login='$login'";
		$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

		if (mysql_num_rows ($result) == 0)
		{
			if ($AcceptUnknownUser)
			{
				// login doesn't exist, create it
				$query = "INSERT INTO user (Login, Password) VALUES ('$login', '$password')";
				$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

				// get the user to have his UId
				$query = "SELECT * FROM user WHERE Login='$login'";
				$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

				if (mysql_num_rows ($result) == 1)
				{
					$reason = errorMsg(3008, $login);
					$row = mysql_fetch_array ($result);
					$id = $row["UId"];
					$priv = $row["Privilege"];
					$extended = $row["ExtendedPrivilege"];

					// add the default permission
					$query = "INSERT INTO permission (UId, ClientApplication, AccessPrivilege) VALUES ('$id', 'r2', '$accessPriv')";
					$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

					$res = false;
				}
				else
				{
					$reason = errorMsg(3009, $login);
					$res = false;
				}
			}
			else
			{
				// Check if this is not an unconfirmed account
				$query = "SELECT GamePassword, Email, Language FROM signup_data WHERE login='$login'";
				$result = mysql_query($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

				if (mysql_num_rows($result) == 0)
				{
					$reason = errorMsg(2001, $login, 'checkUserValidity');
					$res = false;
				}
				else
				{
					// Check password to avoid revealing email address to third-party
					$passwordMatchedRow = false;
					while ($row = mysql_fetch_array($result))
					{
						$salt = substr($row['GamePassword'],0,2);
						if (($cp && $row['GamePassword'] == $password) || (!$cp && $row['GamePassword'] == crypt($password, $salt)))
						{
							$passwordMatchedRow = $row;
							break;
						}
					}
					if ($passwordMatchedRow !== false)
					{
						if ($lang == 'unknown')
							setMsgLanguage($passwordMatchedRow['Language']);
						$reason = errorMsg(2002, $passwordMatchedRow['Email']);
					}
					else
						$reason = errorMsg(2004, 'db signup_data');
					$res = false;
				}
			}
		}
		else
		{
			$row = mysql_fetch_array ($result);
			$salt = substr($row["Password"],0,2);
			if (($cp && $row["Password"] == $password) || (!$cp && $row["Password"] == crypt($password, $salt)))
			{
				// Store the real login (with correct case)
				$_GET['login'] = $row['Login'];
				// check if the user can use this application

				$query = "SELECT * FROM permission WHERE UId='".$row["UId"]."' AND ClientApplication='$clientApplication'";
				$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));
				if (mysql_num_rows ($result) == 0)
				{
					if ($AcceptUnknownUser)
					{
						// add default permission
						$query = "INSERT INTO permission (UId, ClientApplication, ShardId, AccessPrivilege) VALUES ('".$row["UId"]."', '$clientApplication', -1, '$domainStatus')";
						$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

						$reason = errorMsg(3010);
						$res = false;
					}
					else
					{
						// no permission
						$reason = errorMsg(3011, $clientApplication, $domainName);
						$res = false;
					}
				}
				else
				{
					// check that the access privilege for the domain
					$permission = mysql_fetch_array($result);

					if (!strstr($permission['AccessPrivilege'], $accessPriv))
					{
						// no right to connect
						if ($AcceptUnknownUser)
						{
							// set an additionnal privilege for this player
							$query = "UPDATE permission set AccessPrivilege='".$permission['AccessPrivilege'].",$accessPriv' WHERE prim=".$permission['prim'];
							$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

							$reason = errorMsg(3012, $accessPriv);
							$res = false;
						}
						else
						{
							// no permission
							$reason = errorMsg(3013, $clientApplication, $domainName, $accessPriv);
							$res = false;
						}
					}
					else
					{

//						// check if the user not already online
//
//						if ($row["State"] != "Offline")
//						{
//							$reason = "$login is already online and ";
//							// ask the LS to remove the client
//							if (disconnectClient ($row["ShardId"], $row["UId"], $tempres))
//							{
//								$reason =  $reason."was just disconnected. Now you can retry the identification (error code 54)";
//
//								$query = "update shard set NbPlayers=NbPlayers-1 where ShardId=".$row["ShardId"];
//								$result = mysql_query ($query) or die ("Can't execute the query: '$query' errno:".mysql_errno().": ".mysql_error());
//
//								$query = "update user set ShardId=-1, State='Offline' where UId=".$row["UId"];
//								$result = mysql_query ($query) or die ("Can't execute the query: '$query' errno:".mysql_errno().": ".mysql_error());
//							}
//							else
//							{
//								$reason = $reason."can't be disconnected: $tempres (error code 55)";
//							}
//							$res = false;
//						}
//						else
//						{
						$id = $row["UId"];
						$priv = $row["Privilege"];
						$extended = $row["ExtendedPrivilege"];
						$res = true;
//						}
					}
				}
			}
			else
			{
				$reason = errorMsg(2004, 'user');
				$res = false;
			}
		}
		mysql_close($link);
		return $res;
	}

	function askSalt($login, $lang)
	{
		global $PHP_SELF;
		global $DBHost, $DBUserName, $DBPassword, $DBName;
		global $AcceptUnknownUser;

		setMsgLanguage($lang);

		$link = mysql_connect($DBHost, $DBUserName, $DBPassword) or die (errorMsgBlock(3004, 'main', $DBHost, $DBUserName));
		mysql_select_db ($DBName) or die (errorMsgBlock(3005, 'main', $DBName, $DBHost, $DBUserName));

		$query = "SELECT Password FROM user WHERE Login='$login'";
		$result = mysql_query ($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

		if (mysql_num_rows ($result) != 1)
		{
			if ($AcceptUnknownUser)
			{
				// just accept the client and return a default salk
				echo "1:AA";
				die;
			}
			else
			{
				// Check if this is not an unconfirmed account
				$query = "SELECT GamePassword, Language FROM signup_data WHERE login='$login'";
				$result = mysql_query($query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysql_error()));

				if (mysql_num_rows($result) == 0)
				{
					// no user record, reject it
					die (errorMsgBlock(2001, $login, 'askSalt'));
				}
				else if (mysql_num_rows($result) == 1)
				{
					// one unconfirmed record, let the client send the encrypted password to get the corresponding email address
					$row = mysql_fetch_array($result);
					$salt = substr($row['GamePassword'], 0, 2);
				}
				else
				{
					if ($lang == 'unknown')
					{
						// several matching records => display a multi-language message now
						$languages = array();
						while ($row = mysql_fetch_array($result))
						{
							$languages[$row['Language']] = true;
						}
						setMsgLanguage(array_keys($languages));
					}
					die (errorMsgBlock(2003));
				}
			}
		}
		else
		{
			$res_array = mysql_fetch_array($result);
			$salt = substr($res_array['Password'], 0, 2);
		}

		echo "1:".$salt;
		mysql_close($link);
	}

?>
