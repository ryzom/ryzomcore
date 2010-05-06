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
	
	$error = '';
	
	if ($allowDownload && $download)
	{
		// query file to BSs
		$query = "*.*.BS.getFileBase64Content $file";
		$qstate = nel_query($query, $commandResult);
		
		//echo "$commandResult";
		
		if ($commandResult)
		{
			$res_array = explode("\n", $commandResult);
			$parse_start = 0;
			while (true)
			{
				if ($parse_start >= count($res_array))
					break;

				$offset = 4;
				list($res_shard) = sscanf($res_array[$parse_start], "----- Result from Shard %s");
				list($dld_file, $num_res, $file_size, $originalMD5) = sscanf($res_array[$parse_start+$offset-1], "file %s lines %d size %d haskey %s");
				$start = $parse_start+$offset;
				$stop = $start+$num_res;
				$parse_start += $num_res+$offset;
				if ($num_res == 0)
					continue;

				$buffer = '';
				for ($line=$start; $line<$stop; ++$line)
					$buffer .= trim($res_array[$line]);

				$decoded = base64_decode($buffer);
				$decodedMD5 = md5($decoded);
				
				if ($originalMD5 != '' && $originalMD5 != $decodedMD5)
				{
					$error = "<font color=#ff0000>ERROR:</font> failed to download file '$file', MD5 signature indicates file is corrupted.";
				}
				else
				{
					header("Content-type: application/bin"); 
					header("Content-Disposition: attachment; filename=".basename($file)); 
					echo $decoded;
	
					die();
				}
			}
		}
		
		if (!$error)
			$error = "<font color=#ff0000>ERROR:</font> failed to download file '$file', file may not exist on any server.";
	}

	if ($allowUpload && $upload)
	{
		$f = fopen($upld_file, "rb");
		if ($f)
		{
			$content = base64_encode(fread($f, $upld_file_size));
			$query = $shard_addr.".putFileBase64Content $file $content";

			$qstate = nel_query($query, $commandResult);
		}
	}

	htmlProlog($_SERVER['PHP_SELF'], "Backup Interface (Character Up/Download)");

	// input variables :
	// - $charid : character id to upload/download
	// - $file : filename to upload/download
	//

	$query = "*.*.BS.State";
	$qstate = nel_query($query, $result);
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
			$shards[] = $l;
		}
	}


	echo "<br><br>\n";

	if ($error)
	{
		echo "<b>$error</b><br><br>\n";
	}

	if ($allowDownload)
	{
		echo "<table border=1>";
		echo "<form method='post' action='$_SERVER['PHP_SELF']'>";
		echo "<tr><td bgcolor=#DDDDEE>Download file</td><tr>";
		echo "<td>";
		echo "<input type=text name='file' size=40 maxlength=255 value='$file'/>";
		echo "<input type=submit name='download' value='Download'/>";
		echo "</td>";
		echo "</tr>";
		echo "</form>";
		echo "</table>";
		
		echo "<br>";
	}

	if ($allowUpload)
	{
		echo "<table border=1>";
		echo "<form enctype='multipart/form-data' method='post' action='$_SERVER['PHP_SELF']'>";
		echo "<tr><td bgcolor=#DDDDEE>Upload file</td><tr>";
		echo "<td>";
		echo "<input type=file name='upld_file' size=40 maxlength=255/><br>";
		echo "<select name=shard_addr>\n";
		foreach ($shards as $shard)
		{
			$addr = $shard['shard'].".".$shard['server'].".".$shard['service'];
			echo "<option value='$addr'".($shard_addr == $addr ? " selected" : "").">$addr</option>";
		}
		echo "</select>\n";
		echo "<input type=text name='file' size=40 maxlength=255 value='$file'/>";
		echo "<input type=submit name='upload' value='Upload'/>";
		echo "</td>";
		echo "</tr>";
		echo "</form>";
		echo "</table>";
	}
	
/*
	echo "<table border=0><tr valign=top>\n";
	echo "<form method=post action='$_SERVER['PHP_SELF']' name='cmdform'>\n";	
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
					echo "<a href='backup_interface.php?file=".$parse["SaveFile"]."'>Load/Save sheet</a>";
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
*/
	htmlEpilog();
?>
