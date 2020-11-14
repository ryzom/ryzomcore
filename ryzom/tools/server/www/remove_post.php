<?php
	include_once('thread_utils.php');

	//
	// INPUT:
	//
	// $user_login			login of the user that consults the forum
	// $forum				forum to remove posts off
	// $thread				thread to remove posts off
	// $select_post_%%		posts to be removed
	//

	importParam('forum');
	importParam('thread');

	check_character_belongs_to_guild($user_login, $forum);
	
	/* if ($forum == $user_login) */
	{
		$posts = array();

		foreach ($_POST as $var => $value)
			if (matchParam($var, "select_post_", $post))
				$posts[] = $post;

		if (count($posts) > 0)
			remove_post($forum, $thread, $posts);
	}

	// redirect browser to new forum page
	//redirect("thread.php?forum=$forum&thread=$thread");
	include_once('thread.php');
?>