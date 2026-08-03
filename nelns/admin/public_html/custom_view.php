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

	// removeView and removeRow are plain hrefs, and SameSite=Lax still sends
	// the session cookie on a top level navigation; make the link carry a
	// token so someone else's page cannot spend a click deleting a view.
	$nelnsCsrf = nelnsCsrfToken();
	$nelnsCsrfUrl = '&csrf='.rawurlencode($nelnsCsrf);
	$nelnsCsrfOk = nelnsCsrfCheck(isset($csrf) ? $csrf : '');
	if (!$nelnsCsrfOk)
	{
		unset($removeView);
		unset($removeRow);
	}

	function reorderViews($uid)
	{
		$result = sqlquery("SELECT tid FROM view_table WHERE uid='".intval($uid)."' ORDER BY ordering");
		$i = 0;
		while ($result && $arr = sqlfetch($result))
		{
			sqlquery("UPDATE view_table SET ordering='$i' WHERE tid='".$arr["tid"]."'");
			++$i;
		}
	}
	
	function swapView($uid, $ordering, $offs)
	{
		$result1 = sqlquery("SELECT tid FROM view_table WHERE uid='".intval($uid)."' AND ordering='".intval($ordering)."'");
		if (!$result1 || sqlnumrows($result1) != 1)
			return;
		$result1 = sqlfetch($result1);
		$tid1 = $result1["tid"];

		$result2 = sqlquery("SELECT tid FROM view_table WHERE uid='".intval($uid)."' AND ordering='".(intval($ordering)+intval($offs))."'");
		if (!$result2 || sqlnumrows($result2) != 1)
			return;
		$result2 = sqlfetch($result2);
		$tid2 = $result2["tid"];
		
		sqlquery("UPDATE view_table SET ordering='".(intval($ordering)+intval($offs))."' WHERE uid='".intval($uid)."' AND tid='$tid1'");
		sqlquery("UPDATE view_table SET ordering='".intval($ordering)."' WHERE uid='".intval($uid)."' AND tid='$tid2'");
	}

	function reorderRows($tid)
	{
		$result = sqlquery("SELECT vid, ordering FROM view_row WHERE tid='".intval($tid)."' ORDER BY ordering");
		$i = 0;
		
		$rows = array();

		while ($result && $arr = sqlfetch($result))
			$rows[] = array($arr['vid'], $arr['ordering']);

		if (count($rows) > 0)
		{
			$i = 0;
			foreach ($rows as $row)
			{
				sqlquery("UPDATE view_row SET ordering='".(-$i-1)."' WHERE tid='".intval($tid)."' AND ordering='".$row[1]."'");
				++$i;
			}

			$i = 0;
			for ($i=0; $i<count($rows); ++$i)
			{
				sqlquery("UPDATE view_row SET ordering='$i' WHERE tid='".intval($tid)."' AND ordering='".(-$i-1)."'");
			}
		}
	}

	function swapRows($tid, $ordering, $offs)
	{
/*
		$result1 = sqlquery("SELECT vid FROM view_row WHERE tid='$tid' AND ordering='$ordering'");
		if (!$result1 || sqlnumrows($result1) != 1)
			return;
		$result1 = sqlfetch($result1);
		$vid1 = $result1["vid"];

		$result2 = sqlquery("SELECT vid FROM view_row WHERE tid='$tid' AND ordering='".($ordering+$offs)."'");
		if (!$result2 || sqlnumrows($result2) != 1)
			return;
		$result2 = sqlfetch($result2);
		$vid2 = $result2["vid"];
		
		sqlquery("UPDATE view_row SET ordering='".($ordering+$offs)."' WHERE tid='$tid' AND vid='".($vid1)."'");
		sqlquery("UPDATE view_row SET ordering='".($ordering)."' WHERE tid='$tid' AND vid='".($vid2)."'");
*/

		sqlquery("UPDATE view_row SET ordering='-1' WHERE tid='".intval($tid)."' AND ordering='".intval($ordering)."'");
		sqlquery("UPDATE view_row SET ordering='".intval($ordering)."' WHERE tid='".intval($tid)."' AND ordering='".(intval($ordering)+intval($offs))."'");
		sqlquery("UPDATE view_row SET ordering='".(intval($ordering)+intval($offs))."' WHERE tid='".intval($tid)."' AND ordering='-1'");

	}

	// -----------------------------
	// page commands

	// Every mutation that takes a tid must prove the caller owns that view.
	// removeView / removeRow already did; the rest used to trust the client.
	function userOwnsView($uid, $tid)
	{
		$result = sqlquery("SELECT tid FROM view_table WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		return $result && sqlnumrows($result) > 0;
	}

	// create a view
	if ($createview)
	{
		// create a table in view_table
		$result = sqlquery("SELECT tid FROM view_table WHERE uid='".intval($uid)."' AND name='".sqlescape($viewname)."'");
		if ($result && sqlnumrows($result) != 0)
		{
			$error = $error."Couldn't create view '".htmlspecialchars($viewname, ENT_QUOTES)."', name already in use<br>\n";
		}
		else
		{
			$result = sqlquery("INSERT INTO view_table SET uid='".intval($uid)."', name='".sqlescape($viewname)."', ordering='255'");
			if (!$result)
			{
				$error = $error."Couldn't create view '".htmlspecialchars($viewname, ENT_QUOTES)."', mySQL request failed<br>\n";
			}
			$result = sqlquery("SELECT tid FROM view_table WHERE uid='".intval($uid)."' AND name='".sqlescape($viewname)."'");
			$result = sqlfetch($result);
			$tid = $result["tid"];

			reorderViews(intval($uid));
		}
		
	}
	// duplicate a view
	else if (isset($dupView) && isset($tid))
	{
		// Only own views (or shared group views the user can already see
		// via the list query). Copy into the caller's uid, never mutate
		// someone else's row without ownership.
		$result = sqlquery("SELECT * FROM view_table WHERE tid='".intval($tid)."' AND (uid='".intval($uid)."' OR uid='".intval($gid)."')");
		if ($result && ($arr = sqlfetch($result)))
		{
			// name/filter were escaped on write, but re-inserting the row
			// without escaping them again is second-order SQLi once a quote
			// is already in the table
			sqlquery("INSERT INTO view_table SET uid='".intval($uid)."', name='CopyOf_".sqlescape($arr["name"])."', ordering='127', filter='".sqlescape($arr["filter"])."'");
			$res2 = sqlquery("SELECT tid FROM view_table WHERE uid='".intval($uid)."' AND ordering='127'");
			$arr=sqlfetch($res2);
			$ntid = $arr["tid"];
			
			$result = sqlquery("SELECT * FROM view_row WHERE tid='".intval($tid)."'");
			while ($result && ($arr=sqlfetch($result)))
			{
				sqlquery("INSERT INTO view_row SET tid='".intval($ntid)."', vid='".intval($arr["vid"])."', name='".sqlescape($arr["name"])."', ordering='".intval($arr["ordering"])."', filter='".sqlescape($arr["filter"])."'");
			}
			
			reorderViews(intval($uid));

			$tid = $ntid;
		}
		else
		{
			$error = $error."Couldn't duplicate view ".htmlspecialchars($tid, ENT_QUOTES).", missing or no access<br>\n";
		}
	}
	// remove a view
	else if (isset($removeView))
	{
		if (!($result = sqlquery("DELETE FROM view_table WHERE uid='".intval($uid)."' AND tid='".intval($removeView)."'"))
			 || sqlaffectedrows() < 1)
		{
			$error = $error."Couldn't remove view ".htmlspecialchars($removeView, ENT_QUOTES).", missing or user doesn't own it<br>\n";
		}
		else
		{
			sqlquery("DELETE FROM view_row WHERE tid='".intval($removeView)."'");
			reorderViews(intval($uid));
		}
	}
	// change view name
	else if (isset($chViewName) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
			sqlquery("UPDATE view_table SET name='".sqlescape($chViewName)."' WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		else
			$error = $error."Couldn't rename view, missing or user doesn't own it<br>\n";
	}
	// change view state
	else if (isset($chViewFilter) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
			sqlquery("UPDATE view_table SET filter='".sqlescape($chViewFilter)."' WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		else
			$error = $error."Couldn't change view filter, missing or user doesn't own it<br>\n";
	}
	// change view state
	else if (isset($chViewDisplay) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
			sqlquery("UPDATE view_table SET display='".sqlescape($chViewDisplay)."' WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		else
			$error = $error."Couldn't change view display, missing or user doesn't own it<br>\n";
	}
	// change view state
	else if (isset($chViewAutoDisplay) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
			sqlquery("UPDATE view_table SET auto_display='".sqlescape($chViewAutoDisplay)."' WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		else
			$error = $error."Couldn't change view auto_display, missing or user doesn't own it<br>\n";
	}
	// change view state
	else if (isset($chViewRefreshRate) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
			sqlquery("UPDATE view_table SET refresh_rate='".sqlescape($chViewRefreshRate)."' WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		else
			$error = $error."Couldn't change view refresh_rate, missing or user doesn't own it<br>\n";
	}
	// swap a view
	else if (isset($moveView) && isset($offs))
	{
		swapView(intval($uid), intval($moveView), intval($offs));
	}
	// add a variable to a view
	else if (isset($addToView) && isset($tid))
	{
		if (hasAccessToVariable($addToView))
		{
			if (!($resultt = sqlquery("SELECT name FROM view_table WHERE uid='".intval($uid)."' AND tid='".intval($tid)."'"))
						|| sqlnumrows($resultt) != 1)
			{
				$error = $error."Couldn't add variable ".htmlspecialchars($addToView, ENT_QUOTES)." to view ".htmlspecialchars($tid, ENT_QUOTES).", view is missing or user doesn't own it<br>\n";
			}
			else
			{
				$resultt = sqlfetch($resultt);
				$result = sqlquery("INSERT INTO view_row SET tid='".intval($tid)."', vid='".intval($addToView)."', name='".sqlescape($variableData[$addToView]["name"])."', ordering='255'");
				if (!$result)
					$error = $error."Couldn't add variable ".htmlspecialchars($variableData[$addToView]["name"], ENT_QUOTES)." to view ".htmlspecialchars($resultt["name"], ENT_QUOTES).", query failed";
				else
					reorderRows(intval($tid));
			}
		}
	}
	// remove a row
	else if (isset($removeRow) && isset($tid))
	{
		$result = sqlquery("SELECT uid FROM view_table WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		if ($result && sqlnumrows($result)>0)
		{
			if (!($result = sqlquery("DELETE FROM view_row WHERE tid='".intval($tid)."' AND ordering='".intval($removeRow)."'"))
				 || sqlaffectedrows() < 1)
			{
				$error = $error."Couldn't remove row ".htmlspecialchars($removeRow, ENT_QUOTES).", missing or user doesn't own it<br>\n";
			}
			else
			{
				reorderRows(intval($tid));
			}
		}
	}
	// swap a row
	else if (isset($moveRow) && isset($tid) && isset($offs))
	{
		$result = sqlquery("SELECT uid FROM view_table WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		if ($result && sqlnumrows($result)>0)
			swapRows(intval($tid), intval($moveRow), intval($offs));
	}
	// change a variable name
	else if ($changeVarName && isset($vid) && isset($tid))
	{
		$result = sqlquery("SELECT uid FROM view_table WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		if ($result && sqlnumrows($result)>0)
		{
			$result = sqlquery("UPDATE view_row SET name='".sqlescape($changeVarName)."' WHERE vid='".intval($vid)."' AND tid='".intval($tid)."'");
		}
	}
	// change a variable state
	else if (isset($changeVarFilter) && isset($vid) && isset($tid))
	{
		$result = sqlquery("SELECT uid FROM view_table WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
		if ($result && sqlnumrows($result)>0)
		{
			$result = sqlquery("UPDATE view_row SET filter='".sqlescape($changeVarFilter)."' WHERE vid='".intval($vid)."' AND tid='".intval($tid)."'");
		}
	}
	// select a new default_view
	else if (isset($default_view))
	{
		sqlquery("UPDATE user SET default_view='".intval($default_view)."' WHERE uid='".intval($uid)."'");
	}

	// change a command name
	else if (isset($chViewCommandName) && isset($vcmd) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
			sqlquery("UPDATE view_command SET name='".sqlescape($chViewCommandName)."' WHERE tid='".intval($tid)."' AND name='".sqlescape($vcmd)."'");
		else
			$error = $error."Couldn't change command name, missing or user doesn't own the view<br>\n";
	}
	else if (isset($chViewCommand) && isset($vcmd) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
			sqlquery("UPDATE view_command SET command='".sqlescape($chViewCommand)."' WHERE tid='".intval($tid)."' AND name='".sqlescape($vcmd)."'");
		else
			$error = $error."Couldn't change command, missing or user doesn't own the view<br>\n";
	}
	else if (isset($rmViewCommand) && isset($vcmd) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
			sqlquery("DELETE FROM view_command WHERE tid='".intval($tid)."' AND name='".sqlescape($vcmd)."'");
		else
			$error = $error."Couldn't remove command, missing or user doesn't own the view<br>\n";
	}
	else if (isset($createViewCommand) && isset($nViewCommand) && isset($nViewCommandName) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
			sqlquery("INSERT INTO view_command SET tid='".intval($tid)."', name='".sqlescape($nViewCommandName)."', command='".sqlescape($nViewCommand)."'");
		else
			$error = $error."Couldn't create command, missing or user doesn't own the view<br>\n";
	}
	else if (isset($changeVidGraph) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
		{
			if (isset($graphState) && $graphState == "on")
			{
				sqlquery("UPDATE view_row SET graph='1' WHERE tid='".intval($tid)."' AND vid='".intval($changeVidGraph)."'");
			}
			else
			{
				sqlquery("UPDATE view_row SET graph='0' WHERE tid='".intval($tid)."' AND vid='".intval($changeVidGraph)."'");
			}
		}
		else
			$error = $error."Couldn't change graph flag, missing or user doesn't own the view<br>\n";
	}

	// give a view to another user
	else if (isset($giveTo) && isset($tid))
	{
		if (userOwnsView($uid, $tid))
		{
			sqlquery("UPDATE view_table SET uid='".intval($giveTo)."' WHERE tid='".intval($tid)."' AND uid='".intval($uid)."'");
			unset($tid);
		}
		else
			$error = $error."Couldn't reassign view, missing or user doesn't own it<br>\n";
	}

	// -----------------------------
	// page display

	htmlProlog(htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES), "Customize views");
	
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

	$res = sqlquery("SELECT default_view FROM user, view_table WHERE user.uid='".intval($uid)."' AND (view_table.uid='".intval($uid)."' OR view_table.uid='".intval($gid)."') AND view_table.tid=user.default_view");
	if ($res && ($arr=sqlfetch($res)))
		$default_view = $arr["default_view"];
		
	unset($availViews);
	unset($userViews);
	unset($groupViews);
	$res = sqlquery("SELECT name, tid, ordering FROM view_table WHERE uid='".intval($uid)."' ORDER BY ordering");
	while ($res && ($arr=sqlfetch($res)))
	{
		$availViews[] = $arr;
		$userViews[] = $arr;
	}
	$res = sqlquery("SELECT name, tid, ordering FROM view_table WHERE uid='".intval($gid)."' ORDER BY ordering");
	while ($res && ($arr=sqlfetch($res)))
	{
		$availViews[] = $arr;
		$groupViews[] = $arr;
	}

	echo "<br>\n";
	echo "<table border=0><tr>\n";
	echo "<td><b>Your default view:</b></td>\n";
	// both ids arrive with the request: keep them numeric like the form below
	echo "<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?tid=".intval($tid)."&sel_vgid=".intval($sel_vgid)."'><td>\n";
	echo "<select name='default_view' onChange='submit()'>\n";
	$selected = false;
	foreach ($availViews as $view)
	{
		$selectedView = ($view["tid"]==$default_view);
		$selected |= $selectedView;
		echo "<option value='".intval($view["tid"])."'".($selectedView ? " selected" : "").">".htmlspecialchars($view["name"], ENT_QUOTES)."\n";
	}
	echo "<option value='0'".(!$selected ? " selected" : "").">None\n";
	echo "</select>\n";
	echo "</td></form>\n";
	echo "</tr></table><br>\n";

	echo "<table><tr valign=top><td>\n";
	echo "<b>Your current views: </b>".help("View")."<br><font size=0>(click name to view/edit table, click radio to select as default view)</font><br>\n";
	echo "<table border=1>\n";
	echo "<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?tid=".intval($tid)."&sel_vgid=".intval($sel_vgid)."'>";
	echo "<tr><th>Index</th><th>[Default] View</th><th>Commands</th></tr>\n";
	if (isset($userViews) && count($userViews)>0)
	{
		foreach ($userViews as $arr)
		{
			$_tname = htmlspecialchars($arr["name"], ENT_QUOTES);
			$_tid = intval($arr["tid"]);
			$color = ($tid == $_tid ? " bgcolor=#eeeeee" : "");
			$self = htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES);
			echo "<tr><td$color>".intval($arr["ordering"])."</td>";
			echo 		"<td$color><input type=radio name=default_view value='$_tid' onClick='submit()'".($_tid==$default_view ? " checked" : "")."><a href='$self?tid=$_tid&sel_vgid=".intval($sel_vgid)."'>$_tname</a></td>".
						"<td$color><a href='$self?removeView=$_tid$nelnsCsrfUrl&tid=".intval($tid)."&sel_vgid=".intval($sel_vgid)."' onClick=\"return confirm('You are about to delete a View')\">Delete</a> ".
							 "<a href='$self?moveView=".intval($arr["ordering"])."&offs=+1&tid=".intval($tid)."&sel_vgid=".intval($sel_vgid)."'>-</a> ".
							 "<a href='$self?moveView=".intval($arr["ordering"])."&offs=-1&tid=".intval($tid)."&sel_vgid=".intval($sel_vgid)."'>+</a> ".
							 "<a href='$self?dupView=true&tid=$_tid&offs=-1&sel_vgid=".intval($sel_vgid)."'>Duplicate</a></td></tr>\n";
		}
	}
	echo "</form>\n";
	echo "<tr><form method=post action='".htmlspecialchars(basename($_SERVER['PHP_SELF']), ENT_QUOTES)."'><td></td>\n";
	echo "<td><input type=text name=viewname maxlength=32 size=16></td>\n";
	echo "<td><input type=submit name=createview value='Create new view'></td>\n";
	echo "</form></tr>\n";
	echo "</table><br>\n";
	
	echo "</td>\n";

	if (isset($groupViews) && count($groupViews)>0)
	{
		echo "<td width=40></td><td>\n";
		echo "<b>".htmlspecialchars($group, ENT_QUOTES)." views: </b>".help("View")."<br><font size=0>(click name to view table, click radio to select as default view)</font><br>\n";
		echo "<table border=1>\n";
		echo "<tr><th>Index</th><th>[Default] View</th><th>Commands</th></tr>\n";
		echo "<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?tid=".intval($tid)."&sel_vgid=".intval($sel_vgid)."'>\n";
		foreach ($groupViews as $arr)
		{
			$_tname = $arr["name"];
			$_tid = $arr["tid"];
			$color = ($tid == $_tid ? " bgcolor=#eeeeee" : "");
			echo "<tr><td$color>".intval($arr["ordering"])."</td>".
						"<td$color><input type=radio name=default_view value='".intval($_tid)."' onClick='submit()'".($_tid==$default_view ? " checked" : "")."><a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?tid=".intval($_tid)."&sel_vgid=".intval($sel_vgid)."'>".htmlspecialchars($_tname, ENT_QUOTES)."</a></td>".
						"<td$color><a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?dupView=true&tid=".intval($_tid)."&offs=-1&sel_vgid=".intval($sel_vgid)."'>Duplicate</a></td></tr>\n";
		}
		echo "</form>\n";
		echo "</table><br>\n";
		echo "</td>\n";
	}
	
	echo "</tr></table>\n";

	if (isset($tid))
	{
		$result = sqlquery("SELECT name, uid, filter, display, auto_display, refresh_rate FROM view_table WHERE (uid='".intval($uid)."' OR uid='".intval($gid)."') AND tid='".intval($tid)."'");
		if (!$result || sqlnumrows($result) == 0)
		{
			echo "<br><b>Can't display table ".htmlspecialchars($tid, ENT_QUOTES)."</b><br>\n";
		}
		else
		{
			echo "<table cellpadding=0 cellspacing=0><tr valign=top><td>\n";

			$result = sqlfetch($result);
			$viewName = $result["name"];
			$viewFilter = $result["filter"];
			$viewDisplay = $result["display"];
			$viewAutoDisplay = $result["auto_display"];
			$viewRefreshRate = $result["refresh_rate"];

			$ownView = ($result["uid"] == $uid);

			echo "<table border=1>\n";
			$viewName_html = htmlspecialchars($viewName, ENT_QUOTES);
			echo "<tr><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?sel_vgid=".intval($sel_vgid)."&tid=".intval($tid)."'><td colspan=3><b>Content of ".($ownView ? "<input name=chViewName value='$viewName_html' size=32 maxlength=32>" : $viewName_html)."</b></td></form>";
			if ($ownView && ($admlogin == "root" || $admlogin == $group || $IsNevrax))
			{
				echo "<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?sel_vgid=".intval($sel_vgid)."&tid=".intval($tid)."'><td colspan=4>Give view to <select name='giveTo' onChange='submit()'>";
				$gresult = sqlquery("SELECT uid, login FROM user ORDER BY login");
				while ($gresult && ($garr=sqlfetch($gresult)))
				{
					echo "<option value='".intval($garr["uid"])."'".($uid == $garr["uid"] ? " selected" : "").">".htmlspecialchars($garr["login"], ENT_QUOTES);
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
										 "WHERE variable.command='variable' AND view_table.uid='".intval($uid)."' AND view_table.tid='".intval($tid)."' AND view_table.tid=view_row.tid AND ".
										 		"view_row.vid=variable.vid ORDER BY ordering");

			if (!$result)
				die("rows select failed !");
				
			unset($rows);

			echo "<tr><th>Index</th><th><b>Variable</b></th><th>Path</th><th>Privilege</th><th>Filter</th><th>Graph</th><th>Commands</th></tr>\n";
			while ($arr = sqlfetch($result))
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
								"<td>".intval($arr["ordering"])."</td>".
								"<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?sel_vgid=".intval($sel_vgid)."&vid=".intval($vid)."&tid=".intval($tid)."'><td><input type=text name=changeVarName maxlength=128 size=16 value='".htmlspecialchars($arr["name"], ENT_QUOTES)."'></td></form>".
								"<td>".htmlspecialchars($arr["path"], ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($priv, ENT_QUOTES)."</td>".
								"<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?sel_vgid=".intval($sel_vgid)."&vid=".intval($vid)."&tid=".intval($tid)."'><td><input type=text name=changeVarFilter maxlength=64 size=16 value='".htmlspecialchars($arr["filter"], ENT_QUOTES)."'></td></form>".
								"<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?sel_vgid=".intval($sel_vgid)."&changeVidGraph=".intval($vid)."&tid=".intval($tid)."'><td><input type=checkBox name=graphState".($arr["graph"] != 0 ? " checked" : "")." onClick='submit()'></td></form>".
								"<td><a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?removeRow=".intval($ordering)."$nelnsCsrfUrl&tid=".intval($tid)."&sel_vgid=".intval($sel_vgid)."' onClick=\"return confirm('You are about to delete a Variable from a View')\">Delete</a> ".
									 "<a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?moveRow=".intval($ordering)."&tid=".intval($tid)."&offs=+1&sel_vgid=".intval($sel_vgid)."'>-</a> ".
									 "<a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?moveRow=".intval($ordering)."&tid=".intval($tid)."&offs=-1&sel_vgid=".intval($sel_vgid)."'>+</a></td></tr>\n";
				}
				else
				{
					echo "<tr>".
								"<td>".intval($arr["ordering"])."</td>".
								"<td>".htmlspecialchars($arr["name"], ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($arr["path"], ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($priv, ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($arr["filter"], ENT_QUOTES)."</td>".
								"<td>".($arr["graph"] != 0 ? "Yes" : "No")."</td>".
								"<td></td></tr>\n";
				}
			}
			echo "<tr height=15><td colspan=7></td></tr>";

			$result = sqlquery("SELECT view_row.name AS name, view_row.vid AS vid, view_row.ordering AS ordering, path, view_row.filter AS filter, graph ".
										 "FROM view_table, view_row, variable ".
										 "WHERE variable.command='command' AND view_table.uid='".intval($uid)."' AND view_table.tid='".intval($tid)."' AND view_table.tid=view_row.tid AND ".
										 		"view_row.vid=variable.vid ORDER BY ordering");

			if (!$result)
				die("rows select failed !");
				
			unset($rows);

			echo "<tr><th>Index</th><th><b>Command</b></th><th colspan=2>Path</th><th colspan=2>Filter</th><th>Commands</th></tr>\n";
			while ($arr = sqlfetch($result))
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
								"<td>".intval($arr["ordering"])."</td>".
								"<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?sel_vgid=".intval($sel_vgid)."&vid=".intval($vid)."&tid=".intval($tid)."'><td><input type=text name=changeVarName maxlength=128 size=16 value='".htmlspecialchars($arr["name"], ENT_QUOTES)."'></td></form>".
								"<td colspan=2>".htmlspecialchars($arr["path"], ENT_QUOTES)."</td>".
								"<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?sel_vgid=".intval($sel_vgid)."&vid=".intval($vid)."&tid=".intval($tid)."'><td colspan=2><input type=text name=changeVarFilter maxlength=64 size=16 value='".htmlspecialchars($arr["filter"], ENT_QUOTES)."'></td></form>".
								"<td><a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?removeRow=".intval($ordering)."$nelnsCsrfUrl&tid=".intval($tid)."&sel_vgid=".intval($sel_vgid)."' onClick=\"return confirm('You are about to delete a Variable from a View')\">Delete</a> ".
									 "<a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?moveRow=".intval($ordering)."&tid=".intval($tid)."&offs=+1&sel_vgid=".intval($sel_vgid)."'>-</a> ".
									 "<a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?moveRow=".intval($ordering)."&tid=".intval($tid)."&offs=-1&sel_vgid=".intval($sel_vgid)."'>+</a></td></tr>\n";
				}
				else
				{
					echo "<tr>".
								"<td>".intval($arr["ordering"])."</td>".
								"<td>".htmlspecialchars($arr["name"], ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($arr["path"], ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($priv, ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($arr["filter"], ENT_QUOTES)."</td>".
								"<td>".($arr["graph"] != 0 ? "Yes" : "No")."</td>".
								"<td></td></tr>\n";
				}
			}

			echo "<tr height=15><td colspan=7></td></tr>";

			echo "<tr><td colspan=7>";
			if ($ownView)
			{
				echo "<table>\n";
				$viewFilter_html = htmlspecialchars($viewFilter, ENT_QUOTES);
				$self_view = htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?sel_vgid=".intval($sel_vgid)."&tid=".intval($tid);
				echo "<form method=post action='$self_view'><tr><th>Filter</th><td><input name=chViewFilter value='$viewFilter_html' size=64 maxlength=64></td></tr></form>";
				echo "<form method=post action='$self_view'><tr><th>Display type</th><td><select name=chViewDisplay onChange='submit()'>";
				echo "<option value='normal'".($viewDisplay=="normal" ? " selected" : "").">Normal display";
				echo "<option value='condensed'".($viewDisplay=="condensed" ? " selected" : "").">Condensed display";
				echo "</select></td></tr></form>\n";
				echo "<form method=post action='$self_view'><tr><th>Display automation</th><td><select name=chViewAutoDisplay onChange='submit()'>";
				echo "<option value='auto'".($viewAutoDisplay=="auto" ? " selected" : "").">Automatic display";
				echo "<option value='manual'".($viewAutoDisplay=="manual" ? " selected" : "").">Manual display";
				echo "</select></td></tr></form>\n";
				echo "<form method=post action='$self_view'><tr><th>Refresh rate</th><td><input name=chViewRefreshRate value='".htmlspecialchars($viewRefreshRate, ENT_QUOTES)."' size=5 maxlength=10> seconds</td></tr></form>\n";
				echo "</table>\n";
			}
			else
			{
				$viewFilter_html = htmlspecialchars($viewFilter, ENT_QUOTES);
				echo "<b>Filter ".($ownView ? "<input name=chViewFilter value='$viewFilter_html' size=64 maxlength=64>" : $viewFilter_html)."</b><br>";
				echo ($viewDisplay == "condensed" ? "Condensed" : "Normal")." display, ";
				echo ($viewAutoDisplay == "auto" ? "Automatic" : "Manual")." display";
			}
			echo "</td></tr>";
			echo "</table>\n";
			
			echo "</td><td width=20>\n";
			echo "</td><td>\n";
			
			if ($ownView)
			{
				echo "<b>Available variables:</b><br><font size=0>(click a variable to add it to view ".htmlspecialchars($viewName, ENT_QUOTES).")</font><br>\n";
	
				echo "<table border=1 cellspacing=1>\n";
				echo "<tr><th>Variable</th>";
				echo "<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?tid=".intval($tid)."'><th><select name=sel_vgid onChange='submit()'>\n";
				foreach ($vargroups as $vargroup => $vgid)
					echo "<option value='".intval($vgid)."'".($sel_vgid == $vgid ? " selected" : "").">".htmlspecialchars($vargroup, ENT_QUOTES)."\n";
				echo "<option value='-1'".(!isset($sel_vgid) || $sel_vgid=='-1' ? " selected" : "").">All Groups\n";
				echo "</select></th></form>\n";
				echo "<th>Path</th><th>State</th><th>Privilege</th></tr>\n";
	
				$result = sqlquery("SELECT command, variable.vid AS vid, variable.name AS name, path, state, variable.vgid AS vgid, variable_group.name AS group_name ".
									"FROM variable, variable_group ".
									"WHERE variable.vgid = variable_group.vgid".(isset($sel_vgid) && $sel_vgid!="-1" ? " AND variable.vgid='".intval($sel_vgid)."'" : "")." ORDER BY variable.command, variable.vgid, variable.name");
				if (!$result)
					die("variable select failed !");
	
				$prevvgid = "";
				$prevvtype = "";
	
				while ($arr = sqlfetch($result))
				{
					$vid = intval($arr["vid"]);
					
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
	
					echo "<tr><td><b><a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?addToView=$vid&tid=".intval($tid)."&sel_vgid=".intval($sel_vgid)."'>".htmlspecialchars($arr["name"], ENT_QUOTES)."</a></b></td>".
								"<td>".htmlspecialchars($arr["group_name"], ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($arr["path"], ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($arr["state"], ENT_QUOTES)."</td>".
								"<td>".htmlspecialchars($priv, ENT_QUOTES)."</td></tr>\n";
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
				$result = sqlquery("SELECT name, command FROM view_command WHERE tid='".intval($tid)."' ORDER BY name");
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
