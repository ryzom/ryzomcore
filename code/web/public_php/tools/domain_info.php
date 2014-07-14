<?php

	include_once('../login/config.php');

	function getDomainInfo($domainId)
	{
		global $DBHost, $DBUserName, $DBPassword, $DBName;
		
		$link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die("can't connect to nel db");
		mysqli_select_db ($link, $DBName) or die("can't select nel db");

		$domainId = (int)$domainId;
		$query = "SELECT * FROM domain WHERE domain_id = $domainId";
		$result = mysqli_query($link, $query) or die("query ($query) failed");
		
		if (mysqli_num_rows($result) == 0)
		{
			die("Can't find row for domain ".$domainId);
		}
		
		$domainInfo = mysqli_fetch_array($result);
		
		return $domainInfo;
	}	

