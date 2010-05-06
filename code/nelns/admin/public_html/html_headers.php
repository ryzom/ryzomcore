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

	function addToLog($str)
	{
		global	$adminLog;
		$adminLog[] = $str;
	}

	function htmlProlog($rootFile, $title, $displayLinks = true, $addToHead = "")
	{
		echo "<html>\n";
		echo "<head>\n";
		echo "<title>$title</title>\n";
		echo "<SCRIPT LANGUAGE=\"JavaScript\">\n";
		echo "<!-- Begin\n";
		echo "function helpPopup(file,topic) {\n";
		echo "ElementWindow = window.open('help.php?file='+file+'&topic='+topic,'NeLToolHelpPopup','width=502,height=350,scrollbars=1')\n";
		echo "}\n";
		echo "function sqlPopup(sqlText) {\n";
		echo "ElementWindow = window.open('sql_connection.php?queries='+sqlText,'NeLToolSqlPopup','width=800,height=350,scrollbars=1')\n";
		echo "}\n";
		echo "// End -->\n";
		echo "</SCRIPT>\n";
		echo $addToHead."\n";
		echo "</head>\n";
		echo "<style type=\"text/css\">\n";
		echo "<!--\n";
		echo "body        { font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; }\n";
		echo "td          { font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; }\n";
		echo "table       { margin: 0px; padding: 0px; border-collapse: collapse; border-spacing: 0;}\n";
		echo "th          { font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; }\n";
		echo "input       { font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; }\n";
		echo "textarea    { font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; width:98%; }\n";
		echo "a:link { color: #000088; text-decoration: none; }\n";
		echo "a:visited { color: #880088; text-decoration: none; }\n";
		echo "a:active { color: #000033; text-decoration: none; }\n";
		echo "a:hover { color: #8800CC; text-decoration: underline; }\n";
		echo "-->\n";
		echo "</style>\n";
		echo "<body>\n";
		
		global	$admcookielogin, $admcookiepassword;
		
		addToLog("admcookielogin=$admcookielogin admcookiepassword=$admcookiepassword");
		addToLog("admlogin=$admcookielogin admcookiepassword=$admcookiepassword");

		if ($displayLinks)
			linkBar($title);
	}

	function htmlEpilog($logInfo = true)
	{
		global $HTTP_POST_VARS, $HTTP_GET_VARS, $sqlQueries, $nel_queries, $nel_updates, $admlogin, $allowrootdebug, $adminLog;

		if ($admlogin == "root")
			echo "<form method=post action='index.php'>Execute Raw NeL query : <input name=executeQuery size=80 maxlength=20480></form>\n";

		if ($admlogin == "root" && $allowrootdebug && $logInfo)
		{
			if (isset($nel_queries) && count($nel_queries)>=1)
			{
				echo "<hr>NeL shard queries\n";
				echo "<ul>\n";
				foreach ($nel_queries as $query)
					echo "<li>$query</li>\n";
				echo "</ul>\n";
				
			}

			displayQueries();

			echo "<hr><pre>HTTP_POST_VARS\n";
			print_r($HTTP_POST_VARS);
			echo "\nHTTP_GET_VAR\n";
			print_r($HTTP_GET_VARS);
			echo "</pre>\n";

			if (is_array($adminLog))
			{
				echo "<hr>\n";
				foreach ($adminLog as $log)
					echo "$log<br>\n";
			}
		}

/*	
			echo "<pre>";
			print_r($GLOBALS);
			echo "</pre>";
*/
		echo "</body>\n";
		echo "</html>\n";
	}

	function button($text, $bdcolor="#888888", $bgcolor="#eeeeee")
	{
		return "<table border=0 cellspacing=0 cellpadding=1><tr><td bgcolor=$bdcolor><table border=0 cellspacing=0 cellpadding=2><tr valign=bottom><td bgcolor=$bgcolor height=21>&nbsp;$text&nbsp;</td></tr></table></td></tr></table>";
	}

	function linkBar($title)
	{
		global	$admlogin, $group;
		echo "<table width=100% cellpadding=0 cellspacing=0><tr>\n";

		if (file_exists("./nel.gif"))
		{
			echo "<td width=40 align=center>";
			echo "<a href='http://www.nevrax.org' border=0><img src='./nel.gif'></a>";
			echo "</td>\n";
		}

		echo "<td>\n";
		echo "<table cellpadding=1 cellspacing=0 border=0 bgcolor=#cccccc width=100%><tr height=100%>\n";

		echo "<td align=left>";
		echo "<table cellpadding=0 cellspacing=2 border=0><tr align=left height=100%>";
		echo "<td>".button($title)."</td>";
		echo "<td>".button("<b>$admlogin</b>/<b>$group</b> on ".$HTTP_HOST.$_SERVER['PHP_SELF'].helpLink("Main"))."</td>";
		echo "</tr></table>";
		echo "</td>\n";

		echo "<td align=right>";
		echo "<table cellpadding=0 cellspacing=2 border=0 height=20><tr align=right height=100%>\n";
		echo "<td>".button("<a href='index.php'>Main page</a>")."</td>";
		echo "<td>".button("<a href='custom_view.php'>Custom View</a>")."</td>";
		echo "<td>".button("<a href='player_locator.php'>Player Locator</a>")."</td>";
		echo "<td>".button("<a href='las_interface.php'>Log Analysis</a>")."</td>";
		echo "<td>".button("<a href='prefs.php'>Preferences</a>")."</td>";
		if ($admlogin == "root" || strtolower($group) == "nevraxgroup")
		{
			echo "<td>".button("<a href='commands.php'>Commands</a>")."</td>";
		}
		if ($admlogin == "root")
		{
			echo "<td>".button("<a href='admin.php'>Administration</a>")."</td>";
		}
		echo "<td>".button("<a href='index.php?command=logout'>Logout</a>")."</td>";
		echo "</tr></table>";
		echo "</td>\n";
		
		echo "</tr></table>\n";
		echo "</td></tr></table>\n";
	}

	function subBar($menus)
	{
		echo "<table cellpadding=1 cellspacing=0 border=0 width=100%><tr><td align=right><table cellpadding=0 cellspacing=2 border=0><tr align=right>\n";
		foreach($menus as $name => $url)
			echo "<td>".button("<a href='$url'>$name</a>")."</td>";
		echo "\n</tr></table></tr></td></table>\n";
	}
	
	function getFileRoot($name)
	{
		$n = basename($name);
		$pos = strrpos($n, ".");
		if ($pos === NULL)
			return $n;
		return substr($n, 0, $pos);
	}

	function helpLink($topic)
	{
		return "<a href='javascript:helpPopup(\"".getFileRoot($_SERVER['PHP_SELF'])."\",\"$topic\")'><sup>+</sup></a>";
	}

	function help($topic)
	{
		return helpLink($topic);
	}

	function helpCommon($topic)
	{
		return "<a href='javascript:helpPopup(\"common\",\"$topic\")'><sup>+</sup></a>";
	}

	function helpAll($directory, $topic)
	{
		return "<a href='javascript:helpPopup(\"$directory\",\"$topic\")'><sup>+</sup></a>";
	}

	function getVar($name)
	{
	        if(isset($_POST[$name])) return $_POST[$name];
	        else if(isset($_GET[$name])) return $_GET[$name];
	}
?>
