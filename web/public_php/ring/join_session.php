<?php
	require_once('../tools/validate_cookie.php');
	include_once('../login/config.php');
	include_once('../tools/domain_info.php');
	include_once('ring_session_manager_itf.php');


class JoinSessionCb extends CRingSessionManagerWeb
{
	/**
	 * Join the specified session
	 */
	function joinSessionResult($userId, $sessionId, $result, $shardAddr, $participantStatus)
	{
		if ($result != 0)
		{
			echo "<h1>Error ".htmlspecialchars($result, ENT_QUOTES)." : '".htmlspecialchars($shardAddr, ENT_QUOTES)."' while trying to join a session </h1>";
			echo '<p><p><a href="web_start.php">Back to menu</a>';
		}
		else
		{
			// ok, we have the info to connect !
			// generate the lua script
			$cookie=convertCookieForActionHandler($_COOKIE["ryzomId"]);
			$luaScript='runAH(nil, "on_connect_to_shard", "cookie='.$cookie.'|fsAddr='.$shardAddr.'")';
			//echo 'luaScrip : '.$luaScript.'<br>';
			echo '<lua>'.$luaScript.'</lua>';

			echo 'You are allowed in the session <br>';
		}
	}
}


if (isset($_POST["sessionId"]))
	joinSessionFromIdPost($_POST["sessionId"]);
			
/**
 * Authenticate and request to join the specified shard
 */
function joinSessionFromIdPost( $destSessionId )
{
	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}
	else
	{
		joinSessionFromId($userId, $domainId, $destSessionId);
	}
}

/**
 * Request to join the specified shard
 */
function joinSessionFromId( $userId, $domainId, $destSessionId )
{
	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}
	else
	{
		echo "Welcome user ".htmlspecialchars($userId, ENT_QUOTES)."<BR>";
		
		$domainInfo = getDomainInfo($domainId);
		$addr = explode(":", $domainInfo["session_manager_address"]);
		$RSMHost = $addr[0];
		$RSMPort = $addr[1];
		
		// ask join to the session manager
		$joinSession = new JoinSessionCb;
		$res = "";
		$joinSession->connect($RSMHost, $RSMPort, $res);
//		$rsmProxy = new CRingSessionManagerWebProxy;

//		$charSlot = getCharSlot(); // if ingame (!=15), the RSM will check if this character has the right to connect to the specified session
//		$charId = ($userId<<4) + $charSlot;
		echo htmlspecialchars($charId, ENT_QUOTES)." of user ".htmlspecialchars($userId, ENT_QUOTES)." joigning session ".htmlspecialchars($destSessionId, ENT_QUOTES)."<br>";
		$joinSession->joinSession($charId, $destSessionId, $domainInfo["domain_name"]);
		
		// wait the the return message
//		$rsmSkel = new CRingSessionManagerWebSkel;
		if ($joinSession->waitCallback() == false)
		{
			echo "No response from server, joinSession failed<br>";
		}
		
		die();
	}
}

?>

