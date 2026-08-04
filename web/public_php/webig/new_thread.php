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

	// both arrive with the request and are placed in the page, so they have to
	// be escaped
	$e_login = htmlspecialchars($user_login, ENT_QUOTES);
	$e_forum = htmlspecialchars($forum, ENT_QUOTES);

	$instance = str_replace(array('%%SENDER%%', '%%UCSENDER%%', 	'%%FORUM_POST%%', 	'%%FORUM%%', 									'%%UCFORUM%%'),
							array($e_login,  	ucfirst($e_login), 	$e_forum, 			htmlspecialchars(nameToURL($forum), ENT_QUOTES),	htmlspecialchars(convert_forum_name($forum), ENT_QUOTES)),
							$new_thread);

	echo $instance;
?>