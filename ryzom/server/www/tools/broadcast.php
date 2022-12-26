<?php

include_once(dirname(__DIR__).'/libs/admin_modules_itf.php');
include_once('utils.php');

array_shift($argv);
$shard = $argv?array_shift($argv):'default';
$message = $argv?implode(' ', $argv):'';

switch($shard) {

	case 'live':
		sendToChat('is starting the Shard Restart Sequence...', '#pub-general', $ShardName, ':recycle:');
		sendToChat('is broadcasting to players', '#pub-general', $ShardName, ':loudspeaker:');
		sendToChat('Le serveur va redémarrer dans 10 minutes. Merci de vous déconnecter en lieu sûr.', '#pub-uni-fr', 'Stagiaire d\''.$ShardName, ':loudspeaker:');
		sendToChat('Der Server wird in 10 Minuten heruntergefahren. Findet eine sichere Stelle und logt aus.', '#pub-uni-de',  $ShardName.'\' Praktikantin', ':loudspeaker:');
		sendToChat('The shard will go down in 10 minutes. Please find a safe location and log out.', '#pub-uni-en', $ShardName.'\' Intern', ':loudspeaker:');
		sendToChat('The shard will go down in 10 minutes. Please find a safe location and log out.', '#pub-uni', $ShardName.' Intern', ':loudspeaker:');
		sendToChat('El servidor se cerrará en 10 minutos. Por favor, encuentre un lugar seguro y cierre la sesión.', '#pub-uni-es', 'Pasante de '.$ShardName, ':loudspeaker:');

		if (!$message) {
			$message = '@{F00F}[de]Der Server wird in $minutes$ Minuten heruntergefahren.\n@{FF0F}Findet eine sichere Stelle und logt aus.';
			$message .= '[en]The shard will go down in $minutes$ minutes.\n@{FF0F}Please find a safe location and log out.';
			$message .= '[fr]Le serveur va redémarrer dans $minutes$ minutes.\n@{FF0F}Merci de vous déconnecter en lieu sûr.';
			$message .= '[es]El servidor se cerrará en $minutos$ minutos.\n@{FF0F}Por favor, encuentre un lugar seguro y cierre la sesión.';
		}
		@queryShard('egs', 'broadcast repeat=11 every=60 '.$message);
		$timer=60;
	break;

	case 'dev':
		if (!$message)
			$message = '@{F00F}The Shard will reboot soon@{FFFF}.';
		@queryShard('egs', 'broadcast during=60 every=5 '.$message);
		$timer = 6;
	break;
}

echo "\n".$timer;
