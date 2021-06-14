<?php
	include_once('utils.php');

	//
	// INPUT:
	//
	// $user_login		login of the user that consults the forum
	// $shard			shard from which the user connects in
	// $forum			forum to view
	//

	importParam('forum');
	importParam('page');

	check_character_belongs_to_guild($user_login, $forum);

	$forum_dir = build_user_dir($forum, $shard);
	
	$fname = $forum_dir.'forum'.(isset($page) && $page!="" && $page!="0" ? "_$page" : '').'.html';
	if (!file_exists($fname))
	{
		include_once('thread_utils.php');
		build_forum_page($forum);
	}

	$f = fopen($fname, 'r');
	echo fread($f, filesize($fname));
	fclose($f);
?>