<?php
	include_once('thread_utils.php');

	//
	// INPUT:
	//
	// $user_login		login of the user that consults the forum
	// $shard			shard from which the client connects in
	// $post_from		user who post the message
	// $post_to			forum in which to post the message
	// $post_thread		thread within the forum in which to post the message
	// $post_content	content of the post
	//

	importParam('post_from');
	importParam('post_to');
	importParam('post_thread');
	importParam('post_content');

	check_character_belongs_to_guild($user_login, $post_to);

	// check mail is valid
	//if (!isset($post_from) || !isset($post_to) || !isset($post_thread) || !isset($post_content))
	//	die('Incomplete post to send');

	// check recipient has an account
	$to_dir = build_user_dir($post_to, $shard);
	$to_index = $to_dir.'forum.index';

	if (trim($post_content) != '')
	{
		$post_from = clean_string($post_from);
		$post_to = clean_string($post_to);
		$post_content = clean_content($post_content);
	
		//
		// send mail to recipient
		//
	
		// add main post to thread
		add_post($post_from, $post_to, $post_content, $post_thread, $last_date);
	
		// rebuild thread page
		build_thread_page($post_to, $post_thread, $num_posts);
		
		// update forum index since thread was touched
		update_forum_index($post_to, $post_thread, $num_posts, $last_date);
	
		// rebuild forum page
		build_forum_page($post_to);
	}

	// redirect browser to new forum page
	//redirect("thread.php?forum=$post_to&thread=$post_thread");
	exportParam('forum', $post_to);
	exportParam('thread', $post_thread);
	//$forum = $post_to;
	//$thread = $post_thread;
	include_once('thread.php');
?>