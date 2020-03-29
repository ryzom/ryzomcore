<?php
	include_once('utils.php');

	//
	// INPUT:
	//
	// $user_login		login of the user that consults the forum
	// $shard			shard from which the client connects in
	// $forum			forum to view
	// $thread			thread within forum to view
	//

	importParam('forum');
	importParam('thread');
	importParam('page');

	check_character_belongs_to_guild($user_login, $forum);

	$user_dir = build_user_dir($forum, $shard);

	$fname = $user_dir."thread_$thread".(isset($page) && $page!="" && $page!="0" ? "_$page" : '').".html";
	if (!file_exists($fname))
	{
		include_once('forum.php');
		die();
	}

	$f = fopen($fname, 'r');
	echo fread($f, filesize($fname));
	fclose($f);
?>