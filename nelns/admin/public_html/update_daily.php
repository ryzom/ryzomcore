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

	// Was $REMOTE_ADDR (a bare global), which is not set from the request on
	// modern PHP — so the check either always failed closed, or, if something
	// else filled the global, could be spoofed. Only the real peer address.
	if (!isset($_SERVER['REMOTE_ADDR']) || $_SERVER['REMOTE_ADDR'] !== '195.68.21.194')
		die();

	// $version used to arrive as a bare global; take it from the request and
	// keep it to a short printable string before it hits the shard row.
	$version = isset($_GET['version']) ? $_GET['version'] : (isset($_POST['version']) ? $_POST['version'] : null);
	if (!is_string($version) || $version === '' || strlen($version) > 64 || !preg_match('/^[A-Za-z0-9._+-]+$/', $version))
		die('NO VERSION SET');

	include('../config.php');

	//error_reporting(E_NOTICE);

	// ext/mysql went away in php 7; same shape, mysqli underneath. Only the
	// two helpers this script actually uses are kept -- the list/num/field
	// name wrappers had no caller and had no mysqli equivalent worth guessing.
	function connectToDatabase($dbhost, $dbname, $dblogin, $dbpasswd)
	{
		$connect_id = @mysqli_connect($dbhost, $dblogin, $dbpasswd);
		if (!$connect_id)
			die("Unable to connect to MySQL server '$dbhost'");
		if (!mysqli_select_db($connect_id, $dbname))
			die("Unable to select MySQL database '$dbname'");
		@mysqli_set_charset($connect_id, 'utf8mb4');
		return array($connect_id, $dbname);
	}

	function sqldbquery($query, $id)
	{
		global	$queries;
		// here log queries
		mysqli_select_db($id[0], $id[1]);
		$res = @mysqli_query($id[0], $query);
		$queries[] = "'$query' on db ".$id[1];
		return $res;
	}

	function sqlfetch(&$result)
	{
		if (!($result instanceof mysqli_result))
			return false;
		$row = mysqli_fetch_array($result);
		return ($row === null) ? false : $row;
	}

	function sqlaffectedrows($id)
	{
		return mysqli_affected_rows($id[0]);
	}

	function sqlnumrows($result)
	{
		if (!($result instanceof mysqli_result))
			return 0;
		return mysqli_num_rows($result);
	}

	function sqlerr($id)
	{
		return "error ".mysqli_errno($id[0]).": ".mysqli_error($id[0]);
	}

	$dbname = "nel";

	$id = connectToDatabase($dbhost, $dbname, $dblogin, $dbpassword);

	$shardid = "61";
	$query = "UPDATE shard SET Version='".mysqli_real_escape_string($id[0], $version)."' WHERE ShardId='$shardid'";
	$result = sqldbquery($query, $id);

	die ($result ? "[OK]" : "[FAILED]");
?>