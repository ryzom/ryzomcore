<?php
	include('../login/config.php');

	function convertCookieForActionHandler($cookie)
	{
		$ret = "";
		for ($i = 0; $i<strlen($cookie); ++$i)
		{
			if ($cookie[$i] == '|')
				$ret .= '_';
			else
				$ret .= $cookie[$i];
		}
		return $ret;
	}

	function validateCookie(&$userId, &$domainId, &$charId)
	{
		global $DBHost, $DBPort, $RingDBUserName, $RingDBPassword, $RingDBName, $AcceptUnknownUser;

		// Most callers do not know the domain yet and pass -1: the cookie row
		// itself carries it (current_domain_id). The domain table has no row
		// for -1 (domain_id is unsigned), so asking getDomainInfo() first
		// killed the request before the cookie was even looked at. Only ask
		// the domain table when the caller supplies a real id; otherwise the
		// configured ring database is where the cookies of this web host live.
		if (intval($domainId) >= 0)
		{
			$domainInfo = getDomainInfo($domainId);
			$ringDBName = $domainInfo['ring_db_name'];
		}
		else
		{
			$ringDBName = $RingDBName;
		}

		if (!isset($_COOKIE["ryzomId"]))
		{
			echo "Cookie not found<BR>";
			return false;
		}
		
		// read the ip and compare with client ip
		$cookie = $_COOKIE["ryzomId"];
		// the cookie is this user's credential: do not print it back out
		sscanf($cookie, "%02X%02X%02X%02X", $b0, $b1, $b2, $b3);
		$addr = $b0 + ($b1<<8) + ($b2<<16) + ($b3<<24);
		$addrStr = long2ip($addr);

		if ($_SERVER["REMOTE_ADDR"] != $addrStr)
		{
			echo "Client ip don't match cookie<BR>";
			return false;
		}

		// check the cookie in the database		
		$link = mysqli_connect($DBHost, $RingDBUserName, $RingDBPassword, NULL, $DBPort) or die ("Can't connect to database");
		mysqli_select_db($link, $ringDBName) or die ("Can't access to the table");

		$cookie = mysqli_real_escape_string($link, $cookie);
		$query = "SELECT user_id, current_status, current_domain_id FROM ring_users where cookie='$cookie'";
		$result = mysqli_query($link, $query) or die ("Can't execute the query");

		if (mysqli_num_rows($result) == 0)
		{
			echo "Can't find cookie in database<BR>";
			return false;
		}
		
		$row = mysqli_fetch_assoc($result);
		
		if ($row["current_status"] != "cs_logged" && $row["current_status"] != "cs_online" )
		{
			echo "User ".htmlspecialchars($row["user_id"], ENT_QUOTES)." is not looged or online<BR>";
			return false;
		}
		
		$userId = $row["user_id"];
		$domainId = $row["current_domain_id"];
		$charId = (intval($userId) << 4) | getCharSlot();

		return true;
	}

	// The character id is userId<<4 | charSlot, so the slot is a four bit
	// field: in game it is 0..14, and 15 means "out of game, any character".
	// The slot arrives with the request, so it has to be masked here -- a
	// larger value carries into the user id part of the character id and
	// addresses another account's characters.
	function getCharSlot()
	{
		if (isset($_GET["charSlot"]))
			$slot = intval($_GET["charSlot"]);
		else if (isset($_POST["charSlot"]))
			$slot = intval($_POST["charSlot"]);
		else
			$slot = 0; // temp dev: use 0 as the "ring character"

		return $slot & 0xf;
	}

