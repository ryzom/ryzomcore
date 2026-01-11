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

	$publicAccess = false;
	$allowNevrax = true;
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
	
	if ($reset_filters)
	{
		$filter_shard = "";
		$filter_server = "";
		$filter_service = "";
		$filter_entity = "";
	}

	htmlProlog($_SERVER['PHP_SELF'], "Commands");
	
	// input variables :
	// - $preselServ : preselected service address
	// - $execCommand : executed command on preselected service, like a normal service
	//

	echo "Services commands<br>\n";

	echo "<table border=1><form method=post action='".$_SERVER['PHP_SELF']."'>\n";
	echo "<tr><th rowspan=2>&nbsp;Filters&nbsp;</th><th>shard</th><th>server</th><th>service</th><th>entity</th><td rowspan=2>&nbsp;<input type=submit name='display_view' value='Update\nfilters'>&nbsp;</td><td rowspan=2>&nbsp;<input type=submit name='reset_filters' value='Reset\nfilters'>&nbsp;</td></tr>\n";
	echo "<tr>\n";
	echo "<td><input type=text name=filter_shard value='$filter_shard' size=12 maxlength=256></td>\n";
	echo "<td><input type=text name=filter_server value='$filter_server' size=12 maxlength=256></td>\n";
	echo "<td><input type=text name=filter_service value='$filter_service' size=12 maxlength=256></td>\n";
	echo "<td><input type=text name=filter_entity value='$filter_entity' size=28 maxlength=1024></td>\n";
	echo "</tr>\n";
	echo "</form></table><br>\n";

	echo "<table border=0><tr valign=top>\n";
	
	echo "<td>\n";

	echo "<table border=0 cellpadding=2 cellspacing=0>\n";
	echo "<tr><th>Shard</th><th>Server</th><th>Service</th></tr>\n";

	$query = "SELECT shard, server, name FROM service";

	if ($filter_shard != "")	$where[] = "shard like '%$filter_shard%'";
	if ($filter_server != "")	$where[] = "server like '%$filter_server%'";
	if ($filter_service != "")	$where[] = "name like '%$filter_service%'";
	
	if (count($where)>=1)
		$query .= " WHERE ".join(" AND ", $where);

	$query .= " ORDER BY shard, server, name";

	$result = sqlquery($query);

	unset($pshard);
	unset($pserver);
	$scolor = 1;

	while ($result && ($arr=sqlfetch($result)))
	{
		$shard = $arr["shard"];
		$server = $arr["server"];
		$service = $arr["name"];
		$addr = "$shard.$server.$service";
		$dshard = ($pshard == $shard ? "" : $shard);
		if ($pshard != $shard)
			$scolor = 1-$scolor;
		$dserver = ($pserver == $server && $pshard == $shard ? "" : $server);
		$dcolor = "bgcolor=".($scolor==0 ? "#EEEEEE" : "DDDDDD");

		if ($presel_shard == $shard)
			$dcolor = "bgcolor=#FFCCDD";

		if ($presel_shard == $shard && $presel_service != "" && strstr($service, $presel_service) != FALSE)
		{
			$dispServ = "<b><a href='".$_SERVER['PHP_SELF']."?preselServ=$addr'>$service</a></b>";
			$dcolor = "bgcolor=#FF88AA";
		}
		else
			$dispServ = "<a href='".$_SERVER['PHP_SELF']."?preselServ=$addr'>$service</a>";

		echo "<tr><td $dcolor>$dshard</td><td $dcolor>$dserver</td><td $dcolor>$dispServ</td></tr></a>\n";
		$pshard = $shard;
		$pserver = $server;
	}

	echo "</table>\n";
	echo "</td>\n";

	echo "<td width=30>&nbsp;</td>\n";
	
	echo "<td>\n";
	echo "<table border=0><form method=post action='".$_SERVER['PHP_SELF']."' name='cmdform'>\n";
	echo "<tr><th align=left>Service Path</th><th align=left>Command (exact service syntax)</th></tr>\n";
	echo "<tr><td><input name=preselServ value='$preselServ' size=32 maxlength=256></td>\n";
	echo "<td><input name=execCommand value='".stripslashes($execCommand)."' size=50 maxlength=20480></td>\n";
	echo "<td><input type=submit value=Execute></td></tr>\n";
	echo "</form></table>\n";

	if (isset($preselServ) && $preselServ != "" && isset($execCommand) && $execCommand != "")
	{
		$fullCmd = $preselServ.".".stripslashes($execCommand);
		logUser($uid, "SYS_COMMAND=".$fullCmd);
		$qstate = nel_query($preselServ.".".stripslashes($execCommand), $commandResult);
	}

	if ($commandResult)
	{
		echo "<textarea rows=60 cols=300 readOnly style='font-family: Terminal, Courier; font-size: 10pt;'>".stripslashes($commandResult)."</textarea>\n";
	}
	
	echo "</td>\n";

	echo "</tr></table>\n";

	echo "<script type='text/javascript'><!--\n";
	echo "if (document.cmdform) { document.cmdform.execCommand.focus(); }\n";
	echo "// --></script>\n";

	htmlEpilog();
?>
