<?php

	include_once('../login/config.php');

	function getDomainInfo($domainId)
	{
		global $DBHost, $DBUserName, $DBPassword, $DBName;
		
		$nelDb = mysql_connect($DBHost, $DBUserName, $DBPassword) or die("can't connect to nel db");
		mysql_select_db ($DBName, $nelDb) or die("can't select nel db");
		$query = "SELECT * FROM domain WHERE domain_id = '".$domainId."'";
		$result = mysql_query ($query) or die("query ".$query." failed");
		
		if (mysql_num_rows($result) == 0)
		{
			die("Can't find row for domain ".$domainId);
		}
		
		$domainInfo = mysql_fetch_array($result);
		
		return $domainInfo;
	}	
	
?>