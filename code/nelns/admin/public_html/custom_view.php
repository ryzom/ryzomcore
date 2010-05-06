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

	$publicAccess = true;
	include('authenticate.php');


	unset($error);

	function reorderViews($uid)
	{
		$result = sqlquery("SELECT tid FROM view_table WHERE uid='$uid' ORDER BY ordering");
		$i = 0;
		while ($result && $arr = mysql_fetch_array($result))
		{
			sqlquery("UPDATE view_table SET ordering='$i' WHERE tid='".$arr["tid"]."'");
			++$i;
		}
	}
	
	function swapView($uid, $ordering, $offs)
	{
		$result1 = sqlquery("SELECT tid FROM view_table WHERE uid='$uid' AND ordering='$ordering'");
		if (!$result1 || mysql_num_rows($result1) != 1)
			return;
		$result1 = mysql_fetch_array($result1);
		$tid1 = $result1["tid"];

		$result2 = sqlquery("SELECT tid FROM view_table WHERE uid='$uid' AND ordering='".($ordering+$offs)."'");
		if (!$result2 || mysql_num_rows($result2) != 1)
			return;
		$result2 = mysql_fetch_array($result2);
		$tid2 = $result2["tid"];
		
		sqlquery("UPDATE view_table SET ordering='".($ordering+$offs)."' WHERE uid='$uid' AND tid='$tid1'");
		sqlquery("UPDATE view_table SET ordering='".($ordering)."' WHERE uid='$uid' AND tid='$tid2'");
	}

	function reorderRows($tid)
	{
		$result = sqlquery("SELECT vid, ordering FROM view_row WHERE tid='$tid' ORDER BY ordering");
		$i = 0;
		
		$rows = array();

		while ($result && $arr = mysql_fetch_array($result))
			$rows[] = array($arr['vid'], $arr['ordering']);

		if (count($rows) > 0)
		{
			$i = 0;
			foreach ($rows as $row)
			{
				sqlquery("UPDATE view_row SET ordering='".(-$i-1)."' WHERE tid='$tid' AND ordering='".$row[1]."'");
				++$i;
			}

			$i = 0;
			for ($i=0; $i<count($rows); ++$i)
			{
				sqlquery("UPDATE view_row SET ordering='$i' WHERE tid='$tid' AND ordering='".(-$i-1)."'");
			}
		}
	}

	function swapRows($tid, $ordering, $offs)
	{
/*
		$result1 = sqlquery("SELECT vid FROM view_row WHERE tid='$tid' AND ordering='$ordering'");
		if (!$result1 || mysql_num_rows($result1) != 1)
			return;
		$result1 = mysql_fetch_array($result1);
		$vid1 = $result1["vid"];

		$result2 = sqlquery("SELECT vid FROM view_row WHERE tid='$tid' AND ordering='".($ordering+$offs)."'");
		if (!$result2 || mysql_num_rows($result2) != 1)
			return;
		$result2 = mysql_fetch_array($result2);
		$vid2 = $result2["vid"];
		
		sqlquery("UPDATE view_row SET ordering='".($ordering+$offs)."' WHERE tid='$tid' AND vid='".($vid1)."'");
		sqlquery("UPDATE view_row SET ordering='".($ordering)."' WHERE tid='$tid' AND vid='".($vid2)."'");
*/

		sqlquery("UPDATE view_row SET ordering='-1' WHERE tid='$tid' AND ordering='".($ordering)."'");
		sqlquery("UPDATE view_row SET ordering='".($ordering)."' WHERE tid='$tid' AND ordering='".($ordering+$offs)."'");
		sqlquery("UPDATE view_row SET ordering='".($ordering+$offs)."' WHERE tid='$tid' AND ordering='-1'");

	}

	// -----------------------------
	// page commands

	// create a view
	if ($createview)
	{
		// create a table in view_table
		$result = sqlquery("SELECT tid FROM view_table WHERE uid='$uid' AND name='$viewname'");
		if ($result && mysql_num_rows($result) != 0)
		{
			$error = $error."Couldn't create view '$viewname', name already in use<br>\n";
		}
		else
		{
			$result = sqlquery("INSERT INTO view_table SET uid='$uid', name='$viewname', ordering='255'");
			if (!$result)
			{
				$error = $error."Couldn't create view '$viewname', mySQL request failed<br>\n";
			}
			$result = sqlquery("SELECT tid FROM view_table WHERE uid='$uid' AND name='$viewname'");
			$result = mysql_fetch_array($result);
			$tid = $result["tid"];

			reorderViews($uid);
		}
		
	}
	// duplicate a view
	else if (isset($dupView) && isset($tid))
	{
		$result = sqlquery("SELECT * FROM view_table WHERE tid='$tid'");
		if ($result && ($arr = sqlfetch($result)))
		{
			sqlquery("INSERT INTO view_table SET uid='$uid', name='CopyOf_".$arr["name"]."', ordering='127', filter='".$arr["filter"]."'");
			$res2 = sqlquery("SELECT tid FROM view_table WHERE uid='$uid' AND ordering='127'");
			$arr=sqlfetch($res2);
			$ntid = $arr["tid"];
			
			$result = sqlquery("SELECT * FROM view_row WHERE tid='$tid'");
			while ($result && ($arr=sqlfetch($result)))
			{
				sqlquery("INSERT INTO view_row SET tid='$ntid', vid='".$arr["vid"]."', name='".$arr["name"]."', ordering='".$arr["ordering"]."', filter='".$arr["filter"]."'");
			}
			
			reorderViews($uid);

			$tid = $ntid;
		}
	}
	// remove a view
	else if (isset($removeView))
	{
		if (!($result = sqlquery("DELETE FROM view_table WHERE uid='$uid' AND tid='$removeView'"))
			 || mysql_affected_rows() < 1)
		{
			$error = $error."Couldn't remove view $removeView, missing or user doesn't own it<br>\n";
		}
		else
		{
			sqlquery("DELETE FROM view_row WHERE tid='$removeView'");
			reorderViews($uid);
		}
	}
	// change view name
	else if (isset($chViewName))
	{
		sqlquery("UPDATE view_table SET name='$chViewName' WHERE tid='$tid'");
	}
	// change view state
	else if (isset($chViewFilter))
	{
		sqlquery("UPDATE view_table SET filter='$chViewFilter' WHERE tid='$tid'");
	}
	// change view state
	else if (isset($chViewDisplay))
	{
		sqlquery("UPDATE view_table SET display='$chViewDisplay' WHERE tid='$tid'");
	}
	// change view state
	else if (isset($chViewAutoDisplay))
	{
		sqlquery("UPDATE view_table SET auto_display='$chViewAutoDisplay' WHERE tid='$tid'");
	}
	// change view state
	else if (isset($chViewRefreshRate))
	{
		sqlquery("UPDATE view_table SET refresh_rate='$chViewRefreshRate' WHERE tid='$tid'");
	}
	// swap a view
	else if (isset($moveView) && isset($offs))
	{
		swapView($uid, $moveView, $offs);
	}
	// add a variable to a view
	else if (isset($addToView) && isset($tid))
	{
		if (hasAccessToVariable($addToView))
		{
			if (!($resultt = sqlquery("SELECT name FROM view_table WHERE uid='$uid' AND tid='$tid'"))
						|| mysql_num_rows($resultt) != 1)
			{
				$error = $error."Couldn't add variable $addToView to view $tid, view is missing or user doesn't own it<br>\n";
			}
			else
			{
				$resultt = mysql_fetch_array($resultt);
				$result = sqlquery("INSERT INTO view_row SET tid='$tid', vid='$addToView', name='".$variableData[$addToView]["name"]."', ordering='255'");
				if (!$result)
					$error = $error."Couldn't add variable ".$variableData[addToView]["name"]." to view ".$resultt["name"].", query failed";
				else
					reorderRows($tid);
			}
		}
	}
	// remove a row
	else if (isset($removeRow) && isset($tid))
	{
		$result = sqlquery("SELECT uid FROM view_table WHERE tid='$tid' AND uid='$uid'");
		if ($result && mysql_num_rows($result)>0)
		{
			if (!($result = sqlquery("DELETE FROM view_row WHERE tid='$tid' AND ordering='$removeRow'"))
				 || mysql_affected_rows() < 1)
			{
				$error = $error."Couldn't remove row $removeRow, missing or user doesn't own it<br>\n";
			}
			else
			{
				reorderRows($tid);
			}
		}
	}
	// swap a row
	else if (isset($moveRow) && isset($tid) && isset($offs))
	{
		$result = sqlquery("SELECT uid FROM view_table WHERE tid='$tid' AND uid='$uid'");
		if ($result && mysql_num_rows($result)>0)
			swapRows($tid, $moveRow, $offs);
	}
	// change a variable name
	else if ($changeVarName && isset($vid) && isset($tid))
	{
		$result = sqlquery("SELECT uid FROM view_table WHERE tid='$tid' AND uid='$uid'");
		if ($result && mysql_num_rows($result)>0)
		{
			$result = sqlquery("UPDATE view_row SET name='$changeVarName' WHERE vid='$vid' AND tid='$tid'");
		}
	}
	// change a variable state
	else if (isset($changeVarFilter) && isset($vid) && isset($tid))
	{
		$result = sqlquery("SELECT uid FROM view_table WHERE tid='$tid' AND uid='$uid'");
		if ($result && mysql_num_rows($result)>0)
		{
			$result = sqlquery("UPDATE view_row SET filter='$changeVarFilter' WHERE vid='$vid' AND tid='$tid'");
		}
	}
	// select a new default_view
	else if (isset($default_view))
	{
		sqlquery("UPDATE user SET default_view='$default_view' WHERE uid='$uid'");
	}

	// change a command name
	else if (isset($chViewCommandName) && isset($vcmd) && isset($tid))
	{
		sqlquery("UPDATE view_command SET name='$chViewCommandName' WHERE tid='$tid' AND name='$vcmd'");
	}
	else if (isset($chViewCommand) && isset($vcmd) && isset($tid))
	{
		sqlquery("UPDATE view_command SET command='$chViewCommand' WHERE tid='$tid' AND name='$vcmd'");
	}
	else if (isset($rmViewCommand) && isset($vcmd) && isset($tid))
	{
		sqlquery("DELETE FROM view_command WHERE tid='$tid' AND name='$vcmd'");
	}
	else if (isset($createViewCommand) && isset($nViewCommand) && isset($nViewCommandName) && isset($tid))
	{
		sqlquery("INSERT INTO view_command SET tid='$tid', name='$nViewCommandName', command='$nViewCommand'");
	}
	else if (isset($changeVidGraph) && isset($tid))
	{
		if (isset($graphState) && $graphState == "on")
		{
			sqlquery("UPDATE view_row SET graph='1' WHERE tid='$tid' AND vid='$changeVidGraph'");
		}
		else
		{
			sqlquery("UPDATE view_row SET graph='0' WHERE tid='$tid' AND vid='$changeVidGraph'");
		}
	}

	// give a view to another user
	else if (isset($giveTo) && isset($tid))
	{
		sqlquery("UPDATE view_table SET uid='$giveTo' WHERE tid='$tid'");
		unset($tid);
	}

	// -----------------------------
	// page display

	htmlProlog($_SERVER['PHP_SELF'], "Customize views");
	
	if ($error)
	{
		echo "<b>Reported errors:</b><br>$error<br>\n";
	}

	unset($vargroups);
	$result = sqlquery("SELECT * FROM variable_group ORDER BY name");
	while ($result && ($arr=sqlfetch($result)))
	{
		if ((!isset($sel_vgid) || $sel_vgid == "") && $arr["name"] == "NoGroup")
			$sel_vgid = $arr["vgid"];
		$vargroups[$arr["name"]] = $arr["vgid"];
	}


	
	// -----------------------------
	// display customizable views

	$res = sqlquery("SELECT default_view FROM user, view_table WHERE user.uid='$uid' AND (view_table.uid='$uid' OR view_table.uid='$gid') AND view_table.tid=user.default_view");
	if ($res && ($arr=sqlfetch($res)))
		$default_view = $arr["default_view"];
		
	unset($availViews);
	unset($userViews);
	unset($groupViews);
	$res = sqlquery("SELECT name, tid, ordering FROM view_table WHERE uid='$uid' ORDER BY ordering");
	while ($res && ($arr=sqlfetch($res)))
	{
		$availViews[] = $arr;
		$userViews[] = $arr;
	}
	$res = sqlquery("SELECT name, tid, ordering FROM view_table WHERE uid='$gid' ORDER BY ordering");
	while ($res && ($arr=sqlfetch($res)))
	{
		$availViews[] = $arr;
		$groupViews[] = $arr;
	}

	echo "<br>\n";
	echo "<table border=0><tr>\n";
	echo "<td><b>Your default view:</b></td>\n";
	echo "<form method=post action='".$_SERVER['PHP_SELF']."?tid=$tid&sel_vgid=$sel_vgid'><td>\n";
	echo "<select name='default_view' onChange='submit()'>\n";
	$selected = false;
	foreach ($availViews as $view)
	{
		$selectedView = ($view["tid"]==$default_view);
		$selected |= $selectedView;
		echo "<option value='".$view["tid"]."'".($selectedView ? " selected" : "").">".$view["name"]."\n";
	}
	echo "<option value='0'".(!$selected ? " selected" : "").">None\n";
	echo "</select>\n";
	echo "</td></form>\n";
	echo "</tr></table><br>\n";

	echo "<table><tr valign=top><td>\n";
	echo "<b>Your current views: </b>".help("View")."<br><font size=0>(click name to view/edit table, click radio to select as default view)</font><br>\n";
	echo "<table border=1>\n";
	echo "<form method=post action='".$_SERVER['PHP_SELF']."?tid=$tid&sel_vgid=$sel_vgid'>";
	echo "<tr><th>Index</th><th>[Default] View</th><th>Commands</th></tr>\n";
	if (isset($userViews) && count($userViews)>0)
	{
		foreach ($userViews as $arr)
		{
			$_tname = $arr["name"];
			$_tid = $arr["tid"];
			$color = ($tid == $_tid ? " bgcolor=#eeeeee" : "");
			echo "<tr><td$color>".$arr["ordering"]."</td>";
			echo 		"<td$color><input type=radio name=default_view value='$_tid' onClick='submit()'".($_tid==$default_view ? " checked" : "")."><a href='".$_SERVER['PHP_SELF']."?tid=$_tid&sel_vgid=$sel_vgid'>$_tname</a></td>".
						"<td$color><a href='".$_SERVER['PHP_SELF']."?removeView=$_tid&tid=$tid&sel_vgid=$sel_vgid' onClick=\"return confirm('You are about to delete a View')\">Delete</a> ".
							 "<a href='".$_SERVER['PHP_SELF']."?moveView=".$arr["ordering"]."&offs=+1&tid=$tid&sel_vgid=$sel_vgid'>-</a> ".
							 "<a href='".$_SERVER['PHP_SELF']."?moveView=".$arr["ordering"]."&offs=-1&tid=$tid&sel_vgid=$sel_vgid'>+</a> ".
							 "<a href='".$_SERVER['PHP_SELF']."?dupView=true&tid=$_tid&offs=-1&sel_vgid=$sel_vgid'>Duplicate</a></td></tr>\n";
		}
	}
	echo "</form>\n";
	echo "<tr><form method=post action='".basename($_SERVER['PHP_SELF'])."'><td></td>\n";
	echo "<td><input type=text name=viewname maxlength=32 size=16></td>\n";
	echo "<td><input type=submit name=createview value='Create new view'></td>\n";
	echo "</form></tr>\n";
	echo "</table><br>\n";
	
	echo "</td>\n";

	if (isset($groupViews) && count($groupViews)>0)
	{
		echo "<td width=40></td><td>\n";
		echo "<b>$group views: </b>".help("View")."<br><font size=0>(click name to view table, click radio to select as default view)</font><br>\n";
		echo "<table border=1>\n";
		echo "<tr><th>Index</th><th>[Default] View</th><th>Commands</th></tr>\n";
		echo "<form method=post action='".$_SERVER['PHP_SELF']."?tid=$tid&sel_vgid=$sel_vgid'>\n";
		foreach ($groupViews as $arr)
		{
			$_tname = $arr["name"];
			$_tid = $arr["tid"];
			$color = ($tid == $_tid ? " bgcolor=#eeeeee" : "");
			echo "<tr><td$color>".$arr["ordering"]."</td>".
						"<td$color><input type=radio name=default_view value='$_tid' onClick='submit()'".($_tid==$default_view ? " checked" : "")."><a href='".$_SERVER['PHP_SELF']."?tid=$_tid&sel_vgid=$sel_vgid'>$_tname</a></td>".
						"<td$color><a href='".$_SERVER['PHP_SELF']."?dupView=true&tid=$_tid&offs=-1&sel_vgid=$sel_vgid'>Duplicate</a></td></tr>\n";
		}
		echo "</form>\n";
		echo "</table><br>\n";
		echo "</td>\n";
	}
	
	echo "</tr></table>\n";

	if (isset($tid))
	{
		$result = sqlquery("SELECT name, uid, filter, display, auto_display, refresh_rate FROM view_table WHERE (uid='$uid' OR uid='$gid') AND tid='$tid'");
		if (!$result || mysql_num_rows($result) == 0)
		{
			echo "<br><b>Can't display table $tid</b><br>\n";
		}
		else
		{
			echo "<table cellpadding=0 cellspacing=0><tr valign=top><td>\n";

			$result = mysql_fetch_array($result);
			$viewName = $result["name"];
			$viewFilter = $result["filter"];
			$viewDisplay = $result["display"];
			$viewAutoDisplay = $result["auto_display"];
			$viewRefreshRate = $result["refresh_rate"];

			$ownView = ($result["uid"] == $uid);

			echo "<table border=1>\n";
			echo "<tr><form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&tid=$tid'><td colspan=3><b>Content of ".($ownView ? "<input name=chViewName value='$viewName' size=32 maxlength=32>" : $viewName)."</b></td></form>";
			if ($ownView && ($admlogin == "root" || $admlogin == $group || $IsNevrax))
			{
				echo "<form method=post action='".$_SERVER['PHP_SEL']."?sel_vgid=$sel_vgid&tid=$tid'><td colspan=4>Give view to <select name='giveTo' onChange='submit()'>";
				$gresult = sqlquery("SELECT uid, login FROM user ORDER BY login");
				while ($gresult && ($garr=sqlfetch($gresult)))
				{
					echo "<option value='".$garr["uid"]."'".($uid == $garr["uid"] ? " selected" : "").">".$garr["login"];
				}
				echo "</select>";
			}
			else
			{
				echo "<td colspan=3>";
			}
			echo "</td></tr>";
			
			$result = sqlquery("SELECT view_row.name AS name, view_row.vid AS vid, view_row.ordering AS ordering, path, view_row.filter AS filter, graph ".
										 "FROM view_table, view_row, variable ".
										 "WHERE variable.command='variable' AND view_table.uid='$uid' AND view_table.tid='$tid' AND view_table.tid=view_row.tid AND ".
										 		"view_row.vid=variable.vid ORDER BY ordering");

			if (!$result)
				die("rows select failed !");
				
			unset($rows);

			echo "<tr><th>Index</th><th><b>Variable</b></th><th>Path</th><th>Privilege</th><th>Filter</th><th>Graph</th><th>Commands</th></tr>\n";
			while ($arr = mysql_fetch_array($result))
			{
				$vid = $arr["vid"];
				
				if (!hasAccessToVariable($vid))
					continue;

				$priv = getVariableRight($vid);

				$rows[$vid] = $arr["name"];

				$ordering = $arr["ordering"];
				if ($ownView)
				{
					echo "<tr>".
								"<td>".$arr["ordering"]."</td>".
								"<form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&vid=$vid&tid=$tid'><td><input type=text name=changeVarName maxlength=128 size=16 value='".$arr["name"]."'></td></form>".
								"<td>".$arr["path"]."</td>".
								"<td>$priv</td>".
								"<form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&vid=$vid&tid=$tid'><td><input type=text name=changeVarFilter maxlength=64 size=16 value='".$arr["filter"]."'></td></form>".
								"<form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&changeVidGraph=$vid&tid=$tid'><td><input type=checkBox name=graphState".($arr["graph"] != 0 ? " checked" : "")." onClick='submit()'></td></form>".
								"<td><a href='".$_SERVER['PHP_SELF']."?removeRow=$ordering&tid=$tid&sel_vgid=$sel_vgid' onClick=\"return confirm('You are about to delete a Variable from a View')\">Delete</a> ".
									 "<a href='".$_SERVER['PHP_SELF']."?moveRow=$ordering&tid=$tid&offs=+1&sel_vgid=$sel_vgid'>-</a> ".
									 "<a href='".$_SERVER['PHP_SELF']."?moveRow=$ordering&tid=$tid&offs=-1&sel_vgid=$sel_vgid'>+</a></td></tr>\n";
				}
				else
				{
					echo "<tr>".
								"<td>".$arr["ordering"]."</td>".
								"<td>".$arr["name"]."</td>".
								"<td>".$arr["path"]."</td>".
								"<td>$priv</td>".
								"<td>".$arr["filter"]."</td>".
								"<td>".($arr["graph"] != 0 ? "Yes" : "No")."</td>".
								"<td></td></tr>\n";
				}
			}
			echo "<tr height=15><td colspan=7></td></tr>";

			$result = sqlquery("SELECT view_row.name AS name, view_row.vid AS vid, view_row.ordering AS ordering, path, view_row.filter AS filter, graph ".
										 "FROM view_table, view_row, variable ".
										 "WHERE variable.command='command' AND view_table.uid='$uid' AND view_table.tid='$tid' AND view_table.tid=view_row.tid AND ".
										 		"view_row.vid=variable.vid ORDER BY ordering");

			if (!$result)
				die("rows select failed !");
				
			unset($rows);

			echo "<tr><th>Index</th><th><b>Command</b></th><th colspan=2>Path</th><th colspan=2>Filter</th><th>Commands</th></tr>\n";
			while ($arr = mysql_fetch_array($result))
			{
				$vid = $arr["vid"];
				
				if (!hasAccessToVariable($vid))
					continue;

				$priv = getVariableRight($vid);

				$rows[$vid] = $arr["name"];

				$ordering = $arr["ordering"];
				if ($ownView)
				{
					echo "<tr>".
								"<td>".$arr["ordering"]."</td>".
								"<form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&vid=$vid&tid=$tid'><td><input type=text name=changeVarName maxlength=128 size=16 value='".$arr["name"]."'></td></form>".
								"<td colspan=2>".$arr["path"]."</td>".
								"<form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&vid=$vid&tid=$tid'><td colspan=2><input type=text name=changeVarFilter maxlength=64 size=16 value='".$arr["filter"]."'></td></form>".
								"<td><a href='".$_SERVER['PHP_SELF']."?removeRow=$ordering&tid=$tid&sel_vgid=$sel_vgid' onClick=\"return confirm('You are about to delete a Variable from a View')\">Delete</a> ".
									 "<a href='".$_SERVER['PHP_SELF']."?moveRow=$ordering&tid=$tid&offs=+1&sel_vgid=$sel_vgid'>-</a> ".
									 "<a href='".$_SERVER['PHP_SELF']."?moveRow=$ordering&tid=$tid&offs=-1&sel_vgid=$sel_vgid'>+</a></td></tr>\n";
				}
				else
				{
					echo "<tr>".
								"<td>".$arr["ordering"]."</td>".
								"<td>".$arr["name"]."</td>".
								"<td>".$arr["path"]."</td>".
								"<td>$priv</td>".
								"<td>".$arr["filter"]."</td>".
								"<td>".($arr["graph"] != 0 ? "Yes" : "No")."</td>".
								"<td></td></tr>\n";
				}
			}

			echo "<tr height=15><td colspan=7></td></tr>";

			echo "<tr><td colspan=7>";
			if ($ownView)
			{
				echo "<table>\n";
				echo "<form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&tid=$tid'><tr><th>Filter</th><td><input name=chViewFilter value='$viewFilter' size=64 maxlength=64></td></tr></form>";
				echo "<form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&tid=$tid'><tr><th>Display type</th><td><select name=chViewDisplay onChange='submit()'>";
				echo "<option value='normal'".($viewDisplay=="normal" ? " selected" : "").">Normal display";
				echo "<option value='condensed'".($viewDisplay=="condensed" ? " selected" : "").">Condensed display";
				echo "</select></td></tr></form>\n";
				echo "<form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&tid=$tid'><tr><th>Display automation</th><td><select name=chViewAutoDisplay onChange='submit()'>";
				echo "<option value='auto'".($viewAutoDisplay=="auto" ? " selected" : "").">Automatic display";
				echo "<option value='manual'".($viewAutoDisplay=="manual" ? " selected" : "").">Manual display";
				echo "</select></td></tr></form>\n";
				echo "<form method=post action='".$_SERVER['PHP_SELF']."?sel_vgid=$sel_vgid&tid=$tid'><tr><th>Refresh rate</th><td><input name=chViewRefreshRate value='$viewRefreshRate' size=5 maxlength=10> seconds</td></tr></form>\n";
				echo "</table>\n";
			}
			else
			{
				echo "<b>Filter ".($ownView ? "<input name=chViewFilter value='$viewFilter' size=64 maxlength=64>" : $viewFilter)."</b><br>";
				echo ($viewDisplay == "condensed" ? "Condensed" : "Normal")." display, ";
				echo ($viewAutoDisplay == "auto" ? "Automatic" : "Manual")." display";
			}
			echo "</td></tr>";
			echo "</table>\n";
			
			echo "</td><td width=20>\n";
			echo "</td><td>\n";
			
			if ($ownView)
			{
				echo "<b>Available variables:</b><br><font size=0>(click a variable to add it to view $viewName)</font><br>\n";
	
				echo "<table border=1 cellspacing=1>\n";
				echo "<tr><th>Variable</th>";
				echo "<form method=post action='".$_SERVER['PHP_SELF']."?tid=$tid'><th><select name=sel_vgid onChange='submit()'>\n";
				foreach ($vargroups as $vargroup => $vgid)
					echo "<option value='$vgid'".($sel_vgid == $vgid ? " selected" : "").">$vargroup\n";
				echo "<option value='-1'".(!isset($sel_vgid) || $sel_vgid=='-1' ? " selected" : "").">All Groups\n";
				echo "</select></th></form>\n";
				echo "<th>Path</th><th>State</th><th>Privilege</th></tr>\n";
	
				$result = sqlquery("SELECT command, variable.vid AS vid, variable.name AS name, path, state, variable.vgid AS vgid, variable_group.name AS group_name ".
									"FROM variable, variable_group ".
									"WHERE variable.vgid = variable_group.vgid".(isset($sel_vgid) && $sel_vgid!="-1" ? " AND variable.vgid='$sel_vgid'" : "")." ORDER BY variable.command, variable.vgid, variable.name");
				if (!$result)
					die("variable select failed !");
	
				$prevvgid = "";
				$prevvtype = "";
	
				while ($arr = mysql_fetch_array($result))
				{
					$vid = $arr["vid"];
					
					if (!hasAccessToVariable($vid))
						continue;
					
					$priv = getVariableRight($vid);

					if ($prevvtype != "" && $prevvtype != $arr["command"])
					{
						echo "<tr height=15><td colspan=5></td></tr>\n";
						echo "<tr><th>Command</th><th>Group</th><th>Path</th><th>State</th><th>Privilege</th></tr>\n";

						$prevvgid = "";
					}

					if ($prevvgid != "" && $prevvgid != $arr["vgid"])
					{
						echo "<tr height=5><td colspan=5></td></tr>\n";
					}
					$prevvgid = $arr["vgid"];
					$prevvtype = $arr["command"];
	
					echo "<tr><td><b><a href='".$_SERVER['PHP_SELF']."?addToView=$vid&tid=$tid&sel_vgid=$sel_vgid'>".$arr["name"]."</a></b></td>".
								"<td>".$arr["group_name"]."</td>".
								"<td>".$arr["path"]."</td>".
								"<td>".$arr["state"]."</td>".
								"<td>$priv</td></tr>\n";
				}
				echo "</table>\n";
			}

			echo "</td></tr></table>\n";
/*			
			//if ($admlogin == "root" || $group == $admlogin)
			{
				echo "<table><tr valign=top><td>\n";

				echo "<br><br><b>View commands</b><br>\n";
				echo "<table border=1><tr><th>Name</th><td align=center><b>Command</b> <font size=1>(with full parameter list)</font></td><th></th></tr>\n";
				$result = sqlquery("SELECT name, command FROM view_command WHERE tid='$tid' ORDER BY name");
				while ($result && ($arr = sqlfetch($result)))
				{
					echo "<tr><form method=post action='$_SERVER['PHP_SELF']?tid=$tid&vcmd=".$arr["name"]."'><td><input name=chViewCommandName value='".$arr["name"]."' size=16 maxlength=32></td></form><form method=post action='$_SERVER['PHP_SELF']?tid=$tid&vcmd=".$arr["name"]."'><td><input name=chViewCommand value='".$arr["command"]."' size=32 maxlength=32></td></form><form method=post action='$_SERVER['PHP_SELF']?tid=$tid&vcmd=".$arr["name"]."'><td><input type=submit name=rmViewCommand value='Delete' onClick=\"return confirm('You are about to delete a Command')\"></td></form></tr>\n";
				}
				echo "<tr><td colspan=3 height=5></td></th>\n";
				echo "<tr><form method=post action='$_SERVER['PHP_SELF']?tid=$tid'><td><input name=nViewCommandName size=16 maxlength=32></td><td><input name=nViewCommand size=32 maxlength=32></td><td><input type=submit name=createViewCommand value='Create'></td></form></tr>\n";
				echo "</table>\n";
				
				echo "</table>\n";

				echo "</td></tr></table><br>\n";
			}
*/
		}
	}

	htmlEpilog();

?>
