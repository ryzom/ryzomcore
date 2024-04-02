<?php

include_once(dirname(__DIR__).'/libs/admin_modules_itf.php');
include_once('utils.php');

array_shift($argv);
$shard = $argv?array_shift($argv):'default';
$message = $argv?implode(' ', $argv):'';

switch($shard) {

	case 'live':
		sendToChat('The server is starting the Shard Restart Sequence...',  $RocketChatGeneral, $ShardName.'\' Intern', ':recycle:');
		sendToChat('The server is broadcasting to players to keep a safe place', $RocketChatGeneral, $ShardName.'\' Intern', ':loudspeaker:');
		sendToChat('The shard will go down in 10 minutes. Please find a safe location and log out.', $RocketChatUniverse, $ShardName.'\' Intern', ':loudspeaker:');

		if (!$message) {
			$message = '@{F00F}[de]Der Server wird in $minutes$ Minuten heruntergefahren.\n@{FF0F}Findet eine sichere Stelle und logt aus.';
			$message .= '[en]The shard will go down in $minutes$ minutes.\n@{FF0F}Please find a safe location and log out.';
			$message .= '[fr]Le serveur va redémarrer dans $minutes$ minutes.\n@{FF0F}Merci de vous déconnecter en lieu sûr.';
			$message .= '[es]El servidor se cerrará en $minutos$ minutos.\n@{FF0F}Por favor, encuentre un lugar seguro y cierre la sesión.';
		}

		@queryShard('egs', 'broadcast repeat=11 every=60 '.$message);
		$timer = 60;
	break;

	case 'dev':
		if (!$message)
			$message = '@{F00F}The Shard will reboot soon@{FFFF}.';
		@queryShard('egs', 'broadcast during=60 every=5 '.$message);
		$timer = 6;
	break;
}

echo "\n".$timer;
