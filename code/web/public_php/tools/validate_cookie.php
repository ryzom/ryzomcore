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
		$domainInfo = getDomainInfo($domainId);
		
		global $DBHost, $RingDBUserName, $RingDBPassword, $AcceptUnknownUser;
		
		if (!isset($_COOKIE["ryzomId"]))
		{
			echo "Cookie not found<BR>";
			return false;
		}
		
		// read the ip and compare with client ip
		$cookie = $_COOKIE["ryzomId"];
		echo "Cookie is $cookie<BR>";
		sscanf($cookie, "%02X%02X%02X%02X", $b0, $b1, $b2, $b3);
		$addr = $b0 + ($b1<<8) + ($b2<<16) + ($b3<<24);
		printf("Addr is %X<BR>", $addr);
		$addrStr = long2ip($addr);
		echo "addrStr is $addrStr<br>";

		if ($_SERVER["REMOTE_ADDR"] != $addrStr)
		{
			echo "Client ip don't match cookie<BR>";
			return false;
		}

		// check the cookie in the database		
		$link = mysqli_connect($DBHost, $RingDBUserName, $RingDBPassword) or die ("Can't connect to database host:$DBHost user:$RingDBUserName");
		mysqli_select_db($link, $domainInfo['ring_db_name']) or die ("Can't access to the table dbname:" . $domainInfo['ring_db_name']);

		$cookie = mysqli_real_escape_string($link, $cookie);
		$query = "SELECT user_id, current_status, current_domain_id FROM ring_users where cookie='$cookie'";
		$result = mysqli_query($link, $query) or die ("Can't execute the query: ".$query);

		if (mysqli_num_rows($result) == 0)
		{
			echo "Can't find cookie $cookie in database<BR>";
			return false;
		}
		
		$row = mysqli_fetch_assoc($result);
		
		if ($row["current_status"] != "cs_logged" && $row["current_status"] != "cs_online" )
		{
			echo "User $row[user_id] is not looged or online<BR>";
			return false;
		}
		
		$userId = $row["user_id"];
		$domainId = $row["current_domain_id"];
//		$charId = ($userId*16) + (getCharSlot()) & 0xf;
		$charId = $userId*16 + getCharSlot();
		
		return true;
	}
	
	function getCharSlot()
	{
		global $_GET, $_POST;		
		if (isset($_GET["charSlot"]))
			return $_GET["charSlot"];
		else if (isset($_POST["charSlot"]))
			return $_POST["charSlot"];
		else
			return 0; // temp dev: use 0 as the "ring character"
	}

