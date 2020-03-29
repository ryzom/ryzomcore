<?php

include_once('thread_utils.php');
include_once('mail_utils.php');
include_once('admin_utils.php');

if (($user_login != "support" || ($remote_addr != "192.168.1.153" && $remote_addr != "192.168.3.1")) && $remote_addr != "127.0.0.1")
{
	die();
}

importParam('recover_forum');
if ($recover_forum)
{
	echo "recover forum $recover_forum<br>\n";
	recover_thread($recover_forum);
	die();
}

importParam('recover_thread');
importParam('recover_threadthread');
if ($recover_thread && isset($recover_threadthread))
{
	echo "recover forum $recover_thread<br>\n";
	recover_one_thread($recover_thread, $recover_threadthread);
	die();
}

importParam('rename_forum');
importParam('into_forum');
if ($rename_forum)
{
	echo "rename forum $rename_forum into $into_forum<br>\n";
	rename_forum($rename_forum, $into_forum);
	die();
}


importParam('shard');

importParam('mailbox');
importParam('mail');

if ($mail)
{
	$mdir = get_user_dir($mailbox, $shard);
	readfile($mdir.$mail);
	die();
}
else if ($mailbox)
{
	display_mailbox_content($shard, $mailbox);
	die();
}

importParam('forum');
importParam('thread');

if ($thread)
{
	display_thread_content($shard, $forum, $thread);
	die();
}
else if ($forum)
{
	display_forum_content($shard, $forum);
	die();
}

?>