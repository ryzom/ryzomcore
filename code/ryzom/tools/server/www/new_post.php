<?php
	include_once('utils.php');

	//
	// INPUT:
	//
	// $user_login		login of the user that consults the forum
	// $forum			forum in which to post
	// $thread			thread within the forum in which to post
	// $subject			subject of the thread
	//

	importParam('forum');
	importParam('thread');
	importParam('subject');

	check_character_belongs_to_guild($user_login, $forum);

	read_template('new_post.html', $new_post);

	$instance = str_replace(array('%%SENDER%%', '%%UCSENDER%%', 		'%%FORUM_POST%%', 	'%%FORUM%%', 		'%%UCFORUM%%', 				'%%THREAD%%', '%%SUBJECT%%'), 
							array($user_login,  ucfirst($user_login), 	$forum, 			nameToURL($forum),  convert_forum_name($forum), $thread,      $subject),
							$new_post);

	echo $instance;
?>