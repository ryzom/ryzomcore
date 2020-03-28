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
	include('request_interface.php');

	if ($preselServ != "")
	{
		list($presel_shard, $presel_server, $presel_service) = explode(".", $preselServ);
		
		if ($presel_service)
		{
			$aliases = split('[/-]', $presel_service);
			if (count($aliases) == 3)
				$presel_service = $aliases[0];
		}
	}

	htmlProlog($_SERVER['PHP_SELF'], "Player Locator");

	echo "<script><!--\n";
	echo "//----------------------------------\n";
	echo "function clickOnEntity(entity)\n";
	echo "{\n";
	echo "	player_select[entity] = !player_select[entity];\n";
	echo "	if (player_select[entity])\n";
	echo "	{\n";
	echo "		document.getElementById('player_'+entity).style.background = player_sel_bgcolor[entity];\n";
	echo "	}\n";
	echo "	else\n";
	echo "	{\n";
	echo "		document.getElementById('player_'+entity).style.background = player_bgcolor[entity];\n";
	echo "	}\n";
	echo "	var total_select = 0;\n";
	echo "	var list_select = ''\n";
	echo "	var list_active = ''\n";
	echo "	for (i=0; i<num_player; ++i)\n";
	echo "	{\n";
	echo "		if (player_select[i])\n";
	echo "		{\n";
	echo "			if (list_select != '')	list_select += ',';\n";
	echo "			++total_select;\n";
	echo "			list_select += player_eid[i];\n";
	echo "			if (player_state[i] == 'Offline')\n";
	echo "			{\n";
	echo "				if (list_active != '')	list_active += ',';\n";
	echo "				list_active += player_uidslot[i];\n";
	echo "			}\n";
	echo "		}\n";
	echo "	}\n";
	echo "	if (total_select > 1)	list_select = '['+list_select+']';\n";
	echo "	document.getElementById('filter_entity_hidden').value = list_select;\n";
	echo "	document.getElementById('active_player_hidden').value = list_active;\n";
	echo "	window.status = list_active;\n";
	echo "}\n";
	echo "\n";
	echo "//----------------------------------\n";
	echo "function submitForm(entity)\n";
	echo "{\n";
	echo "	return true;\n";
	echo "}\n";
	echo "\n";
	echo "//--></script>\n";

	// input variables :
	// - $preselServ : preselected service address
	// - $execCommand : executed command on preselected service, like a normal service
	//

	echo "<br><br>\n";

	echo "<table border=0><tr valign=top>\n";
	echo "<form method=post action='".$_SERVER['PHP_SELF']."' name='cmdform'>\n";	
	echo "<td>\n";

	$result = sqlquery("SELECT DISTINCT shard FROM service ORDER BY shard");

	echo "<select multiple size=".(sqlnumrows($result))." name='selshards[]'>";
	while ($result && ($arr=sqlfetch($result)))
	{
		$selected = (isset($selshards) && in_array($arr["shard"], $selshards)) || 
					(isset($admfilter_shard) && $admfilter_shard!="" && strstr($arr["shard"], $admfilter_shard) ||
					($admfilter_shard == "" && !isset($selshards)));

		if ($selected)
			$selected_shards[] = $arr["shard"];

		echo "<option value='".$arr["shard"]."'".($selected ? " selected" : "").">".$arr["shard"];
	}
	echo "</select>\n";

	echo "</td>\n";

	echo "<td width=30>&nbsp;</td>\n";

	echo "<td>\n";
	echo "<table border=0>\n";
	echo "<tr><th align=left>Player/Character name</th></tr>\n";
	echo "<tr><td><input name=char_name value='".stripslashes($char_name)."' size=50 maxlength=20480></td>\n";
	echo "<td><input type=submit value='Locate'></td></tr>\n";
	echo "</form></table>\n";

	if (isset($char_name))
	{
		$addr = (count($selected_shards) > 0 ? "[".join(",", $selected_shards)."]" : "*").".*.EGS.playerInfo $char_name";
		$qstate = nel_query($addr, $commandResult);
	}

	if ($commandResult)
	{
		$res_array = explode("\n", $commandResult);

		$parse_start = 0;
/*
		echo "<pre>";
		print_r($res_array);
		echo "</pre>\n";
*/
		echo "<br><br>\n";
		
		$num_player = 0;

		echo "<script><!--\n";
		echo "var player_eid = new Array(30000);\n";
		echo "var player_select = new Array(30000);\n";
		echo "var player_bgcolor = new Array(30000);\n";
		echo "var player_sel_bgcolor = new Array(30000);\n";
		echo "var player_state = new Array(30000);\n";
		echo "var player_uidslot = new Array(30000);\n";
		echo "//--></script>\n";

		while (true)
		{
			if ($res_array[$parse_start] == "")
				break;

			$offset = 4;
			list($res_shard) = sscanf($res_array[$parse_start], "----- Result from Shard %s");
			list($num_res) = sscanf($res_array[$parse_start+$offset-1], "%d");
			$start = $parse_start+$offset;
			$stop = $start+$num_res;
			$parse_start += $num_res+$offset;

			$last_uid = "";
			$icolor = 0;
			
			echo "<b>Result of search for '$char_name' on Shard '$res_shard' ($num_res entr".($num_res>1 ? "ies" : "y")." found)</b><br>\n";
			echo "<font size=-2>Click on EntityId to get directly to DefaultPlayer view, <br>or click anywhere else to select a player and then click Select Players button.</font><br><br>\n";
			
			echo "<table border=1><tr><th>UId</th><th>UserName</th><th>EId</th><th>EntityName</th><th>EntitySlot</th><th>State</th><th>Ext commands</th></tr>\n";

			for ($line=$start; $line<$stop; ++$line)
			{
				$l = explode(" ", $res_array[$line]);
				
				for ($i=1; $i<count($l); $i+=2)
					$parse[$l[$i-1]] = str_replace("'", "", $l[$i]);
				$parse["State"] = $l[count($l)-1];

				$chUser = ($last_uid == "");
				if ($last_uid != "" && $parse["UId"] != $last_uid)
				{
					$icolor = 1-$icolor;
					$chUser = true;
				}
				
				$last_uid = $parse["UId"];

				$bgcolor_online = ($icolor ? "#CCEECC" : "#BBDDBB");
				$bgcolor_offline = ($icolor ? "#EEEEEE" : "#DDDDDD");
				$bgcolor = ($parse['State'] == 'Online') ? $bgcolor_online : $bgcolor_offline;
				$sel_bgcolor = ($icolor ? "#FFDDAA" : "#FFCC88");

				echo "<script><!--\n";
				echo "player_eid[$num_player]='".$parse["EId"]."';\n";
				echo "player_select[$num_player]=false;\n";
				echo "player_bgcolor[$num_player]='$bgcolor';\n";
				echo "player_sel_bgcolor[$num_player]='$sel_bgcolor';\n";
				echo "player_uidslot[$num_player]='".$parse["UId"]." ".$parse["EntitySlot"]."';\n";
				echo "player_state[$num_player]='".$parse["State"]."';\n";
				echo "//--></script>\n";
				
				echo "<tr id='player_$num_player' onClick='clickOnEntity($num_player); return true;' bgcolor=$bgcolor>";
				echo "<td>".($chUser ? $parse["UId"] : "")."</td>";
				echo "<td>".$parse["UserName"]."</td>";
				echo "<td><a href='index.php?select_view=DefaultPlayer&filter_shard=$res_shard&filter_entity=".$parse["EId"]."'>".$parse["EId"]."</a></td>";
				echo "<td>".$parse["EntityName"]."</td>";
				echo "<td>".$parse["EntitySlot"]."</td>";
				echo "<td>".$parse["State"]."</td>";
				echo "<td>";
				if (isset($parse["SaveFile"]))
				{
					echo "<a href='backup_interface.php?charid=".$parse["EId"]."&file=".$parse["SaveFile"]."'>Load/Save sheet</a>";
				}
				echo "</td>";
				echo "</tr>\n";
				
				++$num_player;
			}
			
			echo "</table>\n";
		}

		echo "<script><!--\n";
		echo "var num_player = $num_player;\n";
		echo "//--></script>\n";

		echo "<form name='select_player_form' method=post action='index.php?select_view=DefaultPlayer'>\n";
		echo "<input type=submit name='from_player_locator' value='Select Players'>\n";
		echo "<input id='filter_entity_hidden' type=hidden name=filter_entity value=''>\n";
		echo "<input id='active_player_hidden' type=hidden name=active_player value=''>\n";
		echo "</form>\n";
	}
	
	echo "</td>\n";

	echo "</tr></table>\n";

	echo "<script type='text/javascript'><!--\n";
	echo "if (document.cmdform) { document.cmdform.execCommand.focus(); }\n";
	echo "// --></script>\n";

	htmlEpilog();
?>
