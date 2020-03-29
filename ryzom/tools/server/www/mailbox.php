<?php
	include_once('utils.php');

	importParam('page');

	unset($user);
	$user = $user_login;

	$user_dir = build_user_dir($user, $shard);

	$fname = $user_dir.'mailbox.html';
	if (!file_exists($fname))
	{
		include_once('mail_utils.php');
		build_mail_page($user);
	}

	$fname = $user_dir.'mailbox'.(isset($page) && $page!="" && $page!="0" ? "_$page" : '').'.html';
	if (!file_exists($fname))
		die("INTERNAL ERROR 10");

	$f = fopen($fname, 'r');
	echo fread($f, filesize($fname));
	fclose($f);

	// remove new_mails	file when user checks its mailbox
	$fname = $user_dir.'new_mails';
	if (file_exists($fname))
		unlink($fname);

	/* display_time(); */
?>
