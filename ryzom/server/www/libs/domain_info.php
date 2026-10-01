<?php

	include_once('../config.php');

	function getDomainInfo($domainId)
	{
		$nelDb = mysqli_connect(DB_NEL_HOST, DB_NEL_USER, DB_NEL_PASS) or die("can't connect to nel db");
		mysqli_select_db ($nelDb, DB_NEL_NAME) or die("can't select nel db");
		$query = "SELECT * FROM domain WHERE domain_id = '".$domainId."'";
		$result = mysqli_query ($nelDb, $query) or die("query ".$query." failed");

		if (mysqli_num_rows($result) == 0)
		{
			die("Can't find row for domain ".$domainId);
		}

		$domainInfo = mysqli_fetch_array($result);

		return $domainInfo;
	}
