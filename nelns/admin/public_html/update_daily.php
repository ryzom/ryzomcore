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

	if ($REMOTE_ADDR != '195.68.21.194')
		die();

	if (!isset($version))
		die('NO VERSION SET');

	include('../config.php');

	//error_reporting(E_NOTICE);

	// connect to database
	function connectToDatabase($dbhost, $dbname, $dblogin, $dbpasswd)
	{
		$connect_id;
		if (!($connect_id = mysql_connect($dbhost, $dblogin, $dbpasswd)))
			die("Unable to connect to MySQL server '$dbhost'");
		if (!mysql_select_db ($dbname, $connect_id))
			die("Unable to select MySQL database '$dbname'");
		return array($connect_id, $dbname);
	}

	function sqldbquery($query, $id)
	{
		global	$queries;
		// here log queries
		mysql_select_db($id[1], $id[0]);
		$res = mysql_query($query, $id[0]);
		$queries[] = "'$query' on db ".$id[1];
		return $res;
	}

	function sqlfetch(&$result)
	{
		return mysql_fetch_array($result);
	}
	
	function sqlaffectedrows($id)
	{
		return mysql_affected_rows($id[0]);
	}
	
	function sqllistfields($table, $id)
	{
		return mysql_list_fields($id[1], $table, $id[0]);
	}
	
	function sqlnumrows($result)
	{
		return mysql_num_rows($result);
	}
	
	function sqlnumfields($result)
	{
		return mysql_num_fields($result);
	}
	
	function sqlfieldname($result, $i)
	{
		return mysql_field_name($result, $i);
	}

	function sqlerr($id)
	{
		return "error ".mysql_errno($id[0]).": ".mysql_error($id[0]);
	}
	
	$dbname = "nel";

	$id = connectToDatabase($dbhost, $dbname, $dblogin, $dbpassword);

	$shardid = "61";
	$query = "UPDATE shard SET Version='$version' WHERE ShardId='$shardid'";
	$result = sqldbquery($query, $id);

	die ($result ? "[OK]" : "[FAILED]");
?>