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

	// Help was reachable with no session: anyone who could hit this url could
	// rewrite a help topic, and the body is rendered as HTML for every later
	// viewer. Same login bar as the rest of the tool.
	include('authenticate.php');

	$file = getVar('file');
	$topic = getVar('topic');
	$edit = getVar('edit');
	$help_body = getVar('help_body');
	$update = getVar('update');

	// The body is still HTML by design for root authors, so only root (and
	// the nevrax group when allowNevrax is on) may edit or update. Any
	// authenticated user may still read.
	$canEditHelp = ($admlogin === 'root' || (!empty($allowNevrax) && !empty($IsNevrax)));
	if (!$canEditHelp)
	{
		$edit = false;
		$update = false;
	}

	htmlProlog(htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES), "Help for '".htmlspecialchars($file, ENT_QUOTES)."/".htmlspecialchars($topic, ENT_QUOTES)."'", false);

	$file = mysql_real_escape_string($file);
	$topic = mysql_real_escape_string($topic);
	// $file and $topic come from the url and are also written into
	// this page, so keep escaped copies for that
	$file_html = htmlspecialchars($file, ENT_QUOTES);
	$topic_html = htmlspecialchars($topic, ENT_QUOTES);
	$file_url = rawurlencode($file);
	$topic_url = rawurlencode($topic);
	$view = true;
	if ($edit)
	{
		echo "<p align=justify><b>Edit help</b> for <b>$file_html/$topic_html</b><br>\n";
		echo "<i>Hints/Warning:</i> Text note is not processed, and will be display as is, meaning that all HTML tags <b>must</b> be valid.\n";
		echo "References to other help pages are formatted like<br>&lt;a href='help.php?file=<i>file</i>&amp;topic=<i>topic</i>'&gt;<i>blahblah</i>&lt;/a&gt;<br>\n";
		echo "where <i>file</i> referres to a valid php file (e.g. /index.php) and <i>topic</i> to a valid topic name. For common Help Notes, <i>file</i> should be set to 'common'.\n";
		echo "You may also use curved brackets '{' and '}' to point to a link (e.g. 'info about {NeL}' will point to a common help note on 'NeL' topic.)<br>\n";
		$result = mysql_query("SELECT help_body FROM help_topic WHERE file='$file' AND topic='$topic'");
		$help_body = "[Write your help note here]";
		if ($result && ($arr=mysql_fetch_array($result)))
			$help_body = $arr["help_body"];
		
		echo "<center>\n";
		echo "<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."'>\n";
		// The body is stored as HTML on purpose, but the edit form itself
		// must not re-open the tags while the author is typing.
		echo "<textarea name=help_body rows=10 cols=70>".htmlspecialchars($help_body, ENT_QUOTES)."</textarea><br>\n";
		echo "<input type=submit name='update' value='Update'>\n";
		echo "<input type=submit value='Cancel'>\n";
		echo "<input type=hidden name=file value='$file_html'>\n";
		echo "<input type=hidden name=topic value='$topic_html'>\n";
		echo "</form>\n";
		echo "</center>\n";
		$view = false;
	}
	else if ($update)
	{
		mysql_query("DELETE FROM help_topic WHERE file='$file' AND topic='$topic'");
		$help_body_escaped = mysql_real_escape_string($help_body);
		mysql_query("INSERT INTO help_topic SET file='$file', topic='$topic', help_body='$help_body_escaped'");
	}

	if ($view)
	{
		$result = mysql_query("SELECT help_body FROM help_topic WHERE file='$file' AND topic='$topic'");
		if ($result && ($body=mysql_fetch_array($result)))
		{
			echo "<b>Help for '$file_html/$topic_html':</b><br>\n";
			if ($canEditHelp)
				echo "<a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?edit=1&file=$file_url&topic=$topic_url'>Edit Help</a><br><hr>\n";
			// Curly-bracket links are rewritten into anchors; the topic part
			// of the match is put into both the href and the link text, so
			// encode it for each place. The rest of the body is still HTML
			// by design (root-only edit).
			$help_html = nl2br($body["help_body"]);
			$help_html = preg_replace_callback(
				"/\x7b([^\x7d]+)\x7d/",
				function ($m) {
					$t = $m[1];
					return "<a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)
						."?file=common&topic=".rawurlencode($t)."'>"
						.htmlspecialchars($t, ENT_QUOTES)."</a>";
				},
				$help_html
			);
			echo "<p align=justify>".$help_html."<br>\n";
		}
		else
		{
			echo "<b>No help found for '$file_html/$topic_html'.</b><br>\n";
			if ($canEditHelp)
				echo "If you want to create an <b>Help note</b> for this topic, <a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?edit=1&file=$file_url&topic=$topic_url'>click here</a>.\n";
		}
	}

	htmlEpilog(false);
?>
