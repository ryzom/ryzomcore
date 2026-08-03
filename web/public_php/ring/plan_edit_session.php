<?php 
	include('../tools/validate_cookie.php');
	include('ring_session_manager_itf.php');
	include_once('../tools/domain_info.php');
	include('../login/config.php');

	function scheduleSessionResult($charId, $sessionId, $result, $resultString)
	{
		echo "Create session result :<br>";
		if ($result == 1)
		{
			echo "Session ".htmlspecialchars($sessionId, ENT_QUOTES)." created for character ".htmlspecialchars($charId, ENT_QUOTES)."<br>";
			
			echo "<h2>Your session have been planned, thank you<h2><br>";
		}
		else
		{
			echo "Failed to create a session for character ".htmlspecialchars($charId, ENT_QUOTES)." with error ".htmlspecialchars($resultString, ENT_QUOTES)." <br>";
		}
	}

	///////////////////////////
	// Main code
	///////////////////////////
		

	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "<h1>Invalid cookie, please relog<H1>";
		die;
	}

	$domainInfo = getDomainInfo($domainId);
	$addr = explode(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];
?>
	
<h1>Schedule an edit or animation session</h1>
<form action="send_plan_edit_session.php" method="post">
	Session title : <input type="text" name="title" value="enter a title"><br>
	Session description : <input type="text" name="description" value="enter a description"><br>
	<input type='hidden' name='session_type' value='st_edit'>
	<input type="submit" name="button" value="Schedule edition">
</form>
<br>
<br>
<form action="send_plan_edit_session.php" method="post">
	Session title : <input type="text" name="title" value="enter a title"><br>
	Session description : <input type="text" name="description" value="enter a description"><br>
	<input type='hidden' name='session_type' value='st_anim'>
	<input type="submit" name="button" value="Schedule animation">
</form>
<br>	
<br>	
<a href="web_start.php">Return to start menu</a>
