<?php

include_once(dirname(__DIR__).'/libs/admin_modules_itf.php');
include_once('utils.php');

array_shift($argv);
$command = $argv?array_shift($argv):'';
$option = $argv?array_shift($argv):'';

switch($command) {

	case 'test':
		echo 'TEST...';
		echo sendToChat('Atys are processing a reboot...', $RocketChatGeneral, $ShardName.'\' Intern', ':upside_down:');
	break;

	case 'lock':
		@queryShard('su', 'rsm.setWSState '. $ShardId .' RESTRICTED ""');
	break;

	case 'stopEgs':
		@queryShard('egs', 'stopService', 'stopService', true, true);
	break;


	case 'open':
		@queryShard('su', 'rsm.setWSState '. $ShardId .' OPEN ""');
		if ($option == 'players') {
			file_put_contents('/home/nevrax/www/login/server_open_status', 'ds_open'."\n");
			sendToChat('The server is open for o/_--[ EVERYBODY ]--_\o', $RocketChatUniverse , $ShardName.'\' Intern', ':tada:');
			sendToChat('The server is now open to ALL players \o/', $RocketChatGeneral, $ShardName.'\' Intern', ':tada:');
		} else {
			file_put_contents('/home/nevrax/www/login/server_open_status', 'ds_restricted'."\n");
			if ($option != 'silent') {
				sendToChat('The server is open for RYZOM TEAM', $RocketChatUniverse, $ShardName.'\' Intern', ':raised_hands:');
				sendToChat('The server is now in the hands of the Customer Support Team.', $RocketChatGeneral, $ShardName.'\' Intern', ':raised_hands:');
			}
		}
	break;

	case 'kick_them_all':
		$ret = queryShard('egs', 'displayPlayers');
		$out = explode("\n", $ret['raw'][0]);
		$have_player = false;
		foreach($out as $i => $id) {
				$sid = explode(' ', $id);
				if ($sid[0] == 'Player:') {
						queryShard('egs', 'disconnectPlayer '.$sid[1], false);
						echo $sid[3].' has been kicked!'."\n";
						$have_player = true;
				}
		}
		if ($have_player)
			sendToChat('is killing all services...', $RocketChatGeneral, $ShardName, ':broken_heart:');
	break;
}

