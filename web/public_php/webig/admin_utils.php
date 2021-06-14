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

function display_mailbox_content($shard, $user)
{
	$udir = get_user_dir($user, $shard);

	echo "-- MAILBOX $user ($udir)\n";
	$dir = opendir($udir);
	while ($dir && ($file = readdir($dir)))
		if (ereg("^mail_([0-9]*)\.html", $file) || ereg("^_mail_([0-9]*)\.html", $file))
			echo "FILE:$file\n";
	echo "-- END MAILBOX $user\n";
}

function display_forum_content($shard, $user)
{
	$udir = get_user_dir($user, $shard);

	echo "-- FORUM $user ($udir)\n";
	$dir = opendir($udir);
	while ($dir && ($file = readdir($dir)))
		if (ereg("^thread_([0-9]*)\.index", $file) || ereg("^_thread_([0-9]*)\.index", $file))
			echo "FILE:$file\n";
	echo "-- END FORUM $user\n";
}

function display_thread_content($shard, $forum, $thread)
{
	$udir = get_user_dir($forum, $shard);
	read_index($udir.$thread, &$header, &$array);
	
	$a = explode("%%", $header);

	echo "-- THREAD $forum $thread\n";
	echo "TOPIC: ".$a[1]." SUBMIT: ".$a[0]."\n";
	if (count($array) > 0)
	{
		foreach ($array as $post)
		{
			echo "AUTHOR: ".$post[0]." DATE: ".$post[2]." POST: ".$post[1]."\n";
		}
	}
	echo "-- END THREAD $forum $thread\n";
}

?>