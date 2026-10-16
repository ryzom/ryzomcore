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

include_once(dirname(__DIR__).'/libs/admin_modules_itf.php');
include_once(dirname(__DIR__).'/config.php');

function sendToChat($texts, $channel='', $username='', $icon='') {
	if (is_array($texts)) {
		var_dump($texts);
		$text = $texts['en'];
	} else
		$text = $texts;

	$post_data = [
		'token' => NOTIFY_TOKEN,
		'channel' => $channel,
		'username' => $username,
		'text' => $icon.' '.$text,
		'json' => 1
		];
	$ch = curl_init(NOTIFY_URL);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($post_data));
	$response = json_decode(curl_exec($ch), true);
	curl_close($ch);
}


function shardLockAccess() {
	@queryShard('su', 'rsm.setWSState '. SHARD_ID .' RESTRICTED ""');
}

