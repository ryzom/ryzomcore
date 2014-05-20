<?php
	require_once('../tools/validate_cookie.php');
	include_once('../login/config.php');
	include_once('../tools/domain_info.php');
	include_once('join_session.php');
	include_once('session_tools.php');

	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}
	
	echo "edit_session : user id = '$userId', char = '$charId', domain = '$domainId'<br>";
	
	$domainInfo = getDomainInfo($domainId);
	
	global $DBHost, $RingDBUserName, $RingDBPassword, $RingDBName;

	$link = mysqli_connect($DBHost, $RingDBUserName, $RingDBPassword) or die ("Can't connect to database host:$DBHost user:$RingDBUserName");
	mysqli_select_db($link, $RingDBName) or die ("Can't access to the db dbname:$RingDBName");

	// Find out if the character has an open editing session
	$query = "SELECT session_id, state ";
	$query .= " FROM sessions";
	$query .= " WHERE (owner = '".$charId."')";
	$query .= " AND (session_type = 'st_edit')";
	$query .= " AND (NOT (state IN ('ss_closed', 'ss_locked')))";
	$result = mysqli_query($link, $query) or die ("Can't execute the query: ".$query);
	$num = mysqli_num_rows($result);
	if ($num > 1)
	{
		echo "Error: more than one editing sessions for char".$charId;
		die;
	}
	
	$sessionId = 0;
	if ($num == 0)
	{
		// Not found => first, create an editing session for this character, start the session and invite himself
		$query = "SELECT char_name FROM characters WHERE char_id = $charId";
		$result = mysqli_query($link, $query) or die ("Can't execute the query: ".$query);
		$num = mysqli_num_rows($result);
		$characterName = "";
		if ($num > 0)
		{
			$row = mysqli_fetch_assoc($result);
			$characterName = $row['char_name'];
		}
		global $SessionId, $SessionToolsResult;
		planEditSession($charId, $domainId, "st_edit", $characterName, "");
		if ($SessionToolsResult === false)
			die;
		startSession($charId, $domainId, $SessionId);
		if ($SessionToolsResult === false)
			die;
		$sessionId = $SessionId;
	}
	else
	{
		$row = mysqli_fetch_assoc($result);
		$sessionId = $row['session_id'];
		$state = $row['state'];
		echo "Found your session: $sessionId ($state)<br>";
		if ($state == "ss_planned")
		{
			// First, start the session
			startSession($charId, $domainId, $sessionId);
			global $SessionId, $SessionToolsResult;
			if ($SessionToolsResult === false)
				die ("Failed to start the session");
			$sessionId = $SessionId;
			
			echo "edit_session.php : the session have been started<br>";
		}
	}
	
	// check that we character have a participation in the session and invite him if needed
	$query = "SELECT count(*) FROM session_participant WHERE session_id = $sessionId AND char_id = $charId";
	$result = mysqli_query($link, $query) or die ("Can't execute the query: ".$query);
	$num = mysqli_num_rows($result);
	if ($num != 1)
		die ("Invalid result whil checking participation for char $charId in session $sessionId<br>");
	$value = mysqli_fetch_row($result);
	if ($value[0] == 0)
	{
		// the character have not is own invitation !
		echo "Missing participation for character $charId owner of session $sessionId, adding it<br>";
		inviteOwnerInSession($charId, $domainId, $sessionId);
	}
	
	echo "edit_session.php : invitation ok<br>";
	
	// Join the session
	joinSessionFromId($userId, $domainId, $sessionId);

