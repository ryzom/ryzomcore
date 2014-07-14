<?php
/* Copyright (C) 2009 Winch Gate Property Limited
 *
 * This file is part of ryzom_api.
 * ryzom_api is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ryzom_api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ryzom_api.  If not, see <http://www.gnu.org/licenses/>.
 */

define('SQL_DEF_TEXT', 0);
define('SQL_DEF_BOOLEAN', 1);
define('SQL_DEF_INT', 2);
define('SQL_DEF_DATE', 3);


// Wrapper for SQL database interactions
class ServerDatabase
{
    var $hostname = '';
    var $username = '';
    var $password = '';
    var $database = '';

    var $_connection = Null;

    function ServerDatabase($host='', $user='', $passwd='', $dbname='')
    {
    	if (($host != '') && ($user != '') && ($dbname != ''))
    	{
	    	$this->hostname = $host;
    		$this->username = $user;
    		$this->password = $passwd;
    		$this->database = $dbname;
    	}

		if (($this->hostname != '') && ($this->username != '') && ($this->database != ''))
			$this->_connection = new mysqli($this->hostname, $this->username, $this->password, $this->database);
    }

    function close()
    {
		$this->_connection->close();
    }

    function query($sql_statement)
    {
		$result = $this->_connection->query($sql_statement);
		if (!$result)
			alert('MYSQL', $this->get_error(), 2);
        return $result;
    }

	function select_db($dbname) {
		$this->database = $dbname;
		$this->_connection->select_db($dbname);
	}

	function num_rows($result)
	{
		return $result->num_rows;
	}

    function fetch_row($result, $result_type=MYSQLI_BOTH)
    {
		if (gettype($result) == "object")
			return $result->fetch_array($result_type);
		 return NULL;
    }

	function fetch_assoc($result)
    {
		if (gettype($result) == "object")
			return $result->fetch_assoc();
		return NULL;
    }

    function query_single_row($sql_statement)
    {
        $result = $this->query($sql_statement);
		if (gettype($result) == "object")
			return $result->fetch_array();

		return NULL;
    }

	function free_result($result)
	{
		$result->free();
	}

    function get_error()
    {
		return $this->_connection->errno.': '.$this->_connection->error;
    }

    function last_insert_id()
    {
		return $this->_connection->insert_id;
    }

    function escape_string($escapestr) {
		return $this->_connection->real_escape_string($escapestr);
    }

    function change_to($host,$user,$pass,$dbname)
    {
    	/*$this->close();
    	$this->hostname = $host;
    	$this->username = $user;
    	$this->password = $pass;
    	$this->database = $dbname;
    	$this->ServerDatabase();*/
    }
}

class ryDB {

	private static $_instances = array();
	private $db;
	private $defs = array();
	private $errors = '';


	private function __construct($db_name) {
		global $_RYZOM_API_CONFIG;
		$this->db_name = $db_name;
		$this->db = new ServerDatabase(RYAPI_WEBDB_HOST, RYAPI_WEBDB_LOGIN, RYAPI_WEBDB_PASS, $db_name);
		$this->db->query("SET NAMES utf8");
	}

	public static function getInstance($db_name) {
		if (!array_key_exists($db_name, self::$_instances))
			self::$_instances[$db_name] = new ryDB($db_name);

		self::$_instances[$db_name]->db->select_db($db_name);
		return self::$_instances[$db_name];
	}

	function setDbDefs($table, $defs, $check=true) {
		if ($check)
		{
			$result = $this->db->query('SHOW FIELDS FROM '.$table);
			if (!$result) {
				die("Table [$table] not found in database [$this->db_name]");
			
			}

			$fields = array_keys($defs);
			while ($row = $this->db->fetch_row($result)) {
				if (in_array($row['Field'], $fields))
					unset($fields[array_search($row['Field'], $fields)]);
				else
					alert('DbLib', 'Missing field '.$row['Field']." on DbDef of table [$table] of database [$this->db_name] !", 2);
			}
			if ($fields)
				die('Missing fields ['.implode('] [', $fields)."] in table [$table] of database [$this->db_name] !");
		}
		$this->defs[$table] = $defs;
	}

	function getDefs($table) {
		if ($this->hasDbDefs($table))
			return $this->defs[$table];

		alert('DBLIB', "Please add tables to '$this->db_name' defs using setDbDefs('$table', \$defs)", 2);
	}

	function hasDbDefs($table) {
		return array_key_exists($table, $this->defs);
	}

	function getErrors() {
		return $this->db->get_error();
	}

	function now() {
		return date('Y-m-d H:i:s', time());
	}

	function toDate($timestamp) {
		return date('Y-m-d H:i:s', $timestamp);
	}

	function fromDate($string_date) {
		return strtotime($string_date);
	}

	function addDbTableProp($table, $props) {
		$this->props[$table] = $props;
	}

	function sqlEscape($escapestr) {
		return $this->db->escape_string($escapestr);
	}

	function insertID() {
		return $this->db->last_insert_id();
	}


	/// DIRECT QUERY
	function sqlQuery($sql, $index = false, $result_type = MYSQLI_BOTH) {
		$result = $this->db->query($sql);
		if (!$result)
			return NULL;
		if($index !== false && !is_array($index)){
			$index = array($index);
		}
		$ret = array();
		while ($row = $this->db->fetch_row($result, $result_type)) {
			if($index !== false) {
				// if $index is ['id1', 'id2'], then this code executes as
				// $ret[$row['id1']][$row['id2']] = $row
				$current = &$ret;
				foreach($index as $key){
					if(!isset($row[$key]))
						alert('DBLIB', "Requested index field ($key) was not selected from db");
					$current = &$current[$row[$key]];
				}
				$current = $row;
			} else
				$ret[] = $row;
		}
		return $ret;
	}


	/// QUERY ///
	function sqlSelect($table, $props, $values=array(), $extra='') {
		if ($table) {
			$sql = "SELECT\n\t";
			$params = array();
			$test = array();
			if (!$props)
				alert('DBLIB', "Bad Select on [$table] : missing props");

			foreach($props as $name => $type)
				$params[] = '`'.$this->sqlEscape($name).'`';

			foreach($values as $name => $value) {
				if ($name[0] == '=')
					$test[] = '('.$this->sqlEscape(substr($name, 1)).' LIKE '.var_export($value, true).')';
				else
					$test[] = '('.$this->sqlEscape($name).' = '.var_export($value, true).')';
			}
			$sql .= implode(",\n\t", $params)."\nFROM\n\t".$table."\n";
			if ($test)
				$sql .= "WHERE\n\t".implode("\nAND\n\t", $test);
		}

		if ($extra)
			$sql .= "\n".$extra;
		return $sql.';';

	}

	function querySingle($table, $values=array(), $extra='') {
		$sql = $this->sqlSelect($table, $this->getDefs($table), $values, $extra);
		$result = $this->sqlQuery($sql, false, MYSQLI_BOTH);
		if(empty($result))
			return NULL;
		return $result[0];
	}

	function querySingleAssoc($table, $values=array(), $extra='') {
		$sql = $this->sqlSelect($table, $this->getDefs($table), $values, $extra);
		$result = $this->sqlQuery($sql, false, MYSQLI_ASSOC);
		if(empty($result))
			return NULL;
		return $result[0];
	}

	function query($table, $values=array(), $extra='', $index = false, $result_type = MYSQLI_BOTH) {
		$sql = $this->sqlSelect($table, $this->getDefs($table), $values, $extra);
		return $this->sqlQuery($sql, $index, $result_type);
	}

	function queryAssoc($table, $values=array(), $extra='', $index = false) {
		return $this->query($table, $values, $extra, $index, MYSQLI_ASSOC);
	}

	function queryIndex($table, $index, $values=array(), $extra='') {
		return $this->query($table, $values, $extra, $index, MYSQLI_ASSOC);
	}


	/// INSERT ///
	function sqlInsert($table, $props, $vals) {
		$sql = 'INSERT INTO '.$table.' ';
		$params = array();
		$values = array();
		foreach($props as $name => $type) {
			if (!isset($vals[$name]))
				continue;
			$params[] = '`'.$name.'`';
			switch ($type) {
				case SQL_DEF_BOOLEAN:
					$values[] = $vals[$name]?1:0;
					break;
				case SQL_DEF_INT:
					$values[] = $vals[$name];
					break;
				case SQL_DEF_DATE: // date
					if (is_string($vals[$name]))
						$values[] = "'".$this->sqlEscape($vals[$name])."'";
					else
						$values[] = "'".$this->toDate($vals[$name])."'";
					break;
				default:
					$values[] = "'".$this->sqlEscape($vals[$name])."'";
					break;
			}
		}
		$sql .= "(\n\t".implode(",\n\t", $params)."\n) VALUES (\n\t".implode(",\n\t", $values)."\n);";
		return $sql;
	}

	function insert($table, $values) {
		$sql = $this->sqlInsert($table, $this->getDefs($table), $values);
		$this->db->query($sql);
		return $this->db->last_insert_id();
	}

	/// DELETE ///
	function sqlDelete($table, $values=array(), $where='') {
		$sql = "DELETE FROM\n\t".$table."\n";
		$test = array();
		foreach($values as $name => $value)
			$test[] = '('.$this->sqlEscape($name).' = '.var_export($value, true).')';

		if ($test or $where)
			$sql .= "WHERE\n\t";
		if ($test)
			$sql .= implode("\nAND\n\t", $test);
		if ($where)
			$sql .= "\n".$where;
		return $sql.';';
	}

	function delete($table, $values=array(), $where='') {
		$sql = $this->sqlDelete($table, $values, $where);
		$result = $this->db->query($sql);
		return $result;
	}

	/// UPDATE ///
	function sqlUpdate($table, $props, $vals, $tests, $extra) {
		$sql = 'UPDATE '.$table.' SET ';
		$params = array();
		$test = array();
		$values = array();
		foreach($props as $name => $type) {
			if (!array_key_exists($name, $vals))
				continue;
			switch ($type) {
				case SQL_DEF_BOOLEAN:
					$values[] = '`'.$name.'` = '.($vals[$name]?'1':'0');
					break;
				case SQL_DEF_DATE:
					if (is_string($vals[$name]))
						$values[] = '`'.$name.'` = \''.$this->sqlEscape($vals[$name]).'\'';
					else
						$values[] = '`'.$name.'` = \''.$this->toDate($vals[$name]).'\'';
					break;
				default:
					$values[] = '`'.$name.'` = \''.$this->sqlEscape($vals[$name]).'\'';
					break;
			}
		}
		$sql .= "\n\t".implode(",\n\t", $values)."\n";

		foreach($tests as $name => $value) {
			$test[] = '('.$this->sqlEscape($name).' = '.var_export($value, true).')';
		}
		if ($test)
			$sql .= "WHERE\n\t".implode("\nAND\n\t", $test);

		$sql .= "\n".$extra;

		return $sql;
	}


	function update($table, $values=array(), $test=array(), $extra='') {
		$sql = $this->sqlUpdate($table, $this->getDefs($table), $values, $test, $extra);
		$result = $this->db->query($sql);
		return $result;
	}

	function sqlInsertOrUpdate($table, $props, $vals) {
		$sql = $this->sqlInsert($table, $props, $vals);
		$sql = substr($sql, 0, strlen($sql)-1);
		$params = array();
		$test = array();
		$values = array();
		foreach($props as $prop) {
			if (!array_key_exists($prop[2], $vals))
			continue;
			$type = $prop[0];
			$check = $prop[1];
			$name = $prop[2];
			if ($type{0} == '#')
			$type = substr($type, 1);
			if (($type{0} == '>') or ($type == 'id'))
			continue;
			switch ($type) {
			case 'trad':
				$values[] = '`'.$name."` = '".$this->sqlEscape($vals[$name])."'";
				break;
			case 'textarea':
			case 'string':
			case 'option':
				$values[] =  '`'.$name."` = '".$this->sqlEscape($vals[$name])."'";
				break;
			case 'id':
			case 'int':
			case 'float':
				$values[] =  '`'.$name.'` = '.$this->sqlEscape($vals[$name]);
				break;
			case 'bool':
				$values[] = '`'.$name.'` = '.($vals[$name]?'1':'0');
				break;
			}
		}
		$sql .= "\nON DUPLICATE KEY UPDATE\n\t".implode(",\n\t", $values)."\n";
		return $sql;
	}

	function insertOrUpdate($table, $values) {
		$sql = $this->sqlInsertOrUpdate($table, $this->getDefs($table), $values);
		return $result;
	}


	/// Display
	function getTableHtml($name, $params, $values, $order_by='')
	{
		$ret = '<table cellpadding="0" cellspacing="0" width="100%">';
		$tr_header = '<td align="left" height="32px">&nbsp;';
		$tr_header .= implode('</td><td align="left">&nbsp;', array_keys($params)).'</td>';
		$ret .= _s('t header', $tr_header);
		$i = 0;
		if (!$values)
			return '';
		$current_section = '';
		foreach ($values as $rows) {
			if ($order_by && $rows[$order_by] != $current_section) {
				$current_section = $rows[$order_by];
				if ($current_section != '0')
					$ret .= _s('t row ', '<td>'._s('section', $current_section).'</td>'.str_repeat('<td>'._s('section', '&nbsp;').'</td>', count($params)-1));
			}
			$td = '';
			foreach ($params as $test => $param)
				$td .= '<td align="left" height="22px">&nbsp;'.$rows[$param].'</td>';
			$ret .= _s('t row '.strval($i % 2), $td);
			$i++;
		}
		$ret .= '</table>';
		return $ret;
	}

	/// Update Database Structure

	static function updateDatabaseStruct($defs)
	{
		if (file_exists(RYAPP_PATH.'database.versions'))
			$versions = unserialize(file_get_contents(RYAPP_PATH.'database.versions'));
		else
			$versions = array();

		$c = "Updating DB Structure...\n";
		foreach ($defs as $dbname => $tables) {
			$db = new ServerDatabase(RYAPI_WEBDB_HOST, RYAPI_WEBDB_LOGIN, RYAPI_WEBDB_PASS, $dbname);
			$db->query("SET NAMES utf8");
			$c .= "\n	Selected DB '$dbname'\n";
			foreach ($tables as $table => $sql)
			{
				$version = count($sql);
				if (array_key_exists($table, $versions))
					$diff = $version - $versions[$table];
				else {
					$versions[$table] = 0;
					$diff = $version;
				}

				$c .= "		Table '$table' need v$version (current v".strval($versions[$table].') => ');

				if ($diff > 0) {
					$sql_to_run = array_slice($sql, $versions[$table], $diff);
					foreach($sql_to_run as $sql_run) {
						if ($sql_run) {
							$c .= "Run sql... ";
							$result = $db->query($sql_run);
						} else
							$c .= "KO!!!";
					}
					if ($result) {
						$c .= "OK";
						$versions[$table] = $version;
					}
				} else
					$c .= "OK";
				$c .= "\n";
			}
			$c .= "\n";
			$db->close();
		}
		file_put_contents(RYAPP_PATH.'database.versions', serialize($versions));
		return '<pre>'.$c.'<pre>';
	}
}

?>
