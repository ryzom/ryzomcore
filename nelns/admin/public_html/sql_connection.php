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

	include('../config.php');

	// The whole tool used to talk to the database through ext/mysql, which
	// php dropped in 7.0: nothing here could run on any supported php. Same
	// wrappers, mysqli underneath, one connection handle in $sqlLink.
	$sqlLink = null;

	// connect to database
	function connectToDatabase($dbhost, $dbname, $dblogin, $dbpasswd)
	{
		global	$sqlLink;
		$sqlLink = @mysqli_connect($dbhost, $dblogin, $dbpasswd);
		if (!$sqlLink)
			return "Unable to connect to MySQL server";
		if (!mysqli_select_db($sqlLink, $dbname))
			return "Unable to select MySQL database";
		@mysqli_set_charset($sqlLink, 'utf8mb4');
		return FALSE;
	}

	// default connection to database
	function defaultConnectToDatabase()
	{
		global	$dbhost, $dbname, $dblogin, $dbpassword;
		return connectToDatabase($dbhost, $dbname, $dblogin, $dbpassword);
	}

	function sqllink()
	{
		global	$sqlLink;
		return $sqlLink;
	}

	function sqlescape($value)
	{
		global	$sqlLink;
		if (!$sqlLink)
			return addslashes((string)$value);
		return mysqli_real_escape_string($sqlLink, (string)$value);
	}

	function sqlquery($query)
	{
		// here log queries
		global	$sqlQueries, $sqlLink;
		if (!$sqlLink)
			return false;
		$res = @mysqli_query($sqlLink, $query);
		$sqlQueries[] = $query.(($res)?"":" ***FAILED***: ".mysqli_error($sqlLink));
		return $res;
	}

	// The callers pass whatever sqlquery() gave back, including the `false` of
	// a failed query and the `true` of an insert; hand back false for anything
	// that is not a result set instead of letting mysqli raise.
	function sqlfetch(&$result)
	{
		if (!($result instanceof mysqli_result))
			return false;
		$row = mysqli_fetch_array($result);
		return ($row === null) ? false : $row;
	}

	function sqlnumrows(&$result)
	{
		if (!($result instanceof mysqli_result))
			return 0;
		return mysqli_num_rows($result);
	}

	function sqlchrows(&$result)
	{
		return sqlaffectedrows();
	}

	function sqlaffectedrows()
	{
		global	$sqlLink;
		return $sqlLink ? mysqli_affected_rows($sqlLink) : 0;
	}

	function sqlerror()
	{
		global	$sqlLink;
		return $sqlLink ? mysqli_error($sqlLink) : '';
	}

	function displayQueries()
	{
		global	$sqlQueries;
		if (isset($sqlQueries))
		{
			echo "<br><br><hr><ul>";
			foreach ($sqlQueries as $query)
				echo "<li>".htmlspecialchars($query, ENT_QUOTES)."</li>\n";
			echo "</ul>";
/*
			echo "<br><br><p align=right><textarea rows=15 cols=100 readOnly>";
			foreach ($sqlQueries as $query)
				echo "$query\n---\n";
			echo "</textarea>\n";
*/
		}
	}
	
	// sqlPopup() opens this script with ?queries=... so it can show the
	// query text. Only accept it from the query string (not as a bare
	// global — that path used to exist under register_globals), and escape
	// it: the text is whatever the caller put in the url.
	if (isset($_GET['queries']) && is_string($_GET['queries']))
	{
		echo "<textarea rows=15 cols=100 readOnly>";
		echo htmlspecialchars($_GET['queries'], ENT_QUOTES);
		echo "</textarea>\n";
		die();
	}

?>