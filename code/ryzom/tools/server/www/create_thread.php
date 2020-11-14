<?php
	include_once('thread_utils.php');

	//
	// INPUT:
	//
	// $user_login		login of the user that consults the forum
	// $shard			shard from which the user connects in
	// $post_from		user who create the thread
	// $post_to			forum in which the thread is to be created
	// $post_subject	topic of the new thread
	// $post_content	content of the first post (optional)
	//

	importParam('post_from');
	importParam('post_to');
	importParam('post_subject');
	importParam('post_content');

	check_character_belongs_to_guild($user_login, $post_to);

	// check mail is valid
	//if (!isset($post_from) || !isset($post_to) || !isset($post_subject))
	//	die('Incomplete post to send');
	//if ($post_subject == '')	$post_subject = 'No Subject';

	// check recipient has an account
	$to_dir = build_user_dir($post_to, $shard);
	$to_index = $to_dir.'forum.index';
	
	if (!file_exists($to_index))
	{
		build_forum_page($post_to);
	}
	
	if (trim($post_subject) != '')
	{
		$post_from = clean_string($post_from);
		$post_to = clean_string($post_to);
		$post_subject = clean_string($post_subject);
	
		//
		// send mail to recipient
		//
	
		// create new thread index
		add_thread_to_forum_index($post_from, $post_to, $post_subject, $index);
	
		// create thread file
		create_thread($post_from, $post_to, $post_subject, $index);
	
		// add main post to thread
		if ($post_content != "")
			add_post($post_from, $post_to, $post_content, $index, $last_date);
	
		// rebuild thread page
		build_thread_page($post_to, $index, $num_posts);
	
		// update forum index since thread was touched
		update_forum_index($post_to, $index, $num_posts, $last_date);
	
		// rebuild forum page
		build_forum_page($post_to);
	}

	// redirect browser to new forum page
	//redirect("forum.php?forum=$post_to");
	exportParam('forum', $post_to);
	//$forum = $post_to;
	include('forum.php');
?>