<?php
// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

	include('html_headers.php');
	include('sql_connection.php');

	$file = getVar('file');
	$topic = getVar('topic');
	$edit = getVar('edit');
	$help_body = getVar('help_body');
	$update = getVar('update');

	htmlProlog($_SERVER['PHP_SELF'], "Help for '$file/$topic'", false);

	$result = defaultConnectToDatabase();
	if ($result)
	{
		echo "Can't find help, database connection failed.<br>[$result]<br>\n";
	}
	else
	{
		$view = true;
		if ($edit)
		{
			echo "<p align=justify><b>Edit help</b> for <b>$file/$topic</b><br>\n";
			echo "<i>Hints/Warning:</i> Text note is not processed, and will be display as is, meaning that all HTML tags <b>must</b> be valid.\n";
			echo "References to other help pages are formatted like<br>&lt;a href='help.php?file=<i>file</i>&amp;topic=<i>topic</i>'&gt;<i>blahblah</i>&lt;/a&gt;<br>\n";
			echo "where <i>file</i> referres to a valid php file (e.g. /index.php) and <i>topic</i> to a valid topic name. For common Help Notes, <i>file</i> should be set to 'common'.\n";
			echo "You may also use curved brackets '{' and '}' to point to a link (e.g. 'info about {NeL}' will point to a common help note on 'NeL' topic.)<br>\n";
			$result = mysql_query("SELECT help_body FROM help_topic WHERE file='$file' AND topic='$topic'");
			$help_body = "[Write your help note here]";
			if ($result && ($arr=mysql_fetch_array($result)))
				$help_body = $arr["help_body"];
			
			echo "<center>\n";
			echo "<form method=post action='".$_SERVER['PHP_SELF']."'>\n";
			echo "<textarea name=help_body rows=10 cols=70>$help_body</textarea><br>\n";
			echo "<input type=submit name='update' value='Update'>\n";
			echo "<input type=submit value='Cancel'>\n";
			echo "<input type=hidden name=file value='$file'>\n";
			echo "<input type=hidden name=topic value='$topic'>\n";
			echo "</form>\n";
			echo "</center>\n";
			$view = false;
		}
		else if ($update)
		{
			mysql_query("DELETE FROM help_topic WHERE file='$file' AND topic='$topic'");
			mysql_query("INSERT INTO help_topic SET file='$file', topic='$topic', help_body='$help_body'");
		}

		if ($view)
		{
			$result = mysql_query("SELECT help_body FROM help_topic WHERE file='$file' AND topic='$topic'");
			if ($result && ($body=mysql_fetch_array($result)))
			{
				echo "<b>Help for '$file/$topic':</b><br>\n";
				echo "<a href='".$_SERVER['PHP_SELF']."?edit=1&file=$file&topic=$topic'>Edit Help</a><br><hr>\n";
				echo "<p align=justify>".ereg_replace("\x7b([^\x7d]+)\x7d", "<a href='".$_SERVER['PHP_SELF']."?file=common&topic=\\1'>\\1</a>", nl2br($body["help_body"]))."<br>\n";
			}
			else
			{
				echo "<b>No help found for '$file/$topic'.</b><br>\n";
				echo "If you want to create an <b>Help note</b> for this topic, <a href='".$_SERVER['PHP_SELF']."?edit=1&file=$file&topic=$topic'>click here</a>.\n";
			}
		}
	}

	htmlEpilog(false);
?>
