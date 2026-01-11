<?php
include_once('../tools/validate_cookie.php');
include_once('ring_session_manager_itf.php');
include_once('../tools/domain_info.php');
include_once('../login/config.php');

function planEditSession($charId, $domainId, $sessionType, $title, $desc)
{
	$domainInfo = getDomainInfo($domainId);
	$addr = split(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];

	$rsm = new ScheduleSessionCb();
	$res="";
	$rsm->connect($RSMHost, $RSMPort, $res);
	if ($res != "")
	{
		echo "Error connecting to session manager<br>";
		echo '<a href="web_start.php">Return to start menu</a>';
		die();
	}

	// send the create session message
	$rsm->scheduleSession(
		$charId, 
		$sessionType,
		$title, 
		0,
		$desc, 
		"sl_a", 
		"at_public", 
		"rt_liberal", 
		"et_short", 
		0			// 0 inscription slots for edit session
		);
	
	$rsm->waitCallback();
	// the rest of the work is done in the callback
}

$SessionId = 0;
$DomainId = 0;
$SessionToolsResult = false;

class ScheduleSessionCb extends CRingSessionManagerWeb
{
	function scheduleSessionResult($charId, $sessionId, $result, $resultString)
	{
		global $SessionId, $DomainId, $SessionToolsResult;
		$SessionId = $sessionId;
//		$DomainId = $domainId;
		echo "Create session result :<br>";
		if ($result == 0)
		{
			$SessionToolsResult = true;
			echo "Session $sessionId created for char $charId<br>";
			echo "<h2>Your session has been planned, thank you<h2><br>";
		}
		else
		{
			$SessionToolsResult = false;
			echo "Failed to create a session for char $charId with error $resultString <br>";
		}
	}
}

$rsmProxy = false;
$callbackClient = false;
$rsmSkel = false;

function startSession($charId, $domainId, $sessionId)
{
	global $SessionId, $DomainId, $SessionToolsResult;

	$domainInfo = getDomainInfo($domainId);
	$addr = split(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];

	$SessionId = $sessionId;
	$DomainId = $domainId;
	
	// ask to start the session
	global $rsmProxy, $callbackClient, $rsmSkel;
	$startSession = new StartSessionCb;
	$res = "";
	$startSession->connect($RSMHost, $RSMPort, $res);
	echo "Starting session for character ".$charId." in session ".$sessionId."<br>";
	global $SessionId;
	$SessionId = $sessionId;
	$startSession->startSession($charId, $sessionId);
	
	// wait the the return message
	$startSession->waitCallback() or die("No reponse from session manager");
}

function inviteOwnerInSession($charId, $domainId, $sessionId)
{
	global $SessionId, $DomainId, $SessionToolsResult;

	// first, set the result to false
	$SessionToolsResult = false;

	$domainInfo = getDomainInfo($domainId);
	$addr = split(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];
	
	global $rsmProxy, $rsmSkel, $userId, $charId, $callbackClient, /*$SessionId,*/ $SessionToolsResult;
	global $DBHost, $RingDBUserName, $RingDBPassword;

	$SessionId = $sessionId;
	$DomainId = $domainId;

	$link = mysqli_connect($DBHost, $RingDBUserName, $RingDBPassword) or die("Can't connect to ring database");
	mysqli_select_db($link, $domainInfo['ring_db_name']) or die ("Can't access to the db dbname:" . $domainInfo['ring_db_name']);

	$sessionId = (int) $sessionId;
	$query = "select session_type from sessions where session_id=".$sessionId;
	$result = mysqli_query($link, $query) or die ("Can't execute the query: ".$query);
	if (mysqli_num_rows($result) != 1)
	{
		echo "Can't find 1 row for ring session ".$sessionId."<br>";
		die();
	}
	$row = mysqli_fetch_row($result);
	$session_type = $row[0];
	$mode = ($session_type == "st_edit") ? "sps_edit_invited" : "sps_anim_invited";
	echo "Inviting character ".$charId." of user ".$userId." in session ".$sessionId."<br>";

	$inviteOwner = new InviteOwnerCb;
	$res = "";
	$inviteOwner->connect($RSMHost, $RSMPort, $res);
	$inviteOwner->inviteCharacter($charId, $sessionId, $charId, $mode);

	// wait the the return message
	if ($inviteOwner->waitCallback() == false)
	{
		echo "No response from server, invite failed<br>";
		die();
	}
}

class StartSessionCb extends CRingSessionManagerWeb
{
	function invokeResult($userId, $resultCode, $resultString)
	{
		global $rsmProxy, $rsmSkel, $userId, $charId, $callbackClient, $SessionId, $DomainId, $SessionToolsResult;
		
		if ($resultCode != 0)
		{
			$SessionToolsResult = false;
			echo "<h1>Error ".$resultCode." : '".$resultString."' while trying to start the session ".$SessionId."</h1>";
			echo '<p><p><a href="web_start.php">Back to menu</a>';
		}
		else
		{
			// ok, the session is started, invite the session owner in the session
			$SessionToolsResult = false;
			
			echo "Start of session $SessionId success, now inviting character $charId in the sesison<br>";
			
			inviteOwnerInSession($charId, $DomainId, $SessionId);
		}
	}
}

class InviteOwnerCb extends CRingSessionManagerWeb
{
	function invokeResult($userId, $resultCode, $resultString)
	{
		global $rsmProxy, $rsmSkel, $userId, $charId, $callbackClient, $SessionId, $DomainId, $SessionToolsResult;
		
		// jump back to main page
		echo "<h1>The session ".$SessionId." have been started</h1>";
		if ($resultCode == 0)
		{
			$SessionToolsResult = true;
			echo "<h1>You are automaticaly invited in the session</h1>";
		}
		else
		{
			$SessionToolsResult = false;
			echo "<h1>Failed to invite you in the started session !</h1>";
			echo "<h1>Error ".$resultCode." : '".$resultString."' while trying to join the session ".$SessionId."</h1>";
		}
		echo '<p><p><a href="web_start.php">Back to menu</a>';
	}
}

