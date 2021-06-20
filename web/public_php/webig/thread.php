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

	include_once('utils.php');

	//
	// INPUT:
	//
	// $user_login		login of the user that consults the forum
	// $shard			shard from which the client connects in
	// $forum			forum to view
	// $thread			thread within forum to view
	//

	importParam('forum');
	importParam('thread');
	importParam('page');
	global $forum;
	global $thread;
	global $page;

	check_character_belongs_to_guild($user_login, $forum);

	$user_dir = build_user_dir($forum, $shard);

	$fname = $user_dir."thread_$thread".(isset($page) && $page!="" && $page!="0" ? "_$page" : '').".html";
	if (!file_exists($fname))
	{
		include_once('forum.php');
		die();
	}

	$f = fopen($fname, 'r');
	echo fread($f, filesize($fname));
	fclose($f);
?>