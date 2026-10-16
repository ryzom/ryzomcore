<?php
	require_once('../tools/validate_cookie.php');
	include_once('../login/config.php');
	include_once('../tools/domain_info.php');
	include_once('ring_session_manager_itf.php');

	class InvitePioneerCb extends CRingSessionManagerWeb
	{
		function invokeResult($userId, $resultCode, $resultString)
		{
			global $step, $rsmProxy, $rsmSkel, $userId, $callbackClient;

			echo "Receive result";

			if ($resultCode == 0)
			{
				echo "<h1>The character ".$_POST["charName"]." have been invited in session ".$_POST["sessionId"].".</h1>";
			}
			else
			{
				echo "<h1>Failed to invite player ".$_POST["charName"]." in session ".$_POST["sessionId"]." : ".$resultString."</h1>";
			}	
		}
	}

	$step = 0;
	
	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}

	echo "Welcome user $userId<BR>";
	
	$domainInfo = getDomainInfo($domainId);
	$addr = split(":", $domainInfo["session_manager_address"]);
	$RSMHost = $addr[0];
	$RSMPort = $addr[1];
	
	if (isset($_POST["execute"]))
	{
		// lookup in the database to convert character name into
		global $DBHost, $RingDBUserName, $RingDBPassword;

		$link = mysqli_connect($DBHost, $RingDBUserName, $RingDBPassword) or die ("Can't connect to database host:$DBHost user:$RingDBUserName");
		mysqli_select_db($link, $domainInfo['ring_db_name']) or die ("Can't access to the table dbname:" . $domainInfo['ring_db_name']);
		
		// extract the character that have the specified name
		$charName = mysqli_real_escape_string($link, $_POST['charName']);
		$query = "select char_id, char_name from characters where char_name = '$charName'";
		$result = mysqli_query($link, $query) or die ("Can't execute the query: ".$query);

		if (mysqli_num_rows($result) == 0)
		{
			echo "<h1>Can't find the character ".$_POST["charName"]."<h1>";
		}
		else
		{
			$row = mysqli_fetch_assoc($result);
			$currentSession = $row['char_id'];
			$currentchar = $row['char_name'];

			// send the invitation info to the session manager
			$invitePioneer = new InvitePioneerCb;
			$res = "";
			$invitePioneer->connect($RSMHost, $RSMPort, $res);
//			$rsmProxy = new CRingSessionManagerWebProxy;

			// TODO: not sure it works with a char slot > 0
			$invitePioneer->inviteCharacter(($userId*16) + getCharSlot(), $_POST["sessionId"], $row[0], $_POST["mode"]);
			
			echo "wait result...";
			// wait the the return message
//			$rsmSkel = new CRingSessionManagerWebSkel;
			if (!$invitePioneer->waitCallback())
				echo "<h2>No response from server</h2><br>";
			else				
				echo "Result received...";
		}
	}
	else
	{
		// buid a form to gather info about the character to invite

		echo "<h1>Invite a player in the session ".$_POST["sessionId"]."</h1>";
		echo "<form action='invite_pioneer.php' method='post'>Type in character name:<br>";
		echo "<input type='text' name='charName' value=''>";
		echo "<input type='submit' name='button' value='Invite'>";
		echo "<input type='hidden' name='sessionId' value='".$_POST["sessionId"]."'>";
		echo "<input type='hidden' name='mode' value='".$_POST["mode"]."'>";
		echo "<input type='hidden' name='execute'>";
		echo "</form> ";
	}

	
?>
<p><a href="web_start.php">Return to main</a></p>
