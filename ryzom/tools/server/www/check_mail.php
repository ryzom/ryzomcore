<?php
	include_once('utils.php');

	unset($user);
	$user = $user_login;

	$user_dir = get_user_dir($user, $shard);
	if (!is_dir($user_dir))
		die("0");

	$fname = $user_dir.'new_mails';
	if (file_exists($fname))
	{
		unlink($fname);
		die("1");
	}
	else
	{
		die("0");
	}
?>
