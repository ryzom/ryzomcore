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

	importParam('page');
	global $page;

	unset($user);
	$user = $user_login;

	$user_dir = build_user_dir($user, $shard);

	$fname = $user_dir.'mailbox.html';
	if (!file_exists($fname))
	{
		include_once('mail_utils.php');
		build_mail_page($user);
	}

	$fname = $user_dir.'mailbox'.(isset($page) && $page!="" && $page!="0" ? "_$page" : '').'.html';
	if (!file_exists($fname))
		die("INTERNAL ERROR 10");

	$f = fopen($fname, 'r');
	echo fread($f, filesize($fname));
	fclose($f);

	// remove new_mails	file when user checks its mailbox
	$fname = $user_dir.'new_mails';
	if (file_exists($fname))
		unlink($fname);

	/* display_time(); */
?>
