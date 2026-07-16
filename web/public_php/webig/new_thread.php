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
	// $forum			forum to create thread into
	//

	importParam('forum');
	global $forum;

	check_character_belongs_to_guild($user_login, $forum);

	read_template('new_thread.html', $new_thread);

	$instance = str_replace(array('%%SENDER%%', '%%UCSENDER%%', '%%FORUM_POST%%', '%%FORUM%%', '%%UCFORUM%%'), 
							array($user_login,  ucfirst($user_login), $forum, nameToURL($forum),      convert_forum_name($forum)),
							$new_thread);

	echo $instance;
?>