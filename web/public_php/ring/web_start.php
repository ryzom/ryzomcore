<?php
	// Ring web start page: the session management menu the in-game browser
	// opens (RingMainURL from the login reply; the /webAdmin client command).
	// Every other page in this directory links back here. This is the web
	// route for getting several players into one edit or anim session:
	// plan and start a session, invite characters into it, join what you
	// are invited to, and far-TP between mainland shards.
	require_once('../tools/validate_cookie.php');
	include_once('../login/config.php');
	include_once('../tools/domain_info.php');

	$domainId = -1;
	if (!validateCookie($userId, $domainId, $charId))
	{
		echo "Invalid cookie !";
		die();
	}

	$domainInfo = getDomainInfo($domainId);

	global $DBHost, $DBPort, $RingDBUserName, $RingDBPassword;
	$link = mysqli_connect($DBHost, $RingDBUserName, $RingDBPassword, NULL, $DBPort) or die ("Can't connect to database");
	if (function_exists('nel_mysqli_set_charset'))
		nel_mysqli_set_charset($link);
	mysqli_select_db($link, $domainInfo['ring_db_name']) or die ("Can't access to the db");

	$charSlot = getCharSlot();
	$charId = intval($charId);
	$userId = intval($userId);

	function esc($v) { return htmlspecialchars((string)$v, ENT_QUOTES); }

	// hidden fields every action form carries
	function actionFields($sessionId)
	{
		global $charSlot;
		return "<input type='hidden' name='sessionId' value='".esc($sessionId)."'>"
			 . "<input type='hidden' name='charSlot' value='".intval($charSlot)."'>";
	}

	$charName = '';
	$result = mysqli_query($link, "SELECT char_name FROM characters WHERE char_id = ".$charId);
	if ($result && ($row = mysqli_fetch_row($result)))
		$charName = $row[0];

	echo "<h1>Ring sessions</h1>";
	if ($charName != '')
		echo "Character: <b>".esc($charName)."</b> (slot ".intval($charSlot).")<br><br>";
	else
		echo "Character id ".esc($charId)." (slot ".intval($charSlot).", no character record yet)<br><br>";

	//
	// Sessions this character owns
	//
	echo "<h2>Your sessions</h2>";
	$query = "SELECT session_id, session_type, title, state FROM sessions"
		. " WHERE owner = ".$charId." AND session_type IN ('st_edit', 'st_anim')"
		. " ORDER BY session_id";
	$result = mysqli_query($link, $query) or die ("Can't execute the query");
	if (mysqli_num_rows($result) == 0)
	{
		echo "No session. ";
		echo "<a href='plan_edit_session.php?charSlot=".intval($charSlot)."'>Schedule one</a>, ";
		echo "or <a href='edit_session.php?charSlot=".intval($charSlot)."'>launch the editor</a> ";
		echo "(creates and starts your edit session in one step).<br>";
	}
	else
	{
		echo "<table border=1 cellpadding=4>";
		echo "<tr><th>Id</th><th>Type</th><th>Title</th><th>State</th><th>Actions</th></tr>";
		while ($row = mysqli_fetch_assoc($result))
		{
			$sid = intval($row['session_id']);
			echo "<tr>";
			echo "<td>".$sid."</td>";
			echo "<td>".esc($row['session_type'])."</td>";
			echo "<td>".esc($row['title'])."</td>";
			echo "<td>".esc($row['state'])."</td>";
			echo "<td>";
			if ($row['state'] == 'ss_planned')
			{
				echo "<form action='start_session.php' method='post'>".actionFields($sid)
					. "<input type='submit' value='Start'></form> ";
				echo "<form action='cancel_session.php' method='post'>".actionFields($sid)
					. "<input type='submit' value='Cancel'></form> ";
			}
			if ($row['state'] == 'ss_open' || $row['state'] == 'ss_locked')
			{
				// invitation mode follows the session type
				if ($row['session_type'] == 'st_edit')
				{
					echo "<a href='invite_pioneer.php?sessionId=".$sid."&mode=sps_edit_invited&charSlot=".intval($charSlot)."'>Invite editor</a> ";
				}
				else
				{
					echo "<a href='invite_pioneer.php?sessionId=".$sid."&mode=sps_anim_invited&charSlot=".intval($charSlot)."'>Invite animator</a> ";
					echo "<a href='invite_pioneer.php?sessionId=".$sid."&mode=sps_play_invited&charSlot=".intval($charSlot)."'>Invite player</a> ";
				}
				echo "<form action='join_session.php' method='post'>".actionFields($sid)
					. "<input type='submit' value='Join'></form> ";
				echo "<form action='close_session.php' method='post'>".actionFields($sid)
					. "<input type='submit' value='Close'></form> ";
			}
			echo "</td></tr>";
		}
		echo "</table>";
		echo "<a href='plan_edit_session.php?charSlot=".intval($charSlot)."'>Schedule another session</a><br>";
	}
	echo "<br>";

	//
	// Sessions this character is invited in / participates in
	//
	echo "<h2>Your invitations</h2>";
	$query = "SELECT sp.session_id, sp.status, s.session_type, s.title, s.state, c.char_name AS owner_name"
		. " FROM session_participant AS sp"
		. " JOIN sessions AS s ON s.session_id = sp.session_id"
		. " LEFT JOIN characters AS c ON c.char_id = s.owner"
		. " WHERE sp.char_id = ".$charId." AND s.owner != ".$charId
		. " AND s.session_type IN ('st_edit', 'st_anim')"
		. " AND s.state != 'ss_closed'"
		. " ORDER BY sp.session_id";
	$result = mysqli_query($link, $query) or die ("Can't execute the query");
	if (mysqli_num_rows($result) == 0)
	{
		echo "No invitation.<br>";
	}
	else
	{
		echo "<table border=1 cellpadding=4>";
		echo "<tr><th>Id</th><th>Type</th><th>Title</th><th>Owner</th><th>State</th><th>Your role</th><th>Actions</th></tr>";
		while ($row = mysqli_fetch_assoc($result))
		{
			$sid = intval($row['session_id']);
			echo "<tr>";
			echo "<td>".$sid."</td>";
			echo "<td>".esc($row['session_type'])."</td>";
			echo "<td>".esc($row['title'])."</td>";
			echo "<td>".esc($row['owner_name'])."</td>";
			echo "<td>".esc($row['state'])."</td>";
			echo "<td>".esc($row['status'])."</td>";
			echo "<td>";
			if ($row['state'] == 'ss_open')
			{
				echo "<form action='join_session.php' method='post'>".actionFields($sid)
					. "<input type='submit' value='Join'></form>";
			}
			else
			{
				echo "waiting for start";
			}
			echo "</td></tr>";
		}
		echo "</table>";
	}
	echo "<br>";

	//
	// Mainland shards
	//
	echo "<h2>Mainland shards</h2>";
	echo "<form action='join_shard.php' method='post'>";
	echo "<input type='hidden' name='ml' value='1'>";
	echo "<input type='hidden' name='charSlot' value='".intval($charSlot)."'>";
	echo "<input type='submit' value='Far TP to best mainland shard'>";
	echo "</form>";

	// per-shard teleport, like displayAllShards() but from the shard table
	// alone: the session manager refuses the ones that are down
	global $DBName;
	$nelLink = mysqli_connect($DBHost, $DBUserName, $DBPassword, NULL, $DBPort);
	if ($nelLink)
	{
		if (function_exists('nel_mysqli_set_charset'))
			nel_mysqli_set_charset($nelLink);
		if (mysqli_select_db($nelLink, $DBName))
		{
			$result = mysqli_query($nelLink, "SELECT ShardId, Name, FixedSessionId, Online FROM shard WHERE domain_id = ".intval($domainId));
			if ($result && mysqli_num_rows($result) > 0)
			{
				echo "<table border=1 cellpadding=4>";
				echo "<tr><th>Shard</th><th>Session</th><th>Online</th><th></th></tr>";
				while ($row = mysqli_fetch_assoc($result))
				{
					echo "<tr>";
					echo "<td>".esc($row['Name'])."</td>";
					echo "<td>".esc($row['FixedSessionId'])."</td>";
					echo "<td>".($row['Online'] ? "yes" : "no")."</td>";
					echo "<td><form action='join_shard.php' method='post'>"
						. "<input type='hidden' name='destSessionId' value='".esc($row['FixedSessionId'])."'>"
						. "<input type='hidden' name='charSlot' value='".intval($charSlot)."'>"
						. "<input type='submit' value='Teleport'".($row['Online'] ? "" : " disabled")."></form></td>";
					echo "</tr>";
				}
				echo "</table>";
			}
		}
	}
