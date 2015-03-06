<?php
	require_once('../tools/validate_cookie.php');
	include_once('../login/config.php');
	include_once('../tools/domain_info.php');
	include_once('ring_session_manager_itf.php');

class JoinShardCb extends CRingSessionManagerWeb
{
	/**
	 * Join the specified shard
	 */
	function joinSessionResult($userId, $sessionId, $result, $shardAddr, $participantStatus)
	{
		global $JoinSessionResultCode, $JoinSessionResultMsg;
		$JoinSessionResultCode = $result;
		$JoinSessionResultMsg = $shardAddr;
		if ($result != 0)
		{
			global $FSHostLuaMode, $FSHostResult, $FSHostResultStr;
			$FSHostResult = 0;
			$FSHostResultStr = "Error ".$result." : '".$shardAddr."' while trying to join a shard";
			if ($FSHostLuaMode)
			{
				echo $FSHostResultStr;
				echo '<p><p><a href="web_start.php">Back to menu</a>';
			}
		}
		else
		{
			// ok, we have the info to connect !
			// generate the lua script
			$cookie=convertCookieForActionHandler($_COOKIE["ryzomId"]);
			global $FSHostLuaMode, $FSHostResult, $FSHostResultStr;
			$FSHostResult = 1;
			$FSHostResultStr = $shardAddr;
			if ($FSHostLuaMode)
			{
				$luaScript='runAH(nil, "on_connect_to_shard", "cookie='.$cookie.'|fsAddr='.$shardAddr.'")';
				//echo 'luaScript : '.$luaScript.'<br>';
				echo '<lua>'.$luaScript.'</lua>';
				global $verbose;
				if ($verbose)
					echo '<br>Teleporting to shard<br>';
			}
		}
	}

	/**
	 * Receive the result of the shard list, then call the global $getShardListCallback with the associative array as argument
	 */
	function getShardsResult($userId, $resultStr)
	{
		global $getShardListCallback;

		$onlineShardsBySessionId = array();
		$resultArray = explode(';', $resultStr);
		foreach ($resultArray as $shardInfo)
		{
			$shardAttr = explode(',', $shardInfo);
			if (isset($shardAttr[1]))
				$onlineShardsBySessionId[$shardAttr[0]] = $shardAttr[1];
		}
		$getShardListCallback($onlineShardsBySessionId);
	}
}

// External use
$FSHostLuaMode = false;
$FSHostResultStr = 0;

// Internal use
$verbose = true;
$FSHostResult = 0;

// THE ABOVE DECLARATIONS ARE NEEDED FOR ALL CODE BELOW THIS LINE
if (isset($_POST["ml"]))
	joinMainlandPost();
if (isset($_POST["destSessionId"]))
	joinShardFromIdPost($_POST["destSessionId"]);

/**
 * Authenticate and request to join a mainland shard
 */
function joinMainlandPost()
{
	global $FSHostLuaMode;
	$FSHostLuaMode = true;
	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}
	else
	{
		joinMainland($userId, $domainId);
	}
}

/**
 * Authenticate and request to join the specified shard
 */
function joinShardFromIdPost( $destSessionId )
{
	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}
	else
	{
		joinShardFromId($userId, $domainId, $destSessionId);
	}
}

/**
 * Request to join the specified shard
 */
function joinShardFromId( $userId, $domainId, $destSessionId )
{
	$domainInfo = getDomainInfo($domainId);
	$addr = explode(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];

	// request join to the session manager
	$joinShard = new JoinShardCb;
	$res = "";
	$joinShard->connect($RSMHost, $RSMPort, $res);
	$charSlot = getCharSlot(); // if ingame (!=15), the RSM can check if this character has the right to connect to the specified shard
	$charId = ($userId<<4) + $charSlot;
	echo "Requesting teleportation of $charId/$userId to shard session ".$destSessionId."...<br>";
	$joinShard->joinSession($charId, $destSessionId, $domainInfo["domain_name"]);

	// wait the the return message
	if ($joinShard->waitCallback() == false)
	{
		// Note: the answer is a joinSessionResult message
		echo "No response from server, joinShard failed<br>";
	}
	die();
}

/**
 * Request to get the shard list
 */
function getShardList($userId, $domainId)
{
	$domainInfo = getDomainInfo($domainId);
	$addr = explode(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];

	// request get to the session manager
	$joinShard = new JoinShardCb;
	$res = "";
	$joinShard->connect($RSMHost, $RSMPort, $res);

	$charId = ($userId<<4)+15;
	echo "Retrieving online shards for $charId...<br>";
	$joinShard->getShards($charId);

	// wait the the return message
	if ($joinShard->waitCallback() == false)
	{
		echo "No response from server, getShards failed<br>";
	}
	die();
}

/**
 * Display the list of mainland shards
 * This is a candidate to the callback $getShardListCallback
 */
function displayAllShards(&$onlineShardsBySessionId)
{
	// Get the userId and domainId back
	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}

	// List all shards of the domain, including offline ones
	global $DBName, $DBHost, $DBUserName, $DBPassword;
	$link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die("Can't connect to nel database");
	mysqli_select_db($link, $DBName) or die ("Can't access to the db dbname:$DBName");

	$domainId = (int) $domainId;
	$query = "select * from shard where domain_id = $domainId";
	$resShards = mysqli_query($link, $query) or die ("Can't execute the query: ".$query." ".mysqli_error($link));

	echo "Select a shard to join:<br>";
	//echo "<form name='far_tp' action='join_shard.php' method='post'>";
	while ($rowShard = mysqli_fetch_assoc($resShards))
	{
		$mainlandSessionId = $rowShard['FixedSessionId'];
		$isOnline = isset($onlineShardsBySessionId[$mainlandSessionId]);
		// Radio button not supported by Client's html component. Instead: one form (button) per shard.
		//echo "<input type='radio' name='destSessionId' value='".$mainlandSessionId."' ".($isOnline?"":"disabled ")."/>".$rowShard['Name']." (".($isOnline?"online with $nbOnlinePlayers players":"offline").", version ".$rowShard['Version'].")<br>";
		echo "<form name='far_tp_".$rowShard['ShardId']."' action='join_shard.php' method='post'>";
		echo "<input type='hidden' name='destSessionId' value='".$mainlandSessionId."' />";
		echo "<input type='hidden' name='charSlot' value='".getCharSlot()."'>";
		echo " ".$rowShard['Name']." ".$rowShard['ShardId']." (".($isOnline ? $onlineShardsBySessionId[$mainlandSessionId]." online)" : "offline)");
		if ($isOnline)
			echo "<input type='submit' name='button' value='Teleport' />";
		echo "</form><br>";
	}

	//echo "<input type='submit' name='button' value='Teleport' />";
	//echo "</form></font>";
	die();
}


/**
 * Auto-join a shard.
 * Returns true in case of success, and the FSHost address in $FSHostResultStr.
 * Returns false in case of error, and an explanatory string in $FSHostResultStr.
 */
function joinMainland($userId, $domainId)
{
	$domainInfo = getDomainInfo($domainId);
	$addr = explode(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];

	// request get to the session manager
	$joinsShard = new JoinShardCb;
	$res = "";
	$joinsShard->connect($RSMHost, $RSMPort, $res);

	// set the shard Id to allow any character (by using the special value '15')
	$charId = ($userId<<4)+15;
	global $FSHostLuaMode, $verbose;
	if ($FSHostLuaMode && $verbose)
		echo "Joining a mainland shard for $charId...<br>";
	$joinsShard->joinMainland($charId, $domainInfo["domain_name"]);

	// wait the the return message
	if ($joinsShard->waitCallback() == false)
	{
		global $FSHostResultStr;
		$FSHostResultStr = "No response from server for joinMainland<br>";
		return false;
	}
	global $FSHostResult;
	return $FSHostResult;
}

