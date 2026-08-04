<?php 

	include_once('config.php');

	error_reporting(E_ERROR | E_PARSE);

	// global var
	$link = NULL;
	$dev_ip="192.168.1.169"; //ip where sql error are displayed
	$private_network = "/192\.168\.1\./i"; //ip where the cmd=log&msg=dump function works

	// Peer address only. HTTP_CLIENT_IP / X-Forwarded-For are set by the
	// caller, so trusting them would let anyone claim a private address and
	// open the detailed error path.
	function getIp()
	{
		if (!empty($_SERVER['REMOTE_ADDR']))
			return $_SERVER['REMOTE_ADDR'];
		$ip = getenv("REMOTE_ADDR");
		return ($ip !== false && $ip !== '') ? $ip : '';
	}

	
	// if the player ip is the dev ip then the sql error is explain
	function die2($debug_str = '') // some callers pass nothing: keep the param optional
	{
		global $private_network;
		if ( preg_match($private_network, getIp()) )
		{
			die($debug_str);
		}
		else
		{
			die("GENERIC_ERROR");
		}
	}

	// Get head or post infos from the query url.
	// return default if the value has not be found
	function getPost($value, $default=NULL)
	{
		if ( isSet( $_GET[$value] ) ) { return  $_GET[$value]; }
		if ( isSet($_POST[$value]) ) { return $_POST[$value]; }
		return $default;
	}

//---------------------------------------------------------------------------

	$cmd = getPost("cmd", "get_patch_url");
	switch ($cmd)
	{
	// get Patch url from the nel database

	case "get_patch_url":
		$domain = getPost("domain", "");
		

		if ($domain == "")
		{
			echo "0:wrong domain";
			die2();
		}
		$domainName = getPost("domain");
		$nelLink = mysqli_connect($DBHost, $DBUserName, $DBPassword, NULL, $DBPort) or die2 (__FILE__. " " .__LINE__." Can't connect to database host:$DBHost user:$DBUserName");
		mysqli_select_db ($nelLink, $DBName) or die2 (__FILE__. " " .__LINE__." Can't access to the table dbname:$DBName");

		$domainName = mysqli_real_escape_string($nelLink, $domainName);
		$query = "SELECT backup_patch_url, patch_urls FROM domain WHERE domain_name='$domainName'";	
		$result = mysqli_query ($nelLink, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			
		if (mysqli_num_rows($result) != 1)
		{
			// unrecoverable error, we must giveup (falling through here
			// dereferenced the missing row and answered an empty url list)
			echo "0:unknown domain";
			die2("Can't find domain '".$domainName."' (error code x)");
		}

		$req = mysqli_fetch_array($result);
		
		$backup_patch_url = $req["backup_patch_url"];
		$patch_urls = $req["patch_urls"];
		
		$args = $patch_urls;
		$urls = explode(";", $args);	
		// first display backup url; values come from the domain table and
		// land inside an attribute / element, so escape them
		echo "<version ";

		echo 'serverPath="'.htmlspecialchars($backup_patch_url, ENT_QUOTES).'"';
		echo ">\n";

		// then display default uris
		$first = 0; 
		$last = count($urls);
		for (; $first != $last; $first++)
		{
			if ($urls[$first] == "")
			{
			}
			else
			{
				echo "\t<patchURI>".htmlspecialchars($urls[$first], ENT_QUOTES)."</patchURI>\n";
			}
		}
		echo "</version>\n";

		mysqli_close($nelLink);
		unset($nelLink);
		break;
		
	default:
		echo "0:Unknown command";
		die2();
	}
			

