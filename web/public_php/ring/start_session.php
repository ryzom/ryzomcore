<?php
	require_once('../tools/validate_cookie.php');
	include_once('../login/config.php');
	include_once('../tools/domain_info.php');
	include_once('ring_session_manager_itf.php');
	include_once('session_tools.php');

	$step = 0;
	
	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}
	else
	{
		if ($_SERVER['REQUEST_METHOD'] !== 'POST' || !isset($_POST['sessionId']))
		{
			echo "Missing sessionId";
			die();
		}
		echo "Welcome user ".htmlspecialchars($userId, ENT_QUOTES)."<BR>";

		startSession($charId, $domainId, $_POST["sessionId"]);
//		inviteOwnerInSession($charId, $domainId, $_POST["sessionId"]);

		die();
	}
?>
<p><a href="web_start.php">Return to main</a> </p>
