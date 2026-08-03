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

	function connectDb($errPrefix)
	{
		global $DBHost, $DBPort, $DBUserName, $DBPassword, $DBName;
		$port = isset($DBPort) ? (int)$DBPort : 0;
		$link = mysqli_connect($DBHost, $DBUserName, $DBPassword, NULL, $port) or die ($errPrefix."Database unavailable");
		mysqli_select_db ($link, $DBName) or die ($errPrefix."Database unavailable");
		return $link;
	}

	// The permission and shard tables hang off the domain table now: the
	// client application names the domain (domain.domain_name), same
	// convention the ring login uses.
	function resolveDomainId($link, $clientApplication, $errPrefix)
	{
		$app = mysqli_real_escape_string($link, $clientApplication);
		$result = mysqli_query ($link, "SELECT domain_id FROM domain WHERE domain_name='$app'") or die ($errPrefix."Database error");
		if (mysqli_num_rows($result) != 1)
			die ($errPrefix."Unknown client application '".$clientApplication."' (error code 65)");
		$row = mysqli_fetch_row($result);
		return (int)$row[0];
	}

	// $reason contains the reason why the check failed or success
	// return true if the check is ok
	function checkUserValidity ($login, $password, $clientApplication, $cp, &$id, &$reason, &$priv, &$extended)
	{
		global $AcceptUnknownUser;

		$link = connectDb("");
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
				// FIXME: same trap as the ring login: nel.user.Email is
				// `UNIQUE NOT NULL DEFAULT ''`, so only the first account ever
				// created this way inserts; the next one collides on the empty
				// email. Write a unique placeholder or relax EmailIndex before
				// leaning on $AcceptUnknownUser beyond a single dev account.
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

					// add the default permission for the domain the client
					// asked for (ShardId -1 means any shard of the domain)
					$domainId = resolveDomainId($link, $clientApplication, "");
					$query = "INSERT INTO permission (UId,DomainId) VALUES ('$id', '$domainId')";
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
			$stored = (string)$row["Password"];
			$password = (string)$password;
			$salt = substr($stored, 0, 2);
			// An account row with no password must never authenticate: with
			// cp set the client sends the crypted password and the check is
			// a plain comparison, so an empty column is matched by an empty
			// password field. nel.user.Password is nullable, so such rows
			// can exist. Two characters is also the least crypt() needs for
			// its salt to mean anything.
			if (strlen($stored) < 2 || $password === '')
			{
				$reason = "Bad password (error code 56)";
				$res = false;
			}
			// compare without leaking where the two values stop matching
			elseif (($cp && hash_equals($stored, $password)) || (!$cp && hash_equals($stored, (string)crypt($password, $salt))))
			{
				// check if the user can use this application

			$domainId = resolveDomainId($link, $clientApplication, "");
				$query = "SELECT * FROM permission WHERE UId='".$row["UId"]."' AND DomainId='$domainId'";
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

        $link = connectDb("0:");

        $id = mysqli_real_escape_string($link, $id);
        $domainId = resolveDomainId($link, $clientApplication, "0:");
        $shardId = mysqli_real_escape_string($link, $shardId);
        $query = "SELECT * FROM permission WHERE UId='".$id."' AND DomainId='".$domainId."' AND (ShardId='".$shardId."' OR ShardId='-1')";
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

		$link = connectDb("0:");

		$id = mysqli_real_escape_string($link, $id);
		$domainId = resolveDomainId($link, $clientApplication, "0:");
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

		// the patch url moved from the shard row to the domain row
		$result = mysqli_query ($link, "SELECT patch_urls, backup_patch_url FROM domain WHERE domain_id='".$domainId."'") or die ("0:Database error");
		$domainRow = mysqli_fetch_assoc($result);
		$patchURL = "";
		if ($domainRow)
		{
			$patchURL = trim((string)$domainRow['patch_urls']);
			if ($patchURL == "")
				$patchURL = trim((string)$domainRow['backup_patch_url']);
		}

		$query = "SELECT * FROM shard WHERE domain_id='".$domainId."'";
		$result = mysqli_query ($link, $query) or die ("0:Database error");

		$nbs = 0;
		$res = "";
		if (mysqli_num_rows ($result) > 0)
		{
			//echo "<h1>Please, select a shard:</h1>\n";
			while($row = mysqli_fetch_array($result))
			{
				// same rule as checkShardAccess: a ShardId of -1 in the
				// permission row grants every shard of the domain
				$query2 = "SELECT * FROM permission WHERE UId='".$id."' AND DomainId='".$domainId."' AND (ShardId='".$row["ShardId"]."' OR ShardId='-1')";
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
					$res = $res.$patchURL;
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
		global $AcceptUnknownUser;

		$link = connectDb("0:");

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
			$stored = (string)$res_array['Password'];
			// An empty salt tells the caller which accounts carry no
			// password; hand out a random one instead, the same answer an
			// unknown login gets, and let checkUserValidity refuse.
			$salt = strlen($stored) >= 2 ? substr($stored, 0, 2) : createSalt();
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
