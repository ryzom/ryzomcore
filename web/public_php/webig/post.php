<?php

// Ryzom Core - MMORPG Framework <http://ryzom.dev/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
	global $post_from;
	global $post_to;
	global $post_thread;
	global $post_content;

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