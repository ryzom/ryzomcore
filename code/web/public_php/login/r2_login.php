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


    function get_salt($password)
    {
        if ($password[0] == '$')
        {
            $salt = substr($password, 0, 19);
        }
        else
        {
            $salt = substr($password, 0, 2);
        }
        return $salt;
    }

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

				$link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die (errorMsgBlock(3004, 'main', $DBHost, $DBUserName));
				mysqli_select_db ($link, $DBName) or die (errorMsgBlock(3005, 'main', $DBName, $DBHost, $DBUserName));
				$query = "SELECT * FROM domain WHERE domain_id=$domainId";
				$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

				if( mysqli_num_rows($result) != 1)
				{
					die(errorMsgBlock(3001, $domainId));
				}
				$row = mysqli_fetch_array($result);

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
				// check if the ring user exist, and create it if not
				$ringDb = mysqli_connect($DBHost, $RingDBUserName, $RingDBPassword) or die(errorMsgBlock(3004, 'Ring', $DBHost, $RingDBUserName));
				mysqli_select_db ($ringDb, $domainInfo['ring_db_name']) or die(errorMsgBlock(3005, 'Ring', $domainInfo['ring_db_name'], $DBHost, $RingDBUserName));
				$query = "SELECT user_id FROM ring_users where user_id = '".$id."'";
				$result = mysqli_query ($ringDb, $query) or die(errorMsgBlock(3006, $query, 'Ring', $domainInfo['ring_db_name'], $DBHost, $RingDBUserName, mysqli_error($ringDb)));

				if (mysqli_num_rows($result) == 0)
				{
					// no ring user record, build one
					$login = mysqli_real_escape_string($ringDb, $_GET['login']);
					$query = "INSERT INTO ring_users SET user_id = '$id', user_name = '$login', user_type='ut_pioneer'";
					$result = mysqli_query ($ringDb, $query) or die(errorMsgBlock(3006, $query, 'Ring', $domainInfo['ring_db_name'], $DBHost, $RingDBUserName, mysqli_error($ringDb)));
				}
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

			$LSaddr = explode(":", $domainInfo['login_address']);

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

		$link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die (errorMsgBlock(3004, 'main', $DBHost, $DBUserName));
		mysqli_select_db ($link, $DBName) or die (errorMsgBlock(3005, 'main', $DBName, $DBHost, $DBUserName));

		// we map the client application to the domain name
		$domainName = mysqli_real_escape_string($link, $clientApplication);

		// retreive the domain id
		$query = "SELECT domain_id FROM domain WHERE domain_name='$domainName'";
		$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

		if (mysqli_num_rows($result) == 0)
		{
			// unrecoverable error, we must giveup
			$reason = errorMsg(3007, $domainName);
			mysqli_close($link);
			return false;
		}

		$row = mysqli_fetch_array($result);
		$domainId = $row[0];

		// retreive the domain info
		$domainInfo = getDomainInfo($domainId);

		// convert the domain status enum into the privilege access set
		$accessPriv = strtoupper(substr($domainInfo['status'], 3));

		// now, retrieve the user infos
		$login = mysqli_real_escape_string($link, $login);
		$query = "SELECT * FROM user where Login='$login'";
		$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

		if (mysqli_num_rows ($result) == 0)
		{
			if ($AcceptUnknownUser)
			{
				// login doesn't exist, create it
				$password = mysqli_real_escape_string($link, $password);
				$query = "INSERT INTO user (Login, Password) VALUES ('$login', '$password')";
				$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

				// get the user to have his UId
				$query = "SELECT * FROM user WHERE Login='$login'";
				$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

				if (mysqli_num_rows ($result) == 1)
				{
					$reason = errorMsg(3008, $login);
					$row = mysqli_fetch_assoc ($result);
					$id = $row["UId"];
					$priv = $row["Privilege"];
					$extended = $row["ExtendedPrivilege"];

					// add the default permission
					$query = "INSERT INTO permission (UId, DomainId, AccessPrivilege) VALUES ('$id', '$domainId', '$accessPriv')";
					$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

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
				$reason = errorMsg(2001, $login, 'checkUserValidity');
			}
		}
		else
		{
			$row = mysqli_fetch_assoc ($result);
			$salt = get_salt($row["Password"]);
			if (($cp && $row["Password"] == $password) || (!$cp && $row["Password"] == crypt($password, $salt)))
			{
				// Store the real login (with correct case)
				$_GET['login'] = $row['Login'];
				// check if the user can use this application

				$clientApplication = mysqli_real_escape_string($link, $clientApplication);
				$query = "SELECT * FROM permission WHERE UId='".$row["UId"]."' AND DomainId='$domainId'";
				$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));
				if (mysqli_num_rows ($result) == 0)
				{
					if ($AcceptUnknownUser)
					{
						// add default permission
						$query = "INSERT INTO permission (UId, DomainId, ShardId, AccessPrivilege) VALUES ('".$row["UId"]."', '$domainId', -1, '$domainStatus')";
						$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

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
					$permission = mysqli_fetch_assoc($result);

					if (!strstr($permission['AccessPrivilege'], $accessPriv))
					{
						// no right to connect
						if ($AcceptUnknownUser)
						{
							// set an additionnal privilege for this player
							$query = "UPDATE permission set AccessPrivilege='".$permission['AccessPrivilege'].",$accessPriv' WHERE PermissionId=".$permission['PermissionId'];
							$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

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
//								$result = mysqli_query ($link, $query) or die ("Can't execute the query: '$query' errno:".mysqli_errno($link).": ".mysqli_error($link));
//
//								$query = "update user set ShardId=-1, State='Offline' where UId=".$row["UId"];
//								$result = mysqli_query ($link, $query) or die ("Can't execute the query: '$query' errno:".mysqli_errno($link).": ".mysqli_error($link));
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
		mysqli_close($link);
		return $res;
	}

	function askSalt($login, $lang)
	{
		global $PHP_SELF;
		global $DBHost, $DBUserName, $DBPassword, $DBName;
		global $AcceptUnknownUser;

		setMsgLanguage($lang);

		$link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die (errorMsgBlock(3004, 'main', $DBHost, $DBUserName));
		mysqli_select_db ($link, $DBName) or die (errorMsgBlock(3005, 'main', $DBName, $DBHost, $DBUserName));

		$login = mysqli_real_escape_string($link, $login);
		$query = "SELECT Password FROM user WHERE Login='$login'";
		$result = mysqli_query ($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

		if (mysqli_num_rows ($result) != 1)
		{
			if ($AcceptUnknownUser)
			{
				// just accept the client and return a default salk
				echo "1:AA";
				die;
			}
			else
			{
				die (errorMsgBlock(2001, $login, 'askSalt'));
				// Check if this is not an unconfirmed account
				/*$query = "SELECT GamePassword, Language FROM signup_data WHERE login='$login'";
				$result = mysqli_query($link, $query) or die (errorMsgBlock(3006, $query, 'main', $DBName, $DBHost, $DBUserName, mysqli_error($link)));

				if (mysqli_num_rows($result) == 0)
				{
					// no user record, reject it
					die (errorMsgBlock(2001, $login, 'askSalt'));
				}
				else if (mysqli_num_rows($result) == 1)
				{
					// one unconfirmed record, let the client send the encrypted password to get the corresponding email address
					$row = mysqli_fetch_assoc($result);
					$salt = substr($row['GamePassword'], 0, 2);
				}
				else
				{
					if ($lang == 'unknown')
					{
						// several matching records => display a multi-language message now
						$languages = array();
						while ($row = mysqli_fetch_assoc($result))
						{
							$languages[$row['Language']] = true;
						}
						setMsgLanguage(array_keys($languages));
					}
					die (errorMsgBlock(2003));
				}*/
			}
		}
		else
		{
			$res_array = mysqli_fetch_assoc($result);
			$salt = get_salt($res_array['Password']);
		}

		echo "1:".$salt;
		mysqli_close($link);
	}

