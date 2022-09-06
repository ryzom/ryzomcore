<?php

include_once(dirname(__DIR__).'/libs/admin_modules_itf.php');
include_once(dirname(__DIR__).'/config.php');

function sendToChat($message, $channel='', $username='', $icon='') {
	global $RocketChatHook;
	if ($RocketChatHook) {
		$data = json_encode(array(
				'channel' => $channel,
				'username' => $username,
				'icon_emoji' => $icon,
				'text' => $message,
				)
		);
		$ch = curl_init('https://'.$RocketChatServer.'/hooks/'.$RocketChatHook);
		curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'POST');
		curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
		curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json'));
		$result = curl_exec($ch);
		curl_close($ch);
		return $result;
	} else {
		echo '<font color="orange">'.$message.'</font><br />';
		return true;
	}
}


function shardLockAccess() {
	global $ShardId;
	@queryShard('su', 'rsm.setWSState '. $ShardId .' RESTRICTED ""');
}

