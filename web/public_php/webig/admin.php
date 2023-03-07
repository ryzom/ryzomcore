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
include_once('mail_utils.php');
include_once('admin_utils.php');

// if (($user_login != "support" || ($remote_addr != "192.168.1.153" && $remote_addr != "192.168.3.1")) && $remote_addr != "127.0.0.1")
if (true)
{
	die();
}

importParam('recover_forum');
global $recover_forum;
if ($recover_forum)
{
	echo "recover forum $recover_forum<br>\n";
	recover_thread($recover_forum);
	die();
}

importParam('recover_thread');
importParam('recover_threadthread');
global $recover_thread;
global $recover_threadthread;
if ($recover_thread && isset($recover_threadthread))
{
	echo "recover forum $recover_thread<br>\n";
	recover_one_thread($recover_thread, $recover_threadthread);
	die();
}

importParam('rename_forum');
importParam('into_forum');
global $rename_forum;
global $into_forum;
if ($rename_forum)
{
	echo "rename forum $rename_forum into $into_forum<br>\n";
	rename_forum($rename_forum, $into_forum);
	die();
}


importParam('shard');
global $shard;

importParam('mailbox');
importParam('mail');
global $mailbox;
global $mail;

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
global $forum;
global $thread;

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