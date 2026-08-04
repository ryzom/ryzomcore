<?php 
	include_once('../tools/validate_cookie.php');
	include_once('ring_session_manager_itf.php');
	include_once('../tools/domain_info.php');
	include_once('../login/config.php');
	include_once('session_tools.php');

	///////////////////////////
	// Main code
	///////////////////////////
		

	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "<h1>Invalid cookie, please relog<H1>";
		die;
	}

	planEditSession($charId, $domainId,
		isset($_POST["session_type"]) ? $_POST["session_type"] : "",
		isset($_POST["title"]) ? strval($_POST["title"]) : "",
		isset($_POST["description"]) ? strval($_POST["description"]) : "");
	
	echo "Finish<br>";
	echo '<a href="web_start.php">Return to start menu</a>';

?>
