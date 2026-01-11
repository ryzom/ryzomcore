<?php

include_once(dirname(__DIR__).'/config.php');


$link = mysqli_connect($DBHost, $DBUserName, $DBPassword) or die("DBERR");
mysqli_select_db ($link, $DBName) or die("DBERR");

$query = "SELECT * FROM domain";
$result = mysqli_query($link, $query) or die("DBERR");

if (mysqli_num_rows($result) == 0)
{
	die("NOD");
}

$domainInfo = mysqli_fetch_array($result);

$query = "SELECT * FROM shard WHERE domain_id='".$domainInfo['domain_id']."'";
$result = mysqli_query($link, $query) or die("DBERR");

if (mysqli_num_rows($result) == 0)
{
	die("NOD");
}
echo "AOK";
?>
