<?php
	include_once('utils.php');

	//
	// INPUT:
	//
	// $user_login		login of the user that consults the forum
	// $forum			forum to create thread into
	//

	importParam('forum');

	check_character_belongs_to_guild($user_login, $forum);

	read_template('new_thread.html', $new_thread);

	$instance = str_replace(array('%%SENDER%%', '%%UCSENDER%%', '%%FORUM_POST%%', '%%FORUM%%', '%%UCFORUM%%'), 
							array($user_login,  ucfirst($user_login), $forum, nameToURL($forum),      convert_forum_name($forum)),
							$new_thread);

	echo $instance;
?>