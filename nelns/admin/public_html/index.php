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

	include('authenticate.php');


	include('display_view.php');


	if (isset($reset_filters))
	{
		//$filter_shard = "";
		$filter_server = "";
		$filter_service = "";
		$filter_entity = "";
	}

	$HTTP_VARS = array_merge($HTTP_POST_VARS, $HTTP_GET_VARS);

	unset($sel);
	unset($tid);
	unset($tname);
	unset($updateResult);
	unset($overriden);
	
	if (isset($active_player) && $active_player != "")
	{
		$arr = explode(',', $active_player);
		foreach($arr as $player)
		{
			$pl = explode(' ', $player);
			nel_query("*.*.EGS.loadPlayer ".$pl[0], $dummyResult);
			nel_query("*.*.EGS.activePlayer ".$pl[0]." ".$pl[1], $dummyResult);
		}
	}
	
	if (isset($current_tid) && $current_tid != "")
		$tid = $current_tid;
		
	$updatedShardAnnot = false;

	// select table and display name
	foreach ($HTTP_VARS as $var => $value)
	{
		if (strncmp($var, "selectTid_", 10) == 0)
			$tid = (int)substr($var, 10);
		else if (strncmp($var, "upd_annot_button_", 17) == 0)
		{
			$shard = substr($var, 17);
			
			if ($shardLockState[$shard]['lock_state'] == 1)
			{
				$annot = $GLOBALS["upd_annot_text_".$shard];
				
				$result = sqlquery("SELECT shard FROM shard_annotation WHERE shard='$shard'");
				if (!$result || sqlnumrows($result) == 0)
					sqlquery("INSERT INTO shard_annotation SET shard='$shard'");
	
				sqlquery("UPDATE shard_annotation SET annotation='$annot', user='$uid', post_date=NOW() WHERE shard='$shard'");
				
				$updatedShardAnnot = true;
			}
		}
		else if (strncmp($var, "upd_lock_button_", 16) == 0)
		{
			$shard = substr($var, 16);

			if ($shardLockState[$shard]['lock_state'] != 1)
			{
				$result = sqlquery("SELECT shard FROM shard_annotation WHERE shard='$shard'");
	
				if (!$result || sqlnumrows($result) == 0)
					sqlquery("INSERT INTO shard_annotation SET shard='$shard', annotation='locked by $admlogin', user='$uid', post_date=NOW()");
	
				sqlquery("UPDATE shard_annotation SET lock_user='$uid', lock_ip='$REMOTE_ADDR', lock_date=NOW() WHERE shard='$shard'");
				
				$updatedShardAnnot = true;
			}
		}
		else if (strncmp($var, "upd_unlock_button_", 18) == 0)
		{
			$shard = substr($var, 18);

			if ($shardLockState[$shard]['lock_state'] == 1)
			{
				$result = sqlquery("SELECT shard FROM shard_annotation WHERE shard='$shard'");
				if (!$result || sqlnumrows($result) == 0)
					sqlquery("INSERT INTO shard_annotation SET shard='$shard', annotation='locked by $admlogin', user='$uid', post_date=NOW()");
	
				sqlquery("UPDATE shard_annotation SET lock_user='0' WHERE shard='$shard'");
				
				$updatedShardAnnot = true;
			}
		}
	}
	
	if ($updatedShardAnnot)
	{
		getShardLockState();
	}
	

	if (isset($select_view) && $select_view != "")
		$tname = explode(',', $select_view);

	if (isset($tname) && count($tname)>0 && !isset($tid))
	{
		$found = false;
		foreach($tname as $tbname)
		{
			$result = sqlquery("SELECT tid, name FROM view_table WHERE uid='$uid' AND name='$tbname'");
			if ($result && ($arr=sqlfetch($result)))
			{
				$tid = $arr["tid"];
				$tname = $tbname;
				$found = true;
				break;
			}

			$result = sqlquery("SELECT tid, name FROM view_table WHERE uid='$gid' AND name='$tbname'");
			if ($result && ($arr=sqlfetch($result)))
			{
				$tid = $arr["tid"];
				$tname = $tbname;
				$found = true;
				break;
			}
		}
		
		if (!$found)
		{
			unset($tname);
		}
	}

	if (!isset($tid))
	{
		$result = sqlquery("SELECT default_view FROM user, view_table WHERE user.uid='$uid' AND (view_table.uid='$uid' OR view_table.uid='$gid') AND view_table.tid=user.default_view");
		if ($result && ($arr=sqlfetch($result)))
		{
			$tid=$arr["default_view"];
			if ($tid == 0)
				unset($tid);
		}
		
		if (!isset($tid))
		{
			$result = sqlquery("SELECT tid, name FROM view_table WHERE uid='$uid' ORDER BY ordering LIMIT 1");
			if ($result && ($arr=sqlfetch($result)))
			{
				$tid = $arr["tid"];
				$tname = $arr["name"];
			}
			else
			{
				$result = sqlquery("SELECT tid, name FROM view_table WHERE uid='$gid' ORDER BY ordering LIMIT 1");
				if ($result && ($arr=sqlfetch($result)))
				{
					$tid = $arr["tid"];
					$tname = $arr["name"];
				}
			}
		}
	}

	$refreshRate = 0;
	
	if (isset($tid))
		$current_tid = $tid;
		
	$result = sqlquery("SELECT name, refresh_rate FROM view_table WHERE tid='$tid'");
	if ($result && ($result=sqlfetch($result)))
	{
		$tname = $result["name"];
		$refreshRate = $result["refresh_rate"];
	}

	// get selection
	foreach ($HTTP_VARS as $var => $value)
	{
		if (strncmp($var, "select_", 7) == 0)
		{
			$sel_arr = explode(":", substr($var, 7));
			
			if (!$keep_shard)	$sel_arr[0] = '*';
			if (!$keep_server)	$sel_arr[1] = '*';
			if (!$keep_service)	$sel_arr[2] = '*';
			
			$sel[] = $sel_arr[0].".".$sel_arr[1].".".$sel_arr[2].".*";
		}
		else if (strncmp($var, "execServCommand_", 16) == 0)
		{
			$execServCommand = substr($var, 16);
		}
	}

	if (!isset($sel) || !is_array($sel))
		$sel = array("*.*.*.*");
		
	unset($shards);
	$selShards = array();

	// add shard access to selection
	if ($admlogin != "root" && !$IsNevrax)
	{
		if (count($shardAccess)>0)
		{
			$unfiltered = $sel;
			unset($sel);
			foreach ($shardAccess as $sshard)
			{
				$shards[] = $sshard["shard"];
				$filtershard = $sshard["shard"];
				foreach ($unfiltered as $filter)
				{
					list($shard, $server, $service, $variable) = explode(".", $filter);
					if ($shard == "*" || $shard == $filtershard)
					{
						$sel[] = "$filtershard.$server.$service.$variable";
					}
				}
			}
		}
		else
		{
			unset($sel);
			$shards = array();
		}
	}
	else
	{
		$result = sqlquery("SELECT DISTINCT shard FROM service ORDER BY shard");
		while ($result && ($arr=sqlfetch($result)))
		{
			$shards[] = $arr["shard"];
		}
	}

	unset($cmdResult);

	if (isset($execServCommand) && isset($factPaths) && isset($execServParams) && isset($variableData[$execServCommand]))
	{
		// get command path
		$cmd = $variableData[$execServCommand];
		$path = $cmd["path"];
		
		$paths = expandQuery($factPaths);

		for ($i=0; $i<count($paths); ++$i)
		{
			$fpath = filterPathUsingAliases($paths[$i], $path);
			if ($fpath != "")
				$newPaths[] = $fpath;
		}

		$fullPath = factorizeQuery("[".join($newPaths, ",")."]");

		// filter selection with command
		$fullCmd = $fullPath." ".$execServParams;
		logUser($uid, "COMMAND=".$fullCmd);
		$qstate = nel_query($fullPath." ".$execServParams, $cmdResult);
	}

	unset($ownerTables);

	// display available user and group views
	$result = sqlquery("SELECT view.name AS name, view.tid AS tid, view.uid AS gid, user.login AS owner FROM view_table AS view, user WHERE view.uid=user.uid AND (view.uid='$uid' OR view.uid='$gid') ORDER BY gid, ordering");
	if ($result)
	{
		$owner = "";
		while ($arr = sqlfetch($result))
			$ownerTables[$arr["owner"]][] = $arr;
	}

	htmlProlog($_SERVER['PHP_SELF'], "View Selection '$tname'", true);

	if (isset($tname))
		$current_tname = $tname;

	$use_refreshRate = (isset($form_refreshRate) && $form_refreshRate != 0 ? $form_refreshRate : $refreshRate);

	if ($use_refreshRate > 0)
	{
		echo "<script><!--\n";
		echo "	var sURL = unescape(window.location.pathname);\n";
		echo "	var pos = sURL.indexOf('.php');\n";
		echo "	function refresh() { window.location.replace( sURL ); }\n";
		echo "	if (pos >= 0) {\n";
		echo "		sURL = sURL.substr(0, pos+4)+'?current_tid=$tid&form_refreshRate=$form_refreshRate';\n";
		echo "		setTimeout(\"refresh()\", ".($use_refreshRate*1000).");\n";
		echo "	}\n";
		echo "//--></script>\n";
	}

	echo "<br>\n";
	echo "<table width=100%><form method=post action='".$_SERVER['PHP_SELF']."' name='viewForm'><tr valign=top>\n";

	if (count($ownerTables)>0)
	{
		$clr = array("#E8E8E8", "#E0E0E0");
		$o = 0;
		foreach ($ownerTables as $owner => $ownerViews)
		{
			$i=0;
			$bgcolor=$clr[$o];
			$o = 1-$o;
			foreach ($ownerViews as $arr)
			{
				if ($i%4 == 0)
				{
					if ($i>0)
						echo "</table></td><td width=150 bgcolor=$bgcolor><table><tr><th>&nbsp;</th></tr>\n";
					else
						echo "<td width=150 bgcolor=$bgcolor><table><tr><th align=left>$owner views</th></tr>\n";
				}
				if ($tname == $arr["name"])
					echo "<tr><td nowrap><input type=submit name='selectTid_".$arr["tid"]."' value='View'> <b>".$arr["name"]."</b></td></tr>";
				else
					echo "<tr><td nowrap><input type=submit name='selectTid_".$arr["tid"]."' value='View'> ".$arr["name"]."</td></tr>";
				
				++$i;
			}
			echo "</table></td>\n";
		}
	}
	
	echo "<td width=40>&nbsp;</td>\n";

	//
	$use_filter_shard = $filter_shard;
	$use_filter_server = $filter_server;
	$use_filter_service = $filter_service;
	$use_filter_entity = $filter_entity;
	
	if (!isset($use_filter_shard)   || $use_filter_shard == "")			$use_filter_shard = "*";
	if (!isset($use_filter_server)  || $use_filter_server == "")		$use_filter_server = "*";
	if (!isset($use_filter_service) || $use_filter_service == "")		$use_filter_service = "*";
	if (!isset($use_filter_entity)  || $use_filter_entity == "")		$use_filter_entity = "*";
	
	$view_filter = $use_filter_shard.".".$use_filter_server.".".$use_filter_service.".".$use_filter_entity;

	$selAllShards = false;
	for ($i=0; $i<count($sel); ++$i)
	{
		$sel[$i] = partialFilterPath($sel[$i], $view_filter);
		
		$expSel = explode('.', $sel[$i]);
		if ($expSel[0] == '*')
			$selAllShards = true;
		$selShards[$expSel[0]] = true;
	}
	//
	echo "<td nowrap bgcolor=#E0E0E0>\n";
	if (count($shards) > 0)
	{
		echo "<b>Shard shortcuts</b><table><tr valign=top><td>\n";
		$i = 1;
		echo "<table>";
		echo "<tr><td><a href='".$_SERVER['PHP_SELF']."?current_tid=$tid&form_refreshRate=$form_refreshRate&filter_shard=$link_shard&filter_server=&filter_service='><i>All</i></a></td></tr>";
		foreach ($shards as $link_shard)
		{
			if ($i%5 == 0 && $i>0)
				echo "</table></td><td><table>\n";
			echo "<tr>";
			echo ($selShards[$link_shard]) ? '<td nowrap bgcolor=#C0C0C0><b>' : '<td nowrap>';
			$alias = $shardLockState[$link_shard]['alias'];
			if ($alias == '')
				$alias = $link_shard;
			else
				$alias .= "($link_shard)";
			echo "<a href='".$_SERVER['PHP_SELF']."?current_tid=$tid&form_refreshRate=$form_refreshRate&filter_shard=$link_shard&filter_server=&filter_service='>$alias</a>";
			if ($selShards[$link_shard])
				echo "</b>";
			echo "</td></tr>";
			++$i;
		}
		echo "</table>";
		echo "</td></tr></table>\n";
	}
	echo "</td>\n";

	echo "<td width=100% align=right>\n";
	echo "<table><tr><th>Refresh rate</th></tr>\n";
	echo "<tr><td><select name=form_refreshRate onChange='submit()'>\n";
	echo "<option value='0'".(!isset($form_refreshRate) || $form_refreshRate==0 ? " selected" : "").">View setting (".($refreshRate>0 ? "$refreshRate seconds" : "no refresh").")\n";
	echo "<option value='-1'".($form_refreshRate==-1 ? " selected" : "").">No refresh\n";
	echo "<option value='30'".($form_refreshRate==30 ? " selected" : "").">30 seconds\n";
	echo "<option value='60'".($form_refreshRate==60 ? " selected" : "").">60 seconds\n";
	echo "<option value='120'".($form_refreshRate==120 ? " selected" : "").">2 minutes\n";
	echo "<option value='300'".($form_refreshRate==300 ? " selected" : "").">5 minutes\n";
	echo "</select></td></tr></table>\n";
	echo "</td>\n";

	echo "<td align=right><table><tr><th width=300>Selection</th></tr>\n";
	if (count($sel) > 0)
		foreach ($sel as $s)
			echo "<tr><td>$s</td></tr>\n";
	echo "</table></td>\n";
	
	echo "</tr></table><br>Content of view <font size=3><b>$tname</b></font><br><br>\n";

	echo "<table><tr><td>\n";
	
	$queryErrors = array();

	// update values
	if ($upd_values)
	{
		unset($update);
		$tid = $current_tid;

		foreach ($HTTP_VARS as $var => $value)
			if (strncmp($var, "override_", 9) == 0)
				$overriden[substr($var, 9)]=$value;
				
		$shardAccessForbidden = array();

		foreach ($HTTP_VARS as $var => $value)
		{
			if (strncmp($var, "updvar_", 7) == 0)
			{
				$vv = substr($var, 7);
				$prevValue = $GLOBALS["prevvar_$vv"];

				$var_split = explode("|", $vv);
				$shard = $var_split[0];
				$var_split = $var_split[count($var_split)-1];
				$override = $overriden[$var_split];
				
				if ($shardLockState[$shard]['lock_state'] != 1)
				{
					if (!$shardAccessForbidden[$shard])
					{
						$shardAccessForbidden[$shard] = true;
						$queryErrors[] = "You can't update values on shard '$shard', access is not locked for you";
					}
					continue;
				}

				if ($value != $prevValue)
				{
					$address = str_replace("|", ".", $vv);
					$update[] = "$address=$value";
				}
				else if (isset($override) && $override != "" && $override != $prevValue)
				{
					$address = str_replace("|", ".", $vv);
					$update[] = "$address=$override";
				}
			}
			else if (strncmp($var, "current_select_", 15) == 0)
			{
				$sel[] = $value;
			}
		}

		$sel = array_unique($sel);

		if (isset($update) && count($update > 0))
		{
			if (count($update) > 1)
			{
				$query = "[".join(",", $update)."]";
				$query = factorizeQuery($query);
			}
			else
				$query = $update[0];
				
			$executeQuery = $query;

			$bef = microtime();
			logUser($uid, "UPDATE=".$executeQuery);
			$qstate = nel_query($executeQuery, $updateResult);
			$aft = microtime();
	
			list($usec, $sec) = explode(" ", $bef);
			$bef = ((float)$sec + (float)$usec);
			list($usec, $sec) = explode(" ", $aft);
			$aft = ((float)$sec + (float)$usec);
			$tm = (int)(($aft-$bef)*1000.0);

			$queryResult = "Executed $executeQuery<br>$tm milliseconds computation time<br>\n";
		}
	}
	else if (isset($executeQuery))
	{
		$bef = microtime();
		$qstate = nel_query($executeQuery, $updateResult);
		$aft = microtime();

		list($usec, $sec) = explode(" ", $bef);
		$bef = ((float)$sec + (float)$usec);
		list($usec, $sec) = explode(" ", $aft);
		$aft = ((float)$sec + (float)$usec);
		$tm = (int)(($aft-$bef)*1000.0);

		$queryResult = "Executed $executeQuery<br>$tm milliseconds computation time<br>\n";
	}

	if ($updateResult)
	{
		buildVariableEnv($uid, $gid, $tid, $vardisp, $bounds, $privilege, $tree, $condensed, $autoDisplay);
		displayResult($updateResult, $vardisp, $bounds, $privilege);
	}
	else
	{
		// send select request
		if (count($sel) > 0)
			$sel = array_unique($sel);

		if (count($sel) == 1 && $sel[0] == "")
			unset($sel);

		$queryResult = displayViewTable($uid, $gid, $tid, $sel);
	}

	echo "<input type=hidden name='current_tid' value='$tid'>\n";
	$i=0;
	if (count($sel) > 0)
		foreach ($sel as $selec)
			echo "<input type=hidden name='current_select_".($i++)."' value='$selec'>\n";

	if (isset($tid) && $tid != "")
	{
		$result = sqlquery("SELECT view_row.name AS name, variable.vid AS vid FROM view_row, variable WHERE tid='$tid' AND variable.command='command' AND view_row.vid=variable.vid ORDER BY ordering");
		if ($result && sqlnumrows($result) > 0)
		{
			echo "<br><br><b>Service commands</b> <font size=1>The commands are sent to all services seen in the view above</font><br>\n";
			echo "Command parameters <input name=execServParams size=64 maxlength=128><br>\n";
			echo "<table>\n";
			echo "<tr valign=top>\n";
			echo "<td><table>\n";

			$numInCol = 0;
			while ($result && ($arr=sqlfetch($result)))
			{
				if ($numInCol >= 5 || $arr["name"] == 'SEPARATOR')
				{
					echo "</table></td><td width=30></td><td><table>\n";
					$numInCol = 0;
				}
				if ($arr["name"] != 'SEPARATOR')
				{
					echo "<tr><td><input type=submit name=execServCommand_".$arr["vid"]." value='".$arr["name"]."'></td></tr>\n";
					++$numInCol;
				}
			}

			if (isset($listPath) && count($listPath) > 0)
			{
				$address = "[".join(",", $listPath)."]";
				$address = factorizeQuery($address);

				echo "<input type=hidden name=factPaths value='$address'>\n";
			}

			echo "</table></td>\n";
			echo "</tr></table>\n";
			
			if (isset($cmdResult))
			{
				echo "<br>Result of command '$fullPath $execServParams':<br>\n";
				echo "<table border=1><tr><td>\n";
				echo "<pre>$cmdResult</pre>\n";
				echo "</td></tr></table>\n";
			}
		}
	}
	
	if (count($queryErrors) > 0)
	{
		echo "<br><font size=3><b>Execution errors:</b></font>\n";
		foreach ($queryErrors as $error)
			echo "<br><font size=3 color=#FF0000><b>$error</b></font>\n";
	}
	echo "<br><br><font size=0>$queryResult</font>\n";
	echo "</td></form></tr></table>\n";

	function lvcmp($a, $b)
	{
		return ($a["path"] == $b["path"] ? 0 : $a["path"] < $b["path"] ? -1 : 1);
	}

	if (isset($tid) && $tid != "" && count($listPath) > 0)
	{
		$result = sqlquery("SELECT view_row.name AS name, variable.vid, variable.path, warning_bound, error_bound, alarm_order FROM view_row, variable WHERE tid='$tid' AND variable.vid=view_row.vid AND graph!='0' ORDER BY name");
		
		if ($result && sqlnumrows($result) > 0)
		{
			unset($listVars);
			
			while ($result && ($arr=sqlfetch($result)))
			{
				$gname = $arr["name"];
				$gvid = $arr["vid"];
				$gfilter = $arr["path"];

				$gwarn = $arr["warning_bound"];
				$gerr = $arr["error_bound"];
				$gord = $arr["alarm_order"];

				for ($i=0; $i<count($listPath); ++$i)
				{
					$path = $listPath[$i].".*";
					$varpath = filterPathUsingAliases($path, $gfilter);
					$rrdpath = $rrdrootpath."/".$varpath.".rrd";
					if ($varpath != "" && file_exists($rrdpath))
						$listVars[] = array("path" => $varpath, "name" => $gname, "warn" => $gwarn, "err" => $gerr, "order" => $gord);
				}
			}

			if (count($listVars) > 0)
			{
				usort($listVars, "lvcmp");

				echo "<br><br><b>View Graphs</b><br><br>\n";
				echo "<table><tr valign=top>\n";
				echo "<td><table>\n";
			
				$counter = 0;

				// remove too old picture files
				// opendir for scanning gif files
				if ($dir = @opendir($gifoutputpath))
				{
					unset($eraseFiles);
					$curtime = time();
					while($file = readdir($dir))
					{
						if (substr($file, 0, 7) == "tmppic_")
						{
							$fstats = stat("$gifoutputpath/$file");
							if ($fstats[9] < $curtime - $gifpersistence)
								$eraseFiles[] = "$gifoutputpath/$file";
						}
					}

					closedir($dir);

					if (count($eraseFiles) > 0)
						foreach ($eraseFiles as $file)
							unlink($file);

				}

				mt_srand((float) microtime()*1000000);
				$randFilename = "tmppic_".mt_rand(10000, 99999);

				foreach ($listVars as $var)
				{
					$rrdvar = $var["path"];
					$rrdpath = $rrdrootpath."/".$rrdvar.".rrd";
					$rrdname = $var["name"];
					$rrdwarn = $var["warn"];
					$rrderr = $var["err"];
					$rrdord = strtoupper($var["order"]);

					echo "<tr><th>$rrdvar <font size=0>(using path $rrdpath)</font></th></tr>\n";

					// generate a temp filename
					
					$tempFilename_0 = "$gifhttplocation/".$randFilename."_".$counter."_0.gif";
					$tempFilename_1 = "$gifhttplocation/".$randFilename."_".$counter."_1.gif";
					$tempFilename_2 = "$gifhttplocation/".$randFilename."_".$counter."_2.gif";

					$tempFilenameout_0 = "$gifoutputpath/".$randFilename."_".$counter."_0.gif";
					$tempFilenameout_1 = "$gifoutputpath/".$randFilename."_".$counter."_1.gif";
					$tempFilenameout_2 = "$gifoutputpath/".$randFilename."_".$counter."_2.gif";
					
					++$counter;

					// generate picture according to the temp filename

					unset($result);

					$rrdDEF = "DEF:val=$rrdpath:var:AVERAGE";
					$rrdDraw = "";
					
					if ($rrdwarn != -1)
					{
						$rrdDEF .= " CDEF:warn=val,$rrdwarn,$rrdord,val,0,IF";
						$rrdDraw .= "AREA:warn#FFCC88 ";
					}

					if ($rrderr != -1)
					{
						$rrdDEF .= " CDEF:err=val,$rrderr,$rrdord,val,0,IF";
						$rrdDraw .= "AREA:err#FF4422 ";
					}

					$rrdDraw .= "LINE2:val#0000FF";
					
					$execStr = "rrdtool graph $tempFilenameout_0 --start -1200 $rrdDEF $rrdDraw";
					//echo "exec(\"$execStr\")<br>\n";
					exec($execStr, $result, $retcode1);
//					echo "<tr><td><img src='$tempFilename_0'></td></tr>";

					$execStr = "rrdtool graph $tempFilenameout_1 --start -10800 $rrdDEF $rrdDraw";
					//echo "exec(\"$execStr\")<br>\n";
					exec($execStr, $result, $retcode2);
//					echo "<tr><td><img src='$tempFilename_1'></td></tr>";

					$execStr = "rrdtool graph $tempFilenameout_2 --start -86400 $rrdDEF $rrdDraw";
					//echo "exec(\"$execStr\")<br>\n";
					exec($execStr, $result, $retcode3);
//					echo "<tr><td><img src='$tempFilename_2'></td></tr>";
					echo "<tr><td><img src='$tempFilename_0'><img src='$tempFilename_1'><img src='$tempFilename_2'></td></tr>";

					echo "<tr height=10><td colspan=1></td></tr>";
					
					if ($retcode1 || $retcode2 || $retcode3)
					{
						echo "<b>RRDTool output:</b><br>\n";
						print_r($result);
						echo "<br>\n";
					}
				}

				echo "</table></td>\n";
				echo "</tr></table>\n";
			}
		}
	}

	htmlEpilog();

	//print_r($shardLockState);
?>
