<?php
	include_once('mail_utils.php');
	include_once('thread_utils.php');

	if ($enter_login)
	{
		if ($login)
			setcookie('user_login', $login, time()+86400);
		else
			setcookie('user_login');
	}

	if ($rebuild_mailbox)
	{
		build_mail_page($user);
	}

	if ($rebuild_forum)
	{
		build_forum_page($user);
	}

	if ($rebuild_thread)
	{
		build_thread_page($user, $thread, $num_posts);
	}

	if ($remove_thread)
	{
		remove_thread($user, $thread);
	}



	function selectUser($var)
	{
		return 	"<select name='$var'>\n".
				"<option value='ace'>Ace\n".
				"<option value='ben'>Ben\n".
				"<option value='hulud'>Hulud\n".
				"<option value='lem'>Lem\n".
				"<option value='guild'>Guild\n".
				"<option value='guest'>Guest\n".
				"</select>\n";
	}

	echo "<html><head><title>Fake post form</title></head>\n";
	echo "<body>\n";

	echo "<table><form method=post action='form_post.php'>\n";
	echo "<tr valign=top><td></td><td><input type=submit value='Refresh'></td></tr>\n";
	echo "</form></table>\n";

	echo "<br><hr><br>\n";

	echo "<table><form method=post action='form_post.php'>\n";
	echo "<tr valign=top><td>Login</td><td><input type=text name='login' size=50 maxlength=255 value='$user_login'></td></tr>\n";
	echo "<tr valign=top><td></td><td><input type=submit name='enter_login' value='Enter'></td></tr>\n";
	echo "</form></table>\n";

	echo "<br><hr><br>\n";

	echo "Post a mail<br>\n";
	echo "<table><form method=post action='post_mail.php'>\n";
	echo "<tr valign=top><td>From</td><td>".selectUser('mail_from')."</td></tr>\n";
	echo "<tr valign=top><td>To</td><td>".selectUser('mail_to')."</td></tr>\n";
	echo "<tr valign=top><td>Subject</td><td><input type=text name='mail_subject' size=50 maxlength=255></td></tr>\n";
	echo "<tr valign=top><td>Content</td><td><textarea name='mail_content' rows=20 cols=50></textarea></td></tr>\n";
	echo "<tr valign=top><td></td><td><input type=submit value='Send'></td></tr>\n";
	echo "</form></table>\n";

	echo "<br><hr><br>\n";

	echo "Rebuild a mail box<br>\n";
	echo "<table><form method=post action='form_post.php'>\n";
	echo "<tr valign=top><td>User</td><td>".selectUser('user')."</td></tr>\n";
	echo "<tr valign=top><td></td><td><input type=submit name='rebuild_mailbox' value='Rebuild'></td></tr>\n";
	echo "</form></table>\n";

	echo "<br><hr><br>\n";



	echo "Create a thread in a forum<br>\n";
	echo "<table><form method=post action='create_thread.php'>\n";
	echo "<tr valign=top><td>From</td><td>".selectUser('post_from')."</td></tr>\n";
	echo "<tr valign=top><td>To</td><td>".selectUser('post_to')."</td></tr>\n";
	echo "<tr valign=top><td>Subject</td><td><input type=text name='post_subject' size=50 maxlength=255></td></tr>\n";
	echo "<tr valign=top><td>Content</td><td><textarea name='post_content' rows=20 cols=50></textarea></td></tr>\n";
	echo "<tr valign=top><td></td><td><input type=submit value='Post'></td></tr>\n";
	echo "</form></table>\n";

	echo "<br><hr><br>\n";

	echo "Post a topic in a thread<br>\n";
	echo "<table><form method=post action='post.php'>\n";
	echo "<tr valign=top><td>From</td><td>".selectUser('post_from')."</td></tr>\n";
	echo "<tr valign=top><td>To</td><td>".selectUser('post_to')."</td></tr>\n";
	echo "<tr valign=top><td>Thread</td><td><input type=text name='post_thread' size=50 maxlength=255></td></tr>\n";
	echo "<tr valign=top><td>Content</td><td><textarea name='post_content' rows=20 cols=50></textarea></td></tr>\n";
	echo "<tr valign=top><td></td><td><input type=submit value='Post'></td></tr>\n";
	echo "</form></table>\n";

	echo "<br><hr><br>\n";

	echo "Rebuild a forum<br>\n";
	echo "<table><form method=post action='form_post.php'>\n";
	echo "<tr valign=top><td>User</td><td>".selectUser('user')."</td></tr>\n";
	echo "<tr valign=top><td></td><td><input type=submit name='rebuild_forum' value='Rebuild'></td></tr>\n";
	echo "</form></table>\n";

	echo "<br><hr><br>\n";

	echo "Rebuild a thread<br>\n";
	echo "<table><form method=post action='form_post.php'>\n";
	echo "<tr valign=top><td>User</td><td>".selectUser('user')."</td></tr>\n";
	echo "<tr valign=top><td>Thread</td><td><input type=text name='thread' size=50 maxlength=255></td></tr>\n";
	echo "<tr valign=top><td></td><td><input type=submit name='rebuild_thread' value='Rebuild'></td></tr>\n";
	echo "</form></table>\n";

	echo "<br><hr><br>\n";

	echo "Remove a thread<br>\n";
	echo "<table><form method=post action='form_post.php'>\n";
	echo "<tr valign=top><td>User</td><td>".selectUser('user')."</td></tr>\n";
	echo "<tr valign=top><td>Thread</td><td><input type=text name='thread' size=50 maxlength=255></td></tr>\n";
	echo "<tr valign=top><td></td><td><input type=submit name='remove_thread' value='Remove'></td></tr>\n";
	echo "</form></table>\n";

	echo "<br><hr><br>\n";

	echo "</body>\n";
	echo "</html>\n";

?>