<?php
	include_once('thread_utils.php');

	//
	// INPUT:
	//
	// $user_login			login of the user that consults the forum
	// $forum				forum to remove threads off
	// $selected_thread_%%	threads to be removed
	//

	importParam('forum');
	importParam('thread');

	check_character_belongs_to_guild($user_login, $forum);

	/* if ($forum == $user_login) */
	{
		foreach ($_POST as $var => $value)
		{
			if (matchParam($var, "select_thread_", $thread))
			{
				remove_thread($forum, $thread);
			}
		}
	}

	// redirect browser to new forum page
	//redirect("forum.php?forum=$forum");
	include_once('forum.php');
?>