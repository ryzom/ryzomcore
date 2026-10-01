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


$link = mysqli_connect(DB_NEL_HOST, DB_NEL_USER, DB_NEL_PASS) or die("DBERR");
mysqli_select_db ($link, DB_NEL_NAME) or die("DBERR");

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
