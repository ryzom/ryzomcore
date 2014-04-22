<?php
/***************************************************************************
 *                                 mysql.php
 *                            -------------------
 *   begin                : Saturday, Feb 13, 2001
 *   copyright            : (C) 2001 The phpBB Group
 *   email                : support@phpbb.com
 *
 *   $Id: functions_mysql.php,v 1.2 2006/07/06 15:17:22 powles Exp $
 *
 ***************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 ***************************************************************************/

/***************************************************************************
 *
 * NOTE: this is striped down version from phpBB mysql.php file
 *       modified to work with mysqli
 *
 ***************************************************************************/

if(!defined("SQL_LAYER"))
{

define("SQL_LAYER","mysqli");

class sql_db
{

	var $db_connect_id;
	var $query_result;
	var $num_queries = 0;

	//
	// Constructor
	//
	function sql_db($sqlserver, $sqluser, $sqlpassword, $database, $persistency = true)
	{

		$this->persistency = $persistency;
		$this->user = $sqluser;
		$this->password = $sqlpassword;
		$this->server = $sqlserver;
		$this->dbname = $database;

		if($this->persistency)
		{
			$this->server = 'p:'.$this->server;
		}

		$this->db_connect_id = mysqli_connect($this->server, $this->user, $this->password);
		if($this->db_connect_id)
		{
			if($database != "")
			{
				$this->dbname = $database;
				$dbselect = mysqli_select_db($this->db_connect_id, $this->dbname);
				if(!$dbselect)
				{
					mysqli_close($this->db_connect_id);
					$this->db_connect_id = $dbselect;
				}
			}
			return $this->db_connect_id;
		}
		else
		{
			echo "Connection to mySQL failed!";
			exit;
		}
	}

	//
	// Other base methods
	//
	function sql_close()
	{
		if($this->db_connect_id)
		{
			if($this->query_result)
			{
				@mysqli_free_result($this->query_result);
			}
			$result = mysqli_close($this->db_connect_id);
			return $result;
		}
		else
		{
			return false;
		}
	}

	//
	// Base query method
	//
	function sql_query($query = "", $transaction = FALSE)
	{
		// Remove any pre-existing queries
		unset($this->query_result);
		if($query != "")
		{
			nt_common_add_debug($query);
			$this->num_queries++;
			$this->query_result = mysqli_query($this->db_connect_id, $query);
		}
		if($this->query_result)
		{
			return $this->query_result;
		}
		else
		{
			return ( $transaction == 'END_TRANSACTION' ) ? true : false;
		}
	}

	function sql_select_db($dbname)
	{
		if($this->db_connect_id)
		{
			$result = mysqli_select_db($this->db_connect_id, $dbname);
			return $result;
		}
		return false;
	}
	function sql_reselect_db()
	{
		if($this->db_connect_id)
		{
			$result = mysqli_select_db($this->db_connect_id, $this->dbname);
			return $result;
		}
		return false;
	}
	//
	// Other query methods
	//
	function sql_numrows($query_id = 0)
	{
		if(!$query_id)
		{
			$query_id = $this->query_result;
		}
		if($query_id)
		{
			$result = mysqli_num_rows($query_id);
			return $result;
		}
		else
		{
			return false;
		}
	}
	function sql_affectedrows()
	{
		if($this->db_connect_id)
		{
			$result = mysqli_affected_rows($this->db_connect_id);
			return $result;
		}
		else
		{
			return false;
		}
	}
	function sql_numfields($query_id = 0)
	{
		if(!$query_id)
		{
			$query_id = $this->query_result;
		}
		if($query_id)
		{
			$result = mysqli_num_fields($query_id);
			return $result;
		}
		else
		{
			return false;
		}
	}
	// function sql_fieldname($query_id = 0){}
	// function sql_fieldtype($offset, $query_id = 0){}
	function sql_fetchrow($query_id = 0)
	{
		if(!$query_id)
		{
			$query_id = $this->query_result;
		}
		if($query_id)
		{
			return mysqli_fetch_array($query_id);
		}
		else
		{
			return false;
		}
	}
	function sql_fetchrowset($query_id = 0)
	{
		if(!$query_id)
		{
			$query_id = $this->query_result;
		}
		if($query_id)
		{
			while($row = mysqli_fetch_array($query_id))
			{
				$result[] = $row;
			}
			return $result;
		}
		else
		{
			return false;
		}
	}
	// function sql_fetchfield($field, $rownum = -1, $query_id = 0){}
	// function sql_rowseek($rownum, $query_id = 0){}
	function sql_nextid(){
		if($this->db_connect_id)
		{
			$result = mysqli_insert_id($this->db_connect_id);
			return $result;
		}
		else
		{
			return false;
		}
	}
	function sql_freeresult($query_id = 0){
		if(!$query_id)
		{
			$query_id = $this->query_result;
		}

		if ( $query_id )
		{
			@mysqli_free_result($query_id);

			return true;
		}
		else
		{
			return false;
		}
	}
	function sql_error($query_id = 0)
	{
		$result["message"] = mysqli_error($this->db_connect_id);
		$result["code"] = mysqli_errno($this->db_connect_id);

		return $result;
	}

} // class sql_db

class sql_db_string extends sql_db
{
	//
	// Constructor ($connstring format : mysql://user:password@host/dbname)
	//
	function sql_db_string($connstring, $persistency = true)
	{
		$ret = false;
		if ($connstring != '')
		{
			if (preg_match("#^mysqli?://([^:]+)(?::([^@]*))?@([^\\/]+)/([^/]+)[/]?$#", $connstring, $params))
			{
				$sqlserver		= $params[3];
				$sqluser		= $params[1];
				$sqlpassword	= $params[2];
				$database		= $params[4];

				$ret = $this->sql_db($sqlserver, $sqluser, $sqlpassword, $database, $persistency);
			}
		}

		return $ret;
	}
} // class sql_db_string


} // if ... define

