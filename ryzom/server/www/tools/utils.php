<?php

include_once(dirname(__DIR__).'/libs/admin_modules_itf.php');
include_once(dirname(__DIR__).'/config.php');

function sendToChat($texts, $channel='', $username='', $icon='') {
	$ini = parse_ini_file('/etc/ryzom/shard.ini', true);
	if (is_array($texts)) {
		var_dump($texts);
		$text = $texts['en'];
	} else
		$text = $texts;

	$post_data = [
		'token' => $ini['notify']['token'],
		'channel' => $channel,
		'username' => $username,
		'text' => $icon.' '.$text,
		'json' => 1
		];
	$ch = curl_init($ini['notify']['url']);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($post_data));
	$response = json_decode(curl_exec($ch), true);
	curl_close($ch);
}


function shardLockAccess() {
	$ini = parse_ini_file('/etc/ryzom/shard.ini', true);
	@queryShard('su', 'rsm.setWSState '. $ini['shard']['id'] .' RESTRICTED ""');
}

