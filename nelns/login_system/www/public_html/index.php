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

		$link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die ("Database unavailable");
		mysqli_select_db ($link, $DBName) or die ("Database unavailable");
		$login = mysqli_real_escape_string($link, $login);
		$query = "SELECT * FROM user where Login='$login'";
		$result = mysqli_query ($link, $query) or die ("Database error");

		if (mysqli_num_rows ($result) == 0)
		{
			if ($AcceptUnknownUser)
			{
				if (!$cp)
				{
					// Create a crypted user/pass.
					$password = crypt($password, createSalt());
				}

				// login doesn't exist, create it
				$password = mysqli_real_escape_string($link, $password);
				$query = "INSERT INTO user (Login, Password) VALUES ('$login', '$password')";
				$result = mysqli_query ($link, $query) or die ("Database error");

				// get the user to have his UId
				$query = "SELECT * FROM user WHERE Login='$login'";
				$result = mysqli_query ($link, $query) or die ("Database error");

				if (mysqli_num_rows ($result) == 1)
				{
					$reason = "Login '".$login."' was created because it was not found in database (error code 50)";
					$row = mysqli_fetch_array ($result);
					$id = $row["UId"];
					$priv = $row["Privilege"];
					$extended = $row["ExtendedPrivilege"];

					// add the default permission
					$query = "INSERT INTO permission (UId,ClientApplication) VALUES ('$id', 'snowballs')";
					$result = mysqli_query ($link, $query) or die ("Database error");

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
			$row = mysqli_fetch_array ($result);
			$salt = substr($row["Password"],0,2);
			// compare without leaking where the two values stop matching
			$stored = (string)$row["Password"];
			if (($cp && hash_equals($stored, (string)$password)) || (!$cp && hash_equals($stored, (string)crypt($password, $salt))))
			{
				// check if the user can use this application

			$clientApplication = mysqli_real_escape_string($link, $clientApplication);
				$query = "SELECT * FROM permission WHERE UId='".$row["UId"]."' AND ClientApplication='$clientApplication'";
				$result = mysqli_query ($link, $query) or die ("Database error");
				if (mysqli_num_rows ($result) == 0)
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
							$result = mysqli_query ($link, $query) or die ("Database error");

							$query = "update user set ShardId=-1, State='Offline' where UId=".$row["UId"];
							$result = mysqli_query ($link, $query) or die ("Database error");
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
		mysqli_close($link);
		return $res;
	}

    function checkShardAccess($id, $clientApplication, $shardId)
    {
        global $PHP_SELF;
        global $DBHost, $DBUserName, $DBPassword, $DBName;

        $link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die ("0:Database unavailable");
        mysqli_select_db ($link, $DBName) or die ("0:Database unavailable");

        $id = mysqli_real_escape_string($link, $id);
        $clientApplication = mysqli_real_escape_string($link, $clientApplication);
        $shardId = mysqli_real_escape_string($link, $shardId);
        $query = "SELECT * FROM permission WHERE UId='".$id."' AND ClientApplication='".$clientApplication."' AND (ShardId='".$shardId."' OR ShardId='-1')";;
        $result = mysqli_query ($link, $query) or die ("0:Database error");

        if (mysqli_num_rows ($result) > 0)
        {
            mysqli_close($link);
            return;
        }
        mysqli_close($link);
        die("0:Invalid shard access");
    }

	function displayAvailableShards($id, $clientApplication, $multiplePatchers)
	{
		global $PHP_SELF;
		global $DBHost, $DBUserName, $DBPassword, $DBName;

		$link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die ("0:Database unavailable");
		mysqli_select_db ($link, $DBName) or die ("0:Database unavailable");

		$id = mysqli_real_escape_string($link, $id);
		$clientApplication = mysqli_real_escape_string($link, $clientApplication);
		$query = "SELECT * FROM user WHERE UId='".$id."'";
		$result = mysqli_query ($link, $query) or die ("0:Database error");

		if ($result)
			$uData = mysqli_fetch_array($result);
			
		if (strstr($uData['Privilege'], ':DEV:'))
			$priv = 'dev';
		else if (strlen($uData['Privilege']) > 0)
			$priv = 'gm';
		else
			$priv = '';

		$query = "SELECT * FROM shard WHERE ClientApplication='".$clientApplication."'";
		$result = mysqli_query ($link, $query) or die ("0:Database error");

		$nbs = 0;
		$res = "";
		if (mysqli_num_rows ($result) > 0)
		{
			//echo "<h1>Please, select a shard:</h1>\n";
			while($row = mysqli_fetch_array($result))
			{
				$query2 = "SELECT * FROM permission WHERE UId='".$id."' AND ClientApplication='".$clientApplication."' AND ShardId='".$row["ShardId"]."'";
				$result2 = mysqli_query ($link, $query2) or die ("Database error");
				
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

				// only display the shard if the user has access to it in the
				// permission table (a ProgramName filter used to sit here, but
				// it compared against a variable that was never set, and no
				// schema in this tree carries that column)
				if (mysqli_num_rows ($result2) > 0)
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
		mysqli_close($link);
		return $res;
	}

	function askSalt($login)
	{
		global $PHP_SELF;
		global $DBHost, $DBUserName, $DBPassword, $DBName, $AcceptUnknownUser;

		$link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die ("0:Database unavailable");
		mysqli_select_db ($link, $DBName) or die ("0:Database unavailable");

		$login = mysqli_real_escape_string($link, $login);
		$query = "SELECT Password FROM user WHERE Login='$login'";
		$result = mysqli_query ($link, $query) or die ("0:Database error");

		if (mysqli_num_rows ($result) != 1)
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
			$res_array = mysqli_fetch_array($result);
			$salt = substr($res_array['Password'], 0, 2);
		}

		echo "1:".$salt;
		mysqli_close($link);
	}

// --------------------------------------------------------------------------------------
// main 
// --------------------------------------------------------------------------------------

	// every request parameter is optional as far as PHP is concerned:
	// default the missing ones instead of tripping undefined-index notices
	$cmd = isset($_GET["cmd"]) ? $_GET["cmd"] : "";
	$in_login = isset($_GET["login"]) ? $_GET["login"] : "";
	$in_password = isset($_GET["password"]) ? $_GET["password"] : "";
	$in_clientApplication = isset($_GET["clientApplication"]) ? $_GET["clientApplication"] : "";
	$in_shardid = isset($_GET["shardid"]) ? $_GET["shardid"] : "";

	if ($cmd == "ask")
	{
		askSalt($in_login);
		die();
	}

	// check cp is set (force bool)
	$cp = (isset($_GET["cp"]) && $_GET["cp"] == "1");

	if (!checkUserValidity($in_login, $in_password, $in_clientApplication, $cp, $id, $reason, $priv, $extended))
	{
		echo "0:".$reason;
	}
	else
	{
		if ($cmd == "login")
		{
			checkShardAccess($id, $in_clientApplication, $in_shardid);

			// user selected a shard, try to add the user to the shard

			if (askClientConnection($in_shardid, $id, $in_login, $priv, $extended, $res, $patchURLS))
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
			displayAvailableShards ($id, $in_clientApplication, $cp);
		}
	}
?>
