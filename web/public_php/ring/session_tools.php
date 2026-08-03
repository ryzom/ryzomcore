<?php
include_once('../tools/validate_cookie.php');
include_once('ring_session_manager_itf.php');
include_once('../tools/domain_info.php');
include_once('../login/config.php');

// The generated enum classes carry the sequential values the C++ enums
// use. Two message fields have no such class: R2::TSessionLevel starts
// at 1 instead of 0 (r2_share_itf.h) and the filter parameters are
// bitsets (CEnumBitset) where 0 means "no filter". This carries their
// raw wire value through serialEnum.
class CRawEnumValue
{
	var $Value;

	function __construct($value)
	{
		$this->Value = $value;
	}

	function toString() { return (string)$this->Value; }
	function toInt() { return $this->Value; }
	function fromInt($intValue) { $this->Value = $intValue; }
}

function planEditSession($charId, $domainId, $sessionType, $title, $desc)
{
	// the session type arrives with the request: refuse anything that is
	// not one of the two schedulable types before it goes on the wire
	if ($sessionType != "st_edit" && $sessionType != "st_anim")
	{
		echo "Invalid session type<br>";
		echo '<a href="web_start.php">Return to start menu</a>';
		die();
	}

	$domainInfo = getDomainInfo($domainId);
	$addr = explode(":", $domainInfo["session_manager_address"]);
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

	$st = new RSMGR_TSessionType;
	$st->fromString($sessionType);
	// mirror the defaults the client uses when scheduling from the game
	// (far_tp.cpp for st_edit, connection.cpp for st_anim)
	$ruleType = new RSMGR_TRuleType;
	$ruleType->fromString($sessionType == "st_edit" ? "rt_strict" : "rt_liberal");
	$duration = new RSMGR_TEstimatedDuration;
	$duration->fromString($sessionType == "st_edit" ? "et_long" : "et_medium");
	$animMode = new RSMGR_TAnimMode;
	$animMode->fromString("am_dm");
	$orientation = new RSMGR_TSessionOrientation;
	$orientation->fromString("so_other");

	// send the create session message
	$rsm->scheduleSession(
		$charId,
		$st,
		$title,
		$desc,
		new CRawEnumValue(1),	// R2::TSessionLevel sl_a
		$ruleType,
		$duration,
		0,			// no inscription slots
		$animMode,
		new CRawEnumValue(0),	// race filter: empty bitset, no filtering
		new CRawEnumValue(0),	// religion filter
		new CRawEnumValue(0),	// guild filter
		new CRawEnumValue(0),	// shard filter
		new CRawEnumValue(0),	// level filter
		"",			// language
		$orientation,
		0,			// subscription open
		1			// auto invite, gives the session public access
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
			echo "Session ".htmlspecialchars($sessionId, ENT_QUOTES)." created for char ".htmlspecialchars($charId, ENT_QUOTES)."<br>";
			echo "<h2>Your session has been planned, thank you<h2><br>";
		}
		else
		{
			$SessionToolsResult = false;
			echo "Failed to create a session for char ".htmlspecialchars($charId, ENT_QUOTES)." with error ".htmlspecialchars($resultString, ENT_QUOTES)." <br>";
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
	$addr = explode(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];

	// the session id arrives with the request on some call paths
	$sessionId = intval($sessionId);

	$SessionId = $sessionId;
	$DomainId = $domainId;

	// ask to start the session
	global $rsmProxy, $callbackClient, $rsmSkel;
	$startSession = new StartSessionCb;
	$res = "";
	$startSession->connect($RSMHost, $RSMPort, $res);
	echo "Starting session for character ".htmlspecialchars($charId, ENT_QUOTES)." in session ".$sessionId."<br>";
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
	$addr = explode(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];
	
	global $rsmProxy, $rsmSkel, $userId, $charId, $callbackClient, /*$SessionId,*/ $SessionToolsResult;
	global $DBHost, $DBPort, $RingDBUserName, $RingDBPassword;

	$SessionId = $sessionId;
	$DomainId = $domainId;

	$link = mysqli_connect($DBHost, $RingDBUserName, $RingDBPassword, NULL, $DBPort) or die("Can't connect to ring database");
	if (function_exists('nel_mysqli_set_charset'))
		nel_mysqli_set_charset($link);
	mysqli_select_db($link, $domainInfo['ring_db_name']) or die ("Can't access to the db");

	$sessionId = (int) $sessionId;
	$query = "select session_type from sessions where session_id=".$sessionId;
	$result = mysqli_query($link, $query) or die ("Can't execute the query");
	if (mysqli_num_rows($result) != 1)
	{
		echo "Can't find 1 row for ring session ".$sessionId."<br>";
		die();
	}
	$row = mysqli_fetch_row($result);
	$session_type = $row[0];
	// the role is an enum on the wire, a bare string has no toInt()
	$mode = new RSMGR_TSessionPartStatus;
	$mode->fromString(($session_type == "st_edit") ? "sps_edit_invited" : "sps_anim_invited");
	echo "Inviting character ".htmlspecialchars($charId, ENT_QUOTES)." of user ".htmlspecialchars($userId, ENT_QUOTES)." in session ".$sessionId."<br>";

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
			echo "<h1>Error ".htmlspecialchars($resultCode, ENT_QUOTES)." : '".htmlspecialchars($resultString, ENT_QUOTES)."' while trying to start the session ".htmlspecialchars($SessionId, ENT_QUOTES)."</h1>";
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
			echo "<h1>Error ".htmlspecialchars($resultCode, ENT_QUOTES)." : '".htmlspecialchars($resultString, ENT_QUOTES)."' while trying to join the session ".htmlspecialchars($SessionId, ENT_QUOTES)."</h1>";
		}
		echo '<p><p><a href="web_start.php">Back to menu</a>';
	}
}

