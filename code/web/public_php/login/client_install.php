<?php 

	// LOG database
	$StatsDBHost = "192.168.1.169";
	$StatsDBUserName = "root";
	$StatsDBPassword = "";
	$StatsDBName = "stats";

	include_once('config.php');

	error_reporting(E_ERROR | E_PARSE);

	// global var
	$link = NULL;
	$dev_ip="192.168.1.169"; //ip where sql error are displayed
	$private_network = "/192\.168\.1\./i"; //ip where the cmd=log&msg=dump function works

	//get the ip of the viewer
	function getIp()
	{
		if (getenv("HTTP_CLIENT_IP"))
		{
			$ip = getenv("HTTP_CLIENT_IP");
		}
		elseif(getenv("HTTP_X_FORWARDED_FOR"))
		{
			$ip = getenv("HTTP_X_FORWARDED_FOR");
		}
		else
		{
			$ip = getenv("REMOTE_ADDR");
		}
		return $ip;
	}

	
	// if the player ip is the dev ip then the sql error is explain
	function die2($debug_str)
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
		$nelLink = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die2 (__FILE__. " " .__LINE__." Can't connect to database host:$DBHost user:$DBUserName");
		mysqli_select_db ($nelLink, $DBName) or die2 (__FILE__. " " .__LINE__." Can't access to the table dbname:$DBName");

		$domainName = mysqli_real_escape_string($nelLink, $domainName);
		$query = "SELECT backup_patch_url, patch_urls FROM domain WHERE domain_name='$domainName'";	
		$result = mysqli_query ($nelLink, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			
		if (mysqli_num_rows($result) != 1)
		{
			// unrecoverable error, we must giveup
			$reason = "Can't find domain '".$domainName."' (error code x)";
			$res = false;
		}

		$req = mysqli_fetch_array($result);
		
		$backup_patch_url = $req["backup_patch_url"];
		$patch_urls = $req["patch_urls"];
		
		$args = $patch_urls;
		$urls = explode(";", $args);	
		// first display backup url
		echo "<version ";

		echo 'serverPath="'.$backup_patch_url.'"';
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
				echo "\t<patchURI>$urls[$first]</patchURI>\n";
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
			

