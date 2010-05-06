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

	global $HTTP_POST_VARS, $HTTP_GET_VARS;

	$publicAccess = false;
	$allowNevrax = true;
	include('authenticate.php');

	include('request_interface.php');
	include_once('las_connection.php');

	if (!isset($_POST) && isset($HTTP_POST_VARS))
		$_POST = &$HTTP_POST_VARS;
	if (!isset($_GET) && isset($HTTP_GET_VARS))
		$_GET = &$HTTP_GET_VARS;

	function	importParam($var)
	{
		global	$_POST, $_GET;
		if (isset($_POST[$var]))
			$GLOBALS[$var] = $_POST[$var];
		else if (isset($_GET[$var]))
			$GLOBALS[$var] = $_GET[$var];
		else
			unset($GLOBALS[$var]);
	}

	htmlProlog($_SERVER['PHP_SELF'], "Log Analysis");

	// look for LAS
	$las_query = "*.*.LAS.State";
	$qstate = nel_query($las_query, $result);

	unset($availableLAS);
	if ($qstate)
	{
		$arr = explode(' ', $result);
		$numRes = count($arr);
		$numRows = current($arr);
		$numLines = ($numRes-$numRows-2)/$numRows;
		next($arr);
		for ($i=0; $i<$numRows; ++$i)
		{
			$vars[] = current($arr);
			next($arr);
		}
		unset($shards);
		for ($i=0; $i<$numLines; ++$i)
		{
			unset($l);
			foreach($vars as $var)
			{
				$l[$var] = current($arr);
				next($arr);
			}
			
			$sql_query = "SELECT * FROM server WHERE name='".$l['server']."'";
			$sql_res = sqlquery($sql_query);
			if ($sql_res && ($sql_arr = sqlfetch($sql_res)))
				$l['address'] = $sql_arr['address'];

			$availableLAS[] = $l;
		}
	}
/*
	echo "<pre>";
	print_r($availableLAS);
	echo "</pre>";
*/
	importParam('exec_query');
	importParam('refresh_result');
	importParam('query');
	importParam('query_id');
	importParam('page');

	importParam('build_display_query');
	importParam('build_eid_query');
	importParam('database');
	for ($i=0; $i<10; ++$i)
		importParam("eid_$i");
	importParam('string');
	importParam('start_date');
	importParam('end_date');

	$string = stripslashes($string);
	$query = stripslashes($query);

	// ------------------------------------------------------------------------------------------------------------------------
	
	echo "<br>";

	echo "<table border=0>\n";
	echo "<form method='post' action='".$_SERVER['PHP_SELF']."'>\n";

	echo "<tr valign=top>\n";

	$selectedLAS = '';

	// mandatory information
	echo "<td>\n";
	echo "<table>\n";
	echo "<tr><th align=left>LAS Host</th></tr>\n";
	echo "<tr><td>";
	echo "<select name='las_address'>\n";
	if (count($availableLAS) > 0)
	{
		foreach ($availableLAS as $las)
		{
			if ($las_address == $las['address'])
			{
				$selectedLAS = $las;
				echo "<option value='".$las['address']."' selected>".$las['shard']." ".$las['server']." ".$las['service']."\n";
			}
			else
			{
				echo "<option value='".$las['address']."'>".$las['shard']." ".$las['server']." ".$las['service']."\n";
			}
		}
	}
	if (!$selectedLAS['address'])
	{
		echo "<option value='' selected>---- Select a LAS Host";
	}
	else
	{
		echo "<option value=''>---- Select a LAS Host";
	}
	echo "</select>\n";
	echo "</td></tr>";
	echo "<tr><th align=left>Database</th></tr>\n";
	echo "<tr><td><input type='text' name='database' size=10 maxlength=10 value='$database'></td></tr>";
	echo "<tr><th align=left>Start Date</th></tr>\n";
	echo "<tr><td><input type='text' name='start_date' size=30 maxlength=20 value='$start_date'></td></tr>";
	echo "<tr><th align=left>End Date</th></tr>\n";
	echo "<tr><td><input type='text' name='end_date' size=30 maxlength=20 value='$end_date'></td></tr>";
	echo "</table>\n";
	echo "</td>\n";

	// search by eid
	echo "<td>\n";
	echo "<table>\n";
	echo "<tr><th align=left>EntityIds</th></tr>\n";
	echo "<tr><td>\n";
	for ($i=0; $i<10; ++$i)
		echo "<input id='eid_$i' type='text' name='eid_$i' size=30 maxlength=25 value='".$GLOBALS["eid_$i"]."'><br>\n";
	echo "</td></tr>\n";
	echo "</table>\n";
	echo "</td>\n";

	echo "<script>\n";
	echo "<!--\n";
	echo "	function reorderEId()\n";
	echo "	{\n";
	echo "		lastNull = -1;\n";
	echo "		for (i=0; i<10; ++i)\n";
	echo "		{\n";
	echo "			if (document.getElementById('eid_'+i).value == '')\n";
	echo "				lastNull = i;\n";
	echo "			else if (lastNull >= 0)\n";
	echo "			{\n";
	echo "				document.getElementById('eid_'+lastNull).value = document.getElementById('eid_'+i).value\n";
	echo "				document.getElementById('eid_'+i).value = ''\n";
	echo "				++lastNull;\n";
	echo "			}\n";
	echo "		}\n";
	echo "	}\n";
	echo "	function selectEId(id)\n";
	echo "	{\n";
	echo "		reorderEId();\n";
	echo "		for (i=0; i<10; ++i)\n";
	echo "			if (document.getElementById('eid_'+i).value == id)\n";
	echo "				return;\n";
	echo "		for (i=0; i<10; ++i)\n";
	echo "			if (document.getElementById('eid_'+i).value == '')\n";
	echo "			{\n";
	echo "				document.getElementById('eid_'+i).value = id;\n";
	echo "				return;\n";
	echo "			}\n";
	echo "	}\n";
	echo "	reorderEId();\n";
	echo "-->\n";
	echo "</script>\n";

	// search by string
	echo "<td>\n";
	echo "<table>\n";
	echo "<tr><th align=left>String</th></tr>\n";
	echo "<tr><td><input type='text' name='string' size=100 maxlength=40 value='$string'></td></tr>";
	echo "</table>\n";
	echo "</td>\n";

	echo "</tr>\n";
	echo "<tr valign=top>\n";

	echo "<td align=center><input type='submit' name='build_display_query' value='Display'></td>";
	echo "<td align=center><input type='submit' name='build_eid_query' value='Search Id(s)'></td>";
	echo "<td align=center><input type='submit' name='build_string_query' value='Search String'></td>";

	echo "</tr>\n";

	echo "</tr>\n";
	echo "<tr valign=top>\n";

	echo "<td align=center><input type='submit' name='display_queries' value='Display Queries'></td>";

	echo "</tr>\n";

	echo "</form>\n";
	echo "</table>\n";

	// ------------------------------------------------------------------------------------------------------------------------

	echo "<br>\n";


	if ($build_eid_query || $build_display_query)
	{
		$eids = array();
		for ($i=0; $i<10; ++$i)
			if ($GLOBALS["eid_$i"] != '')
				$eids[] = $GLOBALS["eid_$i"];

		if (count($eids) == 0 || $build_display_query)
		{
			$query = "displayLogs $database $start_date";
			if ($end_date != '')
				$query .= " $end_date";
			$exec_query = true;
		}
		else if (count($eids) > 1)
		{
			$query = "searchEIds $database ".join(' ', $eids)." - $start_date";
			if ($end_date != '')
				$query .= " $end_date";
			$exec_query = true;
		}
		else
		{
			$query = "searchEId $database ".$eids[0]." $start_date";
			if ($end_date != '')
				$query .= " $end_date";
			$exec_query = true;
		}
	}
	else if ($build_string_query)
	{
		if ($string != '')
		{
			$query = "searchString $database \"$string\" $start_date";
			if ($end_date != '')
				$query .= " $end_date";
			$exec_query = true;
		}
	}

/*
	// display query input
	echo "<table>\n";
	echo "<form method='post' action='$_SERVER['PHP_SELF']'>\n";
	echo "<tr>\n";
	echo 	"<td>Query</td>";
	echo 	"<td><input type=text name='query' value='$query' size=128 maxlength=255></td>";
	echo 	"<td><input type=submit name='exec_query' value='Submit Query'></td>\n";
	echo "</tr>\n";
	echo "<input type='hidden' name='database' value='$database'>\n";
	for ($i=0; $i<10; ++$i)
		echo "<input type='hidden' name='eid_$i' value='".$GLOBALS["eid_$i"]."'>\n";
	echo "<input type='hidden' name='start_date' value='$start_date'>\n";
	echo "<input type='hidden' name='end_date' value='$end_date'>\n";
	echo "</form>\n";
	echo "</table>\n";
*/

	if ($display_queries && $selectedLAS['address'])
	{
		$success = displayLASQueries($selectedLAS['address'], $result);
		
		if ($success)
		{
			$a = explode("\n", $result);
			if (count($a) > 0)
			{
				echo "<table border=1 width='100%'><tr><td>";
				echo "<table width='100%'>\n";
				$i = 0;
				foreach ($a as $query)
				{
					if ($query == '')
						continue;

					$qa = explode(':', $query, 4);

					$bg = (($i & 1) == 0 ? '#F8F8FF' : '#F0F0F8');
					++$i;
					echo "<tr bgcolor=$bg>";
					if ($qa[1] == 2)
					{
						echo "<td>".$qa[0]."</td>";
						$refstr = $_SERVER['PHP_SELF']."?refresh_result=1&las_address=".$selectedLAS['address']."&query_id=".$qa[0]."&query=".$qa[2]."&database=$database&string=$string&start_date=$start_date&end_date=$end_date";
						echo "<td><a href='$refstr'>Display</a></td>";
						echo "<td>".$qa[3]."</td>";
					}
					else if ($qa[1] == 1)
					{
						echo "<td>".$qa[0]."</td>";
						echo "<td>Processing ".$qa[2]."%</td>";
						echo "<td>".$qa[3]."</td>";
					}
					else
					{
						echo "<td>".$qa[0]."</td>";
						echo "<td>Cancel</td>";
						echo "<td>".$qa[3]."</td>";
					}
					echo "</tr>\n";
				}
				echo "</table>";
				echo "</td></tr></table>";
			}
		}
	}
	else if ($exec_query && $query && $selectedLAS['address'])
	{
		$success = logQuery($selectedLAS['address'], $query, $result, $query_id);

		echo "<br><br>";
		
		if ($success)
		{
			echo "<b>Query '$query' successfully executed ($result)</b><br>\n";
			echo "Please wait while result is being computed and click 'Refresh result' to display query result.<br>\n";
			echo "<form method='post' action='".$_SERVER['PHP_SELF']."'>\n";
			echo "<input type=submit name='refresh_result' value='Refresh result'>\n";
			echo "<input type=hidden name='las_address' value='".$selectedLAS['address']."'>\n";
			echo "<input type=hidden name='query_id' value='$query_id'>\n";
			echo "<input type=hidden name='query' value='$query'>\n";
			echo "<input type='hidden' name='database' value='$database'>\n";
			for ($i=0; $i<10; ++$i)
				echo "<input type='hidden' name='eid_$i' value='".$GLOBALS["eid_$i"]."'>\n";
			echo "<input type='hidden' name='string' value='$string'>\n";
			echo "<input type='hidden' name='start_date' value='$start_date'>\n";
			echo "<input type='hidden' name='end_date' value='$end_date'>\n";
			echo "</form>\n";
		}
		else
		{
			echo "<b>Query '$query' failed</b>: '$result'<br>\n";
		}
	}
	else if ($refresh_result && isset($query_id) && $selectedLAS['address'])
	{
		$success = getQueryResult($selectedLAS['address'], $query_id, $result, $page, $numpages);

		echo "<br><br>";
		
		if ($success)
		{
			echo "<b>Query '$query' result</b>:<br>\n";
			
			echo "<b>";

			$refstr = $_SERVER['PHP_SELF']."?refresh_result=1&las_address=".$selectedLAS['address']."&query_id=$query_id&query=$query&database=$database&string=$string&start_date=$start_date&end_date=$end_date";
			for ($i=0; $i<10; ++$i)
				if ($GLOBALS["eid_$i"] != '')
					$refstr .= "&eid_$i=".$GLOBALS["eid_$i"];

			if ($page > 0)
				echo "<a href='$refstr&page=0'>&lt;&lt;</a>\n ";
			else
				echo "&lt;&lt; ";

			if ($page > 0)
				echo "<a href='$refstr&page=".($page-1)."'>&lt;</a>\n";
			else
				echo "&lt;";

			if ($page+1 < $numpages)
				echo " <a href='$refstr&page=".($page+1)."'>&gt;</a>\n";
			else
				echo " &gt;";

			if ($page+1 < $numpages)
				echo " <a href='$refstr&page=".($numpages-1)."'>&gt;&gt;</a>\n";
			else
				echo " &gt;&gt;";

			echo "</b>\n";

			if ($numpages < 20)
			{
				$minpage = 0;
				$maxpage = $numpages-1;
			}
			else
			{
				$minpage = $page-10;
				if ($minpage < 0)
					$minpage = 0;
				$maxpage = $minpage+20;
				if ($maxpage >= $numpages)
					$maxpage = $numpages-1;
				$minpage = $maxpage-20;
			}

			for ($p=$minpage; $p<=$maxpage; ++$p)
			{
				if ($p == $page)
				{
					echo " <b>$page</b>";
				}
				else
				{
					echo " <a href='$refstr&page=$p'>$p</a>\n";
				}
			}
			
			echo " [$numpages pages]";

			echo "<br>\n";

			$disp = htmlentities($result);
			
			echo "<table border=1 width='100%'><tr><td>";
			echo "<table width='100%'>\n";
			
			$a = explode("\n", $disp);
			$i = 0;
			foreach ($a as $l)
			{
				if (trim($l) == '')
					continue;
				$bg = (($i & 1) == 0 ? '#F8F8FF' : '#F0F0F8');
				++$i;
				$prefix = substr($l, 0, 3);
				echo "<tr bgcolor=$bg>";
				if ($prefix == '#! ')
				{
					echo "<td colspan=3><font color=#FF0000>";
					echo substr($l, 3);
					echo "</font></td>";
				}
				else if ($prefix == '#? ')
				{
					echo "<td colspan=3><font color=#008800>";
					echo substr($l, 3);
					echo "</font></td>";
				}
				else if ($prefix == '## ')
				{
					echo "<td colspan=3><font color=#0000FF>";
					echo substr($l, 3);
					echo "</font></td>";
				}
				else if ($prefix == '#$ ')
				{
					$al = explode(':', substr($l, 3), 4);
					echo "<td width=50>".$al[0]."</td>";
					echo "<td width=100>".trim($al[2])."</td>";
					
					$d = str_repeat('-&nbsp;', $al[1]).$al[3];
					
					$d = ereg_replace('(\(0x[0-9a-fA-F]{10}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\))', '<a onClick="return selectEId('."'".'\\1'."'".')">\\1</a>', $d);
					//$d = ereg_replace('(\(0x[0-9a-fA-F]{10}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\))', '<a onClick="return alert('."'".'tamere'."'".')">\\1</a>', $d);

					echo "<td>$d</td>";
				}
				else
				{
					echo "<td colspan=3><font color=#AAAAAA><i>// $l</i></font></td>";
				}
				echo "</tr>\n";
			}

			echo "</table>";
			echo "</td></tr></table>";
		}
		else
		{
			echo "<b>Failed to get query result</b>: '$result'<br>\n";
			echo "<form method='post' action='".$_SERVER['PHP_SELF']."'>\n";
			echo "<input type=submit name='refresh_result' value='Refresh result'>\n";
			echo "<input type=hidden name='las_address' value='".$selectedLAS['address']."'>\n";
			echo "<input type=hidden name='query_id' value='$query_id'>\n";
			echo "<input type=hidden name='query' value='$query'>\n";
			echo "<input type='hidden' name='database' value='$database'>\n";
			for ($i=0; $i<10; ++$i)
				echo "<input type='hidden' name='eid_$i' value='".$GLOBALS["eid_$i"]."'>\n";
			echo "<input type='hidden' name='string' value='$string'>\n";
			echo "<input type='hidden' name='start_date' value='$start_date'>\n";
			echo "<input type='hidden' name='end_date' value='$end_date'>\n";
			echo "</form>\n";
		}
	}
	else if (($exec_query || $refresh_result) && (!$selectedLAS['address']))
	{
		echo "<b>No LAS Host selected</b>, please restart query with a LAS Host specified.<br>\n";
	}
	
	echo "<br>";

	htmlEpilog();
?>
