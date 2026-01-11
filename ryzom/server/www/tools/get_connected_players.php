<?php

include_once(dirname(__DIR__).'/libs/admin_modules_itf.php');

$data = @queryShard('egs', 'displayPlayers', 'displayPlayers');

$users=array();
$priv_users=array();

if ($data && isset($data['parsed']) && count($data['parsed']) >= 1) {
	foreach ($data['parsed'] as $cid => $user) {
		$name = explode('(', $user[2]);
		$priv = substr($user[4], 2, strlen($user[4])-4);
		if ($priv)
			$priv_users[] = array($name[0], $priv);
		else
			$users[] = array($name[0]);
	}
		echo '<font color="lightgreen">'.count($priv_users).' Users with Privs</font><br />';
		foreach($priv_users as $u)
			echo '<font color="cyan">'.$u[0].'</font><font color="white">('.$u[1].')</font> ';
		echo '<hr /><font color="lightgreen">'.count($users).' Users</font><br /><font color="orange">';
		foreach($users as $u)
			echo $u[0].' ';
		echo '</font>';
} else {
	echo 'No players connected!';
	exit(1);
}
