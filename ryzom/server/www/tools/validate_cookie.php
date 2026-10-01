<?php
// ______                           _____ _                   _   _____           _
// | ___ \                         /  ___| |                 | | |_   _|         | |
// | |_/ /   _ _______  _ __ ___   \ `--.| |__   __ _ _ __ __| |   | | ___   ___ | |___
// |    / | | |_  / _ \| '_ ` _ \   `--. \ '_ \ / _` | '__/ _` |   | |/ _ \ / _ \| / __|
// | |\ \ |_| |/ / (_) | | | | | | /\__/ / | | | (_| | | | (_| |   | | (_) | (_) | \__ \
// \_| \_\__, /___\___/|_| |_| |_| \____/|_| |_|\__,_|_|  \__,_|   \_/\___/ \___/|_|___/
//        __/ |
//       |___/
//
// Ryzom - MMORPG Framework <https://ryzom.com/dev/>
// Copyright (C) 2019  Winch Gate Property Limited
// This program is free software: read https://ryzom.com/dev/copying.html for more details

	include_once('../config.php');

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
		$link = mysqli_connect(DB_NEL_HOST, DB_NEL_USER, DB_NEL_PASS) or die ("Can't connect to database host:".DB_NEL_HOST." user:".DB_NEL_USER);
		mysqli_select_db ($link, DB_RING_NAME) or die ("Can't access to the table dbname:".DB_RING_NAME);
		$query = "SELECT user_id, current_status, current_domain_id FROM ring_users where cookie='$cookie'";
		$result = mysqli_query ($link, $query) or die ("Can't execute the query: ".$query);

		if (mysqli_num_rows ($result) == 0)
		{
			echo "Can't find cookie $cookie in database<BR>";
			return false;
		}

		$row = mysqli_fetch_array($result);
		
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
?>