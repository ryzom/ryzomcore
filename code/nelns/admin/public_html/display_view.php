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

	//include('fake_request_interface.php');
	include('request_interface.php');

	/* displayViewTable
	 * displays view $tid for user $uid in group $gid
	 * $selection is an array of preselected lines in the form shard.server.service.variable
	 * each address member might be incomplete (like '*' or 'GPM*')
	 */
	function displayViewTable($uid, $gid, $tid, $selection)
	{
		global	$keep_shard, $keep_server, $keep_service, $display_view;

		// 1. create views tree
		buildVariableEnv($uid, $gid, $tid, $vardisp, $bounds, $privilege, $tree, $condensed, $autoDisplay);

		if ($autoDisplay || $display_view)
		{
			// 2. select branches
			if (count($selection) > 0)
			{
				foreach ($selection as $sel)
				{
					if ($sel != "")
					{
						$selarray = expandQuery($sel);
						
						foreach ($selarray as $sel)
						{
							$address = explode(".", $sel);
							//echo "select address ".$sel."<br>\n";
							addToSelectNode($tree, $address, 0);
						}
					}
				}
			}

			// 3. remove unselected branches
			clearupNode($tree);

			// 4. build and factorize query
			$query = factorizeQuery(buildQuery($tree));

			if ($numsteps == 5)
			{
				echo "Unsupported entity request now<br>\n";
				return;
			}

			$bef = microtime();
			$qstate = nel_query($query, $result);
			$aft = microtime();
	
			list($usec, $sec) = explode(" ", $bef);
			$bef = ((float)$sec + (float)$usec);
			list($usec, $sec) = explode(" ", $aft);
			$aft = ((float)$sec + (float)$usec);
			$tm = (int)(($aft-$bef)*1000.0);
	
			if (!$qstate)
				echo "<b>$result</b>\n";
		}
		
		displayResult($result, $vardisp, $bounds, $privilege, $condensed, $autoDisplay);

		return "Executed query '$query'<br>$tm milliseconds computation time<br>\n";
	}
	
	function buildVariableEnv($uid, $gid, $tid, &$vardisp, &$bounds, &$privilege, &$tree, &$condensed, &$autoDisplay)
	{
		$result = sqlquery("SELECT view_row.name AS user_name, variable.name AS var_name, path, warning_bound, error_bound, alarm_order, view_row.filter as varfilter, view_table.filter as viewfilter, display, auto_display, view_row.vid AS vid ".
								 "FROM variable, view_row, view_table ".
								 "WHERE variable.command='variable' AND (view_table.uid='$uid' OR view_table.uid='$gid') AND view_table.tid='$tid' AND view_row.tid='$tid' AND variable.vid=view_row.vid ORDER BY view_row.ordering");

		while ($result && ($arr=sqlfetch($result)))
		{
			$vid = $arr["vid"];
			if (!hasAccessToVariable($vid)) 
				continue;

			$path = $arr["path"];
			if ($arr["varfilter"] != "" && ($path = filterPath($path, $arr["varfilter"])) == "")
				continue;
			if ($arr["viewfilter"] != "" && ($path = filterPath($path, $arr["viewfilter"])) == "") 
				continue;

			$condensed = ($arr["display"] == "condensed");
			$autoDisplay = ($arr["auto_display"] == "auto");

			$varName = getVarName($path);
			$vardisp[$varName] = $arr["user_name"];
			$bounds[$varName] = array($arr["warning_bound"], $arr["error_bound"], $arr["alarm_order"]);
			$privilege[$varName] = getVariableRight($vid);

			$address = explode(".", $path);
			//echo "add to tree address $path<br>\n";
			if (!isset($numsteps))
				$numsteps = count($address);
			if	($numsteps != count($address))
			{
				echo "Invalid table <b>$tid</b>, contains different variable path length (typically, mixed shard/server/service variables with entity variables)\n";
				return;
			}
			addToNode($tree, $address, 0);
		}
	}
	
	function humanReadableToRaw($str)
	{
		$l=strlen($str);

		if ($l==0)
			return 0;

		$regexp = '([\+-]?[0-9]+|[\+-]?[0-9]+\.[0-9]*|[\+-]?[0-9]+\.[0-9]*e[0-9]+) *(b|kb|mb|gb|h|mn|s|d|ms)?';

		if (!eregi($regexp, strtolower($str), $regs))
			return 0;

		$num = (float)$regs[1];
		$unit = $regs[2];

		switch($unit)
		{
		case "kb":	return	$num*1024;
		case "mb":	return	$num*1024*1024;
		case "gb":	return	$num*1024*1024*1024;
		case "mn":	return	$num*60;
		case "h":	return	$num*3600;
		case "ms":	return	$num/1000.0;
		case "d":	return	$num*86400;
		default:		return	$num;
		}

		return $num;
	}
	
	function vcmp($a, $b)
	{
		return ($a["_path_"] == $b["_path_"] ? 0 : $a["_path_"] < $b["_path_"] ? -1 : 1);
	}

	function displayResult($result, $vardisp, $bounds, $privilege, $condensed=false, $autoDisplay=true)
	{
		global	$admlogin, $filter_shard, $filter_server, $filter_service, $filter_entity, $current_tid, $listPath, $current_tname, $usersData;
		global	$uid, $REMOTE_ADDR, $shardLockState, $enablelock, $IsNevrax;
		
		addToLog($result);
		
		$colorKeyWords = array("Offline" => "#FF4422", "Failed" => "#FF4422");
		
		//echo "result=$result<br>\n";
		
		echo "<table border=1>\n";
		echo "<tr><th rowspan=2>&nbsp;Filters&nbsp;</th><th>shard</th><th>server</th><th>service</th><th>entity</th><td rowspan=2>&nbsp;<input type=submit name='display_view' value='Display\nView'>&nbsp;</td><td rowspan=2>&nbsp;<input type=submit name='reset_filters' value='Reset\nFilters'>&nbsp;</td></tr>\n";
		echo "<tr>\n";
		echo "<td><input type=text name=filter_shard value='$filter_shard' size=12 maxlength=256></td>\n";
		echo "<td><input type=text name=filter_server value='$filter_server' size=12 maxlength=256></td>\n";
		echo "<td><input type=text name=filter_service value='$filter_service' size=12 maxlength=256></td>\n";
		echo "<td><input type=text name=filter_entity value='$filter_entity' size=28 maxlength=1024></td>\n";
		echo "</tr>\n";
		echo "</table><br>\n";

		unset($maxvarwidth);

		if (strlen($result)>0)
		{
			$arr = explode(" ", $result);
			$result = &$arr;
			$numRes = count($result);
			$numRows = current($result);
			if ($numRows < 1)
			{
				echo "<b>NeL query returned no answer</b><br>\n";
				return;
			}
			$numLines = ($numRes-$numRows-2)/$numRows;
			//echo "numRes=$numRes, numLines=$numLines, numRows=$numRows<br>\n";
			next($result);

			for ($i=0; $i<$numRows; ++$i)
			{
				$vars[] = current($result);
				next($result);
			}

			unset($lineMap);
			unset($resultArray);

			for ($i=0; $i<$numLines; ++$i)
			{
				$shard = '*';
				$server = '*';
				$service = '*';
				unset($entity);
				$path = "";

				foreach($vars as $var)
				{
					$maxvarwidth[$var] = strlen($var);

					$val = current($result);

					next($result);

					if ($val == "???")
						unset($val);

					if ($var == "shard" || $var == "server" || $var == "service" || $var == "entity")
					{
						if ($path != "")
							$path .= ".";
						$path .= $val;

						if ($var == "shard")	{ $shard = $val; $listShard[] = $shard; }
						if ($var == "server")	$server = $val;
						if ($var == "service")	$service = $val;
						if ($var == "entity")	$entity = $val;
					}
					else
					{
						$line = $lineMap[$path];
						if (!isset($line))
						{
							$line = count($resultArray);
							$lineMap[$path] = $line;
							$resultArray[$line]["shard"] = $shard;
							$resultArray[$line]["server"] = $server;
							$resultArray[$line]["service"] = $service;
							$resultArray[$line]["_path_"] = $path;

							$serviceDecomp = explode("/", $service);
							if (count($serviceDecomp) == 1)
							{
								$serviceAlias = $service;
								$serviceSid = $service;
							}
							else
							{
								$serviceAlias = $serviceDecomp[0];
								$serviceSid = $serviceDecomp[1];
							}

							if (isset($entity))
							{
								$resultArray[$line]["entity"] = $entity;
								$listPath[] = $shard.".".$server.".".$service.".".$entity;
							}
							else
							{
								$listPath[] = $shard.".".$server.".".$service;
							}
						}

						if (isset($val))
							$resultArray[$line][$var] = $val;

						//echo "set resultArray[$line][$var]=$val<br>\n";
					}
				}
			}
			
			//echo "<pre>\n";
			//print_r($resultArray);
			//echo "</pre>\n";

			if (count($listShard) > 0)
			{
				$listShard = array_unique($listShard);
				sort($listShard);

				echo "<table border=1>\n";
				echo "<tr><th>Shard</th>";
				if ($enablelock)
					echo "<th>Locked by</th><th></th>";
				echo "<th>Annotation by</th><th width='500'>Annotation</th><th></th></tr>\n";
				foreach($listShard as $shard)
				{
					$annotation = &$shardLockState[$shard];

					$hasColor = time()-strtotime($annotation['post_date'])< 120;
					$lockState = $annotation['lock_state'];
					echo "<tr".($hasColor ? ' bgcolor=#FFBB88' : '')."><td>&nbsp;$shard&nbsp;</td>";
	
					if ($enablelock)
					{
						echo	"<td><font size=0>&nbsp;".($lockState != 0 ? ($usersData[$annotation['lock_user']]["login"]."/".$annotation['lock_ip']."&nbsp;<br>&nbsp;".$annotation['lock_date']) : "not locked")."&nbsp;</font></td>";
						echo 	"<td>";
						if ($lockState == 0)
							echo "<input type=submit name='upd_lock_button_$shard' value='Lock'>";
						else if ($lockState == 1)
							echo "<input type=submit name='upd_unlock_button_$shard' value='Unlock'>";
						else
							echo "<input type=submit name='upd_lock_button_$shard' value='Force Lock'>";
						echo 	"</td>";
					}
							
					echo	"<td>&nbsp;".$usersData[$annotation['user_annot']]["login"]."&nbsp;/&nbsp;".$annotation['post_date']."&nbsp;</td>";
					echo	"<td>&nbsp;".($lockState == 1 ? ("<input type=text name='upd_annot_text_$shard' value='".$annotation['annot']."' size=90 maxlength=255".($hasColor ? " style='background-color: #FFBB88;'" : "").">") : $annotation['annot'])."&nbsp;</td>";
					echo	"<td>".($lockState == 1 ? ("<input type=submit name='upd_annot_button_$shard' value='Update'>") : (""))."</td>";
							
					echo "</tr>\n";
				}
				echo "</table><br>\n";
			}

			echo "<table border=1>\n";
			echo "<tr valign=center>";
			foreach ($vars as $var)
			{
				$display = (isset($vardisp[$var]) ? $vardisp[$var] : "<i>".$var."</i>");
				if ($var == "shard" || $var == "server" || $var == "service"  || $var == "entity" || !$condensed)
					echo "<th>$display</th>";
				else
					break;
			}

			if ($condensed)
				echo "<th>Variable</th><th>Value</th>";

			echo "</tr>\n";

			if (count($resultArray) > 0)
				usort($resultArray, "vcmp");

			$prevShard = "";
			$idxShard = 0;
			$dispUpdateButton = false;

			if ($condensed)
			{
				for ($i=0; $i<count($resultArray); ++$i)
				{
					$shard = '*';
					$server = '*';
					$service = '*';
					unset($entity);

					$arr = &$resultArray[$i];

					if ($arr["shard"] != $prevShard)
					{
						if ($prevShard != "")
							$idxShard = 1-$idxShard;
						$prevShard = $arr["shard"];
					}

					$bgcolor = ($idxShard == 0 ? ($i&1 ? "#DDDDDD" : "#EEEEEE") : ($i&1 ? "#D0DDDD" : "#E0EEEE"));

					$first = true;

					echo "<tr bgcolor=$bgcolor>";

					for ($j=0; $j<count($vars); ++$j)
					{
						$var = $vars[$j];
						if ($var != "shard" && $var != "server" && $var != "service" && $var != "entity")
							break;

						$val = $arr[$var];
						$bgcolor2 = ((strstr($val, "((TIMEOUT))") == FALSE) ? $bgcolor : "#FF4422");

						if ($var == "shard")					$shard = $val;
						if ($var == "server")					$server = $val;
						if ($var == "entity" && $val != "")		$entity = $val;

						//
						// parse service name
						//
						if ($var == "service")
						{
							splitServiceName($val, $serviceAlias, $serviceName, $serviceId);
							$service = $serviceName;
						}

						$vval = ($var == "service" ? $serviceName : $val);
						if ($var == "shard")				$selects = "&filter_shard=$shard";
						else if ($var == "server")			$selects = "&filter_shard=$shard&filter_server=$server";
						else if ($var == "service")			$selects = "&filter_shard=$shard&filter_server=$server&filter_service=$service";
						else if ($var == "entity")			$selects = "&filter_shard=$shard&filter_server=$server&filter_service=$service&filter_entity=$vval";
						$vval .= ",Default".ucfirst(strtolower($var == "entity" ? "player" : $var)).(isset($current_tname) ? ",$current_tname" : "");

						echo "<td bgcolor=$bgcolor2 nowrap>&nbsp;<a href='".$_SERVER['PHP_SELF']."?select_view=$vval$selects'>$val</a>".((($admlogin=="root" || $IsNevrax) && $var=="service") ? "&nbsp;<a href='commands.php?preselServ=$shard.$server.$serviceAlias'><font size=1>[cmd]</font></a>" : "")."</td>";
					}

					$numprev = $j;

					for (; $j<count($vars); ++$j)
					{
						$var = $vars[$j];
						$val = $arr[$var];

						if (!isset($val) || $val == "")
							continue;

						if (!$first)
						{
							echo "<tr bgcolor=$bgcolor>";
							for ($k=0; $k<$numprev; ++$k)
								echo "<td bgcolor=$bgcolor></td>";
						}

						$first = false;

						$maxvarwidth[$var] = max($maxvarwidth[$var], strlen($val));

						$bound = &$bounds[$var];
						$state = 0;
						if (isset($bound) && $val != "")
						{
							$realval = humanReadableToRaw($val);
							if ($bound[2] == 'gt')
							{
								if ($bound[0]!=-1 && $realval >= $bound[0])
									$state = 1;
								if ($bound[1]!=-1 && $realval >= $bound[1])
									$state = 2;
							}
							else
							{
								if ($bound[0]!=-1 && $realval <= $bound[0])
									$state = 1;
								if ($bound[1]!=-1 && $realval <= $bound[1])
									$state = 2;
							}
						}

						$btcolor = ($state==0 ? $bgcolor : ($state==1 ? "#FFCC88" : "#FF4422"));

						if (isset($colorKeyWords[$val]))
							$btcolor = $colorKeyWords[$val];

						if ($privilege[$var] == 'rw' && $shardLockState[$shard]['lock_state'] == 1)
						{
							$valdisp = "<input name='updvar_$shard|$server|$serviceAlias|".(isset($entity)?"$entity|":"")."$var' value='$val' size=16 maxlength=64 style='background-color: $btcolor;'><input type=hidden name='prevvar_$shard|$server|$serviceAlias|".(isset($entity)?"$entity|":"")."$var' value='$val'>";
							$dispUpdateButton = true;
						}
						else
						{
							$valdisp = "$val";
						}

						echo "<td>$var</td><td bgcolor=$btcolor nowrap>$valdisp</td></tr>\n";
					}
				}
			}
			else
			{
				for ($i=0; $i<count($resultArray); ++$i)
				{
					$arr = &$resultArray[$i];
					foreach($vars as $var)
					{
						$val = $arr[$var];
						$maxvarwidth[$var] = max($maxvarwidth[$var], strlen($val));
					}
				}

				for ($i=0; $i<count($resultArray); ++$i)
				{
					$shard = '*';
					$server = '*';
					$service = '*';
					unset($entity);
					unset($line);

					$arr = &$resultArray[$i];

					if ($arr["shard"] != $prevShard)
					{
						if ($prevShard != "")
							$idxShard = 1-$idxShard;
						$prevShard = $arr["shard"];
					}

					$bgcolor = ($idxShard == 0 ? ($i&1 ? "#DDDDDD" : "#EEEEEE") : ($i&1 ? "#D0DDDD" : "#E0EEEE"));

					foreach($vars as $var)
					{
						$val = $arr[$var];

						if ($val == "???")	$val = NULL;
						$addlink = false;

						$bgcolor2 = ((strstr($val, "((TIMEOUT))") == FALSE) ? $bgcolor : "#FF4422");

						if ($var == "shard")					{ $addlink = true; $shard = $val; }
						if ($var == "server")					{ $addlink = true; $server = $val; }
						if ($var == "entity" && $val != "")		{ $addlink = true; $entity = $val; }

						//
						// parse service name
						//
						if ($var == "service")
						{
							splitServiceName($val, $serviceAlias, $serviceName, $serviceId);
							$service = $serviceName;
							$addlink = true;
						}

						//$maxvarwidth[$var] = max($maxvarwidth[$var], strlen($val));

						$bound = &$bounds[$var];
						$state = 0;
						if (isset($bound) && $val != "")
						{
							$realval = humanReadableToRaw($val);
							if ($bound[2] == 'gt')
							{
								if ($bound[0]!=-1 && $realval >= $bound[0])
									$state = 1;
								if ($bound[1]!=-1 && $realval >= $bound[1])
									$state = 2;
							}
							else
							{
								if ($bound[0]!=-1 && $realval <= $bound[0])
									$state = 1;
								if ($bound[1]!=-1 && $realval <= $bound[1])
									$state = 2;
							}
						}

						$btcolor = ($state==0 ? $bgcolor : ($state==1 ? "#FFCC88" : "#FF4422"));

						if (isset($colorKeyWords[$val]))
							$btcolor = $colorKeyWords[$val];

						if (strstr($val, "((TIMEOUT))") != FALSE)
							$btcolor = "#FF4422";

						if ($privilege[$var] == 'rw' && $shardLockState[$shard]['lock_state'] == 1)
						{
							$len = max(min($maxvarwidth[$var], 16), 4);
							$valdisp = "<input name='updvar_$shard|$server|$serviceAlias|".(isset($entity)?"$entity|":"")."$var' value='$val' size=$len maxlength=64 style='background-color: $btcolor;'><input type=hidden name='prevvar_$shard|$server|$serviceAlias|".(isset($entity)?"$entity|":"")."$var' value='$val'>";
							$dispUpdateButton = true;
						}
						else
						{
							$valdisp = "$val";
						}

						if ($addlink)
						{
							$vval = ($var == "service" ? $serviceName : $val);
							if ($var == "shard")				$selects = "&filter_shard=$shard";
							else if ($var == "server")			$selects = "&filter_shard=$shard&filter_server=$server";
							else if ($var == "service")			$selects = "&filter_shard=$shard&filter_server=$server&filter_service=$vval";
							else if ($var == "entity")			$selects = "&filter_shard=$shard&filter_server=$server&filter_service=$service&filter_entity=$vval";
							$vval .= ",Default".ucfirst(strtolower($var == "entity" ? "player" : $var)).(isset($current_tname) ? ",$current_tname" : "");

							$line .= "<td bgcolor=$btcolor nowrap>&nbsp;<a href='".$_SERVER['PHP_SELF']."?select_view=$vval$selects'>$valdisp</a>".((($admlogin=="root" || $IsNevrax) && $var=="service") ? "&nbsp;<a href='commands.php?preselServ=$shard.$server.$serviceAlias'><font size=1>[cmd]</font></a>" : "")."&nbsp;</td>";
						}
						else
						{
							$line .= "<td bgcolor=$btcolor nowrap>&nbsp;$valdisp&nbsp;</td>";
						}
					}

					echo "<tr bgcolor=$bgcolor>";
					echo $line;
					echo "</tr>\n";
				}
			}


			if (!$condensed)
			{
				echo "<tr height=5><td colspan=".(count($vars)+1)."></td></tr>";
				echo "<tr>";
				foreach($vars as $var)
				{
					$len = max(min($maxvarwidth[$var], 16), 4);
					if ($var != "shard" && $var != "server" && $var != "service" && $var != "entity" && $privilege[$var] == 'rw')
						echo "<td>&nbsp;<input name='override_$var' size=$len maxlength=64>&nbsp;</td>";
					else
						echo "<td><i>&nbsp;All</i></td>";
				}
				echo "</tr>\n";
			}

			echo "</table><br>\n";
			if ($dispUpdateButton)
				echo "<input type=submit name='upd_values' value='Update Values'>\n";
		}
		else
		{
			echo "<b>NeL query returned no answer</b><br>\n";
		}
	}


	function addToNode(&$node, &$address, $step)
	{
		if ($step >= count($address))
			return;
		//echo "addToNode:$address[$step]<br>\n";
		addToNode($node[$address[$step]], $address, $step+1);
	}

	function filterPath($path, $filter)
	{
		$pnodes = explode(".", $path);
		$fnodes = explode(".", $filter);
		
		for ($i=0; $i<count($pnodes); ++$i)
		{
			if ($fnodes[$i][0] == '@')
			{
				$rnodes[] = substr($fnodes[$i], 1);
			}
			else if ($fnodes[$i] == "*" || $fnodes[$i] == "#" || $fnodes[$i] == "" || $pnodes[$i] == $fnodes[$i])
			{
				$rnodes[] = $pnodes[$i];
			}
			else if ($pnodes[$i] == "*" || $pnodes[$i] == "#")
			{
				$rnodes[] = $fnodes[$i];
			}
			else
			{
				return "";
			}
		}
		
		//echo "filter($path, $filter)=".join(".", $rnodes)."<br>\n";

		return join(".", $rnodes);
	}

	function partialFilterPath($path, $filter)
	{
		$pnodes = explode(".", $path);
		$fnodes = explode(".", $filter);
		
		for ($i=0; $i<count($pnodes); ++$i)
		{
			if ($fnodes[$i][0] == '@')
			{
				$rnodes[] = substr($fnodes[$i], 1);
			}
			else if ($fnodes[$i] == "*" || $fnodes[$i] == "#" || $fnodes[$i] == "" || (strstr($pnodes[$i], $fnodes[$i]) !== FALSE) && $i==0 || $pnodes[$i] == $fnodes[$i])
			{
				$rnodes[] = $pnodes[$i];
			}
			else if ($pnodes[$i] == "*" || $pnodes[$i] == "#")
			{
				$rnodes[] = $fnodes[$i];
			}
			else
			{
				return "";
			}
		}
		
		//echo "filter($path, $filter)=".join(".", $rnodes)."<br>\n";

		return join(".", $rnodes);
	}

	function splitServiceName($service, &$serviceAlias, &$serviceName, &$serviceId)
	{
		if (($p1 = strpos($service, "/")) === FALSE || ($p2 = strpos($service, "-")) === FALSE || $p2<$p1)
		{
			$serviceName = $service;
			$serviceAlias = $service;
			$serviceId = $service;
		}
		else
		{
			$serviceName = substr($service, $p1+1, $p2-$p1-1);
			$serviceAlias = substr($service, 0, $p1);
			$serviceId = substr($service, $p2);
		}
	}

	function filterPathUsingAliases($path, $filter)
	{
		$pnodes = explode(".", $path);
		$fnodes = explode(".", $filter);
		
		for ($i=0; $i<count($pnodes) || $i<count($fnodes); ++$i)
		{
			$aliases = split( '[/-]', $pnodes[$i] );
			if (count($aliases) == 3)
			{
				$pmatch =  $aliases[1];
				$palias =  $aliases[0];
			}
			else
			{
				$pmatch =  $aliases[0];
				$palias =  $aliases[0];
			}

			if ($fnodes[$i][0] == '@')
			{
				$rnodes[] = substr($fnodes[$i], 1);
			}
			else if ($fnodes[$i] == "*" || $fnodes[$i] == "#" || $fnodes[$i] == "" || $pmatch == $fnodes[$i])
			{
				$rnodes[] = $palias;
			}
			else if ($pnodes[$i] == "*" || $pnodes[$i] == "#" || $pnodes[$i] == "")
			{
				$rnodes[] = $fnodes[$i];
			}
			else
			{
				return "";
			}
		}
		
		//echo "filter($path, $filter)=".join(".", $rnodes)."<br>\n";

		return join(".", $rnodes);
	}

	function addToSelectNode(&$node, &$address, $step)
	{
		//echo "--addToSelectNode(step=$step,count(address)=".count($address).")<br>\n";
		if ($node == NULL)
		{
			return true;
		}
		else if ($step >= count($address))
		{
			$nodes = array_keys($node);
			foreach($nodes as $nod)
			{
				if ($nod == "___flag")
					continue;
				$subnode = &$node[$nod];
				addToSelectNode($subnode, $address, $step+1);
				$subnode["___flag"] = true;
			}
			return true;
		}

		$sel = $address[$step];
		$sel_reg = "^".str_replace("*", ".*", $sel)."$";
		$sel_cut = str_replace("*", "&&&&&", $sel);

		//echo "----addToSelectNode: sel=$sel sel_reg=$sel_reg sel_cut=$sel_cut<br>\n";

		$nodes = array_keys($node);
		
		$flag = false;

		foreach($nodes as $nod)
		{
			if ($nod == "___flag")
				continue;

			$subnode = &$node[$nod];
			$nod_reg = "^".str_replace("#", ".*", str_replace("*", ".*", $nod))."$";
			//$nod_reg = "^".str_replace("*", ".*", $nod)."$";
			$nod_cut = str_replace("*", "&&&&&", $nod);

			//echo "addToSelectNode: nod=$nod nod_reg=$nod_reg nod_cut=$nod_cut<br>\n";

			// if subnode matches selection, subnode is more restrictive and then flag subnode and keep subnode
			if (eregi($sel_reg, $nod_cut))
			{
				//echo "node $nod matches select $sel<br>\n";
				if (addToSelectNode($subnode, $address, $step+1))
				{
					$subnode["___flag"] = true;
					$flag = true;
				}
			}
			// if selection matches subnode, selection is more restrictive and then copy subnode as selection, flag new subnode and keep it
			else if (eregi($nod_reg, $sel_cut))
			{
				//echo "selection $sel matches node $nod, adding node $sel<br>\n";
				addNode($node[$sel], $subnode);
				$subnode = &$node[$sel];
				if (addToSelectNode($subnode, $address, $step+1))
				{
					$subnode["___flag"] = true;
					$flag = true;
				}
			}
		}
		
		return $flag;
	}
	
	function clearupNode(&$node)
	{
		if (!is_array($node))
			return;

		$nodes = array_keys($node);

		foreach ($nodes as $key)
		{
			$subnode = &$node[$key];
			if ($subnode["___flag"])
			{
				unset($subnode["___flag"]);
				clearupNode($subnode);
			}
			else
			{
				unset($node[$key]);
			}
		}
	}
	
	function dispNode(&$node, $indent)
	{
		if (count($node) < 1)
			return;

		foreach ($node as $key => $subnode)
		{
			if ($key != "___flag")
			{
				for ($i=0;$i<$indent; ++$i)
					echo "-";
				echo $key.($subnode["___flag"] ? " flag":"")."<br>\n";
				dispNode($subnode, $indent+1);
			}
		}
	}
	
	function addNode(&$dest, &$src)
	{
		if (!is_array($src) || count($src) < 1)
			return;

		foreach ($src as $key => $subnode)
			if ($key != "___flag")
				addNode($dest[$key], $subnode);
	}
	
	function buildQuery(&$node)
	{
		if (!is_array($node) || count($node) < 1)
			return;
			
		$hasChild = count($node)>1;

		if ($hasChild)
			$sub .= "[";
		$i = 0;
		foreach ($node as $key => $subnode)
		{
			$sub .= "$key";
			if (is_array($subnode) && count($subnode)>=1)
			{
				$sub .= ".";
				$sub .= buildQuery($subnode);
			}
			if ($i < count($node)-1)
				$sub .= ",";
			++$i;
		}
		if ($hasChild)
			$sub .= "]";

		return $sub;
	}





	//
	function factorizeQuery($query)
	{
		global	$factorizeForward;
		
		$factorizeForward = true;
		$ret = factorizeQueryInternal($query);
/*		$factorizeForward = false;
		$ret = factorizeQueryInternal($ret)
*/		return $ret;
	}

	function factorizeQueryInternal($query)
	{
		global	$alev;
		++$alev;
		
		if ($query == "")
			return $query;	

		//echo "$alev factorizeQuery($query)<br>\n";
		$blocs = splitQuery($query);
		unset($newQuery);
		
		$first = true;
		
		foreach ($blocs as $bloc)
		{
			$newBloc = factorizeBloc($bloc);
			if (!$first)
				$newQuery .= '.';
			$newQuery .= $newBloc;
			$first = false;
		}
		
		//echo "$alev factorizeQuery($query) = $newQuery<br>\n";
		--$alev;
		return $newQuery;
	}

	function splitQuery($query)
	{
		$i=0;
		while ($i<strlen($query))
		{
			$bloc = "";
			$level = 0;
			while ($i<strlen($query) && (($char=$query{$i})!='.' || $level!=0))
			{
				if ($char == '[')			++$level;
				else if ($char == ']')	--$level;
				$bloc .= $char;
				++$i;
			}
			++$i;
			if ($level != 0)
				echo "Error on query '$query', badly formed (missing end bracket?)<br>\n";
			$arr[] = $bloc;
		}
		return $arr;
	}
	
	function separateLastQueryBloc($query, &$lead, &$last)
	{
		$i=strlen($query)-1;
		$level = 0;
		while ($i>=0 && (($char=$query{$i})!='.' || $level!=0))
		{
			if ($char == '[')			++$level;
			else if ($char == ']')	--$level;
			--$i;
		}
		if ($level != 0)
			echo "Error on query '$query', badly formed (missing end bracket?)<br>\n";

		if ($i == -1)
		{
			$lead = "";
			$last = $query;
		}
		else
		{
			$lead = substr($query, 0, $i);
			$last = substr($query, $i+1);
		}
	}

	function separateFirstQueryBloc($query, &$first, &$follow)
	{
		$i=0;
		$level = 0;
		while ($i<strlen($query) && (($char=$query{$i})!='.' || $level!=0))
		{
			if ($char == '[')			++$level;
			else if ($char == ']')	--$level;
			++$i;
		}
		if ($level != 0)
			echo "Error on query '$query', badly formed (missing end bracket?)<br>\n";

		if ($i == strlen($query))
		{
			$follow = "";
			$first = $query;
		}
		else
		{
			$first = substr($query, 0, $i);
			$follow = substr($query, $i+1);
		}
	}

	function factorizeBloc($bloc)
	{
		global	$factorizeForward;
		return $factorizeForward ? factorizeForwardBloc($bloc) : factorizeBackwardBloc($bloc);
	}

	function factorizeForwardBloc($bloc)
	{
		if ($bloc{0} != '[')
			return $bloc;

		$queries = splitBloc($bloc);
		
		if (count($queries) == 0)
			return;
		if (count($queries) == 1)
			return $queries[0];

		global	$alev;
		++$alev;
		//echo "$alev factorizeForwardBloc($bloc)<br>\n";

		unset($factors);
		foreach ($queries as $query)
		{
			$newQuery = factorizeQueryInternal($query);
			separateFirstQueryBloc($query, $first, $follow);
			//echo "$alev found factor $first - $follow<br>\n";
			
			$factors[$first][] = $follow;
		}

		if (count($factors) == 1)
		{
			reset($factors);
			$a = array_keys($factors);
			$factor = $a[0];
			$follows = $factors[$factor];
			if (count($follows) == 1)
			{
				$factorized = $factor.($follows[0] != "" ? '.'.$follows[0] : "");
			}
			else
			{
				$fbloc = joinArray($follows, ",");
				$factorized = ($fbloc == "") ? $factor : $factor.'.'.factorizeBloc('['.$fbloc.']');
			}
		}
		else
		{
			unset($subblocs);
			foreach ($factors as $factor => $follows)
			{
				//echo "$alev factor $factor, ".count($follows)." leads<br>\n";
				if (count($follows) == 1)
				{
					$subblocs[] = $factor.($follows[0] != "" ? '.'.$follows[0] : "");
				}
				else
				{
					$fbloc = joinArray($follows, ",");
					$subblocs[] = ($fbloc == "") ? $factor : $factor.'.'.factorizeBloc('['.$fbloc.']');
				}
			}
			$factorized = '['.joinArray($subblocs, ",").']';
		}
		
		//echo "$alev factorizeForwardBloc($bloc) = $factorized<br>\n";
		--$alev;
		return $factorized;
	}

	function factorizeBackwardBloc($bloc)
	{
		if ($bloc{0} != '[')
			return $bloc;

		$queries = splitBloc($bloc);
		
		if (count($queries) == 0)
			return;
		if (count($queries) == 1)
			return $queries[0];

		global	$alev;
		++$alev;
		//echo "$alev factorizeBloc($bloc)<br>\n";

		unset($factors);
		foreach ($queries as $query)
		{
			$newQuery = factorizeQueryInternal($query);
			separateLastQueryBloc($query, $lead, $last);
			//echo "$alev found factor $lead - $last<br>\n";
			
			$factors[$last][] = $lead;
		}

		if (count($factors) == 1)
		{
			reset($factors);
			$a = array_keys($factors);
			$factor = $a[0];
			$leads = $factors[$factor];
			if (count($leads) == 1)
			{
				$factorized = ($leads[0] != "" ? $leads[0].'.' : "").$factor;
			}
			else
			{
				$fbloc = joinArray($leads, ",");
				$factorized = ($fbloc == "") ? $factor : factorizeBloc('['.$fbloc.']').'.'.$factor;
			}
		}
		else
		{
			unset($subblocs);
			foreach ($factors as $factor => $leads)
			{
				//echo "$alev factor $factor, ".count($leads)." leads<br>\n";
				if (count($leads) == 1)
				{
					$subblocs[] = ($leads[0] != "" ? $leads[0].'.' : "").$factor;
				}
				else
				{
					$fbloc = joinArray($leads, ",");
					$subblocs[] = ($fbloc == "") ? $factor : factorizeBloc('['.$fbloc.']').'.'.$factor;
				}
			}
			$factorized = '['.joinArray($subblocs, ",").']';
		}
		
		//echo "$alev factorizeBloc($bloc) = $factorized<br>\n";
		--$alev;
		return $factorized;
	}

	function splitBloc($bloc)
	{
		if ($bloc{0} != '[')
		{
			$arr[] = $bloc;
		}
		else
		{
			if ($bloc{strlen($bloc)-1}!=']')
				echo "Error on bloc '$bloc', unexpected character after ']'<br>\n";

			$i=1;
			while ($i<strlen($bloc)-1)
			{
				$alt = "";
				$level = 0;
				while ( $i<strlen($bloc)-1 && ( ($char=$bloc{$i}) != ','  ||  $level!=0 ) )
				{
					if ($char == '[')			++$level;
					else if ($char == ']')	--$level;
					$alt .= $char;
					++$i;
				}
				++$i;
				if ($level != 0)
					echo "Error on bloc '$bloc', badly formed (missing end bracket?)<br>\n";
				$arr[] = $alt;
			}
		}
		return $arr;
	}
	
	function joinArray(&$array, $sep)
	{
		if (count($array) == 0)
			return "";
		$first = true;
		foreach ($array as $val)
		{
			$result .= ($first ? "" : $sep).$val;
			$first = false;
		}
		return $result;
	}
	
	function getVarName($path)
	{
		$arr = explode(".", $path);
		return $arr[count($arr)-1];
	}

	function isAtom($atom)
	{
		for ($i=0; $i<strlen($atom); ++$i)
			if ($atom{$i} == '.' || $atom{$i} == '[')
				return false;
		return true;
	}

	// takes a factorized query, and expands it in a list a simple paths
	function expandQuery($query)
	{
		if (isAtom($query))
			return array($query);

		//echo "expand query: $query<br>";
		$blocs = splitQuery($query);
		//echo "splitted query: blocs=";
		//print_r($blocs);
		//echo "<br>-------------------<br>";

		for ($i=0; $i<count($blocs); ++$i)
		{
			$subblocs = splitBloc($blocs[$i]);
			//echo "splitted blocs: subblocs=";
			//print_r($subblocs);
			//echo "<br>-------------------<br>";

			unset($nodes);
			$nodes = array();
			for ($ii=0; $ii<count($subblocs); ++$ii)
			{
				$nodes = array_merge($nodes, expandQuery($subblocs[$ii]));
			}

			if (count($current_nodes) >= 1)
			{
				$num_nodes = count($current_nodes);
				for ($j=0; $j<$num_nodes; ++$j)
				{
					for ($k=0; $k<count($nodes)-1; ++$k)
						$current_nodes[] = $current_nodes[$j].".".$nodes[$k];
					$current_nodes[$j] .= ".".$nodes[$k];
				}
			}
			else
			{
				$current_nodes = $nodes;
			}
		}
		return $current_nodes;
	}

?>
