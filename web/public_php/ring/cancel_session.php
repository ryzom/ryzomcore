<?php
	require_once('../tools/validate_cookie.php');
	include_once('../login/config.php');
	include_once('../tools/domain_info.php');
	include_once('ring_session_manager_itf.php');

	
	class CancelSessionCb extends CRingSessionManagerWeb
	{
		function invokeResult($userId, $resultCode, $resultString)
		{
			global $step, $rsmProxy, $rsmSkel, $userId, $callbackClient;
			
			if ($resultCode != 0)
			{
				echo "<h1>Error ".$resultCode." : '".$resultString."' will trying to cancel the session ".$_POST["sessionId"]."</h1>";
				echo '<p><p><a href="web_start.php">Back to menu</a>';
			}
			else
			{
				// ok, the session is closed (or almost to close)
				echo "<h1>Session ".$_POST["sessionId"]." has been cancelled</h1>";
			}
		}
	}

	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}
	else
	{
		$domainInfo = getDomainInfo($domainId);
		$addr = split(":", $domainInfo["session_manager_address"]);
		$RSMHost = $addr[0];
		$RSMPort = $addr[1];
		
		// ask to start the session
		$cancelSessionCb = new CancelSessionCb;
		$res = "";
		$cancelSessionCb->connect($RSMHost, $RSMPort, $res);
		$cancelSessionCb->cancelSession($charId, $_POST["sessionId"]);
		
		// wait the the return message
		$cancelSessionCb->waitCallback();
	}
?>
<p><a href="web_start.php">Return to main</a> </p>
