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
	// $forum				forum to remove threads off
	// $selected_thread_%%	threads to be removed
	//

	importParam('forum');
	importParam('thread');
	global $forum;
	global $thread;

	check_character_belongs_to_guild($user_login, $forum);

	// Same rule as remove_post: membership is not enough to wipe another
	// member's threads. The forum index stores the author in column 0.
	global $shard;
	$forum_dir = get_user_dir($forum, $shard);
	read_index($forum_dir.'forum.index', $header, $threads);

	foreach ($_POST as $var => $value)
	{
		// the thread index comes from the name of the posted field and
		// ends up in the file names that get renamed
		if (matchParam($var, "select_thread_", $thread) && safe_index_param($thread))
		{
			$author = null;
			for ($i = 0; $i < count($threads); ++$i)
			{
				if (isset($threads[$i][4]) && trim($threads[$i][4]) === trim($thread))
				{
					$author = isset($threads[$i][0]) ? trim($threads[$i][0]) : '';
					break;
				}
			}
			if ($author !== null && strcasecmp($author, $user_login) === 0)
				remove_thread($forum, $thread);
		}
	}

	// redirect browser to new forum page
	//redirect("forum.php?forum=$forum");
	include_once('forum.php');
?>