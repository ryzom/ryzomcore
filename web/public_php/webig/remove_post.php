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
	// $user_login			login of the user that consults the forum
	// $forum				forum to remove posts off
	// $thread				thread to remove posts off
	// $select_post_%%		posts to be removed
	//

	importParam('forum');
	importParam('thread');
	global $forum;
	global $thread;

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