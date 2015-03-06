<?php
	require_once('../tools/validate_cookie.php');
	include_once('../login/config.php');
	include_once('../tools/domain_info.php');
	include_once('ring_session_manager_itf.php');

	class CloseSessionCb extends CRingSessionManagerWeb
	{
		function invokeResult($userId, $resultCode, $resultString)
		{
			global $step, $rsmProxy, $rsmSkel, $userId, $callbackClient;
			
			if ($resultCode != 0)
			{
				echo "<h1>Error ".$resultCode." : '".$resultString."' will trying to close the session ".$_POST["sessionId"]."</h1>";
				echo '<p><p><a href="web_start.php">Back to menu</a>';
			}
			else
			{
				// ok, the session is closed (or almost to close)
				echo "<h1>Session ".$_POST["sessionId"]." is begin closed</h1>";
				echo '<p><a href="web_start.php">Return to main</a> </p>';
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
		$closeSession = new CloseSessionCb;
		$res = "";
		$closeSession->connect($RSMHost, $RSMPort, $res);
//		$rsmProxy = new CRingSessionManagerWebProxy;
		$closeSession->closeSession($charId, $_POST["sessionId"]);
		
		// wait the the return message
//		$rsmSkel = new CRingSessionManagerWebSkel;
		$closeSession->waitCallback();
			
		die();
	}
	
?>
<p><a href="web_start.php">Return to main</a> </p>
