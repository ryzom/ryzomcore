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

	unset($user);
	$user = $user_login;

	importParam('msg');
	global $msg;

	$user_dir = get_user_dir($user, $shard);
	$fname = $user_dir."mail_$msg.html";
	if (!is_dir($user_dir) || !file_exists($fname))
	{
		include_once('mailbox.php');
		die();
	}

	$f = fopen($fname, 'r');
	echo fread($f, filesize($fname));
	fclose($f);

	/* display_time(); */
?>