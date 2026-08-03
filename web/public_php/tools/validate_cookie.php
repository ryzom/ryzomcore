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

	// NLMISC::lowbias32 (hash-prospector variant), 32-bit unsigned throughout
	function lowbias32($x)
	{
		$x = ($x ^ ($x >> 16)) & 0xFFFFFFFF;
		$x = ($x * 0x21f0aaad) & 0xFFFFFFFF;
		$x = ($x ^ ($x >> 15)) & 0xFFFFFFFF;
		$x = ($x * 0x735a2d97) & 0xFFFFFFFF;
		$x = ($x ^ ($x >> 15)) & 0xFFFFFFFF;
		return $x;
	}

	// CInetAddress(ipStr).hash32() as the login service computes it when it
	// builds the cookie: the address stored as an IPv4-mapped IPv6 address,
	// mixed 32 bits at a time (little endian) through lowbias32, then one
	// final round folding in the port -- always 0 for a bare address string.
	// Returns false when the address does not parse.
	function addressHash32($ipStr)
	{
		$bin = @inet_pton((string)$ipStr);
		if ($bin === false)
			return false;
		if (strlen($bin) == 4)
			$bin = str_repeat("\x00", 10)."\xff\xff".$bin;
		$h = 0;
		for ($i = 0; $i < 16; $i += 4)
		{
			$chunk = ord($bin[$i]) | (ord($bin[$i+1]) << 8) | (ord($bin[$i+2]) << 16) | (ord($bin[$i+3]) << 24);
			$h = lowbias32($h ^ $chunk);
		}
		return lowbias32($h); // ^ port, which is 0 here
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

		// Distinct failure strings map cookie/IP/status state for an attacker.
		// Say the same thing to the client; keep detail for the server log only.
		$authFailed = function ($detail) {
			error_log('validateCookie: ' . $detail);
			echo "Authentication failed<BR>";
			return false;
		};

		if (!isset($_COOKIE["ryzomId"]))
		{
			return $authFailed('cookie missing');
		}
		
		// The first cookie field is CInetAddress::hash32() of the address
		// the login request came from (see CLoginService::on_login). It
		// stopped being the raw IPv4 when the engine went dual-stack, so
		// recompute the same hash from this request's address and compare
		// hashes. The cookie is this user's credential: never print it.
		$cookie = $_COOKIE["ryzomId"];
		if (!preg_match('/^[0-9A-Fa-f]{8}\|/', $cookie))
		{
			return $authFailed('cookie malformed');
		}
		$cookieAddr = hexdec(substr($cookie, 0, 8));
		$requestAddr = addressHash32($_SERVER["REMOTE_ADDR"]);

		if ($requestAddr === false || $cookieAddr !== $requestAddr)
		{
			return $authFailed('cookie address mismatch');
		}

		// check the cookie in the database		
		$link = mysqli_connect($DBHost, $RingDBUserName, $RingDBPassword, NULL, $DBPort) or die ("Can't connect to database");
		mysqli_select_db($link, $ringDBName) or die ("Can't access to the table");

		$cookie = mysqli_real_escape_string($link, $cookie);
		$query = "SELECT user_id, current_status, current_domain_id FROM ring_users where cookie='$cookie'";
		$result = mysqli_query($link, $query) or die ("Can't execute the query");

		if (mysqli_num_rows($result) == 0)
		{
			return $authFailed('cookie not in database');
		}
		
		$row = mysqli_fetch_assoc($result);
		
		if ($row["current_status"] != "cs_logged" && $row["current_status"] != "cs_online" )
		{
			return $authFailed('user status '.$row["current_status"].' for user '.$row["user_id"]);
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

	/*
	 * Front-end service addresses from the session manager land in the
	 * client action-handler string and the login "1#..." protocol line.
	 * Only host:port (or bare host) forms that cannot reframe those payloads.
	 */
	function validShardAddr($addr)
	{
		if (!is_string($addr) || $addr === '')
			return false;
		// IPv4 / hostname / limited IPv6-ish, optional :port; no spaces,
		// quotes, pipes, hashes or control characters.
		return (bool)preg_match('/^[A-Za-z0-9._:-]{1,128}$/', $addr)
			&& strpos($addr, '|') === false
			&& strpos($addr, '#') === false
			&& strpos($addr, '"') === false
			&& strpos($addr, "'") === false;
	}

