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

include_once(dirname(__DIR__).'/config.php');


if (php_sapi_name() !== 'cli')
	die('no soup for you');

$link = mysqli_connect(DB_NEL_HOST, DB_NEL_USER, DB_NEL_PASS) or die("DBERR");
mysqli_select_db ($link, DB_NEL_NAME) or die("DBERR");


$domain = mysqli_escape_string($link, $argv[1]);
$host = mysqli_escape_string($link, $argv[2]);
$dbring = mysqli_escape_string($link, $argv[3]);
$desc = mysqli_escape_string($link, $argv[4]);
$id = mysqli_escape_string($link, $argv[5]);
$name = mysqli_escape_string($link, $argv[6]);

$query = "SELECT * FROM domain WHERE domain_name = '$domain'";
$result = mysqli_query($link, $query) or die("DBERR");
if (mysqli_num_rows($result) == 0)
{
	echo("NOD");
	$insert = "INSERT INTO `domain` (`domain_name`, `status`, `patch_version`, `backup_patch_url`, `patch_urls`, `login_address`, `session_manager_address`, `ring_db_name`, `web_host`, `web_host_php`, `description`) VALUES ".
	"('$domain', 'ds_open', 0, 'http:/$host/patch_test', NULL, '$host:49998', '$host:49999', '$dbring', '$host:30000', '$host:40916', '$desc');";
	mysqli_query($link, $insert) or die("DBERR : $insert");
	$result = mysqli_query($link, $query) or die("DBERR");
}

if (mysqli_num_rows($result) != 0)
{
	$domainInfo = mysqli_fetch_array($result);
	$query = "SELECT * FROM shard WHERE domain_id='".$domainInfo['domain_id']."'";
	$result = mysqli_query($link, $query) or die("DBERR");
	if (mysqli_num_rows($result) == 0)
	{

		$query = "INSERT INTO `shard` (`ShardId`, `domain_id`, `WsAddr`, `NbPlayers`, `Name`, `WSOnline`, `ClientApplication`, `Version`, `PatchURL`, `DynPatchURL`, `FixedSessionId`, `State`, `MOTD`) VALUES ".
			"($id, ".$domainInfo['domain_id'].", NULL, 0, '$desc', 0, '$domain', '', '', '', 0, 'ds_open', '');";
		$result = mysqli_query($link, $query) or die("DBERR: $query");
	}
	echo "OK";
}
?>
