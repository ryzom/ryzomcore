<?php

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