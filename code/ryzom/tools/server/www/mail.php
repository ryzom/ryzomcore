<?php
	include_once('utils.php');

	unset($user);
	$user = $user_login;

	importParam('msg');

	$user_dir = get_user_dir($user, $shard);
	$fname = $user_dir."mail_$msg.html";
	if (!is_dir($user_dir) || !file_exists($fname))
	{
		include_once('mailbox.php');
		die();
	}

	$f = fopen($fname, 'r');
	echo fread($f, filesize($fname));
	fclose($f);

	/* display_time(); */
?>