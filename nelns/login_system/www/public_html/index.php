<?php 

	include_once('../config.php');
	include_once('service_connection.php');

	// The answers here are the "1:.." / "0:.." lines the client parses, and
	// several of them quote the login and the client application back
	// ("Unknown login %s"). With the default html content type a browser
	// renders that, so say what this is and stop it guessing otherwise.
	header('Content-Type: text/plain; charset=utf-8');
	header('X-Content-Type-Options: nosniff');

// ---------------------------------------------------------------------------------------- 
// Functions
// ---------------------------------------------------------------------------------------- 

	function createSalt()
	{
		$chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
			
		return substr($chars, random_int(0, strlen($chars)-1), 1).substr($chars, random_int(0, strlen($chars)-1), 1);
	}

	// $reason contains the reason why the check failed or success
	// return true if the check is ok
	function checkUserValidity ($login, $password, $clientApplication, $cp, &$id, &$reason, &$priv, &$extended)
	{
		global $DBHost, $DBUserName, $DBPassword, $DBName, $AcceptUnknownUser;

		$link = mysql_connect($DBHost, $DBUserName, $DBPassword) or die ("Database unavailable");
		mysql_select_db ($DBName) or die ("Database unavailable");
		$login = mysql_real_escape_string($login);
		$query = "SELECT * FROM user where Login='$login'";
		$result = mysql_query ($query) or die ("Database error");

		if (mysql_num_rows ($result) == 0)
		{
			if ($AcceptUnknownUser)
			{
				if (!$cp)
				{
					// Create a crypted user/pass.
					$password = crypt($password, createSalt());
				}

				// login doesn't exist, create it
				$password = mysql_real_escape_string($password);
				$query = "INSERT INTO user (Login, Password) VALUES ('$login', '$password')";
				$result = mysql_query ($query) or die ("Database error");

				// get the user to have his UId
				$query = "SELECT * FROM user WHERE Login='$login'";
				$result = mysql_query ($query) or die ("Database error");

				if (mysql_num_rows ($result) == 1)
				{
					$reason = "Login '".$login."' was created because it was not found in database (error code 50)";
					$row = mysql_fetch_array ($result);
					$id = $row["UId"];
					$priv = $row["Privilege"];
					$extended = $row["ExtendedPrivilege"];

					// add the default permission
					$query = "INSERT INTO permission (UId,ClientApplication) VALUES ('$id', 'snowballs')";
					$result = mysql_query ($query) or die ("Database error");

					$res = true;
				}
				else
				{
					$reason = "Can't fetch the login '".$login."' (error code 51)";
					$res = false;
				}
			}
			else
			{
				$reason = "Unknown login '".$login."' (error code 52)";
				$res = false;
			}
		}
		else
		{
			$row = mysql_fetch_array ($result);
			$salt = substr($row["Password"],0,2);
			// compare without leaking where the two values stop matching
			$stored = (string)$row["Password"];
			if (($cp && hash_equals($stored, (string)$password)) || (!$cp && hash_equals($stored, (string)crypt($password, $salt))))
			{
				// check if the user can use this application

			$clientApplication = mysql_real_escape_string($clientApplication);
				$query = "SELECT * FROM permission WHERE UId='".$row["UId"]."' AND ClientApplication='$clientApplication'";
				$result = mysql_query ($query) or die ("Database error");
				if (mysql_num_rows ($result) == 0)
				{
					// no permission
					$reason = "You can't use the client application '$clientApplication' (error code 53)";
					$res = false;
				}
				else
				{
					// check if the user not already online

					if ($row["State"] != "Offline")
					{
						$reason = "$login is already online and ";
						// ask the LS to remove the client
						if (disconnectClient ($row["ShardId"], $row["UId"], $tempres))
						{
							$reason =  $reason."was just disconnected. Now you can retry the identification (error code 54)";

							$query = "update shard set NbPlayers=NbPlayers-1 where ShardId=".$row["ShardId"];
							$result = mysql_query ($query) or die ("Database error");

							$query = "update user set ShardId=-1, State='Offline' where UId=".$row["UId"];
							$result = mysql_query ($query) or die ("Database error");
						}
						else
						{
							$reason = $reason."can't be disconnected: $tempres (error code 55)";
						}
						$res = false;
					}
					else
					{
						$id = $row["UId"];
						$priv = $row["Privilege"];
						$extended = $row["ExtendedPrivilege"];
						$res = true;
					}
				}
			}
			else
			{
				$reason = "Bad password (error code 56)";
				$res = false;
			}
		}
		mysql_close($link);
		return $res;
	}

    function checkShardAccess($id, $clientApplication, $shardId)
    {
        global $PHP_SELF;
        global $DBHost, $DBUserName, $DBPassword, $DBName;

        $link = mysql_connect($DBHost, $DBUserName, $DBPassword) or die ("0:Database unavailable");
        mysql_select_db ($DBName) or die ("0:Database unavailable");

        $id = mysql_real_escape_string($id);
        $clientApplication = mysql_real_escape_string($clientApplication);
        $shardId = mysql_real_escape_string($shardId);
        $query = "SELECT * FROM permission WHERE UId='".$id."' AND ClientApplication='".$clientApplication."' AND (ShardId='".$shardId."' OR ShardId='-1')";;
        $result = mysql_query ($query) or die ("0:Database error");

        if (mysql_num_rows ($result) > 0)
        {
            mysql_close($link);
            return;
        }
        mysql_close($link);
        die("0:Invalid shard access");
    }

	function displayAvailableShards($id, $clientApplication, $multiplePatchers)
	{
		global $PHP_SELF;
		global $DBHost, $DBUserName, $DBPassword, $DBName;

		$link = mysql_connect($DBHost, $DBUserName, $DBPassword) or die ("0:Database unavailable");
		mysql_select_db ($DBName) or die ("0:Database unavailable");
		
		$id = mysql_real_escape_string($id);
		$clientApplication = mysql_real_escape_string($clientApplication);
		$query = "SELECT * FROM user WHERE UId='".$id."'";
		$result = mysql_query ($query) or die ("0:Database error");

		if ($result)
			$uData = mysql_fetch_array($result);
			
		if (strstr($uData['Privilege'], ':DEV:'))
			$priv = 'dev';
		else if (strlen($uData['Privilege']) > 0)
			$priv = 'gm';
		else
			$priv = '';

		$query = "SELECT * FROM shard WHERE ClientApplication='".$clientApplication."'";
		$result = mysql_query ($query) or die ("0:Database error");
		
		$nbs = 0;
		$res = "";
		if (mysql_num_rows ($result) > 0)
		{
			//echo "<h1>Please, select a shard:</h1>\n";
			while($row = mysql_fetch_array($result))
			{
				$query2 = "SELECT * FROM permission WHERE UId='".$id."' AND ClientApplication='".$clientApplication."' AND ShardId='".$row["ShardId"]."'";
				$result2 = mysql_query ($query2) or die ("Database error");
				
				$online = $row["Online"];
				$uOnline = 1;

				switch ($online)
				{
					case 0:
						$uOnline = 0;
						break;
					case 1:
						$uOnline = ($priv == 'dev' ? 1 : 2);
						break;
					case 2:
						$uOnline = (($priv == 'dev' || $priv == 'gm') ? 1 : 2);
						break;
					default:
						$uOnline = 1;
						break;
				}

				// only display the shard if the user have the good application name AND access to this shard with the permission table
				if (mysql_num_rows ($result2) > 0 && $row["ProgramName"] == $programName)
				{
					$nbs++;
					$res = $res.$row["Version"]."|";
					$res = $res.$uOnline."|";
					$res = $res.$row["ShardId"]."|";
					$res = $res.$row["Name"]."|";
					$res = $res."999999|";
					$res = $res.$row["WsAddr"]."|";
					$res = $res.$row["PatchURL"];
					if (strlen($row["DynPatchURL"]) > 0 && $multiplePatchers)
						$res = $res."|".$row["DynPatchURL"];
					$res = $res."\n";
				}
			}
		}

		echo "1:".$nbs."\n";
		echo $res;
		mysql_close($link);
		return $res;
	}

	function askSalt($login)
	{
		global $PHP_SELF;
		global $DBHost, $DBUserName, $DBPassword, $DBName, $AcceptUnknownUser;

		$link = mysql_connect($DBHost, $DBUserName, $DBPassword) or die ("0:Database unavailable");
		mysql_select_db ($DBName) or die ("0:Database unavailable");

		$login = mysql_real_escape_string($login);
		$query = "SELECT Password FROM user WHERE Login='$login'";
		$result = mysql_query ($query) or die ("0:Database error");

		if (mysql_num_rows ($result) != 1)
		{
			if ($AcceptUnknownUser)
			{
				$salt = createSalt();
			}
			else
			{
				die ("0:Unknown login $login (error code 64)");
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

// --------------------------------------------------------------------------------------
// main 
// --------------------------------------------------------------------------------------

	if ($_GET["cmd"] == "ask")
	{
		askSalt($_GET["login"]);
		die();
	}

	// check cp is set (force bool)
	$cp = ($_GET["cp"] == "1");

	if (!checkUserValidity($_GET["login"], $_GET["password"], $_GET["clientApplication"], $cp, $id, $reason, $priv, $extended))
	{
		echo "0:".$reason;
	}
	else
	{
		if ($_GET["cmd"] == "login")
		{
			checkShardAccess($id, $_GET["clientApplication"], $_GET["shardid"]);

			// user selected a shard, try to add the user to the shard

			if (askClientConnection($_GET["shardid"], $id, $_GET["login"], $priv, $extended, $res, $patchURLS))
			{
				// access granted, send cookie and addr
				echo "1:".$res;

				// LS sent patching URLS? Add them at the end of the string
				if (strlen($patchURLS) > 0)
					echo ' '.$patchURLS;

/*
				// OBSOLETE: emergency patch URI already sent at displayAvailableShards - no need to add it
				// There is a default patching address? Add it at the end of the patching URLS
				$query = "SELECT PatchURL FROM shard WHERE ShardId='$shardid'";
				$result = mysql_query($query);
				if ($result && ($array=mysql_fetch_array($result)))
				{
					$patchURL = $array['PatchURL'];
					if (strlen($patchURL) > 0)
					{
						echo (strlen($patchURLS) > 0 ? '|' : ' ').$patchURL;
					}
				}
*/
			}
			else
			{
				// access denied, display why
				echo "0:".$res;
			}
		}
		else
		{
			// user logged, display the available shard
			displayAvailableShards ($id, $_GET["clientApplication"], $cp);
		}
	}
?>
